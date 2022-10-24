#pragma once
//
// Created by pannous on 15.07.20.
//

#include <cstddef> // size_t  FINE with wasm!

typedef const char *chars;
typedef unsigned char *bytes;

extern void error1(chars message, chars file, int line);

#ifndef WASM
#else
//#ifdef PURE_WASM << unnecessary
//#define size_t unsigned long
//#define size_t unsigned int // error: 'operator new' takes type size_t ('unsigned long') as first parameter
#endif


// there are two aspects of wasm memory: the internal memory starting at 0 and the external c-pointer *wasm_memory if the VM provides it
// worse there is the native_runtime which may hold the wasm_runtime running in the VM!

extern "C" void *wasm_memory;// this is the C POINTER to wasm_memory in the wasm VM! only available in the C runtime, not in wasm!
//extern "C" char *wasm_memory_chars;// this is the C POINTER to wasm_memory in the wasm VM! only available in the C runtime, not in wasm!

// TODO: let the native_runtime mess with wasm_memory DIRECTLY?

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
#define WASM_MEMORY_SIZE 0xF0000000
#else
//#define WASM_MEMORY_SIZE 0x20000000000

//#define MEMORY_SIZE 0x2000000000000000  // ~ (2**64)/10 // what for?
#define MEMORY_SIZE 0x2000000000000
//#define MEMORY_SIZE 0x20000000000  // not enough!
//#define MEMORY_SIZE 0x200000000  // not enough!

#endif
#define HEAP_OFFSET 0
#endif


extern "C" /*unsigned */ char *current;// memory + heap_offset
extern "C" void panic();//
#ifndef WASM_ENABLE_INTERP
#ifndef SDL
//extern "C" // conflicts with signal.h
#endif
#endif

#ifndef WASM

int raise(chars error); // conflicts with signal.h if 'extern'
#else
extern int raise(chars error); // conflicts with signal.h if 'extern'
#endif

int squari(int a);// stupid test remove
//extern unsigned int *memory;
//extern unsigned int *& __unused heap;

typedef char32_t codepoint;// ☃ is a single code point but 3 UTF-8 code units (char's), and 1 UTF-16 code unit (char16_t)

extern "C" int puts(const char *);// stdio
extern "C" void putx(int i);// hex
extern "C" void putp(void *f);// pointer
extern "C" void puti(int i);
//extern "C" void putp(long *char_pointer);
extern "C" void put_char(codepoint c);
//extern "C" void put_char(char c);
//extern "C" int putchar(int c);// stdio

extern "C" void putf(float f);

double powd(double x, double y);

extern "C" long squarel(long n);// test wasm, otherwise use x² => x*x in analyze!
extern "C" double square(double n);// test wasm

extern int sqrt1(int a);

long powi(int a, int b);// short harder

//extern "C" double pow2(double x, double y);
//extern "C"
//double pow(double x, double y);// todo: merge pow.wasm lib (7kb!!)
//#undef pow
//float pow(double x, double y);// todo: merge pow.wasm lib (7kb!!)

//bl	0x100003f6c ; symbol stub for: _pow

//extern float powf(float x, float y);
//void printf(int);
void *alloc(int size, int num);
//void *calloc(int size, int num);// alloc cleared
//void *calloc(size_t __count, size_t __size);// __result_use_check __alloc_size(1,2);
//inline _LIBCPP_INLINE_VISIBILITY float       pow(float __lcpp_x, float __lcpp_y) _NOEXCEPT;
// Provided by /opt/wasm/wasi-sdk/share/wasi-sysroot/include/__functions_malloc.h:15:7:
// Provided by /opt/wasm/wasi-sdk/share/wasi-sysroot//include/stdlib.h
#ifdef PURE_WASM
void *calloc(size_t __nmemb, size_t __size) __attribute__((__malloc__, __warn_unused_result__));
//void* calloc(int i,int num=1);// different from complicated alloc.h type!
#endif

//extern unsigned int memory_size;
//extern "C" int memory_size;// doesn't compile! :(
//int memory_size = 1024 * 10;


//TypeError: wasm function signature contains illegal type:
// ONLY INT and FLOAT in wasm functions!!!
// helpers calling

void warn(chars);

//extern void error1(chars message, chars file = 0, int line = 0);

//void error(chars); macro

class Node;

class String;

void print(String);

void print(String *s);

void print(long i);

//extern __inline int isalnum ( int c );
int isalnum0(int c);

extern "C" void *memset(void *ptr, int value, size_t num);

#ifdef WASM

// long is 4 byte in Wasm/Windows WTH
//typedef long long long  // WTH
//#define long long long  // WTH

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

//extern "C" void * memset ( void * ptr, int value, size_t num ); not extern!
//#ifndef WASI
//#endif
//int rand();
//proc_exit, environ_get, environ_sizes_get

extern "C" void exit(int fd) __attribute__((__noreturn__, import_module("wasi_unstable"), import_name("proc_exit"))); // wasmtime ++
//extern "C" void exit(int fd) __attribute__((__noreturn__, import_module("wasi_snapshot_preview1"), import_name("proc_exit")));// wasmer WTF

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

void trace(chars x);