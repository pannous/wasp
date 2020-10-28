//
// Created by pannous on 12.06.20.
//

#include "Wasp.h" // parse
#include "Angle.h" // analyze

#define assert_ast(α, β) if (!assert_equals_x(analyze(parse(α)),parse(β))){printf("%s != %s",#α,#β);backtrace_line();}


void testFunctionParams(){
	assert_equals(parse("f(x)=x*x").param->first(),"x");
	assert_equals("f(x)=x*x;f(3)","9");// functions => angle!
}

void testOperatorBinding(){
	assert_ast("a and b", "and(a,b)");
}

void testAllAngle(){
	testFunctionParams();
}

void testAngle() {
	testAllAngle();
}