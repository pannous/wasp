#pragma once
//
// Created by pannous on 15.07.20.
//

// YOU NEED extern "C" to keep the name for UNIMPLEMENTED functions only

#include <cstddef> // size_t  FINE with wasm!

typedef char32_t codepoint;// 'letter' ☃ is a single code point but 3 UTF-8 code units (char's), and 1 UTF-16 code unit (char16_t)

typedef const char *chars;
typedef unsigned char *bytes;

[[noreturn]]
extern void error1(chars message, chars file, int line);


// there are two aspects of wasm memory: the internal memory starting at 0 and the external c-pointer *wasm_memory if the VM provides it
// worse there is the native_runtime which may hold the wasm_runtime running in the VM!
// todo : depends on clang_options! Sometimes it is needed sometimes it can't be there
extern "C" unsigned int *memory;// =0; always, BUT heap_offset/current is higher from beginning!
extern void *wasm_memory;// this is the C POINTER to wasm_memory in the wasm VM! only available in the C runtime, not in wasm!
//extern "C" char *wasm_memory_chars;// this is the C POINTER to wasm_memory in the wasm VM! only available in the C runtime, not in wasm!
//extern "C" char *memoryChars;
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

#define MEMORY_SIZE 0x2000000000000000  // ~ (2**64)/10 // what for?
//#define MEMORY_SIZE 0x2000000000000
//#define MEMORY_SIZE 0x20000000000  // not enough!
//#define MEMORY_SIZE 0x200000000  // not enough!

#endif
#define HEAP_OFFSET 0x80000
#endif

extern unsigned char __heap_base;// set via lld with -Clink-arg=--export=__heap_base
extern "C" /*unsigned */ char *current;// memory + heap_offset
extern "C" void panic();//


#ifndef WASM

int raise(chars error); // conflicts with signal.h if 'extern'
#else
extern "C" int raise(chars error); // conflicts with signal.h if 'extern'
#endif
//
//#if WASM
//extern "C" void *memmove(char *dest, const char *source, size_t num);
//#endif
//void *memcpy(void *destination, const void *source, size_t num);// asm ("memcpy");;
#if LINUX
extern "C" void* memcpy(void*, const void*, size_t) noexcept;
extern "C" void *memset(void *ptr, int value, size_t num) noexcept;
extern "C" void *memmove(void *__dst, const void *__src, size_t num) noexcept;
//#else
#elif WASM
extern "C" void *memset(void *__dst, int __c, size_t __n) __attribute__((__nothrow__, __leaf__, __nonnull__(1)));
extern "C" void *memcpy(void *__restrict__ __dst, const void *__restrict__ __src, size_t __n) __attribute__((__nothrow__, __leaf__, __nonnull__(1, 2)));
extern "C" void *memmove(void *__dst, const void *__src, size_t __n) __attribute__((__nothrow__, __leaf__, __nonnull__(1, 2)));
#else
extern "C" void *memcpy(void *, const void *, size_t);
extern "C" void *memset(void *ptr, int value, size_t num);
extern "C" void *memmove(void *__dst, const void *__src, size_t num);
#endif



//__attribute__((import_module("env"), import_name("memcpy")));;
extern "C" void memcpy0(char *destination, char *source, size_t num);

void memcpy1(bytes dest, bytes source, int i);


// if MY_WASI make sure to IMPLEMENT THEM ALL via fd_write !!
// todo: alias all to print
void put_chars(char *c, size_t len = 0);

class String;

extern "C" //  destroys the export type signature! but required by stdio.h:178:6:
int puts(const char *);// stdio
int put_s(String *);// stdio
void putp(void *f);// pointer
void puti(int i);

void putl(long long l);

void putx(long long l);

//void putp(long *char_pointer);
void put_char(codepoint c);
//void put_char(char c);
extern "C" int putchar(int c);// stdio

void putf(float f);

void putd(double f);

int square(int n);// test wasm

double powd(double x, double y);

long squarel(long n);// test wasm, otherwise use x² => x*x in analyze!
double square_double(double n);// test wasm

extern double sqrt1(double a);// wasm has own, egal only used in Interpret.cpp


//extern "C" double pow2(double x, double y);
//extern "C"
//double pow(double x, double y);// todo: merge pow.wasm lib (7kb!!)
//#undef pow
//float pow(double x, double y);// todo: merge pow.wasm lib (7kb!!)

//bl	0x100003f6c ; symbol stub for: _pow

//extern float powf(float x, float y);
//void printf(int);

void *alloc(int num, int size);// => malloc / calloc
//void *calloc(int size, int num);// alloc cleared
//void *calloc(size_t __count, size_t __size);// __result_use_check __alloc_size(1,2);
//inline _LIBCPP_INLINE_VISIBILITY float       pow(float __lcpp_x, float __lcpp_y) _NOEXCEPT;
// Provided by /opt/wasm/wasi-sdk/share/wasi-sysroot/include/__functions_malloc.h:15:7:
// Provided by /opt/wasm/wasi-sdk/share/wasi-sysroot//include/stdlib.h
#ifdef PURE_WASM
//void *calloc(size_t __nmemb, size_t __size) __attribute__((__malloc__, __warn_unused_result__));
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


#ifdef WASM

// long is 4 byte in Wasm/Windows WTH
//typedef long long long  // WTH
//#define long long long  // WTH

#ifndef WASI
#ifdef WASM
//void printf(chars no_format);
typedef unsigned int uint32_t;

#ifndef _STDIO_H
//    void printf(chars);
extern "C"
int printf(const char *__restrict, ...);
#endif

void printf(chars, chars);

void printf(char const *format, int i);

void printf(char const *format, uint32_t i);

void printf(char const *format, long l);

void printf(char const *format, long long l);

void printf(char const *format, double d);

void printf(char const *format, chars, int);

void printf(chars format, int, int);

void printf(chars format, uint32_t, uint32_t);

void printf(chars format, long i, long);

void printf(chars format, double i, double j);

void printf(chars format, chars i);

void printf(chars format, chars i, chars j);

void printf(chars format, chars i, chars j, int l);

void printf(chars format, chars i, chars j, chars l);

void printf(chars format, chars i, chars j, chars k, int l);

#endif
#endif

//extern "C" void * memset ( void * ptr, int value, size_t num ); not extern!
//#ifndef WASI
//#endif
//int rand();
//proc_exit, environ_get, environ_sizes_get

#ifndef WASI
extern "C" void exit(int fd) __attribute__((__noreturn__, import_module("wasi_unstable"), import_name("proc_exit"))); // wasmtime ++
#endif
//extern "C" void exit(int fd) __attribute__((__noreturn__, import_module("wasi_snapshot_preview1"), import_name("proc_exit")));// wasmer WTF

//extern "C" void exit(int code);
//extern "C" void exit(int fd) __attribute__((__noreturn__));

//void exit(int fd) __attribute__((import_module("wasi_snapshot_preview1"), import_name("exit")));

#endif

//#ifndef WASM
#ifdef WASI
extern "C" int printf(chars s, ...);  //stdio
#endif
//extern "C" int printf(chars s, String c);  conflict






// most important WASI function
//extern "C" void _fd_write(int fd, const wasi_buffer *iovs, size_t iovs_len, size_t *nwritten);
//void fd_write(int FD, char **strp, int *len, int *nwritten); // todo len or len* ?  wasi_buffer={char*, … ?}

struct c_io_vector {
    char *string;
    size_t length;
};

//#if WASI or MY_WASI
//__attribute__((import_module("wasi_snapshot_preview1"), import_name("fd_write")))
__attribute__((import_module("wasi_unstable"), import_name("fd_write")))
extern "C" int fd_write(int fd, c_io_vector *iovs, size_t iovs_count, size_t *nwritten);
//void fd_write_host(int FD, char **strp, int *ignore, int *ignore) compatible signature

[[noreturn]]
__attribute__((import_module("wasi_unstable"), import_name("proc_exit")))
extern "C" void proc_exit(int exitcode);


__attribute__((import_module("wasi_unstable"), import_name("args_sizes_get")))
extern "C" int args_sizes_get(char **argv, int *argc);

static size_t argc() {
    char *argv = (char *) alloc(1000, 1);
    int argc;
    args_sizes_get(&argv, &argc);
    return argc;
}
