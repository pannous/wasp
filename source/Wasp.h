#include "String.h"
#include "Angle.h"
#include "Map.h" // list

//static
extern chars function_list[];

Node parse(String source);// wasp -> data
bool is_operator(codepoint ch);// todo: next?
//extern String operator_list[];// resolve xor->operator ... semantic wasp parser really?
extern List<chars> operator_list;

extern float function_precedence;

float precedence(String name);

float precedence(Node &operater);

float precedence(char group); // special: don't mix
//Node emit(String code);// moved to angle
