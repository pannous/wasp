
#include "List.h"
#include "Node.h"
//const Node LongType("LongType", clazz);
//const Node DoubleType("DoubleType", clazz);//.setType(type);
// todo : when do we really need THESE Nodes instead of Type / Primitives?
Node LongType("LongType", clazz); // RealType FloatType
Node DoubleType("DoubleType", clazz); //.setType(type);
Node IntegerType("IntegerType", clazz);
Node ByteType("Byte", clazz); // Byte conflicts with mac header
Node ByteCharType("ByteCharType", clazz); // ugly by design: don't use ascii chars like that.
Node ShortType("Short", clazz); // mainly for c abi interaction, not used internally (except for compact arrays)
Node StringType("string", clazz); // ⚠️:
// Node StringType("String", clazz);  String maps to string_struct!
Node BoolType("BoolType", clazz);
Node CodepointType("CodepointType", clazz);
Node TemplateType("TemplateType", clazz); // `strings with $values`

//const Node DoubleType{.name="DoubleType", .kind=classe};//.setType(type);
//const Node DoubleType{name:"DoubleType", kind:classe};//.setType(type);


//https://en.wikipedia.org/wiki/Operators_in_C_and_C%2B%2B#Operator_precedence
//List<String> rightAssociatives = {"=", "?:", "+=", "++:"};// a=b=1 == a=(b=1) => a=1
//chars ras[] = {"=", "?:", "+=", "++", 0};
//List<chars> rightAssociatives = List(ras);
//List<chars> rightAssociatives = {"=", "?:", "-…", "+=", "++…"};// a=b=1 == a=(b=1) => a=1
//List<chars> rightAssociatives = {"=", "?:", "-…", "+=", "++…",0};// a=b=1 == a=(b=1) => a=1
List<String> rightAssociatives = {"=", "?:", "-…", "+=", "++…"}; // a=b=1 == a=(b=1) => a=1
// compound assignment


// still needs to check a-b vs -i !!
List<chars> prefixOperators = {
    "exp", "abs",/*norm*/ "not", "¬", "!", "√", "-" /*signflip*/, "--", "++", /*"+" useless!*/
    "~", "&", "$", "return", "sizeof", "new", "delete[]", "floor", "round", "ceil", "peek", "poke", "len"
};
List<chars> suffixOperators = {
    "++", "--", "…++", "…--", "⁻¹", "⁰", /*"¹",*/ "²", "³", "ⁿ", "…%", /* 23% vs 7%5 */ "％",
    "﹪", "٪",
    "‰"
}; // modulo % ≠ ％ percent
//List<chars> prefixOperators = {"not", "!", "√", "-…" /*signflip*/, "--…", "++…"/*, "+…" /*useless!*/, "~…", "*…", "&…",
//							  "sizeof", "new", "delete[]"};
//List<chars> suffixOperators = { "…++", "…--", "⁻¹", "⁰", /*"¹",*/ "²", "³", "ⁿ", "…%", "％", "﹪", "٪",
//							   "‰"};// modulo % ≠ ％ percent

extern List<chars> operator_list;

List<chars> infixOperators = operator_list;
// todo: norm all those unicode variants first!
// ᵃᵇᶜᵈᵉᶠᵍʰᶥʲᵏˡᵐⁿᵒᵖʳˢᵗᵘᵛʷˣʸᶻ ⁻¹ ⁰ ⁺¹ ⁽⁾ ⁼ ⁿ

// handled different from other operators, because … they affect the namespace context
List<chars> setter_operators = {"="};
List<chars> return_keywords = {"return", "yield", "as", "=>", ":", "->"}; // "convert … to " vs "to print(){}"
List<chars> function_operators = {":="}; //, "=>", "->" ,"<-"};
List<chars> function_keywords = {
    "def", "defn", "define", "to", "ƒ", "fn", "fun", "func", "function", "method",
    "proc", "procedure"
};
List<String> function_modifiers = {
    "public", "static", "export", "import", "extern", "external", "C", "global",
    "inline", "virtual",
    "override", "final", "abstract", "private", "protected", "internal", "const",
    "constexpr", "volatile", "mutable", "thread_local", "synchronized", "transient",
    "native"
};

List<chars> closure_operators = {"::", ":>", "=>", "->"}; // |…| { … in } <- =: reserved for right assignment
List<chars> key_pair_operators = {":"}; // -> =>
List<chars> declaration_operators = {
    ":=", "=",
    "::=" /*until global keyword*/
}; //  i:=it*2  vs i=1  OK?  NOT ":"! if 1 : 2 else 3

// ... todo maybe unify variable symbles with function symbols at angle level and differentiate only when emitting code?
// x=7
// double:=it*2  // variable of type 'block' ?

List<String> builtin_constants = {"pi", "π", "tau", "τ", "euler", "ℯ"};

List<String> class_keywords = {
    "struct", "type", "class",
    "prototype", /*"trait", "impl"*/
}; // "interface", record see wit -> module

// functions group externally square 1 + 2 == square(1 + 2) VS √4+5=√(4)+5
chars control_flows[] = {"if", "while", "for", "unless", "until", "as soon as", 0};

List<String> extra_reserved_keywords = {"func"}; // EXTRA!  // use bool isKeyword(String) !!

//List<Kind> class_kinds = {clazz, prototype, interface, structs};// record see wit
