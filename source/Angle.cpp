//
// Created by pannous on 18.05.20.
//
#define _main_

#include <functional>
#include "Wasp.h"
#include "Angle.h"
#include "Node.h"
#include "Util.h"
#include "Map.h"
#include "Interpret.h"
#import "wasm_helpers.h" // IMPORT so that they don't get mangled!
#include "wasm_emitter.h"
#include "WitReader.h"

#ifndef RUNTIME_ONLY
#if INCLUDE_MERGER

#include "wasm_merger.h"

#endif
#endif

Module *module; // todo: use?
bool use_interpreter = false;
Node &result = *new Node();
WitReader witReader;

List<String> aliases(String name);

List<String> builtin_constants = {"pi", "π", "tau", "τ", "euler", "ℯ"};
List<String> class_keywords = {"struct", "type", "class", "prototype",
        /*"trait", "impl"*/};// "interface", record see wit -> module
//List<Kind> class_kinds = {clazz, prototype, interface, structs};// record see wit

//Map<String, Function> functions; // todo Maps don't free memory and cause SIGKILL after some time <<<
Map<String, Function> functions = {.capacity=1000};
// todo ONLY emit of main module! for funcs AND imports, serialized differently (inline for imports and extra functype section)
//Map<String, Function> library_functions; see:
List<Module *> libraries;// used modules from (automatic) import statements e.g. import math; use log; …  ≠
// functions of preloaded libraries are found WITHOUT `use` `import` statement (as in Swift) !

Map<int64, bool> analyzed = {.capacity=1000};// avoid duplicate analysis (of if/while) todo: via simple tree walk, not this!


// todo : use proper context ^^ instead of:
//Map<String /*function*/, List<String> /* implicit indices 0,1,2,… */> locals;
//Map<String /*function*/, List<Valtype> /* implicit indices 0,1,2,… */> localTypes;

Map<String, Global> globals;
//List<Global> globalVariables;

short arrayElementSize(Node &node);

Function *use_required(Function *function);

void addLibrary(Module *modul);


// functions group externally square 1 + 2 == square(1 + 2) VS √4+5=√(4)+5
chars control_flows[] = {"if", "while", "unless", "until", "as soon as", 0};

class Arg {
public:
    String function;
    String name;
//	Valtype type;
//	Valtype kind;
//    Type type;
    Node *type;
    Node modifiers;
};

// 'private header'
bool addLocal(Function &context, String name, Type Type, bool is_param); // must NOT be accessible from Emitter!

void addWasmArrayType(Type value_type) {
    if (isGeneric(value_type)) {
        addWasmArrayType(value_type.generics.value_type);
        return;
    }
    auto array_type_name = String(typeName(value_type)) + "s";// int-array -> “ints”
    Node array_type;
    array_type.kind = arrays;
    array_type.type = types[typeName(value_type)];// todo more robust
    types.add(array_type_name, array_type.clone());
    // link to type index later
}


Node getType(Node node) {
    auto name = node.name;
    bool vector = false;
    if (name.endsWith("es")) { // addresses
        // todo: cities …
        name = name.substring(0, -3);
        vector = true;
    } else if (name.endsWith("s")) { // ints …
        name = name.substring(0, -2);
        vector = true;
    }
    Node typ;
    if (types.has(name)) {
        typ = *types[name];
        typ.kind = clazz;
    } else {
        typ = *new Node(name);
        typ.kind = clazz;
        types[name] = &typ;
    }
    if (vector) {
        // holup typ.kind = arrays needs to be applied to the typed object!
        typ.kind = arrays;
        typ.type = typ.clone();
//		typ.kind=vectors; // ok, copy value
    }
    return typ;
}

bool isPlural(Node &word) {
    auto name = word.name;
    static List<String> plural_exceptions = {"flags", "puts", "plus", "minus", "times", "is", "has", "was", "does",
                                             "equals"};
    if (plural_exceptions.contains(name)) return false;
    if (name.endsWith("s"))return true;
    return false;
}

bool isType(Node &expression) {
    auto name = expression.name;
//    if (isPlural(expression))// very week criterion: houses=[1,2,3]
//        return true;
    if (name.empty())return false;
    return types.has(name);
}


Node constants(Node n) {
    if (eq(n.name, "not"))return True;// not () == True; hack for missing param todo: careful!
    if (eq(n.name, "one"))return Node(1);
    if (eq(n.name, "two"))return Node(2);
    if (eq(n.name, "three"))return Node(3);
    return n;
}


//Map<int64, int> _isFunction; INEFFICIENT! // true not a 'type'
// Hashmap _isFunction; // todo
bool isFunction(String op, bool deep_search) {
//	if(_isFunction[op.hash()])return true;
    if (op.empty())return false;
    if (op == "‖")return false;
    if (functions.has(op))return true;
    if (functions.has(op))return true;// pre registered signatures
    if (op.in(function_list))
        return true;
    if (deep_search and findLibraryFunction(op, true))
        return true;// linear lookup ok as long as #functions < 1000 ? nah getting SLOW QUICKLY!!
//	if(op.in(functor_list))
//		return true;
    return false;
}

bool isFunction(Node &op) {
    if (op.name.empty())return false;
    if (op.kind == strings)return false;
    if (op.kind == declaration)return false;
    return isFunction(op.name);
}


Node interpret(String code) {
    Node parsed = parse(code);
    return parsed.interpret();
}


#ifndef RUNTIME_ONLY

Code &compile(String code, bool clean = true);// exposed to wasp.js
#endif

// todo: merge with emit
Node eval(String code) {
#ifdef RUNTIME_ONLY
    return parsed; // no interpret, no emit => pure data  todo: WARN
#else
#ifndef WASI
    if (use_interpreter) {
        Node parsed = parse(code);
        return parsed.interpret();
    } else
#endif
    {
        Code &binary = compile(code, true);
        binary.save();// to debug
        smart_pointer_64 results = binary.run();
        auto _resultNode = smartNode(results);
        if (!_resultNode)return ERROR;
        Node &resultNode = *_resultNode;
#if not RELEASE and not MY_WASM
        print("» %s\n"s % resultNode.serialize().data);
#endif
        return resultNode;
    }
#endif

}

Node &groupTypes(Node &expression, Function &context);

Signature &groupFunctionArgs(Function &function, Node &params) {
    //			left = analyze(left, name) NO, we don't want args to become variables!
    List<Arg> args;
    Node &nextType = Double;
    if (params.length == 0) {
        params = groupTypes(params, function);
        if (params.name != function.name)
            args.add({function.name, params.name, params.type ? params.type : &nextType});
    }
    for (Node &arg: params) {
        if (isType(arg)) {
            if (args.size() > 0 and not args.last().type)
                args.last().type = types[arg.name];
            else nextType = arg;
        } else {
            if (arg.name != function.name)
                args.add({function.name, arg.name, arg.type ? arg.type : &nextType, params});
        }
    }

    Signature &signature = function.signature;
    for (Arg arg: args) {
        auto name = arg.name;
        if (empty(name))
            name = String("$"s + signature.size());
//            error("empty argument name");
        if (function.locals.has(name)) {
            error("duplicate argument name: "s + name);
        }
        addLocal(function, name, arg.type, true);
        signature.add(arg.type, name);// todo: arg type, or pointer
    }
    if (params.value.node) {
        Node &ret = params.values();
        Type type = mapType(ret.name);
        signature.returns(type);
    }
    return signature;
}

String extractFunctionName(Node &node) {
    if (not node.name.empty() and node.name != ":=")
        return node.name;
    if (node.length > 1)
        return node.first().name;
    // todo: public go home to family => go_home
    return node.name;
}

// if a then b else c == a and b or c
// (a op c) => op(a c)
// further right means higher prescedence/binding, gets grouped first
// todo "=" ":" handled differently?



//https://en.wikipedia.org/wiki/Operators_in_C_and_C%2B%2B#Operator_precedence
//List<String> rightAssociatives = {"=", "?:", "+=", "++:"};// a=b=1 == a=(b=1) => a=1
//chars ras[] = {"=", "?:", "+=", "++", 0};
//List<chars> rightAssociatives = List(ras);
//List<chars> rightAssociatives = {"=", "?:", "-…", "+=", "++…"};// a=b=1 == a=(b=1) => a=1
//List<chars> rightAssociatives = {"=", "?:", "-…", "+=", "++…",0};// a=b=1 == a=(b=1) => a=1
List<String> rightAssociatives = {"=", "?:", "-…", "+=", "++…"};// a=b=1 == a=(b=1) => a=1
// compound assignment


// still needs to check a-b vs -i !!
List<chars> prefixOperators = {"abs",/*norm*/  "not", "¬", "!", "√", "-" /*signflip*/, "--", "++", /*"+" useless!*/
                               "~", "&", "$", "return",
                               "sizeof", "new", "delete[]", "floor", "round", "ceil", "peek", "poke"};
List<chars> suffixOperators = {"++", "--", "…++", "…--", "⁻¹", "⁰", /*"¹",*/ "²", "³", "ⁿ", "…%", /* 23% vs 7%5 */ "％",
                               "﹪", "٪",
                               "‰"};// modulo % ≠ ％ percent
//List<chars> prefixOperators = {"not", "!", "√", "-…" /*signflip*/, "--…", "++…"/*, "+…" /*useless!*/, "~…", "*…", "&…",
//							  "sizeof", "new", "delete[]"};
//List<chars> suffixOperators = { "…++", "…--", "⁻¹", "⁰", /*"¹",*/ "²", "³", "ⁿ", "…%", "％", "﹪", "٪",
//							   "‰"};// modulo % ≠ ％ percent



List<chars> infixOperators = operator_list;
// todo: norm all those unicode variants first!
// ᵃᵇᶜᵈᵉᶠᵍʰᶥʲᵏˡᵐⁿᵒᵖʳˢᵗᵘᵛʷˣʸᶻ ⁻¹ ⁰ ⁺¹ ⁽⁾ ⁼ ⁿ

// handled different than other operators, because … they affect the namespace context
List<chars> setter_operators = {"="};
List<chars> return_keywords = {"return", "yield", "as", "=>", ":", "->"}; // "convert … to " vs "to print(){}"
List<chars> function_operators = {":="};//, "=>", "->" ,"<-"};
List<chars> function_keywords = {"def", "defn", "define", "to", "ƒ", "fn", "fun", "func", "function", "method",
                                 "proc", "procedure"};
List<String> function_modifiers = {"public", "static", "export", "import", "extern", "C"};

List<chars> closure_operators = {"::", ":>", "=>", "->"}; // <- =: reserved for right assignment
List<chars> key_pair_operators = {":"};
List<chars> declaration_operators = {":=", "=",
                                     "::=" /*until global keyword*/}; //  i:=it*2  vs i=1  OK?  NOT ":"! if 1 : 2 else 3

// ... todo maybe unify variable symbles with function symbols at angle level and differentiate only when emitting code?
// x=7
// double:=it*2  // variable of type 'block' ?


Node &groupIf(Node n, Function &context) {
    if (n.length == 0 and !n.value.data)
        error("no if condition given");
    if (n.length == 1 and !n.value.data)
        error("no if block given");
    Node &condition = n.first();
    Node then;
    if (n.value.data)
        then = n.values();
    else if (n.kind == key) {
        condition = n.from(1);
        then = *n.value.node;
    } else if (n.length > 1)then = n[1];
    if (then == "else")
        then = condition.values();

    if (n.has("then")) {
        condition = n.to("then");
        then = n.from("then");
    }

    if (condition.kind == key and condition.value.data)// or condition.next // and !condition.next)
        then = condition.values();

    if (n.has(":") /*before else: */) { // todo remove since ":" is always parsed immediate (right?)
        condition = n.to(":");
        if (condition.has("else"))
            condition = condition.to("else");// shouldn't happen?
        if (then.length == 0)
            then = n.from(":");
    } else if (condition.has(":")) {// as child
        then = condition.from(":");
        //		condition = condition.interpret();// compile time evaluation?!
    }

    Node otherwise;
    if (n.has("else")) {
        otherwise = n["else"].values();
        if (otherwise.empty())
            otherwise = n.last();
    }
    if (then.has("then"))
        then = n.from("then");
    if (then.has("else")) {
        otherwise = then.from("else");
        then = then.to("else");
    }
    if (then.name == ":") { // todo remove since ":" is always parsed immediate (right?)
        if (then.length > 1)
            otherwise = then[1];
        if (then.length > 2)
            error("too many clauses for if?");
        then = then.first();
    }
    if (n.length == 3 and otherwise.isEmpty())
        otherwise = n[2];
    if (otherwise.isEmpty() and n.next) {
        if (n.next->name == "else") {
            otherwise = *n.next->next;
//            otherwise = n.next->values();
        }
//        else
//            otherwise = *n.next;
    }
    Node *eff = new Node("if");
    Node &ef = *eff;
    ef.kind = expression;
    //	ef.kind = ifStatement;
    if (condition.length > 0)condition.setType(expression);// so far treated as group!
    if (then.length > 0)then.setType(expression);
    if (otherwise.length > 0)otherwise.setType(expression);

    ef["condition"] = analyze(condition, context);
    ef["then"] = analyze(then, context);
    ef["else"] = analyze(otherwise, context);
    //	condition = analyze(condition);
    //	then = analyze(then);
    //	otherwise = analyze(otherwise);
    //	ef.add(condition); breaks even with clone() why??
    //	ef.add(then);
    //	ef.add(otherwise);
    //	Node &node = ef["then"];// debug
//	Node &node = ef[2];// debug
    analyzed[ef.hash()] = true;
    return ef;
}

List<String> collectOperators(Node &expression) {
    List<String> operators;
    String previous;
    for (Node &op: expression) {
        String &name = op.name;
        if (not name)continue;
//		name = normOperator(name);// times => * aliases
        if (name.endsWith("="))// += etc
            operators.add(name);
        else if (prefixOperators.has(name)) {
            if (name == "-" and is_operator(previous.codepointAt(0)))
                operators.add(name + "…");//  -2*7 ≠ 1-(2*7)!
            else
                operators.add(name);
        }
//		WE NEED THE RIGHT PRECEDENCE NOW! -2*7 ≠ 1-(2*7)! or is it? √-i (i FIRST)  -√i √( first)
//		else if (prefixOperators.has(op.name+"…"))// and IS_PREFIX
//			operators.add(op.name+"…");
        else if (suffixOperators.has(name))
            operators.add(name);
        else if (operator_list.has(name))
            //			if (op.name.in(operator_list))
            operators.add(name);
        else if (op.kind == Kind::operators)
            operators.add(op.name);

//else if (suffixOperators.has(op.name+"…"))
//	operators.add(op.name);
        //		if (op.name.in(function_list))
        //			operators.add(op.name);
        //		if (op.name.in(functor_list))
        //			operators.add(op.name);
        previous = name;
        if (contains(import_keywords, (chars) name.data))
            break;
    }
    auto by_precedence = [](String &a, String &b) { return precedence(a) > precedence(b); };
    operators.sort(by_precedence);
    return operators;
}


bool isVariable(Node &node) {
    if (node.kind != reference and node.kind != key and !node.isSetter())
        return false;
    if (node.kind == strings)return false;
    return /*node.parent == 0 and*/ not node.name.empty() and node.name[0] >= 'A';// todo;
}

bool isPrimitive(Node &node) {
    // should never be cloned so always compare by reference ok?
    if (&node == &Int)return true;
    if (&node == &Bool)return true;
    if (&node == &Long)return true;
    if (&node == &Double)return true;
    if (&node == &ByteType)return true;
    if (&node == &ByteChar)return true;
    if (&node == &Charpoint)return true;
    if (&node == &ShortType)return true;
    if (&node == &StringType)return true;
    Kind type = node.kind;
    if (type == longs or type == strings or type == reals or type == bools or type == arrays or type == buffers)
        return true;
    if (type == codepoint1)// todo ...?
        return true;
    return false;
}

Map<String, Node *> types = {.capacity=1000}; // builtin and defined Types
//const Node Long("Long", clazz);
//const Node Double("Double", clazz);//.setType(type);
// todo : when do we really need THESE Nodes instead of Type / Primitives?
Node Long("Long", clazz);
Node Double("Double", clazz);//.setType(type);
Node Int("Int", clazz);
Node ByteType("Byte", clazz);// Byte conflicts with mac header
Node ByteChar("ByteChar", clazz);// ugly by design: don't use ascii chars like that.
Node ShortType("Short", clazz);// mainly for c abi interaction, not used internally (except for compact arrays)
Node StringType("String", clazz);
Node Bool("Bool", clazz);
Node Charpoint("Charpoint", clazz);

//const Node Double{.name="Double", .kind=classe};//.setType(type);
//const Node Double{name:"Double", kind:classe};//.setType(type);

// todo: see NodeTypes.h for overlap with numerical returntype integer …
// these are all boxed class types, for primitive types see Type and Kind
void initTypes() {
    if (types.size() > 10)return;
    types.add("i8", &ByteType);// use in u8.load etc
    types.add("u8", &ByteType);// use in u8.load etc
    types.add("int8", &ByteType);// use in u8.load etc
    types.add("uint8", &ByteType);// use in u8.load etc
    types.add("byte", &ByteType);// use in u8.load etc
//    types.add("sint8", &Int); NOT MAPPED until required
//    types.add("sint", &Int);

//    types.add("char", &Byte);
    types.add("char", &Charpoint);// todo : warn about abi conflict? CAN'T USE IN STRUCT
    types.add("character", &Charpoint);
    types.add("charpoint", &Charpoint);
    types.add("i32", &Int);
    types.add("int", &Int);
    types.add("int32", &Int);
    types.add("integer", &Int);
    types.add("long", &Long);
    types.add("double", &Double);
    types.add("float", &Double);
    for (int i = 0; i < types.size(); ++i) {
        auto typ = types.values[i];
        if (int64(typ) < 0 or int64(typ) > 10000000l)
            continue;// todo: this type reflection is bad anyways?
//            error("bad wasm type initialization");
//        if(typ)typ->setType(clazz);
    }
}


Node &constructInstance(Node &node, Function &function);

Node &groupTypes(Node &expression, Function &context) {
    // todo delete type declarations double x, but not double x=7
    // todo if expression.length = 0 and first.length=0 and not next is operator return ø
    // ways to set type:
    /*
	 * int x
	 * x:int
	 * x=7  via pre-evaluation of rest!!!
	 * x int    unstable, discouraged!
	 * */
    if (types.size() == 0)initTypes();
    if (isType(expression)) {// double \n x,y,z  extra case :(
        Node &type = *types[expression.name];
        auto is_primitive = isPrimitive(type);
        if (not is_primitive and (type.kind == structs or type.kind == clazz)) // or type == wasmtype_struct
            return constructInstance(expression, context);

        if (expression.length > 0) {// point{x=1 y=2} point{x y}
            for (Node &typed: expression) {// double \n x = 4
                typed.setType(&type);
                addLocal(context, typed.name, mapType(typed.name), false);
            }
            expression.name = "";// hack
            expression.kind = groups;
            return expression.flat();
        } else if (expression.next) {
            expression.next->type = expression.clone();
            return *expression.next;
        } else if (expression.length == 0) {
            return *getType(expression).clone();
        } else {
            //  type name as variable name!
            expression.type = getType(expression).clone();
        }
    }
    for (int i = 0; i < expression.length; i++) {
        Node &node = expression.children[i];
        if (not isType(node))
            continue;
        if (node.length > 0) {
            node = groupTypes(node, context);// double (x,y,z)
            continue;
        }
        static Node typeDummy;// todo: remove how?
        Node &typed = typeDummy;
//		if (node.next and not is_operator(node.next->name[0])) {
//			typed = *node.next;
        if (i < expression.length - 1 and not is_operator(expression.children[i + 1].name[0])) {
            typed = expression.children[i + 1];
        } else if (i > 1) {
            typed = expression.children[i - 1];
        } else {
#ifdef DEBUG
            error("Type without object: "s + node.serialize() + "\n" + node.Line());// may be ok
#else
            error("Type without object: "s+node.serialize());// may be ok
#endif
            typed = NIL;
        }
//			if (operator_list.has(typed.name))
//				continue; // 3.3 as int …
        auto aType = types[node.name];
        if (not aType)continue;

        if (typed.name == "as") { // danger edge cases!
            expression.remove(i - 1, i);
            expression.children[i - 2].type = aType;// todo bug, should be same as
            typed = expression.children[i - 2];
            typed.type = aType;
            continue;
        } else {
            expression.remove(i, i);
        }
        while (isPrimitive(typed) or
               (typed.kind == reference and typed.length == 0)) {// BAD criterion for next!
            typed.type = aType;// ref ok because types can't be deleted ... rIgHt?
            if (typed.kind == reference or typed.isSetter())
                addLocal(context, typed.name, mapType(aType->name), false);
            // HACK for double x,y,z => z.type=Double !
            if (i + 1 < expression.length)
                typed = expression[++i];
            else if (typed.next) typed = *typed.next;
                // else outer group types currently not supported ((double x) y z)
            else break;
        }
    }
//	if(isPrimitive(expression)
    return expression.flat(); // (1) => 1
}

Node &constructInstance(Node &node, Function &function) {
    Node &type = *types[node.name];
    if (node.values().name == "func") return node;
//        /* todo what is this??  test-tuple: func(other: list<u8>, test-struct: test-struct, other-enum: test-enum) -> tuple<string, s64>

    node.type = &type;// point{1 2} => Point(1,2)
    check_eq_or(node.length, type.length, "field count mismatch");
    node.kind = constructor;

    for (int i = 0; i < node.length; ++i) {
        node[i].type = type[i].type;
//        Valtype valtype = mapType(*node[i].type);
//        Valtype valtype1 = mapTypeToWasm(node[i].kind);
//        check_eq_or(valtype , valtype1,
//                    error("incompatible value for field %s of type "s % type[i].name + type.name + " in " + node.serialize()));

    }
    return node;
}


void updateLocal(Function &context, String name, Type type) {
//#if DEBUG
    Local &local = context.locals[name];
    auto type_name = typeName(type);
    auto oldType = local.type;
    if (oldType == none or oldType == unknown_type) {
        local.type = type;
    } else if (oldType != type and type != void_block and type != voids and (Type) type != unknown_type) {
        if (use_wasm_arrays) {
            // TODO: CLEANUP
            if (oldType == node) {
                addWasmArrayType(type);
                local.type = type;// make more specific!
            } else if (isGeneric(oldType)) {
                auto kind = oldType.generics.kind;
                auto valueType1 = oldType.generics.value_type;
                if (valueType1 != type) todow(
                        "generic type mismatch "s + typeName(oldType) + " vs " + type_name);
            } else if (oldType == wasmtype_array or oldType == array) {
                addWasmArrayType(type);
                local.type = genericType(array, type);
            }
        } else
            warn("local in context %s already known "s % context.name + name + " with type " + typeName(oldType) +
                 ", ignoring new type " + type_name);
    }
    // ok, could be cast'able!
}

// return: done?
// todo return clear enum known, added, ignore ?
bool addLocal(Function &context, String name, Type type, bool is_param) {
    if (name.empty()) {
        warn("empty reference in "s + context);
        return true;// 'done' ;)
    }
    // todo: kotlin style context sensitive symbols!
    if (builtin_constants.has(name))
        return true;
    if (context.signature.has(name)) {
        return false; // known parameter!
    }
    if (globals.has(name))
        error(name + " already declared as global"s);
    if (isFunction(name, true))
        error(name + " already declared as function"s);
    if (not context.locals.has(name)) {
        int position = context.locals.size();
        context.locals.add(name, Local{.is_param=is_param, .position=position, .name=name, .type=type});
        return true;// added
    } else {
        updateLocal(context, name, type);
        return false;// already there
    }
//#endif
}

Node &groupSetter(String name, Node &body, Function &context) {
    Node *decl = new Node(name);//node.name+":={…}");
    decl->setType(assignment);
    decl->add(body.clone());// addChildren makes emitting harder
    auto type = preEvaluateType(body, context);
    if (not addLocal(context, name, type, false)) {
        Local &local = context.locals[name];
        local.type = type;// update type! todo: check if cast'able!
    }
    return *decl;
}

Node extractReturnTypes(Node decl, Node body);


Node &classDeclaration(Node &node, Function &function);


Node &classDeclaration(Node &node, Function &function) {
    if (node.length < 2)
        error("wrong class declaration format; should be: class name{…}");
    Node &dec = node[1];
    String &kind_name = node.first().name;
    if (kind_name == "struct") {
//        if (use_wasm_structs) distinguish implementation later!
        dec.kind = structs;
    } else if (kind_name == "class" or kind_name == "type")
        dec.kind = clazz;
    else todo(kind_name);

    String &name = dec.name;
    int pos = 0;
    Node *type;
    // todo other layouts and default values and class constructors, functions, getters … … …
    for (Node &field: dec) {
        if (isType(field)) {
            type = &field;
            continue;
        } else
            type = &field.values();
        if (not type or not isType(*type) or not types.has(type->name))
            error("class field needs type");
        else
            field.type = types[type->name]; // int => Int
        field.kind = fields;
        field.value.longy = pos;
        field["position"] = pos++;
    }
    if (types.position(name) >= 0) {
        if (types[name] == dec)
            warn("compatible declaration of %s already known"s % name);
        else
            error("incompatible structure %s already declared:\n"s % name + types[name]->serialize() /*+ node.line*/);
    } else {
        types.add(name, dec.clone());
    }
    return dec;
}


Node &funcDeclaration(String name, Node &node, Node &body, Node *returns, Module *mod) {
//    if(functions.has(name)) polymorph
// is_used to fake test wit signatures
    Function function = {.name=name, .module = mod, .is_import=true, .is_declared= not body.empty(), .is_used=true,};// todo: clone!
    function.body = &body;
    function.signature = groupFunctionArgs(function, node);
    if (returns and function.signature.return_types.size() == 0)
        function.signature.returns(mapType(returns->name));
    functions.insert_or_assign(name, function);
    return Node().setType(functor).setValue(Value{.data=&function});// todo: clone!  todo functor => Function* !?!?
}


void discard(Node &node) {
// nop to explicitly ignore unused-variable (for debugging) or no-discard (e.g. in emitData of emitArray )
}

// bad : we don't know which
void use_runtime(const char *function) {
    findLibraryFunction(function, false);
//    Function &function = functions[function];
//    if (not function.is_import and not function.is_runtime)
//        error("can only use import or runtime "s + function);
//    function.is_used = true;
//    function.is_import = true;// only in this module, not in original !
}

Node &
groupFunctionDeclaration(String &name, Node *return_type, Node modifieres, Node &arguments, Node &body,
                         Function &context) {
//	String &name = fun.name;
//	silent_assert(not is_operator(name[0]));
//	trace_assert(not is_operator(name[0]));
    if (is_operator(name[0]))// todo ^^
    todo("is_operator!");// remove if it doesn't happen

    if (name and not function_operators.has(name)) {
        if (context.name != "wasp_main") todo("inner functions");
        if (not functions.has(name)) {
            functions.add(name, *new Function{.name=name});
        }
    }
    Function &function = functions[name]; // different from context!
    function.is_declared = true;
    function.is_import = false;
    // todo : un-merge x=1 x:1 vs x:=it function declarations for clarity?
    if (setter_operators.has(name) or key_pair_operators.has(name)) {
        body = analyze(body, function);
        if (arguments.has("global"))
            globals.insert_or_assign(name, Global{.index=globals.count(), .name=name, .type=mapType(
                    body.type), .value=body.clone()});
        return groupSetter(name, body, function);
    }

    Signature &signature = groupFunctionArgs(function, arguments);
    if (signature.size() == 0 and function.locals.size() == 0 and body.has("it", false, 100)) {
        addLocal(function, "it", float64, true);
        signature.add(float64, "it");// todo: any / smarti! <<<
    }
    body = analyze(body, function);// has to come after arg analysis!
    if (!return_type)
        return_type = extractReturnTypes(arguments, body).clone();
    if (return_type)
        signature.returns(mapTypeToPrimitive(*return_type));// explicit double sin(){} // todo other syntaxes+ multi
    Node &decl = *new Node(name);//node.name+":={…}");
    decl.setType(declaration);
    decl.add(body.clone());
    function.body = body.clone();//.flat(); // debug or use?
//    function.body= &body;
    return decl;
}

Node &groupOperators(Node &expression, Function &context);

Node extractModifiers(Node &node);


Node &groupKebabMinus(Node &node, Function &function);

Node extractModifiers(Node &expression) {
    Node modifieres;
    for (auto child: expression) {
        if (function_modifiers.contains(child.name)) {
            modifieres.add(child);
            expression.remove(child);
        }
    }
    return modifieres;
}


// def foo(x,y) => x+y  vs  groupFunctionDeclaration foo := x+y
Node &groupFunctionDefinition(Node &expression, Function &context) {
    auto first = expression.first();
    Node modifieres = extractModifiers(expression);
    auto kw = expression.containsAny(function_keywords);
    if (expression.index(kw) != 0) error("function keywords must be first");
    expression.children++;
    expression.length--;
    auto fun = expression.first();
    Node return_type;
    Node arguments = groupTypes(fun.childs(), context); // children f(x,y)
    Node body;
    auto ret = expression.containsAny(return_keywords);
    if (ret) {
        return_type = expression.from(ret);
        expression = expression.to(ret);
        body = return_type.values(); // f(x,y) -> int { x+y }
    } else if (expression.size() == 3) {// f(x,y) int { x+y }
        return_type = expression[1];
        body = expression.last();
    } else body = fun.values();

    auto opa = expression.containsAny(function_operators); // fun x := x+1
    if (opa)
        body = expression.from(opa);
    return groupFunctionDeclaration(fun.name, &return_type, NIL, arguments, body, context);
}

// f x:=x*x  vs groupFunctionDefinition fun x := x*x
Node &groupFunctionDeclaration(Node &expression, Function &context) {
    Node modifieres = extractModifiers(expression);
    auto op = expression.containsAny(function_operators);
    auto left = expression.to(op);
    auto rest = expression.from(op);
    auto fun = left.first();
    return groupFunctionDeclaration(fun.name, 0, left, left, rest, context);
}

Node &groupDeclarations(Node &expression, Function &context) {
//    if (expression.kind != Kind::expression)return expression;// 2022-19 sure??
    if (expression.contains(":=")) {
        return groupFunctionDeclaration(expression, context);
    }

    auto first = expression.first();
    if (expression.length == 2 and isType(first.first()) and
        expression.last().kind == objects) {// c style double sin() {}
        expression = groupTypes(expression, context);
        return groupFunctionDeclaration(first.name, first.type, NIL, first.values(), expression.last(), context);
    }
    for (Node &node: expression) {
        if (&node == 0) {
            todow("BUG &node==0");
            continue;
        }
        String &op = node.name;
        if (isPrimitive(node) and node.isSetter()) {
            if (globals.has(op)) {
                warn("Cant set globals yet!");
                continue;
            }
            addLocal(context, op, preEvaluateType(node, context), false);
            if (node.length >= 2)
                info("c-style function?");
            else
                continue;
        }
        if (node.kind == reference or (node.kind == key and isVariable(node))) {// only constructors here!
            if (not globals.has(op) and not isFunction(node)) {
                Type evaluatedType = unknown_type;
                if (use_wasm_arrays)
                    evaluatedType = preEvaluateType(node, context);// todo turns sin π/2 into 1.5707963267948966 ;)
                addLocal(context, op, evaluatedType, false);
            }
            continue;
        }// todo danger, referenceIndices i=1 … could fall through to here:
        if (node.kind != declaration and not declaration_operators.has(op))
            continue;
        if (op.empty())continue;
        if (op == "=" or op == ":=") continue; // handle assignment via groupOperators !
        if (op == "::=") continue; // handle globals assignment via groupOperators !
        // todo: public export function jaja (a:num …) := …

        // BEGINNING OF Declaration ANALYSIS
        Node left = expression.to(node);// including public… + ARGS! :(
        Node rest = expression.from(node); // body
        String name = extractFunctionName(left);
        if (left.length == 0 and not declaration_operators.has(node.name))
            name = node.name;// todo: get rid of strange heuristics!
        if (node.length > 1) {
            // C style double sin(x) {…} todo: fragile criterion!! also why is body not child of sin??
            name = node.first().name;
            left = node.first().first();// ARGS
            rest = node.last();
            if (rest.kind == declaration)rest = rest.values();
            context.locals.remove(name);// not a variable!
        }
////			todo i=1 vs i:=it*2  ok ?

        if (name.empty())
            continue;
        if (isFunction(name, true)) {
            node.kind = call;
            continue;
        }
//			error("Symbol already declared as function: "s + name);
        // if (function.locals.has(name)) // todo double := it * 2 ; double(4)
//				error("Symbol already declared as variable: "s + name);
//			if (isImmutable(name))
//				error("Symbol declared as constant or immutable: "s + name);
        return groupFunctionDeclaration(name, 0, left, left, rest, context);
    }
    return expression;
}

Node extractReturnTypes(Node decl, Node body) {
    return Double;// Long;// todo
}

// outer analysis 3 + 3  ≠ inner analysis +(3,3)
// maybe todo: normOperators step (in angle, not wasp!)  3**2 => 3^2
Node &groupOperators(Node &expression, Function &context) {
    if (analyzed.has(expression.hash()))
        return expression;

//    if ("a-b" and (expression.kind == reference or expression.kind == Kind::expression) and expression.name.contains('-'))
//        return groupKebabMinus(expression, context); // extra logic for a-b kebab-case vs minus

    Function &function = context;
    List<String> operators = collectOperators(expression);
    String last = "";
    int last_position = 0;

    if (expression.kind == Kind::operators) {
        if (expression.name == "include") {
            warn(expression.serialize());
            Node &file = expression.values();
            addLibrary(&loadModule(file.name));
            return NUL;// no code, just meta
        }
//		else todo("ungrouped dangling operator");
    }
// inner ops get grouped first: 3*42≥2*3 => 1. group '*' : (3*42)≥(2*3)
    for (String &op: operators) {
//        trace("operator");
//        trace(op);
        if (op.empty())
            error("empty operator BUG");
        if (op == "else")continue;// handled in groupIf
        if (op == "module") {
            warn("todo modules");
            if (module)
                module->name = expression.last().name;
            return NUL;
        }
        if (op == "-")
            debug = true;
        if (op == "-…") op = "-";// precedence hack

//        todo op=use_import();continue ?
        if (op == "%")
            functions["modulo_float"].is_used = true;// no wasm i32_rem_s i64_rem_s for float/double
        if (op == "%")
            functions["modulo_double"].is_used = true;
        if (op == "==" or op == "is" or op == "equals")use_runtime("eq");
        if (op == "include")
            return NUL;// todo("include again!?");
        if (op != last) last_position = 0;
        bool fromRight = rightAssociatives.has(op);// << 2022-12-31 currently fails here after 3 runs
        fromRight = fromRight || isFunction(op, true);
        fromRight = fromRight || (prefixOperators.has(op) and op != "-"); // !√!-1 == !(√(!(-1)))
        int i = expression.index(op, last_position, fromRight);
        if (i < 0) {
            if (op == "-" or op == "‖") //  or op=="?"
                continue;// ok -1 part of number, ‖3‖ closing ?=>if
            i = expression.index(op, last_position, fromRight);// try again for debug
            expression.print();
            error("operator missing: "s + op);
        }
        op = checkCanonicalName(op);

        // we can't keep references here because expression.children will get mutated later via replace(…)
        Node &node = expression.children[i];
        if (node.length)continue;// already processed
        Node &next = expression.children[i + 1];
        next = analyze(next, context);
        Node prev;
        if (i > 0)
            prev = expression.children[i - 1];
//            prev = expression.to(op);
//        else error("binop?");

// todo move "#" case here:
        if (isPrefixOperation(node, prev, next)) {// ++x -i
            // PREFIX Operators
            isPrefixOperation(node, prev, next);
            node.kind = Kind::operators;// todo should have been parsed as such!
            if (op == "-")//  -i => -(0 i)
                node.add(new Node(0));// todo: use f64.neg
            node.add(next);
            expression.replace(i, i + 1, node);
        } else {
            if (op == "#" and not use_wasm_strings)
                findLibraryFunction("getChar", false);

            prev = analyze(prev, context);
            auto lhs_type = preEvaluateType(prev, context);
            if (op == "+" and (lhs_type == Primitive::charp or lhs_type == Primitive::stringp or lhs_type == strings)) {
                findLibraryFunction("concat", true);
                findLibraryFunction("_Z6concatPKcS0_", true);
            }
            if (op == "^" or op == "^^" or op == "**") {// todo NORM operators earlier
                findLibraryFunction("pow", false);
//                findLibraryFunction("powi", false);
//                functions["pow"].is_used = true;
//                functions["powi"].is_used = true;
            }
            if (suffixOperators.has(op)) { // x²
                // SUFFIX Operators
                if (op == "ⁿ") {
                    findLibraryFunction("pow", false);
                    functions["pow"].is_used = true;
                }
                if (i < 1)error("suffix operator misses left side");
                node.add(prev);
                if (op == "²") {
                    node.add(prev);
                    node.name = "*"; // x² => x*x
                }
//				analyzed.insert_or_assign(node.hash(), true);
                expression.replace(i - 1, i, node);
                i--;
            } else if (op.in(function_list)) {// handled above!
                while (i++ < node.length)
                    node.add(expression.children[i]);
                expression.replace(i, node.length, node);
            } else if (isFunction(next)) { // 3 + double 8
                Node &rest = expression.from(i + 1);
                Node &args = analyze(rest, context);
                node.add(prev);
                node.add(args);
                expression.replace(i - 1, i + 1, node);// replace ALL REST
                expression.remove(i, -1);

            } else {
                auto var = prev.name;
                if (op.endsWith("=") and not op.startsWith("::") and prev.kind == reference) {
                    // x=7 and x*=7
                    // todo can remove hack?
                    Type inferred_type = preEvaluateType(next, context);
                    if (addLocal(context, var, inferred_type, false)) {
                        if (op.length > 1 and op.endsWith("=") and not op.startsWith(":")) // x+=1 etc
                            error("self modifier on unknown reference "s + var);
                    } else {
                        Local &local = function.locals[var];
                        // variable is known but not typed yet, or type again?
                        if (local.type == unknown_type) {
                            local.type = inferred_type;// mapType(next);
                        }
                    }
                }
                //#endif
                if (not(op == "#" and prev.empty() and prev.kind != reference)) // ok for #(1,2,3) == len
                    node.add(prev);
                node.add(next);

                if (op == "::=") {
                    if (prev.kind != reference)error("only references can be assigned global (::=)"s + var);
//					if(function.locals.has(prev.name))error("global already known as local "s +prev.name);// let's see what happens;)
                    if (globals.has(var))error("global already set "s + var);// todo reassign ok if …
                    globals.add(var, Global{.index=globals.size(), .name=var, .type=unknown, .value=next.clone()});
                    // type set in globalSection, after emitExpression
                    // don't forget to emit
                } else if (op.length > 1 and op.endsWith("=") and op[0] != ':')
                    // Complicated way to express *= += -= … self assignments
                    if (op[0] != '=' and op[0] != '!' and op[0] != '?' and op[0] != '<' and op[0] != '>') {
                        // *= += etc
                        node.name = String(op.data[0]);
                        Node *setter = prev.clone();
//					setter->setType(assignment);
                        setter->value.node = node.clone();
                        node = *setter;
                    }
                if (node.name == "?")
                    node = groupIf(node, context);// consumes prev and next
//				analyzed.add(node.hash(), true);
                if (i > 0)
                    expression.replace(i - 1, i + 1, node);
                else {
                    expression.replace(i, i + 1, node);
                    warn("operator missing argument");
                }
                if (expression.length == 1 and expression.kind == Kind::groups) {
                    return expression.first();//.setType(operators);
                }
            }
        }
        last_position = i;
        last = op;
    }
    return expression;
}

// extra logic for a-b kebab-case vs minus operator
Node &groupKebabMinus(Node &node, Function &context) {
    auto name = node.name;
    auto re = name.substring(0, name.indexOf('-'));
    auto lhs = Node(re, true);
    if (context.locals.has(re) or globals.has(re)) {
        Node op = Node("-").setType(operators, true);
        auto right = name.substring(name.indexOf('-') + 1);
        auto rhs = analyze(*new Node(right, true), context);// todo expression!
        op.add(lhs);
        op.add(rhs);
        return *op.clone();
    }
    return node;
}

Module &loadRuntime() {
#if MY_WASM
	static Module wasp;
//    Module &wasp=*module_cache["wasp"s.hash()];
//    wasp.functions["powi"].signature.returns(int32);
	return wasp;
#else
    Module &wasp = read_wasm("wasp-runtime.wasm");
    wasp.functions["getChar"].signature.returns(codepoint1);
    return wasp;
#endif
}


Type preEvaluateType(Node &node, Function &context) {
    // todo: some kind of Interpret eval?
    // todo: combine with compile time eval! <<<<<
    if (node.kind == expression) {
        if (node.name == "if") // if … then (type 1) else (type 2)
            return commonType(preEvaluateType(node["then"], context), preEvaluateType(node["else"], context));
        if (node.length == 1)return preEvaluateType(node.first(), context);
        node = groupOperators(node, context);
        return mapType(node.name);
    }
    if (node.kind == operators) {
        Node &lhs = node[0];
        auto lhs_type = preEvaluateType(lhs, context);
        if (node.length == 1)
            return lhs_type;
        Node &rhs = node[1];
        auto rhs_type = preEvaluateType(rhs, context);// todo lol
        auto type = commonType(lhs_type, rhs_type);
        // todo: operators which are not endofunctions
        return type;
//        if(lhs.kind==arrays)
    }
    if (isGroup(node.kind)) {
//        arrayElementSize(node);// adds type as BAD SIDE EFFECT
//        if (use_wasm_arrays) {// todo
//        auto valueType = preEvaluateType(node.values(), context);
        auto valueType = commonElementType(node);
        if (valueType == unknown_type) return unknown_type;
        return genericType(node.kind, valueType);
//        }
    }
    if (node.kind == reference) {
        if (context.locals.has(node.name)) {
            auto local = context.locals[node.name];
            return local.type;
        }
        if (node.size() == 1 and use_wasm_arrays)// todo
            return preEvaluateType(node.first(), context);
    }
    return mapType(node);
}


Module &loadModule(String name) {
#if WASM
    todow("loadModule in WASM");
    return *new Module();
#else
    if (name == "wasp-runtime.wasm")
        return loadRuntime();
    return read_wasm(name);// we need to read signatures!
#endif
}

String &checkCanonicalName(String &name) {
    if (name == "**")info("The power operator in angle is simply '^' : 3^2=9.");// todo: alias infoing mechanism
    if (name == "^^")info("The power operator in angle is simply '^' : 3^2=9. Also note that 1 xor 1 = 0");
    if (name == "||")info("The disjunction operator in angle is simply 'or' : 1 or 0 = 1");
    if (name == "&&")info("The conjunction operator in angle is simply 'and' : 1 and 1 = 1");
    return name;
}


// √π -i ++j !true … not delete(x)
bool isPrefixOperation(Node &node, Node &lhs, Node &rhs) {
    if (prefixOperators.has(node.name)) {
//		if (infixOperators.has(node.name) or suffixOperators.has(node.name)) {
        if (lhs.kind == reference)return false; // i++
        if (isPrimitive(lhs))return false; // 3 -1
        if (lhs.kind == operators) return lhs.length == 0;
        if ((lhs.isEmpty() or lhs.kind == operators) and lhs.name != "‖")
            return true;
        return false;
    }
    return false;
}

Node &groupFunctionCalls(Node &expressiona, Function &context) {
    if (expressiona.kind == declaration)return expressiona;// handled before
    Function *import = findLibraryFunction(expressiona.name, false);
    if (import or isFunction(expressiona)) {
        expressiona.setType(call, false);
        if (not functions.has(expressiona.name))
            error("! missing import for function "s + expressiona.name);
//		if (not expressiona.value.node and arity>0)error("missing args");
        functions[expressiona.name].is_used = true;
    }

    for (int i = 0; i < expressiona.length; ++i) {
        Node &node = expressiona.children[i];
        String &name = node.name;
        if (name == "if") // kinda functor
        {
            auto args = expressiona.from("if");
            Node &iff = groupIf(node.length > 0 ? node.add(args) : args, context);
            int j = expressiona.lastIndex(iff.last().next) - 1;
            if (i == 0 and j == expressiona.length - 1)return iff;
            if (j > i)
                expressiona.replace(i, j, iff);// todo figure out if a>b c d e == if(a>b)then c else d; e boundary
            continue;
        }
        if (name == "while") {
            // todo: move into groupWhile
            if (node.length == 2) {
                node[0] = analyze(node[0], context);
                node[1] = analyze(node[1], context);
                continue;// all good
            }
            if (node.length == 1) {// while()… or …while()
                node[0] = analyze(node[0], context);
                Node then = expressiona.from("while");// todo: to closer!?
                int remaining = then.length;
                node.add(analyze(then, context).clone());
                expressiona.remove(i + 1, i + remaining);
                continue;
            } else {
                Node n = expressiona.from("while");
                Node &iff = groupWhile(n, context);// todo: sketchy!
                int j = expressiona.lastIndex(iff.last().next) - 1;// huh?
                if (j > i)expressiona.replace(i, j, iff);
            }
        }
        if (isFunction(node)) // needs preparsing of declarations!
            node.kind = call;
        if (node.kind != call)
            continue;

        Function *ok = findLibraryFunction(name, true);
        if (not ok and not functions.has(name))// todo load lib!
            error("missing import for function "s + name);
        Function &function = functions[name];
        Signature &signature = function.signature;
        function.is_used = true;

        int minArity = signature.size();// todo: default args!
        int maxArity = signature.size();

        if (node.length > 0) {
//			if minArity == …
            node = analyze(node.flat(), context);//  f(1,2,3) vs f([1,2,3]) ?
            continue;// already done how
        }
        if (minArity == 0)continue;
        Node rest;
        if (i < expressiona.length - 1 and expressiona[i + 1].kind == groups) {// f(x)
            // todo f (x) (y) (z)
            // todo expressiona[i+1].length>=minArity
            rest = expressiona[i + 1];
            if (rest.length > 1)
                rest.setType(expression);
            Node args = analyze(rest, context);
            node.add(args);
            expressiona.remove(i + 1, i + 1);
            continue;
        }
        rest = expressiona.from(i + 1);
        int arg_length = rest.length;
        if (not arg_length and rest.kind == reference) arg_length = 1;
        if (not arg_length and rest.value.data) arg_length = 1;
        if (arg_length > 1)
            rest.setType(expression);// SUUURE?
        if (rest.kind == groups or rest.kind == objects)// and rest.has(operator))
            rest.setType(expression);// todo f(1,2) vs f(1+2)
//		if (hasFunction(rest) and rest.first().kind != groups)
//				warn("Ambiguous mixing of functions `ƒ 1 + ƒ 1 ` can be read as `ƒ(1 + ƒ 1)` or `ƒ(1) + ƒ 1` ");
        if (rest.first().kind == groups)
            rest = rest.first();
        // per-function precedence does NOT really increase readability or bug safety
        if (rest.value.data) {
            maxArity--;// ?
            minArity--;
        }
        if (arg_length < minArity) {
            print(function);
            print((String) signature);
            error("missing arguments for function %s, given %d < expected %d. "
                  "defaults and currying not yet supported"s % name % arg_length % minArity);
        } else if (arg_length == 0 and minArity > 0)
            error("missing arguments for function %s, or to pass function pointer use func keyword"s % name);
//		else if (rest.first().kind == operators) { // random() + 1 == random + 1
//			// keep whole expressiona for later analysis in groupOperators!
//			return expressiona;
//		} else if (arg_length >= maxArity) {
        Node &args = analyze(rest, context);// todo: could contain another call!
        node.add(args);
        if (rest.kind == groups)
            expressiona.remove(i + 1, i + 1);
        else
            expressiona.remove(i + 1, i + arg_length);
//		} else
//			todo("missing arity match case");
    }
    return expressiona;
}


void addLibraryFunctionAsImport(Function &func) {
    func.is_used = true;
    if (func.is_declared)return;
    if (func.is_builtin)return;
//	auto function_known = functions.has(func.name);

    // ⚠️ this function now lives inside Module AND as import inside "wasp_main" functions list, with different wasm_index!
#if WASM
    Function& import=*new Function;
#else
    Function & import=functions[func.name];// copy function info from library/runtime to main module
#endif
//	if(function_known)
//        import = functions[func.name];
    if (import.is_declared)return;
    import.signature = func.signature;
    import.signature.type_index = -1;
    import.signature.parameter_types = func.signature.parameter_types;
    import.is_runtime = false;// because here it is an import!
    import.is_import = true;
    import.is_used = true;
#if WASM
//	if(!function_known)
		functions.add(func.name, import);
#endif
}

bool eq(Module *x, Module *y) { return x->name == y->name; }// for List: libraries.has(library)

// todo: clarify registerAsImport side effect
// todo: return the import, not the library function
Function *findLibraryFunction(String name, bool searchAliases) {
    if (name.empty())return 0;
    if (functions.has(name))return use_required(&functions[name]);
#if WASM
	if (loadRuntime().functions.has(name)){
		return use_required(&loadRuntime().functions[name]);
	}
#endif
    if (contains(funclet_list, name)) {
#if WASM
//        auto funclet = getWaspFunclet(name);// todo!
        todo("getWaspFunclet");
        return 0;
#else
        Module &funclet_module = read_wasm(findFile(name, "lib"));
//        check(funclet_module.functions.has(name));
        auto funclet = funclet_module.functions[name];
        addLibrary(&funclet_module);
        return use_required(&funclet);
#endif
    }
    if (name.in(function_list) and libraries.size() == 0)
        libraries.add(&loadModule("wasp-runtime.wasm"));// on demand

//	if(functions.has(name))return &functions[name]; // ⚠️ returning import with different wasm_index than in Module!
    for (Module *library: libraries) {//} module_cache.valueList()) {
        // todo : multiple signatures! concat(bytes, chars, …) eq(…)
        int position = library->functions.position(name);
        if (position >= 0) {
            Function &func = library->functions.values[position];
            return use_required(&func);
        }
    }
    Function *function = 0;
    if (searchAliases) {
        for (String alias: aliases(name)) {
            function = findLibraryFunction(alias, false);
            use_required(function);
        }
    }
    auto normed = normOperator(name);
    if (normed == name)
        return use_required(function);
    else
        return findLibraryFunction(normed, false);
}

void addLibrary(Module *modul) {
    if (not modul)return;
    for (auto lib: libraries)
        if (lib->name == modul->name)return;
    libraries.add(modul);// link it later via import or use its code directly?
}

Function *use_required(Function *function) {
    if (!function or not function->name or function->name.empty())
        return 0;// todo how?
    addLibraryFunctionAsImport(*function);
    if (function->name == "quit")
        functions["proc_exit"].is_used = true;
    if (function->name == "puts")
        functions["fd_write"].is_used = true;
    for (Function &variant: function->variants) {
        addLibraryFunctionAsImport(variant);
    }
    for (String &alias: aliases(function->name)) {
        auto ali = findLibraryFunction(alias, false);
        if (ali)addLibraryFunctionAsImport(*ali);
    }
    for (auto vari: function->variants) {
        vari.is_used = true;
    }
//    for(Function& dep:function.required)
//        dep.is_used = true;
    return function;
}

List<String> aliases(String name) {

    List<String> found;
#if MY_WASM
    return found;
#endif
//	switch (name) // statement requires expression of integer type
    if (name == "pow") {
        found.add("powi");
        found.add("pow_long");
    }
    if (name == "puti")
        found.add("_Z5printl");
    if (name == "len")
        found.add("strlen");
    if (name == "int" or name == "atoi" or name == "atol" or name == "parseInt")
        found.add("parseLong");
//        found.add("_Z7strlen0PKc");
    if (name == "atoi" or name == "int") {
        // todo type vs fun!
        found.add("_Z5atoi0PKc");
    }
    if (name == "concat") {// todo: programmatic!
        if (not use_wasm_strings)
            found.add("_Z6concatPKcS0_"); // this is the signature we call for concat(char*,char*) … todo : use String.+
    }
    if (name == "+") {
        found.add("add");
        found.add("plus");
        found.add("concat");
        if (not use_wasm_strings)
            found.add("_Z6concatPKcS0_"); // this is the signature we call for concat(char*,char*) … todo : use String.+
    }
    if (name == "eq") {
        if (not use_wasm_strings)
            found.add("_Z2eqPKcS0_i"); // eq(char const*, char const*, int)
//        found.add("_Z2eqR6StringPKc"); // eq(String&, char const*)
    }
    if (name == "=") {
//        found.add("is");
//        found.add("be");
    }
    if (name == "#") {// todo
//        found.add("getChar");
    }
    return found;
}

// todo: un-adhoc this!
Node &groupWhile(Node &n, Function &context) {
    if (n.length == 0 and !n.value.data)
        error("no if condition given");
    if (n.length == 1 and !n.value.data)
        error("no if block given");

    Node &condition = n.children[0];
    Node then;
    if (n.length == 0) then = n.values();
    if (n.length == 1) {
        if (n.next)
            then = *n.next;
            //		else if(previous)
//			then = previous
        else
            error("missing block for while statement");// should be in parser/analyzer or carry over code pointer!
    }
    if (n.length > 0) then = n[1];
    if (n.length >= 2 and !n.value.data) {
//		return n; // all good!
        condition = n[0];
        then = n[1];
    }

    // todo: UNMESS how? UNMESS by applying operator ":" first a/r grouping in valueNode
    if (n.has(":") /*before else: */) {
        condition = n.to(":");
        then = n.from(":");
    } else if (condition.has(":")) {// as child
        then = condition.from(":");
    }
    if (n.has("do")) {
        condition = n.to("do");
        then = n.from("do");
    }
    if (then.has("do"))
        then = n.from("do");

    if (condition.value.data and !condition.next)
        then = condition.values();
    if (condition.kind == reference) {
        // find better condition todo HOW TO UNMESS??
        for (Node &child: n) {
            if (child.kind == groups or child.kind == objects) {// while x y z {}
                condition = n.to(child);
                then = child;
                break;
            }
        }
    }

    Node *whilo = new Node("while");// regroup cleanly
    Node &ef = *whilo;
    ef.kind = expression;// todo no longer functor?
    //	ef.kind = ifStatement;
    //	if (condition.length > 0)condition.setType(expression);// so far treated as group! todo: expression should be ok even if it's group!
//	if (then.length > 0)then.setType(expression);// NO! it CAN BE A GROUP!, e.g. while(i++){log(1);put(2);}
//	ef.add(analyze(condition).clone());
//	ef.add(analyze(then).clone());
//	ef.length = 2;
    ef["condition"] = analyze(condition, context);
    ef["then"] = analyze(then, context);
    analyzed[ef.hash()] = 1;
    return ef;
}

//
//extern "C" Node *analyze(Node &node){
//	return &analyze(node, "wasp_main");
//}


Node &analyze(Node &node, Function &function) {
    String &context = function.name;
    if (context != "global" and !functions.has(context)) {
        function.is_declared = true;
        functions.add(context, function);// index not known yet
    }
    int64 hash = node.hash();
    if (analyzed.has(hash))
        return node;

    // data keyword leaves data completely unparsed, like lisp quote `()
    auto first = node.first().name;
    if (first == "data" or first == "quote")
        return node;

    // group: {1;2;3} ( 1 2 3 ) expression: (1 + 2) tainted by operator
    Kind type = node.kind;
    String &name = node.name;

    if (name == "if")return groupIf(node, function);
    if (name == "while")return groupWhile(node, function);
    if (name == "?")return groupIf(node, function);
    if (name == "module") {
        if (!module)module = new Module();
        module->name = node.string(); // todo: use?
        return NUL;
    }
#if not WASM
    if (not first.empty() and class_keywords.contains(first))
        return classDeclaration(node, function);
#endif
    // if(function_operators.contains(name))...
    if (node.kind == key and node.values().name == "func")
        return funcDeclaration(node.name, node.values(), NUL /* no body here */ , 0, function.module);
//        return witReader.analyzeWit(node);
    // add: func(a: float32, b: float32) -> float32

    if ((type == expression and not name.empty() and not name.contains("-")))
        addLocal(function, name, int32, false);//  todo deep type analysis x = π * fun() % 4
    if (type == key) {
        if (node.value.node /* i=ø has no node */)
            node.value.node = analyze(*node.value.node, function).clone();
        if (node.length > 0) {
            // (double x, y)  (while x) : y
            auto first = node.first().first();
            if (isType(first))
                return groupTypes(node, function);
            else if (first.name == "while")
                return groupWhile(node, function);
            else if (first.name == "if")
                return groupIf(node, function);
            else if (node.length > 1)
                error("unknown key expression: "s + node.serialize());
        }
        addLocal(function, name, node.value.node ? mapType(node.value.node) : none, false);
    }
    if (isPrimitive(node)) {
        if (isVariable(node))
            addLocal(function, name, mapType(node), false);
        return node;// nothing to be analyzed!
    }


//    if(function_keywords.contains(first))
    if (node.containsAny(function_keywords))
        return groupFunctionDefinition(node, function);

    //todo merge/clean
    bool is_function = isFunction(node); // call, NOT definition
    if (type == operators or type == call or is_function) {
//		Function *import =
        findLibraryFunction(node.name, true);// sets functions[name].is_import = true;
        if (is_function and type != operators) {
            node.kind = call;
        }
        Node &grouped = groupOperators(node, function);// outer analysis id(3+3) => id(+(3,3))
        if (grouped.length > 0)
            for (Node &child: grouped) {// inner analysis while(i<3){i++}
//				if (&child == 0)continue;
                child = analyze(child, function);// REPLACE with their ast
            }
        if (is_function)
            functions[name].is_used = true;
        return grouped;
    }

    Node &groupedTypes = groupTypes(node, function);
    if (isPrimitive(node)) return node;
    Node groupedDeclarations = groupDeclarations(groupedTypes, function);
    Node &groupedFunctions = groupFunctionCalls(groupedDeclarations, function);
    Node &grouped = groupOperators(groupedFunctions, function);
    if (analyzed[grouped.hash()])return grouped;// done!
    analyzed.insert_or_assign(grouped.hash(), 1);

    if (isGroup(type)) {
        // children of lists analyzed individually
        if (grouped.length > 0)
            for (Node &child: grouped) {
                if (!child.name.empty() and wit_keywords.contains(child.name) and child.kind != strings /* TODO … */)
                    return witReader.analyzeWit(node);// can't MOVE there:
                child = analyze(child, function);// REPLACE ref with their ast ok?
            }
    }
    return grouped;
//	return *grouped.clone();// why?? where is leak?
}


extern "C" int64 run_wasm_file(chars file) {
    let buffer = load(String(file));
#if RUNTIME_ONLY
    error("RUNTIME_ONLY");
    return -1;
#else
    return run_wasm((bytes) buffer.data, buffer.length);
#endif
}

void fixFunctionNames() {
    for (String name: functions) {
        functions[name].name = name;
    }
}

void preRegisterFunctions() {
    functions.clear();
//  postpone for simple programs!
//    Module &runtime = read_wasm("wasp-runtime.wasm"); // ok, cached!
//    runtime.code.needs_relocate = false; // may be set to true depending on main code emitted
//
//    for (int i = 0; i < runtime.functions.size(); ++i)
//        runtime.functions.values[i].is_used = false; // reset after last used, as libraries are shared between tests

    functions["proc_exit"].import();
    functions["proc_exit"].signature.add(int32, "exit_code");// file descriptor
//    functions["proc_exit"].module = new Module{.name="wasi_unstable"};
    functions["proc_exit"].module = new Module{.name="wasi_snapshot_preview1"}; // f'ing wasmedge can't wasi_unstable

    functions["fd_write"].import();
    functions["fd_write"].signature.add(int32, "fd");// file descriptor
    functions["fd_write"].signature.add((Type) pointer, "iovs");
    functions["fd_write"].signature.add(size32, "iovs_len");
    functions["fd_write"].signature.add((Type) pointer, "nwritten");// size_t *  out !
    functions["fd_write"].signature.returns(int32);
//    functions["fd_write"].module=new Module{.name="wasi"};
//    functions["fd_write"].module = new Module{.name="wasi_unstable"};
    functions["fd_write"].module = new Module{.name="wasi_snapshot_preview1"};

    functions["puts"].builtin();
    functions["puts"].signature.add((Type) stringp).returns(int32);// stdio conform!!

    functions["len"].builtin();// via wasp abi len(any)=*(&any)[1]
    functions["len"].signature.add((Type) array).returns(int32);// todo any wasp type

    functions["quit"].builtin();// no args, calls proc_exit(0)

// TESTS! not useful otherwise!
//    functions["square"].import().signature.add(int32).returns(int32);// test only!!

#if MULTI_VALUE
    functions["wasp_main"].signature.returns(i64).returns(i32);// [result, type32] transparently (no flipped stack order)
#else
    functions["wasp_main"].signature.returns(i64);
#endif
//    functions["paint"].import().signature.returns(voids);// paint surface
//    functions["init_graphics"].import().signature.returns(pointer);// surface

    // BUILTINS
//    functions["nop"].builtin(); NOT A FUNCTION! an op
    functions["id"].builtin().signature.add(i32t).returns(i32t);
    functions["modulo_float"].builtin().signature.add(float32).add(float32).returns(float32);
    functions["modulo_double"].builtin().signature.add(float64).add(float64).returns(float64);

    fixFunctionNames();
}

void clearAnalyzerContext() {
//    clearEmitterContext()
//	needs to be outside analyze, because analyze is recursive
#ifndef RUNTIME_ONLY
    libraries.clear();// todo: keep runtime or keep as INACTIVE to save reparsing
//    module_cache.clear(); NOO not the cache lol
    types.clear();
    globals.clear();
    call_indices.clear();
    call_indices.setDefault(-1);
    functions.clear();
    analyzed.clear();// todo move much into outer analyze function!
    functions.clear();// always needs to be followed by
    preRegisterFunctions();// BUG Signature wrong cpp file
#endif
}


// emit via library merge
Node runtime_emit(String prog) {
#ifdef RUNTIME_ONLY
    printf("emit wasm not built into release runtime");
    return ERROR;
#endif
    libraries.clear();// todo reuse
    clearAnalyzerContext();
    clearEmitterContext();
    Module &runtime = loadRuntime();
    runtime.code.needs_relocate = false;
    Code code = compile(prog, false);// should use libraries!
    code.needs_relocate = false;
    code.save("merged.wasm");
    int64 result_val = code.run();// todo parse stdout string as node and merge with emit() !
    return *smartNode(result_val);
}


// smart pointers returned if ABI does not allow multi-return, as in int main(){}

Node smartNode32(int smartPointer32) {
    auto result = smartPointer32;
    auto smart_pointer = result & 0x00FFFFFF;// data part
    if ((result & 0xF0000000) == array_header_32 /* and abi=wasp */ ) {
        // smart pointer to smart array
        int *index = ((int *) wasm_memory) + smart_pointer;
        int kind = *index++;
        if (kind == array_header_32)
            kind = *index++;
        int len = *index++; // todo: leb128 vector later
        Node arr = Node();
//		arr.kind.value = kind;
        int pos = 0;
        while (len-- > 0) {
            auto val = index[pos++];
            arr.add(new Node(val));
        }
        arr.kind = objects;
//			check_eq(arr.length,len);
//			check(arr.type=…
        return arr;
    }
    if ((result & 0xF0000000) == string_header_32 /* and abi=wasp */ ) {
        // smart pointer for string
        return Node(((char *) wasm_memory) + smart_pointer);
    }
    error1("missing smart pointer type "s + typeName(Type(smartPointer32)));
    return Node();
}

float precedence(Node &operater) {
    String &name = operater.name;
//	if (operater == NIL)return 0; error prone
    if (operater.kind == reals)return 0;//;1000;// implicit multiplication HAS to be done elsewhere!
    if (operater.kind == longs)return 0;//;1000;// implicit multiplication HAS to be done elsewhere!
    if (operater.kind == strings)return 0;// and empty(name)

    // todo: make live easier by making patterns only operators if data on the left
    if (operater.kind == patterns and operater.parent) return 98;// precedence("if") * 0.98
    //	todo why do groups have precedence again?
//	if (operater.kind == groups) return 99;// needs to be smaller than functor/function calls
    if (operater.name.in(function_list))return 999;// function call todo: remove here
    if (empty(name))return 0;// no precedence
    return precedence(name);
}

List<String> reserved_keywords = {"func"};// todo…
void addGlobal(Node &node) {
    String &name = node.name;
    if (reserved_keywords.has(name) or contains(import_keywords, name))
        error("Can't add reserved keyword "s + name);
    if (not globals.has(name)) {
        globals.add(name, Global{/*id*/globals.count(), name, mapType(node.type), node.clone(), /*mutable=*/ true});
    } else {
        warn("global %s already a registered symbol: %o "s % name % globals[name].value);
    }
}


List<String> demangle_args(String &fun);

Function getWaspFunction(String name) {
    if ("floor"s == name)error1("use builtin floor!");


    if (loadRuntime().functions.has(name)) {
//        print("already got function "s+name);
        return loadRuntime().functions[name];
    }
    Function f{.name=name, .is_import=true, .is_runtime=true};
    if (name.contains("(")) {
        String brace = name.substring(name.indexOf('(') + 1, name.indexOf(')'));
        f.name = name.substring(0, name.indexOf('('));
        auto args = brace.split(", ");
        for (auto arg: args)
            f.signature.add(mapType(arg));
        if (f.name == "square")f.signature.returns(int32);
        if (f.name == "pow")f.signature.returns(float64);
//        auto returnType = getReturnType(name);
//        f.signature.returns(returnType);
    } else {
        if (name == "_start");
        else if (name == "__wasm_call_ctors");
        else if (name == "__cxa_allocate_exception");
        else if (name == "__cxa_throw");
        else if (name == "__cxa_end_catch");
        else if (name == "__cxa_find_matching_catch");
        else if (name == "__cxa_begin_catch");
        else if (name == "__cxa_atexit");
        else if (name == "__main_argc_argv")
            f.signature.add(int32, name = "argc").add(i32, name = "argv").returns(int32);
        else if (name == "getField")f.signature.add(node_pointer).add(smarti64).returns(node);// wth
        else if (name == "smartNode")f.signature.add(int64s).returns(node);
        else if (name == "pow")f.signature.add(float64).add(float64).returns(float64);
        else if (name == "kindName")f.signature.add(type32).returns(charp);
        else if (name == "formatReal")f.signature.add(float64).returns(charp);
        else if (name == "strlen")f.signature.add(charp).returns(int32);
        else if (name == "free")f.signature.add(pointer);
        else if (name == "square")f.signature.add(int32).returns(int32);
        else if (name == "malloc")f.signature.add(size32).returns(pointer);// todo 64
        else if (name == "aligned_alloc")f.signature.add(size32, name = "alignment").add(size32).returns(pointer);
        else if (name == "realloc")f.signature.add(pointer).add(size32).returns(pointer);
        else if (name == "calloc")f.signature.add(size32).add(size32).returns(pointer);
        else if (name == "memcpy")f.signature.add(pointer).add(pointer).add(size32);
        else if (name == "memset")f.signature.add(pointer).add(int32).add(size32);
        else if (name == "memcmp")f.signature.add(pointer).add(pointer).add(size32).returns(int32);
        else if (name == "memmove")f.signature.add(pointer).add(pointer).add(size32);
        else if (name == "memchr")f.signature.add(pointer).add(int32).add(size32).returns(pointer);
        else if (name == "strchr")f.signature.add(charp).add(int32).returns(charp);
        else if (name == "strrchr")f.signature.add(charp).add(int32).returns(charp);
        else if (name == "strcat")f.signature.add(charp).add(charp).returns(charp);
        else if (name == "strncat")f.signature.add(charp).add(charp).add(size32).returns(charp);
        else if (name == "strcmp")f.signature.add(charp).add(charp).returns(int32);
        else if (name == "strncmp")f.signature.add(charp).add(charp).add(size32).returns(int32);
        else if (name == "strcpy")f.signature.add(charp).add(charp).returns(charp);
        else if (name == "strncpy")f.signature.add(charp).add(charp).add(size32).returns(charp);
        else if (name == "strdup")f.signature.add(charp).returns(charp);
        else if (name == "strndup")f.signature.add(charp).add(size32).returns(charp);
        else if (name == "strpbrk")f.signature.add(charp).add(charp).returns(charp);
        else if (name == "strspn")f.signature.add(charp).add(charp).returns(size32);
        else if (name == "strcspn")f.signature.add(charp).add(charp).returns(size32);
        else if (name == "strstr")f.signature.add(charp).add(charp).returns(charp);
        else if (name == "strtok")f.signature.add(charp).add(charp).returns(charp);
        else if (name == "getchar")f.signature.returns(int32);
        else if (name == "get_char")f.signature.returns(int32);
        else if (name == "get_int")f.signature.returns(int32);
        else if (name == "get_float")f.signature.returns(float32);
        else if (name == "get_double")f.signature.returns(float64);
        else if (name == "str")f.signature.add(charp).returns(strings);
        else if (name == "puts")f.signature.add(charp);
        else if (name == "putx")f.signature.add(int32);
        else if (name == "putf")f.signature.add(float32);
        else if (name == "putd")f.signature.add(float64);
        else if (name == "putp")f.signature.add(pointer);
        else if (name == "putl")f.signature.add(longs);
        else if (name == "printf")f.signature.add(charp);// todo …
        else if (name == "printNode")f.signature.add(node_pointer);
        else if (name == "put_chars")f.signature.add(charp);
        else if (name == "putchar")f.signature.add(int32);
        else if (name == "put_char")f.signature.add(int32);
        else if (name == "put_string")f.signature.add(charp);
        else if (name == "put_int")f.signature.add(int32);
        else if (name == "put_float")f.signature.add(float32);
        else if (name == "put_double")f.signature.add(float64);
        else if (name == "size_of_node")f.signature.returns(size32);
        else if (name == "size_of_string")f.signature.returns(size32);
        else if (name == "serialize")f.signature.add(node_pointer);// also Node::serialize
        else if (name == "raise")f.signature.add(charp);
        else if (name == "Parse")f.signature.add(charp).returns(node_pointer);
        else if (name == "run")f.signature.add(charp).returns(charp);
        else if (name == "system")f.signature.add(charp).returns(int32);
        else if (name == "testCurrent");
        else if (name == "run_wasm_file")f.signature.add(charp).returns(smarti64);
        else if (name == "panic");
            // IGNORE js bridges :
        else if (name == "registerWasmFunction");
        else if (name.startsWith("test"));
        else todo("getWaspFunction "s + name);
    }
    if (!loadRuntime().functions.has(f.name)) {
        loadRuntime().functions.add(f.name, f);
    }
//    else todo("getWaspFunction "s + name);
//    else if(name=="powi")f.signature.add(int32).add(int32).returns(int64s);
    return f;
}

extern "C" void registerWasmFunction(chars name, chars mangled) {
    if ("floor"s == name)return; // use builtin!
    getWaspFunction(name);
//    if (!functions.has(name))functions.add(name, getWaspFunction(name));
//    if (!loadRuntime().functions.has(mangled))
//	    loadRuntime().functions.add(mangled, getWaspFunction(name));
}