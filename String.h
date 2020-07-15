//
// Created by me on 19.12.19.
//

//#ifndef MARK_STRING_H
//#define MARK_STRING_H
class String;
#define breakpoint_helper printf("\n%s:%d breakpoint_helper\n",__FILE__,__LINE__);
typedef const char *chars;

extern "C" void err(chars error);
extern "C" void error(chars error);
extern "C" void warn(chars error);
extern "C" void warning(chars error);
extern "C" chars fetch(chars url);
extern "C" void* alloc(long i);

#ifdef WASM
//void printf(const char *s);  //stdio
//void print(const char *format, int i);
void printf(char const *format, int i);
void printf(const char *format, chars i);
void printf(const char *format, chars i, int line);
void printf(const char *format, chars i, chars j);
void printf(const char *format, chars i, chars j, int l);
void printf(const char *format, chars i, chars j, chars k, int l);
#ifndef malloc
extern void* malloc(size_t __size);// __result_use_check __alloc_size(1);
#endif
#endif
//#endif //MARK_STRING_H
