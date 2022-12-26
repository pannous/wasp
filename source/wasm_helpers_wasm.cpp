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

int64 powl(int64 a, int b) {
    int64 res = 0;
    int n = 1;
    do {
        if (b & 1) res += a * n;
        n *= 2;
        b = b >> 1;
    } while (b);
    return res;
}

inline int64 doubleToLongBits(double a) {
    return *((int64 *) &a);
}

inline double longBitsToDouble(int64 a) {
    return *((double *) &a);
}

double powd(double a, double b) {
    int x = (int) (doubleToLongBits(a) >> 32);
    int y = (int) (b * (x - 1072632447) + 1072632447);
    return longBitsToDouble(((int64) y) << 32);
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
    if (check_overflow and MEMORY_SIZE and (int64) current >= MEMORY_SIZE) {
#ifndef WASI
        error("TOO BIG LOL");
        puti(sizeof(Node));// 64
        puti(sizeof(String));// 20
        puti(sizeof(Value));// 8 int64
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
//    error("printf not linked");
    warn("printf not linked");
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
extern "C" int64 run_wasm(bytes buffer, int buf_size) {
    print("⚠️ run_wasm not available. wasp built without runtime and not embedded in a host which exposes the following function:");
    print("extern int64 run_wasm(uint8* buffer, size_t buffer_length)");
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
    while (num < MAX_MEM and size / 8 * num > 0) { ((int64 *) mem)[--num * size / 8] = 0; }
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

void memcpy0(char *destination, char *source, size_t num) {
    if ((int64) destination + num >= MEMORY_SIZE)return;
//		panic();
    if ((int64) source + num >= MEMORY_SIZE)return;
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

int system(chars command) {
    print("no system commands in current wasi!");
    return -1;
}

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
    return 1;// OK (int) c;// stdio
}

int put_s(String *s) {
    fd_write(1, (c_io_vector *) s, 1, 0);
    return 0;//(int) s;// stdio
}

int puti(int i) {
    put_chars(formatLong(i), 0);
//    newline();
//    printf("%d", i);
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

void putp(int64 char_pointer) {// workaround for m3, which can't link pointers:  od.link_optional<puts>("*", "puts")
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
    if (argv[0] != values) {
        warn("invalid wasi arguments. ignoring…");
        return args;
    }
//            todo("wasmtime args_get is currently broken. Please use wasmer. ")
    for (int i = 0; i < argc; i++) {
        auto string = argv[i];
        args.add(String(string));
    }
    args.capacity = (int) (int64) argv;// hack ;)
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

// un-export at link time to use main:_start
extern "C" void _start() {
    auto args = arguments();
//    smart_pointer_32 result = main(args.size(), (char **) args.capacity /*hack ;)*/);
//    print(smartNode(result));
#if RUNTIME_ONLY
    print("Wasp runtime not meant to be executed. Use wasp or wasp.wasm \n");
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
    if ((int64) x >= MEMORY_SIZE || ((int64) x) == 0x200000000LL) {
        put_chars("corrupt string", 13);
        put_chars(formatLong((int) x));
        return 0;
//        puts(x);
//        puti((int) (int64) x);// 0x1000000 16777216
//        error("corrupt string");
    }
    if ((int64) x == 0x1ffffffffLL || (int64) x >= 0xffffffff00000000LL ||
        ((int64) x >= 0x100000000LL and (int64) x <= 0x100100000LL))
        return false;// todo: valgrind debug corruption, usually because of not enough memory
//#if !WASM
//    return strlen(x);
//#endif
    while (l < MAX_STRING_LENGTH and (int64) x < MEMORY_SIZE - 1 and *x++)
        l++;
    return l;
}
