//#pragma once
//
// Created by pannous on 15.07.20.
//

// ALL FUNCTIONS IN THIS FILE WILL BE COMPILED IN WASM CONTEXT: the wasp.wasm runtime.

// ASSUME everything is MY_WASI from now on!

#include "wasm_helpers.h"
#include "String.h"
#include "Backtrace.h"
#include "Code.h"

#include <typeinfo>       // operator typeid
#include <cstdlib> // OK in WASM!

int MAX_MEM = 65536 * 1024;// todo lol  INSIDE wasm memory!

__attribute__ ((visibility("default")))
void *get_heap_base(void) {
    return &__heap_base;
}

char *current = (char *) get_heap_base() + 80000;

extern "C" void *memmove(void *dest, const void *source, size_t num) {
    while (num < MAX_MEM and --num >= 0)
        ((char *) dest)[num] = ((char *) source)[num];
    return dest;
// memmove will never return anything other than dest.  It's useful for chaining
}

void free(void *) {/*lol*/}

_LIBCPP_OVERRIDABLE_FUNC_VIS void operator delete(void *) _NOEXCEPT {}

long powl(long a, int b) {
    long res = 0;
    int n = 1;
    do {
        if (b & 1) res += a * n;
        n *= 2;
        b = b >> 1;
    } while (b);
    return res;
}

inline long long doubleToLongBits(double a) {
    return *((long *) &a);
}

inline double longBitsToDouble(long long a) {
    return *((double *) &a);
}

double powd(double a, double b) {
    int x = (int) (doubleToLongBits(a) >> 32);
    int y = (int) (b * (x - 1072632447) + 1072632447);
    return longBitsToDouble(((long long) y) << 32);
}

double pow(double a, double b) {
    return powd(a, b);
}


#ifndef WASI

void *malloc(size_t size) {//}  __result_use_check __alloc_size(1){ // heap
    if (size > 1000000) {
        warn("malloc of huge memory chunk:");
        printf("%x", size);
        error("implausible size ;)");
    }
//        size = 10000;
    if ((int) current < 100000) {
//        puti((int) current);
//        print("get_heap_base:");
        current = (char *) get_heap_base();
//        puti((int) current);
    }
    void *last = current;
    current += size;
//	if(size>1000)
    bool check_overflow = false;// wasm trap: out of bounds memory access OK
    if (check_overflow and MEMORY_SIZE and (long) current >= MEMORY_SIZE) {
#ifndef WASI
        error("TOO BIG LOL");
        puti(sizeof(Node));// 64
        puti(sizeof(String));// 20
        puti(sizeof(Value));// 8 long
        puti((int) last);
        puti((int) memory);
        puti((int) current);
        puti((int) HEAP_OFFSET);
        puti(MEMORY_SIZE);
//		error("OUT OF MEMORY");// needs malloc :(
#endif
        panic();
        last = current = (char *) (4 * HEAP_OFFSET);// reset HACK todo!
    }
    return last;
}

#endif

int printf(const char *__restrict format, ...) {
//    todo better
    put_chars((char *) format, strlen(format));
    return 1;
}


void printf(chars format, uint32_t i) {
    print(String(format) % (int) i);
}


void printf(chars format) {
    print(format);
}

void printf(chars format, int i) {
    print(String(format) % i);
}

void printf(char const *format, size_t i) {
    print(String(format) % i);
}

void printf(char const *format, long long l) {
    print(String(format) % l);
}


void printf(chars format, chars value) {
    print(String(format).replace("%s", value));
}

void printf(chars format, chars i, chars j) {
    print(String(format).replace("%s", i).replace("%s", j));
}

void printf(chars format, chars i, chars j, int l) {
    print(String(format).replace("%s", i).replace("%s", j).replace("%d", String(l)));
}

void printf(chars format, chars i, chars j, chars l) {
    print(String(format).replace("%s", i).replace("%s", j).replace("%d", l));
}

void printf(chars format, long i, long j) {
    if (contains(format, "%ld"))
        print(String(format).replace("%ld", String(i)).replace("%ld", String(j)));
    else
        print(String(format).replace("%d", String(i)).replace("%d", String(j)));
}

void printf(chars format, int i, int j) {
    print(String(format).replace("%d", String(i)).replace("%d", String(j)));
}

void printf(chars format, uint32_t i, uint32_t j) {
    print(String(format).replace("%d", String((int) i)).replace("%d", String((int) j)));
}


void printf(chars format, double d) {
    print(String(format) % d);
}

void printf(chars format, long l) {
    print(String(format) % l);
}

void printf(chars format, double i, double j) {
    print(String(format).replace("%f", String(i)).replace("%f", String(j)));
}

void printf(chars format, chars val, int value) {
    print(String(format).format((char *) val).format(value));
}

void printf(chars format, void *value) {
    print(String(format).replace("%p", String((long) value)));
}

//#if MY_WASI

extern "C" void *__cxa_allocate_exception(size_t thrown_size) { return 0; }
extern "C" void __cxa_throw(
        void *thrown_exception,
        struct type_info *tinfo,
        void (*dest)(void *)) {}
extern "C" int __cxa_atexit(int, int, int) {
    proc_exit(0);
    return 0;
}

#if not MY_WASM
extern "C" long run_wasm(bytes buffer, int buf_size) {
    print("⚠️ run_wasm not available. wasp built without runtime and not embedded in a host which exposes the following function:");
    print("extern long run_wasm(uint8* buffer, size_t buffer_length)");
    print("Please vote here to make this a WASI standard: https://github.com/WebAssembly/WASI/issues/477");
    error("⚠️ run_wasm not available. You can use wasp-full.wasm which comes with wasm-runtime builtin.");
//    breakpoint_helper
    return 0;
}
#endif


void *alloc(int num, int size) {
    return calloc(num, size);
}


#ifndef WASI

// unmapped import calloc : header provided by stdlib.h but we need our own implementation!
//void *calloc(int size, int num) {// clean ('0') alloc
///opt/wasm/wasi-sdk/share/wasi-sysroot/include/stdlib.h
// /opt/wasm/wasi-sdk/share/wasi-sysroot/include/__functions_malloc.h redundant!
static char count[] = "0";
int calloc_counter = 0;

void debugCalloc(size_t num, size_t size) {
    calloc_counter++;
    if (calloc_counter == 1000 or num * size > 100000) {
        print("calloc");
        puti(num);
        print("*");
        puti(size);
        print("---calloc");
    }

}

void *calloc(size_t num, size_t size) //__attribute__((__malloc__, __warn_unused_result__))
{
    debugCalloc(num, size);
    char *mem = (char *) malloc(size * num);
//#ifndef WASM
    //fails in WASI, why??
    while (num < MAX_MEM and size / 8 * num > 0) { ((long *) mem)[--num * size / 8] = 0; }
//#endif
    return mem;
}

extern "C" void *memset(void *ptr, int value, size_t num) {
    // todo very expensive
    int *p = (int *) ptr;
    while (num-- > 0)*p++ = value;
    return ptr;//?
}

#endif
// WOW CALLED INTERNALLY FROM C!!
//extern "C"


// new operator for ALL objects
void *operator new[](size_t size) { // stack
    char *use = current;
    current += size;
    return use;
}

// new operator for ALL objects
void *operator new(size_t size) { // stack
    char *use = current;
    current += size;
    return use;
}

void _cxa_allocate_exception() {
    print("_cxa_allocate_exception!");
}

void _cxa_throw() {
    print("_cxa_throw");
    error("OUT OF MEMORY");
}


void printf(long l) {
    printf("%ld", l);
}


String Backtrace(int skip, int skipEnd) {
    return "Backtrace: TODO";
}


void memcpy1(bytes dest, bytes source, int i) {
    while (i < MAX_MEM and --i >= 0)
        dest[i] = source[i];
}

void memcpy0(char *destination, char *source, size_t num) {
    if ((long) destination + num >= MEMORY_SIZE)return;
//		panic();
    if ((long) source + num >= MEMORY_SIZE)return;
//		panic();
    while (--num < MAX_MEM) {
        destination[num] = source[num];
    }
}
//void * memcpy (void * destination, const void * source, size_t num ){
//	memcpy0((char *) destination, (char *) source, num);
//}


#ifndef WASI
extern "C"
void *memcpy(void *destination, const void *source, size_t num) {
    memcpy0((char *) destination, (char *) source, num);
    return destination;// yes?
}
#endif

typedef struct wasi_buffer {
    const void *buf;
    size_t buf_len;
} wasi_buffer;


#if WASI and not MY_WASM
extern "C" int raise(chars error){
    printf("%s" , error);
    return -1;
}
#endif

//#include <stdlib.h>
void exit0() {
//	exit(0);
}

//wasm-ld: error: duplicate symbol: exit
//extern "C" void exit(int fd){
// todo HOW??
// Error while importing "wasi_snapshot_preview1"."proc_exit": unknown import.
//}

#ifndef MY_WASM

void panic() {
#ifndef WASM
    raise("panic");
#else
    char *x = 0;
    x[-1] = 2;// Bus error: 10
#endif
}


void put_chars(char *c, size_t len) {
    c_io_vector civ{.string=c, .length=len ? len : strlen(c)};
    fd_write(1, &civ, 1, 0);
}

extern "C" // destroys the export type signature! stdio.h:178:6
int puts(chars c) { // // int return needed for stdio compatibilty !
    c_io_vector civ{.string=(char *) c, .length=(size_t) strlen(c)};
    fd_write(1, &civ, 1, 0);
    return (int) c;// stdio
}

int put_s(String *s) {
    fd_write(1, (c_io_vector *) s, 1, 0);
    return (int) s;// stdio
}

void puti(int i) {
    put_chars(formatLong(i), 0);
    newline();
//    printf("%d", i);
}

void putl(long long l) {
    printf("%lld", l);
}

[[maybe_unused]] void putx(long long l) {
    printf("%llx", l);
}

void putp(long char_pointer) {// workaround for m3, which can't link pointers:  od.link_optional<puts>("*", "puts")
    printf("%s", (char *) char_pointer);
}

void put_char(codepoint c) {
    printf("%c", c);
}

void putf(float f) {
    puts(formatLong((long) f));
    puts(".");
    puts(formatLong(((long) (f * 1000)) % 1000));
//    printf("%f\n", f);
}

void putd(double f) {
    printf("%f\n", f);
}

void putp(void *f) {
    printf("%p\n", f);
}

#endif



List<String> arguments() {
    List<String> args;
    int argc;
    int len;
    args_sizes_get(&argc, &len);
//    print("argc");
//    print(argc);
    char **argv = (char **) alloc(argc, 8);
    char *values = (char *) alloc(len, 1);
    args_get(argv, values);
    // is argv guaranteed to point into values?
    if (argv[0] != values)
            todo("wasmtime args_get is currently broken. Please use wasmer. ")
        for (int i = 0; i < argc; i++) {
            auto string = argv[i];
            args.add(String(string));
        }
    args.capacity = (int) (long) argv;// hack ;)
    return args;
}

//extern "C"
int main(int argc, char **argv);

#ifdef RUNTIME_ONLY
extern "C" long main(); // error: 'main' must return 'int'
//extern "C" long wasp_main();
#endif

extern "C" void _start() {
    auto args = arguments();
    for (auto arg: args)
        put(arg);
#if RUNTIME_ONLY
    smart_pointer_64 result = main();
#else
    smart_pointer_32 result = main(args.size(), (char **) args.capacity /*hack ;)*/);
#endif
    print(new Node(result));
//#if RUNTIME_ONLY
//    print("Wasp runtime not meant to be executed. Use wasp or wasp.wasm \n");
//#endif
}

extern "C" int putchar(int c) {// stdio
    put_char(c);
    return c;
}


extern "C"
size_t strlen(const char *x) {
#if not WASM
    if (!x)return 0;
#endif
    int l = 0;
    if ((long long) x >= MEMORY_SIZE || ((long long) x) == 0x200000000LL) {
        puts(x);
//        puti((int) (long) x);// 0x1000000 16777216
        error("corrupt string");
    }
    if ((long long) x == 0x1ffffffffLL || (long long) x >= 0xffffffff00000000LL ||
        ((long long) x >= 0x100000000LL and (long long) x <= 0x100100000LL))
        return false;// todo: valgrind debug corruption, usually because of not enough memory
//#if !WASM
//    return strlen(x);
//#endif
    while (l < MAX_STRING_LENGTH and (long long) x < MEMORY_SIZE - 1 and *x++)
        l++;
    return l;
}