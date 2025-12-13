#pragma once

#include "ffi_marshaller.h"
#include "ffi_loader.h"
#include "Context.h"

// Dynamic FFI Wrapper System
// This replaces hardcoded wrappers with runtime signature-based dispatch

#ifdef WASMEDGE

#include "wasmedge/wasmedge.h"

// Single generic wrapper for WasmEdge - handles ALL FFI functions dynamically
WasmEdge_Result ffi_dynamic_wrapper(
    void *context_data,
    const WasmEdge_CallingFrameContext *CallFrameCxt,
    const WasmEdge_Value *In,
    WasmEdge_Value *Out
) {
    // Context contains FFISignature with function pointer and signature info
    FFIMarshaller::FFISignature* sig = (FFIMarshaller::FFISignature*)context_data;

    if (!sig || !sig->function_ptr) {
        return WasmEdge_Result_Fail;
    }

    // Get wasm memory for string conversions
    // extern uint8_t *wasm_memory;  // Defined in wasm_runner_edge.cpp

    // Dynamically marshal parameters based on signature
    List<FFIMarshaller::FFIValue> args(sig->param_types.size());

    for (int i = 0; i < sig->param_types.size(); i++) {
        FFIMarshaller::FFIValue arg;

        switch (sig->param_types[i]) {
            case FFIMarshaller::CType::Int32:
                arg.i32 = WasmEdge_ValueGetI32(In[i]);
                arg.i32 = FFIMarshaller::to_c_int32(arg.i32);
                break;

            case FFIMarshaller::CType::Int64:
                arg.i64 = WasmEdge_ValueGetI64(In[i]);
                arg.i64 = FFIMarshaller::to_c_int64(arg.i64);
                break;

            case FFIMarshaller::CType::Float32:
                arg.f32 = WasmEdge_ValueGetF32(In[i]);
                arg.f32 = FFIMarshaller::to_c_float(arg.f32);
                break;

            case FFIMarshaller::CType::Float64:
                arg.f64 = WasmEdge_ValueGetF64(In[i]);
                arg.f64 = FFIMarshaller::to_c_double(arg.f64);
                break;

            case FFIMarshaller::CType::String: {
                int32_t offset = WasmEdge_ValueGetI32(In[i]);
                arg.str = FFIMarshaller::offset_to_c_string(wasm_memory, offset);
                break;
            }

            default:
                arg.i64 = 0;
                break;
        }

        args.add(arg);
    }

    // Call the native function dynamically
    FFIMarshaller::FFIValue result = FFIMarshaller::call_dynamic(*sig, args);

    // Marshal return value based on signature
    switch (sig->return_type) {
        case FFIMarshaller::CType::Int32: {
            int32_t ret = FFIMarshaller::from_c_int32(result.i32);
            Out[0] = WasmEdge_ValueGenI32(ret);
            break;
        }

        case FFIMarshaller::CType::Int64: {
            int64_t ret = FFIMarshaller::from_c_int64(result.i64);
            Out[0] = WasmEdge_ValueGenI64(ret);
            break;
        }

        case FFIMarshaller::CType::Float32: {
            float ret = FFIMarshaller::from_c_float(result.f32);
            Out[0] = WasmEdge_ValueGenF32(ret);
            break;
        }

        case FFIMarshaller::CType::Float64: {
            double ret = FFIMarshaller::from_c_double(result.f64);
            Out[0] = WasmEdge_ValueGenF64(ret);
            break;
        }

        case FFIMarshaller::CType::String: {
            int32_t offset = FFIMarshaller::c_string_to_offset(wasm_memory, result.str);
            Out[0] = WasmEdge_ValueGenI32(offset);
            break;
        }

        default:
            Out[0] = WasmEdge_ValueGenI32(0);
            break;
    }

    return WasmEdge_Result_Success;
}

#endif // WASMEDGE

#ifdef WASMTIME

#include "wasmtime.h"

// Single generic wrapper for Wasmtime - handles ALL FFI functions dynamically
static wasm_trap_t* ffi_dynamic_wrapper_wasmtime(
    void *env,
    wasmtime_caller_t *caller,
    const wasmtime_val_t *args,
    size_t nargs,
    wasmtime_val_t *results,
    size_t nresults
) {
    // Env contains FFISignature with function pointer and signature info
    FFIMarshaller::FFISignature* sig = (FFIMarshaller::FFISignature*)env;

    if (!sig || !sig->function_ptr) {
        return NULL; // Or create trap
    }

    // Get wasm memory for string conversions
    extern void *wasm_memory;  // Defined in wasmtime_runner.cpp

    // Dynamically marshal parameters based on signature
    List<FFIMarshaller::FFIValue> ffi_args(sig->param_types.size());

    for (int i = 0; i < sig->param_types.size(); i++) {
        FFIMarshaller::FFIValue arg;

        switch (sig->param_types[i]) {
            case FFIMarshaller::CType::Int32:
                arg.i32 = args[i].of.i32;
                arg.i32 = FFIMarshaller::to_c_int32(arg.i32);
                break;

            case FFIMarshaller::CType::Int64:
                arg.i64 = args[i].of.i64;
                arg.i64 = FFIMarshaller::to_c_int64(arg.i64);
                break;

            case FFIMarshaller::CType::Float32:
                arg.f32 = args[i].of.f32;
                arg.f32 = FFIMarshaller::to_c_float(arg.f32);
                break;

            case FFIMarshaller::CType::Float64:
                arg.f64 = args[i].of.f64;
                arg.f64 = FFIMarshaller::to_c_double(arg.f64);
                break;

            case FFIMarshaller::CType::String: {
                int32_t offset = args[i].of.i32;
                arg.str = FFIMarshaller::offset_to_c_string(wasm_memory, offset);
                break;
            }

            default:
                arg.i64 = 0;
                break;
        }

        ffi_args.add(arg);
    }

    // Call the native function dynamically
    FFIMarshaller::FFIValue result = FFIMarshaller::call_dynamic(*sig, ffi_args);

    // Marshal return value based on signature
    switch (sig->return_type) {
        case FFIMarshaller::CType::Int32:
            results[0].kind = WASMTIME_I32;
            results[0].of.i32 = FFIMarshaller::from_c_int32(result.i32);
            break;

        case FFIMarshaller::CType::Int64:
            results[0].kind = WASMTIME_I64;
            results[0].of.i64 = FFIMarshaller::from_c_int64(result.i64);
            break;

        case FFIMarshaller::CType::Float32:
            results[0].kind = WASMTIME_F32;
            results[0].of.f32 = FFIMarshaller::from_c_float(result.f32);
            break;

        case FFIMarshaller::CType::Float64:
            results[0].kind = WASMTIME_F64;
            results[0].of.f64 = FFIMarshaller::from_c_double(result.f64);
            break;

        case FFIMarshaller::CType::String: {
            int32_t offset = FFIMarshaller::c_string_to_offset(wasm_memory, result.str);
            results[0].kind = WASMTIME_I32;
            results[0].of.i32 = offset;
            break;
        }

        default:
            results[0].kind = WASMTIME_I32;
            results[0].of.i32 = 0;
            break;
    }

    return NULL; // No trap
}

#endif // WASMTIME

// Helper to create FFI signature from Wasp Signature
inline void create_ffi_signature(
    FFIMarshaller::FFISignature& ffi_sig,
    class Signature& wasp_sig,
    void* func_ptr
) {
    ffi_sig.function_ptr = func_ptr;

    // Convert parameter types
    for (int i = 0; i < wasp_sig.parameters.size(); i++) {
        Type wasp_type = wasp_sig.parameters[i].type;
        FFIMarshaller::CType c_type = FFIMarshaller::detect_c_type(wasp_type);
        ffi_sig.param_types.add(c_type);
    }

    // Convert return type
    Type return_wasp_type = wasp_sig.return_types.last(none);
    ffi_sig.return_type = FFIMarshaller::detect_c_type(return_wasp_type);
}
