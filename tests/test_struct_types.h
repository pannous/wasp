#include "../source/asserts.h"
// void test_wasm_structs();


void testStruct() {
    tests_executed++;
    use_wasm_structs = true;
    // builtin with struct/record
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
    tests_executed++;
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

void testWasmGC() {
    tests_executed++;
    //    assert_emit("y=(1 4 3)[1]", 4);
    //    assert_is("x=(1 4 3);x#2", 4);
    //assert_emit("42",42);
    use_wasm_structs = true;
    use_wasm_strings = true;
    use_wasm_arrays = true;
    //    assert_emit("x=(1 2 3)", 0);
    Node fun;
    fun.name = "first";
    fun.kind = declaration; // ≠ functor;
    fun.type = types["u8"];

    Node fun_type;
    fun.name = "my_callback";
    fun.kind = clazz;
    //	fun.kind = functor; todo

    //	testGcFunctionReferences();
    assert_emit("(type $int_callback (func (result i32)))", fun_type); // e.g. random provider
    assert_emit("(type $my_notification (func ))", fun_type);
    assert_emit("(type $my_callback (func (param i32) (result i32)))", fun_type);
    //	testGcFunctionReferenceParameters();
    //	testGcReferenceParameters();
    assert_emit("def first(array);", fun);
    assert_emit("def first(array<u8>);", fun);
    assert_emit("def first(list<u8>);", fun);
    assert_emit("x=(5 4 3);u8 first(list<u8> y){y#1};first(x)", 5);
    assert_emit("x=(5 6 7);#x", 3);
    assert_emit("x=(5 6 7);x#2", 6);
    assert_emit("'world'#1", 'w');
    assert_emit("y=(1 4 3)#2", 4);
    assert_emit(("id(3*42)≥2*3"), 1)
    assert_emit("#'abcde'", 5);
    assert_emit("x='abcde';#x", 5);
    assert_emit("x=(1 2 1 2 1);#x", 5);
    //	assert_emit("#(1 2 1)", 3);

    assert_emit("x='abcde';x#4='f';x[3]", 'f');
    assert_emit("42", 42); // basics
    //    assert_emit("x=(1 2 3);x[1]", 2);
    //    assert_emit("x=(1 2 3);2", 2);
    //    assert_emit("(1 2 3)[1]", 2);
    //    exit(0);
    //    assert_emit("x=[1 2 3];x[1]", 2);
    //    assert_emit("x=[1 2 3];x[1]=4;x[1]", 4);
    assert_emit("struct a{x:int y:int z:int};a{1 3 4}.y", 3);

    assert_emit("'abcd'", "abcd");
    assert_emit("'ab'+'cd'=='abcd'", 1);
    assert_emit("abcde='fghij';42", 42);
    //    assert_emit("abcd='fghij';#abcd", 5);
    //    assert_emit("abcde='fghij'", "fghij"); // main can't return stringrefs!

    //    exit(0);
}

static void test_wasm_node_struct() {

    tests_executed++;
    // auto wasp_object_code = "a{b:c}";
    auto wasp_object_code = "a{b:42}";
    auto aNode=parse(wasp_object_code);
    assert_emit(wasp_object_code, aNode);
}

static void test_wasm_linear_memory_node() {
    tests_executed++;
    // auto wasp_object_code = "a{b:c}";
    auto wasp_object_code = "a{b:42}";
    auto aNode=parse(wasp_object_code);
    assert_emit(wasp_object_code, aNode);
}

static void test_wasm_structs() {
    tests_executed++;
    test_wasm_node_struct();
    auto aNode = Node("A").setKind(clazz);
    aNode["a"]=IntegerType;
    auto a2 = analyze("class A{a:int}");
    assert_equals(aNode, a2);
    assert_emit("class A{a:int}", aNode);
}