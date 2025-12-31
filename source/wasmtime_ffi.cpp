#include "wasmtime_internal.h"
#include "ffi_loader.h"
#include "ffi_marshaller.h"
#include "ffi_dynamic_wrapper.h"
#include "Context.h"
#include "Util.h"
#include <dlfcn.h>

#ifdef NATIVE_FFI
extern Map<String, Function> functions;
extern Map<String, Module *> native_libraries;
#endif

#if WASMTIME_PATCH
#include "wasmtime_extension.h"

int64 read_struct_field(const wasmtime_anyref_t &anyref, int field) {
    wasmtime_val_t field_val;

    if (wasmtime_anyref_struct_get_field(context, &anyref, field, &field_val)) {
        if (field_val.kind == WASMTIME_I32) {
            printf("Field %d value: %d\n", field, field_val.of.i32);
            return field_val.of.i32;
        }
        if (field_val.kind == WASMTIME_I64) {
            printf("Field %d value: %lld\n", field, field_val.of.i64);
            return field_val.of.i64;
        }
        if (field_val.kind == WASMTIME_F64) {
            printf("Field %d value: %f\n", field, field_val.of.f64);
            return (int64) field_val.of.f64;
        }
        printf("Field %d has unexpected type %u\n", field, field_val.kind);
        todo("General Field struct excess");
        wasmtime_val_unroot(&field_val);
    }
    printf("Failed to get field %d\n", field);
    error("Failed to get field\n");
}

Node struct_to_node_wasmtime(const wasmtime_anyref_t &anyref) {
    trace("Got structref - attempting to reflect fields");
    Node result;
    result.kind = structs;

    for (int fieldIdx = 0; fieldIdx < 8; fieldIdx++) {
        wasmtime_val_t field_val;
        if (!wasmtime_anyref_struct_get_field(context, &anyref, fieldIdx, &field_val)) {
            break;
        }

        Node fieldNode;
        if (field_val.kind == WASMTIME_I32) {
            fieldNode = Node(field_val.of.i32);
        } else if (field_val.kind == WASMTIME_I64) {
            fieldNode = Node(field_val.of.i64);
        } else if (field_val.kind == WASMTIME_F32) {
            fieldNode = Node((double) field_val.of.f32);
        } else if (field_val.kind == WASMTIME_F64) {
            fieldNode = Node(field_val.of.f64);
        } else {
            wasmtime_val_unroot(&field_val);
            continue;
        }

        result[fieldIdx] = fieldNode;
        wasmtime_val_unroot(&field_val);
    }

    trace("Reflected struct with "s + formatLong(result.size()) + " fields");
    trace(result);
    return result;
}

int64 read_ref(const wasmtime_val_t &results) {
    if (results.kind == WASMTIME_ANYREF or results.kind == WASMTIME_EXTERNREF) {
        wasmtime_anyref_t anyref = results.of.anyref;
        if (wasmtime_anyref_is_struct(context, &anyref)) {
            trace("new_object returned a structref");
            Node result = struct_to_node_wasmtime(anyref);
            return result.toSmartPointer();
        }
        trace("new_object returned an anyref but not a structref");
    }
    return 0;
}
#endif

void preload_ffi() {
#ifdef NATIVE_FFI
    static bool raylib_preloaded = false;
    if (!raylib_preloaded) {
        raylib_preloaded = true;
        trace("raylib preloading for Wasmtime FFI compatibility\n");
        dlopen("/opt/homebrew/lib/libraylib.dylib", RTLD_LAZY | RTLD_GLOBAL);
    }
#endif
    static bool sdl2_preloaded = false;
    if (!sdl2_preloaded) {
        sdl2_preloaded = true;
        trace("SDL2 preloading for Wasmtime FFI compatibility\n");
        dlopen("/opt/homebrew/lib/libSDL2.dylib", RTLD_LAZY | RTLD_GLOBAL);
    }
}

#ifdef NATIVE_FFI
void setup_ffi_imports(wasmtime_linker_t *linker, Module &meta) {
    Map<String, Module *> runtime_libraries{10};
    for (int i = 0; i < meta.functions.size(); i++) {
        Function &func = meta.functions.values[i];
        if (func.is_ffi && !func.ffi_library.empty()) {
            String lib_name = func.ffi_library;
            if (!runtime_libraries.has(lib_name)) {
                Module *lib_module = new Module();
                lib_module->name = lib_name;
                lib_module->is_native_library = true;
                runtime_libraries.add(lib_name, lib_module);
            }
            Module *lib_module = runtime_libraries[lib_name];
            if (!lib_module->functions.has(func.name)) {
                lib_module->functions.add(func.name, func);
            }
        }
    }

    for (int i = 0; i < runtime_libraries.size(); i++) {
        Module *lib_module = runtime_libraries.values[i];
        if (!lib_module || !lib_module->is_native_library) continue;

        String lib_name = lib_module->name;

        for (int j = 0; j < lib_module->functions.size(); j++) {
            Function &func = lib_module->functions.values[j];
            String func_name = func.name;

            void *ffi_func = ffi_loader.get_function(lib_name, func_name);
            if (ffi_func) {
                Signature &wasp_sig = func.signature;

                int param_count = wasp_sig.parameters.size();
                FFIMarshaller::CType return_ctype = FFIMarshaller::get_return_ctype(wasp_sig);
                int return_count = (return_ctype == FFIMarshaller::CType::Void) ? 0 : 1;

                wasm_valtype_t **P = param_count > 0 ? new wasm_valtype_t *[param_count] : nullptr;
                wasm_valtype_t **R = return_count > 0 ? new wasm_valtype_t *[return_count] : nullptr;

                for (int k = 0; k < param_count; k++) {
                    FFIMarshaller::CType param_ctype = FFIMarshaller::get_param_ctype(wasp_sig, k);
                    switch (param_ctype) {
                        case FFIMarshaller::CType::Int32:
                        case FFIMarshaller::CType::String:
                            P[k] = wasm_valtype_new(WASM_I32);
                            break;
                        case FFIMarshaller::CType::Int64:
                            P[k] = wasm_valtype_new(WASM_I64);
                            break;
                        case FFIMarshaller::CType::Float32:
                            P[k] = wasm_valtype_new(WASM_F32);
                            break;
                        case FFIMarshaller::CType::Float64:
                            P[k] = wasm_valtype_new(WASM_F64);
                            break;
                        default:
                            P[k] = wasm_valtype_new(WASM_I32);
                    }
                }

                if (return_count > 0) {
                    switch (return_ctype) {
                        case FFIMarshaller::CType::Int32:
                            R[0] = wasm_valtype_new(WASM_I32);
                            break;
                        case FFIMarshaller::CType::Int64:
                            R[0] = wasm_valtype_new(WASM_I64);
                            break;
                        case FFIMarshaller::CType::Float32:
                            R[0] = wasm_valtype_new(WASM_F32);
                            break;
                        case FFIMarshaller::CType::Float64:
                            R[0] = wasm_valtype_new(WASM_F64);
                            break;
                        default:
                            R[0] = wasm_valtype_new(WASM_I32);
                    }
                }

                wasm_valtype_vec_t params, results;
                wasm_valtype_vec_new(&params, param_count, P);
                wasm_valtype_vec_new(&results, return_count, R);

                wasm_functype_t *ffi_type = wasm_functype_new(&params, &results);

                FFIMarshaller::FFIContext *ctx = create_ffi_context(&wasp_sig, ffi_func, func_name);

                wasmtime_error_t *derr = wasmtime_linker_define_func(
                    linker, lib_name.data, lib_name.length, func_name.data, func_name.length, ffi_type,
                    ffi_dynamic_wrapper_wasmtime, ctx, NULL);

                wasm_functype_delete(ffi_type);

                if (P) delete[] P;
                if (R) delete[] R;

                if (derr) {
                    trace("FFI: Failed to define "s + func_name + " from " + lib_name);
                    wasmtime_error_delete(derr);
                } else {
                    trace("FFI: Loaded "s + func_name + " from " + lib_name + " with signature");
                }
            } else {
                warn("FFI: Failed to load "s + func_name + " from " + lib_name);
            }
        }
    }
}
#else
void setup_ffi_imports(wasmtime_linker_t *linker, Module &meta) {
    // FFI not available in WASM builds
}
#endif
