#pragma once

#define allow_untyped_nodes true  // IMPORTANT!  {a b c}#2"=="b" VALID or NOT ?!?

//typedef int64 i64;
typedef long long int64;
typedef unsigned long long uint64;
typedef unsigned long long u64;
//typedef uint64 bytes8;
typedef unsigned char byte;
typedef const char *chars;
typedef byte *bytes;

#define breakpoint_helper printf("\n%s:%d breakpoint_helper\n",__FILE__,__LINE__);

//#define min(a, b) (a < b ? a : b)
static int64 min(int64 a, int64 b) { return (a < b ? a : b); }

#if LINUX
static bool isnumber(char x){return x>='0' and x<='9';}
#endif

// saves one backtrace entry:
//#if WASM
//#define error(msg) {\
//put_chars("\nERROR\n"); \
//put_chars(__FILE__);\
//put_chars(":");\
//puti(__LINE__);\
//put_chars((char *) (msg));\
//put_chars("\n");   \
//proc_exit(-1); }
//#else
#define error(msg) error1(msg,__FILE__,__LINE__)
#define werror(msg) warn(msg)
//#endif



#define internal_error(msg) error1("internal error: " msg,__FILE__,__LINE__)

//typedef int64 int64;  = 0x7E, in enum

static bool I_know_what_I_am_doing = false;

#if RUNTIME_ONLY
#define todo(msg)
#define todow(msg)
#else
#define todo(msg) {breakpoint_helper;error1(String("TODO ") + String(msg),__FILE__,__LINE__);}
#define todow(msg) {warn(String("TODO ") + String(msg));}
//#define todow(msg) {breakpoint_helper;warn(str("TODO ") + msg);}
#endif

#if TRACE
static bool tracing = true;
#else
static bool tracing = false;
#endif

#if RELEASE
// SEND trace args to NIRVANA so they don't get emitted!
#define trace(x)
#define tracef(x, ...)
#else
#define trace(x) if(tracing)print(x)
#define tracef(x, ...) if(tracing)printf(x,__VA_ARGS__)
#endif

typedef unsigned char byte;
typedef const char *chars;
typedef byte *bytes;

// silent ++
#define check_is(α, β) if((α)!=(β)){printf("%s != %s :\n",#α,#β);print(α);print(" != ");print(β);printf("\n%s:%d\n",__FILE__,__LINE__);proc_exit(0);}
#define check_eq(α, β) if((α)!=(β)){printf("%s != %s :\n",#α,#β);printf("\n%s:%d\n",__FILE__,__LINE__);proc_exit(0);}
#define check_eq_or(α, β, ɣ) if((α)!=(β)){printf("%s != %s : ",#α,#β);printf("\n%s:%d\n",__FILE__,__LINE__);ɣ;proc_exit(0);}

//#define assert(test) if(!(test)){printf("\nNOT PASSING %s\n%s:%d\n",#test,__FILE__,__LINE__);proc_exit(0);}
#define check_silent(test) if(!(test)){printf("\nNOT PASSING %s\n%s:%d\n",#test,__FILE__,__LINE__);proc_exit(0);}

#define check(test) {printf("CHECKING %s\n%s:%d\n",#test,__FILE__,__LINE__); \
  if(test){print("OK check passes: ");printf("%s\n",#test);}else{printf("\nNOT PASSING %s\n%s:%d\n",#test,__FILE__,__LINE__);proc_exit(0);}}

#include "String.h" // AFTER defines!
#include "smart_types.h"

[[noreturn]]
extern void error1(chars message, chars file = 0, int line = 0);

extern void info(chars);

extern void warn(chars);

extern void warning(chars);

extern chars fetch(chars url);

int fileSize(char const *file);

chars concat(chars a, chars b);

template<class S>
// list HAS TO BE 0 terminated! Dangerous C!! ;)
bool contains(S list[], S match);

bool contains(chars list[], chars match);

unsigned int wordHash(const char *str, int max_chars = 20);

char *readFile(chars filename, int *size_out = 0);// 0 = err

//bool similar(float a, float b);
bool similar(double a, double b);

void lowerCase(char *string, int i);

double mod_d(double x, double y);

extern double pi;

bytes concat(bytes a, bytes b, int len_a, int len_b);

bytes concat(bytes a, char b, int len);

bytes concat(char section, bytes a, int len_a);

//inline int abs_i(int x) noexcept;
//inline int64 abs_l(int64 x) noexcept;
//inline float abs_f(float x) noexcept;

//float ln(float y);
//float log(float y,float base=10);
//#undef log2
//#undef log10
//float log2(float y)noexcept;
//float log10(float y)noexcept;
//double sin(double x);
//double cos(double x);
//double pow(double x, double y);
//double floor(double x);

void warn(String warning);

bool fileExists(String filename);

String findFile(String filename, String current_dir);// empty() if not found
//char *findFile(char* filename);// 0 if not found
String load(String file);

bool isSmartPointer(int64 d);

//Node smartValue(int64 smartPointer);
Node *smartNode(smart_pointer_64 smartPointer64);

int64 file_last_modified(char *file);

String extractFuncName(const String &fun);

String extractPath(String file);

bool isDir(const char *name);

#define maxi(a, b) a<b?b:a

//#define max(a, b) a<b?b:a too many conflicts with math.h common.h …
//inline just hides export name in wasm, no other optimization todo: maybe … somehow? use maxi! don't care!
static bool max(int64 a, int64 b) {
    return a < b ? b : a;
}

typedef String Path;

#define quit(α) printf("quit %s",#α);breakpoint_helper;proc_exit(42);

//#include <cstdlib> // exit
//static void quit(){
//    breakpoint_helper;
//    exit(42);
//}
template<class S>
class List;

[[maybe_unused]] List<String> arguments();
