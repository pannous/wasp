#include "Angle.h" // emit
#include "Wasp.h"
#include "wasm_reader.h"
#include "wasm_merger.h"

//#define assert_emit(α, β) printf("%s\n%s:%d\n",α,__FILE__,__LINE__);if (!assert_equals_x(emit(α),β)){printf("%s != %s",#α,#β);backtrace_line();}
#define assert_emit(α, β) try{printf("%s\n%s:%d\n",α,__FILE__,__LINE__);if (!assert_equals_x(emit(α),β)){printf("%s != %s",#α,#β);backtrace_line();}}catch(chars x){printf("%s\nIN %s",x,α);backtrace_line();}

void testWasmFunctionDefiniton() {
//	assert_is("add1 x:=x+1;add1 3", (long) 4);
	assert_emit("add1 x:=x+1;add1 3", (long) 4);
	assert_emit("double x:=x*2;double(4)", 8)
	assert_emit("double:=it*2; double 3", 6)
	assert_emit("double:=it*2; double 3*4", 24)
	assert_emit("double:=it*2; double(3*42) > double 2*3", 1)
	//0 , 1 , 1 , 2 , 3 , 5 , 8 , 13 , 21 , 34 , 55 , 89 , 144
	assert_emit("fib x:=if x<2 then x else fib(x-1)+fib(x-2);fib(7)", 13)
	assert_emit("fib:=if it<2 then it else fib(it-1)+fib(it-2);fib(7)", 13)
	skip(
	assert_emit("fib:=it<2 and it or fib(it-1)+fib(it-2);fib(7)", 13)
	assert_emit("fib:=it<2 then it or fib(it-1)+fib(it-2);fib(7)", 13)
	assert_emit("fib:=it<2 or fib(it-1)+fib(it-2);fib(4)", 5)
	assert_emit("fib:=it<2 then 1 else fib(it-1)+fib(it-2);fib(4)", 5)
			)
}

void testWasmFunctionCalls() {

	assert_emit("id (3+3)", (long) 6);
	assert_emit("square 3", 9);
	assert_emit("id 123", (long) 123);
	assert_is("id 3+3", 6);
	assert_emit("logf 3.1", (long) 0);// auto return 0 if call returns void
	assert_emit("logi 3", (long) 0);
	assert_emit("logi 3+3", (long) 0);
	assert_emit("4*5 + square 2*3", (long) 56);
	assert_emit("id 3+3", (long) 6);
	assert_emit("3 + square 3", (long) 12);
	assert_emit("1+2 + square 1+2", (long) 12);

	assert_is("id 3+3", 6);
	assert_emit("3 + id 3+3", (long) 9);
	assert_emit("3 + √9", (long) 6);
	assert_emit("id(3*42) > id 2*3", 1)
	assert_emit("square(3*42) > square 2*3", 1)
}

void testConstReturn() {
	assert_emit(("42"), 42)
}

void testPrint() {
	assert_emit(("print 42"), 42)
}

void testMathPrimitives() {
	skip(
			assert_emit(("42.1"), 42.1) // todo: return &Node(42.1) or print value to stdout
			assert_emit(("-42.1"), 42.1)
	)
	assert_emit(("42"), 42)
	assert_emit("-42", -42)
	assert_emit(("2000000000"), 2000000000)
	assert_emit(("-2000000000"), -2000000000)
}

void testFloatOperators() {
	assert_equals(eval("42.0/2.0"), 21)
	assert_emit(("3.0+3.0*3.0"), 12)
	assert_emit(("42.0/2.0"), 21)
	assert_emit(("42.0*2.0"), 84)
	assert_emit(("42.0+2.0"), 44)
	assert_emit(("42.0-2.0"), 40)
	assert_emit(("3.0+3.0*3.0"), 12)
	assert_emit(("3.1>3.0"), true)
	assert_emit(("2.1<3.0"), true)
	skip(
			"BUG IN WASM?? should work!?"
			assert_emit(("2.1<=3.0"), true)
			assert_emit(("3.1>=3.0"), true)
	)

	assert_emit(("3.0+3.0*3.0>3.0+3.0+3.0"), true)
	assert_emit(("3.0+3.0*3.0<3.0*3.0*3.0"), true)
	assert_emit(("3.0+3.0*3.0<3.0+3.0+3.0"), false)
	assert_emit(("3.0+3.0*3.0>3.0*3.0*3.0"), false) // 0x1.8p+1 == 3.0
	assert_emit(("3.0+3.0+3.0<3.0+3.0*3.0"), true)
	assert_emit(("3.0*3.0*3.0>3.0+3.0*3.0"), true)
}

void testMathOperators() {
//	assert_emit(("42 2 *"), 84)
	assert_equals(eval("7%5"), 2)
	assert_equals(eval("42/2"), 21)
	assert_emit(("42/2"), 21)
	assert_emit(("42*2"), 84)
	assert_emit(("42+2"), 44)
	assert_emit(("42-2"), 40)
	assert_emit(("3+3*3"), 12)
	assert_emit(("3+3*3>3+3+3"), true)
	assert_emit(("3+3*3<3*3*3"), true)
	assert_emit(("3+3*3<3+3+3"), false)
	assert_emit(("3+3*3>3*3*3"), false)
	assert_emit(("3+3+3<3+3*3"), true)
	assert_emit(("3*3*3>3+3*3"), true)
//	assert_emit(("3⁰"),1);
//	assert_emit(("3¹"),3);
//	assert_emit(("3²"),9);
//	assert_emit(("3³"),27);
//	assert_emit(("3⁴"),9*9);
	skip(
			assert_emit(("42^2"), 1764) NO SUCH PRIMITIVE
	)
}

void testComparisonMath() {
	// may be evaluated by compiler!
	assert_emit(("3*42>2*3"), 1)
	assert_emit(("3*1<2*3"), 1)
	assert_emit(("3*452==452*3"), 1)
	assert_emit(("3*42≥2*3"), 1)
	assert_emit(("3*2≥2*3"), 1)
	assert_emit(("3*2≤2*3"), 1)
	assert_emit(("3*2≤24*3"), 1)
	assert_emit(("3*13!=14*3"), 1)
	assert_emit(("3*13<=14*3"), 1)
	assert_emit(("3*15>=14*3"), 1)
	assert_emit(("3*42<2*3"), False);
	assert_emit(("3*1>2*3"), False);
	assert_emit(("3*452!=452*3"), False);
	assert_emit(("3*13==14*3"), False);
	assert_emit(("3*13>=14*3"), False);
	assert_emit(("3*15<=14*3"), False);
	assert_emit(("3*42≥112*3"), false)
	assert_emit(("3*2≥112*3"), false)
	assert_emit(("3*12≤2*3"), false)
	assert_emit(("3*112≤24*3"), false)
}


void testComparisonId() {
	// may be evaluated by compiler!
	assert_emit("id(3*42 )> id 2*3", 1)
	assert_emit("id(3*1)< id 2*3", 1)
	assert_emit("id(3*452)==452*3", 1)
	assert_emit(("id(3*42)≥2*3"), 1)
	assert_emit(("id(3*2)≥2*3"), 1)
	assert_emit(("id(3*2)≤2*3"), 1)
	assert_emit(("id(3*2)≤24*3"), 1)
	assert_emit(("id(3*13)!=14*3"), 1)
	assert_emit(("id(3*13)<= id 14*3"), 1)
	assert_emit(("id(3*13)<= id 14*3"), 1)

	assert_emit(("id(3*15)>= id 14*3"), 1)
	assert_emit(("id(3*42)< id 2*3"), False);
	assert_emit(("id(3*1)> id 2*3"), False);
	assert_emit(("id(3*452)!=452*3"), False);
	assert_emit(("id(3*13)==14*3"), False);
	assert_emit(("id(3*13)>= id 14*3"), False);
	assert_emit(("id(3*15)<= id 14*3"), False);
	assert_emit(("id(3*13)<= id 14*3"), 1)
	assert_emit(("id(3*42)≥112*3"), false)
	assert_emit(("id(3*2)≥112*3"), false)
	assert_emit(("id(3*12)≤2*3"), false)
	assert_emit(("id(3*112)≤24*3"), false)
}

void testComparisonIdPrecedence() {
	// may be evaluated by compiler!
	assert_emit("id 3*42 > id 2*3", 1)
	assert_emit("id 3*1< id 2*3", 1)
	assert_emit("id 3*452==452*3", 1)
	assert_emit(("id 3*42≥2*3"), 1)
	assert_emit(("id 3*2≥2*3"), 1)
	assert_emit(("id 3*2≤2*3"), 1)
	assert_emit(("id 3*2≤24*3"), 1)
	assert_emit(("id 3*13!=14*3"), 1)
	assert_emit(("id 3*13<= id 14*3"), 1)
	assert_emit(("id 3*13<= id 14*3"), 1)

	assert_emit(("id 3*15>= id 14*3"), 1)
	assert_emit(("id 3*42< id 2*3"), False);
	assert_emit(("id 3*1> id 2*3"), False);
	assert_emit(("id 3*452!=452*3"), False);
	assert_emit(("id 3*13==14*3"), False);
	assert_emit(("id 3*13>= id 14*3"), False);
	assert_emit(("id 3*15<= id 14*3"), False);
	assert_emit(("id 3*13<= id 14*3"), 1)
	assert_emit(("id 3*42≥112*3"), false)
	assert_emit(("id 3*2≥112*3"), false)
	assert_emit(("id 3*12≤2*3"), false)
	assert_emit(("id 3*112≤24*3"), false)
}

void testComparisonPrimitives() {
	// may be evaluated by compiler!
	assert_emit(("42>2"), 1)
	assert_emit(("1<2"), 1)
	assert_emit(("452==452"), 1)
	assert_emit(("42≥2"), 1)
	assert_emit(("2≥2"), 1)
	assert_emit(("2≤2"), 1)
	assert_emit(("2≤24"), 1)
	assert_emit(("13!=14"), 1)
	assert_emit(("13<=14"), 1)
	assert_emit(("15>=14"), 1)
	assert_emit(("42<2"), False);
	assert_emit(("1>2"), False);
	assert_emit(("452!=452"), False);
	assert_emit(("13==14"), False);
	assert_emit(("13>=14"), False);
	assert_emit(("15<=14"), False);
	assert_emit(("42≥112"), false)
	assert_emit(("2≥112"), false)
	assert_emit(("12≤2"), false)
	assert_emit(("112≤24"), false)
}

void testWasmLogicPrimitives() {

	skip( // todo: if emit returns Node:
			assert_emit(("false").name, False.name);// NO LOL emit only returns number
			assert_emit(("false"), False);
	)

	assert_emit("true", True);
	assert_emit("true", true);
	assert_emit("true", 1);

	assert_emit("false", false);
	assert_emit("false", False);
	assert_emit("false", (long) 0);

	assert_emit("nil", false);
	assert_emit("null", false);
	assert_emit("null", (long) 0);
	assert_emit("null", (long) nullptr);
	assert_emit("ø", false);
	assert_emit("nil", NIL);
}


void testWasmLogicUnaryVariables() {

}

void testWasmLogicUnary() {
	assert_emit("not 0.0", true);
	assert_emit("not ø", true);
	assert_emit("not false", true);
	assert_emit("not 0", true);

	assert_emit("not true", false);
	assert_emit("not 1", false);
	assert_emit("not 123", false);
}

void testWasmLogicOnObjects() {
	assert_emit("not 'a'", false);
	assert_emit("not {a:2}", false);
	assert_emit("not {a:0}", false);// maybe

	assert_emit("not ()", true);
	assert_emit("not {}", true);
	assert_emit("not []", true);
	assert_emit("not ({[ø]})", true); // might skip :)

}

void testWasmLogic() {
	skip(
// should be easy to do, but do we really want this?
			assert_emit("true true and", true);
			assert_emit("false true and", false);
			assert_emit("false false and ", false);
			assert_emit("true false and ", false);
	)
	check(parse("false and false").length == 3);
	assert_emit("false and false", false);
	assert_emit("false and true", false);
	assert_emit("true and false", false);
	assert_emit("true and true", true);
	assert_emit("true or false and false", true);// == true or (false)

	assert_emit("false xor true", true);
	assert_emit("true xor false", true);
	assert_emit("false xor false", false);
	assert_emit("true xor true", false);
	assert_emit("false or true", true);
	assert_emit("false or false", false);
	assert_emit("true or false", true);
	assert_emit("true or true", true);
}

void testWasmIf() {
	assert_emit("if(condition=2,then=3,else=4)", 3); // this is what happens under the hood (?)
	assert_emit("if 2 : 3 else 4", 3);
	assert_emit("if 2 then 3 else 4", 3);
	assert_emit("if(2,3,4)", 3);
	assert_emit("if({2},{3},{4})", 3);
	assert_emit("if(condition=2,then=3)", 3);
	assert_emit("if(2){3}{4}", 3);
}


void testWasmMemoryIntegrity() {
#ifndef WASM
	return;
#endif

	if (!MEMORY_SIZE) {
		error("NO MEMORY");
	}
	printf("MEMORY start at %ld\n", (long) memory);
	printf("current start at %ld\n", (long) current);
//	Bus error: 10  if i > MEMORY_SIZE
// Fails at 100000, works at 100001 WHERE IS THIS SET?
//	int start=125608;
	int start = HEAP_OFFSET * 2;// out of bounds table access CORRUPTION!
	int end = MEMORY_SIZE / 4; // /4 because 1 int = 4 bytes
	for (int i = start; i < end; ++i) {
		int tmp = memory[i];
//		memory[i] = memory[i]+1;
//		memory[i] = memory[i]-1;
		memory[i] = i;
//		if(i%10000==0)logi(i);// logi USES STACK, so it can EXHAUST if called too often!
		if (memory[i] != i) {
			printf("MEMORY CORRUPTION at %d", i);
			exit(0);
		}
		memory[i]=tmp;// else test fail
	}
}

void testRecentRandomBugs() {
//		testGraphQlQuery();

	check(operator_list.has("+"));
	check(not(bool) Node("x"));
	check(false == (bool) Node("x"));
	check(Node("x") == false);
	assert_is("x", Node(false));// passes now but not later!!

	assert_is("x", false);// passes now but not later!!
	assert_is("y", false);
	assert_is("x", false);
	assert(eval("ç='☺'") == "☺");// fails later => bad pointer?
	assert(eval("(2+1)==(4-1)") == 1);
	assert(eval("3==2+1") == 1);
	assert(eval("2+1==2+1") == 1);
	assert_emit("square 3", 9);
	assert_emit("id (3+3)", (long) 6);
	const Node &node = parse("x:40;x+1");
	check(node.length == 2)
	check(node[0]["x"] == 40)


//0 , 1 , 1 , 2 , 3 , 5 , 8 , 13 , 21 , 34 , 55 , 89 , 144
//	assert_emit("fib(it-1)",3);
	assert_emit("if 4>1 then 2 else 3", 2)

	assert_emit("double := it * 2 ; double(4)", 8)
	assert_emit("double:=it*2;double(4)", 8)

	assert_emit("1 -3 - square 3+4", (long) -51);
	assert_emit("1+2 + square 3+4", (long) 52);

	assert_emit("4*5 + square 2*3", (long) 56);
//	assert_emit("id 3*42> id 2*3", 1)
	assert_emit("x=41;if x>1 then 2 else 3", 2)
	assert_emit("x:41;if x>1 then 2 else 3", 2)
	assert_emit("x:41;if x<1 then 2 else 3", 3)


	assert_emit("x:41;x+1", 42)

//	exit(1);
//	const Node &node1 = parse("x:40;x++;x+1");
//	check(node.length==3)
//	check(node[0]["x"]==40)
//	exit(1);
	assert_emit("3 + √9", (long) 6);
	assert_emit("square 3", 9);
	assert_emit("-42", -42)
}


//testWasmControlFlow
void wasm_todos() {
	assert_emit(("42.1"), 42.1) // main returns int, should be pointer to value!
	skip(
			testsFailingInWasm();
			assert_emit("0.0", (long) 0);// can't emit float yet
			assert_emit(("x*=14"), 1)
			assert_emit(("x=15;x>=14"), 1)
//			Ambiguous mixing of functions `ƒ 1 + ƒ 1 ` can be read as `ƒ(1 + ƒ 1)` or `ƒ(1) + ƒ 1`
			assert_emit("id 3*42 > id 2*3", 1)
			assert_emit("square 3*42 > square 2*3", 1)
			assert_emit("double:=it*2; double 3*42 > double 2*3", 1)
	)
}

//void testRefactor(){
//	wabt::Module *module = readWasm("t.wasm");
//	refactor_wasm(module, "__original_main", "_start");
//	module = readWasm("out.wasm");
//	check(module->funcs.front()->name == "_start");
//}


//#include "wasm_merger.h"
void testMergeWabt() {
//	merge_files({"test-lld-wasm/main.wasm", "test-lld-wasm/lib.wasm"});
//	wabt::Module *main = readWasm("test-lld-wasm/main.wasm");
//	wabt::Module *module = readWasm("test-lld-wasm/lib.wasm");
//	Module *merged = merge_wasm(main, module);
//	save_wasm(merged);
//	int ok=run_wasm(merged);
//	int ok=run_wasm("a.wasm");
//	check(ok==42);
}

void testWasmRuntimeExtension() {
#ifndef RUNTIME_ONLY
	functionIndices.setDefault(-1);
	Module runtime = read_wasm("wasp.wasm");
	Node charged = analyze(parse("teste:=42"));
	Code lib = emit(charged, &runtime, "maine");
	lib.save("main.wasm");// partial wasm!
	Module main = read_wasm("main.wasm");
	Code code = merge_wasm(runtime, main);
	code.save("merged.wasm");
	read_wasm("merged.wasm");
	int result = code.run();
	check_eq(result, 42);

#endif
}

//testMerge
void testWasmModuleExtension() {
#ifndef RUNTIME_ONLY
	functionSignatures.clear();
	Node charged = analyze(parse("test:=42"));
	Code lib = emit(charged, 0, "lib_main");
	lib.save("lib.wasm");
	Module module = read_wasm("lib.wasm");
	charged = analyze(parse("test"));
	Code main = emit(charged, &module);
	main.save("main.wasm");
	Module prog = read_wasm("main.wasm");
	Code merged = merge_wasm(module, prog);
	merged.save("merged.wasm");
	read_wasm("merged.wasm");
	int ok = merged.run();
//	int ok = main.run();
	check(ok==42);
#endif
}



void testMergeRelocate(){
#ifndef RUNTIME_ONLY
	// doesn't work: cannot insert imports or function types!
//	emit("test");
//	merge_files({"test.wasm", "test-lld-wasm/lib.wasm"});
	Module lib = read_wasm("test-lld-wasm/lib.wasm");
	Module main = read_wasm("test-lld-wasm/main.wasm");
//	Module main=read_wasm("test.wasm");
	Code merged=merge_wasm(lib, main);
	merged.save("merged.wasm");
	Module merged1 = read_wasm("merged.wasm");
	merged.run();
//	wabt::Module *merged=merge_wasm(lib, main);
//	save_wasm(merged, "prog.wasm");
#endif
}

void testAllWasm() {
	testWasmMemoryIntegrity();
	// todo: reuse all tests via
	//	interpret = false;
	// constant things may be evaluated by compiler!
	testWasmModuleExtension();
	testWasmRuntimeExtension();
#ifdef RUNTIME_ONLY
	logs("RUNTIME_ONLY");
	logs("NO WASM emission...");
	return;
#endif
//	testMerge();
//	exit(0);
//	testRefactor();
	assert_emit("double x := x * 2 ; double(4)", 8)
//	assert_emit("double := it * 2 ; double(4)", 8)

	read_wasm("main.wasm");
	assert_emit("-42", -42)
	assert_emit("x=41;x+1", 42)
	assert_emit("x=40;y=2;x+y", 42)
	assert_emit("id(4*42) > id 2+3", 1)
	assert_emit("double := it * 2 ; double(4)", 8)
	assert_emit("double:=it*2; double 3", 6)
	assert_emit("fib x:=if x<2 then x else fib(x-1)+fib(x-2);fib(7)", 13)
	assert_emit("fib x:=if x<2 then x else{fib(x-1)+fib(x-2)};fib(7)", 13)
	assert_emit("add1 x:=x+1;add1 3", (long) 4);

//	testRecentRandomBugs();
//	run_wasm("../t.wasm");

// TRUE TESTS:
	testWasmFunctionDefiniton();
	testWasmFunctionCalls();
	testFloatOperators();
	testWasmLogicUnary();
	testConstReturn();
	testWasmLogic();
	testWasmLogicPrimitives();
	testMathOperators();
	testMathPrimitives();
	testComparisonPrimitives();
	testComparisonMath();
	testComparisonId();
	skip(
			wasm_todos();
			testWasmLogicOnObjects();
	)
}
