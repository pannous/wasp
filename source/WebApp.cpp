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

    int64 result() {
        int64 r;
        int64 t;
        pthread_mutex_lock(&m_mutex);
        r = m_result;
        m_done = false;// restart / allow another wait
        pthread_mutex_unlock(&m_mutex);
        return r;
    }


    Node resultNode() {
        pthread_mutex_lock(&m_mutex);
        Node result(m_result, (smart_type_64) m_type);
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

webview::webview w(true, nullptr);// global for lambdas
std::string testWebview(std::string s);


void render(String html) {
    std::string data = html.data;
    w.navigate("data:text/html," + data);
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
    w.navigate("data:text/html," + html->str());
}
//char *page=0;// use inline html, else go straight to page todo: file:// URLs?
// char *page="test";// doesn't
//char *page="data:text/html,test";// OK!
// char *page="https://wasm-feature-detect.surma.technology/";// referenceTypes supported in Safari but not in WebKit!?
// char *page="file://index.html";// doesn't
// char *page="file:///Users/me/index.html";// works, but how to get local file paths AT COMPILE TIME?? ok for debugging!
#ifdef SERVER
chars page = "http://localhost:8080/test.html";
#else
chars page = "file:///Users/me/wasp/source/test.html";
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
    w.bind(name, func);
}

void bind(char *name, char *(*func)(char *)) {
    w.bind(name, [func](std::string s) -> std::string {
        func(s.data());
        return s;
    });
}

void splitLog(const std::string s) {
    w.set_title(s);
    int index = 0;
    std::cout << s << std::endl;
    std::string item;
//	while (index < 100) {
//		item = webview::json_parse(s, "", index++);
//		std::cout << item << std::endl;
//		if (item.length() == 0)break;
//	}
}

void navigate(String url) {
    page = url;
    w.navigate(page);
}

int64 open_webview(String url = "") {
    if (!url.empty())page = url;
    printf("\nWebView!\n");

    // add [w] to closure to make it local
    w.set_title("Example");
    w.value("alert('js injected into every page')");
    w.set_size(480 * 4, 320 * 4, WEBVIEW_HINT_NONE);// default
    w.set_size(480, 320, WEBVIEW_HINT_MIN);// minimum size, also: MAX, FIXED
    w.bind("run", [](std::string s) -> std::string {
        throwing = false;
        const std::string &code = webview::json_parse(s, "", 0);
        printf("RUN: %s", code.data());
        panicking = false;
        std::thread compile(eval, String(code.data()));
        compile.detach();
        return "compiling…";// will run wasm HERE and print result
    });
    w.bind("exit", [](std::string s) -> std::string {
        printf("EXIT");
        exit(0);
    });
    w.bind("server", [](std::string s) -> std::string {
        std::thread teste(start_server, 9999);
        teste.detach();
        return s;
    });
    w.bind("close", [](std::string s) -> std::string {
        w.terminate();
        exit(0);
    });
    w.bind("destroy", [](std::string s) -> std::string {
        w.terminate();
        return s;
    });

    w.bind("wasm_done", [](std::string s) -> std::string {
        printf("wasm_done  result json = %s", s.c_str());
        const std::string &string = webview::json_parse(s, "", 0);
#if MULTI_VALUE
        auto type = webview::json_parse(string, "", 0);
        auto val = webview::json_parse(string, "", 1);
        waiter.done(std::stol(val), std::stol(type));
#else
        int64 result0 = std::stol(string);
        printf("wasm_done  result = %d \n", result0);
        waiter.done(result0);
#endif
        return s;
    });
    w.bind("wasm_error", [](std::string s) -> std::string {
        printf("wasm_error %s \n", s.data());
        waiter.done(-1);
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
//        w.eval(s);
        return s;
    });

    // window.open and window.navigate don't work
    // w.bind("test", [](std::string s) -> std::string { w.eval("window.location.href='https://www.yay.com/'"); return s;});
    w.bind("test", [](std::string s) -> std::string {
        testWebview(s);
        return s;
    });// works, with
    w.bind("alert", [](std::string s) -> std::string {
        splitLog(s);
        return s;
    });// no native popup?
    // why does alert('a') print 'a' alert(1) print 1, even though lambda type is string?
    w.bind("log", [](std::string s) -> std::string {
        splitLog(s);
        return s;
    });
    w.bind("add", [](std::string s) -> std::string {
        auto a = std::stol(webview::json_parse(s, "", 0));
        auto b = std::stol(webview::json_parse(s, "", 1));
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
//    w.eval("alert('ok!?')");// was w.exec, js injected into current page
    //    CGBitmapContextCreate(w.window(),1600,1200,32,8/*?*/, 0,0);
    //    CGBitmapContextCreateWithData(w.window(),1600,1200,32,8/*?*/, 0,0,0,0);
    //    webview_bind(w.window(),"custom_function")
    //        webview::cocoa_wkwebview_engine;

    w.run(); //  we have to call our tests from js to continue in thread!!!
    printf("DONE\n");//never reached, even after calling terminate() from js/c/wasp
    return 0;
}

extern "C" int64 init_graphics() {
    open_webview();
}

//int paint(int wasm_offset) {
// copy wasp data to canvas
//}

void run_wasm_async(unsigned char *bytes, int length) {
    std::stringstream ss;
    ss << "code=new Uint8Array([";
    for (int i = 0; i < length; i++) ss << ((short) bytes[i]) << ",";
    ss << "]);wasmx(code);";
    w.eval(ss.str());
}


int paint(int wasm_offset) {
    w.eval("paintWasmToCanvas()");// data coming from wasm
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
    w.eval(js);// hangs if …
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
//	testCurrent();
    return s;
}

std::string
fetch(std::string s) { return "I can download/load any file I want!!"; }// but return type is wrong so we need json.parse or some cast!
