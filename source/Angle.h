#pragma once

#include "Node.h"
#include "Code.h"
#include "wasm_reader.h"

extern bool throwing;
extern List<Module *> libraries;// merged:
//extern Map<String, Signature> functionSignatures;// for funcs AND imports, serialized differently (inline for imports and extra functype section)
//extern Map<String, Function> library_functions;// use as import (implicit and explicit)
extern Map<String, Function> functions;// for funcs AND imports, serialized differently (inline for imports and extra functype section)

//#include "Map.h"
// The Angle language is a semantic layer on top of Wasp data format
// It is operator driven https://github.com/pannous/angle/wiki/
class Angle {
public:
    static Node &analyze(Node code);
};

void clearAnalyzerContext();

extern Module *module;// todo: put all these in <<<<
//extern List<String> declaredFunctions;
extern Map<String, Function> functions; // todo <<<
// todo add to Function as context! on the other hand declaredFunctions live in different world than imported ?
//extern Map<String /*function*/, List<String> /* implicit indices 0,1,2,… */> locals;
//extern Map<String /*function*/, List<Valtype> /* implicit indices 0,1,2,… */> localTypes;

extern Map<String, Node * /* modifiers/values/init expressions*/> globals; // access from Angle!
void addGlobal(Node &node);

extern Map<String /*name*/, Valtype> globalTypes;

Node &groupWhile(Node &n, Function &context);

bool isPrimitive(Node &node);

bool isType(Node &node);
//
//enum ParseOptions {
//    nix = 0,
//    no_main = 1,
//    data_only = 2,
//    ignore_errors = 4,
//};

//List<String> collectOperators(Node &expression);// test/debug only
//ParserOptions parse(String code, ParseOptions *);// wasp -> data  // this is the pure Wasp part

//__attribute__((export_name("analyze")))
Node &analyze(Node &node, Function &function = *new Function{.name="main"});// wasp -> node  // build ast via operators
Node eval(String code);// interpret OR emit :
Node interpret(String code);// wasp -> code -> data   // interpreter mode vs:
//Node emit(String code, ParseOptions options = nix);//  doesn't work with std::thread compile(emit, String(code.data()))
Node run(String source);// wasp -> code -> wasm() -> data
Node runtime_emit(String prog);// wasp -> code + runtime -> wasm (via emit)
//extern Map<String, Signature> functionSignatures;

Node constants(Node n);

void preRegisterFunctions();

bool isFunction(Node &op);

bool isFunction(String op);

// int is not a true angle type, just an alias for long.
// todo: but what about interactions with other APIs? add explicit i32 !
// todo: in fact hide most of this under 'number' magic umbrella
extern Map<String, Node> types;// by name
int run_wasm_file(chars file);

bool isPrefixOperation(Node &node, Node &lhs, Node &rhs);

String &checkCanonicalName(String &name);

void refineSignatures(Map<String, Function> &map);

Module &loadModule(String name);

Function *findLibraryFunction(String name, bool searchAliases);

bool addLocal(Function &context, String name, Valtype valtype, bool is_param);

Node &groupFunctionCalls(Node &expressiona, Function &context);
