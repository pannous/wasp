//
// Created by me on 27.11.20.
//

#ifndef WASP_WASM_READER_WABT_H
#define WASP_WASM_READER_WABT_H

#include "ir.h"
#include "wasm_emitter.h"

//using wabt::Module;

wabt::Module *read_wasm(bytes data, int size, const char *filename);

wabt::Module *readWasm(char const *file);

wabt::Module *refactor_wasm(wabt::Module *module, chars old_name, chars new_name);

void save_wasm(wabt::Module *module, chars file = "out.wasm");

void remove_function(wabt::Module *module, chars fun);


#endif //WASP_WASM_READER_WABT_H
