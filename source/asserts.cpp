#pragma once

#ifndef WASM
//#include <codecvt> // utf8 magic ...?
//#include <libc.h>
#include "Node.h"
#include "Angle.h"
#include "tests.h"

#endif

//#DANGER!!! DONT printf(#test) DIRECTLY if #test contains "%s" => VERY SUBTLE BUGS!!!

// todo assert_is ≠ assert_run == assert_emit_with_wasm_runtime!


//#define check(test) if(test){printf("OK check passes %s\n",#test);}else{printf("NOT PASSING %s\n%s:%d\n",#test,__FILE__,__LINE__);exit(1);}
//#define check(test) if(test){log("OK check passes: ");print(#test);}else{printf("NOT PASSING %s\n%s:%d\n",#test,__FILE__,__LINE__);exit(1);}
//#define check(test) if(test){log("OK check passes: ");print(#test);}else{printf("NOT PASSING %s\n%s:%d\n",#test,__FILE__,__LINE__);exit(1);}


bool assert_equals_x(String a, String b, char *context) {
	if (a == b) printf("OK %s==%s %s\n", a.data, b.data, context);
	else printf("\nFAILED assert_equals!\n %s should be %s %s\n", a.data, b.data, context);
	return a == b;
}

bool assert_equals_x(String *a, String b, char *context = "") {
	return assert_equals_x(*a, b, context);
}

bool assert_equals_x(Node &a, char *b, char *context = "") {
	if (a.name != b)printf("\nFAILED assert_equals! %s should be %s %s\n", a.name.data ? a.name.data : "???", b, context);
	else printf("OK %s==%s %s\n", a.name.data, b, context);
	return a == b;
}


bool assert_equals_x(Node a, const char *b, char *context = "") {
	if (a.name != b)printf("\nFAILED assert_equals! %s should be %s %s\n", a.name.data, b, context);
	else printf("OK %s==%s %s\n", a.name.data, b, context);
	return a == b;
}


bool assert_equals_x(Node a, int b, char *context = "") {
	if (a != Node(b))print("\nFAILED assert_equals! %lld should be %d %s\n"s % a.value.longy % b % context);
	else if (a.kind == reals) printf("OK %f==%f\n", a.value.real, (double) b);
	else printf("OK %lld==%d\n", a.value.longy, b);
	return a == b;
}

// WTF why is char* unexplicitly cast to bool!?!
bool assert_equals_x(Node a, bool b, char *context = "") {
	if (a != Node(b))print("\nFAILED assert_equals! %lld should be %d %s\n"s % a.value.longy % b % context);
	else printf("OK %lld==%d\n", a.value.longy, b);
	return a == b;
}

bool assert_equals_x(Node &a, double b, char *context) {
	if (a != Node(b))print("\nFAILED assert_equals! %lld should be %f %s\n"s % a.value.longy % b % context);
	else printf("OK %f==%f\n", a.value.real, b);
	return a == b;
}

bool assert_equals_x(Node a, double b, char *context = "") {
	if (a != Node(b))print("\nFAILED assert_equals! %lld should be %f %s\n"s % a.value.longy % b % context);
	else printf("OK %f==%f\n", a.value.real, b);
	return a == b;
}

//bool assert_equals_x(Node &a, long b, char *context = "") {
//	if (!(a == b))printf("\nFAILED assert_equals! %s should be %d %s\n"s, a.name, b, context);
//	else printf("OK %ld==%ld %s\n", a.value.longy, b, context);
//	return a == b;
//}


bool assert_equals_x(Node a, long b, char *context = "") {
	if (!(a == b))print("\nFAILED assert_equals! %s %lld should be %ld %s\n"s % a.name.data % a.value.longy % b % context);
	else print("OK %lld==%ld %s\n"s % a.value.longy % b % context);
	return a == b;
}


bool assert_equals_x(Node a, String b, char *context = "") {
	String &name = a.name;
	bool ok = name == b or a == b; //  b == name or  !(name != b and b != a.value.string;)
	if (ok)
		printf("OK %s==%s %s\n", name.data, b.data, context);
	else
		printf("\nFAILED assert_equals! %s should be %s %s\n", name.data, b.data, context);
	return ok;
}


bool assert_equals_x(Node a, Node b, char *context = "") {
	//	check(NIL.value.longy == 0);// WHEN DOES IT BREAK??
	char *as = a.serialize().data;
	char *bs = b.serialize().data;
	if (a == b) {
		printf("OK %s == %s   %s\n", as, bs, context);
	} else
		printf("\nFAILED assert_equals! %s should be %s %s\n", as, bs, context);
	printf("%s != %s\n", as, bs);
	return a == b;
}

bool assert_equals_x(Node *a, const Node *b, char *context = "") {
	if (!a) {
		if (b)printf("\nFAILED assert_equals! NULL should be %s %s\n", b->serialize().data, context);
		return !b;
	}
	return assert_equals_x(*a, *b, context);
}

bool assert_equals_x(Node *a, Node *b, char *context = "") {
	if (!a)return !b;
	if (!b)return !a;
	return assert_equals_x(*a, *b, context);
}

//bool assert_equals(chars a, chars b, char *context = "") {
//	if (a != b)// err
//		printf("F\nAILED assert_equals! %s should be %s %s\n"s % a % b % context);
//	else printf("OK %s==%s %s\n"s % a % b % context);
//	return a == b;
//}

bool assert_equals_x(long a, long b, char *context) {
	if (a != b)print("\nFAILED assert_equals! %d should be %d %s\n"s % a % b % context);
	else print("OK %ld==%ld %s\n"s % a % b % context);
	return a == b;
}

bool assert_equals_x(int a, int b, char *context = "") {
	if (a != b)print("\nFAILED assert_equals! %d should be %d %s\n"s % a % b % context);
	else print("OK %d==%d %s\n"s % a % b % context);
	return a == b;
}


bool assert_equals_x(double a, double b, char *context) {
	auto ok = similar(a, b);
	if (!ok)print("\nFAILED assert_equals!\n %f should be %f %s\n"s % a % b % context);
	else print("OK %f==%f %s\n"s % a % b % context);
	return ok;
}

bool assert_equals_x(float a, float b, char *context = "") {
	auto ok = similar(a, b);
	if (!ok)print("\nFAILED assert_equals!\n %f should be %f %s\n"s % a % b % context);
	else print("OK %f==%f %s\n"s % a % b % context);
	return ok;
}

//# DEFINES CAN MESS WITH LOCALS!! so use α, β

//bool assert_isx(char *mark, Node expect);
//bool assert_isx(char *mark, chars expect);

bool assert_isx(char *mark, Node expect) {
	try {
		Node left = eval(mark);
		if (left.kind == reals or expect.kind == reals)
			return assert_equals_x(left.floate(), expect.floate(), mark);
		if (left.kind == longs or expect.kind == longs) {
			long b = expect.numbere();
			return assert_equals_x(left.numbere(), b, mark);
		}
		if (left != expect)
			//			breakpoint_helper
			if (left != expect)// Redundant for Breakpoint ;)
				printf("FAILED %s ≠ %s\n", left.name.data, expect.name.data);
		return left == expect;
	} catch (SyntaxError *err) {
		printf("\nERROR IN TEST\n");
		printf("%s", err->data);
	} catch (String *err) {
		printf("\nERROR IN TEST\n");
		printf("%s", err->data);
	} catch (chars err) {
		printf("\nERROR IN TEST\n");
		printf("%s\n", err);
	} catch (...) {
		raise("\nERROR IN TEST (no further data):\n");
	}
	return false;
}

bool assert_isx(char *mark, chars expect) {
	return assert_isx(mark, Node(expect));// explicit conversion ok!
}

bool assert_isx(char *mark, int expect) {
	return assert_isx(mark, Node(expect));// explicit conversion ok!
}

bool assert_isx(char *mark, long expect) {
	return assert_isx(mark, Node(expect));// explicit conversion ok!
}

bool assert_isx(char *mark, double expect) {
	return assert_isx(mark, Node(expect));// explicit conversion ok!
}

bool assert_isx(char *mark, bool expect) {
	return assert_isx(mark, Node(expect));
}


Node assert_parsesx(chars mark) {
	try {
		result = parse(mark);
		print(result);
		return result;
	} catch (chars err) {
		printf("\nTEST FAILED WITH ERROR\n");
		printf("%s\n", err);
	} catch (String &err) {
		printf("\nTEST FAILED WITH ERRORs\n");
		printf("%s\n", err.data);
	} catch (SyntaxError &err) {
		printf("\nTEST FAILED WITH SyntaxError\n");
		printf("%s\n", err.data);
	} catch (...) {
		printf("\nTEST FAILED WITH UNKNOWN ERROR (maybe POINTER String*)? \n");
	}
	return ERROR;// DANGEEER 0 wrapped as Node(int=0) !!!
}
//#define assert_parses(wasp) result=assert_parsesx(wasp);if(result==NIL){printf("\n%s:%d\n",__FILE__,__LINE__);exit(1);}
#define assert_parses(mark) result=assert_parsesx(mark);if(result==ERROR){printf("NOT PARSING %s\n%s:%d\n",#mark,__FILE__,__LINE__);exit(1);}

#define skip(test) printf("\nSKIPPING %s\n%s:%d\n",#test,__FILE__,__LINE__);
#define todo_emit(ɣ) if(not eval_via_emit){ɣ;}else printf("skipping emit case %s",#ɣ);
#define skip_wasm(ɣ) if(not eval_via_emit){ɣ;}else printf("skipping emit case %s",#ɣ);
//printf("\nSKIPPING %s\n%s:%d\n",#test,__FILE__,__LINE__);

bool ok;

// MACRO to catch the line number. WHY NOT WITH TRACE? not precise:   testMath() + 376
#define assert_is(mark, result) \
printf("TEST %s==%s\n",#mark,#result); \
printf("%s:%d\n",__FILE__,__LINE__);\
ok=assert_isx(mark,result);\
if(ok)printf("PASSED %s==%s\n",#mark,#result);\
else{printf("FAILED %s==%s\n",#mark,#result);\
printf("%s:%d\n",__FILE__,__LINE__);exit(1);}


// for better readability, not (yet) semantic
String normSerialization(String input) {
	input = input.replaceAll("; ", ";");
	input = input.replaceAll(" ;", ";");
	input = input.replaceAll("( ", "(");
	input = input.replaceAll("  ", " ");
	input = input.replaceAll(":", "=");// danger!
//	input = input.replaceAll(" ", "");// VERY danger! (1 2 3) 123
	return input;
}


void assertSerialize(const char *input) {
	Node parsed = parse(input);
	const String &serialized = parsed.serialize();
	Node parsed2 = parse(serialized);
	bool equalsX = assert_equals_x(parsed, parsed2);
	if (!equalsX) {
		print(parsed);
		print("≠");
		print(parsed2);

		print("----------------");
		print(input);
		print("≠");
		print(serialized);
		print("----------------");
		print(normSerialization(input));
		print("≠");
		print(normSerialization(serialized));
		error("Serialization Error");
	}
}

