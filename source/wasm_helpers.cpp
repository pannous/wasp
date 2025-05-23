#include "wasm_helpers.h"
#include "Node.h"
#include "Code.h"
#include <cmath>

// functions in this file are wasm realted and applicable in both the guest and host context
// For implementations witch are only applicable in guest OR host see wasm_helpers_host.cpp / wasm_helpers_wasm.cpp


// call this from builtin _start => wasp_main !
extern "C" void printNode(smart_pointer_64 node) {
    print(smartNode(node));
}

const char *RUNTIME_ONLY_ERROR = "This variant of wasp.wasm compiled as 'RUNTIME_ONLY'";
#ifdef RUNTIME_ONLY // No Angle.cpp!
void clearContext() {}
Node &analyze(Node &node, String context) { return *new Node(RUNTIME_ONLY_ERROR); }
Node eval(String code) { return Node(RUNTIME_ONLY_ERROR); }
Node interpret(String code) { return Node(RUNTIME_ONLY_ERROR); }
extern "C" int64 run_wasm_file(chars file) {
	error(RUNTIME_ONLY_ERROR);
	return -1;
}
//void testCurrent(){}// why??
#endif
#if not CONSOLE

void console() { error(RUNTIME_ONLY_ERROR); }

#endif
#ifdef RUNTIME_ONLY_MOCK
// mock
Node analyze(Node data){return data;};// wasp -> code  // build ast via operators
Node eval(String code){return Node(code);};// wasp -> code -> data   // interpreter mode vs:
Node emit(String code){return Node(code);};//  wasp -> code -> wasm  // to debug currently same as:
//Node parse(String code) {};// wasp -> data  // this is the pure Wasp part
//Node run(String source){};// wasp -> code -> wasm() -> data
//Code &emit(Node root_ast, Module *runtime0, String _start){};
Node Node::evaluate(bool){ return *this; }
int read_wasm(chars wasm_path){};
int run_wasm(chars wasm_path){};
int run_wasm(bytes data,int size){}
Module& read_wasm(chars file){return *new Module();}
Node &analyze(Node &node, String context) { return *new Node(RUNTIME_ONLY_ERROR); }
Node eval(String code) { return Node(RUNTIME_ONLY_ERROR); }
Node interpret(String code) { return Node(RUNTIME_ONLY_ERROR); }
void clearContext() {}
extern "C" int64 run_wasm_file(chars file) {
	error(RUNTIME_ONLY_ERROR);
	return -1;
}
void console() { error(RUNTIME_ONLY_ERROR); }
void testCurrent(){}// why??
#endif


//#if WASM
// NEEDS TO BE IN Wasp because __wasm_call_ctors !
//unsigned int *memory=0;// NOT USED without wasm! static_cast<unsigned int *>(malloc(1000000));

// https://stackoverflow.com/questions/49980381/find-the-start-of-heap-using-lld-and-s2wasm

int isNumber(char c) {
    return (c >= '0' and c <= '9');
}

int isalnum0(int c) {
    return (c >= '0' and c <= '9') or (c >= 'a' and c <= 'z') or (c >= 'A' and c <= 'Z'); // todo lol
}

// todo put to util
#if not WEBAPP
[[noreturn]]
#endif
void error1(chars message, chars file, int line) {
#ifdef _Backtrace_
    //	Backtrace(2);// later, in raise
#endif
    printf("%s", message);
#if WEBAPP
#endif
#if WASM
	put_chars("\n⚠️ERROR\n");
	// put_chars("vscode:"s+file+":"+line);
	put_chars(""s+file+":"+line);
//    put_chars((char *)file);
//    put_chars(":");
//    puti(line); !!?
	put_chars((char *) (message));
	proc_exit(-1);// wasmtime hack to print backtrace:  exit with invalid exit status outside of [0..126) ;)
#else
    if (file)printf("\n%s:%d\n", file, line);
    raise(message);
#endif
#if not WEBAPP
    //	if (throwing)
    // throw Error(message); // [[noreturn]] should not return
    throw message; // [[noreturn]] should not return
#else
    proc_exit(-1);
    throw message;
#endif
}


void newline() {
    put_chars("\n", 1);
    //    put_char('\n');
}

void info(chars msg) {
    if (not tracing)return; // todo finer levels!
#if not RELEASE
    print(msg);
#endif
}

void warn(chars warning) {
    print(warning);
}

void warn(String warning) {
#if not RELEASE
    printf("%s\n", warning.data);
#endif
}

void warning(chars warning) {
    print(warning); // for now
}

int raise(chars error) {
#if WASM
	put_chars("\n⚠️ERROR\n");
	put_chars((char *) error);
	put_chars("\n");
#endif
    if (panicking)
        proc_exit(-1);
#if not WEBAPP // ⚠️ not caught in std::thread teste(testCurrent); => crashes WebApp
    if (throwing)
        throw error;
    // throw Error(error);
#endif
    return -1;
}


// todo: how to do routes in wasp? serve "/" { "hello" }   serve "/file" { "file" }
// https://developer.fermyon.com/wasm-languages/cpp
// https://developer.fermyon.com/wasm-languages/c
// >> https://deislabs.io/posts/introducing-wagi-easiest-way-to-build-webassembly-microservices/ <<
// wrapper to serve functions in wasp
void serve(chars path = "") {
    // todo called from _start !
    if (getenv("SERVER_SOFTWARE"))
        printf("Content-Type: text/plain\n\n"); // todo html
    const char *p = getenv("QUERY_STRING");
    String query = p;
    Strings queries = query.split("&");
    Map<String, chars> params;
    for (auto q: queries) {
        Strings kv = q.split("=");
        if (kv.size() == 2) {
            params[kv[0]] = kv[1].data;
            tracef("QUERY PARAM: %s: %s\n", kv[0].data, kv[1].data);
        }
    }
    // todo: call wasp function here
    //    auto args=params;
    //    smart_pointer_32 result = main(args.size(), (char **) args.capacity /*hack ;)*/);
    printf("Hello, World!\n");
}

/* HTTP_HOST = foo.example.com
SERVER_NAME = foo.example.com
SERVER_SOFTWARE = WAGI/1
GATEWAY_INTERFACE = CGI/1.1
HTTP_USER_AGENT = curl/7.64.1
X_FULL_URL = http://foo.example.com/hello?greet=matt
REQUEST_METHOD = GET
SERVER_PROTOCOL = http
HTTP_ACCEPT = *
SERVER_PORT = 80
PATH_INFO = /hello
PATH_TRANSLATED = /env ???
QUERY_STRING = greet=matt
*/

void serve_file(chars path = "") {
    const char *p = getenv("X_RELATIVE_PATH");
    printf("Content-Type: text/plain\n\n");

    FILE *fptr = fopen(p, "r");
    if (fptr == NULL) {
        fprintf(stderr, "Fileserver: File not found %s\n", p);
        printf("Status: 404\n\nNot Found\n");
        exit(0);
    }

    fprintf(stderr, "Fileserver: Loading file %s\n", p);

    int ch;
    while ((ch = fgetc(fptr)) != EOF) {
        putchar(ch);
    }

    fclose(fptr);
}

// wasm has sqrt opcode, ignore √ in interpreter for now! cmath only causes problems, including 1000 on mac and print()
double sqrt1(double a) {
#if WASM
	// IT WORKS!
	__asm("local.get 0");
	__asm("f64.sqrt");
	__asm("local.set 0");
	return a;
#else
    return sqrt(a);
#endif
}


#if NO_TESTS
extern "C" void testCurrent(){
	print("no tests");
}
#endif

#if MY_WASM

Code &compile(String code, bool clean = true);// exposed to wasp.js

extern "C" char *run(chars x) {
	auto code = compile(x);
	auto smartNode = code.run();// async in js
	if (smartNode) {
		auto pNode = reconstructWasmNode(smartNode);
		if (pNode)
			return pNode->serialize();
	}
	return (char *) "need asyncify for result";
}
#endif
