//
// Created by me on 23.10.20.
//

#include <include/wasm_c_api.h>
#include "wasm_runner.h"
/*
 * Copyright (C) 2019 Intel Corporation.  All rights reserved.
 * SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
 */

#include "wasm_export.h"
#include "bh_read_file.h"
#include "bh_getopt.h"
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


//int32_t is just int
int intToStr(int x, char *str, int str_len, int digit) {}// wavm can call char* ! no JS restrictions!!

int32_t get_pow(wasm_exec_env_t exec_env, int32_t x, int32_t y) {
	printf("exec_env %s", exec_env);
	return x*y;
}
long get_pow2(long x, long y) {
	return x*y;
}
wasm_trap_t* hello_callback(const wasm_val_t args[], wasm_val_t results[]) {
	printf("Calling back...\n");
	printf("> Hello World!\n");
	return NULL;
}
//void logi2(int x);
void logi2(wasm_exec_env_t exec_env,int x){
	printf ("calling into native function: %s\n", __FUNCTION__);
	printf("%d\n", x);
}
//void logi(int x){
//	printf("%d", x);
//}


int fail(const char *format, const char *val) {
	printf(format, val);
	throw "FAIL";
	return -1;
}

int fail(const char *string) {
	printf("FAIL %s", string);
	return -1;
}
typedef struct wasm_byte_vec_t2 {
    size_t size;
    wasm_byte_t* data;// ptr_or_none
    size_t num_elems;
    size_t size_of_elem;
  } wasm_byte_vec_t2;

int main2(int argc, char *argv_main[]) {
// ALTERNATIVE mechanism:
// running under mode 0 , yeah zero = old shit
	wasm_byte_vec_t binary={170, (wasm_byte_t*)buffer0,170,1};
	wasm_engine_t* engine = wasm_engine_new();
	wasm_store_t* store = wasm_store_new(engine);
	wasm_functype_t* hello_type = wasm_functype_new_0_0();
	wasm_func_t* hello_func = wasm_func_new(store, hello_type, hello_callback);
	const wasm_extern_t* imports[] = { wasm_func_as_extern(hello_func) };
	wasm_module_t* module2 = wasm_module_new(store, &binary);
	wasm_instance_t* instance = wasm_instance_new(store, module2, imports, NULL);
	wasm_extern_vec_t exports;
	wasm_instance_exports(instance, &exports);
	const wasm_func_t* run_func = wasm_extern_as_func(exports.data[0]);
	wasm_func_call(run_func, NULL, NULL);

}
int main(int argc, char *argv_main[]) {
	try {

		char *wasm_path = "test.wasm";

		static char global_heap_buf[512 * 1024];
		const uint8 *buffer;
		char error_buf[128];

		wasm_module_t module = NULL;
		wasm_module_inst_t module_inst = NULL;
		wasm_exec_env_t exec_env = NULL;
		uint32 buf_size, stack_size = 8092, heap_size = 8092;
		wasm_function_inst_t func2 = NULL;
		char *native_buffer = NULL;
		uint32_t wasm_buffer = 0;

		RuntimeInitArgs init_args;
		memset(&init_args, 0, sizeof(RuntimeInitArgs));

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
				{        // WAVM can call f(char*)! No JS restrictions!!
						{
								"get_pow",            // the name of WASM function name
								(void *) get_pow,            // the native function pointer
								"(ii)i",            // the function prototype signature, avoid to use i32
								NULL                // attachment is NULL
						},
						{
								"get_pow2",            // the name of WASM function name
								(void *) get_pow2,            // the native function pointer
								"(II)I",            // the function prototype signature, avoid to use i32
								NULL                // attachment is NULL
						},
						{
								"logi",            // the name of WASM function name
								(void *) logi2,            // the native function pointer
								"(i)",            // the function prototype signature, avoid to use i32
								NULL                // attachment is NULL
						}
				};

		init_args.mem_alloc_type = Alloc_With_Pool;
		init_args.mem_alloc_option.pool.heap_buf = global_heap_buf;
		init_args.mem_alloc_option.pool.heap_size = sizeof(global_heap_buf);

		// Native symbols need below registration phase
		init_args.n_native_symbols = sizeof(native_symbols) / sizeof(NativeSymbol);
		init_args.native_module_name = "env";
		init_args.native_symbols = native_symbols;

		if (!wasm_runtime_full_init(&init_args))
			return fail("Init runtime environment FAILED.\n");

		buffer = buffer0;
		buf_size = sizeof(buffer0); // ONLY WORKS ON THE SPOT!
		buffer = (uint8 *) bh_read_file_to_buffer(wasm_path, &buf_size);
		if (!buffer) {
			return fail("Open wasm app file [%s] FAILED.\n", wasm_path);
		}
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

		uint32 argv[0];
		argv[0] = 10000;
//	memcpy(&argv[1], &arg_d, sizeof(arg_d));

		wasm_function_inst_t func = NULL;
		if (!(func = wasm_runtime_lookup_function(module_inst, "generate_float", NULL))) {
			return fail("The generate_float wasm function is not found.\n");
		}

		// pass 4 elements for function arguments
		if (!wasm_runtime_call_wasm(exec_env, func, 4, argv)) {
			return fail("call wasm function generate_float FAILED. %s\n", wasm_runtime_get_exception(module_inst));
		}

		float ret_val = *(float *) argv;// oh reuse args as return? that's evil ;) TODO
		printf("Native finished calling wasm function generate_float(), returned a float value: %ff\n", ret_val);

		/////////////////////////
		// MAIN / _START
		/////////////////////////

		wasm_function_inst_t _start;
		if (!(_start = wasm_runtime_lookup_function(module_inst, "main", NULL))) {
			if (!(_start = wasm_runtime_lookup_function(module_inst, "_start", NULL))) {
				return fail("The main function is not found.\n");
			}
		}
		if (!wasm_runtime_call_wasm(exec_env, _start, 0, argv)) {
			return fail("call wasm function main/_start FAILED. %s\n", wasm_runtime_get_exception(module_inst));
		}
		printf("RETURN VALUE: %d\n", argv[0]);

		// Next we will pass a buffer to the WASM function
		uint32 argv2[4];

		// must allocate buffer from wasm instance memory space (never use pointer from host runtime)
		wasm_buffer = wasm_runtime_module_malloc(module_inst, 100, (void **) &native_buffer);

		*(float *) argv2 = ret_val;   // the first argument
		argv2[1] = wasm_buffer;     // the second argument is the wasm buffer address
		argv2[2] = 100;             //  the third argument is the wasm buffer size
		argv2[3] = 3;               //  the last argument is the digits after decimal point for converting float to string

		if (!(func2 = wasm_runtime_lookup_function(module_inst, "float_to_string", NULL))) {
			return fail("The wasm function float_to_string wasm function is not found.\n");
		}

		if (wasm_runtime_call_wasm(exec_env, func2, 4, argv2)) {
			printf("Native finished calling wasm function: float_to_string, returned a formatted string: %s\n",
			       native_buffer);
		} else {
			return fail("call wasm function float_to_string FAILED. error: %s\n",
			            wasm_runtime_get_exception(module_inst));
		}

		cleanup:
		if (exec_env) wasm_runtime_destroy_exec_env(exec_env);
		if (module_inst) {
			if (wasm_buffer) wasm_runtime_module_free(module_inst, wasm_buffer);
			wasm_runtime_deinstantiate(module_inst);
		}
		if (module) wasm_runtime_unload(module);
		wasm_runtime_destroy();
		return 0;
	} catch (const char *err) {
		printf("\nERROR\n");
		printf("%s", err);
	}
}
