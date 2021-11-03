#include "String.h"

//#define check_eq(α, β) if((α)!=(β)){if(debug_reader)printf("%s != %s : ",#α,#β);print(α); \
if(debug_reader)printf("!=");put(β);if(debug_reader)printf("\n%s:%d\n",__FILE__,__LINE__);exit(0);}

#define check(test) printf("CHECKING %s\n%s:%d\n",#test,__FILE__,__LINE__);\
if(test){print("OK check passes: ");print(#test);}else{printf("\nNOT PASSING %s\n%s:%d\n",#test,__FILE__,__LINE__);exit(0);}

bool fileExists(String filename);

String findFile(String filename);// empty() if not found
//char *findFile(char* filename);// 0 if not found

template<class S>
bool contains(List<S> list, S match);

template<class S>
// list HAS TO BE 0 terminated! Dangerous C!! ;)
bool contains(S list[], S match);

bool contains(chars list[], chars match);

unsigned int wordHash(const char *str, int max_chars = 20);

char *readFile(chars filename, int *size_out = 0);// 0 = err

bool similar(float a, float b);

void lowerCase(char *string, int i);

double mod_d(double x, double y);

extern double pi;

bytes concat(bytes a, bytes b, int len_a, int len_b);

bytes concat(bytes a, char b, int len);

bytes concat(char section, bytes a, int len_a);

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