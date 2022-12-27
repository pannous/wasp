#include "wasmtime.h"
#include "wasm_reader.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <wasm.h>
//#include <wasmtime.h>
#include "Util.h"
#include <math.h>

//#undef assert // assert.h:92 not as good!
//#define assert(condition) try{\
//if((condition)==0){printf("\n%s\n",#condition);error("assert FAILED");}else printf("\nassert OK: %s\n",#condition);\
//}catch(chars m){printf("\n%s\n%s\n%s:%d\n",m,#condition,__FILE__,__LINE__);exit(1);}


static void exit_with_error(const char *message, wasmtime_error_t *error, wasm_trap_t *trap);

const wasm_functype_t *funcType(Signature &signature);

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

typedef wasm_trap_t *(wasm_wrap)(void *, wasmtime_caller_t *, const wasmtime_val_t *, size_t, wasmtime_val_t *, size_t);
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
wrap(square) {// stupid basic test, wasm import => runtime linking, of cause not useful
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
        return NULL;// BUG!
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

wrap(args_sizes_get) { // mock
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

wrap(putc) {// put_char
    int i = args[0].of.i32;
    printf("%c", i);
    return NULL;
}

wrap(nop) {
    return NULL;
}

wrap(atoi) {
    int n = args[0].of.i32;
    auto a = (chars) ((char *) wasm_memory) + n;
    int i = parseLong(a);
    results[0].of.i32 = i;
    return NULL;
}

wrap(exit) { // proc_exit
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

wrap(absf) {// todo remove, we have f64.abs!
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
// own WASI mock
    if (name == "fd_write") return &wrap_fd_write;
    if (name == "args_get")return &wrap_args_get;
    if (name == "args_sizes_get")return &wrap_args_sizes_get;

    if (name == "todo") return &wrap_todo;
    if (name == "memset") return &wrap_memset;// should be provided by wasp!!
    if (name == "calloc") return &wrap_calloc;
    if (name == "_Z5abs_ff") return &wrap_absf;// why??
// todo get rid of these again!
    if (name == "_Z7consolev") return &wrap_nop;

    if (name == "__cxa_guard_acquire") return &wrap_nop;// todo!?
    if (name == "__cxa_guard_release") return &wrap_nop;// todo!?
    if (name == "_Z13init_graphicsv") return &wrap_nop;
    if (name == "_Z21requestAnimationFramev") return wrap_fun(test_lambda);
//	if (name == "_Z21requestAnimationFramev") return  wrap_fun(requestAnimationFrame);


// catch these with #ifdef s !!!
    if (name == "_Z13run_wasm_filePKc") return &wrap_nop;// todo!?
//	if (name == "_Z8typeName7Valtype") return &wrap_nop;// todo!?
//	if (name == "_Z8run_wasmPhi") return &wrap_nop;
//	if (name == "_Z11testCurrentv") return &wrap_nop;// hmmm self test?

    if (name == "fopen") return &wrap_nop;// todo!?
    if (name == "fseek") return &wrap_nop;// todo!?
    if (name == "ftell") return &wrap_nop;// todo!?
    if (name == "fread") return &wrap_nop;// todo!?
    if (name == "system") return &wrap_nop;// danger!


    if (name == "__cxa_begin_catch") return &wrap_nop;
    if (name == "_ZdlPv") return &wrap_nop;// delete

    // todo: merge!
    if (name == "_Z5raisePKc") return &wrap_exit;
    if (name == "_ZSt9terminatev") return &wrap_exit;
    if (name == "__cxa_atexit") return &wrap_exit;

    if (name == "__cxa_demangle") return &wrap_nop;
    if (name == "proc_exit") return &wrap_exit;
    if (name == "panic") return &wrap_exit;
    if (name == "raise") return &wrap_exit;
    if (name == "square") return &wrap_square;

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
    if (name == "pow") return &wrap_pow;// logd

    if (name == "log") return &wrap_logd;// logd
    if (name == "logd") return &wrap_logd;// logd
    if (name == "putc") return &wrap_putc;
//    if (name == "putchar") return &wrap_putc;// todo: remove duplicates!
    if (name == "put_char") return &wrap_putc;// todo: remove duplicates!
    if (name == "wasp_main") return &hello_callback;
    if (name == "memory")
        return 0;// not a funciton
    error("unmapped import "s + name);
    return 0;
}

static void exit_with_error(const char *message, wasmtime_error_t *error, wasm_trap_t *trap) {
//	error(message);
    fprintf(stderr, "error: %s\n", message);
    wasm_byte_vec_t error_message;
    if (error != NULL) {
        wasmtime_error_message(error, &error_message);
        wasmtime_error_delete(error);
    } else {
        wasm_trap_message(trap, &error_message);
        wasm_trap_delete(trap);
    }
    fprintf(stderr, "%.*s\n", (int) error_message.size, error_message.data);
//    wasm_byte_vec_delete(&error_message);
    // let Wasp handle this :
    throw Error((char *) message);// todo copy sprintf error_message backtrace;
}

bool done = 0;
wasm_engine_t *engine;
wasmtime_store_t *store;
wasmtime_context_t *context;

void init_wasmtime() {
    engine = wasm_engine_new();
    assert(engine != NULL);
    store = wasmtime_store_new(engine, NULL, NULL);
    assert(store != NULL);
//	wasmtime_context_get_data(context);// get some_meta_data
    context = wasmtime_store_context(store);
    done = 1;// can we really reuse these or does it result in errors like:
//	thread '<unnamed>' panicked at 'object used with the wrong store', /opt/wasm/wasmtime/crates/wasmtime/src/func.rs:682:9
//	fatal runtime error: failed to initiate panic, error 5
//  Process finished with exit code 134 (interrupted by signal 6: SIGABRT)

// done = 0 doesn't help:
//	UndefinedBehaviorSanitizer:DEADLYSIGNAL
//	==59468==ERROR: UndefinedBehaviorSanitizer: BUS on unknown address (pc 0x00019e9b3b6c bp 0x00016f6f7940 sp 0x00016f6f7930 T1023049)
//	==59468==The signal is caused by a UNKNOWN memory access.

//	void free_wasmtime(){
    //	wasmtime_store_delete(store);
    //	wasm_engine_delete(engine);
}

void add_wasmtime_memory() {
    wasm_limits_t limits = {
            .min = 0,//100, SIGBUS / EXC_BAD_ACCESS if too small / out of bounds access (naturally)
//			.max = 0x7FFFFFFF,//  assertion failed: … <= absolute_max
    };
    const wasm_memorytype_t *memtype = wasm_memorytype_new(&limits);
    wasmtime_memory_t wasmtimeMemory{.store_id=1, .index=0}; // filled later:
    auto ok = wasmtime_memory_new(context, memtype, &wasmtimeMemory);
    if (!ok)error("wasmtime_memory_new failed");
    uint8_t *memory_data = wasmtime_memory_data(context, &wasmtimeMemory);
    wasm_memory = memory_data;// todo: keep old?
}


extern "C" int64 run_wasm(unsigned char *data, int size) {
    if (!done)
        init_wasmtime();
    wasmtime_error_t *error;
    wasmtime_module_t *modul = NULL;

    error = wasmtime_module_new(engine, (uint8_t *) data, size, &modul);
    if (error != NULL)exit_with_error("failed to compile module", error, NULL);

    wasm_trap_t *trap = NULL;// (wasm_trap_t *) malloc(10000); //wasm_trap_new((wasm_store_t *)store, NULL); //"Error?"
    wasmtime_instance_t instance;

    // UNCACHED because each main.wasm is different
    Module &meta = read_wasm(data, size);// wasmtime module* sucks so we read it ourselves!
    int importCount = meta.import_count;
    wasmtime_extern_t imports[1 + importCount * 2];
    int i = 0;
    // LINK IMPORTS!
    for (String &import_name: meta.import_names) {
        if (import_name.empty())break;
        if (import_name == "memory")continue;// todo filter before
//        printf("import: %s\n", import_name.data);
        wasmtime_extern_t import;
        wasmtime_func_t link;
//		Signature &signature = meta.signatures[import_name];
        if (!meta.functions.has(import_name))
            error("impossible");
        Function &function = meta.functions[import_name];
        function.name = import_name;
        Signature &signature = function.signature;
        if (import_name == "proc_exit") {
            signature.return_types.clear();
            signature.wasm_return_type = none;
        }
//        function.name = signature.debug_name = import_name;
        const wasm_functype_t *type = funcType(signature);
        wasm_wrap (*callback) = link_import(import_name);
        wasmtime_func_new(context, type, callback, NULL, NULL, &link);
        import.kind = WASMTIME_EXTERN_FUNC;
        import.of.func = link;
        imports[i++] = import;
    }

    error = wasmtime_instance_new(context, modul, imports, importCount, &instance, &trap);
    if (error != NULL || trap != NULL) exit_with_error("failed to instantiate", error, trap);

    wasmtime_extern_t run;
    wasmtime_extern_t memory_export;
    // WDYM?? 	thread '<unnamed>' panicked at 'index out of bounds: the len is 447 but the index is 4294967295'
    // wasmtime::instance::Instance::_get_export::h5e31a076a79e322b
    bool ok = wasmtime_instance_export_get(context, &instance, "wasp_main", 4, &run);
//	assert(ok);
//	assert(run.kind == WASMTIME_EXTERN_FUNC);
    ok = wasmtime_instance_export_get(context, &instance, "memory", 6, &memory_export);
//	assert(ok);
//	assert(memory_export.kind == WASMTIME_EXTERN_MEMORY);
    if (ok) {
        wasmtime_memory_t wasmtimeMemory = memory_export.of.memory;
        uint8_t *memory_data = wasmtime_memory_data(context, &wasmtimeMemory);
        if (memory_data)
            wasm_memory = memory_data;
        else
            warn("wasm module exports no memory");
    } else
        warn("wasm module exports no memory");

//	else error("wasmtime_instance_export_get failed");


//	wasmtime_memory_t memory(store_id:0/*store.id*/, 0);
//wasmtime_store

//	wasm_extern_vec_t exports;
//	const wasm_instance_t* instance2= reinterpret_cast<const wasm_instance_t *>(&instance);
//	wasm_instance_exports(instance2, &exports);
//	size_t ix = 0;
//	wasm_memory_t* memory = get_export_memory(&exports, ix++);// wrong api

//	check(wasm_memory_size(memory) == 2);
//	check(wasm_memory_data_size(memory) == 0x20000);
//	check(wasm_memory_data(memory)[0] == 0);
//	wasm_memory_data(memory);
//	byte_t* memory_data=wasm_memory_data((wasm_memory_t*)&memory);




    wasmtime_val_t results;
    wasmtime_func_t wasmtimeFunc = run.of.func;

//    Undefined symbols for architecture arm64:
//  "_wasmtime_error_delete", referenced from:
    // todo wasmtime 4.0 api?

//	int nresults = wasm_func_result_arity(wasmFunc); // needs workaround in wasmtime
//	WDYM??? thread '<unnamed>' panicked at 'object used with the wrong store', /opt/wasm/wasmtime/crates/wasmtime/src/func.rs:682:9
//    auto functypeResults = wasm_functype_results(funcType); // workaround stopped working
//    int nresults = functypeResults->size;
//    if (nresults > 1)
//        print("Using multi-value!");
//	else
//		print("Using single-value (smart pointer)");
//	wasm_func_t* wasmFunc = (wasm_func_t*)&wasmtimeFunc;
//	nargs=wasm_func_param_arity if nargs>0 args =[…]
    int nresults = 1;// todo how, wasmtime?
    error = wasmtime_func_call(context, &run.of.func, NULL, 0, &results, nresults, &trap);
    // object used with the wrong store WDYM??
    if (error != NULL || trap != NULL)exit_with_error("failed to call function", error, trap);
    int64_t result = results.of.i64;
    if (results.kind == WASMTIME_I32)
        result = results.of.i32;
    result = smartNode(result)->toSmartPointer();// COPY potential RESULT DATA from wasm memory to HOST!!

//    printf("%lld", result);
    if (nresults > 1) {
        wasmtime_val_t results2 = *(&results + 1);
        auto type = results2.of.i32;
//      wasm multi-value ABI swaps stack order, so no need to swap here
//		wasmtime_val_t results2 = *(&results + 1);
//		result = results2.of.i64;
//		auto type = results.of.i32;
        if (type >= undefined and type <= last_kind) {
//            if(tracing)
            printf("TYPE: %s\n", typeName(type));
        } else printf("Unknown type 0x%x\n", (unsigned int) type);
    }
    wasmtime_module_delete(modul);
    return result;
}

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

const wasm_functype_t *funcType(Signature &signature) {
// ⚠️ these CAN NOT BE REUSED! interrupted by signal 11: SIGSEGV or BIZARRE BUGS if you try!
//     ⚠ wasm_valtype_t *i = wasm_valtype_new(WASM_I32);  ⚠️

    // todo multi-value
    Type returnType0 = signature.return_types.last(none);
    Valtype returnType = mapTypeToWasm(returnType0);
    int param_count = signature.parameter_types.size();
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
        Type &type = signature.parameter_types[0];
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
