#include <webview.h>
#include <iostream>
#include <sstream>
// 104234 bytes if compiled with -Oz
// todo: remove std::string from webview.h for much smaller size?

#include <condition_variable>
#include <thread>
#include <chrono>
//#include<pthread.h> c way?
//pthread_t wait_for_wasm[2];

std::condition_variable wasm_condition;
//https://en.cppreference.com/w/cpp/thread/condition_variable
std::mutex wasm_result_mutex;
//	https://www.cplusplus.com/reference/mutex/mutex/
// Even if the shared variable is atomic, it must be modified under the mutex

#include <stdatomic.h>

int wasm_error_code = -1;

class Wait {
public:
	void done(int result) {
		pthread_mutex_lock(&m_mutex);
		m_done = true;
		m_result = result;
		pthread_cond_broadcast(&m_cond);
		pthread_mutex_unlock(&m_mutex);
	}

	void wait() {
		pthread_mutex_lock(&m_mutex);
		while (!m_done) pthread_cond_wait(&m_cond, &m_mutex);
		pthread_mutex_unlock(&m_mutex);
	}

	int result() {
		int r;
		pthread_mutex_lock(&m_mutex);
		r = m_result;
		pthread_mutex_unlock(&m_mutex);
		return r;
	}

private:
	atomic_int m_result = -3;
	pthread_mutex_t m_mutex;
	pthread_cond_t m_cond;
	bool m_done;
};

static Wait waiter;

void wasm_done(int wasm_result0) {
	waiter.done(wasm_result0);
	{
		std::lock_guard<std::mutex> lk(wasm_result_mutex);
		wasm_error_code = 0;
	}
//	std::cerr << "Notifying ...\n";
	wasm_condition.notify_all();//
}

void wasm_error(std::string s) {
	waiter.done(-2);
	{
		std::lock_guard<std::mutex> lk(wasm_result_mutex);
		wasm_error_code = -1;
	}
}

webview::webview w(true, nullptr);// global for lambdas
std::string testWebview(std::string s);

void testCurrent();

void render(std::string html) {
	w.navigate("data:text/html," + html);
}

//char *page=0;// use inline html, else go straight to page todo: file:// URLs?
// char *page="test";// doesn't
//char *page="data:text/html,test";// OK!
// char *page="https://wasm-feature-detect.surma.technology/";// referenceTypes supported in Safari but not in WebKit!?
// char *page="file://index.html";// doesn't
// char *page="file:///Users/me/index.html";// works, but how to get local file paths AT COMPILE TIME?? ok for debugging!
char *page = "file:///Users/me/wasp/source/test.html";
// char *page="data:text/html,<body onclick='close()'>test</body>";
// char *page=page || "data:text/html,\n<html><body style='height:999px;' onclick='close()'>X</body></html>";// why not??
// char *page="data:text/html, <!doctype html><html><body style='height: 1000px;' onclick='close()'>X</body></html>";// why not??
// char *page=R"(data:text/html,test)";
// char *page=R"HTML(data:text/html,test)HTML";
// todo: best way for compile time loading of pages?
// char* page=
// #include "index.html"
const char *home = page ? page : "https://wasm-feature-detect.surma.technology/";


// bind any function to the webview, should also work for wasm/wasp functions, no?
void bind(char *name, std::string (*func)(std::string)) {
	w.bind(name, func);
}

void bind(char *name, char *(*func)(char *)) {
	w.bind(name, [func](std::string s) -> std::string {
		func(s.data());
		return s;
	});
}

int init_graphics() {
	printf("\nWebView!\n");

	// add [w] to closure to make it local
	w.set_title("Example");
	w.init("alert('js injected into every page')");
	w.set_size(480 * 4, 320 * 4, WEBVIEW_HINT_NONE);// default
	w.set_size(480, 320, WEBVIEW_HINT_MIN);// minimum size, also: MAX, FIXED
	w.bind("exit", [](std::string s) -> std::string {
		printf("EXIT");
		exit(0);
		return s;
	});
	w.bind("close", [](std::string s) -> std::string {
		w.terminate();
		exit(0);
		return s;
	});
	w.bind("destroy", [](std::string s) -> std::string {
		w.terminate();
		return s;
	});

	w.bind("wasm_done", [](std::string s) -> std::string {
		const std::string &string = webview::json_parse(s, "", 0);
		int result0 = std::stoi(string);
		printf("wasm_done  result = %d \n", result0);
		wasm_done(result0);
		return s;
	});
	w.bind("wasm_error", [](std::string s) -> std::string {
		printf("wasm_error %s \n", s.data());
		wasm_error(webview::json_parse(s, "", 0));
		return s;
	});
	w.bind("terminate", [](std::string s) -> std::string {
		w.terminate();
		return s;
	});
	// w.bind("destroy", [](std::string s) -> std::string { w.destroy(); return s;}); NOPE

	w.bind("home", [](std::string s) -> std::string {
		w.navigate(home);
		return s;
	});
	w.bind("$", [](std::string s) -> std::string {
		printf("$('%s')? jquery needs to be injected!", s.data());
		w.eval(s);
		return s;
	});

	// window.open and window.navigate don't work
	// w.bind("test", [](std::string s) -> std::string { w.eval("window.location.href='https://www.yay.com/'"); return s;});
	w.bind("test", [](std::string s) -> std::string {
		testWebview(s);
		return s;
	});// works, with
	w.bind("alert", [](std::string s) -> std::string {
		if (not s.find(",")) // keep [x,y]
			s = webview::json_parse(s, "", 0);// [x] => x
		w.set_title(s);
		std::cout << s << std::endl;
		return s;
	});// no native popup?
	// why does alert('a') print 'a' alert(1) print 1, even though lambda type is string?
	w.bind("logs", [](std::string s) -> std::string {
		w.set_title(s);
		std::cout << s << std::endl;
		return s;
	});
	w.bind("add", [](std::string s) -> std::string {
		auto a = std::stoi(webview::json_parse(s, "", 0));
		auto b = std::stoi(webview::json_parse(s, "", 1));
		return std::to_string(a + b);
	});

	// printf("%s",w.return("{'test':'value'}"));// json_parse_c()?
	// w.on_message() private but interesting… !
	// [[config preferences] setValue:@YES forKey:@"developerExtrasEnabled"]

//  onKeyPress='exit()'
	if (page) w.navigate(page);
//    else // todo: simple templates? use wasp::String ;)
//    w.eval("document.onclick=test");// no effect
	// w.eval("document.onclick=exit");// no effect

//    w.resolve("canvas? direct? may be hard or impossible",0,0);
	//      std::wstring userDataFolder =
	//        wideCharConverter.from_bytes(std::getenv("APPDATA"));
	w.eval("alert('ok!?')");// was w.exec, js injected into current page
	//    CGBitmapContextCreate(w.window(),1600,1200,32,8/*?*/, 0,0);
	//    CGBitmapContextCreateWithData(w.window(),1600,1200,32,8/*?*/, 0,0,0,0);
	//    webview_bind(w.window(),"custom_function")
	//        webview::cocoa_wkwebview_engine;

	w.run(); //  we have to call our tests from js to continue in thread!!!
	printf("DONE\n");//never reached, even after calling terminate() from js/c/wasp
	return 0;
}


void requestAnimationFrame() {
	// copy wasp data to canvas
}

void run_wasm_async(unsigned char *bytes, int length) {
	std::stringstream ss;
	ss << "code=new Uint8Array([";
	for (int i = 0; i < length; i++) ss << ((short) bytes[i]) << ",";
	ss << "]);wasmx(code);";
	w.eval(ss.str());
}


// forced synchronous
int run_wasm_sync(unsigned char *bytes, int length) {
	// 1. save to APPDATA folder and then fetch via js
	// NOPE "fetch api cannot load file" could bind my own fetch though!

	// 2. serialize as ecma string "code=[0x00, 0x01, ..." and  eval
	std::stringstream ss;
	ss << "code=new Uint8Array([";
	for (int i = 0; i < length; i++) {
		ss << std::hex << std::showbase << ((int) bytes[i]) << ", ";
	}
	ss << "]);wasmx(code);";
	std::string wasm = ss.str();
	w.eval(wasm);
//	sleep(2);// lets the sub-thread wait but returns immediately WTF!?! I see why people hate c
//	waiter.wait();
	{
		std::unique_lock<std::mutex> lk(wasm_result_mutex);
		std::cerr << "Waiting... \n";
//		wasm_condition.wait(lk, [] { return wasm_error_code >= 0; });
		wasm_condition.wait(lk);
		std::cerr << "...finished waiting. result = " << waiter.result();
//		printf("finished Waiting  result = %d \n", wasm_result);
	}
	// 3. feed natively how? BBQ OMG JIT wasm LLInt (low level interpreter)
	// irrelevant / unprofessional? https://www.youtube.com/watch?v=1v4wPoMskfo
	// https://webkit.org/blog/9329
	return waiter.result();// cant wait!
}


int run_wasm(unsigned char *bytes, int length) {
	run_wasm_sync(bytes, length);
	return waiter.result();
}

std::string testWebview(std::string s) {
	std::thread teste(testCurrent);
	teste.detach();
//	testCurrent();
	return s;
}

std::string
fetch(std::string s) { return "I can download/load any file I want!!"; }// but return type is wrong so we need json.parse or some cast!
