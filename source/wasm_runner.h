#pragma once
//
// Created by me on 23.10.20.
//

typedef unsigned char *bytes;

long run_wasm(bytes buffer, int buf_size);

long run_wasm(char *wasm_path = "test.wasm");

long run_wasm2(char *wasm_path = "test.wasm");

#ifdef WABT
//#include "ir.h" // Intermediate representation
//int run_wasm(wabt::Module *module);
#undef Module
#endif
