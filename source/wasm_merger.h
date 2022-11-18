//
// Created by me on 09.12.20.
//

#ifndef WASP_WASM_MERGER_H
#define WASP_WASM_MERGER_H

#include "Code.h"
#include "wasm_reader.h"

Code &merge_wasm(Module lib, Module main);

Code &merge_binaries(List<Code *> binaries);

Code &merge_binaries(Code &main, Code &lib);

#endif //WASP_WASM_MERGER_H
