//
// Created by me on 09.12.20.
//

#ifndef WASP_WASM_MERGER_H
#define WASP_WASM_MERGER_H

#include "Code.h"
#include "wasm_reader.h"

Code merge_wasm(Module lib, Module main);

#endif //WASP_WASM_MERGER_H
