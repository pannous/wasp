//
// Created by pannous on 12.06.20.
//

#include "Wasp.h" // parse
#include "Angle.h" // analyze

#define assert_ast(α, β) if (!assert_equals_x(analyze(parse(α)),parse(β))){printf("%s != %s",#α,#β);backtrace_line();}
#define assert_eval(α, β) if (!assert_equals_x(eval(α),β)){printf("%s != %s",#α,#β);backtrace_line();}


void testFunctionParams(){
//	assert_equals(parse("f(x)=x*x").param->first(),"x");
	assert_equals("f(x)=x*x;f(3)","9");// functions => angle!
}

void testOperatorBinding(){
	assert_ast("a and b", "and(a,b)");
}

void testIf(){

	assert_eval("if (2) {3} else 4", 3);
	assert_eval("if(2){3} else 4", 3);
	assert_eval("if(0){3} else 4", 4);

	assert_eval("if 2 {3} else {4}", 3);
	assert_eval("if 0 {3} else {4}", 4);
	assert_eval("if (2) {3} else {4}", 3);
	assert_eval("if(2){3} else {4}", 3);
	assert_eval("if(0){3} else {4}", 4);

//skip(
	assert_eval("if (0) {3} else {4}", 4);
	assert_eval("if(2):{3} else 4", 3);
	assert_eval("if(2):{3} else {4}", 3);
//		)
	assert_eval("if 2:{3} else 4", 3);
	assert_eval("if 2:3 else 4", 3);
	assert_eval("if 2:{3} else {4}", 3);
	assert_eval("if 2:3 else {4}", 3);
	assert_eval("if 2 {3}", 3);
	assert_eval("if(0):{3}", false);
	assert_eval("if(0):{3} else 4", 4);
	assert_eval("if (0) {3}", false);
	assert_eval("if 2 then 3 else 4", 3);
			assert_eval("if (0) {3} else 4", 4);
//	assert_eval("2 then 3 else 4", 3);
	assert_eval("2 and 3 or 4", 3);
	assert_eval("1 and 0 or 4", 4);
	assert_eval("if 1 then 0 else 4", (long)0);
			assert_eval("if 0:{3} else 4", 4);

	skip(
	assert_eval("if 0:3 else 4", 4);
			assert_eval("if 0:3 else 4", 4);
			assert_eval("if(2):{3}", 3);
	assert_eval("if 0:3", false);
	)

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

	skip(
	assert_eval("if(0):{3} else {4}", 4);
	assert_eval("if 0:{3} else {4}", 4);
	assert_eval("if 0:3 else {4}", 4);
			assert_eval("if(condition=2,then=3)", 3);

	)

//	assert_eval("if 2 , 3 , 4", 3);
//	assert_eval("if{2 , 3 , 4}", 3);
	assert_eval("if 2 : 3 else 4", 3);
//	assert_eval("if 2 then 3 else 4", 3);
	assert_eval("if(2,3,4)", 3);
	assert_eval("if({2},{3},{4})", 3);
	assert_eval("if(2){3}{4}", 3);
	skip(
	assert_parses("if(condition=2,then=3,else=4)");
	check(result["condition"]==2);
	check(result["else"]==4);
	assert_eval("if(condition=2,then=3,else=4)", 3); // this is what happens under the hood (?)
			)
}

void testAllAngle(){
	testIf();
//	testFunctionParams();
}

void testAngle() {
	testAllAngle();
}