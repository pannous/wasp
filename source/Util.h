#include "String.h"

#define check(test) if(test){log("\nOK check passes: ");printf("%s\n",#test);}else{ \
printf("\nNOT PASSING: "); log(#test);printf("%s:%d\n",__FILE__,__LINE__); \
exit(1);}

bool fileExists(String filename);

String findFile(String filename);// empty() if not found
//char *findFile(char* filename);// 0 if not found

template<class S>
bool contains(List<S> list, S match);

template<class S>
// list HAS TO BE 0 terminated! Dangerous C!! ;)
bool contains(S list[], S match);

unsigned int wordHash(const char *str, int max_chars = 20);

char *readFile(chars filename);// 0 = err
