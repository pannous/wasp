#include "Angle.h" // emit
#include "Wasp.h"
#include "wasm_reader.h"
#include "wasm_merger.h"

#define assert_throws(αα)  {printf("%s\n%s:%d\n",αα,__FILE__,__LINE__);bool old=panicking;try{ \
panicking=false;emit(αα);printf("SHOULD HAVE THROWN!\n%s\n",αα);backtrace_line(); \
}catch(chars){}catch(String*){}catch(...){};panicking=old;}

#define assert_emit(α, β) printf("%s\n%s:%d\n",α,__FILE__,__LINE__);if (!assert_equals_x(emit(α),β)){printf("%s != %s",#α,#β);backtrace_line();}
//#define assert_emit(α, β) try{printf("%s\n%s:%d\n",α,__FILE__,__LINE__);if (!assert_equals_x(emit(α),β)){printf("%s != %s",#α,#β);backtrace_line();}}catch(chars x){printf("%s\nIN %s",x,α);backtrace_line();}

#ifndef RUNTIME_ONLY
// use assert_emit if runtime is not needed!! much easier to debug
#define assert_run(mark, result) printf("\n%s:%d\n", __FILE__, __LINE__);check_eq(runtime_emit(mark), result);
#else
#define assert_run(a, b) skip(a)
#endif


void testWasmStuff();


void testEmitter() {
#ifndef RUNTIME_ONLY
	clearContext();
	Node node = Node(42);
	Code &code = emit(node);
	int result = code.run();
	check(result == 42);
#endif
}


void testGlobals() {
	assert_emit("π", 3);
}

void test_get_local() {
	assert_emit("add1 x:=$0+1;add1 3", (long) 4);
}

void testWasmFunctionDefiniton() {
//	assert_is("add1 x:=x+1;add1 3", (long) 4);

	assert_emit("add1 x:=x+1;add1 3", (long) 4);
	assert_emit("add2 x:=x+2;add2 3", (long) 5);
	skip(
			assert_emit("expression_as_return:=y=9;expression_as_return", (long) 9);
			assert_emit("addy x:= y=2 ; x+y ; addy 3", (long) 5);
	)

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


void testWasmTernary() {
	assert_emit("2>1?3:4", 3);
	assert_emit("1>0?3:4", 3);
	assert_emit("2<1?3:4", 4);
	assert_emit("1<0?3:4", 4);
	skip(
			assert_emit("fac:= it<=0 ? 1 : it * fac it-1; fac(5)", 5 * 4 * 3 * 2 * 1);
	// What seems to be the problem?
	)
}

void testLazyEvaluation() {
//	if lazy_operators.has(op) and … not numeric? …
	//	if op==or emitIf(not lhs,then:rhs)
	//	if op==or emitIf(lhs,else:rhs)
	//	if op==and emitIf(lhs,then:rhs)
	assert_emit("fac:= it<=0 or it * fac it-1; fac(5)", 5 * 4 * 3 * 2 * 1);// requires lazy evaluation
}

void testWasmFunctionCalls() {
	assert_emit("logs 'ok'", (long) 0);
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

	assert_emit("x=3;x*=3", 9)
	assert_emit("'hello';(1 2 3 4);10", 10);
	data_mode = false;
	assert_emit("i=ø; not i", true);
	assert_emit("0.0", (long) 0);// can't emit float yet
	assert_emit(("x=15;x>=14"), 1)
	assert_emit("i=1.0;i", 1.0);// works first time but not later in code :(
	assert_emit("i=0.0;i", 0.0);//
	assert_is("3*-1", -3);
	assert_emit("3*-1", -3);

	assert_emit("maxi=3840*2160", 3840 * 2160);
	assert_emit("maxi=3840*2160;maxi", 3840 * 2160);
	assert_emit("blue=255;green=256*255;", 256 * 255);
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
	assert_emit("i=123.4;i", 123);// main returning int
	assert_emit("i=1.0;i", 1.0);
	assert_emit("i=3;i", 3);
	assert_emit("i=1.0;i", 1.0);

	assert_emit(("2.1<=3.0"), true)

	skip(
			assert_emit("i=8;i=i/2;i", 4);// make sure i stays a-float
			assert_emit("i=1.0;i=3;i=i/2;i=i*4", 6.0);// make sure i stays a-float
			"BUG IN WASM?? should work!?"
			assert_emit(("3.1>=3.0"), true)
	)

	assert_emit(("3.0+3.0*3.0>3.0+3.0+3.0"), true)
	assert_emit(("3.0+3.0*3.0<3.0*3.0*3.0"), true)
	assert_emit(("3.0+3.0*3.0<3.0+3.0+3.0"), false)
	assert_emit(("3.0+3.0*3.0>3.0*3.0*3.0"), false) // 0x1.8p+1 == 3.0
	assert_emit(("3.0+3.0+3.0<3.0+3.0*3.0"), true)
	assert_emit(("3.0*3.0*3.0>3.0+3.0*3.0"), true)
}

void testNorm() {

	assert_emit("‖-3‖", 3);
	assert_emit("‖3‖-1", 2);
	assert_emit("‖-3‖/3", 1);
	assert_emit("‖-3‖/-3", -1);
	assert_emit("‖3‖/-3", -1);
	assert_emit("-‖3‖/-3", 1);
	assert_emit("-‖-3‖/3", -1);
	assert_emit("-‖-3‖/-3", 1);
	skip( // todo : real BUG:
			assert_emit("1-‖3‖/-3", 2);
			assert_emit("1-‖-3‖/3", 0);
			assert_emit("1-‖-3‖/-3", 2);
			assert_emit("1-‖-3‖-1", -3);
			assert_emit("√9*-‖-3‖/3", -3);
			assert_emit("√9*‖-3‖/-3", -3);
			assert_emit("√9*-‖-3‖/-3", 3);
	)

}

void testMathOperators() {
//	assert_emit(("42 2 *"), 84)
	assert_emit("- -3", 3);
	assert_emit("1 - 3 - square 3+4", (long) -51);// OK!
	assert_emit("1- -3", 4);
	assert_emit("1 - -3", 4);
	skip(
			assert_emit("1 - - 3", 4);// -1 uh ok?
	)

	assert_throws("1--3");// should throw, variable missed by parser! 1 OK'ish
	//	assert_emit("1--3", 4);// should throw, variable missed by parser! 1 OK'ish

	assert_emit("‖-3‖", 3);
	assert_emit("-‖-3‖", -3);
	assert_emit("‖-3‖+1", 4);
#ifndef WASMTIME
	assert_emit("3^2", 9);
	assert_emit("3^1", 3);
	assert_emit("√3^2", 3);
	assert_emit("√3^0", 1);
	assert_emit(("42^2"), 1764);// NO SUCH PRIMITIVE

#endif
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
	assert_emit("fib:=if it<2 then it else fib(it-1)+fib(it-2);fib(7)", 13)
	assert_emit("fac:= if it<=0 : 1 else it * fac it-1; fac(5)", 5 * 4 * 3 * 2 * 1);


	assert_emit("i=3;i*-1", -3);
	assert_is("3*-1", -3);
	assert_emit("3*-1", -3);
	assert_emit("-√9", -3);

	assert_emit("i=3.70001;.3+i", 4);

	assert_emit("i=3.71;.3+i", 4);
	assert_emit("i=3.7;.3+i", 4);
	assert_is("4-1", 3);//

	assert_run("x=123;x + 4 is 127", true);
	assert_emit("i=3;i++", 4);
	assert_emit("i=1;while i<9:i++;i+1", 10);
	assert_emit("ceil 3.7", 4);
	assert_emit("- √9", -3);
	assert_emit("i=-9;-i", 9);
	assert_emit("√ π ²", 3);
	assert_emit(("3²"), 9);
	skip(
			assert_emit(("3⁰"), 1);// get UNITY of set (1->e auto cast ok?)
			assert_emit(("3¹"), 3);
			assert_emit(("3³"), 27);// define inside wasp!
			assert_emit(("3⁴"), 9 * 9);
	)

	assert_emit("i=3.70001;.3+i", 4);
	assert_emit("i=3.7;.3+i", 4);

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
	assert_emit("452*3==id(3*452)", 1)
	assert_emit("452*3==id 3*452", 1)
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
	skip(
//	Ambiguous mixing of functions `ƒ 1 + ƒ 1 ` can be read as `ƒ(1 + ƒ 1)` or `ƒ(1) + ƒ 1`
			assert_emit("id 3*42 > id 2*3", 1)
			assert_emit("id 3*1< id 2*3", 1)
	)
	assert_emit("id(3*42)> id 2*3", 1)
	assert_emit("id(3*1)< id 2*3", 1)
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


void testWasmVariables0() {
//	  (func $i (type 0) (result i32)  i32.const 123 return)  NO LOL
//	assert_emit("i=123;i", 123);
	assert_emit("i:=123;i+1", 124);
	assert_emit("i=123;i+1", 124);

	assert_emit("i=123;i", 123);
	assert_emit("i=1;i", 1);
	assert_emit("i=false;i", false);
	assert_emit("i=true;i", true);
	assert_emit("i=0;i", 0);
	assert_emit("i:=true;i", true);
	assert_emit("i=true;i", true);
	assert_emit("i=123.4;i", 123);// main returning int
	skip(
			assert_emit("i=0.0;i", 0.0);
			assert_emit("i=ø;i", nullptr);
			assert_emit("i=123.4;i", 123.4);// main returning int
	)
}

void testWasmIncrement() {
	assert_emit("i=2;i++", 3);
	skip(
			assert_emit("i=0;w=800;h=800;pixel=(1 2 3);while(i++ < w*h){pixel[i]=i%2 };i ", 800 * 800);
//				assert_error("i:=123;i++", "i is a closure, can't be incremented");
	)
}

void testWasmLogicUnaryVariables() {
	assert_emit("i=0.0; not i", true);
	assert_emit("i=false; not i", true);
	assert_emit("i=0; not i", true);
	data_mode = false;//todo remove crutch
	assert_emit("i=true; not i", false);
	assert_emit("i=ø; not i", true);
	data_mode = true;
	assert_emit("i=1; not i", false);
	assert_emit("i=123; not i", false);
}

void testSelfModifying() {
	assert_emit("i=3;i*=3", (long) 9);
	assert_emit("i=3;i+=3", (long) 6);
	assert_emit("i=3;i-=3", (long) 0);
	assert_emit("i=3;i/=3", (long) 1);
	//	assert_emit("i=3;i√=3", (long) ∛3);
	skip(
			assert_emit("i=3^1;i^=3", (long) 27);
			assert_throws("i*=3");// well:
			assert_emit("i*=3", (long) 0);
	)
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
	assert_run("not 'a'", false);
	assert_run("not {a:2}", false);
	skip(
			assert_run("not {a:0}", false);// maybe
	)

	assert_run("not ()", true);
	assert_run("not {}", true);
	assert_run("not []", true);
	assert_run("not ({[ø]})", true); // might skip :)

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


	assert_emit("¬ 1", 0);
	assert_emit("¬ 0", 1);

	assert_emit("0 ⋁ 0", 0);
	assert_emit("0 ⋁ 1", 1);
	assert_emit("1 ⋁ 0", 1);
	assert_emit("1 ⋁ 1", 1);

	assert_emit("0 ⊻ 0", 0);
	assert_emit("0 ⊻ 1", 1);
	assert_emit("1 ⊻ 0", 1);
	assert_emit("1 ⊻ 1", 0);

	assert_emit("1 ∧ 1", 1);
	assert_emit("1 ∧ 0", 0);
	assert_emit("0 ∧ 1", 0);
	assert_emit("0 ∧ 0", 0);

	assert_emit("1 ⋁ 1 ∧ 0", 1);
	assert_emit("1 ⋁ 0 ∧ 1", 1);
	assert_emit("1 ⋁ 0 ∧ 0", 1);
	assert_emit("0 ⋁ 1 ∧ 0", 0);
	assert_emit("0 ⋁ 0 ∧ 1", 0);
	assert_emit("¬ (0 ⋁ 0 ∧ 1)", 1);
}

void testWasmLogicNegated() {
	assert_emit("not true and not true", not true);
	assert_emit("not true and not false", not true);
	assert_emit("not false and not true", not true);
	assert_emit("not false and not false", not false);
	assert_emit("not false or not true and not true", not false);// == not false or (not true)

	assert_emit("not true xor not false", not false);
	assert_emit("not false xor not true", not false);
	assert_emit("not true xor not true", not true);
	assert_emit("not false xor not false", not true);
	assert_emit("not true or not false", not false);
	assert_emit("not true or not true", not true);
	assert_emit("not false or not true", not false);
	assert_emit("not false or not false", not false);
}

void testWasmLogicCombined() {
	assert_emit("3<1 and 3<1", 3 < 1);
	assert_emit("3<1 and 9>8", 3 < 1);
	assert_emit("9>8 and 3<1", 3 < 1);
	assert_emit("9>8 and 9>8", 9 > 8);
	assert_emit("9>8 or 3<1 and 3<1", 9 > 8);// == 9>8 or (3<1)

	assert_emit("3<1 xor 9>8", 9 > 8);
	assert_emit("9>8 xor 3<1", 9 > 8);
	assert_emit("3<1 xor 3<1", 3 < 1);
	assert_emit("9>8 xor 9>8", 3 < 1);
	assert_emit("3<1 or 9>8", 9 > 8);
	assert_emit("3<1 or 3<1", 3 < 1);
	assert_emit("9>8 or 3<1", 9 > 8);
	assert_emit("9>8 or 9>8", 9 > 8);
}

void testWasmIf() {
	assert_emit("if 2 : 3 else 4", 3);
	assert_emit("if 2 then 3 else 4", 3);
	skip(
			assert_emit("if(2){3}{4}", 3);
			assert_emit("if({2},{3},{4})", 3);
			assert_emit("if(2,3,4)", 3); // bad border case EXC_BAD_ACCESS because not anayized!
			assert_emit("if(condition=2,then=3)", 3);
			assert_emit("if(condition=2,then=3,else=4)", 3); // this is what happens under the hood (?)
			assert_emit("fib:=it<2 then 1 else fib(it-1)+fib(it-2);fib(4)", 5)
			assert_emit("fib:=it<2 and it or fib(it-1)+fib(it-2);fib(7)", 13)
			assert_emit("fib:=it<2 then it or fib(it-1)+fib(it-2);fib(7)", 13)
			assert_emit("fib:=it<2 or fib(it-1)+fib(it-2);fib(4)", 5)
	)
}

void testWasmWhile() {
	assert_emit("i=1;while i<9:i++;i+1", 10);
	assert_emit("i=1;while(i<9){i++};i+1", 10);
	assert_emit("i=1;while(i<9 and i > -10){i+=2;i--};i+1", 10);
	skip(
			assert_emit("i=1;while(i<9)i++;i+1", 10);// needs valueNode conceptual overhaul
	)
}


void testWasmMemoryIntegrity() {
	return;
#ifndef WASM
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
	long end = MEMORY_SIZE / 4; // /4 because 1 int = 4 bytes
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
		memory[i] = tmp;// else test fail
	}
}

void testOldRandomBugs() {
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

//	assert_emit("1 -3 - square 3+4", (long) -51);
	assert_emit("1+2 + square 3+4", (long) 52);

	assert_emit("4*5 + square 2*3", (long) 56);
	//	assert_emit("id 3*42> id 2*3", 1)
	assert_emit("x:=41;if x>1 then 2 else 3", 2)
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


void testWasmRuntimeExtensionMock() {
#ifndef RUNTIME_ONLY
	functionIndices.setDefault(-1);
	Module runtime = read_wasm("lib.wasm");// test:=42
	Signature mock;// todo read Signature from wasm!?
	functionSignatures.insert_or_assign("test", mock.returns(int32));
	Node charged = analyze(parse("test"));
	Code calling = emit(charged, &runtime, "maine");
	calling.save("main.wasm");// partial wasm!
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
	printf("testWasmModuleExtension");
#ifndef RUNTIME_ONLY
	functionSignatures.clear();
//	memoryHandling=0;
	Node charged = analyze(parse("test:=42"));
	breakpoint_helper
//	Code lib = emit(charged, 0, nil);// no main
	Code lib = emit(charged, 0, 0);// "lib_main");
	lib.save("lib.wasm");

	Module module = read_wasm("lib.wasm");
	declaredFunctions.clear();// <-- only newly declared functions (that nead a Code block later), others via functionIndex …
	charged = analyze(parse("test"));// call test() from lib
	Code main = emit(charged, &module, "main");
//	int ok1 = main.run();// todo: why not merge_wasm on emit? module data is all there? yeah but not in parsed Code … form
//	check(ok1==42);
	main.save("main.wasm");// this is NOT a valid wasm module, because all the indices are offset to the lib!

	// we do NOT wan't to add 10000 imports here, so that the indices match, do we?
	functionSignatures.clear();
	Module prog = read_wasm("main.wasm");
	Code merged = merge_wasm(module, prog);
	merged.save("merged.wasm");
	read_wasm("merged.wasm");
	int ok = merged.run();// why is wabt so SLOOOOW now??
//	int ok = main.run();
//  WASM module load failed: multiple memories  in w.m.r.
	assert_equals(ok, 42);
#endif
}


void testWasmRuntimeExtension() {
	assert_run("x=123;x + 4 is 127", true);
	assert_run("atoi0('123'+'456')", 123456);
	assert_run("'123' is '123'", true);
	assert_emit("x:43", 43);
	assert_run("x:43", 43);
	assert_run("ok+1", 43);

//	functionSignatures["ok"].returns(int32);
//	assert_emit("x='123';x + '4' is '1234'", true);// unknown function concat: needs runtime
	assert_run("'123' + '4' is '1234'", true);// ok
//	assert_run("x='123';x + '4' is '1234'", true);// not ok
	assert_run("ok+1", 43);
	assert_run("oki(1)", 43);
//	assert_run("not_ok",-1);// error

//	functionSignatures["okf"].returns(float32);
	assert_run("okf(1)", 43);
	assert_run("okf(1.0)", 43);
	assert_run("42.5", 42);// truncation ≠ proper rounding!
	assert_run("okf5(1.5)", 43);
	functionSignatures["atoi0"].returns(int32);
//	assert_run("printf('123')", 123);
	assert_run("strlen0('123')", 3);
	assert_run("atoi0('123')", 123);
	assert_run("atoi0('123000')+atoi0('456')", 123456);
	assert_run("atoi0('123'+'456')", 123456);
	// works with ./wasp but breaks in webapp
//	assert_run("x=123;x + 4 is 127", true);
	// works with ./wasp but breaks now:

	//	assert_run("okf(1)", 43);
	//	assert_run("43", 43);
	//	assert_run("logs 'hello' 'world'", "hello world");
	//	assert_run("hello world", "hello world");// unresolved symbol printed as is

	//	assert_run("'123'='123'", true);// parsed as keyNode a:b !?!? todo!
	//	assert_run("'123' = '123'", true);
	assert_run("ok+1", 43);
	assert_run("'123' == '123'", true);
	assert_run("'123' is '123'", true);
	assert_run("'123' equals '123'", true);
	assert_run("x='123';x is '123'", true);
	//	assert_run("string('123') equals '123'", true); // string() makes no sense in angle:
	//	assert_run("'123' equals string('123')", true);//  it is internally already a string whenever needed
	//	assert_run("atoi0(str('123'))", 123);
	//	assert_run("atoi0(string('123'))", 123);

	//	assert_run("oki(1)", 43);
	//	assert_emit("logs('123'+'456');", 123456);// via import not via wasp!
	//assert_emit("double := it * 2 ; double(4)", 8)
	//	check(Valtype::charp!=Valtype::pointer)

	skip(
			assert_run("atoi0('123')", 123);
			assert_run("'123'", 123);// result printed and parsed?
			assert_run("printf('123')", 123);// result printed and parsed?
	)
	skip( // if not compiled as RUNTIME_ONLY library:
			check(functionSignatures.has("tests"))
			assert_run("tests", 42);
	)
}


void testMergeRelocate() {
#ifndef RUNTIME_ONLY
	// doesn't work: cannot insert imports or function types!
//	emit("test");
//	merge_files({"test.wasm", "test-lld-wasm/lib.wasm"});
	Module lib = read_wasm("test-lld-wasm/lib.wasm");
	Module main = read_wasm("test-lld-wasm/main.wasm");
//	Module main=read_wasm("test.wasm");
	Code merged = merge_wasm(lib, main);
	merged.save("merged.wasm");
	Module merged1 = read_wasm("merged.wasm");
	merged.run();
//	wabt::Module *merged=merge_wasm(lib, main);
//	save_wasm(merged, "prog.wasm");
#endif
}


void testStringIndicesWasm() {
	assert_emit("'abcde'#4", 'd');//
	assert_emit("x='abcde';x#4", 'd');//
	assert_emit("x='abcde';x#4='x';x#4", 'x');

	assert_emit("x='abcde';x#4='x';x#4", 'x');
	assert_emit("x='abcde';x#4='x';x#5", 'e');

	assert_emit("x='abcde';x#4='x';x[3]", 'x');
	assert_emit("x='abcde';x#4='x';x[4]", 'e');
	assert_emit("i=0;x='abcde';x#4='x';x[4]", 'e');

	assert_emit("'hello';(1 2 3 4);10", 10);// -> data array […;…;10] ≠ 10
	assert_run("x='123';x=='123'", true);// ok
	assert_run("x='123';x is '123'", true);// ok
	assert_run("'hello';(1 2 3 4);10", 10);// -> data array […;…;10] ≠ 10
	assert_run("'123' + '4' is '1234'", true);// ok
	assert_run("x='123';x + '4' is '1234'", true);// ok

//	assert_emit("'world'[1]", 'o');
	assert_emit("'world'#1", 'w');
	assert_emit("'world'#2", 'o');
	assert_emit("'world'#3", 'r');
	skip( // todo move angle syntax to test_angle
			assert_emit("char #1 in 'world'", 'o');
			assert_emit("char 1 in 'world'", 'o');
			assert_emit("2nd char in 'world'", 'o');
			assert_emit("2nd byte in 'world'", 'o');
			assert_emit("'world'#-1", 'd');
	)

	assert_emit("hello='world';hello#1", 'w');
	assert_emit("hello='world';hello#2", 'o');
	//	assert_emit("pixel=100 int(s);pixel#1=15;pixel#1", 15);
	skip(
			assert_emit("hello='world';hello#1='W';hello#1", 'W');// diadic ternary operator
			assert_emit("hello='world';hello[0]='W';hello[0]", 'W');// diadic ternary operator
	)
	//	assert_emit("hello='world';hello#1='W';hello", "World");
	//	exit(0);
}

void testObjectPropertiesWasm() {
	assert_emit("x={a:3,b:4,c:{d:true}};x.a", 3);
	assert_emit("x={a:3,b:true};x.b", 1);
	assert_emit("x={a:3,b:4,c:{d:true}};x.c.d", 1);
	//assert_emit("x={a:3,b:'ok',c:{d:true}};x.b", "ok");
	assert_emit("x={a:3,b:'ok',c:{d:5}};x.c.d", 5);//deep
}

void testArrayIndicesWasm() {
#ifndef WEBAPP
	assert_throws("surface=(1,2,3);i=1;k#i=4;k#i")// no such k!
//	caught in wrong place?
#endif

//	testArrayIndices(); //	check node based (non-primitive) interpretation first
	data_mode = true;// todo remove hack
	assert_emit("x={1 2 3}; x#2=4;x#2", 4);
	assert_emit("logs('ok');", 0);
	assert_emit("logs('ok');(1 4 3)#2", 4);
	assert_emit("{1 4 3}#2", 4);

	assert_emit("x={1 4 3};x#2", 4);
	assert_emit("logs('ok');(1 4 3)#2", 4);
	assert_emit("{1 4 3}[1]", 4);
	assert_emit("logs('ok');(1 4 3)#2", 4);
	assert_emit("(1 4 3)[1]", 4);
	assert_emit("logs('ok');(1 4 3)#2", 4);
	assert_throws("(1 4 3)#0");
	skip(
	// todo patterns as lists
			assert_emit("[1 4 3]#2", 4);
			assert_is("[1 2 3]#2", 2);// check node based (non-primitive) interpretation first
			assert_throws("(1 4 3)#4", 4);// todo THROW!
	)


	//	Node empty_array = parse("pixel=[]");
	//	check(empty_array.kind==patterns);
	//
	//	Node construct = analyze(parse("pixel=[]"));
	//	check(construct["rhs"].kind == patterns or construct.length==1 and construct.first().kind==patterns);
	//	emit("pixel=[]");
	//	exit(0);


	skip(
	// todo create empty array
			assert_emit("pixel=[];pixel[1]=15;pixel[1]", 15);
			assert_emit("pixel=();pixel#1=15;pixel#1", 15);// diadic ternary operator
			assert_emit("pixel array;pixel#1=15;pixel#1", 15);// diadic ternary operator
			assert_emit("pixel=100 ints;pixel[1]=15;pixel[1]", 15);
	)


}


// random stuff todo: put in proper tests
void testWasmStuff() {
	assert_emit("double x := x * 2 ; double(4)", 8)
//	assert_emit("double := it * 2 ; double(4)", 8)
	assert_emit("-42", -42)
	assert_emit("x=41;x+1", 42)
	assert_emit("x=40;y=2;x+y", 42)
	assert_emit("id(4*42) > id 2+3", 1)
	assert_emit("double := it * 2 ; double(4)", 8)
	assert_emit("double:=it*2; double 3", 6)
	assert_emit("fib x:=if x<2 then x else fib(x-1)+fib(x-2);fib(7)", 13)
	assert_emit("fib x:=if x<2 then x else{fib(x-1)+fib(x-2)};fib(7)", 13)
	assert_emit("add1 x:=x+1;add1 3", (long) 4);
}


void testRecentRandomBugs() {
	assert_emit("√π²", 3);
	assert_emit("i=-9;√-i", 3);
	assert_run("x=123;x + 4 is 127", true);
#ifndef WASMTIME
	assert_emit("n=3;2ⁿ", 8);
	//	function attempted to return an incompatible value WHAT DO YOU MEAN!?
#endif

	assert_emit("logs('ok');(1 4 3)#2", 4);
	assert_emit("‖-3‖", 3);
	assert_emit("√100²", 100);
	assert_emit("logs('ok');", 0);
// move to tests() once OK'

	assert_parses("{ç:☺}");
	assert(result["ç"] == "☺");

	skip(
			assert_emit("i=ø; not i", true);
	)
}


void testSquareExpWasm() {
	let π = 3;//.141592653589793;
	// todo smart pointer return from main for floats!
	assert_emit("3²", 9);
	assert_emit("3.0²", 9);
	assert_emit("√100²", 100);
	assert_emit("√ π ²", π);
	assert_emit("√π ²", π);
	assert_emit("√ π²", π);
	assert_emit("√π²", π);
	skip(
			assert_emit("π²", 9.869604401089358 /*π*π*/);
	)
	assert_emit("π", 3/*.1415926535897*/);
	assert_emit("π*1000000.", 3141592/*6535897*/);
	assert_emit("π ²", 9/*.869604401089358 /*π*π*/);
	assert_emit("π*1000000", 3141592/*6535897*/);
	assert_emit("i=-9;-i", 9);
	assert_emit("- √9", -3);
	assert_emit(".1 + .9", 1);
	assert_emit("-.1 + -.9", -1);
	assert_emit("√9", 3);
	//	assert_emit("√-9 is -3i", -3);// if «use complex numbers»
	assert_emit(".1", 0);
#ifndef WASMTIME
	skip(
			assert_emit("i=-9;√-i", 3);
	)
	assert_emit("n=3;2ⁿ", 8);
	assert_emit("n=3.0;2.0ⁿ", 8);
	//	function attempted to return an incompatible value WHAT DO YOU MEAN!?
#endif
}

void testRoundFloorCeiling() {
	assert_emit("ceil 3.7", 4);
	assert_emit("floor 3.7", 3);// todo: only if «use math» namespace
//	assert_emit("ceiling 3.7", 4);// todo: only if «use math» namespace
	assert_emit("round 3.7", 4);
//	assert_emit("i=3.7;.3+i", 4);// floor
	// lol "⌊3.7⌋" is cursed and is transformed into \n\t or something in wasm and IDE!
	//	assert_emit("⌊3.7", 3);// floor
	//	assert_emit("⌊3.7⌋", 3);// floor
	//	assert_emit("3.7⌋", 3);// floor
	//	//assert_emit("i=3.7;.3 + ⌊i", 3);// floor
	//	//assert_emit("i=3.7;.3+⌊i⌋", 3);// floor
	//	assert_emit("i=3.7;.3+i⌋", 3);// floor
	//	assert_emit("i=3.7;.3+ floor i", 3);// floor
}


//testWasmControlFlow
void wasm_todos() {
	skip(
			assert_emit("i=0;w=800;h=800;pixel=(1 2 3);while(i++ < w*h){pixel[i]=i%2 };i ", 800 * 800);

			assert_emit(("42.1"), 42.1) // main returns int, should be pointer to value!

			//			Ambiguous mixing of functions `ƒ 1 + ƒ 1 ` can be read as `ƒ(1 + ƒ 1)` or `ƒ(1) + ƒ 1`
			assert_emit("id 3*42 > id 2*3", 1)
			assert_emit("square 3*42 > square 2*3", 1)
			assert_emit("double:=it*2; double 3*42 > double 2*3", 1)
	// is there a situation where a COMPARISON is ambivalent?
	// sleep ( time > 8pm ) and shower ≠ sleep time > ( 8pm and true)
	)
}

void testWasmMutableGlobal() {
//	assert_emit("$k=7",7);// ruby style, conflicts with templates `hi $name`
	assert_emit("k::=7", 7);// global variable not visually marked as global, not as good as:
	assert_emit("global k=7", 7);// python style, as always the best
	assert_emit("global.k=7", 7);//  currently all globals are exported
	assert_emit("global k:=7", 7);//  global or function?
	assert_emit("export k=7", 7);//  all exports are globals, naturally.
	assert_emit("export k=7", 7);//  all exports are globals, naturally.
	assert_emit("export f:=7", 7);//  exports can be functions too.
	assert_emit("global export k=7", 7);//  todo warn("redundant keyword global: all exports are globals")
	assert_emit("global int k=7", 7);// python style, as always the best
	assert_emit("global int k:=7", 7);//  global or function?
	assert_emit("export int k=7", 7);//  all exports are globals, naturally.
	assert_emit("export int k=7", 7);//  all exports are globals, naturally.
	assert_emit("export int f:=7", 7);//  exports can be functions too.
	assert_emit("global int k", 0);// todo error without init value?
	assert_emit("export int k", 0);//

	assert_emit("import int k", 7);//  all imports are globals, naturally.
	assert_emit("import const int k", 7);//  all imports are globals, naturally.
	assert_emit("import mutable int k", 7);//  all imports are globals, naturally.

	assert_emit("import int k=7", 7);//  import with initializer
	assert_emit("import const int k=7", 7);//  import with initializer
	assert_emit("import mutable int k=7", 7);//  import with initializer

	assert_emit("import int k=7.1", 7);//  import with cast initializer
	assert_emit("import const int k=7.1", 7);//  import with cast initializer
	assert_emit("import mutable int k=7.1", 7);//  import with cast initializer

	assert_emit("import k=7", 7);//  import with inferred type
	assert_emit("import const k=7", 7);//  import with inferred type
	assert_emit("import mutable k=7", 7);//  import with inferred type

	assert_emit("global int k", 7);//   all globals without value are imports??
	assert_emit("global const int k", 7);//   all globals without value are imports??
	assert_emit("global mutable int k", 7);//   all globals without value are imports??
	assert_emit("global mut int k", 7);//   all globals without value are imports??
	// remember that the concepts of functions and properties shall be IDENTICAL to the USER!
	// this does not impede the above, as global exports are not properties, but something to keep in mind
}

void testCustomOperators() {
	assert_emit(("suffix operator ⁰ := 1; 3⁰"), 1);// get UNITY of set (1->e auto cast ok?)
	assert_emit(("suffix ⁰ := 1; 3⁰"), 1);// get UNITY of set (1->e auto cast ok?)
	assert_emit(("suffix operator ³ := it*it*it; 3³"), 27);// define inside wasp!
	assert_emit(("suffix operator ³ := it*it*it; .5³"), 1 / 8);
	assert_emit(("suffix ³ := it*it*it; 3³"), 27);// define inside wasp!

//	assert_emit(("alias to the third = ³"),1);
//	assert_emit(("3⁴"),9*9);
}

void testIndexWasm() {
	assert_emit("i=1;k='hi';k#i", 'h'); // BUT IT WORKS BEFORE!?!
	assert_emit("i=1;k='hi';k[i]", 'i')
	//	assert_throws("i=0;k='hi';k#i")// todo internal boundary checks? nah, later ;) done by VM:
	// WASM3 error: [trap] out of bounds memory accessmemory size: 65536; access offset: 4294967295
	assert_emit("k='hi';k#1=97;k#1", 'a')
	assert_emit("k='hi';k#1='a';k#1", 'a')
	assert_emit("k='hi';i=1;k#i=97;k#i", 'a')
	assert_emit("k=(1,2,3);i=1;k#i=4;k#i", 4)
	assert_emit("k=(1,2,3);i=1;k#i=4;k#1", 4)

	assert_emit("k='hi';k#1=65;k#2", 'i')
	assert_emit("k=(1,2,3);i=1;k#i=4;k#i", 4)
	assert_emit("i=2;k='hio';k#i", 'i')
}


void testImportWasm() {
//	Code fourty_two=emit(analyze(parse("ft=42")));
//	fourty_two.save("fourty_two.wasm");
	assert_emit("import fourty_two;ft*2", 42 * 2);
	assert_emit("import fourty_two", 42);
	assert_emit("include fourty_two", 42);
	assert_emit("require fourty_two", 42);
	assert_emit("include fourty_two;ft*2", 42 * 2);
	assert_emit("require fourty_two;ft*2", 42 * 2);
}

void testMathLibrary() {
	assert_emit("i=-9;√-i", 3);
	assert_emit("i=-9;√ -i", 3);
//		assert_emit("use math;√π²", 3);
}

void testAllWasm() {

//	data_mode = false;
	testWasmMemoryIntegrity();
#ifdef RUNTIME_ONLY
	logs("RUNTIME_ONLY");
	logs("NO WASM emission...");
//	return;
#endif
	skip(
	//	assert_run not compatible with Wasmer, don't ask why, we don't know;)
			testCustomOperators();
			testWasmMutableGlobal();
			testIndexWasm();// breaks on second run WHY?
	)
	testEmitter();
	testMathLibrary();
	testArrayIndicesWasm();
	testStringIndicesWasm();
	testWasmFunctionDefiniton();
	testSquareExpWasm();
	testRoundFloorCeiling();
	testWasmLogicCombined();
	testGlobals();
	wasm_todos();
	testWasmWhile();
	skip(
			testCustomOperators();
			testWasmLogicOnObjects();
			testObjectPropertiesWasm();
	)

//	exit(21);
	testWasmIncrement();
// TRUE TESTS:
	testComparisonIdPrecedence();
	testRecentRandomBugs();
	testOldRandomBugs();
	testWasmStuff();
	testWasmFunctionDefiniton();
	test_get_local();
	testWasmFunctionCalls();
	testFloatOperators();
	testWasmLogicUnary();
	testWasmLogicUnaryVariables();
	testWasmLogic();
	testWasmLogicNegated();
	testWasmLogicPrimitives();
	testConstReturn();
	testWasmIf();
	testMathOperators();
	testMathPrimitives();
	testSelfModifying();
	testNorm();
	testComparisonPrimitives();
	testComparisonMath();
	testComparisonId();
	testWasmVariables0();
	testWasmVariables0();
	testSquareExpWasm();
	testRoundFloorCeiling();
	wasm_todos();
	testWasmTernary();
	testWasmRuntimeExtension();
	skip(
			testWasmModuleExtension();// multiple memories, egal, runtimeExtension works
			testWasmRuntimeExtensionMock();
	)
//	data_mode = true;// allow
}
