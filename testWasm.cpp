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
	assert_equals(emit("-42.1"), 42.1) // may be evaluated by compiler!
	)
	assert_equals(emit("42"), 42) // may be evaluated by compiler!
	assert_equals(emit("-42"), -42) // may be evaluated by compiler!
	assert_equals(emit("2000000000"), 2000000000) // may be evaluated by compiler!
	assert_equals(emit("-2000000000"), -2000000000) // may be evaluated by compiler!
}

void testMathOperators(){
//	assert_equals(emit("42 2 *"), 84) // may be evaluated by compiler!
	assert_equals(eval("42/2"), 21) // may be evaluated by compiler!
	assert_equals(emit("42/2"), 21) // may be evaluated by compiler!
	assert_equals(emit("42*2"), 84) // may be evaluated by compiler!
	assert_equals(emit("42+2"), 44) // may be evaluated by compiler!
	assert_equals(emit("42-2"), 40) // may be evaluated by compiler!
	skip(
	assert_equals(emit("42^2"), 1764) // may be evaluated by compiler! NO SUCH PRIMITIVE
			)
}


void testComparisonPrimitives() {
	assert_equals(emit("42>2"), 1) // may be evaluated by compiler!
	assert_equals(emit("1<2"), 1) // may be evaluated by compiler!
	assert_equals(emit("452==452"), 1) // may be evaluated by compiler!
	assert_equals(emit("13!=14"), 1) // may be evaluated by compiler!
	assert_equals(emit("13<=14"), 1) // may be evaluated by compiler!
	assert_equals(emit("15>=14"), 1) // may be evaluated by compiler!
	assert_equals(emit("42<2"), False);
	assert_equals(emit("1>2"), False);
	assert_equals(emit("452!=452"), False);
	assert_equals(emit("13==14"), False);
	assert_equals(emit("13>=14"), False);
	assert_equals(emit("15<=14"), False);
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

	assert_emit("false xor true", true);
	assert_emit("true xor false", true);
	assert_emit("false xor false", false);
	assert_emit("true xor true", false);
	assert_emit("false or true", true);
	assert_emit("false or false", false);
	assert_emit("true or false", true);
	assert_emit("true or true", true);
}
void wasm_todos(){
	assert_equals(emit("42.1"), 42.1) // main returns int, should be pointer to value!
}
void testAllWasm(){
	// todo: reuse all tests via
//	interpret = false;
	testWasmLogicUnary();
	skip(
	testWasmLogicOnObjects();
			)
	testWasmLogic();
	testWasmLogicPrimitives();
	testMathOperators();
	testMathPrimitives();
	testConstReturn();
}