#pragma once
//
// Created by pannous on 15.07.20.
//
typedef const char *chars;
extern "C" unsigned int *memory;
extern "C" char *memoryChars;
extern "C" unsigned int *current;
//extern unsigned int *memory;
//extern unsigned int *& __unused heap;


extern "C" void logs (const char *,int len=-1 /*auto*/);
extern "C" void logc(char s);
extern "C" void logi(int i);
extern "C" void log_f32(float f);
extern "C" int square(int n);// test wasm
extern "C" int squaref(int n);// test wasm
extern int sqrt1(int a);
void printf(int);
#ifndef WASM
//#include <cstdlib>
//#include "stdlib.h"
void* alloc(int size,int num);
void raise(chars error);
#endif

#ifdef WASM
//extern unsigned int memory_size;
//extern "C" int memory_size;// doesn't compile! :(
//int memory_size = 1024 * 10;

#define size_t unsigned long
//#define size_t int

//TypeError: wasm function signature contains illegal type:
// ONLY INT and FLOAT in wasm functions!!!
extern "C" void exit(int code=0);
extern "C" void logc(char s);
extern "C" void logi(int i);
// helpers calling

void warn(char const*);
void error(char const*);
//void raise(char const*);
extern "C" void raise(char const*);// throws in js, which yields backtrace yay

void log(char*);
void log(char c);
void log(chars s);

//void    *alloc(size_t __size) __result_use_check __alloc_size(1);
//char* alloc(number l);
//void usleep(int l);
//extern "C" void print (const char *);// no \n newline
//extern "C" void logs (const char *);// can't work!
class Node;
class String;
void print(String);
void println(String);
void log(String *s);
int isalnum ( int c );

void printf(const char *s);  //stdio
void printf(char const*, char const*);
//void print(const char *format, int i);
void printf(char const *format, int i);
void printf(char const *format, char const*,int);
void printf(const char *format, chars i);
void printf(const char *format, chars i, chars j);
void printf(const char *format, chars i, chars j, int l);
void printf(const char *format, chars i, chars j, chars k, int l);
void * memcpy ( void * destination, const void * source, size_t num );
void * memset ( void * ptr, int value, size_t num );
//extern void* malloc(size_t __size);// __result_use_check __alloc_size(1);
//extern
//void* alloc(size_t __size);
//extern
void* alloc(int i, int num);
#ifdef WASM
void* calloc(int i,int num);// different from complicated alloc.h type!
#endif

#endif