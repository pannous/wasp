#define WEBVIEW_DEPRECATED_PRIVATE
// ^^ undo deprecation ;)
#include <webview.h>
#include <iostream>
#include <sstream>
#include <thread>
//#include <stdatomic.h>
// 104234 bytes if compiled with -Oz
// todo: remove std::string from webview.h for much smaller size?
#include "Paint.h"
#include "Node.h"
#include "Angle.h"
#include "WebServer.hpp"
#include "NodeTypes.h"
#include "tests.h"

using namespace std;
/* supported in WebKit:
✔️	multiValue
✔️	mutableGlobals
 */

// lsappinfo  shows
/*
75) "wasp" ASN:0x0-0x5fa5fa:
    bundleID=[ NULL ]
    bundle path="/Users/me/dev/apps/wasp/cmake-build-default/wasp"

 77) "wasp Web Content" ASN:0x0-0x5fc5fc:
    bundleID="com.apple.WebKit.WebContent"
    bundle path="/System/Library/Frameworks/WebKit.framework/Versions/A/XPCServices/com.apple.WebKit.WebContent.xpc"
    executable path="/System/Library/Frameworks/WebKit.framework/Versions/A/XPCServices/com.apple.WebKit.WebContent.xpc/Contents/MacOS/com.apple.WebKit.WebContent"
    pid = 9863 !cgsConnection !signalled type="UIElement" flavor=3 Version="16611.3.10.1.6" fileType="XPC!" creator="????" Arch=ARM64 sandboxed

defaults write com.apple.WebKit.WebContent WebKitDeveloperExtras -bool true

 [macOS] Private API usage -> REJECTION
 The commit introducing clipboard access uses private APIs for its macOS implementation.
 This can result in App Review REJECTION for apps submitted to the Mac App Store. We should try to find another way to do this without using private APIs.

 * */

// Call `gtk_window_fullscreen`, convert window to `C.GtkWindow` pointer.
//C.gtk_window_fullscreen((*C.GtkWindow)(window))


class Wait {
public:
	void done(int64 result) {
		pthread_mutex_lock(&m_mutex);
		m_done = true;
		m_result = result;
		pthread_cond_broadcast(&m_cond);
		pthread_mutex_unlock(&m_mutex);
	}

	void done(std::string result) {
		pthread_mutex_lock(&m_mutex);
		m_done = true;
		String data = String(result.data());
		m_result = Node(data, strings).toSmartPointer();
		pthread_cond_broadcast(&m_cond);
		pthread_mutex_unlock(&m_mutex);
	}

	void done(int64 result, int64 type) {
		pthread_mutex_lock(&m_mutex);
		m_done = true;
		m_result = result;
		m_type = type;
		pthread_cond_broadcast(&m_cond);
		pthread_mutex_unlock(&m_mutex);
	}

	void wait() {
		pthread_mutex_lock(&m_mutex);
		while (!m_done)
			pthread_cond_wait(&m_cond, &m_mutex);
		pthread_mutex_unlock(&m_mutex);
	}

#if MULTI_VALUE
	Node result() {
		Node r;
		int64 t;
		pthread_mutex_lock(&m_mutex);
		r = m_result;
		m_done = false;// restart / allow another wait
		pthread_mutex_unlock(&m_mutex);
		return r;
	}
#else

	int64 result() {
		int64 r;
		int64 t;
		pthread_mutex_lock(&m_mutex);
		r = m_result;
		m_done = false;// restart / allow another wait
		pthread_mutex_unlock(&m_mutex);
		return r;
	}

#endif


	Node resultNode() {
		pthread_mutex_lock(&m_mutex);
		Node result(m_result, (SmartPointer64) m_type);
		m_done = false;// restart / allow another wait
		pthread_mutex_unlock(&m_mutex);
		return result;
	}

private:
//	atomic_int
	int64 m_result = -1;
	int64 m_type = -1;
	pthread_mutex_t m_mutex;
	pthread_cond_t m_cond;
	bool m_done;
};

//static
Wait waiter;

webview::webview view(true, nullptr);// global for lambdas
std::string testWebview(std::string s);


void render(String html) {
	std::string data = html.data;
	view.navigate("data:text/html," + data);
}

// this is useless because it can't be called from wasm, only from native code.
// unless the native code parses wasp and holds the Node anyref which can then be handed over by wasm!!!
// this wouldn't work in the browser, but browsers could have their own render()!
// how to get Nodes from native <> linear memory?
// WebView has no shared access to js `memory` object
void render(Node &node, std::stringstream *html) {
	if (not html)html = new std::stringstream();
	if (node.kind != strings)
		*html << "<" << node.name << ">";
	else *html << "<" << node.value.string << ">";
	for (Node &child: node) {
		render(child, html);
	}
	if (node.kind != strings)
		*html << "</" << node.name << ">";
	printf("HTML:\n%s\n", html->str().data());
	view.navigate("data:text/html," + html->str());
}
// char *page=0;// use inline html, else go straight to page
// char *page="test";// doesn't
// char *page="data:text/html,test";// OK!
// char *page="https://wasm-feature-detect.surma.technology/";// referenceTypes supported in Safari but not in WebKit!?
// char *page="file://index.html";// doesn't
// char *page="file:///Users/me/index.html";// works, but how to get local file paths AT COMPILE TIME?? ok for debugging!
#ifdef SERVER
chars page = "http://localhost:8080/test.html";
#else
chars page = "file:///Users/me/wasp/test/test.html";
#endif
// char *page="data:text/html,<body onclick='close()'>test</body>";
// char *page=page || "data:text/html,\n<html><body style='height:999px;' onclick='close()'>X</body></html>";// why not??
// char *page="data:text/html, <!doctype html><html><body style='height: 1000px;' onclick='close()'>X</body></html>";// why not??
// char *page=R"(data:text/html,test)";
// char *page=R"HTML(data:text/html,test)HTML";
// todo: best way for compile time loading of pages?
// char* page=
// #include "index.html"
const char *home = "https://wasm-feature-detect.surma.technology/";


// bind any function to the webview, should also work for wasm/wasp functions, no?
void bind(char *name, std::string (*func)(std::string)) {
	view.bind(name, func);
}

void bind(char *name, char *(*func)(char *)) {
	view.bind(name, [func](std::string s) -> std::string {
		func(s.data());
		return s;
	});
}

void splitLog(const std::string s) {
	std::string item;
	int index = 0;
//	while (index < 100) {
//		item = webview::json_parse(s, "", index++);
//		std::cout << item << std::endl;
//		if (item.length() == 0)break;
//	}
}

void navigate(String url) {
	page = url;
	view.navigate(page);
}


void run_wasm_async(unsigned char *bytes, int length) {
	std::stringstream ss;
	ss << "code=new Uint8Array([";
	for (int i = 0; i < length; i++) ss << ((short) bytes[i]) << ",";
	ss << "]);wasmx(code);";
	view.eval(ss.str());
}


// todo: replaced by requestAnimationFrame pull vs push
int paint(int wasm_offset) {
	view.eval("paintWasmToCanvas()");// data coming from wasm
	return 0;
}

// forced synchronous
#if MULTI_VALUE
Node
#else

int64
#endif
run_wasm_sync(unsigned char *bytes, int length) {
	// 1. save to APPDATA folder and then fetch via js
	// NOPE "fetch api cannot load file" could bind my own fetch though!

	// 2. serialize as ecma string "code=[0x00, 0x01, ..." and  eval
	std::stringstream ss;
	ss << "code=new Uint8Array([";
	for (int i = 0; i < length; i++) {
		ss << std::hex << std::showbase << ((int) bytes[i]) << ", ";
	}
	ss << "]);wasmx(code);";
	const std::basic_stringstream<char, std::char_traits<char>, std::allocator<char>>::string_type &js = ss.str();
	view.eval(js);// hangs if …
	waiter.wait();

	// 3. feed natively how? BBQ OMG JIT wasm LLInt (low level evaler)
	// irrelevant / unprofessional? https://www.youtube.com/watch?v=1v4wPoMskfo
	// https://webkit.org/blog/9329
#if MULTI_VALUE
	return waiter.resultNode();
#else
	return waiter.result();// cant wait!
#endif
}


extern "C" int64 run_wasm(unsigned char *bytes, int length) {
	run_wasm_sync(bytes, length);
	return waiter.result();
}

std::string testWebview(std::string s) {
	std::thread teste(testCurrent);
	teste.detach();
	return s;
}



void console_log(const char *s) {
#if DEBUG
	view.eval("print('" + std::string(s) + "')"); // to $results
#else
	view.eval("console.log('" + std::string(s) + "')");
#endif
}

void load_script_include(String url) {

//	https://github.com/webview/webview/issues/851 Custom URI scheme support

//	https://github.com/MicrosoftEdge/WebView2Feedback/issues/1660
//	WebView2.CoreWebView2.SetVirtualHostNameToFolderMapping("assets", "./assets", CoreWebView2HostResourceAccessKind.Allow);

//	https://github.com/webview/webview/issues/859
//	win32_edge_engine public interface: void set_virtual_hostname( const char *hostname, const char *folderpath)
//	static_cast<ICoreWebView2_3*>(m_webview)->SetVirtualHostNameToFolderMapping(L"assets", L"./assets",COREWEBVIEW2_HOST_RESOURCE_ACCESS_KIND_DENY_CORS);
//	https://appassets.example/assets/xxx -> Client/dist/xxx
//
//	String data = readFile("wasp.js");
	String data = readFile("test/test_include.js");
//	String safe=data.replaceAll("'", "\\'");
//	auto js = "var script = document.createElement('script');script.src = '"_s + safe +
//	          "';document.head.appendChild(script);console.log('XXXXXXXX script included',script);";
//	view.eval(js.data);
	view.eval(data.data);
//	view.window().set_virtual_hostname("assets", "$PWD");
	auto script = "<script>"_s + data + "</script>";
	view.set_html(script.data);
}


int64 open_webview(String url = "") {
	if (!url.empty())page = url;
	printf("\nWebView!\n");

	// add [w] to closure to make it local
	view.set_title("Example");
	view.set_html("<script>alert('js injected into every page')</script>");
//    w.value("alert('js injected into every page')");
	view.set_size(480 * 4, 320 * 4, WEBVIEW_HINT_NONE);// default
	view.set_size(480, 320, WEBVIEW_HINT_MIN);// minimum size, also: MAX, FIXED
	view.bind("run", [](std::string s) -> std::string {
		throwing = false;
		panicking = false;
		const std::string &code = webview::json_parse(s, "", 0);
		printf("RUN code: %s\n", code.data());
		std::thread compile(eval, String(code.data()));
		compile.detach();
//        printf("compiling …\n");
		return "compiling…";// will run wasm HERE and print result
	});
	view.bind("exit", [](std::string s) -> std::string {
		printf("EXIT");
		exit(0);
	});
	view.bind("server", [](std::string s) -> std::string {
		std::thread teste(start_server, 9999);
		teste.detach();
		return s;
	});
	view.bind("close", [](std::string s) -> std::string {
		view.terminate();
		exit(0);
	});
	view.bind("destroy", [](std::string s) -> std::string {
		view.terminate();
		return s;
	});
	// doesn't work because it returns async Promise, which can't be used in wasm
//	view.bind("new_string", [](std::string s) -> std::string {
//		const std::string &string = webview::json_parse(s, "", 0);
//		String data = String(string.data());
//		Node &node = *new Node(data, strings);// todo?
//		return to_string(node.toSmartPointer());// parsed as BigInt later
//	});
	view.bind("wasm_done", [](std::string s) -> std::string {
		printf("wasm_done  result json = %s ", s.c_str());
		const std::string &string = webview::json_parse(s, "", 0);
		if (string.empty())return s;
		if (string.starts_with('"')) {
			waiter.done(string);
			return s;
		}
		long result = std::stol(string);
		if (result)
			waiter.done(std::stol(string));
		else
			waiter.done(string);
//#if MULTI_VALUE
//        auto type = webview::json_parse(string, "", 0);
//        auto val = webview::json_parse(string, "", 1);
//        waiter.done(std::stol(val), std::stol(type));
//#else
//        int64 result0 = std::stol(string);
//        printf("wasm_done  result = %ld %lx \n", result0, result0);
//        waiter.done(result0);
//#endif
		return s;
	});
	view.bind("wasm_error", [](std::string s) -> std::string {
		printf("wasm_error %s \n", s.data());
		waiter.done(-1);
		return s;
	});
	view.bind("terminate", [](std::string s) -> std::string {
		view.terminate();
		return s;
	});
	// w.bind("destroy", [](std::string s) -> std::string { w.destroy(); return s;}); NOPE

	view.bind("home", [](std::string s) -> std::string {
		view.navigate(home);
		return s;
	});
//	view.bind("$", [](std::string s) -> std::string { // just use function $() in wasp.js
//		printf("$('%s')? jquery needs to be injected!", s.data());
//		view.eval(s);
////        w.eval(s);
//		return s;
//	});

	// window.open and window.navigate don't work
	// w.bind("test", [](std::string s) -> std::string { w.eval("window.location.href='https://www.yay.com/'"); return s;});
	view.bind("test", [](std::string s) -> std::string {
		testWebview(s);
		return s;
	});// works, with
//	view.bind("alert", [](std::string s) -> std::string {
//        splitLog(s);
//        return s;
//    });// no native popup?
	// why does alert('a') print 'a' alert(1) print 1, even though lambda type is string?
	view.bind("log", [](std::string s) -> std::string {
		std::cout << s << std::endl;
		view.set_title(s);
		splitLog(s);
		return s;
	});
	view.bind("add", [](std::string s) -> std::string {
		auto a = std::stol(webview::json_parse(s, "", 0));
		auto b = std::stol(webview::json_parse(s, "", 1));
		return std::to_string(a + b);
	});
	// printf("%s",w.return("{'test':'value'}"));// json_parse_c()?
	// w.on_message() private but interesting… !
	// [[config preferences] setValue:@YES forKey:@"developerExtrasEnabled"]

//  onKeyPress='exit()'
	if (page) view.navigate(page);
//    else // todo: simple templates? use wasp::String ;)
//    w.eval("document.onclick=test");// no effect
	// w.eval("document.onclick=exit");// no effect

//    w.resolve("canvas? direct? may be hard or impossible",0,0);
	//      std::wstring userDataFolder =
	//        wideCharConverter.from_bytes(std::getenv("APPDATA"));
	view.eval("alert('ok!?')");// was w.exec, js injected into current page
//    w.eval("alert('ok!?')");// was w.exec, js injected into current page
	//    CGBitmapContextCreate(w.window(),1600,1200,32,8/*?*/, 0,0);
	//    CGBitmapContextCreateWithData(w.window(),1600,1200,32,8/*?*/, 0,0,0,0);
	//    webview_bind(w.window(),"custom_function")
	//        webview::cocoa_wkwebview_engine;

	load_script_include("wasp.js");

	view.run(); //  we have to call our tests from js to continue in thread!!!
	printf("DONE\n");//never reached, even after calling terminate() from js/c/wasp
	return 0;
}

extern "C" int64 init_graphics() {
	open_webview();
}