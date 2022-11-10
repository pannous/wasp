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
extern Map<String, int> functionIndices;// todo functions[].index

Signature &getSignature(String name);

//extern List<String> declaredFunctions; only new functions that will get a Code block, no runtime/imports
//extern Map<String /*function*/, List<String> /* implicit indices 0,1,2,… */> locals; // access from Angle!
//extern Map<String /* implicit indices 0,1,2,… */, Node* /* compile-time modifiers/values? */> globals; // access from Angle!



// private, could be moved:
Code emitWhile(Node &node, String context);

Code emitIf(Node &node, String context);

Code emitCall(Node &node, String context);

Code emitDeclaration(Node fun, Node &body);

Code emitSetter(Node &node, Node &value, String context);

//Code emitExpression(Node *nodes, String context);
Code emitExpression(Node &node, String context/*="main"*/);


[[nodiscard]]
Code emitBlock(Node &node, String functionContext);

//Code emitExpression(Node *node)__attribute__((warn_unused_result));
//Code emitExpression(Node *node)__attribute__((error_unused_result));


//Map<int, String>
List<String> collect_locals(Node node, String context);


[[nodiscard]]
Code cast(Valtype from, Valtype to);

[[nodiscard]]
Code cast(Node &from, Node &to);

[[nodiscard]]
Code emitStringOp(Node op, String context);

//bytes
[[nodiscard]]
Code signedLEB128(int i);

[[nodiscard]]
Code encodeString(char *String);

[[nodiscard]]
Code emitValue(Node &node, String context);
// ≠ emitData

// write data to DATA SEGMENT (vs emitValue on stack)
// MAY return i32.const(pointer)
[[nodiscard]]
Code emitData(Node node, String context);

//typedef Code any;
//typedef Bytes any;

// class Code;

// Code flatten (any arr[]) {
// [].concat.apply([], arr);


Valtype fixValtype(Valtype valtype);

Valtype needsUpgrade(Valtype lhs, Valtype rhs, String string);

//Code emit(String code);//  wasp -> code -> wasm -> data
Code &emit(Node &root_ast, Module *runtime0 = 0, String _start = "main");

//extern "C"
Code &compile(String code);// exposed to wasp.js

void clearEmitterContext();// BEFORE analyze!
