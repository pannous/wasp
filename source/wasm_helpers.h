#pragma once
//
// Created by pannous on 15.07.20.
//

#ifndef WASM

#include <cstddef> // size_t

#else
#define size_t unsigned long
//#define size_t unsigned int error: 'operator new' takes type size_t ('unsigned long') as first parameter
#endif

typedef const char *chars;
typedef unsigned char *bytes;


// todo : depends on clang_options! Sometimes it is needed sometimes it can't be there
#ifndef WASM
#endif
extern "C" unsigned int *memory;// =0; always, BUT heap_offset/current is higher from beginning!
extern "C" char *memoryChars;
//extern "C" int __heap_base; ==
//extern "C" int heap_offset; // via CMake  todo ?
//extern "C" int memory_size; // via CMake  todo ?
//#define memory_size 10485760

#ifndef MEMORY_SIZE
#ifdef WASM
#define MEMORY_SIZE 117964800 // todo: usually via CMAKE!?
#else
#define MEMORY_SIZE 1844674407370955200 //(2**64)/10
#endif
#define HEAP_OFFSET 0
#endif


extern "C" /*unsigned */ char *current;// memory + heap_offset
extern "C" void panic();//
#ifndef WASM_ENABLE_INTERP
extern "C"
#endif
int raise(chars error);// conflicts with signal.h
//extern unsigned int *memory;
//extern unsigned int *& __unused heap;

extern "C" void logs(chars);// ,int len=-1 /*auto*/);
extern "C" void logc(char c);
extern "C" void logx(int i);// hex
extern "C" void logi(int i);
extern "C" void logf32(float f);
extern "C" int square(int n);// test wasm
extern "C" int squaref(int n);// test wasm
extern int sqrt1(int a);

//void printf(int);
void *alloc(int size, int num);



//extern unsigned int memory_size;
//extern "C" int memory_size;// doesn't compile! :(
//int memory_size = 1024 * 10;


//TypeError: wasm function signature contains illegal type:
// ONLY INT and FLOAT in wasm functions!!!
// helpers calling

void warn(chars);

void error(chars);

//void log(char c);
#undef log
//extern "C" void log(chars s);
//extern "C" void log(char*);

//void    *alloc(size_t __size) __result_use_check __alloc_size(1);
//char* alloc(number l);
//void usleep(int l);
//extern "C" void print (chars );// no \n newline
//extern "C" void logs (chars );// can't work!
class Node;

class String;

void print(String);

void print(const Node);

void println(String);

void log(String *s);

//extern __inline int isalnum ( int c );
int isalnum0(int c);

#ifdef WASM
#ifndef WASI
void printf(chars, chars);
//void print(chars format, int i);
void printf(char const *format, int i);
void printf(char const *format, chars,int);
void printf(chars format, int,int);
void printf(chars format, long i, long);

void printf(chars format, double i, double j);

void printf(chars format, chars i);
void printf(chars format, chars i, chars j);
void printf(chars format, chars i, chars j, int l);
void printf(chars format, chars i, chars j, chars l);
void printf(chars format, chars i, chars j, chars k, int l);
#endif
extern "C" void * memset ( void * ptr, int value, size_t num );
//#ifndef WASI
// Provided by /opt/wasm/wasi-sdk/share/wasi-sysroot/include/__functions_malloc.h:15:7:
void *calloc(size_t __nmemb, size_t __size) __attribute__((__malloc__, __warn_unused_result__));
//void* calloc(int i,int num=1);// different from complicated alloc.h type!
//#endif
void free(void*);
//int rand();
//proc_exit, environ_get, environ_sizes_get

extern "C" void exit(int fd) __attribute__((__noreturn__, import_module("wasi_unstable"), import_name("proc_exit"))); // wasmtime ++
//extern "C" void exit(int fd) __attribute__((__noreturn__, import_module("wasi_snapshot_preview1"), import_name("proc_exit")));// wasmer WTF

extern "C" int raise(chars);
//extern "C" void exit(int code);
//extern "C" void exit(int fd) __attribute__((__noreturn__));

//void exit(int fd) __attribute__((import_module("wasi_snapshot_preview1"), import_name("exit")));

#endif

extern "C" int printf(chars s, ...);  //stdio
//extern "C" int printf(chars s, String c);  conflict


extern "C"
void *memcpy(void *destination, const void *source, size_t num);// asm ("memcpy");;
//__attribute__((import_module("env"), import_name("memcpy")));;
extern "C" void memcpy0(char *destination, char *source, size_t num);

void memcpy0(bytes dest, bytes source, int i);

extern "C" void *memmove(void *__dst, const void *__src, size_t num);



//extern "C" void _fd_write(int fd, const wasi_buffer *iovs, size_t iovs_len, size_t *nwritten);
extern "C"
#ifdef WASI
__attribute__((import_module("wasi_unstable"), import_name("fd_write")))
int fd_write(int fd, void *iovs, size_t iovs_len, size_t *nwritten);
//__attribute__((import_module("wasi_snapshot_preview1"), import_name("fd_write")));

#else
int fd_write(int fd, void *iovs, size_t iovs_len, size_t *nwritten);
#endif

//int isalnum(int c);

void trace(chars x);