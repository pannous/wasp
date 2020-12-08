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
Code& unsignedLEB128(long n);
Code& signedLEB128(long value);
extern bytes magicModuleHeader;
extern bytes moduleVersion;

class String;
class Nod;
class ExpressionNod;
//class Block;
//class Block : Node{
//
//	Block *begin() const {
//		return children;  CANT specialize/INHERIT:  obviously
//	}
//
//	Block *end() const{
//		return children + length;
//	}
//};

//char start_function=0x00;//unreachable strange convention
extern char unreachable;//=0x00;//unreachable strange convention
//Code &emit(Node root_ast);
Code &emit(Node root_ast, Module* runtime0=0);
class Signature;

extern Map<String, Valtype> return_types;
extern Map<String /*function*/, List<String> /* implicit indices 0,1,2,… */> locals; // access from Angle!
extern Map<String, Signature> functionSignatures;

