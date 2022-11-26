//
// Created by pannous on 18.05.20.
//
#define _main_

#include "Wasp.h"
#include "Angle.h"
#import "wasm_helpers.h" // IMPORT so that they don't get mangled!
#include "Node.h"
#include "wasm_emitter.h"
#include "Map.h"
#include "wasm_runner.h"


Node interpret(Node n);

Node If(Node condition, Node then);

// todo? UNIFY:
// todo? a if b == if b : a
// todo? a unless b == unless b : a
// todo? a while b == while b : a
// todo? a until b == until b : a
Node If(Node n);

Node eval(String code);

Node do_call(Node left, Node op0, Node right);

Node matchPattern(Node object, Node pattern0);

//Node Node::apply_op(Node left, Node op0, Node right);

//Node Node::interpret(bool expectOperator /* = true*/) ;
