#include "wasmtime.h"
#include "wasm_reader.h"

#include "asserts.h"
#include <stdio.h>
#include <stdlib.h>
#include <wasm.h>
#include "Util.h"
#include <math.h>

//#include <wasmtime.h>
#include "wasmtime.h"
#include "wasm_reader.h"
#include <stdio.h>
#include <stdlib.h>
#include <wasm.h>
#include "Util.h"
#include <math.h>

//#pragma clang diagnostic push
//#pragma clang diagnostic ignored "-Wvla-cxx-extension"

// wasm-c-api/src/wasm-v8.cc:    extern bool FLAG_expose_gc;
// include/wasmtime/conf.h:#define WASMTIME_FEATURE_GC

static void exit_with_error(const char *message, wasmtime_error_t *error, wasm_trap_t *trap);

static wasm_engine_t *engine = NULL;
static wasmtime_store_t *store = NULL;
static wasmtime_context_t *context = NULL;
//static uint8_t *wasm_memory = NULL;
static bool initialized = false;

void init_wasmtime() {
    if (initialized) return; // Prevent re-initialization
    engine = wasm_engine_new();
    assert(engine != NULL);
    store = wasmtime_store_new(engine, NULL, NULL);
    assert(store != NULL);
    context = wasmtime_store_context(store);
    initialized = true;
}

void add_wasmtime_memory() {
    wasm_limits_t limits = {.min = 1, .max = 0}; // Configured memory limits
    wasm_memorytype_t *memtype = wasm_memorytype_new(&limits);
    wasmtime_memory_t memory0;
    wasmtime_error_t *error = wasmtime_memory_new(context, memtype, &memory0);
    wasm_memorytype_delete(memtype);
    if (error != NULL) {
        wasmtime_error_delete(error);
        fprintf(stderr, "Error creating Wasmtime memory\n");
        exit(1);
    }
    wasm_memory = wasmtime_memory_data(context, &memory0);
}

static void exit_with_error(const char *message, wasmtime_error_t *error, wasm_trap_t *trap) {
    fprintf(stderr, "Error: %s\n", message);
    if (error != NULL) {
        wasm_byte_vec_t error_message;
        wasmtime_error_message(error, &error_message);
        fprintf(stderr, "%.*s\n", (int) error_message.size, error_message.data);
        wasm_byte_vec_delete(&error_message);
        wasmtime_error_delete(error);
    } else if (trap != NULL) {
        wasm_byte_vec_t trap_message;
        wasm_trap_message(trap, &trap_message);
        fprintf(stderr, "%.*s\n", (int) trap_message.size, trap_message.data);
        wasm_byte_vec_delete(&trap_message);
        wasm_trap_delete(trap);
    }
    raise("Error in Wasmtime call: "s + message);
    // exit(1);
}

typedef wasm_trap_t *(wasm_wrap)(void *, wasmtime_caller_t *, const wasmtime_val_t *, size_t, wasmtime_val_t *, size_t);

const wasm_functype_t *funcType(Signature &signature);

wasm_wrap *link_import(String name);

extern "C" int64_t run_wasm(unsigned char *data, int size) {
    if (!initialized) init_wasmtime();

    wasmtime_module_t *module0 = NULL;
    // EXC_BAD_ACCESS (code=1, address=0x0) can be caused by List.h capacity too small WTH!
    wasmtime_error_t *error = wasmtime_module_new(engine, data, size, &module0);
    if (error != NULL) exit_with_error("Failed to compile module", error, NULL);

    Module &meta = read_wasm(data, size);
    int import_count = meta.import_count;
    List<wasmtime_extern_t> imports(import_count);
    // std::vector<wasmtime_extern_t> imports(import_count);
    // wasmtime_extern_t imports[import_count];
    meta.functions["getElementById"].signature.add(charp, "id").returns(externref);

    int import_nr = 0;
    for (const String &import_name: meta.import_names) {
        if (import_name.empty()) break;
        wasmtime_func_t func;
        Signature &signature = meta.functions[import_name].signature;
        print(import_name);
        print(signature.format());
        const wasm_functype_t *type0 = funcType(signature);
        wasmtime_func_new(context, type0, link_import(import_name), NULL, NULL, &func);
        wasmtime_extern_t import = {.kind = WASMTIME_EXTERN_FUNC, .of.func = func};
        imports.add(import);
        // imports[import_nr++] = import;
        //        wasm_functype_delete(type0);
    }

    wasmtime_instance_t instance;
    wasm_trap_t *trap = NULL;
    error = wasmtime_instance_new(context, module0, imports.data(), import_count, &instance, &trap);
    wasmtime_module_delete(module0);
    if (error != NULL || trap != NULL) exit_with_error("Failed to instantiate module", error, trap);

    wasmtime_extern_t run;
    bool ok = wasmtime_instance_export_get(context, &instance, "wasp_main", strlen("wasp_main"), &run);
    if (!ok || run.kind != WASMTIME_EXTERN_FUNC) {
        ok = wasmtime_instance_export_get(context, &instance, "main", strlen("main"), &run);
    }
    if (!ok || run.kind != WASMTIME_EXTERN_FUNC) {
        exit_with_error("Failed to retrieve function export wasp_main", NULL, NULL);
    }

    wasmtime_extern_t memory_export;
    if (wasmtime_instance_export_get(context, &instance, "memory", strlen("memory"), &memory_export)) {
        if (memory_export.kind == WASMTIME_EXTERN_MEMORY) {
            wasmtime_memory_t memory0 = memory_export.of.memory;
            wasm_memory = wasmtime_memory_data(context, &memory0);
        }
    }

    wasmtime_val_t results;
    error = wasmtime_func_call(context, &run.of.func, NULL, 0, &results, 1, &trap);
    if (error != NULL || trap != NULL) exit_with_error("Failed to call function", error, trap);

    int64_t result = results.of.i64;
    return result;
}


static void exit_with_error(const char *message, wasmtime_error_t *error, wasm_trap_t *trap);


// ⚠️  'fun' needs to be a Locally accessible symbol (via include, ...)
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

//typedef wasm_trap_t *(wasm_wrap)(void *env, wasmtime_caller_t *caller, const wasmtime_val_t *args, size_t nargs, wasmtime_val_t *results, size_t nresults);


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
    //	results=wasm_valtype_new_i32();
    int n = args[0].of.i32;
    results[0].of.i32 = n * n;
    //	results[0].kind.
    return NULL;
}

// could be unified with wasmer via #define get(0,i32) args[0].data->of.i32 / args[0].of.i32
wrap(square) {
    // stupid basic test, wasm import => runtime linking, of cause not useful
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

//struct iovs{
//    char* string;
//    size_t length;
//};

wrap(fd_write) {
    //    auto fd = args[0].of.i32;
    auto iovs_offset = args[1].of.i32;
    int string_offset = *(int *) (((char *) wasm_memory) + iovs_offset);
    char *s = ((char *) wasm_memory) + string_offset;
    //    size_t iovs_count = args[2].of.i32;
    if (!wasm_memory)
        printf("wasm_memory not linked");
    else
        printf("%s", s);
    return NULL;
}


wrap(putd) {
    float f = args[0].of.f64;
    printf("%f", f);
    return NULL;
}

wrap(args_sizes_get) {
    // mock
    //int args_sizes_get(char **argv, int *argc);
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

wrap(putc) {
    // put_char
    int i = args[0].of.i32;
    printf("%c", i);
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

wrap(download) {
    char *url = (char *) wasm_memory + args[0].of.i32;
    chars result = fetch(url);
    //    uint64 pointer = (uint64) (void *) result;
    uint64 pointer = 0x100000; // INTERNAL POINTER instead of wasm_memory
    results[0].of.i32 = pointer;
    //    results[0].of.i64 = pointer;
    strcpy((char *) wasm_memory + pointer, result); // todo HEAP!
    return NULL;
}

wrap(nop) {
    return NULL;
}

wrap(getElementById) {
    return NULL;
}

wrap(atoi) {
    int n = args[0].of.i32;
    auto a = (chars) ((char *) wasm_memory) + n;
    int i = parseLong(a);
    results[0].of.i32 = i;
    return NULL;
}

wrap(exit) {
    // proc_exit
    printf("\nEXIT!\n (only wasm instance, not host;)\n");
    todow("how to stop instance?");
    //    throw "how to stop instance?";
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

wrap(todo) {
    todo("this function should not be a wasm import, but part of the runtime!!");
    return NULL;
}

wrap(absi) {
    //	todo("this function should not be a wasm import, but part of the runtime!!");
    int64 i = args[0].of.i32;
    results[0].of.i32 = i > 0 ? i : -1;
    return NULL;
}

wrap(absf) {
    // todo remove, we have f64.abs!
    //	todo("this function should not be a wasm import, but part of the runtime!!");
    double i = args[0].of.f32;
    results[0].of.f32 = i > 0 ? i : -1;
    return NULL;
}


void test_lambda() {
    print("requestAnimationFrame lambda");
};

//wrap_any(requestAnimationFrame);
//wrap_any(test_lambda);

#define wrap_fun(fun) [](void *, wasmtime_caller_t *, const wasmtime_val_t *, size_t, wasmtime_val_t *, size_t)->wasm_trap_t*{fun();return NULL;};

wasm_wrap *link_import(String name) {
    // own WASI mock instead of https://docs.wasmtime.dev/c-api/wasi_8h.html
    if (name == "fd_write") return &wrap_fd_write;
    if (name == "args_get")return &wrap_args_get;
    if (name == "args_sizes_get")return &wrap_args_sizes_get;

    if (name == "todo") return &wrap_todo;
    if (name == "memset") return &wrap_memset; // should be provided by wasp!!
    if (name == "calloc") return &wrap_calloc;
    if (name == "_Z5abs_ff") return &wrap_absf; // why??
    // todo get rid of these again!
    if (name == "_Z7consolev") return &wrap_nop;

    if (name == "__cxa_guard_acquire") return &wrap_nop; // todo!?
    if (name == "__cxa_guard_release") return &wrap_nop; // todo!?
    if (name == "_Z13init_graphicsv") return &wrap_nop;
    if (name == "_Z21requestAnimationFramev") return wrap_fun(test_lambda);
    //	if (name == "_Z21requestAnimationFramev") return  wrap_fun(requestAnimationFrame);


    // catch these with #ifdef s !!!
    if (name == "_Z13run_wasm_filePKc") return &wrap_nop; // todo!?
    //	if (name == "_Z8typeName7Valtype") return &wrap_nop;// todo!?
    //	if (name == "_Z8run_wasmPhi") return &wrap_nop;
    //	if (name == "_Z11testCurrentv") return &wrap_nop;// hmmm self test?

    if (name == "fopen") return &wrap_nop; // todo!?
    if (name == "fseek") return &wrap_nop; // todo!?
    if (name == "ftell") return &wrap_nop; // todo!?
    if (name == "fread") return &wrap_nop; // todo!?
    if (name == "system") return &wrap_nop; // danger!


    if (name == "__cxa_begin_catch") return &wrap_nop;
    if (name == "_ZdlPv") return &wrap_nop; // delete

    // todo: merge!
    if (name == "_Z5raisePKc") return &wrap_exit;
    if (name == "_ZSt9terminatev") return &wrap_exit;
    if (name == "__cxa_atexit") return &wrap_exit;

    if (name == "__cxa_demangle") return &wrap_nop;
    if (name == "proc_exit") return &wrap_exit;
    if (name == "panic") return &wrap_exit;
    if (name == "raise") return &wrap_exit;

    if (name == "square") return &wrap_square;
    if (name == "powi") return &wrap_powi;
    if (name == "powd") return &wrap_powd;
    if (name == "pow") return &wrap_pow;

    if (name == "printf") return &wrap_puts;
    if (name == "print") return &wrap_puts;
    //	if (name == "logs") return &wrap_puts;
    //	if (name == "logi") return &wrap_puti;
    //    if (name == "logc") return &wrap_putc;
    // todo: map these to mangled print_xyz !
    if (name == "putx") return &wrap_putx;
    if (name == "puti") return &wrap_puti;
    if (name == "putl") return &wrap_puti;
    if (name == "puts") return &wrap_puts;
    if (name == "putf") return &wrap_putf;
    if (name == "putd") return &wrap_putd;
    if (name == "pow") return &wrap_pow; // logd

    if (name == "log") return &wrap_logd; // logd
    if (name == "logd") return &wrap_logd; // logd
    if (name == "putc") return &wrap_putc;
    //    if (name == "putchar") return &wrap_putc;// todo: remove duplicates!
    if (name == "put_char") return &wrap_putc; // todo: remove duplicates!
    if (name == "download") return &wrap_download;
    if (name == "getElementById") return &wrap_getElementById;
    if (name == "wasp_main") return &hello_callback;
    if (name == "memory")
        return 0; // not a funciton
    error("unmapped import "s + name);
    return 0;
}

//static void exit_with_error(const char *message, wasmtime_error_t *error, wasm_trap_t *trap) {
////	error(message);
//    fprintf(stderr, "error: %s\n", message);
//    wasm_byte_vec_t error_message;
//    if (error != NULL) {
//        wasmtime_error_message(error, &error_message);
//        wasmtime_error_delete(error);
//    } else {
//        wasm_trap_message(trap, &error_message);
//        wasm_trap_delete(trap);
//    }
//    fprintf(stderr, "%.*s\n", (int) error_message.size, error_message.data);
////    wasm_byte_vec_delete(&error_message);
//    // let Wasp handle this :
//    throw Error((char *) message);// todo copy sprintf error_message backtrace;
//}

#define own

static inline own wasm_functype_t *wasm_functype_new_4_1(
    own wasm_valtype_t *p1, own wasm_valtype_t *p2, own wasm_valtype_t *p3, own wasm_valtype_t *p4,
    own wasm_valtype_t *r
) {
    wasm_valtype_t *ps[4] = {p1, p2, p3, p4};
    wasm_valtype_t *rs[1] = {r};
    wasm_valtype_vec_t params, results;
    wasm_valtype_vec_new(&params, 4, ps);
    wasm_valtype_vec_new(&results, 1, rs);
    return wasm_functype_new(&params, &results);
}

wasm_valkind_t mapTypeToWasmtime(Type type) {
    /* enum wasm_valkind_enum {
    WASM_I32,
    WASM_I64,
    WASM_F32,
    WASM_F64,
    WASM_EXTERNREF = 128,
    WASM_FUNCREF,
    };
    */
    switch (type.value) {
        case int32:
            return WASM_I32;
        case i64:
            return WASM_I64;
        case float32:
            return WASM_F32;
        case float64:
            return WASM_F64;
        case externref:
            return WASM_EXTERNREF;
        case funcref:
            return WASM_FUNCREF;
        case charp:
            return WASM_I32;
        default:
            error("unknown type for wasmtime "s + typeName(type));
            return WASM_I32;
    }
}


const wasm_functype_t *funcType(Signature &signature) {
    int param_count = signature.parameters.size();

    // Map return type
    Type returnType = signature.return_types.last(none);
    wasm_valtype_t *return_type = 0;
    if (returnType != nils)
        return_type = wasm_valtype_new(mapTypeToWasmtime(returnType));

    // Allocate parameter types dynamically
    wasm_valtype_vec_t params, results;
    wasm_valtype_t **param_types = nullptr;

    if (param_count > 0) {
        param_types = new wasm_valtype_t *[param_count];
        for (int i = 0; i < param_count; ++i) {
            auto typ = signature.parameters[i].type;
            auto valtype = mapTypeToWasmtime(typ);
            param_types[i] = wasm_valtype_new(valtype);
        }
    }

    wasm_valtype_vec_new(&params, param_count, param_types);
    wasm_valtype_vec_new(&results, return_type ? 1 : 0, return_type ? &return_type : nullptr);

    // Clean up allocated memory for parameters
    // for (int i = 0; i < param_count; ++i) {
    //     wasm_valtype_delete(param_types[i]);
    // }
    // delete[] param_types;

    return wasm_functype_new(&params, &results);
}

const wasm_functype_t *funcType2(Signature &signature) {
    // ⚠️ these CAN NOT BE REUSED! interrupted by signal 11: SIGSEGV or BIZARRE BUGS if you try!
    //     ⚠ wasm_valtype_t *i = wasm_valtype_new(WASM_I32);  ⚠️

    // todo multi-value
    Type returnType0 = signature.return_types.last(none);
    Valtype returnType = mapTypeToWasm(returnType0);
    int param_count = signature.parameters.size();
    if (param_count == 0) {
        switch (returnType) {
            case none:
            case voids:
                return wasm_functype_new_0_0();
            case int32:
                return wasm_functype_new_0_1(wasm_valtype_new(WASM_I32));
            case i64:
                return wasm_functype_new_0_1(wasm_valtype_new(WASM_I64));
            default:
                break;
        }
    }
    if (param_count == 1) {
        Type &type = signature.parameters[0].type;
        Valtype valtype = mapTypeToWasm(type);
        switch (valtype) {
            case int32:
                switch (returnType) {
                    case none:
                        return wasm_functype_new_1_0(wasm_valtype_new(WASM_I32));
                    case int32:
                        return wasm_functype_new_1_1(wasm_valtype_new(WASM_I32), wasm_valtype_new(WASM_I32));
                    default:
                        break;
                }
            case i64:
                switch (returnType) {
                    case none:
                    case voids:
                        return wasm_functype_new_1_0(wasm_valtype_new(WASM_I64));
                    case i64:
                        return wasm_functype_new_1_1(wasm_valtype_new(WASM_I64), wasm_valtype_new(WASM_I64));
                    default:
                        break;
                }
            case float32:
                switch (returnType) {
                    case none:
                    case voids:
                        return wasm_functype_new_1_0(wasm_valtype_new(WASM_F32));
                    case float32:
                        return wasm_functype_new_1_1(wasm_valtype_new(WASM_F32), wasm_valtype_new(WASM_F32));
                    default:
                        break;
                }
            case float64:
                switch (returnType) {
                    case none:
                    case voids:
                        return wasm_functype_new_1_0(wasm_valtype_new(WASM_F64));
                    case float64:
                        return wasm_functype_new_1_1(wasm_valtype_new(WASM_F64), wasm_valtype_new(WASM_F64));
                    default:
                        break;
                }
            default:
                break;
        }
    }
    if (param_count == 2) {
        switch (returnType) {
            case int32:
                return wasm_functype_new_2_1(wasm_valtype_new(WASM_I32), wasm_valtype_new(WASM_I32),
                                             wasm_valtype_new(WASM_I32)); // printf(i32,i32)i32
            case i64:
                return wasm_functype_new_2_1(wasm_valtype_new(WASM_I32), wasm_valtype_new(WASM_I32),
                                             wasm_valtype_new(WASM_I64));
            case float32:
                return wasm_functype_new_2_1(wasm_valtype_new(WASM_F32), wasm_valtype_new(WASM_F32),
                                             wasm_valtype_new(WASM_F32));
            case float64: // why broken in wasmtime??
                return wasm_functype_new_2_1(wasm_valtype_new(WASM_F64), wasm_valtype_new(WASM_F64),
                                             wasm_valtype_new(WASM_F64)); // powd(f64,f64)f64
            default:
                break;
        }
    }
    if (param_count == 3)
        return wasm_functype_new_3_1(wasm_valtype_new(WASM_I32), wasm_valtype_new(WASM_I32),
                                     wasm_valtype_new(WASM_I32), wasm_valtype_new(WASM_I32)); //(char*,char*,i32,)i32 ;)
    if (param_count == 4)
        return wasm_functype_new_4_1(wasm_valtype_new(WASM_I32), wasm_valtype_new(WASM_I32),
                                     wasm_valtype_new(WASM_I32), wasm_valtype_new(WASM_I32),
                                     wasm_valtype_new(WASM_I32)); //(char*,char*,i32,)i32 ;)
    print(signature.format());
    error("missing signature mapping"s + signature.format());
    return 0;
}

//#pragma clang diagnostic pop
