#pragma once

#ifndef WASM
//#include <codecvt> // utf8 magic ...?
//#include <libc.h>
#include "Node.h"
#include "Angle.h"
#include "tests.h"

#endif

#include "asserts.h"

//#DANGER!!! DONT printf(#test) DIRECTLY if #test contains "%s" => VERY SUBTLE BUGS!!!

// todo assert_is ≠ assert_run == assert_emit_with_wasm_runtime!


bool assert_equals_x(String a, String b, chars context) {
    if (a == b) printf(" OK %s==%s %s\n", a.data, b.data, context);
    else printf("FAILED assert_equals!\n %s should be %s %s\n", a.data, b.data, context);
    return a == b;
}

bool assert_equals_x(String *a, String b, chars context = "") {
    return assert_equals_x(*a, b, context);
}

bool assert_equals_x(Node &a, char *b, chars context = "") {
    if (a.name != b)
        printf("FAILED assert_equals! %s should be %s %s\n", a.name.data ? a.name.data : "???", b, context);
    else printf(" OK %s==%s %s\n", a.name.data, b, context);
    return a == b;
}


bool assert_equals_x(Node a, const char *b, chars context = "") {
    if (a.name != b)printf("FAILED assert_equals! %s should be %s %s\n", a.name.data, b, context);
    else printf(" OK %s==%s %s\n", a.name.data, b, context);
    return a == b;
}


bool assert_equals_x(Node a, int b, chars context = "") {
    if (a != Node(b))
        print("\nFAILED assert_equals! %lld should be %d %s\n"s % a.value.longy % b % context);
    else if (a.kind == reals) printf(" OK %f==%f\n", a.value.real, (double) b);
    else
        printf(" OK %lld==%lld\n", a.value.longy, (int64) b);
    return a == b;
}

bool assert_equals_x(Node a, char b, chars context = "") {
    if (a != Node(b))
        print("\nFAILED assert_equals! %lld should be %c %s\n"s % a.value.longy % b % context);
    else
        printf("OK %c==%c\n", (char) a.value.longy, b);
    return a == b;
}

bool assert_equals_x(Node a, wchar_t b, chars context = "") {
    if (a != Node(b))
        print("\nFAILED assert_equals! %lld should be %c %s\n"s % a.value.longy % b % context);
    else
        printf("OK %c==%c\n", (char) a.value.longy, b);
    return a == b;
}

bool assert_equals_x(Node a, char16_t b, chars context = "") {
    if (a != Node(b))
        print("\nFAILED assert_equals! %lld should be %c %s\n"s % a.value.longy % b % context);
    else
        printf(" OK %c==%c\n", (char) a.value.longy, b);
    return a == b;
}

bool assert_equals_x(Node a, codepoint b, chars context = "") {
    if (a != Node(b))
        print("\nFAILED assert_equals! %lld should be %c %s\n"s % a.value.longy % b % context);
    else
        printf(" OK %s==%c\n", a.serialize().data, b);
    return a == b;
}

// WTF why is char* unexplicitly cast to bool!?!
bool assert_equals_x(Node a, bool b, chars context = "") {
    if (a != Node(b))print("\nFAILED assert_equals! %lld should be %d %s\n"s % a.value.longy % b % context);
    else printf(" OK %d==%d\n", (int) a.value.longy, (int) b);
    return a == b;
}


bool assert_equals_x(Node &a, wchar_t b, chars context) {
    if (a != Node(b))print("\nFAILED assert_equals! %s should be %c %s\n"s % a.string().data % b % context);
    else printf(" OK %s==%c\n", a.string().data, b);
    return a == b;
}

bool assert_equals_x(Node &a, double b, chars context) {
    if (a != Node(b))print("\nFAILED assert_equals! %lld should be %f %s\n"s % a.value.longy % b % context);
    else printf(" OK %f==%f\n", a.value.real, b);
    return a == b;
}

bool assert_equals_x(Node a, double b, chars context = "") {
    if (a != Node(b))
        print("\nFAILED assert_equals! %f should be %f %s\n"s % a.value.real % b % context);
    else if (a.kind == longs)
        printf(" OK %lld==%lld\n", (int64) a.value.longy, (int64) b);
    else
        printf(" OK %f==%f\n", a.value.real, b);
    return a == b;
}

//bool assert_equals_x(Node &a, int64 b, chars context = "") {
//	if (!(a == b))prinft("FAILED assert_equals! %s should be %d %s\n"s, a.name, b, context);
//	else printf(" OK %ld==%ld %s\n", a.value.longy, b, context);
//	return a == b;
//}


bool assert_equals_x(Node a, int64 b, chars context = "") {
    if (!(a == b))
        print("\nFAILED assert_equals! %s %lld should be %ld %s\n"s % a.name.data % a.value.longy % b % context);
    else print(" OK %lld==%ld %s\n"s % a.value.longy % b % context);
    return a == b;
}


bool assert_equals_x(Node a, String b, chars context = "") {
    String &name = a.name;
    bool ok = name == b or a == b; //  b == name or  !(name != b and b != a.value.string;)
    if (ok)
        printf(" OK %s==%s %s\n", name.data, b.data, context);
    else
        printf("FAILED assert_equals! %s should be %s %s\n", name.data, b.data, context);
    return ok;
}


bool assert_equals_x(Node a, Node b, chars context = "") {
    //	check(NIL.value.longy == 0);// WHEN DOES IT BREAK??
    char *as = a.serialize().data;
    char *bs = b.serialize().data;
    if (a == b) {
        printf(" OK %s == %s   %s\n", as, bs, context);
    } else {
        printf("FAILED assert_equals! %s should be %s %s\n", as, bs, context);
        printf("%s != %s\n", as, bs);
    }
    return a == b;
}

bool assert_equals_x(Node *a, const Node *b, chars context = "") {
    if (!a) {
        if (b)printf("FAILED assert_equals! NULL should be %s %s\n", b->serialize().data, context);
        return !b;
    }
    return assert_equals_x(*a, *b, context);
}

bool assert_equals_x(Node *a, Node *b, chars context = "") {
    if (!a)return !b;
    if (!b)return false;
    return assert_equals_x(*a, *b, context);
}

//bool assert_equals(chars a, chars b, chars context = "") {
//	if (a != b)// err
//		printf("F\nAILED assert_equals! %s should be %s %s\n"s % a % b % context);
//	else printf(" OK %s==%s %s\n"s % a % b % context);
//	return a == b;
//}

bool assert_equals_x(int64 a, int64 b, chars context) {
    if (a != b)
        print("\nFAILED assert_equals! %d should be %d %s\n"s % a % b % context);
    else print(" OK %ld==%ld %s\n"s % a % b % context);
    return a == b;
}

bool assert_equals_x(int a, int b, chars context = "") {
    if (a != b)print("\nFAILED assert_equals! %d should be %d %s\n"s % a % b % context);
    else print(" OK %d==%d %s\n"s % a % b % context);
    return a == b;
}

bool assert_equals_x(Kind a, Kind b, chars context = "") {
    if (a != b)print("\nFAILED assert_equals! %d should be %d %s\n"s % a % b % context);
    else print(" OK %d==%d %s\n"s % a % b % context);
    return a == b;
}


bool assert_equals_x(double a, double b, chars context) {
    auto ok = similar(a, b);
    if (!ok)print("\nFAILED assert_equals!\n %f should be %f %s\n"s % a % b % context);
    else print(" OK %f==%f %s\n"s % a % b % context);
    return ok;
}

bool assert_equals_x(float a, float b, chars context = "") {
    auto ok = similar(a, b);
    if (!ok)print("\nFAILED assert_equals!\n %f should be %f %s\n"s % a % b % context);
    else print(" OK %f==%f %s\n"s % a % b % context);
    return ok;
}

//# DEFINES CAN MESS WITH LOCALS!! so use α, β

//bool assert_isx(char *mark, chars expect);

bool assert_isx(chars mark, Node expect) {
    try {
        Node left = eval(mark);
        if (left.kind == reals or expect.kind == reals)
            return assert_equals_x(left.floate(), expect.floate(), mark);
        if (left.kind == longs or expect.kind == longs) {
            int64 b = expect.numbere();
            return assert_equals_x(left.numbere(), b, mark);
        }
        if (left != expect)
            //			breakpoint_helper
            if (left != expect)// Redundant for Breakpoint ;)
                printf("FAILED %s ≠ %s\n", left.name.data, expect.name.data);
        return left == expect;
    } catch (SyntaxError *err) {
        print("ERROR IN TEST\n");
        printf("%s", err->data);
    } catch (String *err) {
        print("ERROR IN TEST\n");
        printf("%s", err->data);
    } catch (chars err) {
        print("ERROR IN TEST\n");
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

bool assert_isx(char *mark, int64 expect) {
    return assert_isx(mark, Node(expect));// explicit conversion ok!
}

bool assert_isx(char *mark, double expect) {
    return assert_isx(mark, Node(expect));// explicit conversion ok!
}

bool assert_isx(char *mark, bool expect) {
    return assert_isx(mark, Node(expect));
}

//#define assert_parses(wasp) result=assert_parsesx(wasp);if(result==NIL){print("%s:%d\n",__FILE__,__LINE__);proc_exit(1);}
// ⚠️ CAREFUL parses in DATA_MODE !

#if WASM
#define skip(test)
#else
#define skip(test) print("SKIPPING");print(#test);debug_line();
#endif
#define todo_emit(ɣ) if(not eval_via_emit){ɣ;}else printf("skipping emit case %s",#ɣ);
#define skip_wasm(ɣ) if(not eval_via_emit){ɣ;}else printf("skipping emit case %s",#ɣ);
//print("SKIPPING %s\n%s:%d\n",#test,__FILE__,__LINE__);

bool ok;

extern List<String> done;


#if EMSCRIPTEN
#define assert_is(α, β) printf("%s\n%s:%d\n",α,__FILE__,__LINE__);if (!assert_isx(α,β)){printf("%s != %s",#α,#β);backtrace_line();}
#elif MY_WASM and not EMSCRIPTEN // todo WHY does if MY_WASM not work??
#define assert_is(α, β) if(!done.has(α)){ done.add(α);assert_expect(new Node(β));eval(α);async_yield();};
#else
//// MACRO to catch the line number. WHY NOT WITH TRACE? not precise:   testMath() + 376
#define assert_is(mark, result) \
printf("TEST %s==%s\n",#mark,#result); \
debug_line();\
ok=assert_isx(mark,result);\
if(ok)printf("PASSED %s==%s\n",#mark,#result);\
else{printf("FAILED %s==%s\n",#mark,#result); \
backtrace_line()}
#endif


#define assert_eval assert_is
//#define assert_ast(α, β) if (!assert_equals_x(analyze(parse(α)),parse(β))){printf("%s != %s",#α,#β);backtrace_line();}
//#define assert_eval(α, β) if (!assert_equals_x(eval(α),β)){printf("%s != %s",#α,#β);backtrace_line();}



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


