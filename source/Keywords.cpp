
#include "Keywords.h"

#include "Angle.h"
#include "List.h"
#include "Node.h"
#include "wasm_reader.h"
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


static float function_precedence = 1000;

// todo!
// moved here so that valueNode() works even without Angle.cpp component for micro wasm module
// pre-registered builtin/runtime functions working without any import / include / require / use
chars function_list[] = {
    /*"abs"  f64.abs operator! ,*/ "norm", "square", "root", "put", "print", "printf",
    "getChar", "eq", "concat",
    "test42", "test42i", "test42f",
    "println", "puts", "putf", "putd", "puti", "putl", "len",
    "quit", "parseLong",
    "parseDouble", "strlen",
    "concat_chars", "concat_strings", "concat_bytes",
    "formatLong","formatReal","ftoa","ltoa", "itoa","itoa0", "atol", "atoi", "parseInt",
    "log", "ln", "log10", "log2", "similar",
    "putx", "putc", "get", "set", "peek", "poke", "read",
    "write",
    "$" /* getElementById */,
    //                                                               FUNCLETS via runtime for now:
    //                                                               "pow", "powi", "pow_long", "log", "log10", "log2",
    "lowerCaseUTF",
    0, 0, 0
}; // MUST END WITH 0, else BUG


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



// todo see load_aliases(); -> aliases.wasp
List<String> findAliases(String name) {
    List<String> found;
    // found+=aliases[name] todo !
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


float precedence(String name);
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


// precedence logic sits in between raw Wasp structure and concrete Angle syntax
// precedence should be in the Wasp header but the implementation is subject to moving
// like c++ here HIGHER up == lower value == more important
float precedence(String name) {
    if (eq(name, "abs"))return 0.08;
    if (eq(name, "‖"))return 0.10; // norms / abs

    if (eq(name, "."))return 0.5;
    if (eq(name, "of"))return 0.6;
    if (eq(name, "in"))return 0.7;
    if (eq(name, "from"))return 0.8;

    if (eq(name, "not"))return 1;
    if (eq(name, "¬"))return 1;
    if (eq(name, "-…"))return 1; // unary operators are immediate, no need for prescidence
    if (eq(name, "!"))return 1;
    if (eq(name, "√"))return 1; // !√1 √!-1
    if (eq(name, "^"))return 2; // todo: ambiguity? 2^3+1 vs 2^(x+1)
    if (eq(name, "**"))return 2; //
    if (eq(name, "^^"))return 2; // how did it work without??

    if (eq(name, "#"))return 3; // count
    if (eq(name, "++"))return 3;
    //	if (eq(node.name, "+"))return 3;//
    if (eq(name, "--"))return 3;

    if (eq(name, "/"))return 4.9;
    if (eq(name, "÷"))return 4.9;

    if (eq(name, "times"))return 5;
    if (eq(name, "*"))return 5;
    if (eq(name, "×"))return 5;
    if (eq(name, "add"))return 6;
    if (eq(name, "plus"))return 6;
    if (eq(name, "+"))return 6;
    if (eq(name, "minus"))return 6;
    if (eq(name, "-"))return 6;
    if (eq(name, "%"))
        return 6.1;
    if (eq(name, "rem"))return 6.1;
    if (eq(name, "modulo"))return 6.1;
    if (eq(name, "upto"))return 6.3; // range
    if (eq(name, "…"))return 6.3;
    if (eq(name, "..."))return 6.3;
    if (eq(name, ".."))return 6.3; // excluding range
    if (eq(name, "..<"))return 6.3; // excluding range
    if (eq(name, "<"))return 6.5;
    if (eq(name, "<="))return 6.5;
    if (eq(name, ">="))return 6.5;
    if (eq(name, ">"))return 6.5;
    if (eq(name, "≥"))return 6.5;
    if (eq(name, "≤"))return 6.5;
    if (eq(name, "≈"))return 6.5;
    if (eq(name, "=="))return 6.6;
    if (eq(name, "is"))return 6.6; // careful, use 'be' for := assignment
    if (eq(name, "eq"))return 6.6;
    if (eq(name, "equals"))return 6.6;
    if (eq(name, "is not"))return 6.6; // ambiguity: a == !b vs a != b
    if (eq(name, "isnt"))return 6.6;
    if (eq(name, "isn't"))return 6.6;
    if (eq(name, "equal"))return 10;
    if (eq(name, "≠"))return 10;
    if (eq(name, "!="))return 10;

    if (eq(name, "and"))return 7.1;
    if (eq(name, "&&"))return 7.1;
    if (eq(name, "&"))return 7.1;
    if (eq(name, "∧"))return 7.1; // ⚠️ todo this is POWER for non-boolean! NEVER bitwise and  1^0==0 vs 1^0==1 ⚠ WARN!
    if (eq(name, "⋀"))return 7.1;


    if (eq(name, "xor"))return 7.2;
    if (eq(name, "^|"))return 7.2;
    if (eq(name, "⊻"))return 7.2;

    if (eq(name, "or"))return 7.2;
    if (eq(name, "||"))return 7.2;
    if (eq(name, "∨"))return 7.2;
    if (eq(name, "⋁"))return 7.2;
    //	if (eq(name, "|"))return 7.2;// todo pipe special

    if (eq(name, ":"))return 7.5; // todo:
    if (eq(name, "?"))return 7.6;

    if (name.in(function_list)) // f 1 > f 2
        return 8; // 1000;// function calls outmost operation todo? add 3*square 4+1


    if (eq(name, "⇒"))return 11; // lambdas
    if (eq(name, "=>"))return 11;
    if (eq(name, "::"))return 11; // todo lambda symbol? square = x :: x*x

    //	if (eq(name, ":"))return 12;// construction
    if (eq(name, "="))return 12; // declaration
    if (eq(name, ":="))return 13;
    if (eq(name, "be"))return 13; // counterpart 'is' for ==
    if (eq(name, "::="))return 14; // globals setter
    //	if (eq(name, "is"))return 13;// careful, could be == (6.6)

    if (eq(name, "else"))return 13.09;
    if (eq(name, "then"))return 13.15;
    if (eq(name, "if"))return 100;
    if (eq(name, "while"))return 101;
    //	if (eq(name, "once"))return 101;
    //	if (eq(name, "go"))return 101;
    if (name.in(functor_list)) // f 1 > f 2
        return function_precedence; // if, while, ... statements calls outmost operation todo? add 3*square 4+1

    if (eq(name, "return"))return 1000;
    return 0; // no precedence
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

String &checkCanonicalName(String &name) {
    if (name == "**")info("The power operator in angle is simply '^' : 3^2=9."); // todo: alias infoing mechanism
    if (name == "^^")info("The power operator in angle is simply '^' : 3^2=9. Also note that 1 xor 1 = 0");
    if (name == "||")info("The disjunction operator in angle is simply 'or' : 1 or 0 = 1");
    if (name == "&&")info("The conjunction operator in angle is simply 'and' : 1 and 1 = 1");
    return name;
}