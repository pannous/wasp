#include "String.h"

#define check(test) if(test){log("\nOK check passes: ");printf("%s\n",#test);}else{ \
printf("\nNOT PASSING: "); log(#test);printf("%s:%d\n",__FILE__,__LINE__); \
exit(1);}

bool fileExists(String filename);

String findFile(String filename);// empty() if not found
//char *findFile(char* filename);// 0 if not found