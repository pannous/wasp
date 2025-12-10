#include "../source/asserts.h"
// void test_wasm_structs();
static void test_wasm_node_struct() {
    tests_executed++;
    // auto waspo = "a{b:c}";
    auto waspo = "a{b:42}";
    auto aNode=parse(waspo);
    assert_emit(waspo, aNode);
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