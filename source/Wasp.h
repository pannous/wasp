#include "String.h"
#include "Angle.h"
#include "Map.h" // list

#define let auto

//static
extern chars function_list[];
extern chars functor_list[];// takes blocks … parse differently?

extern codepoint grouper_list[];
extern codepoint opening_special_brackets[];// circumfixOperators «…»
extern chars import_keywords[];
extern List<chars> operator_list;

Node &parse(String source);// wasp -> data
bool is_operator(codepoint ch);// todo: next?
//extern String operator_list[];// resolve xor->operator ... semantic wasp parser really?
//extern List<chars> functor_list;

extern bool data_mode;// stupid Intermediate solution to get html{color=blue} and while{color=blue} todo! auto

#define nil 0
#define null 0


extern float function_precedence;

float precedence(String name);

float precedence(Node &operater);

float precedence(char group); // special: don't mix
//Node emit(String code);// moved to angle
codepoint closingBracket(codepoint bracket);

Node parseFile(String filename);