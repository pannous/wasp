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
#include "Angle.h"
#include <typeinfo>       // operator typeid
#include <cstdlib> // OK in WASM!

extern int MAX_MEM;

extern "C" byte *getHeapEnd() { return heap_end; }

extern "C" void setHeapEnd(byte *neu) {
    // check_silent(neu >= heap_end); // don't allow overwrite! except in reset_heap()
    heap_end = neu;
}

// _LIBCPP_OVERRIDABLE_FUNC_VIS void operator delete(void* __p, std::size_t __sz) _NOEXCEPT;
// void operator delete(void*, unsigned long, std::align_val_t){}
_LIBCPP_OVERRIDABLE_FUNC_VIS void operator delete(void* __p, std::size_t __sz, std::align_val_t) _NOEXCEPT{}

// void operator delete(void*, std::align_val_t){
_LIBCPP_OVERRIDABLE_FUNC_VIS void operator delete(void *__p, std::align_val_t) _NOEXCEPT {
    /*lol*/
}

void free(void *) {
    /*lol*/
}

_LIBCPP_OVERRIDABLE_FUNC_VIS void operator delete(void *) _NOEXCEPT {
}


inline int64 doubleToLongBits(double a) {
    return *((int64 *) &a);
}

inline double longBitsToDouble(int64 a) {
    return *((double *) &a);
}

// double powd(double a, double b) { // todo
//     if(b==0)return 1;
//     trace("VERY crude first approximation of power! 1/4 ≈ 0.22 …!");
//     int x = (int) (doubleToLongBits(a) >> 32);
//     int y = (int) (b * (x - 1072632447) + 1072632447);
//     return longBitsToDouble(((int64) y) << 32);
// }

#include <stdint.h> // modf

double modf(double value, double *iptr) {
    union {
        double d;
        uint64_t i;
    } u = {value};
    int exponent = ((u.i >> 52) & 0x7FF) - 1023;

    if (exponent < 0) {
        *iptr = (value >= 0.0) ? 0.0 : -0.0;
        return value;
    }

    if (exponent >= 52) {
        *iptr = value;
        return (value >= 0.0) ? 0.0 : -0.0;
    }

    uint64_t mask = (1ULL << (52 - exponent)) - 1;
    if ((u.i & mask) == 0) {
        *iptr = value;
        return (value >= 0.0) ? 0.0 : -0.0;
    }

    u.i &= ~mask;
    *iptr = u.d;
    return value - u.d;
}

double powd(double a, double b) {
    if (b == 0.0) return 1.0;
    if (b == 1) return a;
    if (b == -1) return 1 / a;
    if (a == 0.0) return 0;
    if (a < 0.0) todo("Complex numbers"); // return 0.0; // crude safeguard
    if (floor(b) == b) return powdi(a, b);

    union {
        double d;
        uint64_t i;
    } u = {a};
    // log2(a) ≈ exponent - bias + mantissa_fraction
    double exponent = (double) ((u.i >> 52) & 0x7FF) - 1023.0;
    double mantissa = (double) (u.i & ((1ULL << 52) - 1)) / (1ULL << 52);

    double log2a = exponent + mantissa;

    double pow2 = b * log2a;

    // Split integer and fractional part
    double intpart;
    double fracpart = modf(pow2, &intpart);
    // double fracpart = std::modf(pow2, &intpart);
    // double fracpart = mod_d(pow2, intpart); ƒ

    // 2^fracpart ≈ 1 + fracpart * ln(2)
    const double ln2 = 0.6931471805599453;
    double frac_approx = 1.0 + fracpart * ln2;

    // Build result
    union {
        double d;
        uint64_t i;
    } res;
    res.i = (uint64_t) ((intpart + 1023.0) * (1ULL << 52));
    return res.d * frac_approx;
}


int printf(const char *__restrict format, ...) {
    //    todo better
    //    error("printf not linked");
#if not WASM
	warn("printf not linked");
#endif
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

void printf(char const *format, int64 l) {
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

void printf(chars format, int64 i, int64 j) {
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

void printf(chars format, double i, double j) {
    print(String(format).replace("%f", String(i)).replace("%f", String(j)));
}

void printf(chars format, chars val, int value) {
    print(String(format).format((char *) val).format(value));
}

void printf(chars format, void *value) {
    print(String(format).replace("%p", String((int64) value)));
}

extern "C" void *__cxa_allocate_exception(size_t thrown_size) { return 0; }

extern "C" void __cxa_throw(
    void *thrown_exception,
    struct type_info *tinfo,
    void (*dest)(void *)) {
}

extern "C" int __cxa_atexit(int, int, int) {
    // registers a function to be called by exit or when a shared library is unloaded.
    // todo free stuff?
    return 0;
}


void *alloc(int num, int size) {
    return calloc(num, size);
}

//#if not WASI
// /opt/wasm/wasi-sdk/share/wasi-sysroot/include/stdlib.h
// /opt/wasm/wasi-sdk/share/wasi-sysroot/include/__functions_malloc.h redundant!

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

//__attribute__((__malloc__, __warn_unused_result__))
void *calloc(size_t num, size_t size) {
    //    debugCalloc(num, size);
    char *mem = (char *) malloc(size * num);
    while (num < MAX_MEM and size / 8 * num > 0) { ((int64 *) mem)[--num * size / 8] = 0; }
    return mem;
}

void *aligned_alloc(size_t __alignment, size_t __size) {
    while (((long) heap_end) % 8)heap_end++;
    return malloc(__size);
}

extern "C" void *malloc(size_t size) {
    //}  __result_use_check __alloc_size(1){ // heap
    if (size > 1000000) {
        put_chars("malloc of huge memory chunk:");
        putx(size);
        error("implausible size ;)");
    }
    if ((long) heap_end < 10000) {
        //        current = (char*)&__data_end;
        heap_end = &__heap_base;
        //        error("current not set");
    }
    //    while(((long)current)%8)current++;
    void *last = heap_end;
    heap_end += size;
    //	if(size>1000)
    bool check_overflow = false; // wasm trap: out of bounds memory access OK
    if (check_overflow and MAX_MEM and (int64) heap_end >= MAX_MEM) {
        put_chars("OUT OF MEMORY", 13);
        puti(sizeof(Node)); // 64
        puti(sizeof(String)); // 20
        puti(sizeof(Value)); // 8 int64
        puti((int) last);
        puti((int) memory);
        puti((int) heap_end);
        puti(MAX_MEM);
        error("OUT OF MEMORY"); // needs malloc :(
        panic();
    }
    return last;
}

extern "C" void *memset(void *ptr, int value, size_t num) {
    // todo very expensive
    int *p = (int *) ptr;
    while (num-- > 0)*p++ = value;
    return ptr; //?
}


extern "C" void *memmove(void *dest, const void *source, size_t num) {
    while (num < MAX_MEM and num-- > 0)
        ((char *) dest)[num] = ((char *) source)[num];
    return dest;
    // memmove will never return anything other than dest.  It's useful for chaining
}


//#endif

// new operator for ALL objects
void *operator new[](size_t size) {
    // stack
    byte *use = heap_end;
    heap_end += size;
    return use;
}

void *operator new(unsigned long size, std::align_val_t align) {
    while (((long) heap_end) % (long) align)heap_end++;
    byte *use = heap_end;
    heap_end += size;
    return use;
}


// new operator for ALL objects
void *operator new(size_t size) {
    // stack
    byte *use = heap_end;
    heap_end += size;
    return use;
}

void _cxa_allocate_exception() {
    print("_cxa_allocate_exception!");
}

void _cxa_throw() {
    print("_cxa_throw");
    error("OUT OF MEMORY");
}

void printf(int64 l) {
    printf("%ld", l);
}

String Backtrace(int skip, int skipEnd) {
    return "Backtrace: TODO";
}

void memcpy1(bytes dest, bytes source, int i) {
    while (i < MAX_MEM and --i >= 0)
        dest[i] = source[i];
}

#if not EMSCRIPTEN
__attribute__((__nothrow__, __leaf__, __nonnull__(1, 2)))
#endif
extern "C" void *memcpy(void *__restrict__ destination, const void *__restrict__ source, size_t num) {
    //extern "C" void* memcpy(char *destination, char *source, size_t num) {
    //    check_silent((int64) destination + num < MEMORY_SIZE)
    //    check_silent ((int64) source + num < MEMORY_SIZE)
    while (--num < MAX_MEM)
        ((char *) destination)[num] = ((char *) source)[num];
    return destination; //?
}


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

int system(chars command) {
    print("no system commands in current wasi!");
    return -1;
}

void panic() {
#ifndef WASM
	raise("panic");
#else
    char *x = 0;
    x[-1] = 2; // Bus error: 10
#endif
}


void put_chars(chars c, size_t len) {
    c_io_vector civ{.string = c, .length = len ? len : strlen(c)};
    fd_write(1, &civ, 1, 0);
}

extern "C" // destroys the export type signature! stdio.h:178:6
int puts(chars c) {
    // // int return needed for stdio compatibilty !
    c_io_vector civ{.string = c, .length = (size_t) strlen(c)};
    fd_write(1, &civ, 1, 0);
    return 1; // OK (int) c;// stdio
}

int put_s(String *s) {
    fd_write(1, (c_io_vector *) s, 1, 0);
    return 0; //(int) s;// stdio
}

int puti(int i) {
    put_chars(formatLong(i), 0);
    return i;
}


int64 putl(int64 l) {
    put_chars(formatLong(l), 0);
    //    printf("%lld", l);
    return l;
}

[[maybe_unused]] int64 putx(int64 l) {
    printf("%llx", l);
    return l;
}

void putp(int64 char_pointer) {
    // workaround for m3, which can't link pointers:  od.link_optional<puts>("*", "puts")
    printf("%s", (char *) char_pointer);
}

codepoint put_char(codepoint c) {
    printf("%c", c);
    return c;
}

float putf(float f) {
    puts(formatLong((int64) f));
    puts(".");
    puts(formatLong(((int64) (f * 1000)) % 1000));
    return f;
    //    printf("%f\n", f);
}

double putd(double f) {
    printf("%f\n", f);
    return f;
}

void *putp(void *f) {
    printf("%p\n", f);
    return f;
}

extern "C" String *put_string(String *s) {
    if (!s)return 0;
    put_chars(s->data, s->length);
    return s;
}

List<String> arguments() {
    List<String> args;
    int argc;
    int len;
    int has_args = args_sizes_get(&argc, &len);
    if (!has_args or argc == 0 or len == 0)return args;
    check(argc < 1000);
    char **argv = (char **) alloc(argc, 8);
    char *values = (char *) alloc(len, 1);
    args_get(argv, values);
    // is argv guaranteed to point into values?
    if (argv[0] != values) {
        warn("invalid wasi arguments. ignoring…");
        return args;
    }
    //            todo("wasmtime args_get is currently broken. Please use wasmer. ")
    for (int i = 0; i < argc; i++) {
        auto string = argv[i];
        args.add(String(string));
    }
    args.capacity = (int) (int64) argv; // hack ;)
    return args;
}

//extern "C"
int main(int argc, char **argv);

//extern "C" int64 wasp_main(); // directly linked

int64 call_wasp_main(int index) {
    //    (elem (;0;) (i32.const 1) func 333 334)
    //    (func (;333;) (type 1) (param i32) (result i32)
    //    __asm ("local.get 0");
    //    __asm ("call_indirect()->(i32)"); // => clang Segmentation fault
    /* (module
      (table 1111 anyfunc)
      (elem (i32.const 1110) $test)
      (func $test (result i32) (i32.const 42) )
      (func $main (result i32)
        (call_indirect (result i32) ;; / (type $type_i)  redundant!
          (i32.const 1110)
    ) ) )   */
    //    __asm ("nop");
    //    __asm ("nop");
    //    __asm ("nop");
    //    __asm ("nop");
    return index;
}

extern "C" void __wasm_call_ctors();

// un-export at link time to use main:_start
extern "C" void _start() {
    // •	&__data_end yields a plain offset because it’s statically laid out.
    // •	&__heap_base may yield a host pointer due to toolchain/runtime behavior.
    heap_end = &__heap_base;
    __wasm_call_ctors();
    __initial_heap_end = heap_end; // after internal initialization, safely(?) reset on different runs / as "GC" ?
    trace("__heap_base");
    trace(&__heap_base); // ok 0xf5be20
    // once was VERY HIGH 0x54641ddb0 to mapped wasm memory in runtime?
    // does NOT always represent a WASM byte offset (0x00000000 to 0xFFFFFFFF for 4GB max)?
    trace("__data_end");
    trace(&__data_end); // perfect 541dda4 ⚠️ Unused! set by runtime once
    check_silent(&__data_end<=&__heap_base)
    //    current = (char*) &__data_end;
    //    trace("__global_base");
    //    trace(__global_base);
    //    trace("__memory_base");
    //    trace(__memory_base);

    auto args = arguments();
    print("args:");
    print(args.size());
    print(args);
    smart_pointer_32 result = main(args.size(), (char **) args.capacity /*hack ;)*/);
    print(smartNode(result));
#if RUNTIME_ONLY
	print("Wasp runtime not meant to be executed. Use native wasp or wasp.wasm \n");
	// todo interpreter? // we can still use runtime for minimal parsing tasks?
#endif
    //    smart_pointer_64 result = wasp_main();
    // export linked to function with high index so we need to reloc:
    //    __asm ("nop");// works but too late! clang emits local.set 64 local.get 64
    //    __asm ("nop");
    //    __asm ("nop");
    //    __asm ("nop");
    //    __asm ("nop");
    //    __asm ("call 469");
    //    __asm (call_index_wasp_main);
    //    print(new Node(result));
}

extern "C" int putchar(int c) {
    // stdio
    put_char(c);
    return c;
}

extern "C" size_t strlen(const char *x) {
#if WASM
    //	memory fault at wasm address 0xffffffff in linear memory of size 0x40000000
    if ((long) x >= 0x40000000) return 0;
    //		error("strlen: invalid address");
#endif

    int l = 0;
    while (l < MAX_STRING_LENGTH and *x++) l++; // and (int64) x < MAX_MEM - 1? … let it fail!
    return l;
}


#if not MY_WASM
extern "C" int64 run_wasm(bytes buffer, int buf_size) {
    print(
        "⚠️ run_wasm not available. wasp built without runtime and not embedded in a host which exposes the following function:");
    print("extern int64 run_wasm(uint8* buffer, size_t buffer_length)");
    print("Please vote here to make this a WASI standard: https://github.com/WebAssembly/WASI/issues/477");
    error("⚠️ run_wasm not available. You can use wasp-full.wasm which comes with wasm-runtime builtin.");
    //    breakpoint_helper
    return 0;
}
#endif


extern "C" char *serialize(Node *n) {
    if (!n)return 0;
    return n->serialize();
}

extern "C" int size_of_node() {
    return sizeof(Node);
}

extern "C" int size_of_string() {
    return sizeof(String);
}

Type getReturnType(String name) {
    //    if (name.contains("->(i32)"))return int32;
    //    if (name.contains("->(f32)"))return float32;
    //    if (name.contains("->(f64)"))return float64;
    //    todow("getReturnType for "s + name);
    return unknown_type;
}


double pow(double x, double y) {
    // _NOEXCEPT
    return powd(x, y);
    //    return __builtin_pow(x, y);
}


#if RUNTIME_ONLY
Module &read_wasm(bytes buffer, int size0) {
	return *new Module();
}
#endif

#if !MY_WASM
// see wasm_helpers.cpp line ≈ 170
extern "C" char *run(chars code) {
    return eval(code).serialize(); // can't return cause async wasp.js tests
}
#endif

double modulo_double(double a, double b) {
    return a - b * trunc(a / b);
}

float modulo_float(float a, float b) {
    return a - b * trunc(a / b);
}

 #if RUNTIME_ONLY // precedence??
    float precedence(Node &operater){return 0.0f;}
#endif
