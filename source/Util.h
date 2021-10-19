#include "String.h"

//#define check_eq(α, β) if((α)!=(β)){if(debug_reader)printf("%s != %s : ",#α,#β);log(α); \
if(debug_reader)printf("!=");log(β);if(debug_reader)printf("\n%s:%d\n",__FILE__,__LINE__);exit(0);}

#define check(test) printf("CHECKING %s\n%s:%d\n",#test,__FILE__,__LINE__);\
if(test){log("OK check passes: ");log(#test);}else{printf("\nNOT PASSING %s\n%s:%d\n",#test,__FILE__,__LINE__);exit(0);}

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

double mod_d(double x, double y);

extern double pi;