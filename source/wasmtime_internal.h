#pragma once

#include "wasi.h"
#include "wasmtime.h"
#include "wasmtime/val.h"
#include <wasm.h>

// Include actual type definitions instead of forward declarations
#include "String.h"
#include "NodeTypes.h"
#include "Code.h"

// Shared global state
extern wasm_engine_t *engine;
extern wasmtime_store_t *store;
extern wasmtime_context_t *context;
extern bool initialized;

// Helper functions
void exit_with_error(const char *message, wasmtime_error_t *error, wasm_trap_t *trap);
void set_result_null_externref(wasmtime_val_t *out);

// Type mapping
wasm_valkind_t mapTypeToWasmtime(Type type);
const wasm_functype_t *funcType(Signature &signature);

// Host function wrapper type
typedef wasm_trap_t *(wasm_wrap)(void *, wasmtime_caller_t *, const wasmtime_val_t *, size_t, wasmtime_val_t *, size_t);

// Host function linking
wasm_wrap *link_import(String name);

// FFI support
void preload_ffi();
void setup_ffi_imports(wasmtime_linker_t *linker, Module &meta);

#if WASMTIME_PATCH
int64 read_struct_field(const wasmtime_anyref_t &anyref, int field);
int64 read_ref(const wasmtime_val_t &results);
#endif
