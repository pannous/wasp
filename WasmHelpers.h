//
// Created by me on 15.07.20.
//
typedef const char *chars;

#ifndef WASP_WASMHELPERS_H
#define WASP_WASMHELPERS_H

#endif //WASP_WASMHELPERS_H

#ifdef WASM
#define size_t unsigned long
//#define size_t int

//TypeError: wasm function signature contains illegal type:
// ONLY INT and FLOAT in wasm functions!!!
extern "C" void exit(int);
extern "C" void logc(char s);
extern "C" void logi(int i);
// helpers calling

void err(char const*);
void warn(char const*);
void log(char*);
void log(chars s);
//extern "C" void print (const char *);// no \n newline
//extern "C" void logs (const char *);// can't work!
void printf(const char *s);  //stdio
void print(const char *format, int i);
void printf(char const*, char const*);
void printf(char const *format, int i);
void printf(const char *format, chars i);
void printf(const char *format, chars i, int line);
void printf(const char *format, chars i, chars j);
void printf(const char *format, chars i, chars j, int l);
void printf(const char *format, chars i, chars j, chars k, int l);
#ifndef _MALLOC_UNDERSCORE_MALLOC_H_
//extern void* malloc(size_t __size);// __result_use_check __alloc_size(1);
//extern
//void* alloc(size_t __size);
//extern
void* alloc(int i);
#endif
#endif
//#endif //MARK_STRING_H
