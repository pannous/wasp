#pragma once

#include "Code.h"
#include "List.h"
#include "Context.h"


Module &read_wasm(String file);

Module &read_wasm(bytes buffer, int size);

Code &read_code(chars file);

extern Module *module;

extern Map<int64, Module *> module_cache;

Module &loadRuntime();
