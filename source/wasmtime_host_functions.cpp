#include "wasmtime_internal.h"
#include "Context.h"
#include "Util.h"
#include "asserts.h"
#include "Keywords.h"
#include <cstdio>
#include <cmath>
#include <unistd.h>

// Wrapper macros for host functions
#define wrap_any(fun) static wasm_trap_t *wrap_##fun( void *env,\
wasmtime_caller_t *caller,\
const wasmtime_val_t *args,\
size_t nargs,\
wasmtime_val_t *results,\
size_t nresults\
){ fun();return NULL;}

#define wrap(fun) static wasm_trap_t *wrap_##fun( void *env,\
wasmtime_caller_t *caller,\
const wasmtime_val_t *args,\
size_t nargs,\
wasmtime_val_t *results,\
size_t nresults\
)

#define wrap_fun(fun) [](void *, wasmtime_caller_t *, const wasmtime_val_t *, size_t, wasmtime_val_t *, size_t)->wasm_trap_t*{fun();return NULL;};

// Test callback
static wasm_trap_t *hello_callback(
    void *env,
    wasmtime_caller_t *caller,
    const wasmtime_val_t *args,
    size_t nargs,
    wasmtime_val_t *results,
    size_t nresults
) {
    printf("Calling back...\n");
    printf("> Hello World!\n");
    int n = args[0].of.i32;
    results[0].of.i32 = n * n;
    return NULL;
}

// Math operations
wrap(square) {
    int n = args[0].of.i32;
    results[0].of.i32 = n * n;
    return NULL;
}

wrap(logd) {
    double n = args[0].of.f64;
    results[0].of.f64 = log(n);
    return NULL;
}

wrap(pow) {
    double n = args[0].of.f64;
    double x = args[1].of.f64;
    results[0].of.f64 = powd(n, x);
    return NULL;
}

wrap(powi) {
    int64 i = args[0].of.i64;
    int64 x = args[1].of.i64;
    results[0].of.i64 = powi(i, x);
    return NULL;
}

wrap(powd) {
    double i = args[0].of.f64;
    double x = args[1].of.f64;
    results[0].of.f64 = powd(i, x);
    return NULL;
}

wrap(absi) {
    int64 i = args[0].of.i32;
    results[0].of.i32 = i > 0 ? i : -1;
    return NULL;
}

wrap(absf) {
    double i = args[0].of.f32;
    results[0].of.f32 = i > 0 ? i : -1;
    return NULL;
}

// Output functions
wrap(puts) {
    int n = args[0].of.i32;
    if (wasm_memory)
        printf("%s\n", &((char *) wasm_memory)[n]);
    else
        printf("puts / printf can't access null wasm_memory at %d (internal error!)", n);
    return NULL;
}

wrap(putx) {
    int64 i = args[0].of.i64;
    printf("%llx", i);
    return NULL;
}

wrap(puti) {
    int i = args[0].of.i32;
    printf("%d", i);
    return NULL;
}

wrap(putl) {
    int64 i = args[0].of.i64;
    printf("%lld", i);
    return NULL;
}

wrap(putf) {
    if ((int64) args == 0x08)
        return NULL; // BUG!
    float f = args[0].of.f32;
    printf("%f", f);
    return NULL;
}

wrap(putd) {
    float f = args[0].of.f64;
    printf("%f", f);
    return NULL;
}

wrap(putc) {
    int i = args[0].of.i32;
    printf("%c", i);
    return NULL;
}

// WASI mocks
wrap(fd_write) {
    auto iovs_offset = args[1].of.i32;
    int string_offset = *(int *) (((char *) wasm_memory) + iovs_offset);
    char *s = ((char *) wasm_memory) + string_offset;
    if (!wasm_memory)
        printf("wasm_memory not linked");
    else
        printf("%s", s);
    return NULL;
}

wrap(args_sizes_get) {
    int argc = args[0].of.i32;
    int buf_len = args[1].of.i32;
    if (wasm_memory) {
        int *argcp = (int *) ((char *) wasm_memory) + argc;
        int *buf_lens = (int *) ((char *) wasm_memory) + buf_len;
        *argcp = 0;
        *buf_lens = 0;
    }
    return NULL;
}

wrap(args_get) {
    return NULL;
}

// String operations
wrap(concat) {
    int offset1 = args[0].of.i32;
    int offset2 = args[1].of.i32;
    int dest_offset = 20000; // todo! get HEAP_END
    char *s1 = (char *) wasm_memory + offset1;
    char *s2 = (char *) wasm_memory + offset2;
    char *neu = (char *) concat(s1, s2);
    strcpy2((char *) wasm_memory + dest_offset, neu);
    results[0].of.i32 = dest_offset;
    return NULL;
}

wrap(atoi) {
    int n = args[0].of.i32;
    auto a = (chars) ((char *) wasm_memory) + n;
    int i = parseLong(a);
    results[0].of.i32 = i;
    return NULL;
}

// Utility functions
wrap(sleep) {
    int ms = args[0].of.i32;
    usleep(ms * 1000);
    return NULL;
}

wrap(getenv) {
    auto env_var = (char *) wasm_memory + args[0].of.i32;
    const char *value = std::getenv(env_var);
    if (!value) {
        results[0].of.i32 = 0;
        return NULL;
    }
    uint64 pointer = 0x200000;
    strcpy2((char *) wasm_memory + pointer, value);
    results[0].of.i32 = pointer;
    return NULL;
}

wrap(download) {
    char *url = (char *) wasm_memory + args[0].of.i32;
    chars result = fetch(url);
    uint64 pointer = 0x100000;
    results[0].of.i32 = pointer;
    strcpy((char *) wasm_memory + pointer, result);
    return NULL;
}

// Type conversion stubs
wrap(toLong) {
    trace("toLong!!");
    results[0].kind = WASMTIME_I64;
    results[0].of.i64 = 123l;
    return NULL;
}

wrap(toNode) {
    todow("toNode!!");
    set_result_null_externref(&results[0]);
    return NULL;
}

wrap(toString) {
    todow("toString!!");
    results[0].kind = WASM_I32;
    results[0].of.i32 = 2000;
    char *dest = (char *) wasm_memory + results[0].of.i32;
    strcpy2(dest, "hello");
    dest[5] = '\0';
    return NULL;
}

wrap(toReal) {
    trace("toReal!!");
    results[0].kind = WASMTIME_F64;
    results[0].of.f64 = 123.0;
    return NULL;
}

wrap(formatLong) {
    trace("formatLong!!");
    int64 i = args[0].of.i64;
    int32_t offset = 2000;
    char *dest = (char *) wasm_memory + offset;
    strcpy2(dest, formatLong(i));
    results[0].kind = WASMTIME_I32;
    results[0].of.i32 = offset;
    return NULL;
}

// Stubs and no-ops
wrap(nop) {
    return NULL;
}

wrap(todos) {
    trace("TODO implement wasmtime func …");
    return NULL;
}

wrap(todo) {
    todo("this function should not be a wasm import, but part of the runtime!!");
    return NULL;
}

wrap(fprintf) {
    trace("fprintf!!");
    int offset = args[0].of.i32;
    char *arg = (char *) memory + offset;
    printf("%s", arg);
    return NULL;
}

wrap(getElementById) {
    trace("getElementById!!");
    results[0].kind = WASMTIME_EXTERNREF;
    set_result_null_externref(&results[0]);
    return NULL;
}

wrap(exit) {
    printf("\nEXIT!\n (only wasm instance, not host;)\n");
    todow("how to stop instance?");
    exit(42);
    return NULL;
}

wrap(memset) {
    todo("memset");
    return NULL;
}

wrap(calloc) {
    todo("calloc");
    return NULL;
}

void test_lambda() {
    trace("requestAnimationFrame lambda");
}

// Link import name to wrapper function
wasm_wrap *link_import(String name) {
    // WASI mocks
    if (name == "fd_write") return &wrap_fd_write;
    if (name == "args_get") return &wrap_args_get;
    if (name == "args_sizes_get") return &wrap_args_sizes_get;

    // Type conversions
    if (name == "todo") return &wrap_todo;
    if (name == "toNode") return &wrap_toNode;
    if (name == "toString") return &wrap_toString;
    if (name == "toLong") return &wrap_toLong;
    if (name == "toReal") return &wrap_toReal;
    if (name == "formatLong") return &wrap_formatLong;

    // Memory operations
    if (name == "memset") return &wrap_memset;
    if (name == "calloc") return &wrap_calloc;

    // Utility functions
    if (name == "_Z7consolev") return &wrap_nop;
    if (name == "getenv") return &wrap_getenv;
    if (name == "sleep") return &wrap_sleep;
    if (name == "fprintf") return &wrap_puts;

    // C++ runtime stubs
    if (name == "_ZdlPvm") return &wrap_nop;
    if (name == "__cxa_guard_acquire") return &wrap_nop;
    if (name == "__cxa_guard_release") return &wrap_nop;
    if (name == "_Z13init_graphicsv") return &wrap_nop;
    if (name == "_Z21requestAnimationFramev") return wrap_fun(test_lambda);
    if (name == "_Z13run_wasm_filePKc") return &wrap_nop;
    if (name == "__cxa_begin_catch") return &wrap_nop;
    if (name == "_ZdlPv") return &wrap_nop;
    if (name == "__cxa_demangle") return &wrap_nop;
    if (name == "_Z5abs_ff") return &wrap_absf;

    // File operations (stubs)
    if (name == "fopen") return &wrap_nop;
    if (name == "fseek") return &wrap_nop;
    if (name == "ftell") return &wrap_nop;
    if (name == "fread") return &wrap_nop;
    if (name == "fgetc") return &wrap_todo;
    if (name == "fclose") return &wrap_todo;
    if (name == "system") return &wrap_nop;

    // Exit functions
    if (name == "_Z5raisePKc") return &wrap_exit;
    if (name == "_ZSt9terminatev") return &wrap_exit;
    if (name == "__cxa_atexit") return &wrap_exit;
    if (name == "exit") return &wrap_exit;
    if (name == "proc_exit") return &wrap_exit;
    if (name == "panic") return &wrap_exit;
    if (name == "raise") return &wrap_exit;

    // Math operations
    if (name == "square") return &wrap_square;
    if (name == "powi") return &wrap_powi;
    if (name == "powd") return &wrap_powd;
    if (name == "pow") return &wrap_pow;
    if (name == "log") return &wrap_logd;
    if (name == "logd") return &wrap_logd;

    // Output operations
    if (name == "printf") return &wrap_puts;
    if (name == "print") return &wrap_puts;
    if (name == "putx") return &wrap_putx;
    if (name == "puti") return &wrap_puti;
    if (name == "putl") return &wrap_puti;
    if (name == "puts") return &wrap_puts;
    if (name == "putf") return &wrap_putf;
    if (name == "putd") return &wrap_putd;
    if (name == "putc") return &wrap_putc;
    if (name == "put_char") return &wrap_putc;

    // String operations
    if (name == "concat") return &wrap_concat;
    if (name == "atoi") return &wrap_atoi;

    // Network/DOM operations
    if (name == "download") return &wrap_download;
    if (name == "fetch") return &wrap_download;
    if (name == "getElementById") return &wrap_getElementById;

    // Main callback
    if (name == "wasp_main") return &hello_callback;
    if (name == "memory") return 0;

    trace("No static mapping for import: "s + name + " (will try dynamic FFI)");
    return 0;
}
