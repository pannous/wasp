#pragma once
#ifndef WASM
#include <cstring>
#endif

#include "wasm_runner.h"
#include "wasm_reader.h"
#include "Map.h"
#include "Code.h"

typedef const unsigned char *wasm_string;// wasm strings start with their length and do NOT end with 0 !! :(
typedef unsigned char *bytes;


class String;

class Signature;


//extern Map<String, Valtype> return_types;// redundant with functionSignatures todo: remove
//todo extern Map<String, Node> variableTypes;
extern Map<String, int> call_indices;// todo functions[].index

//extern List<String> declaredFunctions; only new functions that will get a Code block, no runtime/imports
//extern Map<String /*function*/, List<String> /* implicit indices 0,1,2,… */> locals; // access from Angle!
//extern Map<String /* implicit indices 0,1,2,… */, Node* /* compile-time modifiers/values? */> globals; // access from Angle!



// private, could be moved:
Code emitWhile(Node &node, Function &context);

Code emitIf(Node &node, Function &context);

Code emitCall(Node &node, Function &context);

Code emitDeclaration(Node &fun, Node &body);

Code emitSetter(Node &node, Node &value, Function &context);

//Code emitExpression(Node *node_pointer, Function& context);
Code emitExpression(Node &node, Function &context/*="wasp_main"*/);


[[nodiscard]]
Code emitBlock(Node &node, Function &context);

//Code emitExpression(Node *node)__attribute__((warn_unused_result));
//Code emitExpression(Node *node)__attribute__((error_unused_result));


//Map<int, String>
List<String> collect_locals(Node node, Function &context);


[[nodiscard]]
Code cast(Valtype from, Valtype to);

[[nodiscard]]
Code cast(Node &from, Node &to, Function &context);

[[nodiscard]]
Code cast(Type from, Type to);


[[nodiscard]]
Code emitStringOp(Node &op, Function &context);

//bytes
[[nodiscard]]
Code signedLEB128(int i);

[[nodiscard]]
Code encodeString(chars String);

[[nodiscard]]
Code emitValue(Node &node, Function &context);
// ≠ emitData

// write data to DATA SEGMENT (vs emitValue on stack)
// MAY return i32.const(pointer)
[[nodiscard]]
Code emitData(Node &node, Function &context);

//typedef Code any;
//typedef Bytes any;

// class Code;

// Code flatten (any arr[]) {
// [].concat.apply([], arr);


Valtype fixValtype(Valtype valtype);

Valtype needsUpgrade(Valtype lhs, Valtype rhs, String string);

Type commonType(Type lhs, Type rhs);

//Code emit(String code);//  wasp -> code -> wasm -> data
Code &emit(Node &root_ast);

//#if MY_WASM
////extern "C"
//Code &compile(String code, bool clean = true);// exposed to wasp.js
//#endif

void clearEmitterContext();// BEFORE analyze!

