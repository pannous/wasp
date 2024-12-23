#include "Util.h"
#include "List.h"
#include "Node.h"
#include "Wasp.h"
#include "Angle.h"
#include "String.h"
#include "Map.h"
#include "tests.h"
#include "Paint.h"

#pragma GCC diagnostic ignored "-Wdeprecated"

#import "test_angle.cpp"
#import "test_wast.cpp"
#import "test_wasm.cpp"

#pragma GCC diagnostic pop

#include "wasm_runner.h"
#include "WitReader.h"
#include "types/Number.h"
#include "own_merge/type.h"
#include "own_merge/common.h"


#include <wasmedge/wasmedge.h>
#include <stdio.h>

struct debug_struct {
    byte a;
    byte b;
    byte c;
    byte d;
    byte e;
    byte f;
    byte g;
    byte h;
    int i;
    int j;
    int k;
    int l;
};

typedef struct {
    int32_t value;  // The single field in the struct
} wasm_struct1;

int test_wasmedge_gc() {
    // Initialize WasmEdge runtime
    WasmEdge_ConfigureContext *Conf = WasmEdge_ConfigureCreate();
    WasmEdge_ConfigureAddProposal(Conf, WasmEdge_Proposal_ReferenceTypes);
    WasmEdge_ConfigureAddProposal(Conf, WasmEdge_Proposal_GC);

    WasmEdge_VMContext *VM = WasmEdge_VMCreate(Conf, NULL);

    // Load the WASM module
//    WasmEdge_String ModulePath = WasmEdge_StringCreateByCString("gc_example.wasm");
    WasmEdge_String ModuleName = WasmEdge_StringCreateByCString("gc_example");
    const char *path = "/Users/me/dev/script/wasm/gc_structs/gc_example.wasm";
    WasmEdge_Result Result = WasmEdge_VMRegisterModuleFromFile(VM, ModuleName, path);
    if (!WasmEdge_ResultOK(Result)) {
        printf("Failed to load module: %s\n", WasmEdge_ResultGetMessage(Result));
        return 1;
    }

    // Run the `new_object` function
    WasmEdge_String FuncName = WasmEdge_StringCreateByCString("new_object");
    WasmEdge_Value Params[1] = {WasmEdge_ValueGenI32(32)};
    WasmEdge_Value Returns[1];
//    WasmEdge_VMRunWasmFromBuffer()
//    Result = WasmEdge_VMRunRegisteredFunction(VM, ModuleName.Buf, FuncName.Buf, Params, 1, Returns, 1);
// Load the WASM module into a buffer
    FILE *wasm_file = fopen(path, "rb");
    if (!wasm_file) {
        printf("Failed to open file: %s\n", path);
        return 1;
    }
    fseek(wasm_file, 0, SEEK_END);
    long wasm_file_size = ftell(wasm_file);
    fseek(wasm_file, 0, SEEK_SET);

    uint8_t *wasm_buffer = (uint8_t *) malloc(wasm_file_size);
    if (wasm_buffer == NULL) {
        printf("Failed to allocate memory for WASM buffer.\n");
        fclose(wasm_file);
        return 1;
    }
    fread(wasm_buffer, 1, wasm_file_size, wasm_file);
    fclose(wasm_file);

    // Run the WASM using the `WasmEdge_VMRunWasmFromBuffer` function
//    WasmEdge_Value Params[1] = {WasmEdge_ValueGenI32(32)};
//    WasmEdge_Value Returns[1];
    Result = WasmEdge_VMRunWasmFromBuffer(VM, wasm_buffer, wasm_file_size, FuncName, Params, 1, Returns, 1);

    free(wasm_buffer); // Free the allocated buffer after use

    if (!WasmEdge_ResultOK(Result)) {
        printf("Failed to execute function: %s\n", WasmEdge_ResultGetMessage(Result));
        return 1;
    }
    if (!WasmEdge_ResultOK(Result)) {
        printf("Failed to execute function: %s\n", WasmEdge_ResultGetMessage(Result));
        return 1;
    }


    auto mem = WasmEdge_StringCreateByCString("memory");
//    WasmEdge_ModuleInstanceContext *module_ctx2 = WasmEdge_VMGetStoreContext(VM);
    WasmEdge_StoreContext *storeContext = WasmEdge_VMGetStoreContext(VM);
    const WasmEdge_ModuleInstanceContext *module_ctx = WasmEdge_VMGetActiveModule(VM);
    WasmEdge_MemoryInstanceContext *memory_ctx = WasmEdge_ModuleInstanceFindMemory(module_ctx, mem);
    uint8_t *memo = WasmEdge_MemoryInstanceGetPointer(memory_ctx, 0, 0);
    if (memo)
        wasm_memory = memo;
    else
        warn("⚠️Can't connect wasmedge memory");


    // Print the result (object reference)
    WasmEdge_Value Return = Returns[0];
    void *pVoid = WasmEdge_ValueGetExternRef(Return);
    wasm_struct1 *gc_struct = (wasm_struct1 *)pVoid;
    printf("Result: %d\n", gc_struct->value);
    if(WasmEdge_ValTypeIsRef(Return.Type)) {
        printf("Result REF: %p\n", pVoid);
    } else {
        printf("Result: %d\n", WasmEdge_ValueGetI32(Return));
    }
    debug_struct *debugs = (debug_struct *) pVoid;
    printf("Result: %p\n", pVoid);
    printf("Result: %d\n", *(int*)pVoid);
    printf("Result: %d\n", WasmEdge_ValueGetI32(Return));
    exit(0);

    // Cleanup
    WasmEdge_VMDelete(VM);
    WasmEdge_ConfigureDelete(Conf);

    return 0;
}

void testMatrixOrder() {
    assert_emit("m=([[1, 2], [3, 4]]);m[0][1]", 2);
    assert_emit("([[1, 2], [3, 4]])[0][1]", 2);
    assert_emit("([[1, 2], [3, 4]])[1][0]", 3);
    assert_emit("([1, 2], [3, 4])[1][0]", 3);
    assert_emit("(1, 2; 3, 4)[1][0]", 3);
    assert_emit("(1, 2; 3, 4)[1,0]", 3);
    assert_emit("(1 2, 3 4)[1,0]", 3);
}

template<class S>
void testListGrowth() {
    List<S> list; // List<S*> even better!
    for (int i = 0; i < 1000; i++) {
        list.add(*new S());
    }
    check_eq(list.size(), 1000);
    for (int i = 0; i < 15; i++) { // 10 VERY SLOW in new implementation! DONT DO 20 => growth by 2^20!!!
        list.grow();
    }
    check(list.capacity > 1000);
}

void testListGrowthWithStrings() {
    List<String> list;
    for (int i = 0; i < 1000; i++) {
        list.add(String(i));
    }
    check_eq(list.size(), 1000);
    check_eq(list[999], new String(999));
    for (int i = 0; i < 10; i++) {
        list.grow();// lots of deep copy!!
    }
    check(list.capacity > 100000);
    check_eq(list[999], new String(999));
}


//void testDwarf();
//void testSourceMap();
void testAssert() {
    assert_emit("assert 1", 1);
    assert_throws("assert 0");// todo make wasm throw, not compile error?
}

void testForLoops() {
//    assert_emit("for i in 1 to 5 : {print i};i", 6);
    assert_emit("for i in 1 to 5 : {puti i};i", 6);
    assert_emit("for i in 1 to 5 {puti i}", 5);
    assert_emit("for i in 1 to 5 {puti i};i", 6); // after loop :(
    assert_emit("for i in 1 to 5 : puti i", 5);
    assert_emit("for i in 1 to 5\n  puti i\ni", 5);
    assert_emit("for i in 1 to 5\n  puti i\ni", 5);
//    assert_emit("sum=0\nfor i in (1..3) {sum+=i}\nsum", 6);
//    assert_emit("sum=0;for i in (1..3) {sum+=i};sum", 6);
//    assert_emit("sum=0;for i=1..3;sum+=i;sum", 6);
}

void testNamedDataSections() {
    assert_emit("fest='def';test='abc'", "abc");
    exit(0);
}

void testAutoSmarty() {
    assert_emit("11", 11);
    assert_emit("'c'", 'c');
    assert_emit("'cc'", "cc");
    assert_emit("π", pi);
//    assert_emit("{a:b}", new Node{.name="a"));
}

void testArguments() {
    assert_emit("#params", 0);// no args, but create empty List anyway
    // todo add context to wasp variable $params
}

void testBadType() {
    skip(
    // TODO strict mode a:b=c => b is type vs data mode a:b => b is data HOW?
            assert_throws("x:yz=1");// "yz" is not a type
    )
}

void testDeepType() {
    parse("a=$canvas.tagName");
//    check_is(result.kind, smarti64);
//    check_is(result.kind, Kind::strings);
}

void testInclude() {
//    assert_emit("include test-include.wasp", 42);
//    assert_emit("use test-include.wasm", 42);
    assert_emit("include test/lib.wasp", 42);
//    assert_emit("include test/lib.wast", 42);
    assert_emit("use test/lib.wasm; test", 42);
//    assert_emit("use https://pannous.com/files/lib.wasm; test", 42);
//    assert_emit("use git://pannous/waps/test/lib.wasm; test", 42);
//    assert_emit("use system:test/lib.wasm; test", 42); // ^^

}

void testExceptions() {
//    assert_emit("(unclosed bracket",123);

    assert_throws("x:int=1;x='ok'");
    assert_throws("x:int=1;x=1.1");
//    assert_emit("x:int=1;x=1.0",1); // might be cast by compiler
//    assert_emit("x=1;x='ok';x=1", 1); // untyped x can be reassigned
    assert_throws("'unclosed quote");
    assert_throws("\"unclosed quote");
    assert_throws("unclosed quote'");
    assert_throws("unclosed quote\"");
    assert_throws("unclosed bracket)");
    assert_throws("(unclosed bracket");
}

void testNoBlock() { // fixed
    assert_parses(R"(
#see math.wasp !
τ=π*2
#assert τ≈6.2831853
#τ≈6.2831853
#τ==6.2831853
    )");
}

void testTypeConfusion() {
    assert_throws("x=1;x='ok'");
    assert_throws("x=1;x=1.0");
    assert_throws("double:=it*2");// double is type i64!
    // todo: get rid of stupid type name double, in C it's float64 OR int64 anyway
}

void testVectorShim() {
    assert_emit("v=[1 2 3];w=[2 3 4];v*w", 2 + 6 + 12);
}

void testHtmlWasp() {
    eval("html{bold{Hello}}"); // => <html><body><bold>Hello</bold></body></html> via appendChild bold to body
    eval("html: h1: 'Hello, World!'"); // => <html><h1>Hello, World!</h1></html>
//	eval("html{bold($myid style=red){Hello}}"); // => <bold id=myid style=red>Hello</bold>
}

void testJS() {
    // todo remove (local $getContext i32)  !
    eval("$canvas.getContext('2d')"); // => invokeReference(canvas, getContext, '2d')
    eval("js{alert('Hello')}"); // => <script>alert('Hello')</script>
    eval("script{alert('Hello')}"); // => <script>alert('Hello')</script>
}

void testInnerHtml() {
#if not WEBAPP and not MY_WASM
    return;
#endif
    auto html = parse("<html><bold>test</bold></html>");
    check_is(html.kind, Kind::strings);
    check(html.value.string);
    check_is(*html.value.string, "<bold>test</bold>");
    auto serialized = html.serialize();
    check_is(serialized, "<html><bold>test</bold></html>");
//	eval("<html><script>alert('ok')");
//	eval("<html><script>alert('ok')</script></html>");
    eval("<html><bold id=b ok=123>test</bold></html>");
    assert_is("$b.ok", 123);
    eval("<script>console.log('ok!')</script>");
    eval("<script>alert('alert ok!')</script>");// // pop up window NOT supported by WebView, so we use print instead
//	eval("$b.innerHTML='<i>ok</i>'");
//	eval("<html><bold id='anchor'>…</bold></html>");
//	eval("$anchor.innerHTML='<i>ok</i>'");
//
////	eval("x=<html><bold>test</bold></html>;$results.innerHTML=x");
//	eval("$results.innerHTML='<bold>test</bold>'");
}

void testHtml() {
//	testHtmlWasp();
//	testJS();
    testInnerHtml();
}


void testReplaceAll() {
    String s = "abaabaa";
    auto replaced = s.replaceAll("a", "ca");
//	auto replaced = s.replaceAll('a', "ca");
    check_is(replaced, "cabcacabcaca");
    auto replaced2 = replaced.replaceAll("ca", "a");
    check_is(replaced2, "abaabaa");
    replaced2.replaceAllInPlace('b', 'p');
    check_is(replaced2, "apaapaa");
}

void testFetch() {
    // todo: use host fetch if available
    auto string1 = fetch("https://pannous.com/files/test");
    auto res = String(string1).trim();
    if (res.contains("not available")) {
        print("fetch not available. set CURL=1 in CMakelists.txt or use host function");
        return;
    }
    check_eq(res, "test 2 5 3 7");
    check_emit("fetch https://pannous.com/files/test", "test 2 5 3 7");
}

void test_getElementById() {
    result = analyze(parse("$result"));
    assert_equals(result.kind, (int64) externref);
    auto nod = eval("$result");
    print(nod);
}

void testCanvas() {
    result = analyze(parse("$canvas"));
    assert_equals(result.kind, (int64) externref);
    auto nod = eval("    ctx = $canvas.getContext('2d');\n"
                    "    ctx.fillStyle = 'red';\n"
                    "    ctx.fillRect(10, 10, 150, 100);");
    print(nod);
}

// run in APP (or browser?)
void testDom() {
    print("testDom");
    preRegisterFunctions();
    result = analyze(parse("getElementById('canvas')"));
    assert_equals(result.kind, call);
    result = eval("getElementById('canvas');");
//	print(typeName(result.kind));
//	assert_equals(result.kind, strings); // why?
//	assert_equals(result.kind, longs); // todo: can't use smart pointers for elusive externref
//	assert_equals(result.kind, bools); // todo: can't use smart pointers for elusive externref
    print(typeName(30));
    print(typeName(9));
//	assert_equals(result.kind, 30);//
//	assert_equals(result.kind,9);//
//	assert_equals(result.kind, (int64) externref); // todo: can't use smart pointers for elusive externref
//	result = eval("document.getElementById('canvas');");
//	result = analyze(parse("$canvas"));
//	assert_equals(result.kind, (int64) externref);
}

inline void print(Primitive l) {
    print(typeName(l));
}

void testDomProperty() {
#ifndef WEBAPP
    return;
#endif
    result = eval("getExternRefPropertyValue($canvas,'width')"); // ok!!
    check_is(result.value.longy, 300); // only works because String "300" gets converted to BigInt 300
//	result = eval("width='width';$canvas.width");
    result = eval("$canvas.width");
    check_eq(result.value.longy, 300);
//	return;
    result = eval("$canvas.style");
    check_is(result.kind, strings);
//	check_is(result.kind, stringp);
    if (result.value.string)
        check_eq(*result.value.string, "dfsa");
//	getExternRefPropertyValue OK  [object HTMLCanvasElement] style [object CSSStyleDeclaration]
// ⚠️ But can't forward result as smarti or stringref:  SyntaxError: Failed to parse String to BigInt
// todo : how to communicate new string as RETURN type of arbitrary function from js to wasp?
// call Webview.getString(); ?

//	embedder.trace('canvas = document.getElementById("canvas");')
//	print(nod);
}

void testHostDownload() {
    assert_emit("download http://pannous.com/files/test", "test 2 5 3 7\n");
}

void testHostIntegration() {
    testHostDownload();
    test_getElementById();
    testDom();
    testDomProperty();
    testCanvas();
    testInnerHtml();
    testJS();
    testFetch();
}


void testTypes() {
    clearAnalyzerContext();
    result = analyze(parse("chars a"));
    assert_equals(result.kind, Kind::reference);
    assert_equals(result.type, &ByteCharType);// todo char ≠ char* !
    assert_equals(result.name, "a");


    result = analyze(parse("int a"));
    assert_equals(result.kind, Kind::reference);
    assert_equals(result.type, &IntegerType);// IntegerType
    assert_equals(result.name, "a");

    result = analyze(parse("string b"));
    assert_equals(result.kind, Kind::reference);
    assert_equals(result.type, &StringType);
    assert_equals(result.name, "b");

    result = analyze(parse("float a,string b"));
    let result0 = result[0];
    assert_equals(result0.kind, Kind::reference);
//	assert_equals(result0.kind, Kind::declaration);
//	todo at this stage it should be a declaration?

    assert_equals(result0.type, &DoubleType);
    assert_equals(result0.name, "a");
    let result1 = result[1];
    assert_equals(result1.kind, Kind::reference);
    assert_equals(result1.type, &StringType);
    assert_equals(result1.name, "b");

}

void testTypes2() {
    result = analyze(parse("a:chars"));
    assert_equals(result.kind, Kind::reference);
    assert_equals(result.type, &ByteCharType);
    assert_equals(result.name, "a");


    result = analyze(parse("a:int"));
    assert_equals(result.kind, Kind::reference);
    assert_equals(result.type, &IntegerType);// IntegerType
    assert_equals(result.name, "a");

    result = analyze(parse("b:string"));
    assert_equals(result.kind, Kind::reference);
    assert_equals(result.type, &StringType);
    assert_equals(result.name, "b");

    result = analyze(parse("a:float,b:string"));
    let result0 = result[0];
    assert_equals(result0.kind, Kind::reference);
//	assert_equals(result0.kind, Kind::declaration);
//	todo at this stage it should be a declaration?
    assert_equals(result0.type, &DoubleType);
    assert_equals(result0.name, "a");
    let result1 = result[1];
    assert_equals(result1.kind, Kind::reference);
    assert_equals(result1.type, &StringType);
    assert_equals(result1.name, "b");
}


void testTypedFunctions() {
    // todo name 'id' clashes with 'id' in preRegisterFunctions()
    result = analyze(parse("int tee(float b, string c){b}"));
    check_is(result.kind, Kind::declaration);
    check_is(result.name, "tee");
    auto signature_node = result["@signature"];
//	auto signature_node = result.metas()["signature"];
    if (not signature_node.value.data)error("no signature");
    Signature &signature = *(Signature *) signature_node.value.data;
    check_is(signature.functions.first()->name, "tee")
    check_is(signature.parameters.size(), 2)
    check_is(signature.parameters.first().name, "b")
    check_is(signature.parameters.first().type, reals);// use real / number for float64  float32
    check_is(signature.parameters.last().name, "c")
    check_is(signature.parameters.last().type, strings);
    let params = signature.parameters.map(+[](Arg f) { return f.name; });
    check_is(params.first(), "b");
}

void testEmptyTypedFunctions() {
    // todo int a(){} should be compiler error
    // todo do we really want / need int a(); void a(){} ?
//	if(ch=='{' and next=='}' and previous==')'){
//		actual.setType(declaration, false);// a(){} => def a!
//		proceed();
//		proceed();
//		break;
//	}
    result = analyze(parse("int a(){}"));
    check_is(result.kind, Kind::declaration);
    check_is(result.name, "a");
    auto signature_node = result["@signature"];
    Signature signature = *(Signature *) signature_node.value.data;
    check_is(signature.functions.first()->name, "a")
    let names2 = signature.functions.map(+[](Function *f) {
        return f->name;
    });
    check_is(names2.size(), 1);
    check_is(names2.first(), "a");

    result = analyze(parse("int a();"));
    check_is(result.kind, Kind::declaration);// header signature
    check_is(result.type, IntegerType);
    check_is(result.name, "a");
}

void testPolymorphism() {
    // debug:
//	auto debug_node = parse("string aaa(string a){return a};\nfloat bbb(float b){return b+1}");
//	auto debug_fun = analyze(debug_node);
    auto node = parse("string test(string a){return a};\nfloat test(float b){return b+1}");
    auto fun = analyze(node);
    auto function = functions["test"];
    check_is(function.is_polymorphic, true);
    check_is(function.variants.size(), 2);
    check_is(function.variants[0]->signature.size(), 1);
//	check_is(function.variants[0].signature.parameters[0].type, (Type) strings); todo
    check_is(function.variants[0]->signature.parameters[0].type, (Type) stringp);
    auto variant = function.variants[1];
    check_is(variant->signature.size(), 1);
    check_is(variant->signature.parameters[0].type, (Type) float32);
}

void testPolymorphism2() {
    clearAnalyzerContext();
    auto node = parse("fun test(string a){return a};\nfun test(float b){return b+1}");
    auto fun = analyze(node);
    auto function = functions["test"];
    check_is(function.is_polymorphic, true);
    check_is(function.variants.size(), 2);
    check_is(function.variants[0]->signature.size(), 1);
    check_is(function.variants[0]->signature.parameters[0].type, (Type) int32);
    check_is(function.variants[1]->signature.size(), 1);
    check_is(function.variants[1]->signature.parameters[0].type, (Type) float32);
}


void testPolymorphism3() {
    assert_emit("fun test(string a){return a};\nfun test(float b){return b+1};\ntest('ok')", "ok");
    assert_emit("fun test(string a){return a};\nfun test(float b){return b+1};\ntest(1.0)", 2.0);
}

//#import "pow.h"
//void testOwnPowerExponentialLogarithm() {
//	check_is(exp(1), 2.718281828459045);
//	check_is(exp(5.5), 244.69193226422033);
//	auto x = powerd(1.5, 5.5);
//	printf("1.5^5.5=%f", x);
//	check_eq(x, 9.30040636712988);
//	auto x1 = powerd(2.5, 1.5);
//	printf("2.5^1.5=%f", x1);
//	check_eq(x1, 3.952847075210474);
//	auto x2 = powerd(2.5, 3.5);
//	check_eq(x2, 24.705294220065465);
//}

void testGenerics() {
    auto type = Type(Generics{.kind = array, .value_type = int16});
//    auto header= type.value & array;
//    auto header= type.value & 0xFFFF0000; // todo <<
    auto header = type.value & 0x0000FFFF; //todo ??
    check_eq(header, array);
}

void testNumbers() {
    Number n = 1;// as comfortable BigInt Object used inside wasp
    check(n == 1.0);
    check(n / 2 == 0.5);
    check(((n * 2) ^ 10) == 1024);
}

void testFunctionDeclaration() {
//    auto node1 = analyze(parse("fn main(){}"));
//    check(node1.kind==declaration);
//    check(node1.name=="main");
    clearAnalyzerContext();
    auto node2 = analyze(parse("fun test(float a):int{return a*2}"));
    check(node2.kind == declaration);
    check(node2.name == "test");
    check_is(functions["test"].signature.size(), 1);
    check_is(functions["test"].signature.parameters[0].name, "a");
    check_is(functions["test"].signature.parameters[0].type, (Type) float_type);
    check(functions["test"].body);
    check_is(*functions["test"].body, analyze(parse("return a*2")));
}

void testRenameWasmFunction() {
    Module &module1 = loadModule("samples/test.wasm");
    module1.functions.at(0).name = "test";
    module1.save("samples/test2.wasm");
}

void testPower() {
    assert_equals(powi(10, 1), 10l);
    assert_equals(powi(10, 2), 100l);
    assert_equals(powi(10, 3), 1000l);
    assert_equals(powi(10, 4), 10000l);
    assert_equals(parseLong("8e6"), 8000000l);
    skip(
            assert_equals(parseLong("8e-6"), 1.0 / 8000000l);
    )
    assert_equals(parseDouble("8.333e-3"), 0.008333l);
    assert_equals(parseDouble("8.333e3"), 8333.0l);
    assert_equals(parseDouble("8.333e-3"), 0.008333l);
//    assert_equals(ftoa(8.33333333332248946124e-03), "0.0083");
    assert_equals(powi(10, 1), 10l);
    assert_equals(powi(10, 2), 100l);
    assert_equals(powi(10, 4), 10000l);
    assert_equals(powi(2, 2), 4l);
    assert_equals(powi(2, 8), 256l);
    skip(
            assert_equals(powd(2, -2), 1 / 4.);
            assert_equals(powd(2, -8), 1 / 256.);
            assert_equals(powd(10, -2), 1 / 100.);
            assert_equals(powd(10, -4), 1 / 10000.);
    )

}

void testMaps0() {
    Map<int, long> map;
    check(map.values[0] == map[0]);
    check(map.values == &(map[0]));
    map[0] = 2;
    check(map.values[0] == 2);
    check(map.size() == 1);
    map[2] = 4;
    check(map.size() == 2);
    check(map.values[1] == 4);
    check(map.keys[1] == 2);
    print((int) map[0]);
    print((int) map[2]);
    print(map[(size_t) 0]);
    print(map[(size_t) 1]);
    check(map[0] == 2);
    check(map[2] == 4);
}


void testMapOfStrings() {
    Map<String, chars> map;
    map["a"] = "1";
    check(map.size() == 1);
    map["a"] = "1";
    check(map.size() == 1);
    check(map.keys[0] == "a");
    check(map.values[0] == "1"s);
    check(map["a"] == "1"s);
//    check(!map.has("b"));
    check(map.position("b") == -1);
    map["b"] = "2";
    check(map.size() == 2);
    check(map.keys[1] == "b");
    check(map.values[1] == "2"s);
    check(map["b"] == "2"s);
}

void testMapOfStringValues() {
    Map<chars, String> map;
    map["a"] = "1";
    check(map.size() == 1);
    check(map.keys[0] == "a"s);
    check(map.values[0] == "1");
    check(map["a"] == "1");
    map["b"] = "2";
    check(map.size() == 2);
    check(map.keys[1] == "b"s);
    check(map.values[1] == "2");
    check(map["b"] == "2");
}

void testMaps1() {
    functions.clear();
    functions.insert_or_assign("abcd", {.name="abcd"});
    functions.insert_or_assign("efg", {.name="efg"});
    check_is(functions.size(), 2);
    check(functions["abcd"].name == "abcd");
    check(functions["efg"].name == "efg");
}

void testMaps2() {
    functions.clear();
    Function abcd;
    abcd.name = "abcd";
    functions["abcd"] = abcd;
    functions["efg"] = {.name="efg"};
    functions["abcd"] = {.name="abcd"};
    functions["efg"] = {.name="efg"};
    check_is(functions.size(), 2);
    print(functions["abcd"]);
    print(functions["abcd"].name);
    check(functions["abcd"].name == "abcd");
    check(functions["efg"].name == "efg");
}

void testMaps() {
    testMaps0();// ok
    testMapOfStrings();
    testMapOfStringValues();
    testMaps1();
    testMaps2();// now ok
}


void testHex() {
    assert_equals(hex(18966001896603L), "0x113fddce4c9b");
    assert_is("42", 42);
    assert_is("0xFF", 255);
    assert_is("0x100", 256);
    assert_is("0xdce4c9b", 0xdce4c9b);
//    assert_is("0x113fddce4c9b", 0x113fddce4c9bl); todo
//	assert_is("0x113fddce4c9b", 0x113fddce4c9bL);
}

void test_fd_write() { // built-in wasi function
    //    assert_emit("x='hello';fd_write(1,20,1,8)", (int64) 0);// 20 = &x+4 {char*,len}
//    assert_emit("puts 'ok';proc_exit(1)\nputs 'no'", (int64) 0);
//    assert_emit("quit",0);
    assert_emit("x='hello';fd_write(1,x,1,8)", (int64) 0);// &x+4 {char*,len}
//    assert_run("len('123')", 3); // Map::len
//    quit()
    assert_emit("puts 'ok'", (int64) 0); // connect to wasi fd_write
    loadModule("wasp");
    assert_emit("puts 'ok'", (int64) 0);
    assert_emit("puti 56", 56);
    assert_emit("putl 56", 56);
//    assert_emit("putx 56", 56);
    assert_emit("putf 3.1", 0);

    check(module_cache.has("wasp-runtime.wasm"s.hash()))

}

void testEnumConversion() {
#if not TRACE
    Valtype yy = (Valtype) Primitive::charp;
    int i = (int) Primitive::charp;
    int i1 = (int) yy;// CRASHES in Trace mode WHY?
    check_is(stackItemSize(Primitive::wasm_float64), 8);
    check_is(i, i1);
    check((Type) Primitive::charp == yy);
    check((Type) yy == Primitive::charp);
    check(Primitive::charp == (Type) yy);
    check(yy == (Type) Primitive::charp);
    check((int) yy == (int) Primitive::charp);
#endif
}

void bindgen(Node &n) {
//    todo parserOptions => formatOptions => Format ! inheritable
//    todo interface host-funcs {current-user: func() -> string}
    print(n.serialize());
}

// https://github.com/WebAssembly/component-model/blob/main/design/mvp/WIT.md#item-use
void testUse() {
    parse("use * from other-file");
    parse("use { a, list, of, names } from another-file");
    parse("use { name as other-name } from yet-another-file");
    // MY SYNTAX :
    parse("use other-file");// MY SYNTAX
    parse("use all from other-file");// MY SYNTAX redundant
    parse("use name from yet-another-file");// MY SYNTAX
    parse("use name from yet-another-file as other-name");// MY SYNTAX
//    parse("use name as other-name from yet-another-file");// MY SYNTAX
}

void testClass() {
    analyze(parse("public data class Person(string FirstName, string LastName);"));
    analyze(parse("public data class Student : Person { int ID; }"));
    analyze(parse("var person = new Person('Scott', 'Hunter'); // positional construction"));
    analyze(parse("otherPerson = person with { LastName = \"Hanselman\" };"));
//    "var (f, l) = person;                        // positional deconstruction"
}

void testStruct() { // builtin with struct/record
    assert_emit("struct a{x:int y:int z:int};a{1 3 4}.y", 3);
    return;
    assert_emit("struct a{x:int y:float};a{1 3.2}.y", 3.2);
    assert_emit("struct a{x:int y:float};b a{1 .2};b.y", .2);
    assert_emit("struct a{x:int y:float};b:a{1 .2};b.y", .2);
    assert_emit("struct a{x:int y:float};b=a{1 .2};b.y", .2);
    assert_emit("struct a{x:int y:float};a b{1 .2};b.y", .2);
    assert_emit("record a{x:u32 y:float32};a b{1 .2};b.y", .2);
    assert_emit(R"(
record person {
    name: string,
    age: u32,
    has-lego-action-figure: bool,
}; x=person{age:22}; x.age)", 22); // todo require optional fields marked as such with '?'
}


void testStruct2() {
    const char *code0 = "struct point{a:int b:int c:string}";
    Node &node = parse(code0);
//    assert_equals(node.kind, Kind::structs);
    assert_equals(node.length, 3);
    assert_equals(IntegerType, node[1].type);
//    const char *code = "struct point{a:int b:int c:string};x=point(1,2,'ok');x.b";
// basal node_pointer act as structs
    assert_emit("point{a:int b:int c:string};x=point(1,2,'ok');x.b", 2)
    assert_emit("data=[1,2,3];struct point{a:int b:int c:string};x=data as struct;x.b", 2)
}


void test_c_numbers() {
//    check(0x1000000000000000l==powi(2,60))
    unsigned int x = -1;
    unsigned int y = 0xFFFFFFFF;
//    signed int biggest = 0x7FFFFFFF;
//    signed int smallest = 0x80000000;// "implementation defined" so might not always pass
    signed int z = -1;
    check(x == y)
    check(x == z)
    check(z == y)
    check((int) -1 == (unsigned int) 0xFFFFFFFF)
}

void testArraySize() { // todo!
    // There should be one-- and preferably only one --obvious way to do it.
    // requires struct lookup and aliases
    assert_emit("pixel=[1 2 4];#pixel", 3);
//  assert_emit("pixel=[1 2 4];pixel#", 3);
    assert_emit("pixel=[1 2 4];pixel size", 3);
    assert_emit("pixel=[1 2 4];pixel length", 3);
    assert_emit("pixel=[1 2 4];pixel count", 3);
    assert_emit("pixel=[1 2 4];pixel number", 3);// ambivalence with type number!
    assert_emit("pixel=[1 2 4];pixel.size", 3);
    assert_emit("pixel=[1 2 4];pixel.length", 3);
    assert_emit("pixel=[1 2 4];pixel.count", 3);
    assert_emit("pixel=[1 2 4];pixel.number", 3);// ambivalence cast
    assert_emit("pixels=[1 2 4];number of pixels ", 3);
    assert_emit("pixels=[1 2 4];size of pixels ", 3);
    assert_emit("pixels=[1 2 4];length of pixels ", 3);
    assert_emit("pixels=[1 2 4];count of pixels ", 3);
    assert_emit("pixel=[1 2 3];pixel.add(5);#pixel", 4);
}


void testArrayOperations() { // todo!
    testArraySize();
    // todo 'do' notation to modify versus return different list!
    assert_emit("pixel=[1 2 3];do add 4 to pixel; pixel", Node(1, 2, 3, 4, 0));
    assert_emit("pixel=[1 2 3];y=pixel + 4; pixel", Node(1, 2, 3, 0));

//        assert_throws("pixel=[1 2 3];pixel + 4;pixel");// unused non-mutating operation
    assert_emit("pixels=[1 2 4];pixel#3", 4);// plural!
    assert_emit("pixel=[1 2 3];pixel + [4]", Node(1, 2, 3, 4, 0));
    assert_emit("pixel=[1 2 3];pixel + 4", Node(1, 2, 3, 4, 0));
    assert_emit("pixel=[1 2 3];pixel<<4", Node(1, 2, 3, 4, 0));
    assert_emit("pixel=[1 2 3];4>>pixel", Node(4, 1, 2, 3, 0));
    assert_emit("pixel=[1 2 3];add(pixel, 4)", Node(1, 2, 3, 4, 0));// julia style
    assert_emit("pixel=[1 2 3];add 4 to pixel", Node(1, 2, 3, 4, 0));
    assert_emit("pixel=[1 2 3];pixel.add 4", Node(1, 2, 3, 4, 0));
    assert_emit("pixel=[1 2 3];pixel add 4", Node(1, 2, 3, 4, 0));
    assert_emit("pixel=[1 2 3];pixel.add(4)", Node(1, 2, 3, 4, 0));
    assert_emit("pixel=[1 2 3];pixel.insert 4", Node(1, 2, 3, 4, 0));
    assert_emit("pixel=[1 2 3];pixel insert 4", Node(1, 2, 3, 4, 0));
    assert_emit("pixel=[1 2 3];pixel.insert(4)", Node(1, 2, 3, 4, 0));
    assert_emit("pixel=[1 2 3];pixel.insert(4,-1)", Node(1, 2, 3, 4, 0));
    assert_emit("pixel=[1 2 3];pixel.insert 4 at end", Node(1, 2, 3, 4, 0));
    assert_emit("pixel=[1 2 3];pixel.insert 4 at -1", Node(1, 2, 3, 4, 0));
    assert_emit("pixel=[1 2 3];insert 4 at end of pixel", Node(1, 2, 3, 4, 0));
    assert_emit("pixel=[1 2 3];pixel.insert(4,0)", Node(4, 1, 2, 3, 0));
    assert_emit("pixel=[1 2 3];pixel.insert 4 at 0", Node(4, 1, 2, 3, 0));
    assert_emit("pixel=[1 2 3];pixel.insert 4 at start", Node(4, 1, 2, 3, 0));
    assert_emit("pixel=[1 2 3];pixel.insert 4 at head", Node(4, 1, 2, 3, 0));
    assert_emit("pixel=[1 2 3];pixel.insert 4 at beginning", Node(4, 1, 2, 3, 0));
    assert_emit("pixels=[1 2 3];insert 4 at start of pixels", Node(4, 1, 2, 3, 0));
    assert_emit("pixel=[1 2 3];pixel - [3]", Node(1, 2, 0));
    assert_emit("pixel=[1 2 3];pixel - 3", Node(1, 2, 0));
    assert_emit("pixel=[1 2 3];remove [3] from pixel", Node(1, 2, 0));
    assert_emit("pixel=[1 2 3];remove 3 from pixel", Node(1, 2, 0));
    assert_emit("pixel=[1 2 3];pixel.remove(3)", Node(1, 2, 0));
    assert_emit("pixel=[1 2 3];pixel.remove 3", Node(1, 2, 0));
    assert_emit("pixel=[1 2 3];pixel remove(3)", Node(1, 2, 0));
    assert_emit("pixel=[1 2 3];pixel remove 3", Node(1, 2, 0));
    assert_emit("pixel=[1 2 3];pixel.remove([3])", Node(1, 2, 0));
    assert_emit("pixel=[1 2 3];pixel.remove [3]", Node(1, 2, 0));
    assert_emit("pixel=[1 2 3];pixel remove([3])", Node(1, 2, 0));
    assert_emit("pixel=[1 2 3];pixel remove [3]", Node(1, 2, 0));
    assert_emit("pixel=[1 2 3 4];pixel.remove([3 4])", Node(1, 2, 0));
    assert_emit("pixel=[1 2 3 4];pixel.remove [3 4]", Node(1, 2, 0));
    assert_emit("pixel=[1 2 3 4];pixel remove([3 4])", Node(1, 2, 0));
    assert_emit("pixel=[1 2 3 4];pixel remove [3 4]", Node(1, 2, 0));
    assert_emit("pixel=[1 2 3 4];pixel remove 3 4", Node(1, 2, 0));
    assert_emit("pixel=[1 2 3 4];pixel remove (3 4)", Node(1, 2, 0));
    assert_emit("pixels=[1 2 3 4];pixels without (3 4)", Node(1, 2, 0));
}

void testArrayCreation() {
//    skip(
// todo create empty array
    assert_emit("pixel=[];pixel[1]=15;pixel[1]", 15);
    assert_emit("pixel=();pixel#1=15;pixel#1", 15);// diadic ternary operator
    assert_emit("pixel array;pixel#1=15;pixel#1", 15);
    assert_emit("pixel:int[100];pixel[1]=15;pixel[1]", 15);
    assert_emit("pixel=int[100];pixel[1]=15;pixel[1]", 15);// todo wasp can't distinguish type ':' from value '=' OK?
    assert_emit("pixel: 100 int;pixel[1]=15;pixel[1]", 15);// number times type = typed array
}

void testIndexOffset() {
    assert_emit("(2 4 3)[1]", 4);
    assert_emit("(2 4 3)#2", 4);
    assert_emit("y=(1 4 3)#2", 4);
    assert_emit("y=(1 4 3)[1]", 4);
    assert_is("x=(1 4 3);x#2=5;x#2", 5);
    assert_is("x=(1 4 3);z=(9 8 7);x#2", 4);
    assert_emit("x=(5 6 7);y=(1 4 3);y#2", 4);
    assert_emit("x=(5 6 7);(1 4 3)#2", 4);
    skip(
            assert_emit("y=(1 4 3);y[1]", 4);// CAN NOT WORK in data_mode because y[1] ≈ y:1 setter
            assert_emit("x=(5 6 7);y=(1 4 3);y[1]", 4);
    )
    assert_emit("(5 6 7);(2 4 3)[0]", 2);
    assert_emit("x=(5 6 7);y=(1 4 3);y#2", 4);
    assert_emit("(5 6 7);(1 4 3)#2", 4);
    assert_emit("x=(5 6 7);(1 4 3)#2", 4);
    skip(
            assert_emit("puts('ok');(1 4 3)#2", 4);
    )
    assert_emit("x=0;while x++<11: nop;", 0);
    assert_emit("i=10007;x=i%10000", 7);
    assert_emit("k=(1,2,3);i=1;k#i=4;k#1", 4)
    assert_emit("k=(1,2,3);i=1;k#i=4;k#1", 4)
    assert_emit("maxi=3840*2160", 3840 * 2160);
    assert_emit("i=10007;x=i%10000", 7);
    assert_is("x=(1 4 3);x#2=5;x#2", 5);
    assert_is("x=(1 4 3);x#2", 4);
}

void testFlagSafety() {
    auto code = "flags empty_flags{}; empty_flags mine = data_mode | space_brace;";
    assert_throws(code) // "data_mode not a member of empty_flags"s
    assert_throws("enum cant_combine{a;b}; a+b;");
    assert_throws("enum context_x{a;b};enum context_y{b;c};b;");
}


void testFlags2() {

    // todo allow just parser-flags{…} in wasp > wit
    auto code = R"(flags parser-flags{
        data_mode
        arrow
        space_brace
       }
       parser-flags my_flags = data_mode + space_brace
    )";
    assert_emit(code, 5)// 1+4
    clearAnalyzerContext();
    Node &parsed = parse(code, {.kebab_case=true});
    Node &node = analyze(parsed);
    check(types.has("parser-flags"))
    check(globals.has("data_mode"))
    check(globals.has("parser-flags.data_mode")) //
    Node &parserFlags = node.first();
    // todo AddressSanitizer:DEADLYSIGNAL why? lldb does'nt fail here
    check(parserFlags.name == "parser-flags")
    check(parserFlags.kind == flags)
    check(parserFlags.length == 3)
    check(parserFlags[1].name == "arrow")
    check(parserFlags[2].value.longy == 4)
    Node &instance = node.last();
    print(instance);
    check(instance.name == "my_flags")
    check(instance.type)
    check(instance.type->name == "parser-flags") // deduced!
    check(instance.kind == flags) // kind? not really type! todo?
    Node my_flags = instance.interpret();
    print(my_flags);
    check(my_flags.value.longy == 5) // 1+4 bit internal detail!
    skip(
            check(my_flags.values().serialize() == "data_mode + space_brace")
    )

//    check(node.last().serialize() == "ParserOptions my_flags = data_mode | space_brace") // todo canonical type serialization!?
}


void testFlags() {
    clearAnalyzerContext();
    Node &parsed = parse("flags abc{a b c}");
    backtrace_line();
    Node &node = analyze(parsed);
    check(node.name == "abc")
    check(node.kind == flags)
    check(node.length == 3);
    check(node[0].name == "a");
    check_is(typeName(node[0].kind), typeName(flag_entry));
    check_is(node[0].kind, flag_entry);
    check(node[0].kind == flag_entry);
    check(node[0].value.longy == 1);
    check(node[0].type);
    check(node[0].type == node);
    check(node[1].value.longy == 2);
    check(node[2].value.longy == 4);
}

void testPattern() {
    result = parse("y[1]", ParserOptions{.data_mode=false});
    check(result[0].kind == patterns);
    check(result[0][0].kind == longs);
    check(result[0][0].value.longy == 1);
//    assert_emit("(2 4 3)[0]", 2);
}

void testWitInterface() {
    Node &mod = Node("host-funcs").setType(modul).add(Node("current-user").setType(functor).add(StringType));
    assert_emit("interface host-funcs {current-user: func() -> string}", mod)
}

void testWitExport() {
    const char *code = "struct point{x:int y:float}";
    Node &node = parse(code);
    bindgen(node);
}


void testWitFunction() {
//    funcDeclaration
// a:b,c vs a:b, c:d

    assert_emit("add: func(a: float32, b: float32) -> float32", 0);
    Module &mod = read_wasm("test.wasm");
    print(mod.import_count);
    check_is(mod.import_count, 1)
    check_is(Node().setType(longs).serialize(), "0")
    check_is(mod.import_names, List<String>{"add"});// or export names?
}

void testWitImport() {

}

void testEqualsBinding() {
    // colon closes with space, not semicolon !
    parse("a = float32, b: float32");
    check(result.length == 1);
    check(result["a"] == "float32");
    Node val;
    val.add(Node("float32"));
    val.add(Node("b").add(Node("float32")));
    check_is(result[0], val);
}

void testColonImmediateBinding() {
    // colon closes with space, not semicolon !
    result = parse("a: float32, b: float32");
    check(result.length == 2);
    check(result["a"] == "float32");
    check(result[0] == Node("a").add(Node("float32")));
    check(result[1] == Node("b").add(Node("float32")));
}

void testWit() {
//    testWitFunction();
//    testWitInterface();
    Node wit;
    wit = (new WitReader())->read("test/merge/world.wit");
    wit = (new WitReader())->read("samples/bug.wit");
    wit = (new WitReader())->read("test/merge/example_dep/index.wit");
    wit = (new WitReader())->read("test/merge/index.wit");
    wit = (new WitReader())->read("samples/wit/typenames.wit");
    wit = (new WitReader())->read("samples/wit/wasi_unstable.wit");
//    check(wit.length > 0);
}

void testHyphenUnits() {
//     const char *code = "1900 - 2000 AD";// (easy with units)
//     assert_analyze(code,"{kind=range type=AD value=(1900,2000)}");
// todo how does Julia represent 10 ± 2 m/s ?
    assert_is("1900 - 2000 AD == 1950 AD ± 50", true);
    assert_is("1900 - 2000 cm == 1950 cm ± 50", true);
    assert_is("1900 - 2000 cm == 1950 ± 50 cm ", true);
}

void testHypenVersusMinus() {
    // Needs variable register in parser.
    const char *code = "a=-1 b=2 b-a";
    assert_emit(code, 3);
    // kebab case
    const char *data = "a-b:2 c-d:4 a-b";
    assert_emit(data, 2);
//    testHyphenUnits();

//    Node &node = parse(data);
}

void testKebabCase() {
    testHypenVersusMinus();
}


/*
0 0 1
64 40 2
8192 2000 3
1048576 100000 4
134217728 8000000 5
17179869184 400000000 6
2199023255552 20000000000 7
281474976710656 1000000000000 8
36028797018963968 80000000000000 9
0 0 1
-65 ffffffffffffffbf 2
-8193 ffffffffffffdfff 3
-1048577 ffffffffffefffff 4
-134217729 fffffffff7ffffff 5
-17179869185 fffffffbffffffff 6
-2199023255553 fffffdffffffffff 7
-281474976710657 fffeffffffffffff 8
-36028797018963969 ff7fffffffffffff 9
 */

// only test once, see lebByteSize for result
void testLebByteSize() {
    check_eq(lebByteSize((int64) -17179869185 + 1), 5)
    check_eq(lebByteSize((int64) -17179869185), 6)
    check_eq(lebByteSize((int64) -17179869185 - 1), 6)
    short last = 1;
    for (int64 i = -63; i > -0x100000000000000l; --i) {
//    for (int64 i = 0; i < 0x10000000000000l; ++i) {
//    for (uint64 i = 0; i < 0x100000000000000; ++i) {
        short size = lebByteSize(i);
        if (size > last) {
//            printf("%ld %lx %d\n", i, i, size);
            last = size;
            i = i * 128 + 129;
        }
    }
}

void testListGrow() {
// tested once, ok
    return;
    List<int> oh = {0, 1, 2, 3};
    for (int i = 4; i < 1000000000; ++i) {
        oh.add(i);
        unsigned int ix = random() % i;
        check_silent(oh[ix] == ix)
    }
    String aok = "ok";
    List<String> ja;// = {ok};
    ja.add(aok);
    String &o1 = ja[0];
    ja.grow();
    String &o3 = ja[0];
    check(o1.data == o3.data);
    o3.data = (char *) "hu";
    check(o1.data == o3.data);
}

void testWasmRunner() {
//	int result = run_wasm("test/test42.wasm");
//	assert_equals(result, 42);
}

void testLeaks() {
    int reruns = 0;
//	int reruns = 100000;
    for (int i = 0; i < reruns; ++i) {//
        printf("\n\n    ===========================================\n%d\n\n\n", i);
//		assert_emit("i=-9;√-i", 3);// SIGKILL after about 3000 emits OK'ish ;)
        assert_run("i=-9;√-i", 3);// SIGKILL after about 120 runs … can be optimized ;)
    }
}

void testWrong0Termination() {
#ifndef WASM
    List<String> builtin_constants = {"pi", "π"};
    assert_equals(builtin_constants.size(), 2);// todo
#endif
}

void testDeepColon() {
    result = parse("current-user: func() -> string");
    check_is(result.kind, key);
    check_is(result.values().name, "func");
    check_is(result.values().values().name, "string");
};

void testDeepColon2() {
    result = parse("a:b:c:d");
    check_is(result.kind, key);
    check_is(result.values().name, "b");
    check_is(result.values().values().values().name, "d");
};


void testStupidLongLong() {
//	int a;
//	long b;// 4 byte in wasm/windows grr
//	long long c;// 8 bytes everywhere (still not guaranteed grr)
//	int64 c;// 8 bytes everywhere (still not guaranteed grr)
    double b;
    float a;
    long double c;// float128 16 byte in wasm wow, don't use anyway;)
    print((int) sizeof(a));
    print((int) sizeof(b));
    print((int) sizeof(c));// what? 16 bytes!?
}

void testFloatReturnThroughMain() {
    double x = 0.0000001;// 3e...
//	double x=1000000000.1;// 4...
//	double x=-1000000000.1;// c1…
//	double x=9999999999999999.99999999;// 43…
//	double x=-9999999999999999.99999999;// c3…
//	double x=1.1;// 3ff199999999999a
//	double x=-1.1;// bff199999999999a
    int64 y = *(int64 *) &x;
#ifndef WASM
    printf("%llx\n", y);
#endif
    y = 0x00FF000000000000;// -> 0.000000 OK
    x = *(double *) &y;
    printf("%lf\n", x);
}

void testArrayS() {
    auto node = analyze(parse("int"));
//	assert_equals( node.type->kind, classe);
    assert_equals(node.kind, clazz);

    auto node2 = analyze(parse("ints"));
    assert_equals(node2.kind, arrays);// type: array<int>

    node = parse("ints x");
//	assert_equals( node.kind, reference);
//	assert_equals( node.kind, arrays);
    assert_equals(node.kind, groups);
    assert_equals(node.type, &DoubleType);
}

void testArrayInitialization() {// via Units
    assert_emit("x : int[100]; x.length", 100)
    assert_emit("x : u8 * 100; x.length", 100) // type times size operation!!
    assert_emit("x : 100 * int; x.length", 100)
    assert_emit("x : 100 * ints; x.length", 100)
    assert_emit("x : 100 ints; x.length", 100) // implicit multiplication, no special case!
    assert_emit("x : 100 int; x.length", 100)
    assert_emit("x : 100 integers; x.length", 100)
    assert_emit("x : 100 numbers; x.length", 100)
    assert_emit("x is 100 times [0]; x.length", 100)
    assert_emit("x is array of size 100; x.length", 100)
    assert_emit("x is an 100 integer array; x.length", 100)
    assert_emit("x is a 100 integer array; x.length", 100)
    assert_emit("x is a 100 element array; x.length", 100)
}

void testArrayInitializationBasics() {// via Units
    auto node = analyze(parse("x : 100 numbers"));
    assert_equals(node.kind, arrays);
    assert_equals(node.length, 100);
}


void testSinus2() {
    assert_emit(R"(
double sin(double x){
    x = modulo_double(x,tau)
    double z = x*x
    double w = z*z
    S1  = -1.66666666666666324348e-01,
    S2  =  8.33333333332248946124e-03,
    S3  = -1.98412698298579493134e-04,
    S4  =  2.75573137070700676789e-06,
    S5  = -2.50507602534068634195e-08,
    S6  =  1.58969099521155010221e-10
    if(x >= pi) return -sin(modulo_double(x,pi))
    double r = S2 + z*(S3 + z*S4) + z*w*(S5 + z*S6)
    return x + z*x*(S1 + z*r)
}; sin π/2 )", 1);// IT WORKS!!!
}

void testSinus() {
    //k=78; fucks it up!!
    assert_emit("double sin(double x){\n"
                "\tx = modulo_double(x,tau)\n"
                "\tdouble z = x*x\n"
                "\tdouble w = z*z\n"
                "\tS1  = -1.66666666666666324348e-01, /* 0xBFC55555, 0x55555549 */\n"
                "\tS2  =  8.33333333332248946124e-03, /* 0x3F811111, 0x1110F8A6 */\n"
                "\tS3  = -1.98412698298579493134e-04, /* 0xBF2A01A0, 0x19C161D5 */\n"
                "\tS4  =  2.75573137070700676789e-06, /* 0x3EC71DE3, 0x57B1FE7D */\n"
                "\tS5  = -2.50507602534068634195e-08, /* 0xBE5AE5E6, 0x8A2B9CEB */\n"
                "\tS6  =  1.58969099521155010221e-10  /* 0x3DE5D93A, 0x5ACFD57C */\n"
                //	            "\ttau =  6.283185307179586 // 2π\n"
                "\tif(x >= pi) return -sin(modulo_double(x,pi))\n"
                "\tdouble r = S2 + z*(S3 + z*S4) + z*w*(S5 + z*S6)\n"
                "\treturn x + z*x*(S1 + z*r)\n"
                "};sin π/2", 1.000000000252271);// IT WORKS!!! todo: why imprecision?
//    exit(1);
}

void test_sinus_wasp_import() {
    // using sin.wasp, not sin.wasm
    // todo: compile and reuse sin.wasm if unmodified
    assert_emit("use sin;sin π/2", 1);
    assert_emit("use sin;sin π", 0);
    assert_emit("use sin;sin 3*π/2", -1);
    assert_emit("use sin;sin 2π", 0);
    assert_emit("use sin;sin -π/2", -1);
}

void testIteration() {
    List<String> args;
    for (auto x: args)
        error("NO ITEM, should'nt be reached "s + x);

//#ifndef WASM
    List<String> list = {"1", "2", "3"};// wow! initializer_list now terminate!
//	List<String> list = {"1", "2", "3", 0};
    int i = 0;
    for (auto x: list) {
        i++;
        trace(x);
    }
    assert_equals(i, 3);

//    Node items = {"1", "2", "3"};
    Node items = Node{"1", "2", "3"};
    i = 0;
    for (auto x: list) {
        i++;
        trace(x);
    }
    assert_equals(i, 3);
//#endif
}

//void testLogarithmInRuntime(){
//	float ℯ = 2.7182818284590;
//	//	assert_equals(ln(0),-∞);
//	assert_equals(log(100000),5.);
//	assert_equals(log(10),1.);
//	assert_equals(log(1),0.);
//	assert_equals(ln(ℯ*ℯ),2.);
//	assert_equals(ln(1),0.);
//	assert_equals(ln(ℯ),1.);
//}


void testUpperLowerCase() {
//    assert_emit("lowerCaseUTF('ÂÊÎÔÛ')", "âêîôû")

    char string[] = "ABC";
    lowerCase(string, 0);
    assert_equals(string, "abc");
    skip(
            char string[] = "ÄÖÜ";
            lowerCase(string, 0);
            assert_equals(string, "äöü");
            char string[] = "ÂÊÎÔÛ ÁÉÍÓÚ ÀÈÌÒÙ AÖU";// String literals are read only!
            lowerCase(string, 0);
            assert_equals(string, "âêîôû áéíóú àèìòù aöu");
            char *string2 = (char *) u8"ÂÊÎÔÛ ÁÉÍÓÚ ÀÈÌÒÙ AÖU";
            lowerCase(string2, 0);
            assert_equals(string2, "âêîôû áéíóú àèìòù aöu");
            chars string3 = "ÂÊÎÔÛ ÁÉÍÓÚ ÀÈÌÒÙ AÖU";
    )
//	g_utf8_strup(string);
}

void testPrimitiveTypes() {
    assert_emit("double 2", 2);
    assert_emit("float 2", 2);
    assert_emit("int 2", 2);
    assert_emit("long 2", 2);
    assert_emit("8.33333333332248946124e-03", 0);
    assert_emit("8.33333333332248946124e+01", 83);
    assert_emit("S1  = -1.6666", -1);
    assert_emit("double S1  = -1.6666", -1);
//	assert_emit("double\n"
//	            "\tS1  = -1.6666", -1);

    assert_emit("grow(double z):=z*2;grow 5", 10);
    assert_emit("grow(z):=z*2;grow 5", 10);
    assert_emit("int grow(double z):=z*2;grow 5", 10);
    assert_emit("double grow(z):=z*2;grow 5", 10);
    assert_emit("int grow(int z):=z*2;grow 5", 10);
    assert_emit("double grow(int z):=z*2;grow 5", 10);
    assert_emit("double\n"
                "\tS1  = -1.66666666666666324348e01, /* 0xBFC55555, 0x55555549 */\n"
                "\tS2  =  8.33333333332248946124e03, /* 0x3F811111, 0x1110F8A6 */\n\nS1", -16);
    assert_emit("double\n"
                "\tS1  = -1.66666666666666324348e01, /* 0xBFC55555, 0x55555549 */\n"
                "\tS2  =  8.33333333332248946124e01, /* 0x3F811111, 0x1110F8A6 */\n\nS2", 83);
    assert_equals(ftoa(8.33333333332248946124e-03), "0.0083");
//	assert_equals(ftoa2(8.33333333332248946124e-03), "8.333E-3");
    assert_emit("S1 = -1.66666666666666324348e-01;S1*100", -16);
    assert_emit("S1 = 8.33333333332248946124e-03;S1*1000", 8);
    skip(
            assert_emit("(2,4) == (2,4)", 1);// todo: array creation/ comparison
            assert_emit("(float 2, int 4.3)  == 2,4", 1);//  PRECEDENCE needs to be in valueNode :(
            assert_emit("float 2, int 4.3  == 2,4", 1);//  PRECEDENCE needs to be in valueNode :(
    //	float  2, ( int ==( 4.3 2)), 4
    )
}

// One of the few tests which can be removed because who will ever change the sin routine?
//void test_sin() {
//	assert_equals(sin(0), 0);
//	assert_equals(sin(pi / 2), 1);
//	assert_equals(sin(-pi / 2), -1);
//	assert_equals(sin(pi), 0);
//	assert_equals(sin(2 * pi), 0);
//	assert_equals(sin(3 * pi / 2), -1);
//
//	assert_equals(cos(-pi / 2 + 0), 0);
//	assert_equals(cos(0), 1);
//	assert_equals(cos(-pi / 2 + pi), 0);
//	assert_equals(cos(-pi / 2 + 2 * pi), 0);
//	assert_equals(cos(pi), -1);
//	assert_equals(cos(-pi), -1);
//}


void testModulo() {
//	assert_equals(mod_d(10007.0, 10000.0), 7)
    assert_emit("10007%10000", 7);
    assert_emit("10007.0%10000", 7);
    assert_emit("10007.0%10000.0", 7);
    assert_emit("10007%10000.0", 7);
    assert_emit("i=10007;x=i%10000", 7);
    assert_emit("i=10007.0;x=i%10000.0", 7);
    assert_emit("i=10007.1;x=i%10000.1", 7);
}

void testRepresentations() {
    result = parse("a{x:1}");
    auto result2 = parse("a:{x:1}");
    assert_equals(result.kind, reference);
    assert_equals(result2.kind, key);
//	a{x:1} ==
}

void testDataMode() {
    result = parse("a b=c", ParserOptions{.data_mode=true});
    print(result);
    check(result.length == 2);// a, b:c

    result = parse("a b = c", ParserOptions{.data_mode=true});
//    check(result.length == 1);// (a b):c
    print(result);

    result = parse("a b=c", ParserOptions{.data_mode=false});
    print(result);
    check(result.length == 4);// a b = c

    result = analyze(result);
    print(result);
    check(result.length == 1);// todo  todo => (a b)=c => =( (a b) c)

    result = parse("<a href=link.html/>", ParserOptions{.data_mode=true, .use_tags=true});
    check(result.length == 1);// a(b=c)
}

void testSignificantWhitespace() {
    skip(testDataMode())
    result = parse("a b (c)");
    check(result.length == 3);
    result = parse("a b(c)");
    check(result.length == 2 or result.length == 1);
    result = parse("a b:c");
    check(result.length == 2);// a , b:c
    check(result.last().kind == key);// a , b:c
    result = parse("a: b c d", {.colon_immediate=false});
    check(result.length == 3);
    check(result.name == "a"); // "a"(b c d), NOT ((a:b) c d)
    check(result.kind == groups);// not key!
    result = parse("a b : c", {.colon_immediate=false});
    check(result.length == 1 or result.length == 2);// (a b):c
    assert_equals(result.kind, key);
    skip(
            assert(eval("1 + 1 == 2"));
            assert_emit("x=y=0;width=height=400;while y++<height and x++<width: nop;y", 400);

    )
    //1 + 1 ≠ 1 +1 == [1 1]
//	assert_is("1 +1", parse("[1 1]"));
    skip(
            assert(eval("1 +1 == [1 1]"));
            assert_is("1 +1", Node(1, 1, 0));
            assert_emit("1 +1 == [1 1]", 1);
            assert_emit("1 +1 ≠ 1 + 1", 1);
            assert(eval("1 +1 ≠ 1 + 1"));
    )
}


void testComments() {
    let c = "blah a b c # to silence python warnings;)\n y/* yeah! */=0 // really";
    result = parse(c);
    check(result.length == 2);
    check(result[0].length == 4);
    check(result[1].length == 3);
}

void testEmptyLineGrouping() {
    auto indented = R"(
a:
  b
  c

d
e
	)";
    auto groups = parse(indented);
    //	auto groups = parse("a:\n b\n c\n\nd\ne\n");
    check(groups.length == 3);// a(),d,e
    auto parsed = groups.first();
    check(parsed.length == 2);
    check(parsed[1] == "c");
    check(parsed.name == "a");
}


//[[maybe_used]]
[[nodiscard("replace generates a new string to be consumed!")]]
//__attribute__((__warn_unused_result__))
int testNodiscard() {
    return 54;
}


void testSerialize() {
    const char *inprint = "green=256*255";
    //	const char *inprint = "blue=255;green=256*255;maxi=3840*2160/2;init_graphics();surface=(1,2,3);i=10000;while(i<maxi){i++;surface#i=blue;};10";
    assertSerialize(inprint);
}


void testDedent2() {
    auto indented = R"(
a:
  b
  c
d
e
	)";
    auto groups = parse(indented);
    //	auto groups = parse("a:\n b\n c\nd\ne\n");
    print(groups.serialize());
    print(groups.length);
    check(groups.length == 3);// a(),d,e
    auto parsed = groups.first();
    check(parsed.name == "a");
    check(parsed.length == 2);
    print(parsed[1]);
    check(parsed[1].name == "c");
}

void testDedent() {
    auto indented = R"(
a
  b
  c
d
e
	)";
    auto groups = parse(indented);
    //	auto groups = parse("a:\n b\n c\nd\ne\n");
    print(groups.serialize());
    print(groups.length);
    check(groups.length == 3);// a(),d,e
    auto parsed = groups.first();
    check(parsed.name == "a");
    check(parsed.length == 2);
    print(parsed[1]);
    check(parsed[1].name == "c");
}

/*
void testWasmSpeed() {
	struct timeval stop, start;
	gettimeofday(&start, NULL);
	time_t s, e;
	time(&s);
	// todo: let compiler comprinte constant expressions like 1024*65536/4
	//out of bounds memory access if only one Memory page!
	//	assert_emit("i=0;k='hi';while(i<1024*65536/4){i++;k#i=65};k[1]", 65)// wow SLOOW!!!
	assert_emit("i=0;k='hi';while(i<16777216){i++;k#i=65};k[1]", 65)// still slow, but < 1s
	//	assert_emit("i=0;k='hi';while(i<16){i++;k#i=65};k[1]", 65)// still slow, but < 1s
	//	70 ms PURE C -O3   123 ms  PURE C -O1  475 ms in PURE C without optimization
	//  141 ms wasmtime very fast (similar to wasmer)
	//  150 ms wasmer very fast!
	//  546 ms in WebKit (todo: test V8/WebView2!)
	//	465 - 3511 ms in WASM3  VERY inconsistent, but ok, it's an interpreter!
	//	1687 ms wasmx (node.js)
	//  1000-3000 ms in wasm-micro-runtime :( MESSES with system clock! // wow, SLOWER HOW!?
	//	so we can never draw 4k by hand wow. but why? only GPU can do more than 20 frames per second
	//	sleep(1);
	gettimeofday(&stop, NULL);
	time(&e);

	printf("took %ld sec\n", e - s);
	printf("took %lu ms\n", ((stop.tv_sec - start.tv_sec) * 100000 + stop.tv_usec - start.tv_usec) / 100);

	exit(0);
}*/

void testImport42() {
    assert_is("import fourty_two", 42);
    assert_is("include fourty_two", 42);
    assert_is("require fourty_two", 42);
    assert_is("import fourty_two;ft*2", 42 * 2);
    assert_is("include fourty_two;ft*2", 42 * 2);
    assert_is("require fourty_two;ft*2", 42 * 2);
}
//
//void testWaspInitializationIntegrity() {
//	check(not contains(operator_list0, "‖"))// it's a grouper!
//}

void testColonLists() {
    auto parsed = parse("a: b c d", {.colon_immediate=false});
    check(parsed.length == 3);
    check(parsed[1] == "c");
    check(parsed.name == "a");
}


void testModernCpp() {
    auto aa = 1. * 2;
    printf("%f", aa);// lol
}

void testDeepCopyBug() {
    chars source = "{c:{d:123}}";
    assert_parses(source);
    check(result["d"] == 123);
}


void testDeepCopyDebugBugBug() {
    testDeepCopyBug();
    chars source = "{deep{a:3,b:4,c:{d:true}}}";
    assert_parses(source);
    check(result.name == "deep");
    result.print();
    Node &c = result["deep"]['c'];
    Node &node = c['d'];
    assert_equals(node.value.longy, (int64) 1);
    assert_equals(node, (int64) 1);
}

void testDeepCopyDebugBugBug2() {
//	chars source = "{deep{a:3,b:4,c:{d:123}}}";
    chars source = "{deep{c:{d:123}}}";
    assert_parses(source);
    Node &c = result["deep"]['c'];
    Node &node = c['d'];
    assert_equals(node.value.longy, (int64) 123);
    assert_equals(node, (int64) 123);
}


void testNetBase() {
    warn("NETBASE OFFLINE");
    if (1 > 0)return;
    chars url = "http://de.netbase.pannous.com:8080/json/verbose/2";
//	print(url);
    chars json = fetch(url);
//	print(json);
    result = parse(json);
    Node results = result["results"];
//	Node Erde = results[0];// todo : EEEEK, auto flatten can BACKFIRE! results=[{a b c}] results[0]={a b c}[0]=a !----
    Node Erde = results;
    assert(Erde.name == "Erde" or Erde["name"] == "Erde");
    Node &statements = Erde["statements"];
    assert(statements.length >= 1); // or statements.value.node->length >=
    assert(result["query"] == "2");
    assert(result["count"] == "1");
    assert(result["count"] == 1);

//	skip(
//			 );
    assert(Erde["name"] == "Erde");
//	assert(Erde.name == "Erde");
    assert(Erde["id"] == 2);// todo : auto numbers when?
    assert(Erde["kind"] == -104);
//	assert(Erde.id==2);
}

void testDivDeep() {
    Node div = parse("div{ span{ class:'bold' 'text'} br}");
    Node &node = div["span"];
    node.print();
    assert(div["span"].length == 2);
    assert(div["span"]["class"] == "bold");
}

void testDivMark() {
    use_polish_notation = true;
    Node div = parse("{div {span class:'bold' 'text'} {br}}");
    Node &span = div["span"];
    span.print();
    assert(span.length == 2);
    assert(span["class"] == "bold");
    use_polish_notation = false;
}

void testDiv() {
    result = parse("div{ class:'bold' 'text'}");
    result.print();
    assert(result.length == 2);
    assert(result["class"] == "bold");
    testDivDeep();
    skip(
            testDivMark();
    )
}

void checkNil() {
    check(NIL.isNil());
    assert_equals(NIL.name.data, nil_name);
    check(nil_name == "nil"s);// WASM
    if (NIL.name.data == nil_name)
        assert_equals(NIL.name, nil_name);
    check(NIL.name.data == nil_name);
    check(NIL.length == 0);
    check(NIL.children == 0);
    check(NIL.parent == 0);
    check(NIL.next == 0);
}

void testMarkAsMap() {
    Node compare = Node();
//	compare["d"] = Node();
    compare["b"] = 3;
    compare["a"] = "HIO";
    Node &dangling = compare["c"];
    check(dangling.isNil());
    checkNil();
    check(dangling == NIL);
    check(&dangling != &NIL);// not same pointer!
    dangling = Node(3);
//	dangling = 3;
    check(dangling == 3);
    check(compare["c"] == 3);
    assert_equals(compare["c"], Node(3));
    Node &node = compare["a"];
    assert(node == "HIO");
    chars source = "{b:3 a:'HIO' c:3}";// d:{}
    Node marked = parse(source);
    Node &node1 = marked["a"];
    assert(node1 == "HIO");
    check(compare["a"] == "HIO");
    check(marked["a"] == "HIO");
    assert(node1 == compare["a"]);
    assert(marked["a"] == compare["a"]);
    assert(marked["b"] == compare["b"]);
    assert(compare == marked);
}


void testMarkSimple() {
    print("testMarkSimple");
    char xx[] = "1";
    Node x = assert_parses(xx);
    Node a = assert_parses("{aa:3}");
    assert_equals(a.value.longy, (int64) 3);
    assert_equals(a, int64(3));
    assert(a == 3);
    assert(a.kind == longs or a.kind == key and a.value.node->kind == longs);
    assert(a.name == "aa");
//	assert(a3.name == "a"_s);// todo? cant


    Node &b = a["b"];
    a["b"] = a;
    assert(a["b"] == a);
    assert(a["b"] == b);
    assert(a["b"] == 3);

    assert(parse("3.") == 3.);
    assert(parse("3.") == 3.f);
//	assert(Mark::parse("3.1") == 3.1); // todo epsilon 1/3≠0.33…
//	assert(Mark::parse("3.1") == 3.1f);// todo epsilon
    result = parse("'hi'");
    check(result.kind == strings);
    check(*result.value.string == "hi");
    check(result == "hi");
    assert(parse("'hi'") == "hi");
    assert(parse("3") == 3);
}


// test only once to understand
void testUTFinCPP() {
    char32_t wc[] = U"zß水🍌"; // or
    printf("%s", (char *) wc);

//	char32_t wc2[] = "z\u00df\u6c34\U0001f34c";/* */ Initializing wide char array with non-wide string literal
    auto wc2 = "z\u00df\u6c34\U0001f34c";
    printf("%s", wc2);

//	auto wc3 = "z\udf\u6c34\U1f34c";// not ok in cpp

    // char = byte % 128   char<0 => utf or something;)
//	using namespace std;
#ifndef WASM
    const char8_t str[9] = u8"عربى";// wow, 9 bytes!
    printf("%s", (char *) str);
#endif
    const char str1[9] = "عربى";
    printf("%s", (char *) str1);
    check(eq((char *) str1, str1));
#ifndef WASM
#ifdef std
    std::string x = "0☺2√";
    // 2009 :  std::string is a complete joke if you're looking for Unicode support
    auto smile0 = x[1];
    char16_t smile1 = x[1];
    char32_t smile = x[1];
//	check(smile == smile1);
#endif
#endif
//	wstring_convert<codecvt_utf8<char32_t>, char32_t> wasm_condition;
//	auto str32 = wasm_condition.from_bytes(str);
    char16_t character = u'牛';
    char32_t hanzi = U'牛';
    wchar_t word = L'牛';
    printf("%c", character);
    printf("%c", hanzi);
    printf("%c", word);

//	for(auto c : str32)
//		cout << uint_least32_t(c) << '\n';
//		char a = '☹';// char (by definition) is one byte (WTF)
//		char[10] a='☹';// NOPE
    chars a = "☹"; // OK
    byte *b = (byte *) a;
    check_eq(a[0], (char) -30); // '\xe2'
    check_eq(a[1], (char) -104); // '\x98'
    check_eq(a[2], (char) -71); // '\xb9'
    check_eq(b[0], (byte) 226); // '\xe2'
    check_eq(b[1], (byte) 152); // '\x98'
    check_eq(b[2], (byte) 185); // '\xb9'
    check_eq(b[3], (byte) 0); // '\0'
}

void testUnicode_UTF16_UTF32() {// constructors/ conversion maybe later
//	char letter = '牛';// Character too large for enclosing character literal type char ≈ byte
    char16_t character = u'牛';
    char32_t hanzi = U'牛';
    wchar_t word = L'牛';
    check(hanzi == character);
    check(hanzi == word);
//	use_interpreter=true
// todo: let wasm return strings!
    assert(interpret("ç='a'") == String(u8'a'));
    assert(interpret("ç='☺'") == String(u'☺'));
    assert(interpret("ç='☺'") == String(L'☺'));
    assert(interpret("ç='☺'") == String(U'☺'));
//	skip(
    assert(interpret("ç='☺'") == String(u"☺"));
    assert(interpret("ç='☺'") == String(u8"☺"));
    assert(interpret("ç='☺'") == String(L"☺"));
    assert(interpret("ç='☺'") == String(U"☺"));
//	)
    check(String(u'牛') == "牛");
    check(String(L'牛') == "牛");
    check(String(U'牛') == "牛");

    check(String(L'牛') == u'牛');
    check(String(L'牛') == U'牛');
    check(String(L'牛') == L'牛');
    check(String(U'牛') == u'牛');
    check(String(U'牛') == U'牛');
    check(String(U'牛') == "牛");
    check(String(U'牛') == L'牛');
    check(String(u'牛') == u'牛');
    check(String(u'牛') == U'牛');
    check(String(u'牛') == L'牛');
    check(String(u'牛') == "牛");
    check(String("牛") == u'牛');
    check(String("牛") == U'牛');
    check(String("牛") == L'牛');
    check(String("牛") == "牛");
//	print(character);
//	print(hanzi);
//	print(word);
    print(sizeof(char32_t));// 32 lol
    print(sizeof(wchar_t));

    assert_parses("ç='☺'");
    assert(interpret("ç='☺'") == "☺");

    assert_parses("ç=☺");
    assert(result == "☺" or result.kind == expression);
}

void testStringReferenceReuse() {
    String x = "ab牛c";
    String x2 = String(x.data, false);
    check(x.data == x2.data);
    String x3 = x.substring(0, 2, true);
    check(x.data == x3.data);
    check(x.length >
          x3.length) // shared data but different length! check shared_reference when modifying it!! &text[1] doesn't work anyway;)
    check(x3 == "ab");
    print(x3);
    // todo("make sure all algorithms respect shared_reference and crucial length! especially print!");
}

//testUTFø  error: stray ‘\303’ in program
void testUTF() {
//    	testUTFinCPP();
    skip(testUnicode_UTF16_UTF32());
    check(utf8_byte_count(U'ç') == 2);
    check(utf8_byte_count(U'√') == 3);
    check(utf8_byte_count(U'🥲') == 4);
    check(is_operator(u'√'))// can't work because ☺==0xe2... too
    check(!is_operator(U'☺'))
    check(!is_operator(U'🥲'))
    check(not is_operator(U'ç'));
    check(is_operator(U'='));
//	check(x[1]=="牛");
    check("a牛c"s.codepointAt(1) == U'牛');
    String x = "a牛c";
    codepoint i = x.codepointAt(1);
    check("牛"s == i);
#ifndef WASM  // why??
    check("a牛c"s.codepointAt(1) == "牛"s);
    check(i == "牛"s);// owh wow it works reversed
#endif
    wchar_t word = L'牛';
    check(x.codepointAt(1) == word);

    assert_parses("{ç:☺}");
    assert(result["ç"] == "☺");

    assert_parses("ç:'☺'");
    skip(
            assert(result == "☺");
    )

    assert_parses("{ç:111}");
    assert(result["ç"] == 111);

    skip(
            assert_parses("ç='☺'");
            assert(eval("ç='☺'") == "☺");

            assert_parses("ç=☺");
            assert(result == "☺" or result.kind == expression);
    )
//	assert(node == "ø"); //=> OK
}


void testMarkMultiDeep() {
    // fragile:( problem :  c:{d:'hi'}} becomes c:'hi' because … bug
    chars source = "{deep{a:3,b:4,c:{d:'hi'}}}";
    assert_parses(source);
    Node &c = result["deep"]['c'];
    Node &node = result["deep"]['c']['d'];
    assert_equals(node, "hi");
    assert(node == "hi"_s);
    assert(node == "hi");
    assert(node == c['d']);
}

void testMarkMulti() {
    chars source = "{a:'HIO' b:3}";
    assert_parses(source);
    Node &node = result['b'];
    print(result['a']);
    print(result['b']);
    assert(result["b"] == 3);
    assert(result['b'] == node);
}

void testMarkMulti2() {
    assert_parses("a:'HIO' b:3  d:{}");
    assert(result["b"] == 3);
}

void testOverwrite() {
    chars source = "{a:'HIO' b:3}";
    assert_parses(source);
    result["b"] = 4;
    assert(result["b"] == 4);
    assert(result['b'] == 4);
}

void testAddField() {
//	chars source = "{}";
    result["e"] = 42;
    assert(result["e"] == 42);
    assert(result['e'] == 42);
}

void testErrors() {
    // use assert_throws
    assert_throws("0/0");
#if defined(WASI) || defined(WASM)
    skip("can't catch ERROR in wasm")
    return;
#endif
    throwing = false;
    result = parse("]");
    assert(result == ERROR);
/*
	ln -s /me/dev/apps/wasp/samples /me/dev/apps/wasp/cmake-build-wasm/out
	ln -s /Users/me/dev/apps/wasp/samples /Users/me/dev/apps/wasp/cmake-build-default/ #out/
  */
    breakpoint_helper
    result = /*Wasp::*/parseFile("samples/errors.wasp");
    throwing = true;
}


void testForEach() {
    int sum = 0;
    for (Node &item: parse("1 2 3"))
        sum += item.value.longy;
    assert(sum == 6);
}

#ifndef WASM
#ifndef WASI
#ifdef APPLE
#include <filesystem>
using files = std::filesystem::recursive_directory_iterator;

void testAllSamples() {
    // FILE NOT FOUND :
//	ln -s /me/dev/apps/wasp/samples /me/dev/apps/wasp/cmake-build-debug/
// ln -s /me/dev/apps/wasp/samples /me/dev/apps/wasp/cmake-build-wasm/
//	ln -s /me/dev/apps/wasp/samples /me/dev/apps/wasp/out/
// ln -s /me/dev/apps/wasp/samples /me/dev/apps/wasp/out/out wtf
    for (const auto &file : files("samples/")) {
        if (!String(file.path().string().data()).contains("error"))
            Mark::/*Wasp::*/parseFile(file.path().string().data());
    }
}
#endif
#endif
#endif

void testSample() {
    result = /*Wasp::*/parseFile("samples/comments.wasp");
}

void testNewlineLists() {
    result = parse("  c: \"commas optional\"\n d: \"semicolons optional\"\n e: \"trailing comments\"");
    assert(result['d'] == "semicolons optional");
}

void testKitchensink() {
    result = /*Wasp::*/parseFile("samples/kitchensink.wasp");
    result.print();
    assert(result['a'] == "classical json");
    assert(result['b'] == "quotes optional");
    assert(result['c'] == "commas optional");
    assert(result['d'] == "semicolons optional");
    assert(result['e'] == "trailing comments"); // trailing comments
    assert(result["f"] == /*inline comments*/ "inline comments");
    skip(
    )
}

void testEval3() {
    auto math = "one plus two";
    result = eval(math);
    assert(result == 3);
}


void testMathExtra() {
    assert_is("15÷5", 3);
    assert_emit("15÷5", 3);
    assert_emit("3⋅5", 15);
    assert_emit("3×5", 15);
    assert_emit("3^3", 27);
    assert_emit("3**3", 27);
    assert_emit("√3**2", 3);
    assert_emit("√3^2", 3);
    assert_is("one plus two times three", 7);
}

void testRoot() {
    skip(
            assert_is("40+√4", 42, 0)
            assert_is("√4", 2);
            assert_is("√4+40", 42);
            assert_is("40 + √4", 42);
    );// todo tokenized as +√
}

void testRootFloat() {
//	skip(  // include <cmath> causes problems, so skip
    assert_is("√42.0 * √42.0", 42.);
    assert_is("√42 * √42.0", 42.);
    assert_is("√42.0*√42", 42);
    assert_is("√42*√42", 42);// round AFTER! ok with f64! f32 result 41.99999 => 41
}


void testDeepLists() {
    assert_parses("{a:1 name:'ok' x:[1,2,3]}");
    assert(result.length == 3);
    assert(result["x"].length == 3);
    assert(result["x"][2] == 3);
}

void testIterate() {
//	parse("(1 2 3)");
    Node empty;
    bool nothing = true;
    for (Node &child: empty) {
        nothing = false;
        child = ERROR;
    }
    check(nothing);
    Node liste = parse("{1 2 3}");
    liste.print();
    for (Node &child: liste) {
        // SHOULD effect result
        child.value.longy = child.value.longy + 10;
    }
    check(liste[0].value.longy == 11)
    for (Node child: liste) {
        // should NOT affect result
        child.value.longy = child.value.longy + 1;
    }
    check(liste[0].value.longy == 11)
}

void testListInitializerList() {
    List<int> oks = {1, 2, 3}; // easy!
    check(oks.size_ == 3)
    check(oks[2] == 3)
}

void testListVarargs() {
    testListInitializerList();
    // ^^ OK just use List<int> oks = {1, 2, 3};
    skip(
            const List<int> &list1 = List<int>(1, 2, 3, 0);
            if (list1.size_ != 3)
                breakpoint_helper
                        check(list1.size_ == 3);
            check(list1[2] == 3);
    )
}


void testLists() {
    testListVarargs();//
    assert_parses("[1,2,3]");
    result.print();
    assert_equals(result.length, 3);
    assert_equals(result.kind, patterns);
    assert(result[2] == 3);
    assert(result[0] == 1);
    skip(
            assert(result[0] == "1");// autocast
    )
    List<int> a = {1, 2, 3};
    List<int> b{1, 2, 3};
    List<short> c{1, 2, 3};
    List<short> d = {1, 2, 3};
    check_eq(a.size_, 3);
    check_eq(b.size_, 3);
    check_eq(a.size_, b.size_);
    check_eq(a[0], b[0]);
    check_eq(a[2], b[2]);
    check_eq(a, b);
//    check_eq(a, c); // not comparable
    check_eq(c, d);
//List<double> c{1, 2, 3};
//List<float> d={1, 2, 3};

//	assert_is("[1,2,3]",1);
}

void testMapsAsLists() {
    assert_parses("{1,2,3}");
    assert_parses("{'a'\n'b'\n'c'}");
    assert_parses("{add x y}");// expression?
    assert_parses("{'a' 'b' 'c'}");// expression?
    assert_parses("{'a','b','c'}");// list
    assert_parses("{'a';'b';'c'}");// list
    assert(result.length == 3);
    assert(result[1] == "b");
//	assert_is("[1,2,3]",1); what?
}


void testLogic() {
    assert_is("true or false", true);
    assert_is("false or true", true);

    assert_is("not true", false);
    assert_is("not false", true); // fourth test fails regardles of complexity?

    assert_is("false or false", false);
    assert_is("true or false", true);
    assert_is("true or true", true);

    assert_is("true and true", true);
    assert_is("true and false", false);
    assert_is("false and true", false);
    assert_is("false and false", false);

    assert_is("false xor true", true);
    assert_is("true xor false", true);
    assert_is("false xor false", false);
    assert_is("true xor true", false);

    assert_is("¬ 1", 0);
    assert_is("¬ 0", 1);

    assert_is("0 ⋁ 0", 0);
    assert_is("0 ⋁ 1", 1);
    assert_is("1 ⋁ 0", 1);
    assert_is("1 ⋁ 1", 1);

    assert_is("0 ⊻ 0", 0);
    assert_is("0 ⊻ 1", 1);
    assert_is("1 ⊻ 0", 1);
    assert_is("1 ⊻ 1", 0);

    assert_is("1 ∧ 1", 1);
    assert_is("1 ∧ 0", 0);
    assert_is("0 ∧ 1", 0);
    assert_is("0 ∧ 0", 0);
}

// use the bool() function to determine if a value is truthy or falsy.
void testTruthiness() {
    result = parse("true");
//	print("TRUE:");
    nl();
    print(result.name);
    nl();
    print(result.value.longy);
    check(True.kind == bools);
    check(True.name == "True");
    check(True.value.longy == 1);
    assert_is("false", false);
    assert_is("true", true);
//	check(True.value.longy == true);
//	check(True.name == "true");
//	check(True == true);
    assert_is("False", false);
    assert_is("True", true);
    assert_is("False", False);
    assert_is("True", True);
    assert_is("false", False);
    assert_is("true", True);
    assert_is("0", False);
    assert_is("1", True);
    skip(
            assert_is("ø", NIL);
    )
    assert_is("nil", NIL);
    assert_is("nil", False);
    assert_is("nil", false);
    assert_is("ø", false);
    skip(
            assert_is("2", true);  // Truthiness != equality with 'true' !
            assert_is("2", True); // Truthiness != equality with 'True' !
            assert_is("{x:0}", true); // wow! falsey so deep?
            assert_is("[0]", true);  // wow! falsey so deep?
    )
    assert_is("1", true);


    assert_is("{1}", true);
    skip(
            assert_is("{x:1}", true);
    )

    todo_emit( // UNKNOWN local symbol ‘x’ in context main OK
            assert_is("x", false);
            assert_is("{x}", false);
            assert_is("cat{}", false);
    )

    // empty referenceIndices are falsey! OK
}

void testLogicEmptySet() {
    if (eval_via_emit) {
        print("todo eval_via_emit testLogicEmptySet …");// todo
        return;
    }
    assert_is("not ()", true); // missing args for operator not
    assert_is("() xor 1", true);
    assert_is("1 xor ()", true);
    assert_is("() xor ()", false);
    assert_is("1 xor 1", false);
    assert_is("() or 1", true);
    assert_is("() or ()", false);
    assert_is("1 or ()", true);
    assert_is("1 or 1", true);

    assert_is("1 and 1", true);
    assert_is("1 and ()", false);
    assert_is("() and 1", false);
    assert_is("() and ()", false);

    assert_is("not 1", false);
    assert_is("{} xor 1", true);
    assert_is("1 xor {}", true);
    assert_is("{} xor {}", false);
    assert_is("1 xor 1", false);
    assert_is("{} or 1", true);
    assert_is("{} or {}", false);
    assert_is("1 or {}", true);
    assert_is("1 or 1", true);

    assert_is("1 and 1", true);
    assert_is("1 and {}", false);
    assert_is("{} and 1", false);
    assert_is("{} and {}", false);

    assert_is("not {}", true);
    assert_is("not 1", false);

    assert_is("[] or 1", true);
    assert_is("[] or []", false);
    assert_is("1 or []", true);
    assert_is("1 or 1", true);

    assert_is("1 and 0", false);
    assert_is("1 and []", false);
    assert_is("[] and 1", false);
    assert_is("[] and []", false);

    assert_is("not []", true);
    assert_is("not 1", false);


    assert_is("[] xor 1", true);
    assert_is("1 xor []", true);
    assert_is("[] xor []", false);
    assert_is("1 xor 1", false);
}


void testLogicOperators() {
    assert_is("¬ 0", 1);
    assert_is("¬ 1", 0);

    assert_is("0 ⋁ 0", 0);
    assert_is("0 ⋁ 1", 1);
    assert_is("1 ⋁ 0", 1);
    assert_is("1 ⋁ 1", 1);

    assert_is("0 ⊻ 0", 0);
    assert_is("0 ⊻ 1", 1);
    assert_is("1 ⊻ 0", 1);
    assert_is("1 ⊻ 1", 0);

    assert_is("1 ∧ 1", 1);
    assert_is("1 ∧ 0", 0);
    assert_is("0 ∧ 1", 0);
    assert_is("0 ∧ 0", 0);
}


void testLogic01() {
    assert_is("0 or 0", false);
    assert_is("0 or 1", true);
    assert_is("1 or 0", true);
    assert_is("1 or 1", true);

    assert_is("1 and 1", true);
    assert_is("1 and 0", false);
    assert_is("0 and 1", false);
    assert_is("0 and 0", false);

    // eor either or
    assert_is("0 xor 0", false);
    assert_is("0 xor 1", true);
    assert_is("1 xor 0", true);
    assert_is("1 xor 1", false);

    assert_is("not 0", true);
    assert_is("not 1", false);
}

void testEqualities() {
    assert_is("1≠2", True);
    assert_is("1==2", False);
    //	assert_is("1=2", False);
    assert_is("1!=2", True);
    assert_is("1≠1", False);
//	assert_is("2=2", True);
    assert_is("2==2", True);
    assert_is("2!=2", False);
}

// test once: not a test, just documentation
void testBitField() {
    union mystruct {// bit fields
        struct {
            short Reserved1: 3;
            short WordErr: 1;
            short SyncErr: 1;
            short WordCntErr: 1;
//            short Reserved2: 10;
        };
        short word_field;
    };
    check_eq(sizeof(mystruct), 2 /*bytes */);
    mystruct x;
    x.WordErr = true;
    check_eq(x.word_field, 8);// 2^^3
}

void testCpp() {
//    testBitField();
//	esult of comparison of constant 3 with expression of type 'bool' is always true
//	assert(1 < 2 < 3);// NOT WHAT YOU EXPECT!
//	assert(3 > 2 > 1);// NOT WHAT YOU EXPECT!
//	assert('a' < 'b' < 'c');// NOT WHAT YOU EXPECT!
//	assert('a' < b and b < 'c');// ONLY WAY <<
}

void testGraphSimple() {
    assert_parses("{  me {    name  } # Queries can have comments!\n}");
    assert(result.children[0].name == "name");// result IS me !!
    assert(result["me"].children[0].name == "name");// me.me = me good idea?
}


void testGraphQlQueryBug() {
    auto graphResult = "{friends: [ {name:x}, {name:y}]}";
    assert_parses(graphResult);
    Node &friends = result["friends"];
    assert(friends[0]["name"] == "x");
}

void testGraphQlQuery() {
    auto graphResult = "{\n  \"data\": {\n"
                       "    \"hero\": {\n"
                       "      \"id\": \"R2-D2\",\n"
                       "      \"height\": 5.6430448,\n"
                       "      \"friends\": [\n"
                       "        {\n"
                       "          \"name\": \"Luke Skywalker\"\n"
                       "        },\n"
                       "        {\n"
                       "          \"name\": \"Han Solo\"\n"
                       "        },\n"
                       "      ]" /* todo \n nextNonWhite */
                       "    }\n"
                       "  }\n"
                       "}";
    assert_parses(graphResult);
    result.print();
    Node &data = result["data"];
    data.print();
    Node &hero = data["hero"];
    hero.print();
    Node &height = data["hero"]["height"];
    height.print();
    Node &id = hero["id"];
    id.print();
    assert(id == "R2-D2");
    assert(height == 5.6430448);
//	assert(height==5.643);
    Node &friends = result["data"]["hero"]["friends"];
    assert(friends[0]["name"] == "Luke Skywalker");
//todo	assert(result["hero"] == result["data"]["hero"]);
//	assert(result["hero"]["friends"][0]["name"] == "Luke Skywalker")// if 1-child, treat as root
}

void testGraphQlQuery2() {
    assert_parses("{\n"
                  "  human(id: \"1000\"){\n"
                  "    name\n"
                  "    height(unit: FOOT)\n"
                  "  }\n"
                  "}");
    assert(result["human"]["id"] == 1000);
    skip(assert(result["id"] == 1000, 0));// if length==1 descend!
}

void testGraphQlQuerySignificantWhitespace() {
    // human() {} != human(){}
    assert_parses("{\n"
                  "  human(id: \"1000\") {\n"
                  "    name\n"
                  "    height(unit: FOOT)\n"
                  "  }\n"
                  "}");
    assert(result["human"]["id"] == 1000);
    skip(assert(result["id"] == 1000, 0));// if length==1 descend!
}

void testGraphParams() {
    assert_parses("{\n  empireHero: hero(episode: EMPIRE){\n    name\n  }\n"
                  "  jediHero: hero(episode: JEDI){\n    name\n  }\n}");
    Node &hero = result["empireHero"];
    hero.print();
    assert(hero["episode"] == "EMPIRE");
    assert_parses("\nfragment comparisonFields on Character{\n"
                  "  name\n  appearsIn\n  friends{\n    name\n  }\n }");
    assert_parses("\nfragment comparisonFields on Character{\n  name\n  appearsIn\n  friends{\n    name\n  }\n}")
// VARIAblE: { "episode": "JEDI" }
    assert_parses("query HeroNameAndFriends($episode: Episode){\n"
                  "  hero(episode: $episode){\n"
                  "    name\n"
                  "    friends{\n"
                  "      name\n"
                  "    }\n"
                  "  }\n"
                  "}")
}

void testRootLists() {
    // vargs needs to be 0 terminated, otherwise pray!
    assert_is("1 2 3", Node(1, 2, 3, 0))
    assert_is("(1 2 3)", Node(1, 2, 3, 0))
    assert_is("(1,2,3)", Node(1, 2, 3, 0))
    assert_is("(1;2;3)", Node(1, 2, 3, 0))
    assert_is("1;2;3", Node(1, 2, 3, 0, 0))//ok
    assert_is("1,2,3", Node(1, 2, 3, 0))
    assert_is("[1 2 3]", Node(1, 2, 3, 0).setType(patterns))
    assert_is("[1 2 3]", Node(1, 2, 3, 0))
    assert_is("[1,2,3]", Node(1, 2, 3, 0))
    assert_is("[1,2,3]", Node(1, 2, 3, 0).setType(patterns));
    assert_is("[1;2;3]", Node(1, 2, 3, 0))
    todo_emit( // todo ?
            assert_is("{1 2 3}", Node(1, 2, 3, 0))
            assert_is("{1,2,3}", Node(1, 2, 3, 0))
            assert_is("{1;2;3}", Node(1, 2, 3, 0))
    )
    todo_emit( // todo symbolic wasm
            assert_is("(a,b,c)", Node("a", "b", "c", 0))
            assert_is("(a;b;c)", Node("a", "b", "c", 0))
            assert_is("a;b;c", Node("a", "b", "c", 0))
            assert_is("a,b,c", Node("a", "b", "c", 0))
            assert_is("{a b c}", Node("a", "b", "c", 0))
            assert_is("{a,b,c}", Node("a", "b", "c", 0))
            assert_is("[a,b,c]", Node("a", "b", "c", 0))
            assert_is("(a b c)", Node("a", "b", "c", 0))
            assert_is("[a;b;c]", Node("a", "b", "c", 0))
            assert_is("a b c", Node("a", "b", "c", 0, 0))
            assert_is("{a;b;c}", Node("a", "b", "c", 0))
            assert_is("[a b c]", Node("a", "b", "c", 0))
    )
}


void testRoots() {
    check(NIL.value.longy == 0);
    assert_is((char *) "'hello'", "hello");
    skip(assert_is("hello", "hello", 0));// todo reference==string really?
    assert_is("True", True)
    assert_is("False", False)
    assert_is("true", True)
    assert_is("false", False)
    assert_is("yes", True)
    assert_is("no", False)
//	assert_is("right", True)
//	assert_is("wrong", False)
    assert_is("null", NIL);
    assert_is("", NIL);
    check(NIL.value.longy == 0);
    assert_is("0", NIL);
    assert_is("1", 1)
    assert_is("123", 123)
    skip(
            assert_is("()", NIL);
            assert_is("{}", NIL);// NOP
    )
}


void testParams() {
//	assert_equals(parse("f(x)=x*x").param->first(),"x");
//    data_mode = true; // todo ?
    Node body = assert_parses("body(style='blue'){a(link)}");
    assert(body["style"] == "blue");

    parse("a(x:1)");
    assert_parses("a(x:1)");
    assert_parses("a(x=1)");
    assert_parses("a{y=1}");
    assert_parses("a(x=1){y=1}");
    skip(assert_parses("a(1){1}", 0));
    skip(assert_parses("multi_body{1}{1}{1}", 0));// why not generalize from the start?
    skip(assert_parses("chained_ops(1)(1)(1)", 0));// why not generalize from the start?

    assert_parses("while(x<3){y:z}");
    Node body2 = assert_parses("body(style='blue'){style:green}");// is that whole xml compatibility a good idea?
    skip(assert(body2["style"] ==
                "green", 0));// body has prescedence over param, semantically param provide extra data to body
    assert(body2[".style"] == "blue");
//	assert_parses("a(href='#'){'a link'}");
//	assert_parses("(markdown link)[www]");
}


void testDidYouMeanAlias() {
    skip(
            Node ok1 = assert_parses("printf('hi')");
            assert_equals(ok1[".warnings"], "DYM print");// THIS CAN NEVER HAVED WORKED! BUG IN TEST PIPELINE!
    )
}

void testEmpty() {
    result = assert_parsesx("{  }");
    assert_equals_x(result.length, 0);
}

void testEval() {
    skip(
            assert_is("√4", 2);
    )
}

void testLengthOperator() {
    todo_emit(assert_is("#{a b c}", 3);)
    skip(assert_is("#(a b c)", 3, 0));// todo: groups
}

void testNodeName() {
    Node a = Node("xor");// NOT type string by default!
    bool ok1 = a == "xor";
    check(a == "xor")
    check(a.name == "xor")
    check(ok1)
}

void testIndentAsBlock() {
    todo_emit(
            assert_is((char *) "a\n\tb", "a{b}")
    )
// 0x0E 	SO 	␎ 	^N 		Shift Out
// 0x0F 	SI 	␏ 	^O 		Shift In
    //	indent/dedent  0xF03B looks like pause!?   0xF032…  it does, what's going on CLion? Using STSong!
    //	https://fontawesome.com/v4.7/icon/outdent looks more like it, also matching context  OK in font PingFang HK?
}// 􀖯􀉶𠿜🕻🗠🂿	𝄉

void testParentContext() {
    chars source = "{a:'HIO' d:{} b:3 c:ø}";
    assert_parses(source);
    result.print();
    Node &a = result["a"];
    a.print();
    assert_equals(a.kind, strings);
    assert_equals(a.value.string, "HIO");
    assert_equals(a.string(), "HIO");// keyNodes go to values!
    assert(a == "HIO");
//	assert_equals(a.name,"a" or"HIO");// keyNodes go to values!
    skip(
            assert_equals(a.kind, key);
            assert(a.name == "HIO");
    )
}

void testParent() {
//	chars source = "{a:'HIO' d:{} b:3 c:ø}";
    chars source = "{a:'HIO'}";
    assert_parses(source);
    Node &a = result["a"];
    print(a);
    check(a.kind == key or a.kind == strings);
    check(a == "HIO");
    check(a.parent == 0);// key is the highest level
    Node *parent = a.value.node->parent;
    check(parent);
    print(parent);// BROKEN, WHY?? let's find out:
    check(*parent == result);
    skip(
    // pointer identity broken by flat() ?
            check(parent == &result);
    )
    testParentContext();// make sure parsed correctly
}

void testAsserts() {
    assert_equals(11, 11);
    assert_equals(11.1f, 11.1f);
//	assert_equals(11.1l, 11.1);
    assert_equals((float) 11., (float) 11.);
//	assert_equals((double)11., (double )11.);
    assert_equals("a", "a");
    assert_equals("a"_s, "a"_s);
    assert_equals("a"_s, "a");
    assert_equals(Node("a"), Node("a"));
    assert_equals(Node(1), Node(1));
}

void testStringConcatenation() {
//	assert_equals(Node("✔️"), True);
//	assert_equals(Node("✔"), True);
//	assert_equals(Node("✖️"), False);
//	assert_equals(Node("✖"), False);
    String huh = "a"_s + 2;
    check_eq(huh.length, 2);
    check_eq(huh[0], 'a');
    check_eq(huh[1], '2');
    check_eq(huh[2], (int64) 0);
    check(eq("a2", "a2"));
    check(eq("a2", "a2", 3));

    assert_equals(huh, "a2");
    assert_equals("a"_s + 2, "a2");
    assert_equals("a"_s + 2.2, "a2.2");
    assert_equals("a"_s + "2.2", "a2.2");
    assert_equals("a"_s + 'b', "ab");
    assert_equals("a"_s + "bc", "abc");
    assert_equals("a"_s + true, "a✔️"_s);
    assert_equals("a%sb"_s % "hi", "ahib");

    assert_equals("a%db"_s % 123, "a123b");
    assert_equals("a%s%db"_s % "hi" % 123, "ahi123b");
}

void testString() {
    String *a = new String("abc");
    String b = String("abc");
    String c = *a;
    print(a);
    print(b);
    print(c);
    printf("...");
//    for (int i = 0; i < 1000; ++i) {
//        puti(i);
//        puts("… x y z");
//        newline();
//            check(c == "abc");
//
//        if (b == "abc");
//        else check(b == "abc");
//    }
//    printf("DONE ...");
//    exit(1);
    check_is(a, b);
    check_is(a, c);
    check_is(b, c);
    let d = "abc";
    print(a->data);
    print(d);
    check(eq(a->data, d));
    check_is(b, "abc");
    check_is(c, "abc");
    check_eq(b, "abc");
    check_eq(c, "abc");
    check(c == "abc");
    check(b == a);
    check(b == c);
    check("%d"s % 5 == "5");
    check("%s"s % "a" == "a");
    check("%s"s % "ja" == "ja");
    check("hi %s ok"s.replace("%s", "ja") == "hi ja ok");
    check("1234%d6789"s % 5 == "123456789");
    check("char %c"s % 'a' == "char a");
    check("%c %d"s % 'a' % 3 == "a 3");
    check("abc"s.replace("a", "d") == "dbc");
    check("hi %s ok"s % "ja" == "hi ja ok");
    check("%s %d"s % "hu" % 3 == "hu 3");
    check("%s %s %d"s % "ha" % "hu" % 3 == "ha hu 3");
    check_is("%c"s % u'γ', "γ");
    check_is("%C"s % U'γ', "γ");
    assert_equals(String("abcd").substring(1, 2, false), "b");
    assert_equals(String("abcd").substring(1, 3, false), "bc");
    assert_equals(String("abcd").substring(1, 2, true/*share*/), "b");// excluding, like js
    assert_equals(String("abcd").substring(1, 3, true), "bc");
    assert_equals(String("abcd").substring(1, 3), "bc");
    assert_equals(String("abcd").substring(1, 2), "b");
    check("%s"s.replace("%s", "ja") == "ja");
    check("hi %s"s.replace("%s", "ja") == "hi ja");
    check("%s ok"s.replace("%s", "ja") == "ja ok");
    check("hi %s ok"s.replace("%s", "ja") == "hi ja ok");
    auto x = "hi %s ok"s % "ja";
    check(x);
    printf("%s", x.data);
    check(x == "hi ja ok");
    check("hi %s ok"s % "ja" == "hi ja ok");
    assert_equals(atoi1('x'), -1);
    assert_equals(atoi1('3'), 3);
    assert_equals(parseLong("١٢٣"), 123l);
    check_eq(parseLong("123"), 123l);// can crash!?!
//	assert_equals( atoi1(u'₃'),3);// op
    assert_equals(parseLong("0"), 0l);
    assert_equals(parseLong("x"), 0l);// todo side channel?
    assert_equals(parseLong("3"), 3l);
    check_eq(" a b c  \n"s.trim(), "a b c");
    assert_equals("     \n   malloc"s.trim(), "malloc");
    assert_equals("     \n   malloc     \n   "s.trim(), "malloc");
    assert_equals("malloc     \n   "s.trim(), "malloc");
    testStringConcatenation();
    testStringReferenceReuse();
    assert_equals_x(parse("١٢٣"), Node(123));
//    assert_is("١٢٣", 123);
    check("abc"_ == "abc");

    check(String(u'☺').length == 3)
    check(String(L'☺').length == 3)
    check(String(U'☺').length == 3)

    auto node1 = interpret("ç='☺'");
    check(node1.kind == strings);
    check(*node1.value.string == u'☺');
    check(*node1.value.string == u'☺');
    assert(node1 == String(u'☺'));
    assert(node1 == String(L'☺'));
    assert(node1 == String(U'☺'));
}


void testNilValues() {
    assert(NIL.name == nil_name);
    assert(NIL.isNil());
    assert_parses("{ç:null}");
    Node &node1 = result["ç"];
    debugNode(node1);
    assert(node1 == NIL);

    assert_parses("{a:null}");
    check(result["a"].value.data == 0)
    check(result.value.data == 0)
    check(result["a"].value.longy == 0)
    check(result.value.longy == 0)
    debugNode(result["a"]);
    print(result["a"].serialize());
    assert(result["a"] == NIL);
    assert(result == NIL);
    assert_equals(result["a"], NIL);

    assert_parses("{ç:ø}");
    Node &node = result["ç"];
    assert(node == NIL);
}


void testConcatenationBorderCases() {
    assert_equals(Node(1, 0) + Node(3, 0), Node(1, 3, 0));// ok
    assert_equals(Node("1", 0, 0) + Node("2", 0, 0), Node("1", "2", 0));
// Border cases: {1}==1;
    assert_equals(parse("{1}"), parse("1"));
// Todo Edge case a=[] a+=1
    assert_equals(Node() + Node("1", 0, 0), Node("1", 0, 0));
    //  singleton {1}+2==1+2 = 12/3 should be {1,2}
    assert_equals(Node("1", 0, 0) + Node("x"_s), Node("1", "x", 0));
}

void testConcatenation() {
    Node node1 = Node("1", "2", "3", 0);
    check(node1.length == 3);
    check(node1.last() == "3");
    check(node1.kind == objects);
    Node other = Node("4").setType(strings);// necessary: Node("x") == reference|strings? => kind=unknown
    check(other.kind == strings);
    check(!other.isNil());
    check(!(&other == &NIL));
    //	address of 'NIL' will always evaluate to 'true' because NIL is const now!
//	check(!(other == &NIL));
//	check(not(&other == &NIL));
//	check(not(other == &NIL));
    check(other != NIL);
#ifndef WASM
    //	check(other != &NIL);
#endif
    check(&other != &NIL);


    check(not other.isNil());
    Node node1234 = node1.merge(other);
//	Node node1234=node1.merge(Node("4"));
//	Node node1234=node1.merge(new Node("4"));
    Node *four = new Node("4");
    node1.add(four);
//	node1=node1 + Node("4");
    check_eq(node1.length, 4);
    check(node1.last() == "4");
//	check(&node1234.last() == four); not true, copied!
    check(node1234.last() == four);
    check(*four == "4");
    node1234.print();

    check_eq(node1234.length, 4);

    node1234.children[node1234.length - 2].print();
    node1234.children[node1234.length - 1].print();
    node1234.last().print();
    check(node1234.last() == "4");

    assert_equals(node1, Node("1", "2", "3", "4", 0));
    Node first = Node(1, 2, 0);
    check_eq(first.length, 2);
    check_eq(first.kind, objects);
    result = first + Node(3);
    check_eq(result.length, 3);
    check(result.last() == 3);

    assert_equals(Node(1, 2, 0) + Node(3), Node(1, 2, 3, 0));
    assert_equals(Node(1, 2, 0) + Node(3, 4, 0), Node(1, 2, 3, 4, 0));
    assert_equals(Node("1", "2", 0) + Node("3", "4", 0), Node("1", "2", "3", "4", 0));
    assert_equals(Node(1) + Node(2), Node(3));
    assert_equals(Node(1) + Node(2.4), Node(3.4));
    assert_equals(Node(1.0) + Node(2), Node(3.0));

    skip(
            assert_equals(Node(1) + Node("a"_s), Node("1a"));
            Node bug = Node("1"_s) + Node(2);
            // AMBIGUOUS: "1" + 2 == ["1" 2] ?
            assert_equals(Node("1"_s) + Node(2), Node("12"));
            assert_equals(Node("a"_s) + Node(2.2), Node("a2.2"));
            // "3" is type unknown => it is treated as NIL and not added!
            assert_equals(Node("1", "2", 0) + Node("3"), Node("1", "2", "3", 0));// can't work ^^
    )
}


void testParamizedKeys() {
//	<label for="pwd">Password</label>

// 0. parameters accessible
    Node label0 = parse("label(for:password)");
    label0.print();
    Node &node = label0["for"];
    assert_equals(node, "password");
    assert_equals(label0["for"], "password");

    // 1. paramize keys: label{param=(for:password)}:"Text"
    Node label1 = parse("label(for:password):'Passwort'");
    label1.print();
    assert_equals(label1, "Passwort");
    assert_equals(label1["for"], "password");
//	assert_equals(label1["for:password"],"Passwort");

// 2. paramize values
// TODO 1. move params of Passwort up to lable   OR 2. preserve Passwort as object in stead of making it string value of label!
    skip(
            Node label2 = parse("label:'Passwort'(for:password)");
            check(label2 == "Passwort");
            assert_equals(label2, "Passwort");
            assert_equals(label2["for"], "password");
            assert_equals(label2["for"], "password");// descend value??
            assert_equals(label2["Passwort"]["for"], "password");
    )

    skip(
//	3. relative equivalence? todo not really
            assert_equals(label1, label2);
            Node label3 = parse("label:{for:password 'Password'}");
    )
}

void testStackedLambdas() {
    result = parse("a{x:1}{y:2}{3}");
    result.print();
    check(result.length == 3);
    check(result[0] == parse("{x:1}"));
    check(result[0] == parse("x:1"));// grouping irrelevant
    check(result[1] == parse("{y:2}"));
    check(result[2] == parse("{3}"));
    check(result[2] != parse("{4}"));

    check(parse("a{x}{y z}") != parse("a{x,{y z}}"));
}

void testIndex() {
    assert_parses("[a b c]#2");
    result.print();
    check(result.length == 3);
    skip(
            assert_is("(a b c)#2", "b");
            assert_is("{a b c}#2", "b");
            assert_is("[a b c]#2", "b");
    )
    todo_emit(
            assert_is("{a:1 b:2}.a", 1)
            assert_is("a of {a:1 b:2}", 1)
            assert_is("a in {a:1 b:2}", 1)
            assert_is("{a:1 b:2}[a]", 1)
            assert_is("{a:1 b:2}.b", 2)
            assert_is("b of {a:1 b:2}", 2)
            assert_is("b in {a:1 b:2}", 2)
            assert_is("{a:1 b:2}[b]", 2)
    )
}

// can be removed because noone touches List.sort algorithm!
void testSort() {
#ifndef WASM
    List<int> list = {3, 1, 2, 5, 4};
    List<int> listb = {1, 2, 3, 4, 5};
    check(list.sort() == listb)
    auto by_precedence = [](int &a, int &b) { return a * a > b * b; };
    check(list.sort(by_precedence) == listb)
    auto by_square = [](int &a) { return (float) a * a; };
    check(list.sort(by_square) == listb)
#endif
}

void testSort1() {
#ifndef WASM
    List<int> list = {3, 1, 2, 5, 4};
    List<int> listb = {1, 2, 3, 4, 5};
    auto by_precedence = [](int &a, int &b) { return a * a > b * b; };
    check(list.sort(by_precedence) == listb)
#endif
}

void testSort2() {
#ifndef WASM
    List<int> list = {3, 1, 2, 5, 4};
    List<int> listb = {1, 2, 3, 4, 5};
    auto by_square = [](int &a) { return (float) a * a; };
    check(list.sort(by_square) == listb)
#endif
}

void testRemove() {
    result = parse("a b c d");
    result.remove(1, 2);
    Node replaced = parse("a d");
    check(result == replaced);
}

void testRemove2() {
    result = parse("a b c d");
    result.remove(2, 10);
    Node replaced = parse("a b");
    check(result == replaced);
}

void testReplace() {
    result = parse("a b c d");
    result.replace(1, 2, new Node("x"));
    Node replaced = parse("a x d");
    check(result == replaced);
}


void testNodeConversions() {
    Node b = Node(true);
    print("b.kind");
    print(b.kind);
    print(typeName(b.kind));
    print("b.value.longy");
    print(b.value.longy);
    check(b.value.longy == 1);
    check(b.kind == bools);
    check(b == True);
    Node a = Node(1);
    check(a.kind == longs);
    check(a.value.longy == 1);
    Node a0 = Node(10ll);
    check(a0.kind == longs);
    check(a0.value.longy == 10);
    Node a1 = Node(1.1);
    check_eq(a1.kind, reals);
    check(a1.kind == reals);
    check(a1.value.real == 1.1);
    Node a2 = Node(1.2f);
    check(a2.kind == reals);
    check(a2.value.real == 1.2f);
    Node as = Node('a');
    check(as.kind == strings or as.kind == codepoint1);
    if (as.kind == strings) {check(*as.value.string == 'a'); }
    if (as.kind == codepoint1) check((codepoint) as.value.longy == 'a');
}


void testWasmString() {
    assert_emit("“c”", 'c');
    assert_emit("“hello1”", Node(String("hello1")));
    assert_emit("“hello2”", Node("hello2").setType(strings));
    assert_emit("“hello3”", Node("hello3"));
    assert_emit("“hello4”", "hello4");
    assert_emit("“a”", "a");
    assert_emit("“b”", "b");
    assert_emit("\"d\"", 'd');
    assert_emit("'e'", 'e');
#if WASM
    assert_emit("'f'", u'f');
    assert_emit("'g'", U'g');
#endif
    assert_emit("'h'", "h");
    assert_emit("\"i\"", "i");
    assert_emit("'j'", Node("j"));
    wasm_string x = reinterpret_cast<wasm_string>("\03abc");
    String y = String(x);
    check(y == "abc");
    check(y.length == 3);
}

void testGroupCascade0() {
    result = parse("x='abcde';x#4='y';x#4");
    check(result.length == 3);
}

void testGroupCascade1() {
    Node result0 = parse("a b; c d");
    check(result0.length == 2);
    check(result0[1].length == 2);
    result = parse("{ a b c, d e f }");
    Node result1 = parse("a b c, d e f ");
    assert_equals(result1, result);
    Node result2 = parse("a b c; d e f ");
    assert_equals(result2, result1);
    assert_equals(result2, result);
    Node result3 = parse("a,b,c;d,e,f");
    assert_equals(result3, result2);
    assert_equals(result3, result1);
    assert_equals(result3, result);
    Node result4 = parse("a, b ,c; d,e , f ");
    assert_equals(result4, result3);
    assert_equals(result4, result2);
    assert_equals(result4, result1);
    assert_equals(result4, result);
}

void testGroupCascade2() {
    result = parse("{ a b , c d ; e f , g h }");
    Node result1 = parse("{ a b , c d \n e f , g h }");
    print(result1.serialize());
    assert_equals(result1, result);
    Node result2 = parse("a b ; c d \n e f , g h ");
    assert_equals(result1, result2);
    assert_equals(result2, result);
}

void testSuperfluousIndentation() {
    result = parse("a{\n  b,c}");
    Node result1 = parse("a{b,c}");
    check(result1 == result);
}

void testGroupCascade() {
//	testGroupCascade2();
//	testGroupCascade0();
//	testGroupCascade1();

    result = parse("{ a b c, d e f; g h i , j k l \n "
                   "a2 b2 c2, d2 e2 f2; g2 h2 i2 , j2 k2 l2}"
                   "{a3 b3 c3, d3 e3 f3; g3 h3 i3 , j3 k3 l3 \n"
                   "a4 b4 c4 ,d4 e4 f4; g4 h4 i4 ,j4 k4 l4}");
    result.print();
    assert_equals(result.kind, groups);// ( {} {} ) because 2 {}!
    auto first = result.first();
    assert_equals(first.kind, objects);// { a b c … }
    assert_equals(first.first().kind, groups);// or expression if x is op
    assert_equals(result.length, 2)// {…} and {and}
    assert_equals(result[0].length, 2) // a…  and a2…  with significant newline
    assert_equals(result[0][0].length, 2)// a b c, d e f  and  g h i , j k l
    assert_equals(result[0][0][0].length, 2)// a b c  and  d e f
    assert_equals(result[0][0], parse("a b c, d e f; g h i , j k l"));// significant newline!
    assert_equals(result[0][1], parse("a2 b2 c2, d2 e2 f2; g2 h2 i2 , j2 k2 l2"));// significant newline!
    assert_equals(result[0][0][0][0].length, 3)// a b c
    skip(
            assert_equals(result[0][0][0][0], parse("a b c"));
    )
    assert_equals(result[0][0][0][0][0], "a");
    assert_equals(result[0][0][0][0][1], "b");
    assert_equals(result[0][0][0][0][2], "c");
    assert_equals(result[0][0][0][1][0], "d");
    assert_equals(result[0][0][0][1][1], "e");
    assert_equals(result[0][0][0][1][2], "f");
    assert_equals(result[1][1][0][1][2], "f4");
    Node reparse = parse(result.serialize());
    print(reparse.serialize());
    check(result == reparse);
}


void testNodeBasics() {
    Node a1 = Node(1);
//	check(a1.name == "1");// debug only!
    check(a1 == 1);
    Node a11 = Node(1.1);
    check_eq(a11.name, "1.1");
    check(a11 == 1.1);

    Node a = Node("a");
    print(a);
    print(a.serialize());
    print(a.name);
    check_eq(a.name, "a");
    check(a.name == "a");
    check(a == "a");
    Node b = Node("c");
    check_eq(b.name, "c");
    a.add(b.clone());
    check_eq(b.name, "c");// wow, worked before, corrupted memory!!
    check_eq(a.length, 1);
    check(a.children);
    Node *b2 = b.clone();
    check_eq(b.name, "c");// wow, worked before, corrupted memory!!
    check(b == b2);
    check_eq(b, a.children[0]);

//	a["b"] = "c";
    check_eq(b, a.children[0]);
    check_eq(b.name, "c");// wow, worked before, corrupted memory!!
    check_eq(a.children[0].name, "c");
    check(a.has("c"));
    check(b == a.has("c"));

//	a["b"] = "c";
    a["d"] = "e";
    check_eq(a.length, 2);
    check(a.has("d"));
    check(a["d"] == "e");
    Node &d = a.children[a.length - 1];
    check(d.length == 0);
    check(d == "e");
    check(d.kind == key);
    a.addSmart(b);// why?
}

void testBUG();

void testEmitBasics();

void testSourceMap();

void testNodeDataBinaryReconstruction() {
    check_is(parse("y:{x:2 z:3}").serialize(), "y{x:2 z:3}");// todo y:{} vs y{}
    assert_emit("y:{x:2 z:3}", parse("y:{x:2 z:3}"));// looks trivial but is epitome of binary (de)serialization!
}

void testArrayIndices() {
    skip(
    // fails second time WHY?
            assert_is("[1 2 3]", Node(1, 2, 3, 0).setType(patterns))
            assert_is("[1 2 3]", Node(1, 2, 3, 0))
    )
    assert_is("(1 4 3)#2", 4);//
    assert_is("x=(1 4 3);x#2", 4);
    assert_is("x=(1 4 3);x#2=5;x#2", 5);
}

// todo: move back into tests() once they work again
void todos() {
    testSinus();// still FRAGILE!
    testWrong0Termination();
    testErrors();// error: failed to call function   wasm trap: integer divide by zero
    assert_is("one plus two times three", 7);
    testMathExtra();// "one plus two times three"==7 used to work?
    testKitchensink();
#if not TRACE
    println("parseLong fails in trace mode WHY?");
    assert_run("parseLong('123000')+parseLong('456')", 123456);
#endif

    assert_emit("use wasp;use lowerCaseUTF;a='ÂÊÎÔÛ';lowerCaseUTF(a);a", "âêîôû")
    assert_emit("y:{x:2 z:3};y.x", 2);
    assert_emit("y:{x:'z'};y.x", 'z'); // emitData( node! ) emitNode()
    assert_emit("y{x:1}", true); // emitData( node! ) emitNode()
    assert_emit("y{x}", true); // emitData( node! ) emitNode()
    assert_emit("{x:1}", true); // emitData( node! ) emitNode()
    assert_emit("y={x:{z:1}};y", true); // emitData( node! ) emitNode()

    testNodeDataBinaryReconstruction();

    read_wasm("lib/stdio.wasm");
    testStruct();

    testWit();
    testColonImmediateBinding();
    testWasmRuntimeExtension();
    testUpperLowerCase();
//    exit(1);
    testDataMode();

    assert_run("#'0123'", 4);// todo at compile?
    assert_run("#[0 1 2 3]", 4);
    assert_run("#[a b c d]", 4);
    assert_run("len('0123')", 4);// todo at compile?
    assert_run("len([0 1 2 3])", 4);
    assert_run("size([a b c d])", 4);
    assert_run("int('123')", 123);
    assert_run("str(123)", "123");
    assert_run("'a'", 'a');
    assert_run("char(0x41)", 'a');
    assert_run("string(123)", "123");
    assert_run("String(123)", "123");
    test_sinus_wasp_import();
    testSinus();// todo FRAGILE fails before!
//    testSinus2();

    assert_emit("(2,4) == (2,4)", 1);// todo: array creation/ comparison
    assert_emit("2,4 == 2,4", 1);
    assert_emit("(2 as float, 4.3 as int)  == 2,4", 1);
    assert_emit("(2 as float, 4.3 as int)  == 2,4", 1);
    assert_emit("‖-2^2 - -2^3‖", 4);// Too many args for operator ‖,   a - b not grouped!
    testParams();
//    run("circle.wasp");
    assert_emit("1 +1 == [1 1]", 1);
    assert_emit("1 +1 ≠ 1 + 1", 1);
    testWasmMutableGlobal();

    // while without body
    assert_emit("i=0;while(i++ <10001);i", 10000)// parsed wrongly! while(  <( ++ i 10001) i)
    assert_emit("1 - 3 - square 3+4", (int64) -51);// OK!
    assert_emit("1 -3 - square 3+4", (int64) -51);// warn "mixing math op with list items (1, -3 … ) !"
    assert_emit("1 - - 3", 4);// -1 uh ok?  warn "what are you doning?"
    assert_emit("use math;⅓ ≈ .3333333 ", 1);
    assert_emit("precision = 3 digits; ⅓ ≈ .333 ", 1);
    assert_throws("i*=3");// well:
    assert_emit("i*=3", (int64) 0);
    // todo: ERRORS when cogs don't match! e.g. remove ¬ from prefixOperators!
    assert_throws("ceiling 3.7");
    // default bug!
    //    	subtract(other complex) := re -= other.re; im -= other.im
    // := is terminated by \n, not by ;!
    assert_throws("xyz 3.7"); // todo SHOULD THROW unknown symbol!
    assert_eval("if(0):{3}", false);// 0:3 messy node
    assert_equals(Node("1", 0) + Node("2"_s),
                  Node("1", "2", 0));// 1+2 => 1:2  stupid border case because 1 not group (1)
    assert_is((char *) "{a b c}#2", "b");// ok, but not for patterns:
    assert_is((char *) "[a b c]#2", "b");// patterns
    assert_is("i=3;i--", 2);// todo bring variables to interpreter
    assert_is("i=3.7;.3+i", 4);// todo bring variables to interpreter
    assert_is("i=3;i*-1", -3);// todo bring variables to interpreter

//	print("OK %s %d"s % ("WASM",1));// only 1 handed over
//    print(" OK %d %d"s % (2, 1));// error: expression result unused [-Werror,-Wunused-value] OK
}

//int dump_nr = 1;
//void dumpMemory(){
//	String file_name="dumps/dump"s+dump_nr++;
//	FILE* file=fopen(file_name,"wb");
//	size_t length = 65536*10;
//	fwrite(memory, length, 1, file);
//	fclose(file);
//}

#ifdef IMPLICIT_NODES
void testNodeImplicitConversions(){
    // looks nice, but only causes trouble and is not necessary for our runtime!
    Node b=true;
    print(typeName(b.kind));
    check(b.value.longy==1);
    check(b.kind==bools);
    check(b==True);
    Node a=1;
    check(a.kind==longs);
    check(a.value.longy=1);
    Node a0=10l;
    check(a0.kind==longs);
    check(a0.value.longy=1);
    Node a1=1.1;
    check_eq(a1.kind,reals);
    check(a1.kind==reals);
    check(a1.value.real=1.1);
    Node a2=1.2f;
    check(a2.kind==reals);
    check(a2.value.real=1.2f);
    Node as = 'a';
    check(as.kind==strings);
    check(as.value.string=='a');
}
#endif

void testUnits() {
    assert_is("1 m + 1km", Node(1001).setType(types["m"]));
}

void testPaint() {
#ifdef SDL
    init_graphics();
    while (1)paint(-1);
#endif
}

void testPaintWasm() {
#ifdef GRAFIX
    //	struct timeval stop, start;
    //	gettimeofday(&start, NULL);
        // todo: let compiler compute constant expressions like 1024*65536/4
//    	assert_emit("i=0;k='hi';while(i<1024*65536/4){i++;k#i=65};k[1]", 65)// wow SLOOW!!!
    //out of bounds memory access if only one Memory page!
        assert_emit("i=0;k='hi';while(i<16777216){i++;k#i=65};paint()", 0)// still slow, but < 1s
        // wow, SLOWER in wasm-micro-runtime HOW!?
    //	exit(0);

    //(√((x-c)^2+(y-c)^2)<r?0:255)
    //(x-c)^2+(y-c)^2
        assert_emit("h=100;r=10;i=100;c=99;r=99;x=i%w;y=i/h;k=‖(x-c)^2+(y-c)^2‖<r",1);
    ////char *wasm_paint_routine = "surface=(1,2);i=0;while(i<1000000){i++;surface#i=i*(10-√i);};paint";
        char *wasm_paint_routine = "w=1920;c=500;r=100;surface=(1,2);i=0;"
                                   "while(i<1000000){"
                                   "i++;x=i%w;y=i/w;surface#i=(x-c)^2+(y-c)^2"
                                   "};paint";
    //((x-c)^2+(y-c)^2 < r^2)?0x44aa88:0xffeedd
    //char *wasm_paint_routine = "surface=(1,2);i=0;while(i<1000000){i++;surface#i=i;};paint";
    //assert_emit(wasm_paint_routine, 0);
    //	char *wasm_paint_routine = "maxi=3840*2160/4/2;init_graphics();surface=(1,2,3);i=0;while(i<maxi){i++;surface#i=i*(10-√i);};";
        eval(wasm_paint_routine);
    //	paint(0);
    //	gettimeofday(&stop, NULL);
    //	printf("took %lu µs\n", (stop.tv_sec - start.tv_sec) * 100000 + stop.tv_usec - start.tv_usec);
    //	printf("took %lu ms\n", ((stop.tv_sec - start.tv_sec) * 100000 + stop.tv_usec - start.tv_usec) / 100);
    //	exit(0);
    //char *wasm_paint_routine = "init_graphics(); while(1){paint()}";// SDL bugs a bit
//        while (1)paint(0);// help a little
#endif
}

void testNodesInWasm() {
    assert_emit("{b:c}", parse("{b:c}"));
    assert_emit("a{b:c}", parse("a{b:c}"));
}

void testSubGroupingIndent() {
    result = parse("x{\ta\n\tb,c,\n\td;\n\te");
    assert_equals(result.length, 3);
    assert_equals(result.first(), "a");
    assert_equals(result.last(), "e");
}

void testSubGrouping() {// todo dangling ',' should make '\n' not close
//	result=parse("a\nb,c,\nd;e");
    result = parse("a\n"
                   "b,c,\n"
                   "d;\n"
                   "e");
    assert_equals(result.length, 3);// b,c,d should be grouped as one because of dangling comma
    assert_equals(result.first(), "a");
    assert_equals(result.last(), "e");
}

void testSubGroupingFlatten() { // ok [a (b,c) d] should be flattened to a (b,c) d
    result = parse("[a\nb,c\nd]");
//	result=parse("a\nb,c\nd");// still wrapped!
    assert_equals(result.length, 3);
    assert_equals(result.first(), "a");
    assert_equals(result.last(), "d");
}

void testBUG() {// move to tests() once done!
//        testRecentRandomBugs();
}

void testBadInWasm() {
    testEnumConversion();
    testDeepColon();
    testDeepColon2();
    testPattern();
}


void assurances() {
#if WASM
    //	check(sizeof(Type32) == 4) // todo:
#else
//    check(sizeof(Type32) == 4) // otherwise all header structs fall apart
    check(sizeof(Type64) == 8) // otherwise all header structs fall apart
//    check(sizeof(Type) == 8) // otherwise all header structs fall apart
#endif
//    check(sizeof(void*)==4) // otherwise all header structs fall apart TODO adjust in 64bit wasm / NORMAL arm64 !!
    check(sizeof(int64) == 8)
}

// todo: merge with testAllWasm, move these to test_wasm.cpp
void testAllEmit() {
    // WASM emit tests under the hood:
    assert_emit("√3^2", 3);// basics
//	assert_emit("42", 42);// basics
//    exit(42);
//    assert_emit("√ π ²", pi);
//    assert_emit("√π²", pi);

    testEmitBasics();
    testSinus();

    testHex();
    testModulo();
    testSmartReturn();
    testWasmString();// with length as header
    testRootLists();
//    return;
    testArrayIndices();
    testMultiValue();
    testLogic();

    testEqualities();
    testLogic01();
    testLogicOperators();
    testCall();
    testRoots();
    testRootFloat();
    testTruthiness();
    testLogicPrecedence();
    testRootLists();
    testHex();
    testArrayIndices();
    testModulo();
    testSmartReturn();
    testWasmString();// with length as header
    testMultiValue();
//    testSinus();

    testAllAngle();
    testRecentRandomBugs();
    testMergeOwn();

    testBadInWasm();
    testIndexOffset();
//    part of
//    testAllWasm() :
//    testRoundFloorCeiling();

#ifdef APPLE
    testAllSamples();
#endif
    check(NIL.value.longy == 0);// should never be modified
    print("ALL TESTS PASSED");
}

void testEmitBasics() {
    assert_emit("true", true);
    assert_emit("false", false)
    assert_emit("8.33333333332248946124e-03", 8.33333333332248946124e-03);
    assert_emit("42", 42)
    assert_emit("-42", -42)
    assert_emit("3.1415", 3.1415);
    assert_emit("-3.1415", -3.1415);
    assert_emit("'ok'", "ok");
    assert_emit("'a'", "a");
    assert_emit("'a'", 'a');
    assert_emit("40", 40);
    assert_emit("41", 41);
    assert_emit("1 ∧ 0", 0);
}


void tests() {
    assurances();
#if not WASM
    testNumbers();
#endif
    testPower();
    testString();
    testNodeBasics();
    testIterate();
    testLists();
    testEval();
    testParent();
    testNoBlock(); // fixed
    testSubGroupingFlatten();
    testNodeConversions();
    testUpperLowerCase();
    testListGrow();
    testGroupCascade();
    testNewlineLists();
    testStackedLambdas();

    testParamizedKeys();
    testForEach();
    testEmpty();
    testDiv();
    testRoot();
    testSerialize();
    skip(
            testPrimitiveTypes();
    )
//	test_sin();
    testIndentAsBlock();
    testDeepCopyDebugBugBug2();// SUBTLE: BUGS OUT ONLY ON SECOND TRY!!!
    testDeepCopyDebugBugBug();
    testComments();
    testEmptyLineGrouping();
    testSwitch();
    testAsserts();
    testFloatReturnThroughMain();
    testSuperfluousIndentation();
    testString();
    testEmptyLineGrouping();
    testColonLists();
    testGraphParams();
    testNodeName();
    testStringConcatenation();
    testStringReferenceReuse();
    testConcatenation();
    testMarkSimple();
    testMarkMulti();
    testMarkMulti2();
    testDedent2();
    testDedent();
    testGroupCascade0();
    testGraphQlQuery();
    print(testNodiscard());
    testCpp();
    testNilValues();
    testMapsAsLists();
    testMaps();
    testLists();
    testDeepLists();
    testGraphParams();
    testAddField();
    testOverwrite();
    testDidYouMeanAlias();
    testNetBase();
    testForEach();
    testLengthOperator();
    testLogicEmptySet();
    testDeepCopyDebugBugBug();
    testDeepCopyDebugBugBug2();
//    testMarkSimpleAssign();
    testMarkMultiDeep();
    testSort();
    testSort1();
    testSort2();
    testReplace();
    testRemove();
    testRemove2();
    testGraphQlQueryBug();
    testGraphQlQuery();// fails sometimes => bad pointer!?
    testGraphQlQuery2();
    testUTF();// fails sometimes => bad pointer!?
    testUnicode_UTF16_UTF32();
    testConcatenationBorderCases();
    testNewlineLists();
    testIndex();
    testGroupCascade();
    testParams();
    testSignificantWhitespace();
    testBUG();
    testFlags();
//    testFlags2();
//    testFlagSafety();
#if WASM
    warn("Currently NOT PASSING via wasmtime -D debug-info=y --dir . wasp.wasm test");
#endif
    testMarkAsMap();
    testFunctionDeclaration();
    testMarkSimple();
    testMarkMultiDeep();

#if WASM
    warn("Normal tests ALL PASSING in wasm!");
    warn("WASM emit tests CURRENTLY __ALL__ SKIPPED or asynchroneous!");
    return;
#else
    testAllEmit();
#endif
    // todo: split in test_wasp test_angle test_emit.cpp
}


void testFibonacci() {
//	assert_emit("fib(n) = n < 2 ? n : fib(n - 1) + fib(n - 2)\nfib(10)", 55);
//	assert_emit("fib(n) := n < 2 ? n : fib(n - 1) + fib(n - 2)\nfib(10)", 55);
//	assert_emit("fib = it < 2 ? 1 : fib(it - 1) + fib(it - 2)\nfib(10)", 55);
    assert_emit("fib := it < 2 ? it : fib(it - 1) + fib(it - 2)\nfib(10)", 55);
}

void pleaseFix() {
    assert_emit("π/2^2", pi / 4);
    assert_emit("(π/2)^2", pi * pi / 4);
}

// 2021-10 : 40 sec for Wasm3
// 2022-05 : 8 sec in Webapp / wasmtime with wasp.wasm built via wasm-runtime
// 2022-12-03 : 2 sec WITHOUT runtime_emit, wasmtime 4.0 X86 on M1
// 2022-12-03 : 10 sec WITH runtime_emit, wasmtime 4.0 X86 on M1
// 2022-12-28 : 3 sec WITH runtime_emit, wasmedge on M1 WOW ALL TESTS PASSING
// ⚠️ CANNOT USE assert_emit in WASM! ONLY via void testRun();
void testCurrent() {
//    testMatrixOrder();
    test_wasmedge_gc();
//    List<const int&> axx = {1, 2, 3};
//    testNamedDataSections();
//    testListGrowth<const int&>();// pointer to a reference error

// todo print as general dispatch depending on smarttype
//    assert_emit("for i in 1 to 5 : {print i};i", 6);
//    assert_emit("a = [1, 2, 3]; a[2]", 3);
    assert_emit("for i in 1 to 5 : {puti i};i", 6);
    testListGrowth<int>();
    testListGrowth<float>();
    testListGrowth<String>();
    testListGrowth<Signature>();
    testListGrowth<wabt::Index>();// just int
    testListGrowth<wabt::Reloc>();
    testListGrowth<wabt::Type>();
    testListGrowth<wabt::Location>();
    testListGrowth<wabt::Result>();
    testListGrowth<wabt::TypeVector>();
    testListGrowth<Function>(); // pretty slow with new List shared_ptr implementation
//    testListGrowth<Map>();
    testListGrowthWithStrings();
    assert_emit("x='abcde';x#4='f';x[3]", 'f'); // SIGSEGV specifically at target_depths_.resize(num_targets); !!?!
    assert_emit("x='abcde';x#4='x';x[3]", 'x'); // SIGSEGV

    testForLoops();
    skip(
    )
    testAutoSmarty();
    testArguments();
//    testSinus();
//    assert_emit("1-‖3‖/-3", 2);
//    testHostDownload();
//    testJS();
//    testHtmlWasp();
    testFlags();
    testTypes();
    skip(
            assert_emit("‖3‖-1", 2);
    )
    assert_emit("1-‖3‖/-3", 2);
    testSinus();
    test_implicit_multiplication(); // todo in parser how?

    assert_emit("-42", -42)
#if WEBAPP
        testHostIntegration();
#endif
    skip(
            testKebabCase();
    )
    testBadType();
    testDeepType();
    testTypedFunctions();
    testTypes();
    testPolymorphism();
//    testPolymorphism2();
    skip(
            testPolymorphism3();
            assert_emit("τ≈6.2831853", true);
    )

//	testDom();
    assert_emit("global x=7", 7);
    assert_eval("if 0:3", false);

    read_wasm("lib/pow.wasm");
//    read_wasm("lib/stdio.wasm");
//    assert_emit("a=√3^2", 3);// todo 2 due to int type guess
//    exit(42);
    assert_emit("π/2^2", pi / 4);
    assert_emit("(π/2)^2", pi * pi / 4);
//    assert_emit("√3^0", 0.971); // very rough power approximation
    assert_emit("√3^0", 1.0);
//    assert_emit("√3^0", 1);

//    testInclude();
//    check_is("τ≈6.2831853",true);
//    assert_emit("τ≈6.2831853", true);
//    assert_emit("square := it*it; square 3", 9);
//    assert_emit("a = [1, 2, 3]; a[1] == a#1", false);
//    assert_emit("a = [1, 2, 3]; a[1] == a#1", 0);
//    exit(42);
//    assert_emit("τ≈6.2831853",1);
//    testExceptions();
//    assert_emit("√ π ²", pi);
//    assert_emit("√π²", pi);


//    testGlobals();
//    testTypeConfusion();


//    testVectorShim();// use GPU even before wasm vector extension is available
//    testSourceMap();
//    return;

//	testDwarf();
//	testFibonacci();
//	initTypes();
//	check_eq(types["u8"],types["byte"]);
//	testUnicode_UTF16_UTF32();
//	assert_emit("print('hi')", 0)
//	assert_emit("puts('hi')", 8)
//	testReplaceAll();
//	return;
//
//	return;
#if WEBAPP or MY_WASM
    testHostIntegration();
#endif
//	exit(1);
//	testOldRandomBugs();

//    assert_emit("3^2", 9);
//	assert_emit("3**2", 9);
//    exit(1);
//	assert_emit("n=3;2ⁿ", 8);
//	assert_emit("k=(1,2,3);i=1;k#i=4;k#i", 4)
//	assert_emit("'αβγδε'#3", U'γ');
//	assert_emit("√9*-‖-3‖/-3", 3);

    skip(
            assert_emit("x=3;y=4;c=1;r=5;((‖(x-c)^2+(y-c)^2‖<r)?10:255", 255);
            assert_emit("i=3;k='αβγδε';k#i='Γ';k#i", u'Γ'); // todo setCharAt
            testGenerics();
    )
#if not WASM
    testWasmGC();
#endif
//    testRenameWasmFunction();
//    testStruct();
//    tests();// make sure all still ok before changes
//    todos();
    tests();// make sure all still ok after messing with memory

#if not WASM
// ⚠️ in WASM these tests are called via async trick
    testAngle();// fails in WASM why?
    testMergeGlobal();
    testAssertRun(); // separate because they take longer (≈10 sec as of 2022.12)
    testAllWasm();
    // ALL tests up to here take only 1 sec !
//    todos();// those not passing yet (skip)
#endif
    print("CURRENT TESTS PASSED");
}

// valgrind --track-origins=yes ./wasp
