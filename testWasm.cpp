#include "Angle.h" // emit
#include "Wasp.h"

#define assert_emit(α, β) if (!assert_equals_x(emit(α),β)){printf("%s != %s",#α,#β);backtrace_line();}

void testConstReturn(){
	assert_equals(emit("42"), 42)
}

void testPrint(){
	assert_equals(emit("print 42"), 42)
}
void testMathPrimitives() {
	skip(
			assert_equals(emit("42.1"), 42.1) // todo: return &Node(42.1) or print value to stdout
	assert_equals(emit("-42.1"), 42.1)
	)
	assert_equals(emit("42"), 42)
	assert_equals(emit("-42"), -42)
	assert_equals(emit("2000000000"), 2000000000)
	assert_equals(emit("-2000000000"), -2000000000)
}

void testMathOperators(){
//	assert_equals(emit("42 2 *"), 84)
	assert_equals(eval("42/2"), 21)
	assert_equals(emit("42/2"), 21)
	assert_equals(emit("42*2"), 84)
	assert_equals(emit("42+2"), 44)
	assert_equals(emit("42-2"), 40)
	assert_equals(emit("3+3*3"), 12)
	assert_equals(emit("3+3*3>3+3+3"), true)
	assert_equals(emit("3+3*3<3*3*3"), true)
	assert_equals(emit("3+3*3<3+3+3"), false)
	assert_equals(emit("3+3*3>3*3*3"), false)
	assert_equals(emit("3+3+3<3+3*3"), true)
	assert_equals(emit("3*3*3>3+3*3"), true)
//	assert_equals(emit("3⁰"),1);
//	assert_equals(emit("3¹"),3);
//	assert_equals(emit("3²"),9);
//	assert_equals(emit("3³"),27);
//	assert_equals(emit("3⁴"),9*9);
	skip(
	assert_equals(emit("42^2"), 1764) NO SUCH PRIMITIVE
			)
}


void testComparisonPrimitives() {
	// may be evaluated by compiler!
	assert_equals(emit("42>2"), 1)
	assert_equals(emit("1<2"), 1)
	assert_equals(emit("452==452"), 1)
	assert_equals(emit("42≥2"), 1)
	assert_equals(emit("2≥2"), 1)
	assert_equals(emit("2≤2"), 1)
	assert_equals(emit("2≤24"), 1)
	assert_equals(emit("13!=14"), 1)
	assert_equals(emit("13<=14"), 1)
	assert_equals(emit("15>=14"), 1)
	assert_equals(emit("42<2"), False);
	assert_equals(emit("1>2"), False);
	assert_equals(emit("452!=452"), False);
	assert_equals(emit("13==14"), False);
	assert_equals(emit("13>=14"), False);
	assert_equals(emit("15<=14"), False);
	assert_equals(emit("42≥112"), false)
	assert_equals(emit("2≥112"), false)
	assert_equals(emit("12≤2"), false)
	assert_equals(emit("112≤24"), false)
}

void testWasmLogicPrimitives() {

	skip( // todo: if emit returns Node:
	assert_equals(emit("false").name, False.name);// NO LOL emit only returns number
	assert_equals(emit("false"), False);
			)

	assert_emit("true", True);
	assert_emit("true", true);
	assert_emit("true", 1);

	assert_emit("false", false);
	assert_emit("false", False);
	assert_emit("false", (long)0);

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
	assert_emit("not false", true);
	assert_emit("not 0", true);
	assert_emit("not 0.0", true);
	assert_emit("not ø", true);

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

void testWasmLogic(){
	skip(
// should be easy to do, but do we really want this?
	assert_emit("true true and", true);
	assert_emit("false true and", false);
	assert_emit("false false and ", false);
	assert_emit("true false and ", false);
	)
	check(parse("false and false").length==3);
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

void testWasmIf(){
	assert_emit("if(condition=2,then=3,else=4)", 3); // this is what happens under the hood (?)
	assert_emit("if 2 : 3 else 4", 3);
	assert_emit("if 2 then 3 else 4", 3);
	assert_emit("if(2,3,4)", 3);
	assert_emit("if({2},{3},{4})", 3);
	assert_emit("if(condition=2,then=3)", 3);
	assert_emit("if(2){3}{4}", 3);
}


void testWasmMemoryIntegrity() {
//	Bus error: 10  if i > memory_size
	for (int i = 0; i <= 32624 / 2 /*memory_size*/; ++i) {
		memory[i] = i;
		if (memory[i] != i) {
//			printf("MEMORY CORRUPTION");
			exit(-1);
		}
	}

//	String s = "ja %s gut"s % "so";
	for (int i = 0; i <= 32624 / 2 /*memory_size*/; ++i) {
		if (memory[i] != i) {
//			logi(i);
		}
	}
}

//testWasmControlFlow
void wasm_todos(){
	assert_equals(emit("42.1"), 42.1) // main returns int, should be pointer to value!
}
void testAllWasm(){
	skip(testsFailingInWasm();)
	// constant things may be evaluated by compiler!
	// todo: reuse all tests via
//	interpret = false;
//	assert_equals(emit("x*=14"), 1)
//	assert_equals(emit("x=15;x>=14"), 1)
	assert_emit("true or false and false", true);// == true or (false)

	testComparisonPrimitives();
//	testWasmControlFlow();
	testWasmLogicUnary();
	testWasmLogic();
	testWasmLogicPrimitives();
	testMathOperators();
	testMathPrimitives();
	testConstReturn();
	skip(
			wasm_todos();
			testWasmLogicOnObjects();
	)
}