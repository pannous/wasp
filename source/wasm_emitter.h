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
extern Map<String, Signature> functionSignatures;// for funcs AND imports, serialized differently (inline for imports and extra functype section)
extern Map<String, int> functionIndices;
//extern Map<String /*function*/, List<String> /* implicit indices 0,1,2,… */> locals; // access from Angle!
//extern Map<String /* implicit indices 0,1,2,… */, Node* /* compile-time modifiers/values? */> globals; // access from Angle!


Code &emit(Node root_ast, Module *runtime0 = 0, String _start = "main");

// private, could be moved:
Code emitWhile(Node &node, String context);

Code emitIf(Node &node, String context);

Code emitCall(Node &node, String context);

Code emitDeclaration(Node fun, Node &body);

Code emitSetter(Node node, Node &value, String context);

//Code emitExpression(Node *nodes, String context);
Code emitExpression(Node &node, String context/*="main"*/);

Valtype mapTypeToWasm(Node n);