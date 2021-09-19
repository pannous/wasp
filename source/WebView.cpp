#include <webview.h>
#include <iostream>
#include <sstream>
// 104234 bytes if compiled with -Oz
// todo: remove std::string from webview.h for much smaller size?

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


unsigned char test_prog_wasm[] = {
		0x00, 0x61, 0x73, 0x6d, 0x01, 0x00, 0x00, 0x00, 0x01, 0x15, 0x04, 0x60,
		0x02, 0x7f, 0x7f, 0x01, 0x7f, 0x60, 0x00, 0x00, 0x60, 0x03, 0x7f, 0x7f,
		0x7f, 0x01, 0x7f, 0x60, 0x00, 0x01, 0x7e, 0x02, 0x1c, 0x02, 0x03, 0x65,
		0x6e, 0x76, 0x0a, 0x65, 0x78, 0x74, 0x5f, 0x6d, 0x65, 0x6d, 0x63, 0x70,
		0x79, 0x00, 0x02, 0x03, 0x65, 0x6e, 0x76, 0x03, 0x73, 0x75, 0x6d, 0x00,
		0x00, 0x03, 0x04, 0x03, 0x01, 0x03, 0x00, 0x05, 0x06, 0x01, 0x01, 0x80,
		0x02, 0x80, 0x02, 0x06, 0x09, 0x01, 0x7f, 0x01, 0x41, 0x80, 0x8c, 0xc0,
		0x02, 0x0b, 0x07, 0x28, 0x04, 0x06, 0x6d, 0x65, 0x6d, 0x6f, 0x72, 0x79,
		0x02, 0x00, 0x04, 0x74, 0x65, 0x73, 0x74, 0x00, 0x04, 0x0b, 0x74, 0x65,
		0x73, 0x74, 0x5f, 0x6d, 0x65, 0x6d, 0x63, 0x70, 0x79, 0x00, 0x03, 0x06,
		0x5f, 0x73, 0x74, 0x61, 0x72, 0x74, 0x00, 0x02, 0x0a, 0x71, 0x03, 0x03,
		0x00, 0x01, 0x0b, 0x59, 0x02, 0x01, 0x7f, 0x01, 0x7e, 0x23, 0x00, 0x41,
		0x10, 0x6b, 0x22, 0x00, 0x24, 0x00, 0x20, 0x00, 0x42, 0x00, 0x37, 0x03,
		0x08, 0x20, 0x00, 0x41, 0xe7, 0x8a, 0x8d, 0x09, 0x36, 0x02, 0x04, 0x20,
		0x00, 0x41, 0xef, 0x9b, 0xaf, 0xcd, 0x78, 0x36, 0x02, 0x00, 0x20, 0x00,
		0x41, 0x08, 0x6a, 0x20, 0x00, 0x41, 0x04, 0x6a, 0x41, 0x04, 0x10, 0x00,
		0x1a, 0x20, 0x00, 0x41, 0x08, 0x6a, 0x41, 0x04, 0x72, 0x20, 0x00, 0x41,
		0x04, 0x10, 0x00, 0x1a, 0x20, 0x00, 0x29, 0x03, 0x08, 0x21, 0x01, 0x20,
		0x00, 0x41, 0x10, 0x6a, 0x24, 0x00, 0x20, 0x01, 0x0b, 0x11, 0x00, 0x20,
		0x00, 0x20, 0x01, 0x6a, 0x20, 0x00, 0x20, 0x01, 0x6b, 0x10, 0x01, 0x41,
		0x02, 0x6d, 0x0b, 0x0b, 0x0a, 0x01, 0x00, 0x41, 0x80, 0x0c, 0x0b, 0x03,
		0xa0, 0x06, 0x50
};

int run_wasm(unsigned char *bytes, int length) {
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
//	printf("%s", wasm.data());

	// 3. feed natively how? BBQ OMG JIT wasm LLInt (low level interpreter)
	// irrelevant / unprofessional? https://www.youtube.com/watch?v=1v4wPoMskfo
	// https://webkit.org/blog/9329
	return -42;// cant wait!
}

std::string testWebview(std::string s) {
	testCurrent();
	return s;
}

std::string
fetch(std::string s) { return "I can download/load any file I want!!"; }// but return type is wrong so we need json.parse or some cast!
