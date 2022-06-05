#pragma once

#include "Node.h"
#include "Code.h"
#include "wasm_reader.h"

extern bool throwing;

//#include "Map.h"
// The Angle language is a semantic layer on top of Wasp data format
// It is operator driven https://github.com/pannous/angle/wiki/
class Angle {
public:
	static Node &analyze(Node code);
};

void clearContext();
extern List<String> declaredFunctions;
extern Map<String /*function*/, List<String> /* implicit indices 0,1,2,… */> locals;
extern Map<String /*function*/, List<Valtype> /* implicit indices 0,1,2,… */> localTypes;

extern Map<String, Node * /* modifiers/values/init expressions*/> globals; // access from Angle!
extern Map<String /*name*/, Valtype> globalTypes;

enum ParseOptions {
	nix = 0,
	no_main = 1,
	data_only = 2,
	ignore_errors = 4,
};

//List<String> collectOperators(Node &expression);// test/debug only
Node &parse(String code);// wasp -> data  // this is the pure Wasp part
Node &analyze(Node &node, String context = "main");// wasp -> node  // build ast via operators
Node eval(String code);// interpret OR emit :
Node emit(String code);//  wasp -> code -> wasm -> data
Node interpret(String code);// wasp -> code -> data   // interpreter mode vs:
//Node emit(String code, ParseOptions options = nix);//  doesn't work with std::thread compile(emit, String(code.data()))

Node run(String source);// wasp -> code -> wasm() -> data
int runtime_emit(String prog);// wasp -> code + runtime -> wasm (via emit)
//extern Map<String, Signature> functionSignatures;

Node constants(Node n);

bool isFunction(Node &op);

bool isFunction(String op);

// types
extern const Node Double;//.setType(type);
extern const Node Long;//.setType(type);
// int is not a true angle type, just an alias for long.
// todo: but what about interactions with other APIs? add explicit i32 !
// todo: in fact hide most of this under 'number' magic umbrella
extern Map<String, Node> types;
int run_wasm_file(chars file);