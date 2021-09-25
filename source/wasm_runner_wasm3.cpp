//
// Created by me on 17.11.20.
//

#include "wasm_runner.h"
#include "wasm3.h"
#include <cstdio>
#include "wasm3_cpp.h"
#include "wasm_helpers.h"
#include "String.h"
#include "Paint.h"
#include <stddef.h>
#include <stdint.h>

//extern int ext_memcpy0(void*, const void*, size_t);

long powi(int a, int b) {
	return powl(a, b);
}

int sum(int a, int b) {
	return a + b;
}

void *ext_memcpy0(void *dst, const void *arg, int32_t size) {
	memcpy0((bytes) dst, (bytes) arg, (size_t) size);
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

double pow2(double x, double y) {// why this crutch? maybe conflicting pow's in all those xyz_math.h
	return pow(x, y);
}

// todo: remove this useless test function
int square2(int a) {
	return a * a;
}

int test_wasm3(const uint8_t *prog, int len) {
//	std::cout << "Loading WebAssembly..." << std::endl;
	try {
		wasm3::environment env;
		wasm3::module mod = env.parse_module(prog, len);
		wasm3::runtime runtime = env.new_runtime(1024);
		runtime.load(mod);

		mod.link_optional<sum>("*", "sum");
		mod.link_optional<ext_memcpy0>("*", "ext_memcpy0");
		mod.link_optional<sqrt1>("*", "√");
		mod.link_optional<square2>("*", "square");
		mod.link_optional<pow2>("*", "pow");
		mod.link_optional<logf32>("*", "logf");// danger logf is cuda function!
		mod.link_optional<logi>("*", "logi");// danger logf is cuda function!
		mod.link_optional<logs>("*", "logs");// todo: replace FAKE DUMMY with adhoc circle implementation
		mod.link_optional<logs>("*", "puts");
//		mod.link_optional<logp>("*", "logs");// added m3_type_to_sig in wasm3_cpp.h !
		mod.link_optional<panic>("*", "panic");
		mod.link_optional<raise>("*", "raise");
		mod.link_optional<init_graphics>("*", "init_graphics");// returns pointer to surface
		mod.link_optional<init_graphics>("*", "requestAnimationFrame");// returns pointer to surface

//		wasm3::function main_fn = runtime.find_function("_start");
		wasm3::function main_fn = runtime.find_function("main");
		auto res = main_fn.call<int>();
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

int run_wasm(bytes wasm_bytes, int len) {
//	test_wasm3(test_prog_wasm, test_prog_wasm_len);
	return test_wasm3((const uint8_t *) wasm_bytes, len);
}


#ifdef WASM3
Code readWasmW3(char const *file) {
	result = ParseWatModule(lexer.get(), &module, &errors, &parse_wast_options);

	if (Succeeded(result) && validate_wasm) {
		ValidateOptions options(wabt_features);
		result = ValidateModule(module.get(), &errors, options);
	}

	if (Succeeded(result)) {
		MemoryStream stream(s_log_stream.get());
		write_binary_options.features = wabt_features;
		result = WriteBinaryModule(&stream, module.get(), write_binary_options);

	IM3Environment environment=m3_NewEnvironment();
	IM3Module module;
	M3Result result = m3_ParseModule(environment, &module, buffer, sz);
	printf("parsed: %s\n", result);
	printf("Module: %s\n", module->name);
//	M3Result  Module_AddFunction  (IM3Module io_module, u32 i_typeIndex, IM3ImportInfo i_importInfo)


//	read(buffer, sz);
}
#endif