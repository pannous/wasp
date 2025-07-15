#include "Angle.h"
#include "Node.h"
#include "tests.h"
#include "asserts.h"
#include "wasm_reader.h"
#include "wasm_helpers.h"

extern "C" Node *testNodeJS(String *s) {
    //        ADD_COMPILE_FLAG("-Wno-address-of-temporary") # ok when there is no stack / GC
    return new Node(String("test"));
}

extern "C" String *testFromJS(String *s) {
    println("testJString…");
    check_is("test from JS"s, *s); // OK
    // check_is("test from JS"s, s); // NOT OK !?!?!? todo <<
    // print(wasp.name);
    //    print("wasp.total_func_count");
    //    print(wasp.total_func_count);
    auto replaced = s->replace("test", "ok").replace("JS", "WASP");
    check_is("ok from WASP"s, replaced);
    return &replaced.clone();
    //    return new String("ok from WASP");
}

extern byte *stack_hack;

void testWaspRuntimeModule();

extern "C" void parseRuntime(bytes buffer, size_t size) {
    info("⚠️ parseRuntime 'wasp' from js provided bytes");
    heap_end = buffer + size; // js HEAP out of sync, even bigger to allocate demangle strings!
    Module &wasp = read_wasm(buffer, size);
    wasp.code.name = "wasp";
    wasp.name = "wasp";
    module_cache.add("wasp"s.hash(), &wasp);
    libraries.add(&wasp);
    info("⚠️ parseRuntime DONE");
    // testWaspRuntimeModule();
}

void testDownload() {
#if not WASM or MY_WASM
	auto res = download("http://pannous.com/files/test");
	print(res);
	check_eq("test 2 5 3 7"s, res);
#endif
}

void testAllWasm();
void testAllEmit();
void testAllAngle();
void testBadInWasm();
void testTodoBrowser();

//  ⚠️ do NOT put synchronous tests here! use testCurrent for those!
extern "C" void testRun() {
    // print("testRun DEACTIVATED!");
    if (done.empty()) // async via done list
        print("💡💡💡NOW RUNNING all async emit TESTS!💡💡💡");
    // we may run into MEMORY OVERFLOW
    // ⚠️ ERROR: RuntimeError: memory access out of bounds / Duplicate export name 'main' (misleading!!)
    testTodoBrowser();
    testAllAngle();
    testAllEmit();
    testAllWasm();
    skip( // TODO!
        testBadInWasm();
    )
    // assert_emit("'TESTS OK'", "TESTS OK");// todo lol
    //    assert_emit("42", 43); // Error: ⚠️ TEST FAILED!  works
    print("testRun SUCCEEDED");
}
