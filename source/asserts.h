#pragma once
#undef assert // <cassert> / <assert.h>  assert.h:92 not as good! and needs "env" "__assert_fail"

#include "List.h"
#include "Util.h" // define backtrace_exit

#if EMSCRIPTEN

#elif MY_WASM or WASM and DEBUG
extern "C" void assert_expect(Node *result);
extern "C" void async_yield();// throw this run and reenter after run_wasm is done
#elif WEBAPP
extern "C" void assert_expect(Node*){} // dummies
extern "C" void async_yield() {}
#endif

extern Node &result;

#if WASM
#define skip(test)
#else
#define skip(test) print("SKIPPING");print(#test);debug_line();
#endif


#if EMSCRIPTEN
#define assert_is(α, β) printf("%s\n%s:%d\n",α,__FILE__,__LINE__);if (!assert_isx(α,β)){printf("%s != %s",#α,#β);backtrace_line();}
#elif MY_WASM and not EMSCRIPTEN
#define assert_is(α, β) if(!done.has(α)){ done.add(α);assert_expect(new Node(β));eval(α);async_yield();};
#else

//// MACRO to catch the line number. WHY NOT WITH TRACE? not precise:   testMath() + 376
#define assert_is(wasp, result) \
printf("TEST %s==%s\n",#wasp,#result); \
debug_line();\
ok=assert_isx(wasp,result);\
if(ok)printf("PASSED %s==%s\n",#wasp,#result);\
else{printf("FAILED %s==%s\n",#wasp,#result); \
backtrace_exit()}
#endif

#define assert(condition) try{\
if((condition)==0){printf("\n%s\n",#condition);error("assert FAILED");}else printf("\nassert OK: %s\n",#condition);\
}catch(chars m){printf("\n%s\n%s",m,#condition);backtrace_exit()}

// TODO silent asserts outside of tests!
#define assert_equals(α, β) if (!assert_equals_x(α,β)){printf("%s != %s",#α,#β);backtrace_exit();}


// todo merge with assert_isx
bool assert_equals_x(String a, String b, chars context = "");

bool assert_equals_x(Node &a, char *b, chars context = "");

bool assert_equals_x(Node a, const char *b, chars context = "");

bool assert_equals_x(Node a, int b, chars context = "");

bool assert_equals_x(Node a, char b, chars context = "");

bool assert_equals_x(Node a, wchar_t b, chars context = "");

bool assert_equals_x(Node a, char16_t b, chars context = "");

bool assert_equals_x(Node a, codepoint b, chars context = "");

bool assert_equals_x(Node a, bool b, chars context = "");

bool assert_equals_x(Node &a, wchar_t b, chars context = "");

bool assert_equals_x(Node &a, double b, chars context = "");

bool assert_equals_x(Node a, double b, chars context = "");

bool assert_equals_x(Node a, int64 b, chars context = "");

bool assert_equals_x(Node a, String b, chars context = "");

bool assert_equals_x(Node a, Node b, chars context = "");

bool assert_equals_x(Node *a, const Node *b, chars context = "");

bool assert_equals_x(Node *a, Node *b, chars context = "");

bool assert_equals_x(int64 a, int64 b, chars context = "");

bool assert_equals_x(int a, int b, chars context = "");

bool assert_equals_x(Kind a, Kind b, chars context = "");

bool assert_equals_x(double a, double b, chars context = "");

bool assert_equals_x(float a, float b, chars context = "");

bool assert_isx(chars wasp, Node expect);

bool assert_isx(chars mark, chars expect);

bool assert_isx(chars mark, int expect);

bool assert_isx(chars mark, int64 expect);

bool assert_isx(chars mark, double expect);

bool assert_isx(chars mark, bool expect);

String normSerialization(String input);

void assertSerialize(const char *input);

Node assert_parsesx(chars mark);

static List<String> done;

#if EMSCRIPTEN
#define assert_emit(α, β) printf("%s\n%s:%d\n",α,__FILE__,__LINE__);if (!assert_equals_x(eval(α),β)){printf("%s != %s",#α,#β);backtrace_exit();}
#elif (MY_WASM or WASM) and not EMSCRIPTEN
#define assert_emit(α, β) if(!done.has(α)){ done.add(α);assert_expect(new Node(β));eval(α);async_yield();};
#else
// #define assert_emit(α, β) printf("%s\n%s:%d\n",α,__FILE__,__LINE__);if (!assert_isx(eval(α),β)){printf("%s != %s",#α,#β);backtrace_exit();}
#define assert_emit(α, β) printf("%s\n%s:%d\n",α,__FILE__,__LINE__);if (!assert_equals_x(eval(α),β)){printf("%s != %s",#α,#β);backtrace_exit();}
#endif
//#define assert_emit(α, β) try{printf("%s\n%s:%d\n",α,__FILE__,__LINE__);if (!assert_equals_x(emit(α),β)){printf("%s != %s",#α,#β);backtrace_exit();}}catch(chars x){printf("%s\nIN %s",x,α);backtrace_exit();}
#define  check_emit assert_emit
#if RUNTIME_ONLY or MY_WASM
#define assert_run(a, b) skip(a)
// use assert_emit if runtime is not needed!! much easier to debug
#else
#define assert_run(mark, result) if(!assert_equals_x(runtime_emit(mark), result)){backtrace_exit();}
#endif


#if WASM or WEBAPP
#define assert_throws(αα)
#else

#define assert_throws(αα)  {print(#αα);debug_line();bool old=panicking;try{ \
panicking=false;throwing=true;eval(αα);printf("SHOULD HAVE THROWN!\n%s\n",#αα);backtrace_exit(); \
}catch(chars){}catch(String*){}catch(...){};panicking=old;}
#endif

#define assert_parses(marka) result=assert_parsesx(marka);if(result==ERROR){printf("NOT PARSING %s\n",marka);backtrace_exit();}
