//
// Created by me on 23.10.20.
//

#ifndef WASP_WASM_RUNNER_H
#define WASP_WASM_RUNNER_H

//#include <bh_platform.h>
typedef unsigned char* bytes;

int run_wasm(bytes buffer, int buf_size);
int run_wasm_file(char* wasm_path = "test.wasm");


#endif //WASP_WASM_RUNNER_H
