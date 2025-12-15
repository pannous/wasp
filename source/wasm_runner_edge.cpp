#if LINUX // todo: VM only!
#define DEFAULT_MAX_MEMORY (2UL * 1024 * 1024 * 1024)  // 2GB or less  in multipass VM
#endif


#include <wasmedge/wasmedge.h>
#include "wasm_runner.h"
#include "Util.h"
#include "Node.h"
#include "ffi_loader.h"
#include "ffi_marshaller.h"
#include "ffi_signatures.h"
#include "ffi_dynamic_wrapper.h"
#include "Context.h"
#include "Keywords.h"
#include <cstdio>
//#include <host/wasi/wasimodule.h>
//#include <host/wasmedge_process/processmodule.h>

// gcc wasmedge_runner.cpp -lwasmedge -o test_wasmedge

// supports externref threads …
// https://wasmedge.org/book/en/sdk/c.html
// https://wasmedge.org/book/en/sdk/c/externref.html

#define VMCxt context
typedef WasmEdge_CallingFrameContext FrameContext;


uint32_t AddFunc(uint32_t A, uint32_t B) { return A + B; }

uint32_t MulFunc(uint32_t A, uint32_t B) { return A * B; }

// Host function to call `SquareFunc` by external reference
WasmEdge_Result ExternSquare(void *Data,
                             const FrameContext *CallFrameCxt,
                             const WasmEdge_Value *In, WasmEdge_Value *Out) {
    error("square should ALWAYS be liked to runtime => be never used here!");
    auto x= WasmEdge_ValueGetI32(In[0]);
    Out[0] = WasmEdge_ValueGenI32(x*x);
    return WasmEdge_Result_Success;
}


// Note: fd_write, proc_exit, fclose, fopen, fprintf, getenv, fgetc, etc.
// are provided by WasmEdge's built-in WASI support via WasmEdge_ConfigureAddHostRegistration
// No need for manual implementations!

WasmEdge_Result getElementById(void *Data, const FrameContext *CallFrameCxt, const WasmEdge_Value *In,
                               WasmEdge_Value *Out) {
    return WasmEdge_Result_Success; // todo dummy!
}

WasmEdge_Result toString(void *Data, const FrameContext *CallFrameCxt, const WasmEdge_Value *In,
                         WasmEdge_Value *Out) {
                         int offset = 0x1000; // todo!!
    strcpy2((char *) wasm_memory+offset, "hello"); // todo dummy! (works!)
    Out[0] = WasmEdge_ValueGenI32(offset);
    return WasmEdge_Result_Success; // todo dummy!
}


WasmEdge_Result toLong(void *Data, const FrameContext *CallFrameCxt, const WasmEdge_Value *In,
                         WasmEdge_Value *Out) {
    Out[0] = WasmEdge_ValueGenI64(123);  // todo dummy!
    return WasmEdge_Result_Success;
}


WasmEdge_Result toReal(void *Data, const FrameContext *CallFrameCxt, const WasmEdge_Value *In,
                         WasmEdge_Value *Out) {
    Out[0] = WasmEdge_ValueGenF64(123.);  // todo dummy!
    return WasmEdge_Result_Success;
}


WasmEdge_Result toNode(void *Data, const FrameContext *CallFrameCxt, const WasmEdge_Value *In,
                         WasmEdge_Value *Out) {
    error("toNode not implemented");
    return WasmEdge_Result_Success;
}


WasmEdge_Result getDocumentBody(void *Data, const FrameContext *CallFrameCxt, const WasmEdge_Value *In,
                                WasmEdge_Value *Out) {
    return WasmEdge_Result_Success;
}

WasmEdge_Result createHtml(void *Data, const FrameContext *CallFrameCxt, const WasmEdge_Value *In,
                           WasmEdge_Value *Out) {
    return WasmEdge_Result_Success;
}

WasmEdge_Result nop(void *Data, const FrameContext *CallFrameCxt, const WasmEdge_Value *In,
                           WasmEdge_Value *Out) {
    return WasmEdge_Result_Success;
}

WasmEdge_Result concat_wrap(void *Data, const FrameContext *CallFrameCxt, const WasmEdge_Value *In, WasmEdge_Value *Out) {
    auto val1 = WasmEdge_ValueGetI32(In[0]);
    auto val2 = WasmEdge_ValueGetI32(In[1]);
    if(val2 == 0){
        Out[0]=WasmEdge_ValueGenI32((uint64_t)val1); // return pointer to result
        return WasmEdge_Result_Success;
    }
    long wasm_memory_size = 0x1000000; // todo: get from VM!
    printf("concat_wrap %d + %d\n", val1, val2);
    check_silent(wasm_memory != 0, "no wasm_memory (for concat)");
    check_silent(val1 >= 0, "val1 < 0 in concat_wrap");
    check_silent(val2 >= 0, "val2 < 0 in concat_wrap");
    check_silent(val1 < wasm_memory_size, "val1 out of bounds in concat_wrap");
    check_silent(val2 < wasm_memory_size, "val2 out of bounds in concat_wrap");
    auto left = (chars) wasm_memory + val1;
    auto right = (chars) wasm_memory + val2;
    printf("concat_wrap  %s\n", left);
    printf("concat_wrap  %s\n", right);
    printf("concat_wrap %s + %s\n", left, right);
    auto result=concat(left, right); // todo: concat and return new string
    int offset = 0; // todo: find free space in wasm_memory!!!
    strcpy2((char *) wasm_memory+offset, result);
    Out[0]=WasmEdge_ValueGenI32((uint64_t)offset); // return pointer to result
    // Out[0]=WasmEdge_ValueGenI32((uint64_t)result); // return pointer to result
    return WasmEdge_Result_Success;
}


WasmEdge_Result download(void *Data, const FrameContext *CallFrameCxt, const WasmEdge_Value *In, WasmEdge_Value *Out) {
    if (wasm_memory == 0)
        error("no wasm_memory (for download)");
    // int64_t val = WasmEdge_ValueGetI64(In[0]);
    auto val = WasmEdge_ValueGetI32(In[0]);
    auto url = (chars) wasm_memory + val;
    printf("val %d\n", val);
    printf("val %p\n", url);
    // auto url = (chars)val;
    printf("download %s\n", url);
    // todo("host download");
    auto data = fetch(url);
    printf("downloaded %s\n", data);
    int l=strlen(data);
    int offset = 0; // todo: find free space in wasm_memory!!!
    strcpy2((char *) wasm_memory+offset, data);
    // if(l>0 and ((char*)wasm_memory)[offset+l-1] == '\n')
        // ((char*)wasm_memory)[offset+l-1] = 0; // remove trailing newline HELLNO!
    Out[0]=WasmEdge_ValueGenI32((uint64_t)offset); // return pointer to result
    return WasmEdge_Result_Success;
}


WasmEdge_Result getExternRefPropertyValue(void *Data,
                                          const FrameContext *CallFrameCxt, const WasmEdge_Value *In,
                                          WasmEdge_Value *Out) {
    // Function type: {externref/anyref, i32} -> {i64}
    // Get struct reference and field index from parameters
    WasmEdge_Value structVal = In[0];
    uint32_t fieldIdx = WasmEdge_ValueGetI32(In[1]);

    // Check if it's a struct reference
#if not LINUX
    if (!WasmEdge_ValueIsStructRef(structVal)) {
        warn("getExternRefPropertyValue: value is not a struct reference");
        Out[0] = WasmEdge_ValueGenI64(0);
        return WasmEdge_Result_Success;
    }

    // Get the field value
    WasmEdge_Value fieldVal;
    WasmEdge_Result result = WasmEdge_ValueGetStructField(structVal, fieldIdx, &fieldVal);

    if (result.Code != WasmEdge_Result_Success.Code) {
        warn("getExternRefPropertyValue: failed to get struct field");
        Out[0] = WasmEdge_ValueGenI64(0);
        return WasmEdge_Result_Success;
    }

    // Convert field value to i64 based on its type
    // Check the type and extract the appropriate value
    if (WasmEdge_ValTypeIsI32(fieldVal.Type)) {
        int32_t val = WasmEdge_ValueGetI32(fieldVal);
        Out[0] = WasmEdge_ValueGenI64(val);
    } else if (WasmEdge_ValTypeIsI64(fieldVal.Type)) {
        int64_t val = WasmEdge_ValueGetI64(fieldVal);
        Out[0] = WasmEdge_ValueGenI64(val);
    } else if (WasmEdge_ValTypeIsF32(fieldVal.Type)) {
        float val = WasmEdge_ValueGetF32(fieldVal);
        Out[0] = WasmEdge_ValueGenI64((int64_t)val);
    } else if (WasmEdge_ValTypeIsF64(fieldVal.Type)) {
        double val = WasmEdge_ValueGetF64(fieldVal);
        Out[0] = WasmEdge_ValueGenI64((int64_t)val);
    } else {
        warn("getExternRefPropertyValue: unsupported field type");
        Out[0] = WasmEdge_ValueGenI64(0);
    }
#endif
    return WasmEdge_Result_Success;
}


// exit_edge removed - WASI provides proc_exit

WasmEdge_Result getenv_wrap(void *Data, const FrameContext *CallFrameCxt, const WasmEdge_Value *In, WasmEdge_Value *Out) {
    auto env_var_offset = WasmEdge_ValueGetI32(In[0]);
    auto env_var = (chars) wasm_memory + env_var_offset;
    const char *value = std::getenv(env_var);
    if (!value) {
        Out[0] = WasmEdge_ValueGenI32(0); // Return null pointer for non-existent env var
        return WasmEdge_Result_Success;
    }
    // Copy value to wasm memory at next available location
    int offset = 0x200000; // Use different offset than download()
    strcpy2((char *) wasm_memory + offset, value);
    Out[0] = WasmEdge_ValueGenI32(offset);
    return WasmEdge_Result_Success;
}

// FFI wrapper for double(double) functions
WasmEdge_Result ffi_wrapper_f64_f64(void *func_ptr, const FrameContext *CallFrameCxt, const WasmEdge_Value *In, WasmEdge_Value *Out) {
    typedef double (*ffi_func_t)(double);
    ffi_func_t func = (ffi_func_t)func_ptr;
    double arg = WasmEdge_ValueGetF64(In[0]);
    double c_arg = FFIMarshaller::to_c_double(arg);
    double c_result = func(c_arg);
    double result = FFIMarshaller::from_c_double(c_result);
    Out[0] = WasmEdge_ValueGenF64(result);
    return WasmEdge_Result_Success;
}

// FFI wrapper for double(double, double) functions
WasmEdge_Result ffi_wrapper_f64_f64_f64(void *func_ptr, const FrameContext *CallFrameCxt, const WasmEdge_Value *In, WasmEdge_Value *Out) {
    typedef double (*ffi_func_t)(double, double);
    ffi_func_t func = (ffi_func_t)func_ptr;
    double arg1 = FFIMarshaller::to_c_double(WasmEdge_ValueGetF64(In[0]));
    double arg2 = FFIMarshaller::to_c_double(WasmEdge_ValueGetF64(In[1]));
    double c_result = func(arg1, arg2);
    double result = FFIMarshaller::from_c_double(c_result);
    Out[0] = WasmEdge_ValueGenF64(result);
    return WasmEdge_Result_Success;
}

WasmEdge_Result abs_wrap(void *Data, const FrameContext *CallFrameCxt, const WasmEdge_Value *In, WasmEdge_Value *Out) {
    int32_t val = WasmEdge_ValueGetI32(In[0]);
    Out[0] = WasmEdge_ValueGenI32(val < 0 ? -val : val);
    return WasmEdge_Result_Success;
}

// FFI wrapper for string functions: char*(char*)
WasmEdge_Result ffi_wrapper_str_str(void *func_ptr, const FrameContext *CallFrameCxt, const WasmEdge_Value *In, WasmEdge_Value *Out) {
    typedef char* (*ffi_func_t)(char*);
    ffi_func_t func = (ffi_func_t)func_ptr;
    int32_t offset = WasmEdge_ValueGetI32(In[0]);
    const char* c_str = FFIMarshaller::offset_to_c_string(wasm_memory, offset);
    char* c_result = func((char*)c_str);
    int32_t result_offset = FFIMarshaller::c_string_to_offset(wasm_memory, c_result);
    Out[0] = WasmEdge_ValueGenI32(result_offset);
    return WasmEdge_Result_Success;
}




// Host function to call `AddFunc` by external reference
WasmEdge_Result ExternAdd(void *Data,
                          const FrameContext *CallFrameCxt,
                          const WasmEdge_Value *In, WasmEdge_Value *Out) {
    // Function type: {externref, i32, i32} -> {i32}
    //	uint32_t (*Func)(uint32_t, uint32_t) = WasmEdge_ValueGetExternRef(In[0]);
    //	uint32_t C = Func(WasmEdge_ValueGetI32(In[1]), WasmEdge_ValueGetI32(In[2]));
    //	Out[0] = WasmEdge_ValueGenI32(C);
    return WasmEdge_Result_Success;
}

// CreateWasiModule() removed - use WasmEdge's built-in WASI via WasmEdge_ConfigureAddHostRegistration instead


// Helper function to create the "extern_module" module instance.
WasmEdge_ModuleInstanceContext *CreateExternModule(WasmEdge_ModuleInstanceContext *HostMod = 0) {
    WasmEdge_String HostName;
    WasmEdge_FunctionTypeContext *HostFType = NULL;
    WasmEdge_FunctionInstanceContext *HostFunc = NULL;
    HostName = WasmEdge_StringCreateByCString("env"); // extern_module
    if (not HostMod) HostMod = WasmEdge_ModuleInstanceCreate(HostName);

    // REMOVED: exit, getenv, fgetc, fclose, fopen, fprintf - these are provided by WasmEdge's built-in WASI
    // Only register custom non-WASI functions below:

    {
        WasmEdge_ValType P[2], R[1];
        R[0] = WasmEdge_ValTypeGenI64();
        P[0] = WasmEdge_ValTypeGenExternRef();
        P[1] = WasmEdge_ValTypeGenI32();
        HostFType = WasmEdge_FunctionTypeCreate(P, 2, R, 1);
        HostFunc = WasmEdge_FunctionInstanceCreate(HostFType, getExternRefPropertyValue, NULL, 0);
        WasmEdge_FunctionTypeDelete(HostFType);
        HostName = WasmEdge_StringCreateByCString("getExternRefPropertyValue");
        WasmEdge_ModuleInstanceAddFunction(HostMod, HostName, HostFunc);
        WasmEdge_StringDelete(HostName);
    } {
        WasmEdge_ValType P[1], R[1];
        R[0] = WasmEdge_ValTypeGenExternRef();
        P[0] = WasmEdge_ValTypeGenI32(); // charp (id:string)
        HostFType = WasmEdge_FunctionTypeCreate(P, 1, R, 1);
        HostFunc = WasmEdge_FunctionInstanceCreate(HostFType, getElementById, NULL, 0);
        WasmEdge_FunctionTypeDelete(HostFType);
        HostName = WasmEdge_StringCreateByCString("getElementById");
        WasmEdge_ModuleInstanceAddFunction(HostMod, HostName, HostFunc);
        WasmEdge_StringDelete(HostName);
    } {
        WasmEdge_ValType P[1], R[1];
        R[0] = WasmEdge_ValTypeGenI32(); // charp (id:string)
        P[0] = WasmEdge_ValTypeGenExternRef();
        HostFType = WasmEdge_FunctionTypeCreate(P, 1, R, 1);
        HostFunc = WasmEdge_FunctionInstanceCreate(HostFType, toString, NULL, 0);
        WasmEdge_FunctionTypeDelete(HostFType);
        HostName = WasmEdge_StringCreateByCString("toString");
        WasmEdge_ModuleInstanceAddFunction(HostMod, HostName, HostFunc);
        WasmEdge_StringDelete(HostName);
    } {
        WasmEdge_ValType P[1], R[1];
        R[0] = WasmEdge_ValTypeGenI64();
        P[0] = WasmEdge_ValTypeGenExternRef();
        HostFType = WasmEdge_FunctionTypeCreate(P, 1, R, 1);
        HostFunc = WasmEdge_FunctionInstanceCreate(HostFType, toLong, NULL, 0);
        WasmEdge_FunctionTypeDelete(HostFType);
        HostName = WasmEdge_StringCreateByCString("toLong");
        WasmEdge_ModuleInstanceAddFunction(HostMod, HostName, HostFunc);
        WasmEdge_StringDelete(HostName);
    } {
        WasmEdge_ValType P[1], R[1];
        R[0] = WasmEdge_ValTypeGenF64();
        P[0] = WasmEdge_ValTypeGenExternRef();
        HostFType = WasmEdge_FunctionTypeCreate(P, 1, R, 1);
        HostFunc = WasmEdge_FunctionInstanceCreate(HostFType, toReal, NULL, 0);
        WasmEdge_FunctionTypeDelete(HostFType);
        HostName = WasmEdge_StringCreateByCString("toReal");
        WasmEdge_ModuleInstanceAddFunction(HostMod, HostName, HostFunc);
        WasmEdge_StringDelete(HostName);
    } {
        WasmEdge_ValType P[1], R[1];
        R[0] = WasmEdge_ValTypeGenI32(); // node pointer in wasm_memory!
        P[0] = WasmEdge_ValTypeGenExternRef();
        HostFType = WasmEdge_FunctionTypeCreate(P, 1, R, 1);
        HostFunc = WasmEdge_FunctionInstanceCreate(HostFType, toNode, NULL, 0);
        WasmEdge_FunctionTypeDelete(HostFType);
        HostName = WasmEdge_StringCreateByCString("toNode");
        WasmEdge_ModuleInstanceAddFunction(HostMod, HostName, HostFunc);
        WasmEdge_StringDelete(HostName);
    } {
        WasmEdge_ValType P[1], R[1];
        R[0] = WasmEdge_ValTypeGenI32(); // charp (id:string)
        P[0] = WasmEdge_ValTypeGenI32(); // charp (id:string)
        HostFType = WasmEdge_FunctionTypeCreate(P, 1, R, 1);
        HostFunc = WasmEdge_FunctionInstanceCreate(HostFType, download, NULL, 0);
        WasmEdge_FunctionTypeDelete(HostFType);
        HostName = WasmEdge_StringCreateByCString("download");
        WasmEdge_ModuleInstanceAddFunction(HostMod, HostName, HostFunc);
        WasmEdge_StringDelete(HostName);
    } {
        WasmEdge_ValType P[1], R[1];
        R[0] = WasmEdge_ValTypeGenI32(); // charp (id:string)
        P[0] = WasmEdge_ValTypeGenI32(); // charp (id:string)
        HostFType = WasmEdge_FunctionTypeCreate(P, 1, R, 1);
        HostFunc = WasmEdge_FunctionInstanceCreate(HostFType, download, NULL, 0);
        WasmEdge_FunctionTypeDelete(HostFType);
        HostName = WasmEdge_StringCreateByCString("fetch"); // todo download alias!
        WasmEdge_ModuleInstanceAddFunction(HostMod, HostName, HostFunc);
        WasmEdge_StringDelete(HostName);
    } {
        WasmEdge_ValType P[2], R[1];
        P[0] = WasmEdge_ValTypeGenI32(); // charp (id:string)
        P[1] = WasmEdge_ValTypeGenI32(); // charp (id:string)
        R[0] = WasmEdge_ValTypeGenI32(); // charp (id:string)
        HostFType = WasmEdge_FunctionTypeCreate(P, 2, R, 1);
        HostFunc = WasmEdge_FunctionInstanceCreate(HostFType, concat_wrap, NULL, 0);
        WasmEdge_FunctionTypeDelete(HostFType);
        HostName = WasmEdge_StringCreateByCString("concat"); // if we don't merge runtime
        WasmEdge_ModuleInstanceAddFunction(HostMod, HostName, HostFunc);
        WasmEdge_StringDelete(HostName);
    } {
        WasmEdge_ValType P[1], R[1];
        R[0] = WasmEdge_ValTypeGenI32(); // charp (result string)
        P[0] = WasmEdge_ValTypeGenI32(); // charp (env var name)
        HostFType = WasmEdge_FunctionTypeCreate(P, 1, R, 1);
        HostFunc = WasmEdge_FunctionInstanceCreate(HostFType, getenv_wrap, NULL, 0);
        WasmEdge_FunctionTypeDelete(HostFType);
        HostName = WasmEdge_StringCreateByCString("getenv");
        WasmEdge_ModuleInstanceAddFunction(HostMod, HostName, HostFunc);
        WasmEdge_StringDelete(HostName);
    } {
        WasmEdge_ValType P[1], R[1];
        R[0] = WasmEdge_ValTypeGenExternRef();
        HostFType = WasmEdge_FunctionTypeCreate(P, 0, R, 1);
        HostFunc = WasmEdge_FunctionInstanceCreate(HostFType, getDocumentBody, NULL, 0);
        WasmEdge_FunctionTypeDelete(HostFType);
        HostName = WasmEdge_StringCreateByCString("getDocumentBody");
        WasmEdge_ModuleInstanceAddFunction(HostMod, HostName, HostFunc);
        WasmEdge_StringDelete(HostName);
    } {
        WasmEdge_ValType P[2], R[1];
        R[0] = WasmEdge_ValTypeGenExternRef();
        P[0] = WasmEdge_ValTypeGenExternRef();
        P[1] = WasmEdge_ValTypeGenI32(); // string
        HostFType = WasmEdge_FunctionTypeCreate(P, 2, R, 1);
        HostFunc = WasmEdge_FunctionInstanceCreate(HostFType, createHtml, NULL, 0);
        WasmEdge_FunctionTypeDelete(HostFType);
        HostName = WasmEdge_StringCreateByCString("createHtml");
        WasmEdge_ModuleInstanceAddFunction(HostMod, HostName, HostFunc);
        WasmEdge_StringDelete(HostName);
    }
    // _ZdlPvm operator delete(void*, unsigned long)
    {
        WasmEdge_ValType P[2], R[0];
        P[0] = WasmEdge_ValTypeGenI32(); // pointer to delete
        P[1] = WasmEdge_ValTypeGenI32(); // size of pointer
        HostFType = WasmEdge_FunctionTypeCreate(P, 2, R, 0);
        HostFunc = WasmEdge_FunctionInstanceCreate(HostFType, nop, NULL, 0);
        WasmEdge_FunctionTypeDelete(HostFType);
        HostName = WasmEdge_StringCreateByCString("_ZdlPvm");
        WasmEdge_ModuleInstanceAddFunction(HostMod, HostName, HostFunc);
        WasmEdge_StringDelete(HostName);
    }

#ifdef NATIVE_FFI
    // Add FFI imports with dynamic signature detection from native libraries (Mac/Linux only)
    // WASM builds cannot load native .so/.dylib files
    for (int i = 0; i < ffi_functions.size(); i++) {
        FFIFunctionInfo& ffi_info = ffi_functions[i];
        String func_name = ffi_info.function_name;
        String lib_name = ffi_info.library_name;
        void* ffi_func = ffi_loader.get_function(lib_name, func_name);
        if (ffi_func) {
            // Detect signature using Wasp's signature detection
            Signature wasp_sig;
            detect_ffi_signature(func_name, lib_name, wasp_sig);

            // Convert to runtime FFISignature for dynamic wrapper
            FFIMarshaller::FFISignature sig = FFIMarshaller::wasp_signature_to_ffi(wasp_sig, ffi_func, func_name);

            // Create WasmEdge function type from signature
            int param_count = sig.param_types.size();
            int return_count = (sig.return_type == FFIMarshaller::CType::Void) ? 0 : 1;

            WasmEdge_ValType* P = param_count > 0 ? new WasmEdge_ValType[param_count] : nullptr;
            WasmEdge_ValType* R = return_count > 0 ? new WasmEdge_ValType[return_count] : nullptr;

            // Map parameter types
            for (int j = 0; j < param_count; j++) {
                switch (sig.param_types[j]) {
                    case FFIMarshaller::CType::Int32:
                    case FFIMarshaller::CType::String:  // Strings are passed as i32 offsets
                        P[j] = WasmEdge_ValTypeGenI32();
                        break;
                    case FFIMarshaller::CType::Int64:
                        P[j] = WasmEdge_ValTypeGenI64();
                        break;
                    case FFIMarshaller::CType::Float32:
                        P[j] = WasmEdge_ValTypeGenF32();
                        break;
                    case FFIMarshaller::CType::Float64:
                        P[j] = WasmEdge_ValTypeGenF64();
                        break;
                    default:
                        P[j] = WasmEdge_ValTypeGenI32();
                }
            }

            // Map return type
            if (return_count > 0) {
                switch (sig.return_type) {
                    case FFIMarshaller::CType::Int32:
                        R[0] = WasmEdge_ValTypeGenI32();
                        break;
                    case FFIMarshaller::CType::Int64:
                        R[0] = WasmEdge_ValTypeGenI64();
                        break;
                    case FFIMarshaller::CType::Float32:
                        R[0] = WasmEdge_ValTypeGenF32();
                        break;
                    case FFIMarshaller::CType::Float64:
                        R[0] = WasmEdge_ValTypeGenF64();
                        break;
                    default:
                        R[0] = WasmEdge_ValTypeGenI32();
                }
            }

            HostFType = WasmEdge_FunctionTypeCreate(P, param_count, R, return_count);

            // Allocate persistent signature for the wrapper
            FFIMarshaller::FFISignature* sig_ptr = new FFIMarshaller::FFISignature(sig);

            // Use dynamic wrapper that dispatches based on signature
            HostFunc = WasmEdge_FunctionInstanceCreate(HostFType, ffi_dynamic_wrapper, sig_ptr, 0);
            WasmEdge_FunctionTypeDelete(HostFType);

            if (P) delete[] P;
            if (R) delete[] R;

            HostName = WasmEdge_StringCreateByCString(func_name);
            WasmEdge_ModuleInstanceAddFunction(HostMod, HostName, HostFunc);
            WasmEdge_StringDelete(HostName);

            trace("FFI: Loaded "s + func_name + " from " + lib_name + " with signature");
        } else {
            warn("FFI: Failed to load "s + func_name + " from " + lib_name);
        }
    }
#endif // NATIVE_FFI

    // Register abs function (often imported from env instead of being a builtin)
    {
        WasmEdge_ValType P[1], R[1];
        P[0] = WasmEdge_ValTypeGenI32();
        R[0] = WasmEdge_ValTypeGenI32();
        HostFType = WasmEdge_FunctionTypeCreate(P, 1, R, 1);
        HostFunc = WasmEdge_FunctionInstanceCreate(HostFType, abs_wrap, NULL, 0);
        WasmEdge_FunctionTypeDelete(HostFType);
        HostName = WasmEdge_StringCreateByCString("abs");
        WasmEdge_ModuleInstanceAddFunction(HostMod, HostName, HostFunc);
        WasmEdge_StringDelete(HostName);
    }

    return HostMod;
}


WasmEdge_ModuleInstanceContext * duplicate_wasi_to_env(WasmEdge_VMContext *VMCxt, WasmEdge_StoreContext *store, chars name) {
    // Try to get the WASI module and duplicate it under "env" name
    // This allows legacy WASM files that import WASI functions from "env" to work
    WasmEdge_String wasi_name = WasmEdge_StringCreateByCString("wasi_snapshot_preview1");
    const WasmEdge_ModuleInstanceContext *wasi_module = WasmEdge_StoreFindModule(store, wasi_name);
    WasmEdge_StringDelete(wasi_name);

    WasmEdge_ModuleInstanceContext *env_module = nullptr;
    if (wasi_module) {
        // Create a new "env" / "wasi_unstable" module and copy WASI functions into it
        WasmEdge_String env_name = WasmEdge_StringCreateByCString(name);// env and wasi_unstable
        env_module = WasmEdge_ModuleInstanceCreate(env_name);

        // Copy common WASI functions to env module
        for (const char* func_name : wasi_function_list) {
            if (!func_name) break;
            WasmEdge_String fname = WasmEdge_StringCreateByCString(func_name);
            const WasmEdge_FunctionInstanceContext *wasi_func = WasmEdge_ModuleInstanceFindFunction(wasi_module, fname);
            if (wasi_func) {
                // Re-export the same function instance under "env" module
                WasmEdge_ModuleInstanceAddFunction(env_module, fname, (WasmEdge_FunctionInstanceContext*)wasi_func);
            }
            WasmEdge_StringDelete(fname);
        }
        WasmEdge_StringDelete(env_name);
    }

    // Add custom functions to the same env module (or create one if WASI not available)
    auto HostMod = CreateExternModule(env_module);
    WasmEdge_Result ok = WasmEdge_VMRegisterModuleFromImport(VMCxt, HostMod);
    if (not WasmEdge_ResultOK(ok)) {
        auto err = WasmEdge_ResultGetMessage(ok);
        printf("Error: %s\n", err);
    }
    return env_module;
}

extern "C" int64 run_wasm(bytes buffer, int buf_size) {
    // perfect except we can't access memory

    /* Create the configure context and add the WASI support. */
    /* This step is not necessary unless you need WASI support. */
    WasmEdge_ConfigureContext *conf = WasmEdge_ConfigureCreate();
    WasmEdge_ConfigureAddHostRegistration(conf, WasmEdge_HostRegistration_Wasi);

    //--enable-instruction-count
    //--enable-gas-measuring
    //--enable-time-measuring
    // wasmedge --enable-all
    // most are enabled by default and need explicit disabling --disable-multi-value …
    // ⚠️ "Though the user can specify enabling the proposal, the support for the proposal is not implemented yet." :(
    // Those implemented have a check mark: ✔️	https://wasmedge.org/book/en/features/proposals.html
    WasmEdge_ConfigureAddProposal(conf, WasmEdge_Proposal_Annotations);
    WasmEdge_ConfigureAddProposal(conf, WasmEdge_Proposal_BulkMemoryOperations);
    WasmEdge_ConfigureAddProposal(conf, WasmEdge_Proposal_Component);
    WasmEdge_ConfigureAddProposal(conf, WasmEdge_Proposal_ExtendedConst); // i32.add in global's init
    WasmEdge_ConfigureAddProposal(conf, WasmEdge_Proposal_ExceptionHandling);
    WasmEdge_ConfigureAddProposal(conf, WasmEdge_Proposal_FunctionReferences); // function pointers!!
    WasmEdge_ConfigureAddProposal(conf, WasmEdge_Proposal_GC);
    WasmEdge_ConfigureAddProposal(conf, WasmEdge_Proposal_ImportExportMutGlobals);
    WasmEdge_ConfigureAddProposal(conf, WasmEdge_Proposal_Memory64);
    WasmEdge_ConfigureAddProposal(conf, WasmEdge_Proposal_MultiMemories); // ✓ --enable-multi-memory
    WasmEdge_ConfigureAddProposal(conf, WasmEdge_Proposal_MultiValue); // ✓
    WasmEdge_ConfigureAddProposal(conf, WasmEdge_Proposal_NonTrapFloatToIntConversions);
    WasmEdge_ConfigureAddProposal(conf, WasmEdge_Proposal_ReferenceTypes); // externref ≠ GC types! :(
    WasmEdge_ConfigureAddProposal(conf, WasmEdge_Proposal_TailCall);
    WasmEdge_ConfigureAddProposal(conf, WasmEdge_Proposal_Threads);
    WasmEdge_ConfigureAddProposal(conf, WasmEdge_Proposal_SignExtensionOperators);
    //    WasmEdge_ConfigureAddProposal(conf, WasmEdge_Proposal_SIMD); BOYCOT! use WASM vector proposal instead!
    // SIMD is proliferating cancer, use https://github.com/WebAssembly/flexible-vectors !

    // --enable-function-reference NOT YET https://github.com/WasmEdge/WasmEdge/pull/2122
    // ⚠️ "Though the user can specify enabling the proposal, the support for the proposal is not implemented yet." :(

    /* The configure and store context to the VM creation can be NULL. */
    WasmEdge_VMContext *VMCxt = WasmEdge_VMCreate(conf, NULL);

    // Get the store context to access registered modules
    WasmEdge_StoreContext *store = WasmEdge_VMGetStoreContext(VMCxt);

    duplicate_wasi_to_env(VMCxt, store,"env");
    duplicate_wasi_to_env(VMCxt, store,"wasi_unstable");

    WasmEdge_Result Res;
    Res = WasmEdge_VMLoadWasmFromBuffer(VMCxt, buffer, buf_size);
    if (!WasmEdge_ResultOK(Res)) printf("⚠️Load WASM failed. Error: %s\n", WasmEdge_ResultGetMessage(Res));
    Res = WasmEdge_VMValidate(VMCxt);
    if (!WasmEdge_ResultOK(Res)) printf("⚠️Validate WASM failed. Error: %s\n", WasmEdge_ResultGetMessage(Res));
    Res = WasmEdge_VMInstantiate(VMCxt);
    if (!WasmEdge_ResultOK(Res)) printf("⚠️Instantiate WASM failed. Error: %s\n", WasmEdge_ResultGetMessage(Res));

    const WasmEdge_ModuleInstanceContext *module_ctx = WasmEdge_VMGetActiveModule(VMCxt);
    store = WasmEdge_VMGetStoreContext(VMCxt);
    auto mem = WasmEdge_StringCreateByCString("memory");
    WasmEdge_MemoryInstanceContext *memory_ctx = WasmEdge_ModuleInstanceFindMemory(module_ctx, mem);
    uint8_t *memo = WasmEdge_MemoryInstanceGetPointer(memory_ctx, 0, 0);
    WasmEdge_StringDelete(mem);
    if (memo)
        wasm_memory = memo;
    else
        warn("⚠️Can't connect wasmedge memory");

    // Try to find which function exists: main, _start, or wasp_main
    WasmEdge_String FuncName = WasmEdge_StringCreateByCString("wasp_main");
    if (not WasmEdge_VMGetFunctionType(VMCxt, FuncName))
        FuncName = WasmEdge_StringCreateByCString("_start");
    if (not WasmEdge_VMGetFunctionType(VMCxt, FuncName))
        FuncName = WasmEdge_StringCreateByCString("main");

    WasmEdge_Value Params[0];
    WasmEdge_Value Returns[1];
    Res = WasmEdge_VMExecute(VMCxt, FuncName, Params, 0, Returns, 1);
    WasmEdge_StringDelete(FuncName);
    if (WasmEdge_ResultOK(Res)) {
        //        int32_t value = WasmEdge_ValueGetI32(Returns[0]);
        if(WasmEdge_ValTypeIsExternRef(Returns[0].Type)) {
            trace("Got externref!");
#if not LINUX
            // Check if it's a struct reference
            if (WasmEdge_ValueIsStructRef(Returns[0])) {
                trace("Got structref - attempting to reflect fields");
                Node result; // turn Box{val=42} into Node(name=Box, kind=struct)[val]=42
                result.kind = structs;

                // Try to extract fields (assuming we know field count or iterate until failure)
                // Most structs have 1-4 fields, try up to 8 as reasonable limit
                for (uint32_t fieldIdx = 0; fieldIdx < 8; fieldIdx++) {
                    WasmEdge_Value fieldVal;
                    WasmEdge_Result res = WasmEdge_ValueGetStructField(Returns[0], fieldIdx, &fieldVal);

                    if (res.Code != WasmEdge_Result_Success.Code) {
                        // No more fields
                        break;
                    }

                    // Convert field value to Node based on type
                    Node fieldNode;
                    if (WasmEdge_ValTypeIsI32(fieldVal.Type)) {
                        fieldNode = Node(WasmEdge_ValueGetI32(fieldVal));
                    } else if (WasmEdge_ValTypeIsI64(fieldVal.Type)) {
                        fieldNode = Node(WasmEdge_ValueGetI64(fieldVal));
                    } else if (WasmEdge_ValTypeIsF32(fieldVal.Type)) {
                        fieldNode = Node(WasmEdge_ValueGetF32(fieldVal));
                    } else if (WasmEdge_ValTypeIsF64(fieldVal.Type)) {
                        fieldNode = Node(WasmEdge_ValueGetF64(fieldVal));
                    } else {
                        // Unknown type, skip
                        continue;
                    }

                    // Add field to result node (use numeric key for now)
                    result[(int)fieldIdx] = fieldNode;
                }

                trace("Reflected struct with "s + formatLong(result.size()) + " fields");
                print(result);
                return result.toSmartPointer();
            } else {
                void *ref = WasmEdge_ValueGetExternRef(Returns[0]);
                trace("Got non-struct externref");
                print(ref);
            }
#else
            void *ref = WasmEdge_ValueGetExternRef(Returns[0]);
            trace("Got externref (struct reflection not available on Linux)");
            print(ref);
#endif
        }
        int64_t value = WasmEdge_ValueGetI64(Returns[0]);
        //        printf("Got result: 0x%llx\n", value);
        auto node1 = smartNode(value);
        //        print(node1);
        auto smartPointer = node1->toSmartPointer();
        return smartPointer;
    } else
        error("WASM EDGE Error: "s+ WasmEdge_ResultGetMessage(Res));
    return -1;
}
