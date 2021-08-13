//
// Created by pannous on 12.06.20.
//

#include "Wasp.h" // parse
#include "Angle.h" // analyze
#include "Node.h"

#define assert_ast(α, β) if (!assert_equals_x(analyze(parse(α)),parse(β))){printf("%s != %s",#α,#β);backtrace_line();}
#define assert_eval(α, β) if (!assert_equals_x(eval(α),β)){printf("%s != %s",#α,#β);backtrace_line();}


void testFunctionParams() {
//	assert_equals(parse("f(x)=x*x").param->first(),"x");
	assert_equals("f(x)=x*x;f(3)", "9");// functions => angle!
}

void testOperatorBinding() {
	assert_ast("a and b", "and(a,b)");
}

void testCall() {
	assert_is("square 3", 9)
	assert_is("square(3)", 9)
	assert_is("square(1+2)", 9)
	assert_is("square 1+2", 9)
	assert_is("1+square(2+3)", 26)
	assert_is("1+square 2+3", 26)
	assert_is("1 + square 1+2", 10)
	skip(
			assert_is("square{i:3}", 9) //todo: match arguments!
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
	assert_is("2+0", 2)
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

void testSwitch() {
//	todo if(1>0) ... innocent groups
	assert_is("{a:1 b:2}[a]", 1)
	assert_is("{a:1+1 b:2}(a)", 2)
	assert_is("x=a;{a:1 b:2}(x)", 1)
	// functor switch(x,xs)=xs[x] or xs[default]
}

/*
#include "smart_types.h"
void testSmartTypes(){
	check(Node(0xC0000020)==' ');
	char* hi="Hello";
	strcpy2(&memoryChars[0x1000], hi);
	printf(">>>%s<<<", &memoryChars[0x1000]);
	check(Node(0x90001000)==hi);

	short typ=getSmartType(0x10000000);
	check(typ==0x1);
	printf("%08x", u'√');// ok 0x221a
	printf("%08x", U'√');// ok 0x221a
	printf("%08x", L'√');// ok 0x221a
//	printf("%08x", u'𒈚');// too small: character too large for enclosing character literal type
	printf("%08x", U'𒈚');// ok 0x1221a
	printf("%08x", L'𒈚');// ok 0x1221a
	check(Node((spointer)0x00000009)==9);
	check(Node(0xC000221a)=="√");
	check(Node(0xC000221a)==String(u'√'));
	check(Node(0xC000221a)==String(U'√'));
	check(Node(0xC000221a)==String(L'√'));
	check(Node(0xC001221A)==String(U'𒈚'));
	check(Node(0xC001221A)=="𒈚");

//	check(Node(0xD808DE1A)==U'𒈚');
	typ=getSmartType(0xC0000000);
	check(typ==0xC);
	check(Node(0xC0000020)==' ');

	check(Node(0xFFFFFFFF)==-1);
}
*/
void nl() {
	log("\n");
}

//Prescedence typo for Precedence
void testLogicPrecedence() {
	check(precedence("and") > 1);
	check(precedence("and") < precedence("or"));
	assert_is("true", true);
	assert_is("false", false);
	Node ok = parse("true or true");
//	const Node &ja = ok.interpret();// Undefined symbols if not compiled with Interpret.cpp
//	print(ja);
	assert_is("true or true", true);
	assert_is("true or false", true);
	assert_is("true and false", false);
	assert_is("true or true and false", true);
	assert_is("true or false and true", true);
	assert_is("true or false and false", true);
	assert_is("false or true and false", false);
	assert_is("false or true and false", false);
}


void testAllAngle() {
	testLogicPrecedence();
//	testSmartTypes();
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