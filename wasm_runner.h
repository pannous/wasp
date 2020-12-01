//
// Created by me on 23.10.20.
//

#ifndef WASP_WASM_RUNNER_H
#define WASP_WASM_RUNNER_H

//#include <bh_platform.h>
typedef unsigned char uint8_t;
int run_wasm(chars buffer, int buf_size);

int run_wasm_file(chars wasm_path = "test.wasm");


#endif //WASP_WASM_RUNNER_H
