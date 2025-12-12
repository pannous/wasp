#pragma once

#include "List.h"
#include "String.h"

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

// Initialize all keyword and operator lists
void initAngleKeywords();
