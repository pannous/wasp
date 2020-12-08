#pragma once
#ifndef WASM
#include <cstring>
#endif
#include "wasm_runner.h"
#include "Map.h"
#include "Code.h"
#include "wasm_reader.h"

typedef const unsigned char* wasm_string;// wasm strings start with their length and do NOT end with 0 !! :(
typedef unsigned char* bytes;

extern bytes magicModuleHeader;
extern bytes moduleVersion;

class String;

class Signature;

extern Map<String, Valtype> return_types;// redundant with functionSignatures todo: remove
extern Map<String, Signature> functionSignatures;// for funcs AND imports, serialized differently (inline for imports and extra functype section)
extern Map<String /*function*/, List<String> /* implicit indices 0,1,2,… */> locals; // access from Angle!

Code &emit(Node root_ast, Module* runtime0=0, String _start="main");

