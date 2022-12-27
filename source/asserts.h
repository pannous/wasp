#pragma once
#undef assert // <cassert> / <assert.h>  assert.h:92 not as good!


#define backtrace_line() {printf("\n%s:%d\n",__FILE__,__LINE__);proc_exit(0);}
//#define backtrace_line() {printf("\nfile://%s :%d\n",__FILE__,__LINE__);proc_exit(0);}
//#define backtrace_line() {printf("\nsubl://%s :%d\n",__FILE__,__LINE__);proc_exit(0);}
//#define backtrace_line(msg) {printf("\n%s\n%s:%d\n",#msg,__FILE__,__LINE__);proc_exit(1);}

#define assert(condition) try{\
if((condition)==0){printf("\n%s\n",#condition);error("assert FAILED");}else printf("\nassert OK: %s\n",#condition);\
}catch(chars m){printf("\n%s\n%s\n%s:%d\n",m,#condition,__FILE__,__LINE__);proc_exit(1);}

// TODO silent asserts outside of tests!
#define assert_equals(α, β) if (!assert_equals_x(α,β)){printf("%s != %s",#α,#β);backtrace_line();}
//#define check_eq assert_equals
//#define check_is assert_equals

bool assert_equals_x(String a, String b, chars context = "");

bool assert_equals_x(int64 a, int64 b, chars context = "");

//bool assert_equals_x(float a, float b, chars context = "");
bool assert_equals_x(double a, double b, chars context = "");
