//
// Created by pannous on 12.06.20.
//

#include "Wasp.h" // parse
#include "Angle.h" // analyze

#define assert_ast(α, β) if (!assert_equals_x(analyze(parse(α)),parse(β))){printf("%s != %s",#α,#β);backtrace_line();}
#define assert_eval(α, β) if (!assert_equals_x(eval(α),β)){printf("%s != %s",#α,#β);backtrace_line();}


void testFunctionParams() {
//	assert_equals(parse("f(x)=x*x").param->first(),"x");
	assert_equals("f(x)=x*x;f(3)", "9");// functions => angle!
}

void testOperatorBinding() {
	assert_ast("a and b", "and(a,b)");
}

void testCall(){
	assert_is("square(3)",9)
	assert_is("square 3",9)
	assert_is("square(1+2)",9)
	assert_is("square 1+2",9)
	assert_is("1+square(2+3)",26)
	assert_is("1+square 2+3",26)
	assert_is("1 + square 1+2",10)
	skip(
	assert_is("square{i:3}",9) //todo: match arguments!
			)
}

void testIf() {
	assert_eval("if(0):{3}", false);
	assert_eval("if(2):{3}", 3);
	assert_eval("if 2 : 3 else 4", 3);

	assert_eval("if 0:3", false);
	skip(
	assert_eval("if '':3", false);
	assert_eval("if ():3", false);
	assert_eval("if ø:3", false);
	assert_eval("if {}:3", false);
	assert_eval("if x:3", false);
			)

	skip(// esotheric shit
	assert_eval("if(0,then=3,else=4)", 4);
	assert_eval("if(1,then=3,else=4)", 3);
	assert_eval("if(2,then=3)", 3);
			assert_eval("if(condition=2,then=3)", 3);
			assert_eval("if(condition=0,then=3,else=4)", 4);
			assert_eval("if(condition=1,then=3,else=4)", 3);
	)
	assert_eval("if(0):{3} else {4}", 4);
	assert_eval("if 0:{3} else {4}", 4);
	assert_eval("if 0:3 else {4}", 4);
	assert_eval("if 0 {3} else {4}", 4);
	assert_eval("if {0}:3 else 4", 4);
	assert_eval("if 0:3 else 4", 4);
	assert_eval("if 0:{3} else 4", 4);
	assert_eval("if (2) {3} else 4", 3);
	assert_eval("if(2){3} else 4", 3);
	assert_eval("if(0){3} else 4", 4);
	assert_eval("if 2 {3} else {4}", 3);
	assert_eval("if 0 {3} else {4}", 4);
	assert_eval("if (2) {3} else {4}", 3);
	assert_eval("if(2){3} else {4}", 3);
	assert_eval("if(0){3} else {4}", 4);
	assert_eval("if (0) {3} else {4}", 4);
	assert_eval("if(2):{3} else 4", 3);
	assert_eval("if(2):{3} else {4}", 3);
	assert_eval("if 2:{3} else 4", 3);
	assert_eval("if 2:3 else 4", 3);
	assert_eval("if 2:{3} else {4}", 3);
	assert_eval("if 2:3 else {4}", 3);
	assert_eval("if 2 {3}", 3);
	assert_eval("if(0):{3} else 4", 4);
	assert_eval("if (0) {3}", false);
	assert_eval("if 2 then 3 else 4", 3);
	assert_eval("if (0) {3} else 4", 4);
//	assert_eval("2 then 3 else 4", 3);
	assert_eval("2 and 3 or 4", 3);
	assert_eval("1 and 0 or 4", 4);
	assert_eval("if 1 then 0 else 4", (long) 0);
	assert_eval("if 0:{3} else 4", 4);


	assert_eval("if 0 {3}", false);
	assert_eval("false or 3", 3);
	assert_eval("false else 3", 3);
	assert_eval("4 or 3", 4);
//	assert_eval("4 else 3", 4);
	assert_eval("if (2) {3}", 3);
	assert_eval("if(2){3}", 3);
	assert_eval("if(0){3}", false);
	assert_eval("if 2:{3}", 3);
	assert_eval("if 2:3", 3);

	assert_eval("if 0 {3} else 4", 4);
	assert_eval("if 2 {3} else 4", 3);

//	assert_eval("if 2 , 3 , 4", 3);
//	assert_eval("if{2 , 3 , 4}", 3);
//	assert_eval("if 2 then 3 else 4", 3);
	assert_eval("if(2,3,4)", 3);
	assert_eval("if({2},{3},{4})", 3);
	assert_eval("if(2){3}{4}", 3);
	skip(
			assert_parses("if(condition=2,then=3,else=4)");
			check(result["condition"] == 2);
			check(result["else"] == 4);
			assert_eval("if(condition=2,then=3,else=4)", 3); // this is what happens under the hood (?)
	)
}


void testIfMath() {
	assert_eval("if 0+2:{3*1} else 4+0", 3);

	skip( // no colon => no work. ok!
	assert_eval("if 2+0 {3*1} else {4*1}", 3);
			assert_eval("if 2+0 {3*1}", 3);

	)
	assert_eval("if(0*2):{3*1} else {4*1}", 4);

	assert_eval("if 2+0 then 3 else 4+0", 3);

//	assert_group("if 2+0 : 3 else 4+0", "(if (2+0) (3) (4+0))");
	assert_eval("if 2+0 : 3 else 4+0", 3);
	assert_eval("if 0*2:{3*1} else {4*1}", 4);
	assert_is("2+0",2)
	assert_eval("if 0*2:3*1", false);
	skip(
	assert_eval("if(2,then=3*1)", 3);
	assert_eval("if(0,then=3,else=4*1)", 4);
	assert_eval("if(1,then=3+0,else=4)", 3);
			)
	assert_eval("if 0*2:3 else {4*1}", 4);
	assert_eval("if 0*2 {3*1} else {4*1}", 4);
	assert_eval("if {0}:3 else 4+0", 4);
	assert_eval("if 0*2:3 else 4+0", 4);
	assert_eval("if 0*2:{3*1} else 4+0", 4);
	assert_eval("if(2*1):{3*1}", 3);
	assert_eval("if (2*1) {3*1} else 4+0", 3);
	assert_eval("if(2*1){3*1} else 4+0", 3);
	assert_eval("if(0*2){3*1} else 4+0", 4);
	assert_eval("if 0*2 {3*1} else {4*1}", 4);
	assert_eval("if (2*1) {3*1} else {4*1}", 3);
	assert_eval("if(2*1){3*1} else {4*1}", 3);
	assert_eval("if(0*2){3*1} else {4*1}", 4);
	assert_eval("if (0*2) {3*1} else {4*1}", 4);
	assert_eval("if(2*1):{3*1} else 4+0", 3);
	assert_eval("if(2*1):{3*1} else {4*1}", 3);
	assert_eval("if 0+2:3 else 4+0", 3);
	assert_eval("if 0+2:{3*1} else {4*1}", 3);
	assert_eval("if 0+2:3 else {4*1}", 3);
	assert_eval("if(0*2):{3*1}", false);
	assert_eval("if(0*2):{3*1} else 4+0", 4);
	assert_eval("if (0*2) {3*1}", false);
	assert_eval("if (0*2) {3*1} else 4+0", 4);
	assert_eval("if 1 then 0 else 4+0", (long) 0);
	assert_eval("if 0*2:{3*1} else 4+0", 4);
	assert_eval("if 0*2 {3*1}", false);
	assert_eval("4 or 3*1", 4);
	assert_eval("if (2*1) {3*1}", 3);
	assert_eval("if(2*1){3*1}", 3);
	assert_eval("if(0*2){3*1}", false);
	assert_eval("if 0+2:{3*1}", 3);
	assert_eval("if 0+2:3*1", 3);
	skip(
	assert_eval("if 0*2 {3*1} else 4+0", 4);
	assert_eval("if 2+0 {3*1} else 4+0", 3);
			)
	assert_eval("if(2,3,4)", 3);
	assert_eval("if({2},{3*1},{4*1})", 3);
	assert_eval("if(2*1){3*1}{4*1}", 3);
}



void testIfGt() {
	assert_eval("if(2<4):{3}", 3);
	assert_eval("1<0 or 3", 3);
	assert_eval("1<0 else 3", 3);
	assert_eval("4 or 3", 4);
	assert_eval("if (1<2) {3} else {4}", 3);
	skip(// maybe later: auto-group:
	assert_eval("if 1<2 {3} else {4}", 3);
			assert_eval("if 0>1 {3} else {4}", 4);
			assert_eval("if (0<1) {3} else {4}", 4);
			assert_eval("if 1<2 {3}", 3);
	)
	assert_eval("if (1<0) {3}", false);
	assert_eval("if (0<1) {3}", 3);

	assert_eval("if(3<0):{3} else {4}", 4);
	assert_eval("if 0>1 : {3} else {4}", 4);
	assert_eval("if 0>1 : 3 else {4}", 4);
	assert_eval("if 0>1 : 3 else 4", 4);
	assert_eval("if 0>1:3 else 4", 4);
	assert_eval("if 0>1:{3} else {4}", 4);
	assert_eval("if 0>1:3 else {4}", 4);

	assert_eval("if 0>1 {3} else {4}", 4);
	assert_eval("if 1<2 : 3 else 4", 3);
//	assert_eval("if 3<2 5 else 4", 4);

	assert_eval("if 0>1:3", false);
	assert_eval("if (2<3) {3} else 4", 3);
	assert_eval("if(2<4){3} else 4", 3);
	assert_eval("if(3<0){3} else 4", 4);

	assert_eval("if (2<3) {3} else {4}", 3);
	assert_eval("if(2<4){3} else {4}", 3);
	assert_eval("if(3<0){3} else {4}", 4);

	assert_eval("if(2<4):{3} else 4", 3);
	assert_eval("if(2<4):{3} else {4}", 3);
	assert_eval("if 1<2:{3} else 4", 3);
	assert_eval("if 1<2:3 else 4", 3);
	assert_eval("if 1<2:{3} else {4}", 3);
	assert_eval("if 1<2:3 else {4}", 3);
	assert_eval("if(3<0):{3}", false);
	assert_eval("if(3<0):{3} else 4", 4);
	assert_eval("if 1<2 then 3 else 4", 3);
//	assert_eval("2 then 3 else 4", 3);
	assert_eval("2 and 3 or 4", 3);
	assert_eval("1 and 0 or 4", 4);
	assert_eval("if 1 then 0 else 4", (long) 0);
	assert_eval("if 0>1:{3} else 4", 4);

	assert_eval("if (0<1) {3} else 4", 3);
	assert_eval("if 0>1 {3}", false);
//	assert_eval("4 else 3", 4);
	assert_eval("if (2<3) {3}", 3);
	assert_eval("if(2<4){3}", 3);
	assert_eval("if(3<0){3}", false);
	assert_eval("if 1<2:{3}", 3);
	assert_eval("if 1<2:3", 3);

	assert_eval("if 0>1 {3} else 4", 4);

//	assert_eval("if 1<2 , 3 , 4", 3);
//	assert_eval("if{2 , 3 , 4}", 3);
//	assert_eval("if 1<2 then 3 else 4", 3);
	skip( // esotheric
	assert_eval("if(2<4,3,4)", 3);
			assert_eval("if(3<{2},{3},{4})", 3);
			assert_eval("if(2<4){3}{4}", 3);
			assert_eval("if 1<2 {3} else 4", 3);

			assert_parses("if(3<condition=2,then=3,else=4)");
			check(result["condition"] == 2);
			check(result["else"] == 4);
			assert_eval("if(3<condition=2,then=3,else=4)", 3); // this is what happens under the hood (?)
	)
}

void testSwitch(){
//	todo if(1>0) ... innocent groups
	assert_is("{a:1 b:2}[a]",1)
	assert_is("{a:1+1 b:2}(a)",2)
	assert_is("x=a;{a:1 b:2}(x)",1)
	// functor switch(x,xs)=xs[x] or xs[default]
}

void testAllAngle() {
	testIf();
	testCall();
	skip(
	testSwitch();
			)
//	testFunctionParams();
}

void testAngle() {
	testAllAngle();
}