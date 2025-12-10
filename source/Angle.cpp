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
//#import "wasm_helpers.h" // IMPORT so that they don't get mangled!
#include "wasm_helpers.h" // IMPORT so that they don't get mangled! huh?
#include "wasm_emitter.h"
#include "WitReader.h"

#ifndef RUNTIME_ONLY
#if INCLUDE_MERGER

#include "wasm_merger.h"

#endif
#endif

extern int __force_link_parser_globals;
int* __force_link_parser_globals_ptr = &__force_link_parser_globals;

Module *module; // todo: use?
bool use_interpreter = false;
Node &result = *new Node();
WitReader witReader;

List<String> aliases(String name);

Map<String, Node *> types = {100}; // builtin and defined Types
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

//Map<String, Function> functions; // todo Maps don't free memory and cause SIGKILL after some time <<<
Map<String, Function> functions = {1000};
// todo ONLY emit of main module! for funcs AND imports, serialized differently (inline for imports and extra functype section)
//Map<String, Function> library_functions; see:
List<Module *> libraries; // used modules from (automatic) import statements e.g. import math; use log; …  ≠
// functions of preloaded libraries are found WITHOUT `use` `import` statement (as in Swift) !

Map<int64, bool> analyzed = {1000};
// avoid duplicate analysis (of if/while) todo: via simple tree walk, not this!


// todo : use proper context ^^ instead of:
//Map<String /*function*/, List<String> /* implicit indices 0,1,2,… */> locals;
//Map<String /*function*/, List<Valtype> /* implicit indices 0,1,2,… */> localTypes;

Map<String, Global> globals;
//List<Global> globalVariables;

short arrayElementSize(Node &node);

Function *use_required_import(Function *function);

void addLibrary(Module *modul);

// 'private header'
bool addLocal(Function &context, String name, Type Type, bool is_param); // must NOT be accessible from Emitter!

Node &groupOperators(Node &expression, Function &context);

Node &groupKebabMinus(Node &node, Function &function);

Node extractModifiers(Node &node);

void useFunction(String name) {
    if (not functions.has(name)) {
        auto fun = findLibraryFunction(name, true); // search aliases
        if (not fun)
            error("function "s + name + " not found"s);
    }
    trace("useFunction: "s + name);
    functions[name].is_used = true;
}

// todo: merge ^^
bool addGlobal(Function &context, String name, Type type, bool is_param, Node *value) {
    if (isKeyword(name))
        error("keyword as global name: "s + name);
    if (name.empty()) {
        warn("empty reference in "s + context);
        return true; // 'done' ;)
    }
    if (builtin_constants.has(name))
        error(name + " is already a builtin constant"s);
    if (context.signature.has(name))
        error(name + " already declared as parameter for function "s + context.name);
    if (globals.has(name)) // ok ?
        error(name + " already declared as global"s);
    //        return true;// already declared
    if (context.locals.has(name))
        error(name + " already declared as local variable"s);
    if (isFunction(name, true))
        error(name + " already declared as function"s);
    //    if(type == int32)
    //        type = int64t; // can't grow later after global init
    if (value and value->kind == expression) {
        warn("only the most primitive expressions are allowed in global initializers => move to wasp_main!");
        //        value = new Node(0); // todo reals, strings, arrays, structs, …
    } else if (not value) {
        //        value = new Node(0);
    } else {
        //        if(type==int64t)type=int32t;// todo: dirty hack for global x=1+2 because we can't cast
    }
    Global global{.index = globals.count(), .name = name, .type = type, .value = value, .is_mutable = true};
    globals[name] = global;
    return true;
}


// currently only used for WitReader. todo: merge with addGlobal
void addGlobal(Node &node) {
    String &name = node.name;
    if (isKeyword(name))
        error("Can't add reserved keyword "s + name);
    if (globals.has(name)) {
        warn("global %s already a registered symbol: %o "s % name % globals[name].value);
    } else {
        Type type = mapType(node.type);
        globals.add(name, Global{
                        /*id*/globals.count(), name, type, node.clone(), /*mutable=*/ true
                    });
    }
}

void addWasmArrayType(Type value_type) {
    if (isGeneric(value_type)) {
        addWasmArrayType(value_type.generics.value_type);
        return;
    }
    auto array_type_name = String(typeName(value_type)) + "s"; // int-array -> “ints”
    Node array_type;
    array_type.kind = arrays;
    array_type.type = types[typeName(value_type)]; // todo more robust
    types.add(array_type_name, array_type.clone());
    // link to type index later
}


Node &getType(Node &node) {
    String name = node.name;
    bool vector = false;
    if (name.endsWith("es")) {
        // addresses
        // todo: cities …
        name = name.substring(0, -3);
        vector = true;
    } else if (name.endsWith("s") and not(name == "chars")) {
        // ints …
        name = name.substring(0, -2);
        vector = true;
    }
    if (types.has(name)) {
        auto &pNode = types[name];
        if (not pNode)
            error1("getType: types corruption: type %s not found (NULL)", name);
        return *pNode;
        //        typ.kind = clazz;
    }
    Node &typ = *new Node(name);
    typ.kind = clazz;
    types[name] = &typ;
    if (vector) {
        // holup typ.kind = arrays needs to be applied to the typed object!
        typ.kind = arrays;
        typ.type = typ.clone();
        //		typ.kind=vectors; // ok, copy value
    }
    return typ;
}

bool isPlural(Node &word) {
    auto &name = word.name;
    static List<String> plural_exceptions = {
        "flags", "puts", "plus", "minus", "times", "is", "has", "was", "does",
        "equals"
    }; // chars?
    if (plural_exceptions.contains(name)) return false;
    if (name.endsWith("s"))return true;
    return false;
}

bool isType(Node &expression) {
    auto &name = expression.name;
    if (expression.kind == functor) //todo ...
        return false;
    if (expression.kind == operators)
        return false;
    if (isPrimitive(expression))
        return false;

    if (name.empty())return false;
    //    if (isPlural(expression))// very week criterion: houses=[1,2,3]
    //        return true;
    Type type = mapType(name, false);
    if (type == none || type == unknown_type)
        return types.has(name);
    else {
        tracef("typeName %s\n", typeName(type));
        return true;
    }
    //	if (not types.has(name))
    //		error1("isType: type %s not found"s% name);
    return types.has(name);
    //		types.add(name, &expression);
    //	return true;
}


Node constants(Node n) {
    if (eq(n.name, "not"))return True; // not () == True; hack for missing param todo: careful!
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
    if (functions.has(op))return true; // pre registered signatures
    if (op.in(function_list))
        return true;
    if (deep_search and findLibraryFunction(op, true))
        return true; // linear lookup ok as long as #functions < 1000 ? nah getting SLOW QUICKLY!!
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

Code &compile(String code, bool clean = true); // exposed to wasp.js
#endif

void debug_wasm_file() {
#if not WASM and not RELEASE
    print("validate-main.sh");
    print("+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++");
    system("./validate-main.sh");
    auto errors = readFile("main.wasm.validation");
    print(errors);
    print("+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++");
    if (strlen(errors) > 0) {
        exit(1);
    }
#endif
}

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
#if WEBAPP
        // errors are not forwarded to js, so catch them here! todo WHY NOT?
        try {
#endif
        Code &binary = compile(code, true);
        binary.save(); // to debug
#if NO_TESTS and not RELEASE
        debug_wasm_file(); // SLOW! use only to debug single file
#endif
        smart_pointer_64 results = binary.run();
        auto _resultNode = smartNode(results);
        if (!_resultNode)return ERROR;
        Node &resultNode = *_resultNode;
#if not RELEASE and not MY_WASM
        print("» %s\n"s % resultNode.serialize().data);
#endif
        return resultNode;
#if WEBAPP
        } catch (chars err) {
            print("eval FAILED WITH internal ERROR\n");
            printf("%s\n", err);
        } catch (String &err) {
            print("eval FAILED WITH ERRORs\n");
            printf("%s\n", err.data);
        } catch (SyntaxError &err) {
            print("eval FAILED WITH SyntaxError\n");
            printf("%s\n", err.data);
        } catch (...) {
            print("eval FAILED WITH UNKNOWN ERROR\n");
        }
        return ERROR;
#endif
    }
#endif
}

Signature &groupFunctionArgs(Function &function, Node &params) {
    //			left = analyze(left, name) NO, we don't want args to become variables!
    List<Arg> args;

    Node *nextType = &DoubleType;
    //    Node *nextType = 0;//types preEvaluateType(params, &function);
    // todo: dynamic type in square / add1 x:=x+1;add1 3
    if (params.length == 0) {
        params = groupTypes(params, function);
        if (params.name != function.name and not params.name.empty())
            args.add({function.name, params.name, params.type ? params.type : nextType});
    } //else
    for (Node &arg: params) {
        if (arg.kind == groups) {
            arg = groupTypes(arg, function,true);
            args.add({function.name, arg.name, arg.type ? arg.type : mapType(nextType), params});
            continue;
        }
        if (isType(arg)) {
            if (args.size() > 0 and args.last().type != unknown_type)
                args.last().type = types[arg.name];
            else nextType = &arg;
        } else {
            if (arg.name != function.name)
                args.add({function.name, arg.name, arg.type ? arg.type : mapType(nextType), params});
        }
    }

    Signature signature = function.signature;
    auto already_defined = function.signature.size() > 0;
    if (function.is_polymorphic or already_defined) {
        signature = *new Signature();
    }
    for (Arg arg: args) {
        auto name = arg.name;
        if (empty(name))
            name = String("$"s + signature.size());
        //            error("empty argument name");
        if (function.locals.has(name)) {
            // TODO
            warn("Already declared as local OR duplicate argument name: "s + name);
            //			error("duplicate argument name: "s + name);
        }
        signature.add(arg.type, name); // todo: arg type, or pointer
        addLocal(function, name, arg.type, true);
    }

    Function *variant = &function;
    if (already_defined /*but not yet polymorphic*/) {
        if (signature == function.signature)
            return function.signature; // function.signature; // ok compatible
        // else split two variants
        Function new_variant = function;
        Function old_variant = function; // clone by value!
        check_is(old_variant.name, function.name);
        old_variant.signature = function.signature;
        new_variant.signature = signature; // ^^ different
        function.is_polymorphic = true;
        //			function.is_used … todo copy other attributes?
        function.signature = *new Signature(); // empty
        function.variants.add(old_variant.clone());
        function.variants.add(new_variant.clone());
    } else if (function.is_polymorphic) {
        // third … variant
        variant = new Function();
        for (Function *fun: function.variants)
            if (fun->signature == signature)
                return fun->signature; // signature;
        variant->name = function.name;
        variant->signature = signature;
        function.variants.add(variant);
    } else if (!already_defined) {
        function.signature = signature;
    }
    for (auto arg: args) {
        auto name = arg.name;
        if (empty(name)) {
            warn("empty argument name, using $n"s);
            name = String("$"s + variant->locals.size());
        }
        addLocal(*variant, name, arg.type, true);
    }
    // NOW add locals to function context:
    //	for (auto arg: variant->signature.parameters)
    //		addLocal(*variant, arg.name, arg.type, true);
    return variant->signature;
    //	return signature;
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

    if (n.has(":") /*before else: */) {
        // todo remove since ":" is always parsed immediate (right?)
        condition = n.to(":");
        if (condition.has("else"))
            condition = condition.to("else"); // shouldn't happen?
        if (then.length == 0)
            then = n.from(":");
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
    if (then.name == ":") {
        // todo remove since ":" is always parsed immediate (right?)
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
    if (condition.length > 0)condition.setKind(expression); // so far treated as group!
    if (then.length > 0)then.setKind(expression);
    if (otherwise.length > 0)otherwise.setKind(expression);

    ef["condition"] = analyze(condition, context);
    ef["then"] = analyze(then, context);
    ef["else"] = analyze(otherwise, context);
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
        if (name.endsWith("=")) // += etc
            operators.add(name);
        else if (prefixOperators.has(name)) {
            if (name == "-" and is_operator(previous.codepointAt(0)))
                operators.add(name + "…"); //  -2*7 ≠ 1-(2*7)!
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
        previous = name;
    }
    auto by_precedence = [](String &a, String &b) { return precedence(a) > precedence(b); };
    operators.sort(by_precedence);
    return operators;
}


bool maybeVariable(Node &node) {
    if (node.kind != reference and node.kind != key and !node.isSetter())
        return false;
    if (node.kind == strings)return false;
    return /*node.parent == 0 and*/ not node.name.empty() and node.name[0] >= 'A'; // todo;
}

bool isGlobal(Node &node, Function &function) {
    if (not node.name.empty()) {
        if (globals.has(node.name))
            return true;
        if (builtin_constants.contains(node.name))
            return true;
    }
    if (node.first().name == "global")
        return true;
    return false;
}

bool isPrimitive(Node &node) {
    // should never be cloned so always compare by reference ok?
    if (&node == NULL)return false;
    if (&node == &IntegerType)return true;
    if (&node == &BoolType)return true;
    if (&node == &LongType)return true;
    if (&node == &DoubleType)return true;
    if (&node == &ByteType)return true;
    if (&node == &ByteCharType)return true;
    if (&node == &CodepointType)return true;
    if (&node == &ShortType)return true;
    if (&node == &ByteCharType)return true;
    if (&node == &StringType)return true; // todo uh not really primitive!?
    Kind type = node.kind;
    if (type == longs or type == strings or type == reals or type == bools or type == arrays or type == buffers)
        return true;
    if (type == codepoint1) // todo ...?
        return true;
    return false;
}

// todo: see NodeTypes.h for overlap with numerical returntype integer …
// these are all boxed class types, for primitive types see Type and Kind
void initTypes() {
    if (types.size() > 3)return;
    types.add("i8", &ByteType); // use in u8.load etc
    types.add("u8", &ByteType); // use in u8.load etc
    types.add("int8", &ByteType); // use in u8.load etc
    types.add("uint8", &ByteType); // use in u8.load etc
    types.add("byte", &ByteType); // use in u8.load etc
    //    types.add("sint8", &IntegerType); NOT MAPPED until required
    //    types.add("sint", &IntegerType);

    //    types.add("char", &Byte);
    types.add("chars", &ByteCharType);

    types.add("char", &CodepointType); // todo : warn about abi conflict? CAN'T USE IN STRUCT
    types.add("character", &CodepointType);
    types.add("charpoint", &CodepointType);
    types.add("codepoint", &CodepointType);
    types.add("i32", &IntegerType);
    types.add("int", &IntegerType);
    types.add("int32", &IntegerType);
    types.add("integer", &IntegerType);
    types.add("long", &LongType);
    types.add("double", &DoubleType);
    types.add("float", &DoubleType);
    types.add("real", &DoubleType); // number
    types.add("number", &DoubleType); // todo!
    types.add("string", &StringType);
    for (int i = 0; i < types.size(); ++i) {
        auto typ = types.values[i];
        if (int64(typ) < 0 or int64(typ) > 10000000l)
            continue; // todo: this type reflection is bad anyways?
        //            error("bad wasm type initialization");
        //        if(typ)typ->setType(clazz);
    }
}


Node &constructInstance(Node &node, Function &function);

Node &groupTypes(Node &expression, Function &context , bool as_param) {
    // todo delete type declarations double x, but not double x=7
    // todo if expression.length = 0 and first.length=0 and not next is operator return ø
    // ways to set type:
    /*
     * int x
     * x:int
     * x=7  via pre-evaluation of rest!!!
     * x int    unstable, discouraged!
     * x::int clear verbosive not yet implemented
     * */
    if (expression.kind == declaration)
        return expression; // later
    if (types.size() == 0)initTypes();

    //	Node typed_list;
    for (int i = 0; i < expression.length; i++) {
        Node &node = expression.children[i];
        if (node.name == "as") {
            i++; // skip type declaration
            expression[i].setKind(clazz, false);
            continue;
        }
        if (not isType(node))
            continue;
        //			if (operator_list.has(typed.name))
        //				continue; // 3.3 as int …
        if (not types.has(node.name))continue;
        auto aType = types[node.name];
        if (not aType)continue;
        expression.remove(i, i); // move type to type field of instance

        Node *typed = 0;

        if (i < expression.length and not is_operator(expression.children[i].name[0])) {
            typed = &expression.children[i];
        }
        while (typed and (isPrimitive(*typed) or
                          (typed->kind == reference and typed->length == 0))) {
            // BAD criterion for next!
            typed->type = aType; // ref ok because types can't be deleted ... rIgHt?
            if (typed->kind == reference or typed->isSetter())
                addLocal(context, typed->name, mapType(aType->name), as_param);
            // HACK for double x,y,z => z.type=DoubleType !
            if (i + 1 < expression.length)
                typed = &expression[++i];
            else if (typed->next) typed = typed->next;
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

    node.type = &type; // point{1 2} => Point(1,2)
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
    if (type == undefined)return; // no type given, ok
    Local &local = context.locals[name];
    auto type_name = typeName(type);
    auto oldType = local.type;
    if (oldType == none or oldType == unknown_type) {
        local.type = type;
    } else if (oldType != type and type != void_block and type != voids and type != unknown_type) {
        if (use_wasm_arrays) {
            // TODO: CLEANUP
            if (oldType == node) {
                addWasmArrayType(type);
                local.type = type; // make more specific!
            } else if (isGeneric(oldType)) {
                auto kind = oldType.generics.kind;
                auto valueType1 = oldType.generics.value_type;
                if (valueType1 != type) todow(
                    "generic type mismatch "s + typeName(oldType) + " vs " + type_name);
            } else if (oldType == wasmtype_array or oldType == array) {
                addWasmArrayType(type);
                local.type = genericType(array, type);
            }
        } else {
            if (!compatibleTypes(oldType, type)) {
                compatibleTypes(oldType, type);
                error("local "s + name + " in context %s already known "s % context.name + " with type " +
                    typeName(oldType) + ", ignoring new type " + type_name);
            }
        }
    }
    // ok, could be cast'able!
}

bool compatibleTypes(Type type1, Type type2) {
    if (type1 == type2)return true;
    if (type1 == longs and type2 == strings)return false; // upcast
    if (type1 == string_struct and type2 == strings)return true;
    if (type1 == stringp and type2 == strings)return true;
    if (type1 == wasm_int32 and type2 == wasm_int64)return true; // upcast
    if (type1 == externref and type2 == stringp)return true; // only via toString(externref) !!!
    if (type1 == ints and type2 == externref)return true; // via toLong(externref)
    if (type1 == reals and type2 == externref)return true; // via toReal(externref)
    if (type1 == stringp and type2 == externref)return true; // assume everything is castable from toString(externref)
    // if (type1 == strings and type2 == externref)return true; // assume everything is castable from toString(externref)
    // if (type1 == string_struct /*strings*/ and type2 == externref)return true; // assume everything is castable from externref
    // if (type2 == externref)return true; // assume everything is castable from externref
    return false;
}

Node &groupGlobal(Node &node, Function &function) {
    // todo: turn wasp_main variables into global variables
    if (node.first().name == "global")
        node = node.from(1);
    if (node.length > 1) {
        node = node.flat();
        if (node.first().kind == reference)
            node.first().setKind(global, false);
        else
            error("global declaration not a reference "s + node.first());
        Node &grouped = groupOperators(node, function).flat();
        //        Node &grouped = analyze(node, function).flat();
        //        if (grouped.kind != operators)
        //            error("global declaration not an assignment "s + grouped);
        //        Node *type = grouped[1].type;
        //        Type type = mapType(grouped[1]);
        Type type = preEvaluateType(grouped[1], function);
        addGlobal(function, grouped[0].name, type, false, grouped[1].clone());
        return grouped;
    } else {
        if (not globals.has(node.name) and not builtin_constants.contains(node.name))
            addGlobal(function, node.name, unknown_type, false, 0);
        // todo update type later
    }
    node.setKind(global, false);
    //    node.setType(reference, false);
    return node;
}


// return: done?
// todo return clear enum known, added, ignore ?
bool addLocal(Function &context, String name, Type type, bool is_param) {
    if(not name)
        error("addLocal: empty name");
    if (isKeyword(name))
        error("keyword as local name: "s + name);
    // todo: kotlin style context sensitive symbols!
    if (builtin_constants.has(name))
        return true;
    if (types.has(name))
        // error("type as local name: "s + name);
        return true; // already declared as type, e.g. Point
    if (context.signature.has(name)) {
        return false; // known parameter!
    }
    if (globals.has(name))
        error(name + " already declared as global"s);
    if (isFunction(name, true))
        error(name + " already declared as function"s);
    if (not context.locals.has(name)) {
        int position = context.locals.size();
        context.locals.add(name, Local{.is_param = is_param, .position = position, .name = name, .type = type});
        return true; // added
    } else {
        updateLocal(context, name, type);
        return false; // already there
    }
    //#endif
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
            type = &field.values().first();
        if (not type and not isType(*type) and not types.has(type->name))
            error("class field needs type %s %s"s % type->name % node.serialize());
        field.type = types[type->name]; // int => IntegerType
        field.kind = fields;
        field.value.longy = pos;
        // field.setMeta("position", (Node)pos++);
        // field.metas()["position"] = pos++;
        field["@position"] = pos++;
    }
    if (types.has(name)) {
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
    Function function = {
        .name = name, .module = mod, .is_import = true, .is_declared = not body.empty(), .is_used = true,
    }; // todo: clone!
    function.body = &body;
    function.signature = groupFunctionArgs(function, node);
    if (returns and function.signature.return_types.size() == 0)
        function.signature.returns(mapType(returns->name));
    functions.insert_or_assign(name, function);
    return Node().setKind(functor).setValue(Value{.data = &function}); // todo: clone!  todo functor => Function* !?!?
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
    // Type return_type;
    // if(return_type0)
    //     return_type = mapType(return_type0);
    // else
    //     return_type = preEvaluateType(body, context);
    //	String &name = fun.name;
    //	silent_assert(not is_operator(name[0]));
    //	trace_assert(not is_operator(name[0]));
    if (is_operator(name[0]) and name != ":=") // todo ^^
        todo("is_operator!"s + name); // remove if it doesn't happen

    if (name and not function_operators.has(name)) {
        if (context.name != "wasp_main") {
            print("broken context");
            print(context.name);
        }
        if (not functions.has(name)) {
            Function fun{.name = name};
            functions.add(name, fun);
        }
    }
    Function &function = functions[name]; // different from context!
    function.name = name;
    function.is_declared = true;
    function.is_import = false;


    Signature &signature = groupFunctionArgs(function, arguments);
    if (signature.size() == 0 and function.locals.size() == 0 and body.has("it", false, 100)) {
        addLocal(function, "it", float64t, true);
        signature.add(float64t, "it"); // todo: any / smarti! <<<
    }
    body = analyze(body, function); // has to come after arg analysis!
    if (!return_type)
        return_type = extractReturnTypes(arguments, body).clone();
    if (return_type)
        signature.returns(mapType(return_type)); // explicit double sin(){} // todo other syntaxes+ multi
    Node &decl = *new Node(name); //node.name+":={…}");
    decl.setKind(declaration);
    decl.add(body.clone());
    function.body = body.clone(); //.flat(); // debug or use?
    //	decl["signature"]=*new Node("signature");
    if (signature.functions.size() == 0)
        signature.functions.add(&function);
    decl["@signature"].value.data = &signature;
    //    function.body= &body;
    return decl;
}

Node extractModifiers(Node &expression) {
    Node modifieres;
    for (auto& child: expression) {
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
    auto kw = expression.containsAny(function_keywords, false); // todo fest='def' QUOTED!!
    if (expression.index(kw) != 0)
        error("function keywords must be first");
    expression.children++;// get rid of first 'function' keyword
    expression.length--;// get rid of first 'function' keyword
    auto &fun = expression.first();
    Node *return_type = 0;
    Node arguments = groupTypes(fun.childs(), context); // children f(x,y)
    Node body;
    auto ret = expression.containsAny(return_keywords);
    if (ret) {
        return_type = &expression.from(ret);
        expression = expression.to(ret);
        body = return_type->values(); // f(x,y) -> int { x+y }
    } else if (expression.length == 3) {
        // f(x,y) int { x+y }
        return_type = &expression[1];
        body = expression.last();
    } else if (expression.length == 2) {
        body = expression.last();
    } else body = fun.values();

    auto opa = expression.containsAny(function_operators); // fun x := x+1
    if (opa)
        body = expression.from(opa);
    return groupFunctionDeclaration(fun.name, return_type, NIL, arguments, body, context);
}

// f x:=x*x  vs groupFunctionDefinition fun x := x*x
Node &groupFunctionDeclaration(Node &expression, Function &context) {
    Node modifieres = extractModifiers(expression);
    auto op = expression.containsAny(function_operators);
    auto &left = expression.to(op);
    auto &rest = expression.from(op);
    auto fun = left.first();
    return groupFunctionDeclaration(fun.name, 0, left, left, rest, context);
}

Node &groupDeclarations(Node &expression, Function &context) {
    if (expression.kind == groups) // handle later!
        return expression;
    //    if (expression.kind != Kind::expression)return expression;// 2022-19 sure??
    if (expression.contains(":=") ) { // add_raw
        return groupFunctionDeclaration(expression, context);
    }

    auto &first = expression.first();
    //	if (contains(functor_list, first.name))
    //		return expression;
    if (expression.kind == declaration) {
        if (isType(first)) {
            auto &fun = expression[1];
            String name = fun.name;
            Node *typ = first.clone();
            Node modifieres = NIL;
            Node &arguments = fun.values();
            Node &body = expression.last();
            return groupFunctionDeclaration(name, typ, NIL, arguments, body, context);
        } else if (isType(first.first())) {
            auto &fun = first[1];
            String name = fun.name;
            Node &typ = first.first();
            Node modifieres = NIL;
            Node &arguments = expression[1];
            Node &body = expression.last();
            return groupFunctionDeclaration(name, &typ, NIL, arguments, body, context);
        } else {
            warn("declaration"s + expression.serialize());
            return expression;
        }
    }
    for (Node &node: expression) {
        String &op = node.name;
        if (isKeyword(op))
            continue;
        if (isPrimitive(node) and node.isSetter()) {
            addLocal(context, op, preEvaluateType(node, context), false);
            continue;
        }
        if ((node.kind == reference and not node.length) or
            (node.kind == key and maybeVariable(node))) {
            // only constructors here!
            if (not globals.has(op) and not isFunction(node) and not builtin_constants.contains(op)) {
                Type evaluatedType = unknown_type;
                if (use_wasm_arrays)
                    evaluatedType = preEvaluateType(node, context); // todo turns sin π/2 into 1.5707963267948966 ;)
                addLocal(context, op, evaluatedType, false);
            }
            continue;
        } // todo danger, referenceIndices i=1 … could fall through to here:
        if (node.kind != declaration and not declaration_operators.has(op))
            continue;
        if (op.empty())continue;
        if (op == "=" or op == ":=") continue; // handle assignment via groupOperators !
        if (op == "::=") continue; // handle globals assignment via groupOperators !
        // todo: public export function jaja (a:num …) := …

        // BEGINNING OF Declaration ANALYSIS
        Node left = expression.to(node); // including public… + ARGS! :(
        Node rest = expression.from(node); // body
        String name = extractFunctionName(left);
        if (left.length == 0 and not declaration_operators.has(node.name))
            name = node.name; // todo: get rid of strange heuristics!
        if (node.length > 1) {
            // C style double sin(x) {…} todo: fragile criterion!! also why is body not child of sin??
            name = node.first().name;
            left = node.first().first(); // ARGS
            rest = node.last();
            if (rest.kind == declaration)rest = rest.values();
            context.locals.remove(name); // not a variable!
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

bool isKeyword(String &op) {
    if (operator_list.has(op))return true;
    if (function_operators.has(op))return true;
    if (prefixOperators.has(op))return true;
    if (suffixOperators.has(op))return true;
    if (extra_reserved_keywords.contains(op))return true;
    if (contains(functor_list, op))return true;
    if (contains(control_flows, op))return true;
    return false;
}

Node extractReturnTypes(Node decl, Node body) {
    return DoubleType; // LongType;// todo
}

void checkRequiredCasts(String &op, const Node &lhs, Node &rhs, Function &context) {
    // todo maybe add cast node here instead of in emit?
    Type left_kind = lhs.kind;
    Type right_kind = rhs.kind;
    if(right_kind == operators or right_kind == expression or right_kind == reference or right_kind == call)
        right_kind = preEvaluateType(rhs, context); // todo: use preEvaluateType for all rhs?

    if (op == "+" and left_kind == strings)
        useFunction("toString");// todo ALL polymorphic variants! => get rid of these:
    if (op == "+" and left_kind == strings and right_kind == longs)
        useFunction("formatLong");
    if (op == "+" and left_kind == strings and (right_kind == reals or right_kind == realsF))
        useFunction("formatReal");
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
            return NUL; // no code, just meta
        }
        //		else todo("ungrouped dangling operator");
    }
    // inner ops get grouped first: 3*42≥2*3 => 1. group '*' : (3*42)≥(2*3)
    for (String &op: operators) {
        //        trace("operator");
        //        trace(op);
        if (op.empty())
            error("empty operator BUG");
        if (op == "else")continue; // handled in groupIf
        if (op == "module") {
            warn("todo modules");
            if (module)
                module->name = expression.last().name;
            return NUL;
        }
        if (op == "-")
            debug = true;
        if (op == "-…") op = "-"; // precedence hack

        //        todo op=use_import();continue ?
        if (op == "%")
            useFunction("modulo_float"); // no wasm i32_rem_s i64_rem_s for float/double
        if (op == "%")
            useFunction("modulo_double");
        if (op == "==" or op == "is" or op == "equals")use_runtime("eq");
        if (op == "include")
            return NUL; // todo("include again!?");
        if (op != last) last_position = 0;
        bool fromRight = rightAssociatives.has(op);
        fromRight = fromRight || isFunction(op, true);
        fromRight = fromRight || (prefixOperators.has(op) and op != "-"); // !√!-1 == !(√(!(-1)))
        int i = expression.index(op, last_position, fromRight);
        if (i < 0) {
            if (op == "-" or op == "‖") //  or op=="?"
                continue; // ok -1 part of number, ‖3‖ closing ?=>if
            i = expression.index(op, last_position, fromRight); // try again for debug
            expression.print();
            error("operator missing: "s + op + " in " + expression.serialize());
        }
        op = checkCanonicalName(op);

        // we can't keep references here because expression.children will get mutated later via replace(…)
        Node &node = expression.children[i];
        if (node.length)continue; // already processed
        Node &next = expression.children[i + 1];
        next = analyze(next, context);
        Node prev;
        if (i > 0) {
            prev = expression.children[i - 1];
            // if(prev.kind == Kind::groups) prev.setType(Kind::expression);
            prev = analyze(prev, context);
        }
        checkRequiredCasts(op, prev, next, context); // todo: move to analyze?

        //            prev = expression.to(op);
        //        else error("binop?");
        if (op == ".") {
            if (prev.kind == referencex) {
                useFunction("invokeExternRef"); // a.b() => invokeExternRef(a, "b")
                useFunction("getExternRefPropertyValue");
                useFunction("setExternRefPropertyValue");
            }
            if (prev.kind == (Kind) Primitive::node)
                useFunction("get");
        }


        // todo move "#" case here:
        if (isPrefixOperation(node, prev, next)) {
            // ++x -i
            // PREFIX Operators
            isPrefixOperation(node, prev, next);
            node.kind = Kind::operators; // todo should have been parsed as such!
            if (op == "-") //  -i => -(0 i)
                node.add(new Node(0)); // todo: use f64.neg
            node.add(next);
            expression.replace(i, i + 1, node);
        } else {
            if (op == "#" and not use_wasm_strings)
                findLibraryFunction("getChar", false);

            prev = analyze(prev, context);
            auto lhs_type = preEvaluateType(prev, context);
            if (op == "+" and (lhs_type == Primitive::charp or lhs_type == Primitive::stringp or lhs_type == strings)) {
                findLibraryFunction("concat", true);
                // findLibraryFunction("_Z6concatPKcS0_", true);
            }
            if (op == "^" or op == "^^" or op == "**" or op == "exp" or op == "ℇ") {
                // todo NORM operators earlier
                findLibraryFunction("pow", false); // old rough!
                // findLibraryFunction("powd", false); // via pow
                findLibraryFunction("powi", false);
            }
            if (suffixOperators.has(op)) {
                // x²
                // SUFFIX Operators
                if (op == "ⁿ") {
                    findLibraryFunction("pow", false);
                    useFunction("pow"); // redirects to
                    // useFunction("powd");
                }
                if (i < 1)
                    error("suffix operator misses left side");
                node.add(prev);
                if (op == "²") {
                    node.add(prev);
                    node.name = "*"; // x² => x*x
                }
                //				analyzed.insert_or_assign(node.hash(), true);
                expression.replace(i - 1, i, node);
                i--;
                //                continue;
            } else if (op.in(function_list)) {
                // handled above!
                while (i++ < node.length)
                    node.add(expression.children[i]);
                expression.replace(i, node.length, node);
            } else if (isFunction(next)) {
                // 3 + double 8
                Node &rest = expression.from(i + 1);
                Node &args = analyze(rest, context);
                node.add(prev);
                node.add(args);
                expression.replace(i - 1, i + 1, node); // replace ALL REST
                expression.remove(i, -1);
            } else {
                auto var = prev.name;
                if ((op == "=" or op == ":=") and (prev.kind == reference or prev.kind == global)) {
                    // ONLY ASSIGNMENT! self modification handled later
                    Type inferred_type = preEvaluateType(next, context);
                    if (prev.kind == global) {
                        if (globals.has(var)) {
                            Global &global = globals[var];
                            global.type = inferred_type; // todo upgrade?
                            if (not global.value)
                                global.value = next.clone();
                        } else {
                            // later
                            //                            addGlobal(context, var, inferred_type, false, *next.clone());
                        }
                    } else if (addLocal(context, var, inferred_type, false)) {
                        // ok
                    } else {
                        // variable is known but not typed yet, or type again?
                        Local &local = function.locals[var];
                        if (local.type == unknown_type)
                            local.type = inferred_type; // mapType(next);
                    }
                } // end of '=' assignment logic
                if (not(op == "#" and prev.empty() and prev.kind != reference)) // ok for #(1,2,3) == len
                    node.add(prev);

                node.add(next);

                if (op.length > 1 and op.endsWith("=") and op[0] != ':')
                    // express *= += -= … self assignments
                    if (op[0] != '=' and op[0] != '!' and op[0] != '?' and op[0] != '<' and op[0] != '>') {
                        // *= += etc
                        node.name = String(op.data[0]);
                        Node *setter = prev.clone();
                        //					setter->setType(assignment);
                        setter->value.node = node.clone();
                        node = *setter;
                    }
                if (node.name == "?")
                    node = groupIf(node, context); // consumes prev and next
                if (i > 0)
                    expression.replace(i - 1, i + 1, node);
                else {
                    expression.replace(i, i + 1, node);
                    warn("operator missing argument");
                }
                if (expression.length == 1 and expression.kind == Kind::groups) {
                    return expression.first(); //.setType(operators);
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
        Node op = Node("-").setKind(operators, true);
        auto right = name.substring(name.indexOf('-') + 1);
        auto &rhs = analyze(*new Node(right, true), context); // todo expression!
        op.add(lhs);
        op.add(rhs);
        return *op.clone();
    }
    return node;
}

Module &loadRuntime() {
#if WASM // and not MY_WASM
    if(not module_cache.has("wasp"s.hash()))
      error("module 'wasp' should already be loaded through js load_runtime_bytes => parseRuntime");
    Module &wasp=*module_cache["wasp"s.hash()];
//    wasp.functions["powi"].signature.returns(int32);
    // if(!libraries.has(&wasp))// on demand per test/app!
    return wasp;
#else
    // Module &wasp = read_wasm("wasp-runtime-debug.wasm");// todo unexpected opcode …!!!
    Module &wasp = read_wasm("wasp-runtime.wasm");
    wasp.functions["getChar"].signature.returns(codepoint1);
    // addLibrary(&wasp); // BREAKS system WHY?
    // if(!libraries.has(&wasp))
    //     libraries.add(&wasp); // BREAKS system WHY?
    return wasp;
#endif
}

Type preEvaluateType(Node &node, Function &context) {
    // e.g √3 => float64t  3/2 => float64t …
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
        if (node.name == "√")return float64t; // todo generalize
        Node &lhs = node.first();
        auto lhs_type = preEvaluateType(lhs, context);
        if (node.length <= 1)
            return lhs_type;
        Node &rhs = node[1];
        auto rhs_type = preEvaluateType(rhs, context); // todo lol
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
        if (node.size() == 1 and use_wasm_arrays) // todo
            return preEvaluateType(node.first(), context);
    }
    return mapType(node);
}


//Type preEvaluateType(Node &node, Function &context) {
Type preEvaluateType(Node &node, Function *context0) {
    if (!context0)context0 = new Function{.name = "ad-hoc"};
    Function &context = *context0;
    return preEvaluateType(node, context);
}


Module &loadModule(String name) {
    if (name == "wasp-runtime.wasm")
        return loadRuntime();
#if WASM and not MY_WASM
    todow("loadModule in WASM: "s+name);
    return *new Module();
#else // getWasmFunclet
    return read_wasm(name); // we need to read signatures!
#endif
}

String &checkCanonicalName(String &name) {
    if (name == "**")info("The power operator in angle is simply '^' : 3^2=9."); // todo: alias infoing mechanism
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

// todo merge with groupFunctionCalls
Node &groupOperatorCall(Node &node, Function &function) {
    {
        bool is_function = isFunction(node);
        findLibraryFunction(node.name, true); // sets functions[name].is_import = true;
        if (is_function and node.kind != operators) {
            node.kind = call;
        }
        Node &grouped = groupOperators(node, function); // outer analysis id(3+3) => id(+(3,3))
        if (grouped.length > 0)
            for (Node &child: grouped) {
                // inner analysis while(i<3){i++}
                child = analyze(child, function); // REPLACE with their ast
            }
        if (is_function)
            functions[node.name].is_used = true;
        return grouped;
    }
}


int findBestVariant(const Function &function, const Node &node, Function *context) {
    if (function.variants.size() == 1) return 0;
    if (function.variants.size() == 0) return -1;
    int best = -1;
    for (Function *variant: function.variants) {
        best++;
        auto &signature = variant->signature;
        if (signature.size() == node.length) {
            bool ok = true;
            for (int i = 0; i < signature.size(); ++i) {
                auto &sig = signature.parameters[i];
                Type aType = sig.type;
                Type pType = preEvaluateType(*node[i].clone(), context);
                if (not compatibleTypes(aType, pType)) {
                    ok = false;
                    break;
                }
            }
            if (ok)
                return best;
        }
    }
    error("no matching function variant for "s + function.name + " with "s + node.serialize());
}

void addLibraryFunctionAsImport(Function &func);

// todo merge with groupOperatorCall
Node &groupFunctionCalls(Node &expressiona, Function &context) {
    if (expressiona.kind == declaration)return expressiona; // handled before
    Function *import = findLibraryFunction(expressiona.name, false);
    if (import or isFunction(expressiona)) {
        expressiona.setKind(call, false);
        if (not functions.has(expressiona.name))
            error("! missing import for function "s + expressiona.name);
        //		if (not expressiona.value.node and arity>0)error("missing args");
        functions[expressiona.name].is_used = true;
    }

    for (int i = 0; i < expressiona.length; ++i) {
        Node &node = expressiona.children[i];
        String &name = node.name;

        // todo: MOVE!
        if (name == "if") // kinda functor
        {
            auto &args = expressiona.from("if");
            Node &iff = groupIf(node.length > 0 ? node.add(args) : args, context);
            int j = expressiona.lastIndex(iff.last().next) - 1;
            if (i == 0 and j == expressiona.length - 1)return iff;
            if (j > i)
                expressiona.replace(i, j, iff); // todo figure out if a>b c d e == if(a>b)then c else d; e boundary
            if (i == 0)
                return expressiona;
            continue;
        }
        //        if (name == "for") {
        //            Node &forr = groupFor(node, context);
        //            int j = expressiona.lastIndex(forr.last().next) - 1;
        //            if (j > i)
        //                expressiona.replace(i, j, forr);
        //            continue;
        //        }
        if (name == "while") {
            // todo: move into groupWhile !!
            if (node.length == 2) {
                node[0] = analyze(node[0], context);
                node[1] = analyze(node[1], context);
                continue; // all good
            }
            if (node.length == 1) {
                // while()… or …while()
                node[0] = analyze(node[0], context);
                Node then = expressiona.from("while"); // todo: to closer!?
                int remaining = then.length;
                node.add(analyze(then, context).clone());
                expressiona.remove(i + 1, i + remaining);
                continue;
            } else {
                Node n = expressiona.from("while");
                Node &iff = groupWhile(n, context); // todo: sketchy!
                int j = expressiona.lastIndex(iff.last().next) - 1; // huh?
                if (j > i)expressiona.replace(i, j, iff);
            }
        }
        if (isFunction(node)) // needs preparsing of declarations!
            node.kind = call;
        if (node.kind != call)
            continue;

        Function *ok = findLibraryFunction(name, true);
        if (not ok and not functions.has(name)) // todo load lib!
            error("missing import for function "s + name);
        Function &function = functions[name];
        function.name = name; // hack shut've Never Been Lost
        Signature &signature = function.signature;
        if (function.is_polymorphic) {
            auto &params0 = expressiona.from(i + 1);
            auto &params = analyze(params0, context);
            int variantNr = findBestVariant(function, *wrap(params), &context);
            Function *variant = function.variants[variantNr];
            signature = variant->signature; // todo hack
            variant->is_used = true;
            if (function.is_runtime and not variant->fullname.empty() and variant->fullname != function.name)
                functions.add(variant->fullname, *variant); // needs extra call index!
            addLibraryFunctionAsImport(*variant);
            print("matching function variant "s + variantNr + " of " + function.name + " with "s + signature.
                  serialize());
            node.add(params); // todo: this is all duplication of code below:
            expressiona.remove(i + 1, -1); // todo
            // expressiona.replace(i+1, -1, params);// todo
            // expressiona.replace(i+1, i + params.length, params);// todo
        }
        // return groupFunctionCallPolymorphic(node, function, expressiona, context);
        function.is_used = true;


        int minArity = signature.size(); // todo: default args!
        int maxArity = signature.size();

        if (node.length > 0) {
            //			if minArity == …
            node = analyze(node.flat(), context); //  f(1,2,3) vs f([1,2,3]) ?
            continue; // already done how
        }
        if (minArity == 0)continue;
        if (maxArity < 0)continue; // todo
        Node rest;
        if (i < expressiona.length - 1 and expressiona[i + 1].kind == groups) {
            // f(x)
            // todo f (x) (y) (z)
            // todo expressiona[i+1].length>=minArity
            rest = expressiona[i + 1];
            if (rest.length > 1)
                rest.setKind(expression);
            Node args = analyze(rest, context);
            node.add(args);
            expressiona.remove(i + 1, i + 1);
            continue;
        }
        rest = expressiona.from(i + 1);
        int arg_length = rest.length;
        if (not arg_length and rest.kind == urls) arg_length = 1;
        if (not arg_length and rest.kind == reference) arg_length = 1;
        if (not arg_length and rest.value.data) arg_length = 1;
        if (arg_length > 1)
            rest.setKind(expression); // SUUURE?
        if (rest.kind == groups or rest.kind == objects) // and rest.has(operator))
            rest.setKind(expression); // todo f(1,2) vs f(1+2)
        //		if (hasFunction(rest) and rest.first().kind != groups)
        //				warn("Ambiguous mixing of functions `ƒ 1 + ƒ 1 ` can be read as `ƒ(1 + ƒ 1)` or `ƒ(1) + ƒ 1` ");
        // per-function precedence does NOT really increase readability or bug safety
        if (rest.value.data) {
            maxArity--; // ?
            minArity--;
        }
        if (arg_length < minArity) {
            print(function.name + (String) signature);
            error("missing arguments for function %s, given %d < expected %d. "
                "defaults and currying not yet supported"s % name % arg_length % minArity);
        } else if (arg_length == 0 and minArity > 0)
            error("missing arguments for function %s, or to pass function pointer use func keyword"s % name);
        Node &args = analyze(rest, context); // todo: could contain another call!
        node.add(args);
        expressiona.remove(i + 1, i + arg_length);
    }
    return expressiona;
}


void addLibraryFunctionAsImport(Function &func) {
    func.is_used = true;
    if (func.is_declared)return;
    if (func.is_builtin)return;
    //	auto function_known = functions.has(func.name);

    // ⚠️ this function now lives inside Module AND as import inside "wasp_main" functions list, with different wasm_index!
    bool function_known = functions.has(func.name);
#if WASM
    Function& import=*new Function();
    if(function_known)
        import = functions[func.name];
#else
    Function &import = functions[func.name]; // copy function info from library/runtime to main module
#endif
    if (import.is_declared)return;
    if (func.is_polymorphic) {
        import.is_polymorphic = func.is_polymorphic; //
        import.variants = func.variants;
    } else {
        import.signature = func.signature;
        import.signature.parameters = func.signature.parameters; // even if polymorph?
    }
    import.signature.type_index = -1;
    import.is_runtime = false; // because here it is an import!
    import.is_import = true; // todo also for polymorph?
    import.is_used = true; // todo also for polymorph?
#if WASM
    if(not function_known)
        functions.add(func.name, import);
#endif
}

Function getWaspFunction(String name);

bool eq(Module *x, Module *y) { return x->name == y->name; } // for List: libraries.has(library)

// todo: clarify registerAsImport side effect
// todo: return the import, not the library function
Function *findLibraryFunction(String name, bool searchAliases) {
    if (name.empty())return 0;
    if (functions.has(name))
        return use_required_import(&functions[name]); // prevents read_wasm("lib")
#if WASM // todo: why only in wasm?
    Module& wasp = loadRuntime();
    if (wasp.functions.has(name)){
        if(!libraries.has(&wasp))// on demand per test/app!
            libraries.add(&wasp);
        return use_required_import(&wasp.functions[name]);
    }
#endif
    if (contains(funclet_list, name)) {
#if WASM and not MY_WASM
        //				todo("getWaspFunclet get library function signature from wasp");
                warn("WASP function "s + name + " getWaspFunclet todo");
                auto pFunction = getWaspFunction(name).clone();
                pFunction->import();
                return use_required_import(pFunction);
#endif
        print("loading funclet "s + name);
        Module &funclet_module = read_wasm(findFile(name, "lib"));
        check(funclet_module.functions.has(name));
        module_cache.insert_or_assign(funclet_module.name.hash(), &funclet_module);
        Function& funclet = funclet_module.functions[name];
        print("GOT funclet "s + name);
        print(funclet.signature);
        addLibrary(&funclet_module);
        return use_required_import(&funclet);
    }
    if (name.in(function_list) and libraries.size() == 0)
        libraries.add(&loadRuntime());
    // libraries.add(&loadModule("wasp-runtime.wasm")); // on demand


    //	if(functions.has(name))return &functions[name]; // ⚠️ returning import with different wasm_index than in Module!
    // todo in WASM
    for (Module *library: libraries) {
        //} module_cache.valueList()) {
        // todo : multiple signatures! concat(bytes, chars, …) eq(…)
        int position = library->functions.position(name);
        if (position >= 0) {
            Function &func = library->functions.values[position];
            return use_required_import(&func);
        }
    }
    Function *function = 0;
    if (searchAliases) {
        for (String alias: aliases(name)) {
            function = findLibraryFunction(alias, false);
            //			use_required(function); // no, NOT required yet
        }
    }
    auto normed = normOperator(name);
    if (normed == name)
        return use_required_import(function);
    else
        return findLibraryFunction(normed, false);
}

void addLibrary(Module *modul) {
    if (not modul)return;
    for (auto &lib: libraries)
        if (lib->name == modul->name)return;
    libraries.add(modul); // link it later via import or use its code directly?
}

// todo merge with similar functions and aliases()
Function *use_required_import(Function *function) {
    if (!function or not function->name or function->name.empty())
        return 0; // todo how?
    addLibraryFunctionAsImport(*function);
    if (function->name == "quit")
        useFunction("proc_exit");
    if (function->name == "puts")
        useFunction("fd_write");
    // for (Function *variant: function->variants) {
    //     addLibraryFunctionAsImport(*variant);
    // }
    for (String &alias: aliases(function->name)) {
        if (alias == function->name)continue;
        auto ali = findLibraryFunction(alias, false);
        if (ali)addLibraryFunctionAsImport(*ali);
    }
    for (auto &vari: function->variants) {
        vari->is_used = true;
    }
    //    for(Function& dep:function.required)
    //        dep.is_used = true;
    return function;
}

// todo see load_aliases(); -> aliases.wasp
List<String> aliases(String name) {
    List<String> found;
#if MY_WASM
    return found;
#endif
    //	switch (name) // statement requires expression of integer type
    if (name == "pow") {
        found.add("pow");
        // found.add("powd"); via pow
        found.add("powf");
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
    if (name == "concat_chars")
        found.add("_Z12concat_charsPKcS0_"); // todo by parsing!!
    if (name == "concat") {
        // todo: programmatic!
        if (not use_wasm_strings)
            found.add("concat"); // OK _Z6concatPKcS0_ via linker:
        // LINKED main.wasm:concat import #1 concat to export #31 _Z6concatPKcS0_ relocated_function_index 23
            // found.add("_Z6concatPKcS0_"); // this is the signature we call for concat(char*,char*) … todo : use String.+
    }
    //	if (name == "+") {
    //		found.add("add");
    //		found.add("plus");
    //		found.add("concat");
    //		if (not use_wasm_strings)
    //			found.add("_Z6concatPKcS0_"); // this is the signature we call for concat(char*,char*) … todo : use String.+
    //	}
    if (name == "eq") {
        if (not use_wasm_strings)
            found.add("_Z2eqPKcS0_i"); // eq(char const*, char const*, int)
        //        found.add("_Z2eqR6StringPKc"); // eq(String&, char const*)
    }
    if (name == "=") {
        //        found.add("is");
        //        found.add("be");
    }
    if (name == "#") {
        // todo
        //        found.add("getChar");
    }
    return found;
}

Node &groupForClassic2(Node &node, Function &context) {
    // for(…){}
    if (node.length < 2)
        error("Invalid 'for' loop structure. Expected initializer and body.");
    // Extract components
    Node &forex = node[0]; // e.g., 'for'
    Node &fore = forex.values(); // e.g., 'i = 0'
    if (fore.size() != 3)
        error("Invalid 'for' loop structure. Expected three parts like for(i=0;i<10;i++){}");

    Node initializer = fore[0];
    Node condition = fore[1];
    Node increment = fore[2];
    initializer = analyze(initializer, context);
    condition = analyze(condition, context);
    increment = analyze(increment, context);
    // todo for loop creates it's own scope!!
    Node &body = analyze(node[1], context); // Loop body
    // Create a node for the 'for' loop
    Node *forNode = new Node("fori");
    Node &grouped = *forNode;
    grouped.kind = expression; // Mark as an expression node

    // Group components into structured nodes
    grouped["initializer"] = initializer; // Store the initializer
    grouped["condition"] = condition; // Store the condition
    grouped["increment"] = increment; // Store the increment
    grouped["body"] = body; // Store the loop body

    return grouped;
}

Node &groupForClassic(Node &node, Function &context) {
    // // for i=0;i<10;i++ {}
    if (node.length == 2)return groupForClassic2(node, context);
    // if (node.length < 4)
    //     error("Invalid 'for' loop structure. Expected initializer, condition, increment, and body.");

    // Extract components
    Node &initializer = node[0]; // e.g., 'i = 0'
    Node &condition = node[1]; // e.g., 'i < 1000'
    Node &increment = node[2]; // e.g., 'i++'
    Node &body = node[3]; // Loop body

    // Validate components
    if (!initializer.value.data)
        error("Missing initializer in 'for' loop.");
    if (!condition.value.data)
        error("Missing condition in 'for' loop.");
    if (!increment.value.data)
        error("Missing increment in 'for' loop.");

    // Create a node for the 'for' loop
    Node *forNode = new Node("fori");
    Node &grouped = *forNode;
    grouped.kind = expression; // Mark as an expression node

    // Group components into structured nodes
    grouped["initializer"] = initializer; // Store the initializer
    grouped["condition"] = condition; // Store the condition
    grouped["increment"] = increment; // Store the increment
    grouped["body"] = body; // Store the loop body

    return grouped;
}

Node &groupForIn(Node &n, Function &context) {
    // for i in 0 to 10 {}
    // for i in 1 < 3 {}
    //    todo : for i < 3
    List<String> ops = {"to", "…", "upto", "<", "<=", "..", "..<", "until", ">", ">=", "...", "downto"};
    if (n[2].name != "in" or not ops.has(n[4].name))
        error("Invalid 'for' loop structure. Expected: for i in begin to end {}");
    Node &variable = n[1];
    addLocal(context, variable.name, int32t, false);
    Node &begin = n[3];
    Node &end = n[5];
    Node &body = end.values(); // for i in 1 to 5 : {print i}
    if (n.length == 6) body = n[5];
    if (n.length > 6) body = n.from(6);
    if (n.length > 6 and n[6].name == ":") body = n.from(7);

    if (body.kind != longs) // todo: hacky! put in there ?
        body.setKind(expression); // if …
    Node *foro = new Node("for");
    Node &ef = *foro;
    ef.kind = expression;
    ef["variable"] = analyze(variable, context);
    ef["begin"] = analyze(begin, context);
    ef["end"] = analyze(end, context);
    ef["body"] = analyze(body, context);
    ef["upto"] = *new Node(n[4].name == "upto" or n[4].name == "<");
    analyzed[ef.hash()] = 1;
    return ef;
}

Node &groupFor(Node &n, Function &context) {
    if (n.length < 2)
        error("Incomplete 'for' loop structure");
    if (n.length == 2) // for(…){}
        return groupForClassic2(n, context);
    if (n.length > 2 and n[2].name == "in")
        return groupForIn(n, context);
    if (n.length > 7 or n[2].name != "in")
        return groupForClassic(n, context);

    // Extract components: variable, iterable, and body
    // for i in iterable {}
    Node &variable = n.children[0];
    Node &iterable = n.children[1];
    Node body = (n.length > 2) ? n.children[2] : Node();

    addLocal(context, variable.name, int32t, false);

    // Create and structure the "for" node
    Node *foro = new Node("for");
    Node &ef = *foro;
    ef.kind = expression;
    ef["variable"] = analyze(variable, context);
    ef["iterable"] = analyze(iterable, context);
    ef["body"] = analyze(body, context);
    analyzed[ef.hash()] = 1;
    return ef;
}

Node &groupWhile(Node &n, Function &context) {
    // Check for minimum structure
    if (n.length == 0 && !n.value.data)
        error("Missing condition for while statement");
    if (n.length == 1 && !n.value.data)
        error("Missing block for while statement");

    // Extract condition and 'then' block
    Node &condition = n.children[0];
    Node then = (n.length > 1) ? n.children[1] : Node(); // Use explicit initialization

    // Handle ":" and "do" grouping
    if (n.has(":")) {
        condition = n.to(":");
        then = n.from(":");
    }

    // Handle standalone conditions and alternative grouping cases
    if (condition.value.data && !condition.next)
        then = condition.values();
    if (condition.kind == reference) {
        for (Node &child: n) {
            if (child.kind == groups || child.kind == objects) {
                condition = n.to(child);
                then = child;
                break;
            }
        }
    }
    // Create and structure the "while" node
    Node *whilo = new Node("while");
    Node &ef = *whilo;
    ef.kind = expression;
    ef["condition"] = analyze(condition, context);
    ef["then"] = analyze(then, context);
    analyzed[ef.hash()] = 1;
    return ef;
}

Node &groupOperatorCall(Node &node, Function &function);

Type guessType(Node &node, Function &function) {
    return preEvaluateType(node, function); // todo: use function context?
}


// todo move to parseTemplate
Node &groupTemplate(Node &node, Function &function) {
    for (Node& child: node) {
        Type kind = child.kind;
        if (kind != strings) {
            child = analyze(child, function); // analyze each child node
            kind = guessType(child, function);
            if (kind == expression)
                kind = child.first().kind; // todo: hacky, but works for now
            if (kind == referencex or kind == reference) useFunction("toString");
            else if (kind == undefined) useFunction("formatLong"); // TODO!
            else if (kind == long32) useFunction("formatLong"); // itoa0
            else if (kind == longs) useFunction("formatLong"); // ltoa
            else if (kind == reals or kind == realsF) useFunction("formatReal"); // ftoa
            else if (kind == doubles) useFunction("formatReal");
            else if (kind == floats) useFunction("formatReal"); // todo via auto upcast?
            else
                error("Unknown template child type: "s + typeName(kind));
        }
    }
    // useFunction("_Z12concat_charsPKcS0_");
    useFunction("concat");
    // useFunction("concat_chars");
    // functions["concat_chars"].is_used = true;
    return node;
}

Node &analyze(String code) {
    return analyze(parse(code));
}
/*
 * ☢️ ⚛ Nuclear Core ⚠️ 🚧
 * turning some knobs might yield some great powers
 * touching anything can result in disaster
 * should be replaced with elegant modular solar panels
 */
Node &analyze(Node &node, Function &function) {
    String &context = function.name;
    if (context != "global" and !functions.has(context)) {
        function.is_declared = true;
        functions.add(context, function); // index not known yet
    }
    int64 hash = node.hash();
    if (analyzed.has(hash))
        return node;

    Kind type = node.kind;
    String &name = node.name;
    auto firstName = node.first().name;

    if (firstName == "data" or firstName == "quote")
        return node; // data keyword leaves data completely unparsed, like lisp quote `()

    if (name == "html") {
        useFunction("getDocumentBody");
        useFunction("createHtml");
        useFunction("createHtmlElement");
        // also getHtmlAttribute / createHtmlAttribute / setHtmlAttribute / property
        return node; // html builder currently not parsed
    }
    if (name == "js" or name == "script" or name == "javascript") {
        useFunction("addScript");
        return node;
    }
    if (name == "if")return groupIf(node, function);
    if (name == "while")return groupWhile(node, function);
    if (name == "for" or firstName == "for")return groupFor(node, function);
    if (name == "?")return groupIf(node, function);
    if (name == "module") {
        if (!module)module = new Module();
        module->name = node.string(); // todo: use?
        return NUL;
    }
#if not WASM // todo why not??
    // class declaration!
    if (not firstName.empty() and class_keywords.contains(firstName))
        return classDeclaration(node, function);
#endif


    // add: func(a: float32, b: float32) -> float32
    if (node.kind == key and node.values().name == "func") // todo move edge case to witReader
        return funcDeclaration(node.name, node.values(), NUL /* no body here */ , 0, function.module);
    //        return witReader.analyzeWit(node);

    if ((type == expression and not name.empty() and not name.contains("-")))
        addLocal(function, name, int32t, false); //  todo deep type analysis x = π * fun() % 4
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
            else if (first.name == "for")
                return groupFor(node, function);
            else if (first.name == "if")
                return groupIf(node, function);
            else if (node.length > 1)
                error("unknown key expression: "s + node.serialize());
        }
        addLocal(function, name, node.value.node ? mapType(node.value.node) : none, false);
    }
    if (type == strings and node.type == &TemplateType)
        return groupTemplate(node, function); // todo: move to witReader

    if (isGlobal(node, function))
        return groupGlobal(node, function);
    if (isPrimitive(node)) {
        if (node.kind == strings and node.next and node.next->kind == strings)
            useFunction("concat");
        if (maybeVariable(node))
            addLocal(function, name, mapType(node), false);
        return node; // nothing to be analyzed!
    }
    // function_keywords.has(node.name / values)
    if (node.containsAny(function_keywords, false))
        return groupFunctionDefinition(node, function);

    if (type == operators or type == call or isFunction(node)) //todo merge/clean
        return groupOperatorCall(node, function); // call, NOT definition

    Node &groupedTypes = groupTypes(node, function);
    Node &groupedDeclarations = groupDeclarations(groupedTypes, function);
    Node &groupedFunctions = groupFunctionCalls(groupedDeclarations, function);
    Node &grouped = groupOperators(groupedFunctions, function);
    if (analyzed[grouped.hash()])return grouped; // done!
    analyzed.insert_or_assign(grouped.hash(), 1);
    if (grouped.kind == referencex or grouped.name.startsWith("$")) {
        // external reference, e.g. html $id
        useFunction("getElementById");
        useFunction("toNode"); // todo get rid of these:
        useFunction("toString");
        useFunction("toLong");
        useFunction("toReal");
        // useFunction("toInt");
        // useFunction("toReal");
    }
    //    Node& last;
    if (isGroup(type)) {
        // handle lists, arrays, objects, … (statements)
        if (grouped.length > 0)
            for (Node &child: grouped) {
                // children of lists analyzed individually
                if (!child.name.empty() and wit_keywords.contains(child.name) and child.kind != strings /* TODO … */)
                    return witReader.analyzeWit(node); // can't MOVE there:
                child = analyze(child, function); // REPLACE ref with their ast ok?
                //                last.next = child;
            }
    }
    return grouped;
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
    functions["proc_exit"].signature.add(int32t, "exit_code"); // file descriptor
    //	wasp-runtime.wasm needs to be recompiled too!
#if WASMEDGE
    functions["proc_exit"].module = new Module{.name = "wasi_snapshot_preview1"};
#else
    functions["proc_exit"].module = new Module{.name="wasi_unstable"};
#endif
    functions["fd_write"].import();
    functions["fd_write"].signature.add(int32t, "fd"); // file descriptor
    functions["fd_write"].signature.add(pointer, "iovs");
    functions["fd_write"].signature.add(size32, "iovs_len");
    functions["fd_write"].signature.add(pointer, "nwritten"); // size_t *  out !
    functions["fd_write"].signature.returns(int32t);
    //    functions["fd_write"].module=new Module{.name="wasi"};
#if WASMEDGE or WASMTIME
    functions["fd_write"].module = new Module{.name = "wasi_snapshot_preview1"};
#else
    functions["fd_write"].module = new Module{.name="wasi_unstable"};
#endif

    functions["printx"].import();
    functions["printx"].signature.add(externref, "extern_reference");

    functions["download"].import();
    functions["download"].signature.add(charp, "url").returns(charp);
    // todo alias ^^
    functions["fetch"].import();
    functions["fetch"].signature.add(charp, "url").returns(charp);

    // DOM functions
    functions["getDocumentBody"].import().signature.returns(externref);
    functions["createHtml"].import();
    functions["createHtml"].signature.add(externref, "parent").add(charp, "innerHtml").returns(externref);
    functions["createHtmlElement"].import();
    functions["createHtmlElement"].signature.add(externref, "parent").add(charp, "tag").add(charp, "id").returns(
        externref);

    functions["addScript"].import().signature.add(charp, "js");

    //#if WASM // no funclets in browser (yet?)
    //    functions["pow"].import(); PREVENTS reading from pow.wasm file!
    //    functions["pow"].signature.add(float64).add(float64).returns(float64);

    functions["print"].import();
    //    functions["print"].signature.add(node).returns(voids);
    functions["print"].signature.add(smarti64).returns(voids);

    functions["getElementById"].import();
    functions["getElementById"].signature.add(charp, "id").returns(externref /*!!*/);

    functions["toNode"].import();
    functions["toNode"].signature.add(referencex, "id").returns(node);
    // todo: do all these via toNode!
    functions["toString"].import();
    functions["toString"].signature.add(referencex, "id").returns(charp);
    functions["toLong"].import();
    functions["toLong"].signature.add(referencex, "id").returns(longs);
    functions["toReal"].import();
    functions["toReal"].signature.add(referencex, "id").returns(reals);

    // functions["formatLong"].import();
    // functions["formatLong"].signature.add(longs).returns(strings);

    functions["concat"].import(); // todo load from runtime AGAIN 2025-06-16
    functions["concat"].signature.add(charp).add(charp).returns(charp);


    functions["invokeExternRef"].import();
    // how to distinguish between functions and properties? 1. no params use getExternRefPropertyValue
    functions["invokeExternRef"].signature.add(externref).add(charp, "method").add(charp, "params").returns(smarti64);

    //    functions["invokeExternRef"].signature.add(externref).add(strings, "method").add(node, "params").returns(smarti64);
    //    TODO get pointer of node on stack

    functions["getExternRefPropertyValue"].import(); // for consumption
    functions["getExternRefPropertyValue"].signature.add(externref, "object").add(charp, "field").returns(smarti64);
    //	functions["getExternRefPropertyValue"].signature.add(externref).add(charp).returns(stringref); // ⚠️ not yet in webview!

    functions["setExternRefPropertyValue"].import();
    functions["setExternRefPropertyValue"].signature.
            add(externref, "object").add(charp, "field").add(smarti64, "value").returns(smarti64);


    //	functions["$"].import();
    //	functions["$"].signature.add(strings).returns(referencex);

    functions["puts"].builtin();
    functions["puts"].signature.add(stringp).returns(int32t); // stdio conform!!

    functions["len"].builtin(); // via wasp abi len(any)=*(&any)[1]
    functions["len"].signature.add(array).returns(int32t); // todo any wasp type

    functions["quit"].builtin(); // no args, calls proc_exit(0)

    // TESTS! not useful otherwise!
    //    functions["square"].import().signature.add(int32).returns(int32);// test only!!

#if MULTI_VALUE
    functions["wasp_main"].signature.returns(i64).returns(i32);// [result, type32] transparently (no flipped stack order)
#else
    //    functions["wasp_main"].signature.returns(i64);
    functions["wasp_main"].signature.returns(smarti64);
#endif
    // check(functions["wasp_main"].name == "wasp_main");
    // check(functions["wasp_main"].signature.return_types.size() >= 1);
    //    functions["paint"].import().signature.returns(voids);// paint surface
    //    functions["init_graphics"].import().signature.returns(pointer);// surface

    // BUILTINS
    //    functions["nop"].builtin(); NOT A FUNCTION! an op
    functions["id"].builtin().signature.add(i32t).returns(i32t);
    functions["modulo_float"].builtin().signature.add(float32t).add(float32t).returns(float32t);
    functions["modulo_double"].builtin().signature.add(float64t).add(float64t).returns(float64t);

    fixFunctionNames();
}

void clearAnalyzerContext() {
    //    clearEmitterContext()
    //	needs to be outside analyze, because analyze is recursive
#ifndef RUNTIME_ONLY
    libraries.clear(); // todo: keep runtime or keep as INACTIVE to save reparsing
    //    module_cache.clear(); NOO not the cache lol
    types.clear();
    globals.clear();
    call_indices.clear();
    call_indices.setDefault(-1);
    analyzed.clear(); // todo move much into outer analyze function!
    functions.clear(); // always needs to be followed by
    preRegisterFunctions(); // BUG Signature wrong cpp file
#endif
}


// emit via library merge
Node runtime_emit(String prog) {
#ifdef RUNTIME_ONLY
    printf("emit wasm not built into release runtime");
    return ERROR;
#endif
    libraries.clear(); // todo reuse
    clearAnalyzerContext();
    clearEmitterContext();
    Module &runtime = loadRuntime();
    runtime.code.needs_relocate = false;
    Code code = compile(prog, false); // should use libraries!
    code.needs_relocate = false;
    code.save("merged.wasm");
    int64 result_val = code.run(); // todo parse stdout string as node and merge with emit() !
    return *smartNode(result_val);
}


// smart pointers returned if ABI does not allow multi-return, as in int main(){}

Node smartNode32(int smartPointer32) {
    auto smart_pointer = smartPointer32 & 0x00FFFFFF; // data part
    if ((smartPointer32 & 0xF0000000) == array_header_32 /* and abi=wasp */) {
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
    if ((smartPointer32 & 0xFF000000) == string_header_32 /* and abi=wasp */) {
        // smart pointer for string
        return Node(((char *) wasm_memory) + smart_pointer);
    }
    error1("missing smart pointer type "s + typeName(Type(smartPointer32)));
    return Node();
}

float precedence(Node &operater) {
    String &name = operater.name;
    //	if (operater == NIL)return 0; error prone
    if (operater.kind == reals)return 0; //;1000;// implicit multiplication HAS to be done elsewhere!
    if (operater.kind == longs)return 0; //;1000;// implicit multiplication HAS to be done elsewhere!
    if (operater.kind == strings)return 0; // and empty(name)

    // todo: make live easier by making patterns only operators if data on the left
    if (operater.kind == patterns and operater.parent) return 98; // precedence("if") * 0.98
    //	todo why do groups have precedence again?
    //	if (operater.kind == groups) return 99;// needs to be smaller than functor/function calls
    if (operater.name.in(function_list))return 999; // function call todo: remove here
    if (empty(name))return 0; // no precedence
    return precedence(name);
}

Function getWaspFunction(String name) {
    // signature only, code must be loaded from wasm or imported and linked
    if ("floor"s == name)error1("use builtin floor!");

    auto &runtime = loadRuntime();
    if (runtime.functions.has(name)) {
        print("runtime has function "s + name);
        return runtime.functions[name];
    }
    Function f{.name = name, .is_import = true, .is_runtime = true};
    if (name.contains("(")) {
        String brace = name.substring(name.indexOf('(') + 1, name.indexOf(')'));
        f.name = name.substring(0, name.indexOf('('));
        auto args = brace.split(", ");
        for (auto arg: args)
            f.signature.add(mapType(arg));
        if (f.name == "square")f.signature.returns(int32t);
        if (f.name == "pow")f.signature.returns(float64t);
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
            f.signature.add(int32t, name = "argc").add(i32, name = "argv").
                    returns(int32t);
        else if (name == "getField")f.signature.add(node_pointer).add(smarti64).returns(node); // wth
        else if (name == "smartNode")f.signature.add(int64s).returns(node);
        else if (name == "pow")f.signature.add(float64t).add(float64t).returns(float64t);
        else if (name == "kindName")f.signature.add(type32).returns(charp);
        else if (name == "formatReal")f.signature.add(float64t).returns(charp);
        else if (name == "strlen")f.signature.add(charp).returns(int32t);
        else if (name == "free")f.signature.add(pointer);
        else if (name == "square")f.signature.add(int32t).returns(int32t);
        else if (name == "malloc")f.signature.add(size32).returns(pointer); // todo 64
        else if (name == "aligned_alloc")f.signature.add(size32, name = "alignment").add(size32).returns(pointer);
        else if (name == "realloc")f.signature.add(pointer).add(size32).returns(pointer);
        else if (name == "calloc")f.signature.add(size32).add(size32).returns(pointer);
        else if (name == "memcpy")f.signature.add(pointer).add(pointer).add(size32);
        else if (name == "memset")f.signature.add(pointer).add(int32t).add(size32);
        else if (name == "memcmp")f.signature.add(pointer).add(pointer).add(size32).returns(int32t);
        else if (name == "memmove")f.signature.add(pointer).add(pointer).add(size32);
        else if (name == "memchr")f.signature.add(pointer).add(int32t).add(size32).returns(pointer);
        else if (name == "strchr")f.signature.add(charp).add(int32t).returns(charp);
        else if (name == "strrchr")f.signature.add(charp).add(int32t).returns(charp);
        else if (name == "strcat")f.signature.add(charp).add(charp).returns(charp);
        else if (name == "strncat")f.signature.add(charp).add(charp).add(size32).returns(charp);
        else if (name == "strcmp")f.signature.add(charp).add(charp).returns(int32t);
        else if (name == "strncmp")f.signature.add(charp).add(charp).add(size32).returns(int32t);
        else if (name == "strcpy")f.signature.add(charp).add(charp).returns(charp);
        else if (name == "strncpy")f.signature.add(charp).add(charp).add(size32).returns(charp);
        else if (name == "strdup")f.signature.add(charp).returns(charp);
        else if (name == "strndup")f.signature.add(charp).add(size32).returns(charp);
        else if (name == "strpbrk")f.signature.add(charp).add(charp).returns(charp);
        else if (name == "strspn")f.signature.add(charp).add(charp).returns(size32);
        else if (name == "strcspn")f.signature.add(charp).add(charp).returns(size32);
        else if (name == "strstr")f.signature.add(charp).add(charp).returns(charp);
        else if (name == "strtok")f.signature.add(charp).add(charp).returns(charp);
        else if (name == "getchar")f.signature.returns(int32t);
        else if (name == "get_char")f.signature.returns(int32t);
        else if (name == "get_int")f.signature.returns(int32t);
        else if (name == "get_float")f.signature.returns(float32t);
        else if (name == "get_double")f.signature.returns(float64t);
        else if (name == "str")f.signature.add(charp).returns(strings);
        else if (name == "puts")f.signature.add(charp);
        else if (name == "putx")f.signature.add(int32t);
        else if (name == "putf")f.signature.add(float32t);
        else if (name == "putd")f.signature.add(float64t);
        else if (name == "putp")f.signature.add(pointer);
        else if (name == "putl")f.signature.add(longs);
        else if (name == "puti")f.signature.add(int32t);
        else if (name == "printf")f.signature.add(charp); // todo …
        else if (name == "printNode")f.signature.add(node_pointer);
        else if (name == "put_chars")f.signature.add(charp);
        else if (name == "putchar")f.signature.add(int32t);
        else if (name == "put_char")f.signature.add(int32t);
        else if (name == "put_string")f.signature.add(charp);
        else if (name == "put_int")f.signature.add(int32t);
        else if (name == "put_float")f.signature.add(float32t);
        else if (name == "put_double")f.signature.add(float64t);
        else if (name == "size_of_node")f.signature.returns(size32);
        else if (name == "size_of_string")f.signature.returns(size32);
        else if (name == "serialize")f.signature.add(node_pointer); // also Node::serialize
        else if (name == "raise")f.signature.add(charp);
        else if (name == "Parse")f.signature.add(charp).returns(node_pointer);
        else if (name == "run")f.signature.add(charp).returns(charp);
        else if (name == "system")f.signature.add(charp).returns(int32t);
        else if (name == "testCurrent");
        else if (name == "run_wasm_file")f.signature.add(charp).returns(smarti64);
        else if (name == "run_wasm")f.signature.add(charp).add(int32t).returns(smarti64);
        else if (name == "panic");
            // IGNORE js bridges :
        else if (name == "registerWasmFunction");
        else if (name.startsWith("test"));
        else if (name == "powi")f.signature.add(int64s).add(int64s).returns(int64s); // as import?
        else if (name == "pow")f.signature.add(float64t).add(float64t).returns(float64t); // as import?
        else todo("getWaspFunction "s + name);
    }
    if (!runtime.functions.has(f.name)) {
        print("manually adding runtime function "s + f.name);
        runtime.functions.add(f.name, f);
    }
    //    else todo("getWaspFunction "s + name);
    //    else if(name=="powi")f.signature.add(int32).add(int32).returns(int64s);
    return f;
}

// called by js why? if we parse runtime all module functions should be there?
// add Synonyms to mangled?
extern "C" void registerWasmFunction(chars name, chars mangled) {
    if ("floor"s == name)return; // use builtin!
    if ("loor"s == name)return; // BUG!
    return;
    getWaspFunction(name);
    if (!functions.has(name))functions.add(name, getWaspFunction(name));
    //    if (!loadRuntime().functions.has(mangled))
    //	    loadRuntime().functions.add(mangled, getWaspFunction(name));
}

bool knownSymbol(String name, Function &context) {
    return context.locals.has(name) || globals.has(name) || builtin_constants.has(name);
}
