//
// Created by me on 23.10.20.
//

// https://pengowray.github.io/wasm-ops/

#include <include/wasm_c_api.h>
#include "wasm_runner.h"
#include "wasm_helpers.h"
/*
 * Copyright (C) 2019 Intel Corporation.  All rights reserved.
 * SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
 */

//#define RAW 1
#define RAW 0

#include "wasm_export.h"

#undef raise

#include "bh_read_file.h"
#include "bh_getopt.h"
#include "Paint.h"
#include "wasm.h"
#include "wasm_runtime_common.h"
#include "wasm_runtime.h"
#include "Util.h"

smart_pointer_64 toSmartPointer(Node *n);

//void logi(int x);
uint8_t buffer0[] = {// test program
        0x00, 0x61, 0x73, 0x6d, 0x01, 0x00, 0x00, 0x00, 0x01, 0x14, 0x04, 0x60, 0x01, 0x7f, 0x00, 0x60,
        0x00, 0x00, 0x60, 0x00, 0x01, 0x7f, 0x60, 0x04, 0x7f, 0x7f, 0x7f, 0x7f, 0x01, 0x7d, 0x02, 0x0c,
        0x01, 0x03, 0x65, 0x6e, 0x76, 0x04, 0x6c, 0x6f, 0x67, 0x69, 0x00, 0x00, 0x03, 0x04, 0x03, 0x01,
        0x03, 0x02, 0x07, 0x1b, 0x02, 0x0e, 0x67, 0x65, 0x6e, 0x65, 0x72, 0x61, 0x74, 0x65, 0x5f, 0x66,
        0x6c, 0x6f, 0x61, 0x74, 0x00, 0x02, 0x06, 0x5f, 0x73, 0x74, 0x61, 0x72, 0x74, 0x00, 0x03, 0x0a,
        0x17, 0x03, 0x02, 0x00, 0x0b, 0x08, 0x00, 0x43, 0xc3, 0xf5, 0x48, 0x40, 0x0f, 0x0b, 0x09, 0x00,
        0x41, 0x30, 0x10, 0x00, 0x41, 0x2a, 0x0f, 0x0b, 0x00, 0x40, 0x04, 0x6e, 0x61, 0x6d, 0x65, 0x01,
        0x24, 0x04, 0x00, 0x04, 0x6c, 0x6f, 0x67, 0x69, 0x01, 0x05, 0x65, 0x6d, 0x70, 0x74, 0x79, 0x02,
        0x0e, 0x67, 0x65, 0x6e, 0x65, 0x72, 0x61, 0x74, 0x65, 0x5f, 0x66, 0x6c, 0x6f, 0x61, 0x74, 0x03,
        0x04, 0x6d, 0x61, 0x69, 0x6e, 0x02, 0x13, 0x04, 0x00, 0x01, 0x00, 0x00, 0x01, 0x00, 0x02, 0x04,
        0x00, 0x00, 0x01, 0x00, 0x02, 0x00, 0x03, 0x00, 0x03, 0x00};
/*
 		run_wasm(buffer0, sizeof(buffer0)); // SHOULD PRINT:
Native finished calling wasm function generate_float(), returned a float value: 3.140000f
CALLING env.logi(i:48 )
48
RETURN VALUE: 42
*/


//int32_t is just int
//int intToStr(int x, char *str, int str_len, int digit) {}// wavm can call char* ! no JS restrictions!!


int square_x(wasm_exec_env_t exec_env, int x) {
    return x * x;
}

// f:float F:double i:int I:int64 no comma in arg list pow :: (FF)F

// https://github.com/bytecodealliance/wasm-micro-runtime/blob/main/doc/export_native_api.md
/*
'i': i32
'I': i64
'f': f32
'F': f64
'*': a buffer address in WASM application
'~': the byte length of WASM buffer as referred by preceding argument "*". It must follow after '*'
'$': a string in WASM application
*/
static NativeSymbol native_symbols[] =
        {        // WAVM can call f(float) f(char*)! No JS restrictions!!
                {
                        "get_pow",            // the name of WASM function name
                        (void *) get_pow,            // the native function pointer
                        "(ii)i",            // the function prototype signature, avoid to use i32
                        NULL,                // attachment is NULL
                        false
                },
//				{"square", (void *) squarl, "(I)I", NULL, false},
//				{"square", (void *) squari, "(i)i", NULL, false},
                {"square", (void *) square_x, "(i)i", NULL, false},
//				{"square", (void *) square_x, "(i)i", NULL, true},
                {"√", (void *) sqrt1, "(i)i", NULL, false},
                {"fd_write", (void *) fd_write_host, "(iiii)i", NULL, false},
                {"puti", (void *) puti, "(i)", NULL, false},
                {"putf", (void *) putf, "(f)", NULL, false},
                {"puts", (void *) puts, "(i)", NULL, false},// pointer
                {"pow", (void *) powd, "(FF)F", NULL, false},
                {"powd", (void *) powd, "(FF)F", NULL, false},
                {"powf", (void *) powf, "(ff)f", NULL, false},
                {"powi", (void *) powi, "(ii)I", NULL, false},
//				{       "powl",              (void *) powi,          "(II)I", NULL, false},
                {"__cxa_begin_catch", (void *) powi, "(*)i", NULL, false},
#ifdef SDL
                {       "init_graphics",     (void *) init_graphics, "()I",   NULL, false},
#endif
        };

//wasm_trap_t *
//wasm_func_callback_t hello_callback() {
//wasm_func_callback_t hello_callback(const wasm_val_vec_t args[], wasm_val_vec_t results[]) {
wasm_trap_t *hello_callback(const wasm_val_vec_t args[], wasm_val_vec_t results[]) {
    printf("Calling back...\n");
    printf("> Hello World!\n");
    return NULL;
}


void logi2(wasm_exec_env_t exec_env, int x) {
    printf("calling into native function: %s\n", __FUNCTION__);
    printf("%d\n", x);
}
//void logi(int x){
//	printf("%d", x);
//}


int fail(chars format, chars val) {
    printf(format, val);
//	throw "FAIL";
    return -1;
}

int fail(chars string) {
    printf("FAIL %s", string);
    return -1;
}

typedef struct wasm_byte_vec_t2 {
    size_t size;
    wasm_byte_t *data;// ptr_or_none
    size_t num_elems;
    size_t size_of_elem;
} wasm_byte_vec_t2;


void init_vm(RuntimeInitArgs init_args, NativeSymbol *native_symbols, int symbol_count) {
    static bool done;
    if (done) {
//		printf("\nWARNING: init_vm was already called before");
//		printf(" CAN'T LOAD NEW native_symbols\n");// 2021/9 why not?
//		return;// ONLY ONCE!!
    } else {
        printf("INITIALIZING WASM VM\n");
        done = true;
    }
    init_args.mem_alloc_type = Alloc_With_System_Allocator;// works with runtime!

//	static char global_heap_buf[512 * 1024];// what is this?
    //	init_args.mem_alloc_type = Alloc_With_Pool;
//	init_args.mem_alloc_option.pool.heap_buf = global_heap_buf;
//	init_args.mem_alloc_option.pool.heap_size = sizeof(global_heap_buf);

    // Native symbols need below registration phase
    init_args.n_native_symbols = symbol_count / sizeof(NativeSymbol);
    init_args.native_module_name = "env";
    init_args.native_symbols = native_symbols;
//	init_args.native_raw_functions = RAW; // false;// true;// use WASM_ENABLE_RAW_NATIVES 1

    if (!wasm_runtime_full_init(&init_args))
        fail("Init runtime environment FAILED.\n");
}

int64 run_wasm(uint8 *buffer, uint32 buf_size, RuntimeInitArgs *init_args0 = 0) {
    try {
        if (not init_args0) {
            static RuntimeInitArgs init_args;
            memset(&init_args, 0, sizeof(RuntimeInitArgs));
            init_vm(init_args, native_symbols, sizeof(native_symbols));// DANGER sizeof only works for []
        } else {
            warn("VM already init_vm'ed!");
        }

        char error_buf[128];

        wasm_module_t module = NULL;
        wasm_module_inst_t module_inst = NULL;
        wasm_exec_env_t exec_env = NULL;
        uint32 stack_size = 8092, heap_size = 8092;
//		wasm_function_inst_t func2 = NULL;
//		char *native_buffer = NULL;
        uint32_t wasm_buffer = 0;


        module = wasm_runtime_load(buffer, buf_size, error_buf, sizeof(error_buf));
        if (!module) {
            return fail("Load wasm module FAILED. error: %s\n", error_buf);
        }
        module_inst = wasm_runtime_instantiate(module, stack_size, heap_size, error_buf, sizeof(error_buf));
        if (!module_inst) {
            return fail("Instantiate wasm module FAILED. error: %s\n", error_buf);
        }
        exec_env = wasm_runtime_create_exec_env(module_inst, stack_size);
        if (!exec_env) {
            return fail("Create wasm execution environment FAILED.\n");
        }
//        memory_instantiate();
//        WASMMemory *wasmMemory = ((WASMModule *)module)->memories;
        WASMMemoryInstance **memories = ((WASMModuleInstance *) module_inst)->memories;
        if (memories) {
            WASMMemoryInstance *memory_ = memories[0];
            wasm_memory = memory_->memory_data;
            memory = reinterpret_cast<unsigned int *>(memory_->memory_data);
        } //else warn("wasm module has no memory (OK)");

        uint32 argv[0];
        argv[0] = 10000;
//	memcpy(&argv[1], &arg_d, sizeof(arg_d));

//		wasm_function_inst_t func = NULL;
//		if (!(func = wasm_runtime_lookup_function(module_inst, "generate_float", NULL))) {
//			fail("The generate_float wasm function is not found.\n");
////			return -1;
//		}
//
//		// pass 4 elements for function arguments
//		if (!wasm_runtime_call_wasm(exec_env, func, 4, argv)) {
//			fail("call wasm function generate_float FAILED. %s\n", wasm_runtime_get_exception(module_inst));
////			return
//		} else{
//		float ret_val = *(float *) argv;// oh reuse args as return? that's evil ;) TODO
//		printf("Native finished calling wasm function generate_float(), returned a float value: %ff\n", ret_val);
//		}


        /////////////////////////
        // MAIN / _START
        /////////////////////////

        wasm_function_inst_t _start;
        if (!(_start = wasm_runtime_lookup_function(module_inst, "wasp_main", NULL))) {
            if (!(_start = wasm_runtime_lookup_function(module_inst, "_start", NULL))) {
                return fail("The main function is not found.\n");
            }
        }
        if (!wasm_runtime_call_wasm(exec_env, _start, 0, argv)) {
            return fail("call wasm function main/_start FAILED. %s\n", wasm_runtime_get_exception(module_inst));
        }
        uint64 result = *(uint64 *) &argv;
        printf("RETURN VALUE: %llx\n", result);

        // Next we will pass a buffer to the WASM function
//		uint32 argv2[4];
//
//		// must allocate buffer from wasm instance memory space (never use pointer from host runtime)
//		wasm_buffer = wasm_runtime_module_malloc(module_inst, 100, (void **) &native_buffer);
//
//		*(float *) argv2 = ret_val;   // the first argument
//		argv2[1] = wasm_buffer;     // the second argument is the wasm buffer address
//		argv2[2] = 100;             //  the third argument is the wasm buffer size
//		argv2[3] = 3;               //  the last argument is the digits after decimal point for converting float to string
//
//		if (!(func2 = wasm_runtime_lookup_function(module_inst, "float_to_string", NULL))) {
//			return fail("The wasm function float_to_string wasm function is not found.\n");
//		}
//
//		if (wasm_runtime_call_wasm(exec_env, func2, 4, argv2)) {
//			printf("Native finished calling wasm function: float_to_string, returned a formatted string: %s\n",
//			       native_buffer);
//		} else {
//			return fail("call wasm function float_to_string FAILED. error: %s\n",
//			            wasm_runtime_get_exception(module_inst));
//		}

        Node *node = smartNode((smart_pointer_64) result);// copy result string!
        if (node) result = toSmartPointer(node);// just return 'pointer' to Node for now
        check_silent(smartNode(result) == node or not node);// try to reconstruct

//		cleanup:
//		done = false;
        if (exec_env) wasm_runtime_destroy_exec_env(exec_env);
        if (module_inst) {
            if (wasm_buffer) wasm_runtime_module_free(module_inst, wasm_buffer);
            wasm_runtime_deinstantiate(module_inst);
        }
        if (module) wasm_runtime_unload(module);
        wasm_runtime_destroy();
        return result;

    } catch (chars err) {
        printf("\n⚠️ERROR\n");
        printf("%s", err);
    }
    return 1;
}


extern "C" int64 run_wasm_file2(chars wasm_path) {
    try {
        RuntimeInitArgs init_args;
        memset(&init_args, 0, sizeof(RuntimeInitArgs));
        init_vm(init_args, native_symbols, sizeof(native_symbols));// DANGER sizeof only works for []
        uint32 buf_size;
        uint8 *buffer;
        buffer = (uint8 *) bh_read_file_to_buffer(wasm_path, &buf_size);
        if (!buffer) {
            fail("Open wasm app file [%s] FAILED.\n", wasm_path);
            exit(-1);
        }
        return run_wasm(buffer, buf_size, &init_args);


    } catch (chars err) {
        printf("\n⚠️ERROR\n");
        printf("%s", err);
    }
    return -1;
}

extern "C" int64 run_wasm(bytes buffer, int buf_size) {
    return run_wasm(reinterpret_cast<uint8 *>(buffer), buf_size, 0);
}
