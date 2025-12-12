
#include "Angle.h"
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

