
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
    check_is("test from JS"s, s);
//    print(wasp.name);
//    print("wasp.total_func_count");
//    print(wasp.total_func_count);
    auto replaced = s->replace("test", "ok").replace("JS", "WASP");
    check_is("ok from WASP"s, replaced);
    return &replaced.clone();
//    return new String("ok from WASP");
}

extern byte *stack_hack;

extern "C" void testRuntime(bytes buffer, size_t size) {
// ⚠️ Problem arises before even doing anything with data…  :
//    testCurrent();// messes with the heap just filled with the wasm file

    heap_end = buffer + size;
//    Code(buffer, size, false).debug();
    Module &wasp = read_wasm(buffer, size);
    wasp.code.name = "wasp";
    wasp.name = "wasp";
    module_cache.add("wasp"s.hash(), &wasp);
// testCurrent();
// loadRuntime();
// testCurrent();
}

void testDownload() {
#if not WASM or MY_WASM
	auto res = download("http://pannous.com/files/test");
	print(res);
	check_eq("test 2 5 3 7"s, res);
#endif
}

//  ⚠️ do NOT put synchronous tests here! use testCurrent for those!
extern "C" void testRun() {
    if (done.empty())
        print("NOW RUNNING all async emit TESTS!");

//    testDownload(); // not on localhost
//	assert_emit("square := it*it; square 3", 9);
	assert_emit("2+3", 5);
//	assert_emit("2+2", 4);
//	assert_emit("2*2", 4);
//	assert_emit("2^2", 4);
//	assert_emit("√9", 3);
//	assert_emit("square 2", 4); // missing import for function square
//	testWasmGC();

//    skip(
//            testIndexOffset();
//    )
//    assert_emit("puts('ok');(1 4 3)#2", 4); // EXPECT 4 GOT 1n


//    return;
//    assert_emit("42", 42);
//    assert_emit("42", 43); // Error: ⚠️ TEST FAILED!  works
//    assert_emit("fib:=if it<2 then it else fib(it-1)+fib(it-2);fib(7)", 13)

	testAllWasm();
	testAllEmit();
	testAllAngle();
//    heap_end=__initial_heap_end+0x100000;// reset on each run!
    print("testRun SUCCEEDED");
}
