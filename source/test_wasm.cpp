#include "Angle.h" // emit
#include "Wasp.h"
#include "wasm_reader.h"
#include "wasm_helpers.h"
#if INCLUDE_MERGER

#include "wasm_merger.h"

#endif

#include "wasm_emitter.h"

#import "asserts.h"

#if WASM
#define assert_throws(αα)
#else
#define assert_throws(αα)  {print(#αα);debug_line();bool old=panicking;try{ \
panicking=false;throwing=true;eval(αα);printf("SHOULD HAVE THROWN!\n%s\n",#αα);backtrace_line(); \
}catch(chars){}catch(String*){}catch(...){};panicking=old;}
#endif

void testMergeGlobal() {
#if INCLUDE_MERGER
    Module &main = loadModule("test/merge/main_global.wasm");
    Module &lib = loadModule("test/merge/lib_global.wasm");
    Code merged = merge_binaries(main.code, lib.code);
    smart_pointer_64 i = merged.save().run();
    assert_equals(i, 42);
#endif
}

void testMergeMemory() {
#if INCLUDE_MERGER
    Module &main = loadModule("test/merge/main_memory.wasm");
    Module &lib = loadModule("test/merge/lib_memory.wasm");
    Code merged = merge_binaries(main.code, lib.code);
    int i = merged.save().run();
    assert_equals(i, 42);
#endif
}


void testMergeRuntime() {
#if INCLUDE_MERGER
    Module &runtime = loadModule("wasp-runtime.wasm");
    Module &main = loadModule("test/merge/main_memory.wasm");
    main.code.needs_relocate = true;
    runtime.code.needs_relocate = false;
    Code merged = merge_binaries(runtime.code, main.code);
    int i = merged.save().run();
    assert_equals(i, 42);
#endif
}


void testMergeOwn() {
    testMergeMemory();
    testMergeGlobal();
#ifdef INCLUDE_MERGER
    Module &main = loadModule("test/merge/main2.wasm");
    Module &lib = loadModule("test/merge/lib4.wasm");
    Code merged = merge_binaries(main.code, lib.code);
//	Code merged = merge_binaries(lib.code,main.code);
    int i = merged.save().run();
    assert_equals(i, 42);
#endif
}

void testWasmStuff();


void testEmitter() {
#ifndef RUNTIME_ONLY
    clearAnalyzerContext();
    clearEmitterContext();
    Node node = Node(42);
    Code &code = emit(node);
    int result = code.run();
    check(result == 42);
#endif
}


void testGlobals() {
    assert_emit("π", pi);
}

void test_get_local() {
    assert_emit("add1 x:=$0+1;add1 3", (int64) 4);
}

void testWasmFunctionDefiniton() {
//	assert_is("add1 x:=x+1;add1 3", (int64) 4);
    assert_emit("fib:=if it<2 then it else fib(it-1)+fib(it-2);fib(7)", 13)
    assert_emit("fac:= if it<=0 : 1 else it * fac it-1; fac(5)", 5 * 4 * 3 * 2 * 1);

    assert_emit("add1 x:=x+1;add1 3", (int64) 4);
    assert_emit("add2 x:=x+2;add2 3", (int64) 5);
    skip(
            assert_emit("expression_as_return:=y=9;expression_as_return", (int64) 9);
            assert_emit("addy x:= y=2 ; x+y ; addy 3", (int64) 5);
    )

    assert_emit("grows x:=x*2;grows(4)", 8)
    assert_emit("grows:=it*2; grows 3", 6)
    assert_emit("grows:=it*2; grows 3*4", 24)
    assert_emit("grows:=it*2; grows(3*42) > grows 2*3", 1)
    assert_emit("factorial:=it<2?1:it*factorial(it-1);factorial 5", 120);

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
//	assert_emit("(1<2)?10:255", 255);

    assert_emit("fac:= it<=0 ? 1 : it * fac it-1; fac(5)", 5 * 4 * 3 * 2 * 1);
    skip(
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
    skip(
            assert_emit("puts 'ok'", (int64) 0);
    )
    assert_emit("i=1;while i<9:i++;i+1", 10);
    assert_emit("ceil 3.7", 4);
    assert_emit("square 3", 9);
//    assert_emit("putf 3.1", 0);
    assert_emit("putf 3.1", 3.1);
    assert_emit("puti 3", (int64) 3);
//    assert_emit("puti 3", 0);
    assert_emit("puti 3", 3);// todo ()
//    assert_emit("puti 3+3", 0);
    assert_emit("puti 3+3", 6);
    assert_emit("4*5 + square 2*3", (int64) 56);
    assert_emit("3 + square 3", (int64) 12);
    assert_emit("1+2 + square 1+2", (int64) 12);

    assert_emit("3 + √9", (int64) 6);
    assert_emit("square(3*42) > square 2*3", 1)
    assert_emit("id(3*42) > id 2*3", 1)
    assert_emit("id 123", (int64) 123);
    assert_emit("id (3+3)", (int64) 6);
    assert_is("id 3+3", 6);
    assert_emit("3 + id 3+3", (int64) 9);
}

void testConstReturn() {
    assert_emit(("42"), 42)
}

void testPrint() {// does wasm print? (visual control!!)
    assert_emit(("print 42"), 42)
    print("OK");
//	printf("%llx\n", -2000000000000ll);
//	printf("%llx", -4615739258092021350ll);
    print("a %d c"s % 3);
    print("a %f c"s % 3.1);
    print("a %x c"s % 15);
    printf("a %d c\n", 3);
    printf("a %f c\n", 3.1);
    printf("a %x c\n", 15);
}

void testMathPrimitives() {
    skip(
            assert_emit(("42.1"), 42.1) // todo: return &Node(42.1) or print value to stdout
            assert_emit(("-42.1"), 42.1)
    )
    assert_emit(("42"), 42)
    assert_emit("-42", -42)
    skip(
            assert_emit(("2000000000"), 2000000000)// todo stupid smart pointers
            assert_emit(("-2000000000"), -2000000000)
    )
    assert_emit(("2000000000000"), (int64) 2000000000000)// auto int64
    assert_emit(("-2000000000000"), (int64) -2000000000000L)

    assert_emit("x=3;x*=3", 9)
    assert_emit("'hello';(1 2 3 4);10", 10);
//	data_mode = false;
    assert_emit("i=ø; not i", true);
    assert_emit("0.0", (int64) 0);// can't emit float yet
    assert_emit(("x=15;x>=14"), 1)
    assert_emit("i=1.0;i", 1.0);// works first time but not later in code :(
    assert_emit("i=0.0;i", 0.0);//
    assert_is("3*-1", -3);
    assert_emit("3*-1", -3);

    skip(// todo NOT SKIP!
            assert_emit("maxi=3840*2160", 3840 * 2160);
            assert_emit("maxi=3840*2160;maxi", 3840 * 2160);
            assert_emit("blue=255;green=256*255;", 256 * 255);
    )
}

void testFloatOperators() {
    assert_is(("42.0/2.0"), 21)
    assert_emit(("3.0+3.0*3.0"), 12)
    assert_emit(("42.0/2.0"), 21)
    assert_emit(("42.0*2.0"), 84)
    assert_emit(("42.0+2.0"), 44)
    assert_emit(("42.0-2.0"), 40)
    assert_emit(("3.0+3.0*3.0"), 12)
    assert_emit(("3.1>3.0"), true)
    assert_emit(("2.1<3.0"), true)
    assert_emit("i=123.4;i", 123.4);// main returning int
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

void testNorm2() {
    assert_emit("1-‖3‖/-3", 2);
    assert_emit("1-‖-3‖/3", 0);
    assert_emit("1-‖-3‖/-3", 2);
    assert_emit("1-‖-3‖-1", -3);
    assert_emit("√9*-‖-3‖/3", -3);
    assert_emit("√9*‖-3‖/-3", -3);
    assert_emit("√9*-‖-3‖/-3", 3);
    assert_emit("f=4;‖-3‖<f", 1);
    assert_emit("i=1;(5-3)>i", 1);
    assert_emit("i=1;‖-3‖>i", 1);
    assert_emit("i=1;‖-3‖<i", 0);
    assert_emit("f=4;‖-3‖>f", 0);
    skip(
            assert_emit("i=1;x=‖-3‖>i", 1);
            assert_emit("f=4;x=‖-3‖<f", 1);
            assert_emit("i=1;x=‖-3‖<i", 0);
            assert_emit("f=4;x=‖-3‖>f", 0);
    )
}

void testNorm() {
    testNorm2();
    assert_emit("‖-3‖", 3);
    assert_emit("‖3‖-1", 2);
    assert_emit("‖-3‖/3", 1);
    assert_emit("‖-3‖/-3", -1);
    assert_emit("‖3‖/-3", -1);
    assert_emit("-‖-3‖/3", -1);
    assert_emit("-‖-3‖/-3", 1);
    assert_emit("-‖3‖/-3", 1);
    assert_emit("‖-3‖>1", 1);
    assert_emit("‖-3‖<4", 1);
    assert_emit("‖-3‖<1", 0);
    assert_emit("‖-3‖>4", 0);
}

void testMathOperators() {
//	assert_emit(("42 2 *"), 84)
    assert_emit("- -3", 3);
    assert_emit("1- -3", 4);
    assert_emit("1 - -3", 4);
    skip(
            assert_emit("1 - - 3", 4);// -1 uh ok?
            assert_throws("1--3");// should throw, variable missed by parser! 1 OK'ish
    )

    //	assert_emit("1--3", 4);// should throw, variable missed by parser! 1 OK'ish

    assert_emit("‖-3‖", 3);
    assert_emit("-‖-3‖", -3);
    assert_emit("‖-3‖+1", 4);
    assert_is(("7%5"), 2)
    assert_is(("42/2"), 21)
//			WebAssembly.Module doesn't validate: control flow returns with unexpected type. F32 is not a I32, in function at index 0
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


    assert_emit("i=3;i*-1", -3);
    assert_is("3*-1", -3);
    assert_emit("3*-1", -3);
    assert_emit("-√9", -3);

    assert_emit("i=3.7;.3+i", 4);
    assert_emit("i=3.71;.3+i", 4.01);
    assert_emit("i=3.70001;.3+i", 4.00001);
    assert_is("4-1", 3);//

    assert_emit("i=3;i++", 4);
    assert_emit("- √9", -3);
    assert_emit("i=-9;-i", 9);
    assert_emit("√ π ²", 3.1415926535896688);
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

void testMathOperatorsRuntime() {
    assert_emit("3^2", 9);
    assert_emit("3^1", 3);
    assert_emit("√3^2", 3);
    assert_emit("√3^0", 1);
    assert_emit(("42^2"), 1764);// NO SUCH PRIMITIVE
    assert_emit("1 - 3 - square 3+4", (int64) -51);// OK!
    assert_is("π**2", (double) 9.869604401089358);
}

void testComparisonMath() {
    // may be evaluated by compiler!
    assert_emit(("3*42>2*3"), 1)
    assert_emit(("3*1<2*3"), 1)
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
	assert_emit(("3*13>=14*3"), False);
	assert_emit(("3*15<=14*3"), False);
	assert_emit(("3*42≥112*3"), false)
	assert_emit(("3*2≥112*3"), false)
	assert_emit(("3*12≤2*3"), false)
	assert_emit(("3*112≤24*3"), false)

//    assert_emit(("3*452==452*3"), 1) // forces runtime
//    assert_emit(("3*13==14*3"), False);
}


void testComparisonId() {
    // may be evaluated by compiler!
    assert_emit("id(3*42 )> id 2*3", 1)
    assert_emit("id(3*1)< id 2*3", 1)
	skip(
			assert_emit("id(3*452)==452*3", 1)
			assert_emit("452*3==id(3*452)", 1)
			assert_emit("452*3==id 3*452", 1)
			assert_emit("id(3*452)==452*3", 1)
			assert_emit(("id(3*13)==14*3"), False);
	)
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
			assert_emit("id 3*452==452*3", 1) // forces runtime
			assert_emit(("id 3*13==14*3"), False);

//	Ambiguous mixing of functions `ƒ 1 + ƒ 1 ` can be read as `ƒ(1 + ƒ 1)` or `ƒ(1) + ƒ 1`
			assert_emit("id 3*42 > id 2*3", 1)
			assert_emit("id 3*1< id 2*3", 1)
	)
    assert_emit("id(3*42)> id 2*3", 1)
    assert_emit("id(3*1)< id 2*3", 1)
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
    assert_emit(("id 3*13>= id 14*3"), False);
    assert_emit(("id 3*15<= id 14*3"), False);
    assert_emit(("id 3*13<= id 14*3"), 1)
    assert_emit(("id 3*42≥112*3"), false)
    assert_emit(("id 3*2≥112*3"), false)
    assert_emit(("id 3*12≤2*3"), false)
    assert_emit(("id 3*112≤24*3"), false)
}

void testComparisonPrimitives() {
    assert_emit(("42>2"), 1)
    assert_emit(("1<2"), 1)
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
	assert_emit(("13>=14"), False);
	assert_emit(("15<=14"), False);
	assert_emit(("42≥112"), false)
	assert_emit(("2≥112"), false)
	assert_emit(("12≤2"), false)
	assert_emit(("112≤24"), false)
#if not WASM
	assert_emit(("452==452"), 1) // forces runtime eq
	assert_emit(("13==14"), False);
#endif
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
    assert_emit("false", (int64) 0);

    assert_emit("nil", false);
    assert_emit("null", false);
    assert_emit("null", (int64) 0);
    assert_emit("null", (int64) nullptr);
    assert_emit("ø", false);
    assert_emit("nil", NIL);
}


void testWasmVariables0() {
//	  (func $i (type 0) (result i32)  i32.const 123 return)  NO LOL
    assert_emit("i=123;i", 123);
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
    assert_emit("8.33333333332248946124e-03", 0);// todo in wasm
    assert_emit("8.33333333332248946124e+01", 83.3333333332248946124);
    assert_emit("8.33333333332248946124e+03", 8333.33333332248946124);
    assert_emit("S1  = -1.6666", -1.6666);
//    assert_emit("grows S1  = -1.6666", -1);
    // may be evaluated by compiler!
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
    skip(
            assert_emit("i=true; not i", false);
    )
    assert_emit("i=ø; not i", true);

    assert_emit("i=1; not i", false);
    assert_emit("i=123; not i", false);
}

void testSelfModifying() {
    assert_emit("i=3;i*=3", (int64) 9);
    assert_emit("i=3;i+=3", (int64) 6);
    assert_emit("i=3;i-=3", (int64) 0);
    assert_emit("i=3;i/=3", (int64) 1);
    //	assert_emit("i=3;i√=3", (int64) ∛3); NO i TIMES √
    skip(
            assert_emit("i=3^1;i^=3", (int64) 27);
            assert_throws("i*=3");// well:
            assert_emit("i*=3", (int64) 0);
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
    assert_emit("not 'a'", false);
    assert_emit("not {a:2}", false);
    skip(
            assert_emit("not {a:0}", false);// maybe
    )

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

    assert_emit("¬ 1", 0);
    assert_emit("¬ 0", 1);

    assert_emit("0 ⋁ 0", 0);
    assert_emit("0 ⋁ 1", 1);
    assert_emit("1 ⋁ 0", 1);
    assert_emit("1 ⋁ 1", 1);

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

    assert_emit("0 ⊻ 0", 0);
    assert_emit("0 ⊻ 1", 1);
    assert_emit("1 ⊻ 0", 1);
    assert_emit("1 ⊻ 1", 0);
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
    assert_emit("i=1;while(i<9)i++;i+1", 10);
    skip(// fails on 2nd attempt todo
            assert_emit("x=y=0;width=height=400;while y++<height and x++<width: nop;y", 400);
    )
    assert_emit("i=1;while(i<9)i++;i+1", 10);
}


void testWasmMemoryIntegrity() {
    return;
#ifndef WASM
#endif

    if (!MAX_MEM) {
        error("NO MEMORY");
    }
    printf("MEMORY start at %lld\n", (int64) memory);
    printf("current start at %lld\n", (int64) heap_end);
//	Bus error: 10  if i > MEMORY_SIZE
// Fails at 100000, works at 100001 WHERE IS THIS SET?
//	int start=125608;
    int start = __heap_base;
    int64 end = 0x1000000;// MAX_MEM / 4; // /4 because 1 int = 4 bytes
    for (int i = start; i < end; ++i) {
        int tmp = memory[i];
//		memory[i] = memory[i]+1;
//		memory[i] = memory[i]-1;
        memory[i] = i;
//		if(i%10000==0)logi(i);// logi USES STACK, so it can EXHAUST if called too often!
        if (memory[i] != i) {
            printf("MEMORY CORRUPTION at %d", i);
            proc_exit(0);
        }
        memory[i] = tmp;// else test fail
    }
}

void testOldRandomBugs() {

    skip(
            assert_emit("x:=41;if x>1 then 2 else 3", 2)
            assert_emit("x=41;if x>1 then 2 else 3", 2)
            assert_emit("x:41;if x>1 then 2 else 3", 2)
            assert_emit("x:41;if x<1 then 2 else 3", 3)


            assert_emit("x:41;x+1", 42)

            assert_emit("grows := it * 2 ; grows(4)", 8)
            assert_emit("grows:=it*2;grows(4)", 8)
    )

    //		testGraphQlQuery();
    check(operator_list.has("+"));
    check(not(bool) Node("x"));
    check(false == (bool) Node("x"));
    check(Node("x") == false);
    assert_throws("x"); // UNKNOWN local symbol ‘x’ in context main  OK
//	assert_is("x", Node(false));// passes now but not later!!
//	assert_is("x", false);// passes now but not later!!
//	assert_is("y", false);
//	assert_is("x", false);
    assert(interpret("ç='☺'") == "☺");// fails later => bad pointer?
    assert_is("(2+1)==(4-1)", true);
    assert(eval("(2+1)==(4-1)") == 1);// suddenly passes !? not with above line commented out BUG <<<
    assert(eval("3==2+1") == 1);
    assert(eval("2+1==2+1") == 1);
    assert_emit("square 3", 9);
    assert_emit("id (3+3)", (int64) 6);
    const Node &node = parse("x:40;x+1");
    check(node.length == 2)
    check(node[0]["x"] == 40)


    //0 , 1 , 1 , 2 , 3 , 5 , 8 , 13 , 21 , 34 , 55 , 89 , 144
    //	assert_emit("fib(it-1)",3);
    assert_emit("if 4>1 then 2 else 3", 2)

//	assert_emit("1 -3 - square 3+4", (int64) -51);
    assert_emit("1+2 + square 3+4", (int64) 52);

    assert_emit("4*5 + square 2*3", (int64) 56);
    //	assert_emit("id 3*42> id 2*3", 1)

    //	exit(1);
    //	const Node &node1 = parse("x:40;x++;x+1");
    //	check(node.length==3)
    //	check(node[0]["x"]==40)
    //	exit(1);
    assert_emit("3 + √9", (int64) 6);
    assert_emit("square 3", 9);
    assert_emit("-42", -42)
}

//void testRefactor(){
//	wabt::Module *module = readWasm("t.wasm");
//	refactor_wasm(module, "__original_main", "_start");
//	module = readWasm("out.wasm");
//	check(module->funcs.front()->name == "_start");
//}

#ifdef WABT_MERGE
#include "wasm_merger.h"
#include "wasm_merger_wabt.h"
//#include "../Frameworks/wabt/src/ir.h"
#endif

void testMergeWabt() {
#ifdef WABT_MERGE
    merge_files({"test/merge/main.wasm", "test/merge/lib.wasm"});
#endif
}


void testMergeWabtByHand() {
//	merge_files({"./playground/test-lld-wasm/main.wasm", "./playground/test-lld-wasm/lib.wasm"});
//wasm
//	wabt::Module *main = readWasm("test-lld-wasm/main.wasm");
//	wabt::Module *module = readWasm("test-lld-wasm/lib.wasm");
//	refactor_wasm(module, "b", "neu");
//	remove_function(module, "f");
//	Module *merged = merge_wasm2(main, module);
//	save_wasm(merged);
//	int ok=run_wasm(merged);
//	int ok=run_wasm("a.wasm");
//	check(ok==42);
}


void testWasmRuntimeExtension() {
#if TRACE
    printf("TRACE mode currently SIGTRAP's in testWasmRuntimeExtension. OK, Switch to Debug mode. WHY though?");
#endif

    assert_run("43", 43);
    assert_run("strlen('123')", 3);
    skip(
//            todo polymorphism
            assert_run("len('123')", 3);
            assert_run("len('1235')", 4);
    )
    assert_run("parseLong('123')", 123);
    assert_run("parseLong('123'+'456')", 123456);
#if not TRACE // todo why??
    assert_run("parseLong('123000') + parseLong('456')", 123456);
    assert_run("x=123;x + 4 is 127", true);
    assert_run("parseLong('123'+'456')", 123456);
    assert_run("'123' is '123'", true);
    assert_run("'123' + '4' is '1234'", true);// ok
#endif

    assert_run("test42+1", 43);
    assert_run("test42i(1)", 43);
    assert_throws("not_ok");// error

    assert_run("test42f(1)", 43);
    assert_run("test42f(1.0)", 43.0);
    assert_run("42.5", 42.5);// truncation ≠ proper rounding!
    assert_run("42.6", 42.6);// truncation ≠ proper rounding!
    assert_run("test42f(1.7)", 43.7);
    skip(
            assert_run("test42f", 41.5); //default args don't work in wasm! (how could they?)
            assert_run("test42f", 41.5); /// … expected f32 but nothing on stack
    )
//	functionSignatures["int"].returns(int32);
//	assert_run("printf('123')", 123);
// works with ./wasp but breaks in webapp
// works with ./wasp but breaks now:

//	assert_run("okf(1)", 43);
//	assert_run("puts 'hello' 'world'", "hello world");
//	assert_run("hello world", "hello world");// unresolved symbol printed as is

    skip(
            assert_run("x=123;x + 4 is 127", true);
//	assert_run("'123'='123'", true);// parsed as key a:b !?!? todo!
//	assert_run("'123' = '123'", true);
    )
    assert_run("'123' == '123'", true);
    assert_run("'123' is '123'", true);
    assert_run("'123' equals '123'", true);
    assert_run("x='123';x is '123'", true);
//	assert_run("string('123') equals '123'", true); // string() makes no sense in angle:
//	assert_run("'123' equals string('123')", true);//  it is internally already a string whenever needed
//	assert_run("atoi0(str('123'))", 123);
//	assert_run("atoi0(string('123'))", 123);

//	assert_run("oki(1)", 43);
//	assert_emit("puts('123'+'456');", 123456);// via import not via wasp!
//assert_emit("grows := it * 2 ; grows(4)", 8)
//	check(Primitive::charp!=Valtype::pointer)

    skip(
            assert_run("'123'", 123);// result printed and parsed?
            assert_run("printf('123')", 123);// result printed and parsed?
    )
    skip( // if not compiled as RUNTIME_ONLY library:
            check(functionSignatures.has("tests"))
            assert_run("tests", 42);
    )
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

//	assert_emit("'world'[1]", 'o');
    assert_emit("'world'#1", 'w');
    assert_emit("'world'#2", 'o');
    assert_emit("'world'#3", 'r');
    skip( // todo move angle syntax to test_angle
            assert_emit("char #1 in 'world'", 'w');
            assert_emit("char 1 in 'world'", 'w');
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
//	data_mode = true;// todo remove hack
    assert_emit("x={1 2 3}; x#3=4;x#3", 4);
#if WASM
    assert_emit("puts('ok');", -1);
#else
    assert_emit("puts('ok');", 8);
    assert_emit("puts('ok');(1 4 3)#2", 4);
#endif
//    assert_emit("puts('ok');", 0);
    assert_emit("{1 4 3}#2", 4);

    assert_emit("x={1 4 3};x#2", 4);
    assert_emit("{1 4 3}[1]", 4);
    assert_emit("(1 4 3)[1]", 4);
    assert_throws("(1 4 3)#0");

    assert_emit("'αβγδε'#3", U'γ');
    assert_emit("i=3;k='αβγδε';k#i", u'γ');
    skip(
            assert_emit("i=3;k='αβγδε';k#i='Γ';k#i", u'Γ'); // todo setCharAt
            assert_emit("[1 4 3]#2", 4);// exactly one op expected in emitIndexPattern
            assert_is("[1 2 3]#2", 2);// check node based (non-primitive) interpretation first
            assert_throws("(1 4 3)#4");// todo THROW!
// todo patterns as lists
    )


//	Node empty_array = parse("pixel=[]");
//	check(empty_array.kind==patterns);
//
//	Node construct = analyze(parse("pixel=[]"));
//	check(construct["rhs"].kind == patterns or construct.length==1 and construct.first().kind==patterns);
//	emit("pixel=[]");
//	exit(0);
}


// random stuff todo: put in proper tests
void testWasmStuff() {
//	assert_emit("grows := it * 2 ; grows(4)", 8)
    assert_emit("-42", -42)
    assert_emit("x=41;x+1", 42)
    assert_emit("x=40;y=2;x+y", 42)
    assert_emit("id(4*42) > id 2+3", 1)
    skip(

            assert_emit("grows x := x * 2 ; grows(4)", 8)
            assert_emit("grows := it * 2 ; grows(4)", 8)
            assert_emit("grows:=it*2; grows 3", 6)
            assert_emit("add1 x:=x+1;add1 3", (int64) 4);
            assert_emit("fib x:=if x<2 then x else fib(x-1)+fib(x-2);fib(7)", 13)
            assert_emit("fib x:=if x<2 then x else{fib(x-1)+fib(x-2)};fib(7)", 13)
    )
}

bool testRecentRandomBugsAgain = true;

void testRecentRandomBugs() {
    if (!testRecentRandomBugsAgain)return;
    testRecentRandomBugsAgain = false;

    // these fail LATER in tests!!
    assert_emit("1-‖3‖/-3", 2);
    assert_emit("i=true; not i", false);
    skip(
            assert_emit("i=3^1;i^=3", (int64) 27);
            assert_throws("i*=3");// well:
            assert_emit("i*=3", (int64) 0);
    )
    assert_emit("maxi=3840*2160", 3840 * 2160);
    assert_emit("√π²", 3);
    assert_emit("i=-9;√-i", 3);
    assert_emit("1- -3", 4);
    assert_emit("width=height=400;height", 400);
    skip(
            assert_throws("1--3");// should throw, variable missed by parser! 1 OK'ish
            assert_emit("x=0;while x++<11: nop;x", 11);
            assert_throws("x==0;while x++<11: nop;x");
    )
    assert_emit("‖-3‖", 3);
    assert_emit("√100²", 100);
//    assert_emit("puts('ok');", 0);
    assert_parsesx("{ç:☺}");
    assert(result["ç"] == "☺");
#ifndef WASMTIME
    assert_run("x=123;x + 4 is 127", true);
    assert_emit("n=3;2ⁿ", 8);
    //	function attempted to return an incompatible value WHAT DO YOU MEAN!?
#endif
// move to tests() once OK'
    skip(
            assert_emit("i=ø; not i", true);// i not a setter if value ø
            assert_emit("x=y=0;width=height=400;while y++<height and x++<width: nop;y", 400);
    )
}


void testSquareExpWasm() {
    let π = pi; //3.141592653589793;
// todo smart pointer return from main for floats!
    assert_emit("3²", 9);
    assert_emit("3.0²", 9);
    assert_emit("√100²", 100);
    assert_emit("√ π ²", π);
    assert_emit("√π ²", π);
    assert_emit("√ π²", π);
    assert_emit("√π²", π);
    assert_emit("π²", π * π);
    assert_emit("π", pi);
    assert_emit("π*1000000.", 3141592.6535897);
    assert_emit("int i=π*1000000", 3141592);
    assert_emit("i=-9;-i", 9);
    assert_emit("- √9", -3);
    assert_emit(".1 + .9", 1);
    assert_emit("-.1 + -.9", -1);
    assert_emit("√9", 3);
//	assert_emit("√-9 is -3i", -3);// if «use complex numbers»
    assert_emit(".1", .1);
#ifndef WASMTIME
    skip(
            assert_emit("i=-9;√-i", 3);
            assert_emit("n=3;2ⁿ", 8);
            assert_emit("n=3.0;2.0ⁿ", 8);
    //	function attempted to return an incompatible value WHAT DO YOU MEAN!?
    )
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

//			WebAssembly.Module doesn't validate: control flow returns with unexpected type. F32 is not a I32, in function at index 0
            assert_is(("42/2"), 21)// in WEBAPP

            assert_emit("i=0;w=800;h=800;pixel=(1 2 3);while(i++ < w*h){pixel[i]=i%2 };i ", 800 * 800);

            assert_emit(("42.1"),
                        42.1) // main returns int, should be pointer to value! result & array_header_32 => smart pointer!

            //			Ambiguous mixing of functions `ƒ 1 + ƒ 1 ` can be read as `ƒ(1 + ƒ 1)` or `ƒ(1) + ƒ 1`
            assert_emit("id 3*42 > id 2*3", 1)
            assert_emit("square 3*42 > square 2*3", 1)
            assert_emit("grows:=it*2; grows 3*42 > grows 2*3", 1)
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
    assert_emit("i=1;k='hi';k#i", 'h'); // BUT IT WORKS BEFORE!?! be careful with i64 smarty return!
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
    // todo generic power i as builtin
#ifndef WASMTIME
    skip(
    // REGRESSION 2023-01-20 variable x-c in context wasp_main emitted as node data:
            assert_emit("x=3;y=4;c=1;r=5;((‖(x-c)^2+(y-c)^2‖<r)?10:255", 255);
    )
#endif
    assert_emit("i=-9;√-i", 3);
    assert_emit("i=-9;√ -i", 3);
//		assert_emit("use math;√π²", 3);
}

void testSmartReturn() {
    assert_emit("1", 1);
    assert_emit("-2000000000000", (int64) -2000000000000l)
    assert_emit("2000000000000", (int64) 2000000000000l)// auto int64
    assert_emit("42.0/2.0", 21);
    assert_emit("42.0/2.0", 21.);
    assert_emit("- √9", -3);
    assert_emit("42/4.", 10.5);
    skip(
            assert_emit("42/4", 10.5);
    )

    assert_is(("42.0/2.0"), 21)
    assert_emit("x='abcde';x#4='f';x[3]", 'f');
    assert_emit(("-1.1"), -1.1)
    assert_emit("'OK'", "OK");
    assert_emit("'a'", Node('a'));
    assert_emit("'a'", Node(u'a'));
    assert_emit("'a'", Node(U'a'));
    assert_emit("'a'", String('a'));
    assert_emit("'a'", String(u'a'));
    assert_emit("'a'", String(U'a'));
//    assert_emit("'a'", 'a'); // … should be 97
//    assert_emit("'a'", u'a');
//    assert_emit("'a'", U'a');
    assert_emit("10007.0%10000.0", 7);
    assert_emit("10007.0%10000", 7);
    assert_emit("x='abcde';x#4='x';x[3]", 'x');
    assert_emit("x='abcde';x[3]", 'd');
//    assert_emit("x='abcde';x[3]", (int) 'd');// currently FAILS … OK typesafe!

}

void testMultiValue() {
#ifdef MULTI_VALUE
    assert_emit("1,2,3", Node(1, 2, 3, 0));
    assert_emit("1;2;3", 3);
    assert_emit("'OK'", "OK");
#endif
}

void testAssertRun() {
// all these have been tested with assert_emit before. now check that it works with runtime
    testWasmRuntimeExtension();

    assert_run("42", 42);
    assert_run("x=123;x + 4 is 127", true); //  assert_run sometimes causes Heap corruption! test earlier
    assert_run("x='123';x is '123'", true);// ok
    assert_run("'hello';(1 2 3 4);10", 10);// -> data array […;…;10] ≠ 10
#if not TRACE
    assert_run("x='123';x + '4' is '1234'", true);// ok
    assert_run("'123' + '4' is '1234'", true);// ok needs runtime for concat()
    assert_run("x='123';x=='123'", true);// ok needs runtime for eq()
#endif
}


void testLogarithm() {
    skip(
            assert_emit("use log; log10(100)", 2.);
    )
}

void testLogarithm2() {
//	float ℯ = 2.7182818284590;
    Function &function = functions["log10"];
    check(function.is_import);
    assert_emit("use math; log10(100)", 2.);
    assert_emit("use math; 10⌞100", 2.);// read 10'er Logarithm
    assert_emit("use math; 100⌟10", 2.);// read 100 lowered by 10's
    assert_emit("use math; 10⌟100", 2.);
    assert_emit("use math; ℯ⌟", 2.);
    assert_emit("use math; ℯ⌟", 2.);
    assert_emit("log10(100)", 2.); // requires pre-parsing lib and dictionary lookup
    assert_emit("₁₀⌟100", 2.); // requires pre-parsing lib and dynamic operator-list extension OR 10⌟ as function name
    assert_emit("10⌟100", 2.); // requires pre-parsing lib and dynamic operator-list extension OR 10⌟ as function name

//    assert_equals(ln(e),abs(1))
    assert_emit("use log;ℯ = 2.7182818284590;ln(ℯ)", 1.);
    assert_emit("use log;ℯ = 2.7182818284590;ln(ℯ)", 1.);
    assert_emit("ℯ = 2.7182818284590;ln(ℯ*ℯ)", 2.);
    assert_emit("ln(1)", 0.);
    assert_emit("log10(100000)", 5.);
    assert_emit("log10(10)", 1.);
    assert_emit("log(1)", 0.);
    skip(
            assert_equals(-ln(0), Infinity);
            assert_equals(ln(0), -Infinity);
            assert_emit("ln(ℯ)", 1.);
    )
}


void testAllWasm() {
    testWasmGC();
//	data_mode = false;
    testWasmMemoryIntegrity();
#ifdef RUNTIME_ONLY
    puts("RUNTIME_ONLY");
    puts("NO WASM emission...");
    //	return;
#endif
    skip(
    //	assert_run not compatible with Wasmer, don't ask why, we don't know;)
            testCustomOperators();
            testWasmMutableGlobal();
    )

    testMathOperators();
    testWasmLogicPrimitives();
    testWasmLogicUnary();
    testWasmLogicUnaryVariables();
    testWasmLogic();
    testWasmLogicNegated();
    testSquareExpWasm();
    testGlobals();

    testComparisonIdPrecedence();
    testWasmStuff();
    testFloatOperators();
    testConstReturn();
    testWasmIf();
    testMathPrimitives();
    testSelfModifying();
    testNorm();
    testComparisonPrimitives();
    testComparisonMath();
    testComparisonId();
    testWasmTernary();
    testSquareExpWasm();
    testRoundFloorCeiling();
    testWasmTernary();
    testArrayIndicesWasm();
    testWasmFunctionCalls();
    testWasmFunctionDefiniton();
    testWasmWhile();

    // the following need MERGE or RUNTIME! todo : split
    testWasmVariables0();


    testIndexWasm();

    wasm_todos();
    testLogarithm();


    testMergeOwn();
    skip(
            testMergeRelocate();
    )
    testMergeWabt();
    testMergeWabtByHand();
    testEmitter();
    testMathLibrary();
    testWasmLogicCombined();
    testMergeWabt();
    wasm_todos();

//	exit(21);
    testWasmIncrement();
// TRUE TESTS:
    testRecentRandomBugs();
    testOldRandomBugs();
    testMathOperatorsRuntime();
    assert_is("١٢٣", 123);// todo UTF RTL control character!

    // Test that IMPLICITLY use runtime /  assert_run
    assert_emit("x=(1 4 3);x#2", 4);
    testStringIndicesWasm();


    skip(
            test_get_local();

            testCustomOperators();
            testWasmLogicOnObjects();
            testObjectPropertiesWasm();
    )
}
