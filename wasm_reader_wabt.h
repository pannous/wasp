//
// Created by me on 27.11.20.
//

#ifndef WASP_WASM_READER_WABT_H
#define WASP_WASM_READER_WABT_H
#include "ir.h"
#include "wasm_emitter.h"
using wabt::Module;
Module *read_wasm(bytes data, int size, const char *filename);
Module* readWasm(char const *file);
Module* refactor_wasm(Module *module, chars old_name, chars new_name);
void save_wasm(Module *module, chars file="out.wasm");
Module* merge_wasm(Module *base, Module *module);
void remove_function(Module *module, chars fun);


#endif //WASP_WASM_READER_WABT_H
