#pragma once

#include "Node.h"
#include "Code.h"

//#include "Map.h"
// The Angle language is a semantic layer on top of Wasp data format
// It is operator driven https://github.com/pannous/angle/wiki/
class Angle {
public:
	static Node &analyze(Node code);
};

extern List<String> declaredFunctions;
extern Map<String /*function*/, List<String> /* implicit indices 0,1,2,… */> locals;
extern Map<String /*function*/, List<Valtype> /* implicit indices 0,1,2,… */> localTypes;

extern Map<String /* implicit indices 0,1,2,… */, Node * /* compile-time modifiers/values? */> globals; // access from Angle!

//List<String> collectOperators(Node &expression);// test/debug only
Node parse(String code);// wasp -> data  // this is the pure Wasp part
Node analyze(Node data);// wasp -> code  // build ast via operators
Node eval(String code);// wasp -> code -> data   // interpreter mode vs:
Node emit(String code);//  wasp -> code -> wasm  // to debug currently same as:
Node run(String source);// wasp -> code -> wasm() -> data
//extern Map<String, Signature> functionSignatures;

Node constants(Node n);

bool isFunction(Node &op);

bool isFunction(String op);