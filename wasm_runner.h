//
// Created by me on 23.10.20.
//

#ifndef WASP_WASM_RUNNER_H
#define WASP_WASM_RUNNER_H

//#include <bh_platform.h>
typedef unsigned char* bytes;

int run_wasm(bytes buffer, int buf_size);
int run_wasm(char* wasm_path = "test.wasm");

#ifdef WABT
#include "ir.h" // Intermediate representation
int run_wasm(wabt::Module* module);
#endif
#undef Module;
#endif //WASP_WASM_RUNNER_H
