#include "wasmtime_internal.h"
#include "wasi.h"
#include "wasmtime.h"
#include "wasmtime/val.h"
#if WASMTIME_PATCH
#include "wasmtime_extension.h"
#endif

#include <cstdio>
#include <cstdlib>
#include <wasm.h>
#include <unistd.h>
#include <cmath>
#include "ffi_loader.h"
#include "ffi_marshaller.h"
#include "ffi_dynamic_wrapper.h"
#include "Context.h"
#include "Util.h"
#include "wasm_reader.h"
#include "asserts.h"
#include "Keywords.h"

// Global state
wasm_engine_t *engine = NULL;
wasmtime_store_t *store = NULL;
wasmtime_context_t *context = NULL;
bool initialized = false;

#ifdef NATIVE_FFI
extern Map<String, Function> functions;
extern Map<String, Module *> native_libraries;
#endif

// Helper to consistently write a null externref result
void set_result_null_externref(wasmtime_val_t *out) {
    out->kind = WASMTIME_EXTERNREF;
#if defined(WASMTIME_ANYREF) || defined(WASMTIME_FEATURE_GC)
    wasmtime_externref_t tmp;
    wasmtime_externref_set_null(&tmp);
    out->of.externref = tmp;
#else
    out->of.externref = NULL;
#endif
}

void init_wasmtime() {
    if (initialized) return;

    wasm_config_t *config = wasm_config_new();
    wasmtime_config_wasm_gc_set(config, true);
    wasmtime_config_wasm_function_references_set(config, true);
    wasmtime_config_wasm_reference_types_set(config, true);

    engine = wasm_engine_new_with_config(config);
    check_silent(engine != NULL);
    store = wasmtime_store_new(engine, NULL, NULL);
    check_silent(store != NULL);
    context = wasmtime_store_context(store);
    initialized = true;
}

void add_wasmtime_memory() {
    wasm_limits_t limits = {.min = 1, .max = 0};
    wasm_memorytype_t *memtype = wasm_memorytype_new(&limits);
    wasmtime_memory_t memory0;
    wasmtime_error_t *error = wasmtime_memory_new(context, memtype, &memory0);
    wasm_memorytype_delete(memtype);
    if (error != NULL) {
        wasmtime_error_delete(error);
        tracef("Error creating Wasmtime memory\n");
        exit(1);
    }
    wasm_memory = wasmtime_memory_data(context, &memory0);
}

void exit_with_error(const char *message, wasmtime_error_t *error, wasm_trap_t *trap) {
    printf("Error: %s\n", message);
    if (error != NULL) {
        wasm_byte_vec_t error_message;
        wasmtime_error_message(error, &error_message);
        printf("%.*s\n", (int) error_message.size, error_message.data);
        wasm_byte_vec_delete(&error_message);
        wasmtime_error_delete(error);
    } else if (trap != NULL) {
        wasm_byte_vec_t trap_message;
        wasm_trap_message(trap, &trap_message);
        printf("%.*s\n", (int) trap_message.size, trap_message.data);
        wasm_byte_vec_delete(&trap_message);
        wasm_trap_delete(trap);
    }
    error("Error in Wasmtime call: "s + message);
}

wasm_valkind_t mapTypeToWasmtime(Type type) {
    switch (type.value) {
        case int32t:
            return WASM_I32;
        case i64:
            return WASM_I64;
        case float32t:
            return WASM_F32;
        case float64t:
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

    if (signature.functions.size() > 1) {
        auto n = signature.functions.first()->name;
        trace(n);
    }

    Type returnType = signature.return_types.last(none);
    wasm_valtype_t *return_type = 0;
    if (returnType != nils) {
        auto rk = mapTypeToWasmtime(returnType);
        return_type = wasm_valtype_new(rk);
    }

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

    return wasm_functype_new(&params, &results);
}

extern "C" int64_t run_wasm(unsigned char *data, int size) {
    Module &meta = read_wasm(data, size);
    if(meta.is_native_library) preload_ffi();
    if(meta.uses_native_library()) preload_ffi();
    if (!initialized) init_wasmtime();

    wasmtime_module_t *module0 = NULL;
    wasmtime_error_t *error = wasmtime_module_new(engine, data, size, &module0);
    if (error != NULL) exit_with_error("Failed to compile module", error, NULL);

    wasmtime_instance_t instance;
    wasm_trap_t *trap = NULL;

#if not  ENABLE_WASI_STDIO
#define ENABLE_WASI_STDIO
#endif
#ifdef ENABLE_WASI_STDIO
    wasmtime_linker_t *linker = wasmtime_linker_new(engine);
    if (linker == NULL) exit_with_error("Failed to create linker", NULL, NULL);

    wasi_config_t *wasi_cfg = wasi_config_new();
    if (wasi_cfg == NULL) exit_with_error("Failed to create WASI config", NULL, NULL);
    wasi_config_inherit_stdin(wasi_cfg);
    wasi_config_inherit_stdout(wasi_cfg);
    wasi_config_inherit_stderr(wasi_cfg);

    wasmtime_error_t *werr = wasmtime_context_set_wasi(context, wasi_cfg);
    if (werr != NULL) exit_with_error("Failed to set WASI on context", werr, NULL);
    werr = wasmtime_linker_define_wasi(linker);
    if (werr != NULL) exit_with_error("Failed to define WASI on linker", werr, NULL);

    // Define custom host functions into linker under module "env"
    for (String &import_name: meta.import_names) {
        if (import_name.empty()) break;
        if (import_name.in(wasi_functions)) continue;
#ifdef NATIVE_FFI
        if (meta.functions.has(import_name)) {
            Function &func = meta.functions[import_name];
            if (func.is_ffi && !func.ffi_library.empty()) {
                continue;
            }
        }
#endif

        Signature &signature = meta.functions[import_name].signature;
        if (import_name == "_ZdlPvm")
            signature.return_types.clear();
        if (import_name == "exit" || import_name == "proc_exit")
            signature.return_types.clear();
        const wasm_functype_t *type0 = funcType(signature);
        wasm_functype_t *own_type = (wasm_functype_t *) type0;
        wasmtime_func_callback_t cb = (wasmtime_func_callback_t) link_import(import_name);
        if (cb) {
            wasmtime_error_t *derr = wasmtime_linker_define_func( linker, "env", 3, import_name.data, import_name.length, own_type, cb, NULL, NULL);
            if (derr) exit_with_error("define_func failed", derr, NULL);
        }
    }

#ifdef NATIVE_FFI
    setup_ffi_imports(linker, meta);
#endif

    tracef("Instantiating module via linker...\n");
    error = wasmtime_linker_instantiate(linker, context, module0, &instance, &trap);
    wasmtime_module_delete(module0);
    tracef("Module instantiated\n");
    if (error != NULL || trap != NULL) exit_with_error("Failed to instantiate module via linker", error, trap);
#else
    // Legacy path: manual import array
    int import_count = meta.import_count;
    List<wasmtime_extern_t> imports(import_count);

    int import_nr = 0;
    for (const String &import_name: meta.import_names) {
        if (import_name.empty()) break;
        wasmtime_func_t func;
        Signature &signature = meta.functions[import_name].signature;
        trace(import_name);
        trace(signature.format());
        const wasm_functype_t *type0 = funcType(signature);
        wasmtime_func_new(context, type0, link_import(import_name), NULL, NULL, &func);
        wasmtime_extern_t import = {.kind = WASMTIME_EXTERN_FUNC, .of.func = func};
        imports.add(import);
    }

    error = wasmtime_instance_new(context, module0, imports.data(), import_count, &instance, &trap);
    wasmtime_module_delete(module0);
    if (error != NULL || trap != NULL) exit_with_error("Failed to instantiate module", error, trap);
#endif

    wasmtime_extern_t run;
    bool ok = wasmtime_instance_export_get(context, &instance, "wasp_main", strlen("wasp_main"), &run);
    if (!ok || run.kind != WASMTIME_EXTERN_FUNC) {
        ok = wasmtime_instance_export_get(context, &instance, "main", strlen("main"), &run);
    }
    if (!ok || run.kind != WASMTIME_EXTERN_FUNC) {
        exit_with_error("Failed to retrieve function export wasp_main", NULL, NULL);
    }

    tracef("Getting memory export...\n");
    fflush(stderr);
    wasmtime_extern_t memory_export;
    if (wasmtime_instance_export_get(context, &instance, "memory", strlen("memory"), &memory_export)) {
        if (memory_export.kind == WASMTIME_EXTERN_MEMORY) {
            wasmtime_memory_t memory0 = memory_export.of.memory;
            wasm_memory = wasmtime_memory_data(context, &memory0);
            tracef("Got memory at %p\n", wasm_memory);
            fflush(stderr);
        }
    }

    wasm_functype_t *func_type = wasmtime_func_type(context, &run.of.func);
    const wasm_valtype_vec_t *params = wasm_functype_params(func_type);
    size_t param_count = params->size;

    tracef("Calling wasp_main function with %zu parameters...\n", param_count);
    fflush(stderr);

    wasmtime_val_t args[2];
    wasmtime_val_t *args_ptr = NULL;

    if (param_count == 2) {
        args[0].kind = WASMTIME_I32;
        args[0].of.i32 = 0;
        args[1].kind = WASMTIME_I32;
        args[1].of.i32 = 0;
        args_ptr = args;
    } else if (param_count > 0) {
        warn("Unexpected parameter count for main function: "s + formatLong(param_count));
    }

    wasmtime_val_t results;
    error = wasmtime_func_call(context, &run.of.func, args_ptr, param_count, &results, 1, &trap);
    wasm_functype_delete(func_type);
    tracef("Function call returned\n");
    fflush(stderr);
    if (error != NULL || trap != NULL) exit_with_error("Failed to call function", error, trap);

    switch (results.kind) {
        case WASMTIME_I32:
            return results.of.i32;
        case WASMTIME_I64:
            return results.of.i64;
        case WASMTIME_F32:
            return (int64_t) (int32_t) results.of.f32;
        case WASMTIME_F64:
            return (int64_t) results.of.f64;
        case WASMTIME_FUNCREF:
            return 0;
        case WASMTIME_EXTERNREF:
        case WASMTIME_ANYREF:
#if WASMTIME_PATCH
            return read_ref(results);
#else
            warn("#if WASMTIME_PATCH not active for WASMTIME_ANYREF/WASMTIME_EXTERNREF");
            return 0;
#endif
        case WASMTIME_V128:
            return 0;
        default:
            return 0;
    }
}
