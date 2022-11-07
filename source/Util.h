#ifndef _Util_h_
#define _Util_h_

extern bool tracing;

typedef unsigned char byte;
typedef const char *chars;
typedef byte *bytes;


enum smart_pointer_masks {
//	float_header_64 = 0x0020000000000000, not needed, use:
	double_mask_64 = 0x7F00000000000000L,
	smart_mask_64 = 0x00FF000000000000L,
//	negative_mask_64 = 0x8000000000000000,
	negative_mask_64 = 0xFF00000000000000L
//	negative_long_mask_64 = 0xBFF0000000000000,
};
//#include "Map.h" // for List signature circular?

#define internal_error(msg) error1("internal error: " msg,__FILE__,__LINE__)
#define error(msg) error1(msg,__FILE__,__LINE__)
#define todo(msg) error1(msg,__FILE__,__LINE__)
//
//void todo(chars error);

extern void error1(chars message, chars file = 0, int line = 0);

//#define check_eq(α, β) if((α)!=(β)){if(debug_reader)printf("%s != %s : ",#α,#β);print(α); if(debug_reader)printf("!=");put(β);if(debug_reader)printf("\n%s:%d\n",__FILE__,__LINE__);exit(0);}


//#define check(test) if(test){log("\nOK check passes: ");log(#test);}else{if(debug_reader)printf("\nNOT PASSING %s\n%s:%d\n",#test,__FILE__,__LINE__);exit(0);}
//#define check_eq(α, β) if((α)!=(β)){if(debug_reader)printf("%s != %s : ",#α,#β);log(α);if(debug_reader)printf("!=");log(β);if(debug_reader)printf("\n%s:%d\n",__FILE__,__LINE__);exit(0);}


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

String findFile(String filename);// empty() if not found
//char *findFile(char* filename);// 0 if not found
String load(String file);

String &hex(long d);

bool isSmartPointer(long long d);

//Node smartValue(long smartPointer);
Node smartNode(long long smartPointer64);

//String smartValue2(long smartPointer);

long file_last_modified(char *file);

