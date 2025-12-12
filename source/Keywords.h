#pragma once

#include "List.h"
#include "String.h"

extern List<chars> operator_list;

// Operator and keyword definitions for the Angle language

// https://en.wikipedia.org/wiki/Operators_in_C_and_C%2B%2B#Operator_precedence
// compound assignment: a=b=1 == a=(b=1) => a=1
extern List<String> rightAssociatives;

// Prefix operators: exp, abs, not, √, -, --, ++, ~, &, $, etc.
extern List<chars> prefixOperators;

// Suffix operators: ++, --, ⁻¹, ⁰, ², ³, ⁿ, %, ‰, etc.
extern List<chars> suffixOperators;

// Infix operators (from operator_list)
extern List<chars> infixOperators;

// Setter operators
extern List<chars> setter_operators;

// Return keywords: return, yield, as, =>, :, ->
extern List<chars> return_keywords;

// Function operators: :=
extern List<chars> function_operators;

// Function keywords: def, defn, define, to, ƒ, fn, fun, func, function, method, proc, procedure
extern List<chars> function_keywords;

// Function modifiers: public, static, export, import, extern, inline, virtual, etc.
extern List<String> function_modifiers;

// Closure operators: ::, :>, =>, ->
extern List<chars> closure_operators;

// Key-value pair operators: :
extern List<chars> key_pair_operators;

// Declaration operators: :=, =, ::=
extern List<chars> declaration_operators;

// Builtin constants: pi, π, tau, τ, euler, ℯ
extern List<String> builtin_constants;

// Class/struct keywords: struct, type, class, prototype
extern List<String> class_keywords;

// Control flow keywords: if, while, for, unless, until, as soon as
extern chars control_flows[];

// Extra reserved keywords
extern List<String> extra_reserved_keywords;

extern chars function_list[];

// extern chars functor_list[];// takes blocks … parse differently?

//chars runtime_function_list[]={};
static chars wasi_function_list[] = {"proc_exit", "fd_write", "args_sizes_get", "args_get","fclose","fgetc","exit","fprintf","fopen","getenv", 0};

// a funclet is a (tiny) wasm function stored in its own little file, e.g. pow.wasm for Math.power
// they get automatically linked when used without requiring the wasp runtime
// see aliases for discoverability
// todo: pre-fill list from current directory(s) / funclet registry / wapm
static chars funclet_list[] = {"pow", "powi", "pow_long", "log", "log10", "log2", "lowerCaseUTF", "print"/*node*/, 0};
//static chars funclet_list[] = {};//  LINK WITH RUNTIME!

static chars functor_list[] = {"if", "while", "for", "go", "do", "until", 0}; // MUST END WITH 0, else BUG


extern chars wasi_function_list[];

extern "C" void registerWasmFunction(chars name, chars mangled);

List<String> findAliases(String name);

float precedence(String name);

void initTypes();


//static