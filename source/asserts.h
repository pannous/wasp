#pragma once
#undef assert // <cassert> / <assert.h>  assert.h:92 not as good!

#include "List.h"

#if EMSCRIPTEN

#elif MY_WASM or WASM and DEBUG
extern "C" void assert_expect(Node *result);
extern "C" void async_yield();// throw this run and reenter after run_wasm is done
#endif

#define backtrace_line() {printf("\n%s:%d\n",__FILE__,__LINE__);proc_exit(0);}

//#if WASM
//#define debug_line() print(__FILE__);print(":");print(__LINE__);
//// printf in WASM messes up the stack, so we can't use it
//#else
//#endif
//#define debug_line() printf("\n%s:%d\n",__FILE__,__LINE__);

//#define backtrace_line() {printf("\nfile://%s :%d\n",__FILE__,__LINE__);proc_exit(0);}
//#define backtrace_line() {printf("\nsubl://%s :%d\n",__FILE__,__LINE__);proc_exit(0);}
//#define backtrace_line(msg) {printf("\n%s\n%s:%d\n",#msg,__FILE__,__LINE__);proc_exit(1);}

#define assert(condition) try{\
if((condition)==0){printf("\n%s\n",#condition);error("assert FAILED");}else printf("\nassert OK: %s\n",#condition);\
}catch(chars m){printf("\n%s\n%s",m,#condition);backtrace_line()}

// TODO silent asserts outside of tests!
#define assert_equals(α, β) if (!assert_equals_x(α,β)){printf("%s != %s",#α,#β);backtrace_line();}
//#define check_eq assert_equals
//#define check_is assert_equals

bool assert_equals_x(String a, String b, chars context = "");

bool assert_equals_x(int64 a, int64 b, chars context = "");

//bool assert_equals_x(float a, float b, chars context = "");
bool assert_equals_x(double a, double b, chars context = "");


static List<String> done;

#if EMSCRIPTEN
#define assert_emit(α, β) printf("%s\n%s:%d\n",α,__FILE__,__LINE__);if (!assert_equals_x(eval(α),β)){printf("%s != %s",#α,#β);backtrace_line();}
#elif MY_WASM and not EMSCRIPTEN
#define assert_emit(α, β) if(!done.has(α)){ done.add(α);assert_expect(new Node(β));eval(α);async_yield();};
#else
#define assert_emit(α, β) printf("%s\n%s:%d\n",α,__FILE__,__LINE__);if (!assert_equals_x(eval(α),β)){printf("%s != %s",#α,#β);backtrace_line();}
#endif
//#define assert_emit(α, β) try{printf("%s\n%s:%d\n",α,__FILE__,__LINE__);if (!assert_equals_x(emit(α),β)){printf("%s != %s",#α,#β);backtrace_line();}}catch(chars x){printf("%s\nIN %s",x,α);backtrace_line();}

#if RUNTIME_ONLY or MY_WASM
#define assert_run(a, b) skip(a)
// use assert_emit if runtime is not needed!! much easier to debug
#else
#define assert_run(mark, result) if(!assert_equals_x(runtime_emit(mark), result)){backtrace_line();}

//#define assert_run(α, β)  auto α1=runtime_emit(α);bool Ok= α == Node(β); print(Ok?"OK":"FAILED"); \
//    print(α1);print(Ok?'=':u'≠');print(β);} \
//    if(!Ok){printf("%s != %s",#α,#β);backtrace_line();}
#endif
