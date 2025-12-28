#pragma once
// YOU NEED extern "C" to keep the name for UNIMPLEMENTED functions only

#include <cstddef> // size_t  FINE with wasm!
#include "smart_types.h"


typedef char32_t codepoint;
// 'letter' ☃ is a single code point but 3 UTF-8 code units (char's), and 1 UTF-16 code unit (char16_t)

typedef const char *chars;
typedef unsigned char *bytes;

#if not WEBAPP
#endif
[[noreturn]]
extern void error1(chars message, chars file, int line);



// there are two aspects of wasm memory: the internal memory starting at 0 and the external c-pointer *wasm_memory if the VM provides it
// worse there is the native_runtime which may hold the wasm_runtime running in the VM!
// todo : depends on clang_options! Sometimes it is needed sometimes it can't be there
extern "C" unsigned int *memory; // =0; always, BUT heap_offset/current is higher from beginning!
extern void *wasm_memory;
// this is the C POINTER to wasm_memory in the wasm VM! only available in the C runtime, not in wasm!

#ifndef MAX_MEM
#ifdef WASM
extern int MAX_MEM;
#else
#define MAX_MEM 0x2000000000000000L  // ~ (2**64)/10 // what for?
#endif
#endif

typedef unsigned char byte; //!
// __heap_base is provided by host and its ADDRESS &__heap_base is the start of the heap area.
extern byte __heap_base; // set via -Wl,--export=__heap_base
extern byte __data_end; // ⚠️ set by runtime once, Unused: using __heap_base instead as one should!
extern byte *__initial_heap_end; // safely(?) reset to after gc?
extern "C" byte *heap_end; // &__heap_base + heap_offset
extern "C" byte *getHeapEnd();

extern "C" void setHeapEnd(byte *neu);

extern "C" void panic();


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
extern "C" void *memcpy(void*, const void*, size_t) noexcept;
extern "C" void *memset(void *ptr, int value, size_t num) noexcept;
extern "C" void *memmove(void *__dst, const void *__src, size_t num) noexcept;
#elif EMSCRIPTEN
extern "C" void *memcpy(void *__restrict, const void *__restrict, size_t);
extern "C" void *memset(void *ptr, int value, size_t num);
extern "C" void *memmove(void *__dst, const void *__src, size_t num);
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
extern "C" void put_chars(chars c, size_t len = 0);

//extern "C" void put_chars(char* c, size_t len = 0);

class String;

extern "C" //  destroys the export type signature! but required by stdio.h:178:6:
int puts(const char *); // stdio
extern "C" String *put_string(String *);

extern "C" void *putp(void *f); // pointer
extern "C" int puti(int i);

extern "C" int64 putl(int64 l);

extern "C" int64 putx(int64 l);

extern "C" float putf(float f);

extern "C" double putd(double f);

//void putp(int64 *char_pointer);
extern "C" codepoint put_char(codepoint c);

//void put_char(char c);
//extern "C" int putchar(int c);// stdio

double powd(double x, double y);

double square_double(double n); // test wasm

extern double sqrt1(double a); // wasm has own, egal only used in Interpret.cpp


//extern "C" double pow2(double x, double y);
//extern "C"
//double pow(double x, double y);// todo: merge pow.wasm lib (7kb!!)
//#undef pow
//float pow(double x, double y);// todo: merge pow.wasm lib (7kb!!)

//bl	0x100003f6c ; symbol stub for: _pow

//extern float powf(float x, float y);
//void printf(int);

extern "C" char *run(chars code);

void *alloc(int num, int size); // => malloc / calloc
#if WASM
extern "C" void *malloc(size_t size);
//extern "C" void *malloc(size_t __size) __attribute__((__malloc__, __warn_unused_result__));
extern "C" void *aligned_alloc(size_t __alignment, size_t __size);// stdlib.h
#endif
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

void print(int l);

//void print(long l);
void print(int64 l);

//void print(Primitive l);

void print(double l); // _Z5printd

void print(size_t l);

//extern __inline int isalnum ( int c );
int isNumber(char c);
int isalnum0(int c);

extern "C" void printNode(smart_pointer_64 node);

#ifdef WASM

// int64 is 4 byte in Wasm/Windows WTH
//typedef int64 int64  // WTH
//#define int64 int64  // WTH

#ifndef WASI
#ifdef WASM
//void printf(chars no_format);
typedef unsigned int uint32_t;

#ifndef _STDIO_H
//    void printf(chars);
extern "C"
int printf(const char *__restrict, ...);
// printf in WASM messes up the stack, so we can't use it
#endif

void printf(chars, chars);

void printf(char const *format, int i);
void printf(char const *format, size_t i);

void printf(char const *format, uint32_t i);

void printf(char const *format, int64 l);

void printf(char const *format, int64 l);

void printf(char const *format, double d);

void printf(char const *format, chars, int);

void printf(chars format, int, int);

void printf(chars format, uint32_t, uint32_t);

void printf(chars format, int64 i, int64);

void printf(chars format, double i, double j);

void printf(chars format, chars i);

void printf(chars format, chars i, chars j);

void printf(chars format, chars i, chars j, int l);

void printf(chars format, chars i, chars j, chars l);

void printf(chars format, chars i, chars j, chars k, int l);

#endif
#endif

#endif

//#ifndef WASM
#ifdef WASI
extern "C" int printf(chars s, ...);  //stdio
#endif


struct c_io_vector {
    chars string;
    size_t length;
};

//#if WASI or MY_WASI

//#if WASMEDGE
// Fucking WasmEdge doesn't support wasi_unstable
#define WASI(import) __attribute__((import_module("wasi_snapshot_preview1"), import_name(#import))) extern
//#else
// Fucking wasmer doesn't support wasi_snapshot_preview1
// #define WASI(import) __attribute__((import_module("wasi_unstable"), import_name(#import))) extern "C"
//#endif
// #define WASI(import) __attribute__((import_module("wasi"), import_name(#import))) extern


//#if not WASM
//[[noreturn]]
//#endif
WASI(proc_exit)
void proc_exit(int exitcode);

WASI(fd_write)
int fd_write(int fd, c_io_vector *iovs, size_t iovs_count, size_t *nwritten);

//void fd_write_host(int FD, char **strp, int *ignore, int *ignore) compatible signature

/**
 * Return command-line argument data sizes.
 * Returns the number of arguments and the size of the argument string data, or an error.
 */
WASI(args_sizes_get) int args_sizes_get(int *argc, int *buf_len);

/**
 * Read command-line argument data.
 * The size of the array should match that returned by `args_sizes_get`.
 * Each argument is expected to be `\0` terminated.
*/
WASI(args_get) int args_get(char **argv, char *argv_buf);

template<class S>
class List;

List<String> arguments();

#if MY_WASM
extern "C" void registerWasmFunction(chars name, chars mangled);
extern "C" void registerWasmFunclet(chars name, bytes funclet, size_t size);// called by host
extern "C" bytes getWasmFunclet(chars name, size_t* size);// calling host
extern "C" chars download(chars name);// curl wget sync download via js / runtime!
#endif


#if WEBAPP
void console_log(const char *s); // print from wasp to browser console ( in addition to stdout )
#endif


int square(int n); // test wasm, needed for SquareFunc << get rid via linking!!
// todo: test polymorphism / multi dispatch
double square(double a);

// int64 square(int64 n); // test wasm, otherwise use x² => x*x in analyze!
// int64 squarel(int64 n); // test wasm, otherwise use x² => x*x in analyze!
