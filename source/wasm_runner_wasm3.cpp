//
// Created by me on 17.11.20.
//

#include "wasm_runner.h"
#include "wasm3.h"
#include <cstdio>
#include "wasm3_cpp.h"
// 2022-12: last release on Jun 2, 2021
#include "wasm_helpers.h"
#include "String.h"
#include "Paint.h"
#include <stddef.h>
#include <stdint.h>

//extern int ext_memcpy0(void*, const void*, size_t);

int sum(int a, int b) {
	return a + b;
}

void *ext_memcpy0(void *dst, const void *arg, int32_t size) {
    memcpy1((bytes) dst, (bytes) arg, (size_t) size);
	return dst;
}

unsigned char test_prog_wasm[] = {
		0x00, 0x61, 0x73, 0x6d, 0x01, 0x00, 0x00, 0x00, 0x01, 0x15, 0x04, 0x60,
		0x02, 0x7f, 0x7f, 0x01, 0x7f, 0x60, 0x00, 0x00, 0x60, 0x03, 0x7f, 0x7f,
		0x7f, 0x01, 0x7f, 0x60, 0x00, 0x01, 0x7e, 0x02, 0x1c, 0x02, 0x03, 0x65,
		0x6e, 0x76, 0x0a, 0x65, 0x78, 0x74, 0x5f, 0x6d, 0x65, 0x6d, 0x63, 0x70,
		0x79, 0x00, 0x02, 0x03, 0x65, 0x6e, 0x76, 0x03, 0x73, 0x75, 0x6d, 0x00,
		0x00, 0x03, 0x04, 0x03, 0x01, 0x03, 0x00, 0x05, 0x06, 0x01, 0x01, 0x80,
		0x02, 0x80, 0x02, 0x06, 0x09, 0x01, 0x7f, 0x01, 0x41, 0x80, 0x8c, 0xc0,
		0x02, 0x0b, 0x07, 0x28, 0x04, 0x06, 0x6d, 0x65, 0x6d, 0x6f, 0x72, 0x79,
		0x02, 0x00, 0x04, 0x74, 0x65, 0x73, 0x74, 0x00, 0x04, 0x0b, 0x74, 0x65,
		0x73, 0x74, 0x5f, 0x6d, 0x65, 0x6d, 0x63, 0x70, 0x79, 0x00, 0x03, 0x06,
		0x5f, 0x73, 0x74, 0x61, 0x72, 0x74, 0x00, 0x02, 0x0a, 0x71, 0x03, 0x03,
		0x00, 0x01, 0x0b, 0x59, 0x02, 0x01, 0x7f, 0x01, 0x7e, 0x23, 0x00, 0x41,
		0x10, 0x6b, 0x22, 0x00, 0x24, 0x00, 0x20, 0x00, 0x42, 0x00, 0x37, 0x03,
		0x08, 0x20, 0x00, 0x41, 0xe7, 0x8a, 0x8d, 0x09, 0x36, 0x02, 0x04, 0x20,
		0x00, 0x41, 0xef, 0x9b, 0xaf, 0xcd, 0x78, 0x36, 0x02, 0x00, 0x20, 0x00,
		0x41, 0x08, 0x6a, 0x20, 0x00, 0x41, 0x04, 0x6a, 0x41, 0x04, 0x10, 0x00,
		0x1a, 0x20, 0x00, 0x41, 0x08, 0x6a, 0x41, 0x04, 0x72, 0x20, 0x00, 0x41,
		0x04, 0x10, 0x00, 0x1a, 0x20, 0x00, 0x29, 0x03, 0x08, 0x21, 0x01, 0x20,
		0x00, 0x41, 0x10, 0x6a, 0x24, 0x00, 0x20, 0x01, 0x0b, 0x11, 0x00, 0x20,
		0x00, 0x20, 0x01, 0x6a, 0x20, 0x00, 0x20, 0x01, 0x6b, 0x10, 0x01, 0x41,
		0x02, 0x6d, 0x0b, 0x0b, 0x0a, 0x01, 0x00, 0x41, 0x80, 0x0c, 0x0b, 0x03,
		0xa0, 0x06, 0x50
};
unsigned int test_prog_wasm_len = 255;

#define WASM_EXPORT __attribute__((used)) __attribute__((visibility ("default")))

int WASM_EXPORT test(int32_t arg1, int32_t arg2) {
	int x = arg1 + arg2;
	int y = arg1 - arg2;
	return sum(x, y) / 2;
}

int64_t WASM_EXPORT test_memcpy0(void) {
	int64_t x = 0;
	int32_t low = 0x01234567;
	int32_t high = 0x89abcdef;
	ext_memcpy0(&x, &low, 4);
	ext_memcpy0(((uint8_t *) &x) + 4, &high, 4);
	return x;
}


int64 run_wasm3(const uint8_t *prog, int len) {
//	std::cout << "Loading WebAssembly..." << std::endl;
    try {
        wasm3::environment env;
        wasm3::module mod = env.parse_module(prog, len);
        int stack_size_bytes = 1024;//*1024; //   BYTES !
        wasm3::runtime runtime = env.new_runtime(
                stack_size_bytes);// OK, WASM3 error: [trap] stack overflow == out of memory
        runtime.load(mod);
        wasm_memory = runtime.getMemory(); // be careful not to free it before result is handled/copied

        mod.link_optional<sum>("*", "sum");
        mod.link_optional<ext_memcpy0>("*", "ext_memcpy0");
        mod.link_optional<sqrt1>("*", "√");
        mod.link_optional<square>("*", "square");
        mod.link_optional<powd>("*", "powd");// todo builtin!
        mod.link_optional<logf>("*", "log");// logarithm, not putf !
        mod.link_optional<powd>("*", "pow");
        mod.link_optional<powi>("*", "powi");
        mod.link_optional<putf>("*", "logf");// danger logf is cuda function!
        mod.link_optional<puti>("*", "logi");// danger logf is cuda function!
        mod.link_optional<puts>("*", "puts");// todo: replace FAKE DUMMY with adhoc circle implementation
        mod.link_optional<puts>("*", "puts");
        mod.link_optional<puts>("*", "print");
//		mod.link_optional<putp>("*", "puts");// added m3_type_to_sig in wasm3_cpp.h !
        mod.link_optional<panic>("*", "panic");
        mod.link_optional<raise>("*", "raise");
#ifdef SDL
        mod.link_optional<init_graphics>("*", "init_graphics");// returns pointer to surface
        mod.link_optional<requestAnimationFrame>("*", "requestAnimationFrame");// returns pointer to surface
#endif
//		wasm3::function main_fn = runtime.find_function("_start");
        wasm3::function main_fn = runtime.find_function("wasp_main");
#if MULTI_VALUE
        //		auto res = main_fn.call<int64,int>();
#else
        auto res = main_fn.call<int64>();
#endif
        return res;

//		wasm3::function test_fn = runtime.find_function("test");
//		auto test_res = test_fn.call<int>(15, 12);
//		std::cout << "result is: " << test_res << std::endl;
    }

	catch (wasm3::error &e) {
		std::cerr << "WASM3 error: " << e.what() << std::endl;
		return -1;
	}
}

extern "C" int64 run_wasm(bytes wasm_bytes, int len) {
//	test_wasm3(test_prog_wasm, test_prog_wasm_len);
    return run_wasm3((const uint8_t *) wasm_bytes, len);
}

//
//#ifdef WASM3
//Code readWasmW3(char const *file) {
//	result = ParseWatModule(lexer, &module, &errors, &parse_wast_options);
//
//	if (Succeeded(result) && validate_wasm) {
//		ValidateOptions options(wabt_features);
//		result = ValidateModule(module, &errors, options);
//	}
//
//	if (Succeeded(result)) {
//		MemoryStream stream(s_log_stream);
//		write_binary_options.features = wabt_features;
//		result = WriteBinaryModule(&stream, module, write_binary_options);
//
//	IM3Environment environment=m3_NewEnvironment();
//	IM3Module module;
//	M3Result result = m3_ParseModule(environment, &module, buffer, sz);
//	printf("parsed: %s\n", result);
//	printf("Module: %s\n", module->name);
////	M3Result  Module_AddFunction  (IM3Module io_module, u32 i_typeIndex, IM3ImportInfo i_importInfo)
//
//
////	read(buffer, sz);
//}
//#endif
