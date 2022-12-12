#include <cmath>
#include "wasm_helpers.h"
#include "Node.h"


#ifdef RUNTIME_ONLY // No Angle.cpp!
const char *RUNTIME_ONLY_ERROR = "This variant of wasp.wasm compiled as 'RUNTIME_ONLY'";
void clearContext() {}
Node &analyze(Node &node, String context) { return *new Node(RUNTIME_ONLY_ERROR); }
Node eval(String code) { return Node(RUNTIME_ONLY_ERROR); }
Node interpret(String code) { return Node(RUNTIME_ONLY_ERROR); }
extern "C" long run_wasm_file(chars file) {
    error(RUNTIME_ONLY_ERROR);
    return -1;
}
//void testCurrent(){}// why??
#endif
const char *RUNTIME_ONLY_ERROR = "This variant of wasp.wasm was compiled as 'RUNTIME_ONLY'";
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
extern "C" long run_wasm_file(chars file) {
    error(RUNTIME_ONLY_ERROR);
    return -1;
}
void console() { error(RUNTIME_ONLY_ERROR); }
void testCurrent(){}// why??
#endif


//#if WASM
// NEEDS TO BE IN Wasp because __wasm_call_ctors !
//unsigned int *memory=0;// NOT USED without wasm! static_cast<unsigned int *>(malloc(1000000));
char *__heap_base = (char *) memory;
//unsigned char* __heap_base=0;
char *memoryChars = (char *) memory;
//int HEAP_OFFSET=65536/2; // todo: how done right? if too low, internal data gets corrupted ("out of bounds table access" etc)
//#define HEAP_OFFSET 65536
//int memory_size=1048576-HEAP_OFFSET; // todo set in CMake !
char *current = (char *) HEAP_OFFSET;
//#endif


int isalnum0(int c) {
    return (c >= '0' and c <= '9') or (c >= 'a' and c <= 'z') or (c >= 'A' and c <= 'Z');// todo lol
}

// todo put to util
[[noreturn]]
void error1(chars message, chars file, int line) {
#ifdef _Backtrace_
//	Backtrace(2);// later, in raise
#endif
    if (file)printf("\n%s:%d\n", file, line);\
    raise(message);
    if (panicking) panic();// not reached
    throw message;// not reached
}


void newline() {
    put_char('\n');
}

void info(chars msg) {
    if (not debug)return;// todo finer levels!
    printf("%s", msg);
    newline();
}

void warn(chars warning) {
    printf("%s", warning);
    newline();
}

void warn(String warning) {
    printf("%s", warning.data);
    newline();
}

void warning(chars warning) {
    printf("%s", warning);// for now
}

int raise(chars error) {
    if (panicking)
        proc_exit(-1);
    throw error;
}


// wasm has sqrt opcode, ignore √ in interpreter for now! cmath only causes problems, including 1000 on mac and print()
double sqrt1(double a) {
#ifndef WASM
    return sqrt(a);
#endif
    todo("wasm has it's own sqrt. how to add wasm inline ");
    return -1;
}

// todo: remove this useless test function
int square(int a) {
    return a * a;
}
