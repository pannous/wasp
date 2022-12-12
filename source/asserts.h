#pragma once
#undef assert // <cassert> / <assert.h>  assert.h:92 not as good!


#define backtrace_line() {printef("\n%s:%d\n",__FILE__,__LINE__);exit(0);}
//#define backtrace_line(msg) {printef("\n%s\n%s:%d\n",#msg,__FILE__,__LINE__);exit(1);}

#define assert(condition) try{\
if((condition)==0){printef("\n%s\n",#condition);error("assert FAILED");}else printef("\nassert OK: %s\n",#condition);\
}catch(chars m){printef("\n%s\n%s\n%s:%d\n",m,#condition,__FILE__,__LINE__);exit(1);}

// TODO silent asserts outside of tests!
#define assert_equals(α, β) if (!assert_equals_x(α,β)){printef("%s != %s",#α,#β);backtrace_line();}
//#define check_eq assert_equals
//#define check_is assert_equals

bool assert_equals_x(String a, String b, char *context = "");

bool assert_equals_x(long a, long b, char *context = "");

//bool assert_equals_x(float a, float b, char *context = "");
bool assert_equals_x(double a, double b, char *context = "");
