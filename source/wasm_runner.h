#pragma once
//
// Created by me on 23.10.20.
//

typedef unsigned char *bytes;
typedef const char *chars;

extern "C" long run_wasm(bytes buffer, int buf_size);

extern "C" long run_wasm_file(chars wasm_path = "test.wasm");

#ifdef WABT
//#include "ir.h" // Intermediate representation
//int run_wasm(wabt::Module *module);
#undef Module
#endif
