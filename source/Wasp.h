#pragma once
#include "String.h"
#include "Angle.h"
#include "Map.h" // list

#define let auto
extern bool eval_via_emit;

extern "C" void testCurrent();

//static
extern chars function_list[];
extern chars functor_list[];// takes blocks … parse differently?

extern codepoint grouper_list[];
extern codepoint opening_special_brackets[];// circumfixOperators «…»
extern chars import_keywords[];
extern List<chars> operator_list;

extern "C" Node *Parse(chars source);

Node &parse(chars source);

Node &parse(String source, ParserOptions parserOptions = {});// wasp -> data
Node assert_parsesx(chars mark);

bool is_operator(codepoint ch);// todo: next?
//extern String operator_list[];// resolve xor->operator ... semantic wasp parser really?
//extern List<chars> functor_list;

// stupid Intermediate solution to get html{color=blue} and while{color=blue} todo!
// parse x[1] as x:1 reference with immediate value or x:[1] reference with pattern
// parse x={y} as x:{y} as x{y} or keep operator =
//extern bool data_mode;// todo put in parserOptions

#define nil 0
#define null 0


extern float function_precedence;

float precedence(String name);

float precedence(Node &operater);

float precedence(char group); // special: don't mix
//Node emit(String code);// moved to angle
codepoint closingBracket(codepoint bracket);

Node parseFile(String filename, ParserOptions options = {});

String &normOperator(String &alias);
