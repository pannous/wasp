#ifndef _Util_h_
#define _Util_h_

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

//#include "Map.h" // for List signature circular?

#define internal_error(msg) error1("internal error: " msg,__FILE__,__LINE__)
#define error(msg) error1(msg,__FILE__,__LINE__)
#define todo(msg) error1(str("TODO ") + msg,__FILE__,__LINE__)
//
//void todo(chars error);

[[noreturn]]
extern void error1(chars message, chars file = 0, int line = 0);

//#define check_eq(α, β) if((α)!=(β)){if(debug_reader)printf("%s != %s : ",#α,#β);print(α); if(debug_reader)printf("!=");put(β);if(debug_reader)printf("\n%s:%d\n",__FILE__,__LINE__);exit(0);}


//#define check(test) if(test){log("\nOK check passes: ");log(#test);}else{if(debug_reader)printf("\nNOT PASSING %s\n%s:%d\n",#test,__FILE__,__LINE__);exit(0);}
//if(debug_reader)
//if(debug_reader)
#define check_eq(α, β) if((α)!=(β)){printf("%s != %s : ",#α,#β);print(α);printf("!=");print(β);printf("\n%s:%d\n",__FILE__,__LINE__);exit(0);}

//#define check_eq assert_equals // no


#define assert(test) if(!(test)){printf("\nNOT PASSING %s\n%s:%d\n",#test,__FILE__,__LINE__);exit(0);}


#define check(test) printf("CHECKING %s\n%s:%d\n",#test,__FILE__,__LINE__); \
  if(test){print("OK check passes: ");print(#test);}else{printf("\nNOT PASSING %s\n%s:%d\n",#test,__FILE__,__LINE__);exit(0);}


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
#endif


#include "String.h"

void warn(String warning);

bool fileExists(String filename);

String findFile(String filename, String current_dir);// empty() if not found
//char *findFile(char* filename);// 0 if not found
String load(String file);

String &hex(long d);

bool isSmartPointer(long long d);

//Node smartValue(long smartPointer);
Node smartNode(long long smartPointer64);

//String smartValue2(long smartPointer);

long file_last_modified(char *file);

String demangle(String &fun);

String extractPath(String file);

bool isDir(const char *name);
