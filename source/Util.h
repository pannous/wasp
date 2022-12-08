#pragma once

//#define min(a, b) (a < b ? a : b)
static long min(long a, long b) { return (a < b ? a : b); }

#if LINUX
static bool isnumber(char x){return x>='0' and x<='9';}
#endif
#define error(msg) error1(msg,__FILE__,__LINE__)
#define internal_error(msg) error1("internal error: " msg,__FILE__,__LINE__)
#define todo(msg) error1(str("TODO ") + msg,__FILE__,__LINE__)

//typedef long long int64;  = 0x7E, in enum
typedef unsigned long long uint64;

static bool I_know_what_I_am_doing = false;

#ifdef TRACE
static bool tracing = true;
#define trace print
#else // DONT TRACE
static bool tracing = false;
#define trace(x) if(tracing)print(x)
#endif

typedef unsigned char byte;
typedef const char *chars;
typedef byte *bytes;

// silent ++
#define check_is(α, β) if((α)!=(β)){printf("%s != %s :\n",#α,#β);print(α);printf(" != ");print(β);printf("\n%s:%d\n",__FILE__,__LINE__);exit(0);}
#define check_eq(α, β) if((α)!=(β)){printf("%s != %s :\n",#α,#β);printf("\n%s:%d\n",__FILE__,__LINE__);exit(0);}
#define check_eq_or(α, β, ɣ) if((α)!=(β)){printf("%s != %s : ",#α,#β);printf("\n%s:%d\n",__FILE__,__LINE__);ɣ;exit(0);}

//#define assert(test) if(!(test)){printf("\nNOT PASSING %s\n%s:%d\n",#test,__FILE__,__LINE__);exit(0);}
#define check_silent(test) if(!(test)){printf("\nNOT PASSING %s\n%s:%d\n",#test,__FILE__,__LINE__);exit(0);}

#define check(test) printf("CHECKING %s\n%s:%d\n",#test,__FILE__,__LINE__); \
  if(test){print("OK check passes: ");print(#test);}else{printf("\nNOT PASSING %s\n%s:%d\n",#test,__FILE__,__LINE__);exit(0);}

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
//inline long abs_l(long x) noexcept;
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

bool isSmartPointer(long long d);

//Node smartValue(long smartPointer);
Node *smartNode(smart_pointer_64 smartPointer64);

long file_last_modified(char *file);

String demangle(String &fun);

String extractPath(String file);

bool isDir(const char *name);

#define maxi(a, b) a<b?b:a
//#define max(a, b) a<b?b:a too many conflicts with math.h common.h …
static inline bool max(long a, long b) {
    return a < b ? b : a;
}

typedef String Path;

