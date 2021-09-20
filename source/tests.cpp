#include "Node.h"
#include "Wasp.h"
#include "Angle.h"
#include "String.h"
#include "Map.h"

#undef assert // <cassert> / <assert.h>

#ifndef WASM

#include <codecvt> // utf8 magic ...?

#endif

Node &result = *new Node();

//#DANGER!!! DONT printf(#test) DIRECTLY if #test contains "%s" => VERY SUBTLE BUGS!!!
#define check(test) if(test){log("\nOK check passes: ");printf("%s\n",#test);}else{ \
printf("\nNOT PASSING: "); log(#test);printf("%s:%d\n",__FILE__,__LINE__); \
exit(1);}

#undef assert // assert.h:92 not as good!
#define assert(condition) try{\
   if((condition)==0){printf("\n%s\n",#condition);error("assert FAILED");}else printf("\nassert OK: %s\n",#condition);\
   }catch(chars m){printf("\n%s\n%s\n%s:%d\n",m,#condition,__FILE__,__LINE__);exit(1);}


//#define check(test) if(test){printf("OK check passes %s\n",#test);}else{printf("NOT PASSING %s\n%s:%d\n",#test,__FILE__,__LINE__);exit(1);}
//#define check(test) if(test){log("OK check passes: ");log(#test);}else{printf("NOT PASSING %s\n%s:%d\n",#test,__FILE__,__LINE__);exit(1);}
//#define check(test) if(test){log("OK check passes: ");log(#test);}else{printf("NOT PASSING %s\n%s:%d\n",#test,__FILE__,__LINE__);exit(1);}

#define backtrace_line() {printf("\n%s:%d\n",__FILE__,__LINE__);exit(0);}
//#define backtrace_line(msg) {printf("\n%s\n%s:%d\n",#msg,__FILE__,__LINE__);exit(1);}
#define check_eq assert_equals


bool assert_equals_x(String a, String b, char *context = "") {
	if (a == b) printf("OK %s==%s %s\n", a.data, b.data, context);
	else printf("\nFAILED assert_equals!\n %s should be %s %s\n"s, a.data, b.data, context);
	return a == b;
}

bool assert_equals_x(String *a, String b, char *context = "") {
	return assert_equals_x(*a, b, context);
}

bool assert_equals_x(Node &a, char *b, char *context = "") {
	if (a.name != b)printf("\nFAILED assert_equals! %s should be %s %s\n"s, a.name, b, context);
	else printf("OK %s==%s %s\n", a.name.data, b, context);
	return a == b;
}


bool assert_equals_x(Node a, const char *b, char *context = "") {
	if (a.name != b)printf("\nFAILED assert_equals! %s should be %s %s\n"s, a.name, b, context);
	else printf("OK %s==%s %s\n", a.name.data, b, context);
	return a == b;
}


bool assert_equals_x(Node a, int b, char *context = "") {
	if (a != Node(b))printf("\nFAILED assert_equals! %d should be %d %s\n"s, a.value.longy, b, context);
	else printf("OK %ld==%ld\n", a.value.longy, (long) b);
	return a == b;
}

// WTF why is char* unexplicitly cast to bool!?!
bool assert_equals_x(Node a, bool b, char *context = "") {
	if (a != Node(b))printf("\nFAILED assert_equals! %d should be %d %s\n"s, a.value.longy, b, context);
	else printf("OK %ld==%ld\n", a.value.longy, (long) b);
	return a == b;
}

bool assert_equals_x(Node &a, double b, char *context = "") {
	if (a != Node(b))printf("\nFAILED assert_equals! %f should be %f %s\n"s, a.value.longy, b, context);
	else printf("OK %f==%f\n", a.value.real, b);
	return a == b;
}

bool assert_equals_x(Node a, double b, char *context = "") {
	if (a != Node(b))printf("\nFAILED assert_equals! %f should be %f %s\n"s, a.value.longy, b, context);
	else printf("OK %f==%f\n", a.value.real, b);
	return a == b;
}

//bool assert_equals_x(Node &a, long b, char *context = "") {
//	if (!(a == b))printf("\nFAILED assert_equals! %s should be %d %s\n"s, a.name, b, context);
//	else printf("OK %ld==%ld %s\n", a.value.longy, b, context);
//	return a == b;
//}


bool assert_equals_x(Node a, long b, char *context = "") {
	if (!(a == b))printf("\nFAILED assert_equals! %s %d should be %d %s\n"s, a.name, a.value.longy, b, context);
	else printf("OK %ld==%ld %s\n", a.value.longy, b, context);
	return a == b;
}


bool assert_equals_x(Node a, String b, char *context = "") {
	String &name = a.name;
	bool ok = name == b or a == b; //  b == name or  !(name != b and b != a.value.string;)
	if (ok)
		printf("OK %s==%s %s\n", name.data, b.data, context);
	else
		printf("\nFAILED assert_equals! %s should be %s %s\n"s, name.data, b, context);
	return ok;
}


bool assert_equals_x(Node a, Node b, char *context = "") {
//	check(NIL.value.longy == 0);// WHEN DOES IT BREAK??
	if (a == b) {
		if (a.name and b.name)
			printf("OK %s==%s %s\n", a.name.data, b.name.data, context);
	} else
		printf("\nFAILED assert_equals! %s should be %s %s\n"s, a, b, context);
	return a == b;
}

//bool assert_equals(chars a, chars b, char *context = "") {
//	if (a != b)// err
//		printf("F\nAILED assert_equals! %s should be %s %s\n"s % a % b % context);
//	else printf("OK %s==%s %s\n"s % a % b % context);
//	return a == b;
//}

bool assert_equals_x(long a, long b, char *context = "") {
	if (a != b)log("\nFAILED assert_equals! %d should be %d %s\n"s % a % b % context);
	else printf("OK %ld==%ld %s\n", a, b, context);
	return a == b;
}

bool assert_equals_x(int a, int b, char *context = "") {
	if (a != b)log("\nFAILED assert_equals! %d should be %d %s\n"s % a % b % context);
	else printf("OK %d==%d %s\n", a, b, context);
	return a == b;
}


inline float abs_(float x)

noexcept {
return x > 0 ? x : -
x;
}

bool assert_equals_x(float a, float b, char *context = "") {
	float epsilon = abs_(a + b) / 100000.;// 𝕚𝚤:=-1
	bool ok = a == b or abs_(a - b) <= epsilon;
	if (!ok)log("\nFAILED assert_equals!\n %f should be %f %s\n"s % a % b % context);
	else log("OK %f==%f %s\n"s % a % b % context);
	return ok;
}

//# DEFINES CAN MESS WITH LOCALS!! so use α, β
#define assert_equals(α, β) if (!assert_equals_x(α,β)){printf("%s != %s",#α,#β);backtrace_line();}

//bool assert_isx(char *mark, Node expect);
//bool assert_isx(char *mark, chars expect);

bool assert_isx(char *mark, Node expect) {
	try {
		Node left = Wasp::eval(mark);
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
		log(result);
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


// MACRO to catch the line number. WHY NOT WITH TRACE? not precise:   testMath() + 376
#define assert_is(mark, result) {\
    printf("TEST %s==%s\n",#mark,#result); \
    printf("%s:%d\n",__FILE__,__LINE__);\
    bool ok=assert_isx(mark,result);\
    if(ok)printf("PASSED %s==%s\n",#mark,#result);\
    else{printf("FAILED %s==%s\n",#mark,#result);\
    printf("%s:%d\n",__FILE__,__LINE__);exit(1);}\
}


#include "test_angle.cpp"
#include "test_wast.cpp"
#include "test_wasm.cpp"

void testModernCpp() {
	auto αα = 1. * 2;
	printf("%f", αα);// lol
}

void testDeepCopyBug() {
	chars source = "{c:{d:123}}";
	assert_parses(source);
	check(result["d"] == 123);
}

void testDeepCopyDebugBugBug() {
	testDeepCopyBug();
	chars source = "{deep{a:3,b:4,c:{d:true}}}";
	assert_parses(source);
	check(result.name == "deep");
	result.log();
	Node &c = result["deep"]['c'];
	Node &node = c['d'];
	assert_equals(node.value.longy, (long) 1);
	assert_equals(node, (long) 1);
}

void testDeepCopyDebugBugBug2() {
//	chars source = "{deep{a:3,b:4,c:{d:123}}}";
	chars source = "{deep{c:{d:123}}}";
	assert_parses(source);
	Node &c = result["deep"]['c'];
	Node &node = c['d'];
	assert_equals(node.value.longy, (long) 123);
	assert_equals(node, (long) 123);
}


void testEmitter() {
#ifndef RUNTIME_ONLY
	Node node = Node(42);
	Code &code = emit(node);
	int result = code.run();
	check(result == 42);
#endif
}

void testNetBase() {
	warn("NETBASE OFFLINE");
	if (1 > 0)return;
	chars url = "http://de.netbase.pannous.com:8080/json/verbose/2";
//	log(url);
	chars json = fetch(url);
//	log(json);
	Node result = parse(json);
	Node results = result["results"];
//	Node Erde = results[0];// todo : EEEEK, auto flatten can BACKFIRE! results=[{a b c}] results[0]={a b c}[0]=a !----
	Node Erde = results;
	assert(Erde.name == "Erde" or Erde["name"] == "Erde");
	Node &statements = Erde["statements"];
	assert(statements.length >= 1); // or statements.value.node->length >=
	assert(result["query"] == "2");
	assert(result["count"] == "1");
	assert(result["count"] == 1);

//	skip(
//			 );
	assert(Erde["name"] == "Erde");
//	assert(Erde.name == "Erde");
	assert(Erde["id"] == 2);// todo : auto numbers when?
	assert(Erde["kind"] == -104);
//	assert(Erde.id==2);
}

void testDivDeep() {
	Node div = parse("div{ span{ class:'bold' 'text'} br}");
	Node &node = div["span"];
	node.log();
	assert(div["span"].length == 2);
	assert(div["span"]["class"] == "bold");
}

void testDivMark() {
	polish_notation = true;
	Node div = parse("{div {span class:'bold' 'text'} {br}}");
	Node &span = div["span"];
	span.log();
	assert(span.length == 2);
	assert(span["class"] == "bold");
	polish_notation = false;
}

void testDiv() {
	Node result = parse("div{ class:'bold' 'text'}");
	result.log();
	assert(result.length == 2);
	assert(result["class"] == "bold");
	testDivDeep();
	skip(
			testDivMark();
	)
}

void checkNil() {
	check(NIL.isNil());
	check(nil_name == "nil");// WASM
	if (NIL.name.data != nil_name)
		assert_equals(NIL.name.data, nil_name);
	check(NIL.name.data == nil_name);
	check(NIL.length == 0);
	check(NIL.children == 0);
	check(NIL.parent == 0);
	check(NIL.next == 0);
}

void testMarkAsMap() {
	Node compare = Node();
//	compare["d"] = Node();
	compare["b"] = 3;
	compare["a"] = "HIO";
	Node &dangling = compare["c"];
	check(dangling.isNil());
	checkNil();
	check(dangling == NIL);
	check(&dangling != &NIL);// not same pointer!
	dangling = Node(3);
//	dangling = 3;
	check(dangling == 3);
	check(compare["c"] == 3);
	assert_equals(compare["c"], Node(3));
	Node &node = compare["a"];
	assert(node == "HIO");
	chars source = "{b:3 a:'HIO' c:3}";// d:{}
	Node marked = parse(source);
	Node &node1 = marked["a"];
	assert(node1 == "HIO");
	check(compare["a"] == "HIO");
	check(marked["a"] == "HIO");
	assert(node1 == compare["a"]);
	assert(marked["a"] == compare["a"]);
	assert(marked["b"] == compare["b"]);
	assert(compare == marked);
}

void testMarkSimpleAssign() {
	assert_parses("a=3");
	Node &a = result["a"];
	assert(a == 3);
}

void testMarkSimple() {
	log("testMarkSimple");
	Node a = assert_parses("{aa:3}");
	assert_equals(a.value.longy, (long) 3);
	assert_equals(a, long(3));
	assert(a == 3);
	assert(a.kind == longs or a.kind == keyNode and a.value.node->kind == longs);
	assert(a.name == "aa"_s);
//	assert(a3.name == "a"_s);// todo? cant


	Node &b = a["b"];
	a["b"] = a;
	assert(a["b"] == a);
	assert(a["b"] == a);
	assert(a["b"] == 3);

	assert(parse("3.") == 3.);
	assert(parse("3.") == 3.f);
//	assert(Mark::parse("3.1") == 3.1); // todo epsilon 1/3≠0.33…
//	assert(Mark::parse("3.1") == 3.1f);// todo epsilon
	Node node = parse("'hi'");
	check(node.kind == strings);
	check(*node.value.string == "hi");
	check(node == "hi");
	assert(parse("'hi'") == "hi");
	assert(parse("3") == 3);
}


void testUTFinCPP() {
	char32_t wc[] = U"zß水🍌"; // or
//	char32_t wc2[] = "z\u00df\u6c34\U0001f34c";/* */ Initializing wide char array with non-wide string literal
	auto wc2 = "z\u00df\u6c34\U0001f34c";
//	auto wc3 = "z\udf\u6c34\U1f34c";// not ok in cpp

	// char = byte % 128   char<0 => utf or something;)
//	using namespace std;
#ifndef WASM
	const char8_t str[9] = u8"عربى";// wow, 9 bytes!
#endif
	const char str1[9] = "عربى";

#ifndef WASM
	std::string x = "0☺2√";
	// 2009 :  std::string is a complete joke if you're looking for Unicode support
	auto smile0 = x[1];
	char16_t smile1 = x[1];
	char32_t smile = x[1];
//	check(smile == smile1);
#endif
//	wstring_convert<codecvt_utf8<char32_t>, char32_t> wasm_condition;
//	auto str32 = wasm_condition.from_bytes(str);
	char16_t character = u'牛';
	char32_t hanzi = U'牛';
	wchar_t word = L'牛';
//	for(auto c : str32)
//		cout << uint_least32_t(c) << '\n';
//		char a = '☹';// char (by definition) is one byte (WTF)
//		char[10] a='☹';// NOPE
	char *a = "☹"; // OK
	byte *b = reinterpret_cast<byte *>(a);
	check_eq(a[0], (char) -30); // '\xe2'
	check_eq(a[1], (char) -104); // '\x98'
	check_eq(a[2], (char) -71); // '\xb9'
	check_eq(b[0], (byte) 226); // '\xe2'
	check_eq(b[1], (byte) 152); // '\x98'
	check_eq(b[2], (byte) 185); // '\xb9'
	check_eq(b[3], (byte) 0); // '\0'
}

void testUnicode_UTF16_UTF32() {// constructors/ conversion maybe later
//	char letter = '牛';// Character too large for enclosing character literal type
	char16_t character = u'牛';
	char32_t hanzi = U'牛';
	wchar_t word = L'牛';
	assert(eval("ç='☺'") == String(u'☺'));
	assert(eval("ç='☺'") == String(L'☺'));
	assert(eval("ç='☺'") == String(U'☺'));

	check(String(u'牛') == "牛");
	check(String(L'牛') == "牛");
	check(String(U'牛') == "牛");

	check(String(L'牛') == u'牛');
	check(String(L'牛') == U'牛');
	check(String(L'牛') == L'牛');
	check(String(U'牛') == u'牛');
	check(String(U'牛') == U'牛');
	check(String(U'牛') == "牛");
	check(String(U'牛') == L'牛');
	check(String(u'牛') == u'牛');
	check(String(u'牛') == U'牛');
	check(String(u'牛') == L'牛');
	check(String(u'牛') == "牛");
	check(String("牛") == u'牛');
	check(String("牛") == U'牛');
	check(String("牛") == L'牛');
	check(String("牛") == "牛");
//	log(character);
//	log(hanzi);
//	log(word);
	logi(sizeof(char32_t));// 32 lol
	logi(sizeof(wchar_t));
}

void testStringReferenceReuse() {
	String x = "ab牛c";
	String x2 = String(x.data, false);
	check(x.data == x2.data);
	String x3 = x.substring(0, 2, true);
	check(x.data == x3.data);
	check(x.length >
	      x3.length) // shared data but different length! check shared_reference when modifying it!! &text[1] doesn't work anyways;)
	check(x3 == "ab");
	log(x3);
	// todo("make sure all algorithms respect shared_reference and crucial length! especially print!");
}

//testUTFø  error: stray ‘\303’ in program
void testUTF() {
	skip(testUnicode_UTF16_UTF32());
	check(utf8_byte_count(U'ç') == 2);
	check(utf8_byte_count(U'√') == 3);
	check(utf8_byte_count(U'🥲') == 4);
	check(is_operator(u'√'))// can't work because ☺==0xe2... too
	check(!is_operator(U'☺'))
	check(!is_operator(U'🥲'))
	check(not is_operator(U'ç'));
	check(is_operator(U'='));
	testUTFinCPP();
//	check(x[1]=="牛");
	check("a牛c"s.codepointAt(1) == U'牛');
	String x = "a牛c";
	codepoint i = x.codepointAt(1);
	check("牛"s == i);
#ifndef WASM  // why??
	check("a牛c"s.codepointAt(1) == "牛"s);
	check(i == "牛"s);// owh wow it works reversed
#endif
	wchar_t word = L'牛';
	check(x.codepointAt(1) == word);

	assert_parses("{ç:☺}");
	assert(result["ç"] == "☺");

	assert_parses("ç:'☺'");
	skip(
			assert(result == "☺");
	)

	assert_parses("{ç:111}");
	assert(result["ç"] == 111);

	skip(
			assert_parses("ç='☺'");
			assert(eval("ç='☺'") == "☺");

			assert_parses("ç=☺");
			assert(result == "☺" or result.kind == expression);
	)
//	assert(node == "ø"); //=> OK
}


void testMarkMultiDeep() {
	// fragile:( problem :  c:{d:'hi'}} becomes c:'hi' because … bug
	chars source = "{deep{a:3,b:4,c:{d:'hi'}}}";
	assert_parses(source);
	Node &c = result["deep"]['c'];
	Node &node = result["deep"]['c']['d'];
	assert_equals(node, "hi");
	assert(node == "hi"_s);
	assert(node == "hi");
	assert(node == c['d']);
}

void testMarkMulti() {
	chars source = "{a:'HIO' b:3}";
	assert_parses(source);
	Node &node = result['b'];
	log(result['a']);
	log(result['b']);
	assert(result["b"] == 3);
	assert(result['b'] == 3);
}

void testMarkMulti2() {
	assert_parses("a:'HIO' b:3  d:{}");
	assert(result["b"] == 3);
}

void testOverwrite() {
	chars source = "{a:'HIO' b:3}";
	assert_parses(source);
	result["b"] = 4;
	assert(result["b"] == 4);
	assert(result['b'] == 4);
}

void testAddField() {
	chars source = "{}";
	result["e"] = 42;
	assert(result["e"] == 42);
	assert(result['e'] == 42);
}

void testErrors() {
#if defined(WASI) || defined(WASM)
	skip("can't catch ERROR in wasm")
	return;
#endif
	throwing = false;
	result = parse("]");
	assert(result == ERROR);
/*
	ln -s /me/dev/apps/wasp/samples /me/dev/apps/wasp/cmake-build-wasm/out
	ln -s /Users/me/dev/apps/wasp/samples /Users/me/dev/apps/wasp/cmake-build-default/ #out/
  */
	breakpoint_helper
	Node node = Wasp::parseFile("samples/errors.wasp");
	throwing = true;
}


void testForEach() {
	int sum = 0;
	for (Node &item : parse("1 2 3"))
		sum += item.value.longy;
	assert(sum == 6);
}

#ifndef WASM
#ifndef WASI
#ifdef APPLE
#include <filesystem>
using files = std::filesystem::recursive_directory_iterator;

void testAllSamples() {
	// FILE NOT FOUND :
//	ln -s /me/dev/apps/wasp/samples /me/dev/apps/wasp/cmake-build-debug/
// ln -s /me/dev/apps/wasp/samples /me/dev/apps/wasp/cmake-build-wasm/
//	ln -s /me/dev/apps/wasp/samples /me/dev/apps/wasp/out/
// ln -s /me/dev/apps/wasp/samples /me/dev/apps/wasp/out/out wtf
	for (const auto &file : files("samples/")) {
		if (!String(file.path().string().data()).contains("error"))
			Mark::Wasp::parseFile(file.path().string().data());
	}
}
#endif
#endif
#endif

void testSample() {
	Node node = Wasp::parseFile("samples/comments.wasp");
}

void testNewlineLists() {
	Node node = parse("  c: \"commas optional\"\n d: \"semicolons optional\"\n e: \"trailing comments\"");
	assert(node['d'] == "semicolons optional");
}

void testKitchensink() {
	Node node = Wasp::parseFile("samples/kitchensink.wasp");
	node.log();
	assert(node['a'] == "classical json");
	assert(node['b'] == "quotes optional");
	assert(node['c'] == "commas optional");
	assert(node['d'] == "semicolons optional");
	assert(node['e'] == "trailing comments"); // trailing comments
	assert(node["f"] == /*inline comments*/ "inline comments");
	skip(
	)
}

void testEval3() {
	auto math = "one plus two";
	Node result = Wasp::eval(math);
	assert(result == 3);
}


void testMathExtra() {
	assert_is("one plus two times three", 7);
}

void testRoot() {
	skip(
			assert_is("40+√4", 42, 0)
			assert_is("√4", 2);
			assert_is("√4+40", 42);
			assert_is("40 + √4", 42);
	);// todo tokenized as +√
}

void testRootFloat() {
	skip(  // include <cmath> causes problems, so skip
//	assert_is("√42*√42", 42.);// todo tokenized as *√
			assert_is("√42 * √42", 42.);
//	assert_is("√42*√42", Node(42.,0 ));
//	assert_is("√42*√42", 42);
//	assert_is("√42*√42",42);// int rounding to 41 todo?
	)
}


void testDeepLists() {
	assert_parses("{a:1 name:'ok' x:[1,2,3]}");
	assert(result.length == 3);
	assert(result["x"].length == 3);
	assert(result["x"][2] == 3);
}

void testIterate() {
//	parse("(1 2 3)");
	Node liste = parse("{1 2 3}");
	liste.log();
	for (Node &child : liste) {
		// SHOULD effect result
		child.value.longy = child.value.longy + 10;
	}
	check(liste[0].value.longy == 11)
	for (Node child : liste) {
		// should NOT effect result
		child.value.longy = child.value.longy + 1;
	}
	check(liste[0].value.longy == 11)
}


void testLists() {
	assert_parses("[1,2,3]");
	result.log();
	assert_equals(result.length, 3);
	assert(result[2] == 3);
	assert(result[0] == 1);
	skip(
			assert(result[0] == "1");// autocast
	)
//	assert_is("[1,2,3]",1);
}

void testMapsAsLists() {
	assert_parses("{1,2,3}");
	assert_parses("{'a'\n'b'\n'c'}");
	assert_parses("{add x y}");// expression?
	assert_parses("{'a' 'b' 'c'}");// expression?
	assert_parses("{'a','b','c'}");// list
	assert_parses("{'a';'b';'c'}");// list
	assert(result.length == 3);
	assert(result[1] == "b");
//	assert_is("[1,2,3]",1); what?
}


void testLogic() {
	assert_is("not true", false);
	assert_is("false xor true", true);
	assert_is("true xor false", true);
	assert_is("false xor false", false);
	assert_is("true xor true", false);
	assert_is("false or true", true);
	assert_is("false or false", false);
	assert_is("true or false", true);
	assert_is("true or true", true);

	assert_is("true and true", true);
	assert_is("true and false", false);
	assert_is("false and true", false);
	assert_is("false and false", false);

	assert_is("not false", true);
}

// use the bool() function to determine if a value is truthy or falsy.
void testTruthiness() {
	Node node = parse("true");
//	log("TRUE:");
	nl();
	log(node.name);
	nl();
	logi(node.value.longy);
	check(True.kind == bools);
	check(True.name == "True");
	check(True.value.longy == 1);
	assert_is("false", false);
	assert_is("true", true);
//	check(True.value.longy == true);
//	check(True.name == "true");
//	check(True == true);
	assert_is("False", false);
	assert_is("True", true);
	assert_is("False", False);
	assert_is("True", True);
	assert_is("false", False);
	assert_is("true", True);
	assert_is("ø", NIL);
	assert_is("nil", NIL);
	assert_is("nil", False);
	assert_is("nil", false);
	assert_is("ø", false);
	skip(
			assert_is("2", true);  // Truthiness != equality with 'true' !
			assert_is("2", True); // Truthiness != equality with 'True' !
			assert_is("{x:0}", true); // wow falsey so deep?
			assert_is("[0]", true);  // wow falsey so deep?
	)
	assert_is("1", true);


	assert_is("{1}", true);
	assert_is("{x:1}", true);

	assert_is("x", false);
	assert_is("{x}", false);
	assert_is("cat{}", false);
	// empty referenceIndices are falsey! OK
}

void testLogicEmptySet() {
	assert_is("not ()", true);
	assert_is("() xor 1", true);
	assert_is("1 xor ()", true);
	assert_is("() xor ()", false);
	assert_is("1 xor 1", false);
	assert_is("() or 1", true);
	assert_is("() or ()", false);
	assert_is("1 or ()", true);
	assert_is("1 or 1", true);

	assert_is("1 and 1", true);
	assert_is("1 and ()", false);
	assert_is("() and 1", false);
	assert_is("() and ()", false);

	assert_is("not 1", false);
	assert_is("{} xor 1", true);
	assert_is("1 xor {}", true);
	assert_is("{} xor {}", false);
	assert_is("1 xor 1", false);
	assert_is("{} or 1", true);
	assert_is("{} or {}", false);
	assert_is("1 or {}", true);
	assert_is("1 or 1", true);

	assert_is("1 and 1", true);
	assert_is("1 and {}", false);
	assert_is("{} and 1", false);
	assert_is("{} and {}", false);

	assert_is("not {}", true);
	assert_is("not 1", false);

	assert_is("[] or 1", true);
	assert_is("[] or []", false);
	assert_is("1 or []", true);
	assert_is("1 or 1", true);

	assert_is("1 and 1", true);
	assert_is("1 and []", false);
	assert_is("[] and 1", false);
	assert_is("[] and []", false);

	assert_is("not []", true);
	assert_is("not 1", false);


	assert_is("[] xor 1", true);
	assert_is("1 xor []", true);
	assert_is("[] xor []", false);
	assert_is("1 xor 1", false);

}

void testLogic01() {
	assert_is("0 xor 1", true);
	assert_is("1 xor 0", true);
	assert_is("0 xor 0", false);
	assert_is("1 xor 1", false);
	assert_is("0 or 1", true);
	assert_is("0 or 0", false);
	assert_is("1 or 0", true);
	assert_is("1 or 1", true);

	assert_is("1 and 1", true);
	assert_is("1 and 0", false);
	assert_is("0 and 1", false);
	assert_is("0 and 0", false);

	assert_is("not 0", true);
	assert_is("not 1", false);
	assert_is("1≠2", True);
	assert_is("1==2", False);
	//	assert_is("1=2", False);
	assert_is("1!=2", True);
	assert_is("1≠1", False);
//	assert_is("2=2", True);
	assert_is("2==2", True);
	assert_is("2!=2", False);
}


void testCpp() {
//	esult of comparison of constant 3 with expression of type 'bool' is always true
//	assert(1 < 2 < 3);// NOT WHAT YOU EXPECT!
//	assert(3 > 2 > 1);// NOT WHAT YOU EXPECT!
//	assert('a' < 'b' < 'c');// NOT WHAT YOU EXPECT!
//	assert('a' < b and b < 'c');// ONLY WAY <<
}

void testGraphSimple() {
	assert_parses("{  me {    name  } # Queries can have comments!\n}");
	assert(result.children[0].name == "name");// result IS me !!
	assert(result["me"].children[0].name == "name");// me.me = me good idea?
}


void testGraphQlQueryBug() {
	auto graphResult = "{friends: [ {name:x}, {name:y}]}";
	assert_parses(graphResult);
	Node &friends = result["friends"];
	assert(friends[0]["name"] == "x");
}

void testGraphQlQuery() {
	auto graphResult = "{\n  \"data\": {\n"
	                   "    \"hero\": {\n"
	                   "      \"id\": \"R2-D2\",\n"
	                   "      \"height\": 5.6430448,\n"
	                   "      \"friends\": [\n"
	                   "        {\n"
	                   "          \"name\": \"Luke Skywalker\"\n"
	                   "        },\n"
	                   "        {\n"
	                   "          \"name\": \"Han Solo\"\n"
	                   "        },\n"
	                   "      ]" /* todo \n nextNonWhite */
	                   "    }\n"
	                   "  }\n"
	                   "}";
	assert_parses(graphResult);
	result.log();
	Node &data = result["data"];
	data.log();
	Node &hero = data["hero"];
	hero.log();
	Node &id = hero["id"];
	id.log();
	assert(id == "R2-D2");
	Node &friends = result["data"]["hero"]["friends"];
	assert(friends[0]["name"] == "Luke Skywalker");
//todo	assert(result["hero"] == result["data"]["hero"]);
//	assert(result["hero"]["friends"][0]["name"] == "Luke Skywalker")// if 1-child, treat as root
}

void testGraphQlQuery2() {
	assert_parses("{\n"
	              "  human(id: \"1000\"){\n"
	              "    name\n"
	              "    height(unit: FOOT)\n"
	              "  }\n"
	              "}");
	assert(result["human"]["id"] == 1000);
	skip(assert(result["id"] == 1000, 0));// if length==1 descend!
}

void testGraphQlQuerySignificantWhitespace() {
	// human() {} != human(){}
	assert_parses("{\n"
	              "  human(id: \"1000\") {\n"
	              "    name\n"
	              "    height(unit: FOOT)\n"
	              "  }\n"
	              "}");
	assert(result["human"]["id"] == 1000);
	skip(assert(result["id"] == 1000, 0));// if length==1 descend!
}

void testGraphParams() {
	assert_parses("{\n  empireHero: hero(episode: EMPIRE){\n    name\n  }\n"
	              "  jediHero: hero(episode: JEDI){\n    name\n  }\n}");
	Node &hero = result["empireHero"];
	hero.log();
	assert(hero["episode"] == "EMPIRE");
	assert_parses("\nfragment comparisonFields on Character{\n"
	              "  name\n  appearsIn\n  friends{\n    name\n  }\n }");
	assert_parses("\nfragment comparisonFields on Character{\n  name\n  appearsIn\n  friends{\n    name\n  }\n}")
// VARIAblE: { "episode": "JEDI" }
	assert_parses("query HeroNameAndFriends($episode: Episode){\n"
	              "  hero(episode: $episode){\n"
	              "    name\n"
	              "    friends{\n"
	              "      name\n"
	              "    }\n"
	              "  }\n"
	              "}")
}

void testRootLists() {
	// vargs needs to be 0 terminated, otherwise pray!
	assert_is("(1,2,3)", Node(1, 2, 3, 0))
	assert_is("(a,b,c)", Node("a", "b", "c", 0))
	assert_is("(1;2;3)", Node(1, 2, 3, 0))
	assert_is("(a;b;c)", Node("a", "b", "c", 0))
	assert_is("a;b;c", Node("a", "b", "c", 0))
	assert_is("1;2;3", Node(1, 2, 3, 0, 0))//ok
	assert_is("1,2,3", Node(1, 2, 3, 0))
	assert_is("a,b,c", Node("a", "b", "c", 0))
	assert_is("{1 2 3}", Node(1, 2, 3, 0))
	assert_is("{a b c}", Node("a", "b", "c", 0))
	assert_is("{1,2,3}", Node(1, 2, 3, 0))
	assert_is("{a,b,c}", Node("a", "b", "c", 0))
	assert_is("{1;2;3}", Node(1, 2, 3, 0))
	assert_is("{a;b;c}", Node("a", "b", "c", 0))
	assert_is("[1 2 3]", Node(1, 2, 3, 0).setType(patterns))
	assert_is("[1 2 3]", Node(1, 2, 3, 0))
	assert_is("[a b c]", Node("a", "b", "c", 0))
	assert_is("[1,2,3]", Node(1, 2, 3, 0))
	assert_is("[1,2,3]", Node(1, 2, 3, 0).setType(patterns));
	assert_is("[a,b,c]", Node("a", "b", "c", 0))
	assert_is("[1;2;3]", Node(1, 2, 3, 0))
	assert_is("[a;b;c]", Node("a", "b", "c", 0))
	assert_is("1 2 3", Node(1, 2, 3, 0))
	assert_is("a b c", Node("a", "b", "c", 0, 0))
	assert_is("(1 2 3)", Node(1, 2, 3, 0))
	assert_is("(a b c)", Node("a", "b", "c", 0))
}


void testRoots() {
	check(NIL.value.longy == 0);
	assert_is("'hello'", "hello");
	skip(assert_is("hello", "hello", 0));// todo reference==string really?
	assert_is("True", True)
	assert_is("False", False)
	assert_is("true", True)
	assert_is("false", False)
	assert_is("yes", True)
	assert_is("no", False)
//	assert_is("right", True)
//	assert_is("wrong", False)
	assert_is("null", NIL);
	assert_is("", NIL);
	check(NIL.value.longy == 0);
	assert_is("0", NIL);
	assert_is("1", 1)
	assert_is("123", 123)
	skip(
			assert_is("()", NIL);
			assert_is("{}", NIL);// NOP
	)
	assert_is("\"hello\"", "hello")
}


void testParams() {
//	assert_equals(parse("f(x)=x*x").param->first(),"x");

	Node body = assert_parses("body(style='blue'){a(link)}");
	assert(body["style"] == "blue");

	parse("a(x:1)");
	assert_parses("a(x:1)");
	assert_parses("a(x=1)");
	assert_parses("a{y=1}");
	assert_parses("a(x=1){y=1}");
	skip(assert_parses("a(1){1}", 0));
	skip(assert_parses("multi_body{1}{1}{1}", 0));// why not generalize from the start?
	skip(assert_parses("chained_ops(1)(1)(1)", 0));// why not generalize from the start?

	assert_parses("while(x<3){y:z}");
	Node body2 = assert_parses("body(style='blue'){style:green}");// is that whole xml compatibility a good idea?
	skip(assert(body2["style"] ==
	            "green", 0));// body has prescedence over param, semantically param provide extra data to body
	assert(body2[".style"] == "blue");
//	assert_parses("a(href='#'){'a link'}");
//	assert_parses("(markdown link)[www]");
}


void testDidYouMeanAlias() {
	Node ok1 = assert_parses("printf('hi')");
	skip(
			assert_equals(ok1[".warnings"], "DYM print");// THIS CAN NEVER HAVED WORKED! BUG IN TEST PIPELINE!
	)
}

void testEmpty() {
	result = assert_parsesx("{  }");
	assert_equals_x(result.length, 0);
}

void testEval() {
	skip(
			assert_is("√4", 2);
	)
}

void testLengthOperator() {
	assert_is("#{a b c}", 3);
	skip(assert_is("#(a b c)", 3, 0));// todo: groups
}

void testNodeName() {
	Node a = Node("xor");// NOT type string by default!
	bool ok = a == "xor";
	check(a == "xor")
	check(a.name == "xor")
	check(ok)
}

void testIndentAsBlock() {
	assert_is("a\n\tb", "a{b}")
}

void testParentContext() {
	chars source = "{a:'HIO' d:{} b:3 c:ø}";
	assert_parses(source);
	result.log();
	Node &a = result["a"];
	a.log();
	assert_equals(a.kind, strings);
	assert_equals(a.value.string, "HIO");
	assert_equals(a.string(), "HIO");// keyNodes go to values!
	assert(a == "HIO");
//	assert_equals(a.name,"a" or"HIO");// keyNodes go to values!
	skip(
			assert_equals(a.kind, keyNode);
			assert(a.name == "HIO");
	)
}

void testParentBUG() {
//	chars source = "{a:'HIO' d:{} b:3 c:ø}";
	chars source = "{a:'HIO'}";

	assert_parses(source);
	Node &a = result["a"];
	Node *parent = a.parent;
	assert(parent);
	log(parent);// BROKEN, WHY?? let's find out:
	assert(parent == &result);
	assert_equals(parent->kind, objects);
	assert_equals(parent->name, "");
	assert(parent == &result);
	testParentContext();// make sure parsed correctly
}

void testAsserts() {
	assert_equals(11, 11);
	assert_equals(11.1f, 11.1f);
//	assert_equals(11.1l, 11.1);
	assert_equals((float) 11., (float) 11.);
//	assert_equals((double)11., (double )11.);
	assert_equals("a", "a");
	assert_equals("a"_s, "a"_s);
	assert_equals("a"_s, "a");
	assert_equals(Node("a"), Node("a"));
	assert_equals(Node(1), Node(1));
}

void testStringConcatenation() {
//	assert_equals(Node("✔️"), True);
//	assert_equals(Node("✔"), True);
//	assert_equals(Node("✖️"), False);
//	assert_equals(Node("✖"), False);
	String huh = "a"_s + 2;
	check_eq(huh.length, 2);
	check_eq(huh[0], 'a');
	check_eq(huh[1], '2');
	check_eq(huh[2], 0);
	check(eq("a2", "a2"));
	check(eq("a2", "a2", 3));

	assert_equals(huh, "a2");
	assert_equals("a"_s + 2, "a2");
	assert_equals("a"_s + 2.2, "a2.2");
	assert_equals("a"_s + "2.2", "a2.2");
	assert_equals("a"_s + 'b', "ab");
	assert_equals("a"_s + "bc", "abc");
	assert_equals("a"_s + true, "a✔️"_s);
	assert_equals("a%sb"_s % "hi", "ahib");

	assert_equals("a%db"_s % 123, "a123b");
	assert_equals("a%s%db"_s % "hi" % 123, "ahi123b");
}

void testString() {
	String *a = new String("abc");
	String b = String("abc");
	String c = *a;
	check(b == "abc");
	check(c == "abc");
	check(b == c);
	check(b == a);
	check("hi %s ok"s.replace("%s", "ja") == "hi ja ok");
	check("abc"_ == "abc");
	check("%d"s % 5 == "5");
	check("%s"s % "ja" == "ja");
	check("1234%d6789"s % 5 == "123456789");
	check("char %c"s % 'a' == "char a");
	check("%c %d"s % 'a' % 3 == "a 3");
	check("abc"s.replace("a", "d") == "dbc");
	check("hi %s ok"s % "ja" == "hi ja ok");
	check("%s %d"s % "hu" % 3 == "hu 3");
	check("%s %s %d"s % "ha" % "hu" % 3 == "ha hu 3");
	assert_equals(String("abcd").substring(1, 2, false), "b");
	assert_equals(String("abcd").substring(1, 3, false), "bc");
	assert_equals(String("abcd").substring(1, 2, true/*share*/), "b");// excluding, like js
	assert_equals(String("abcd").substring(1, 3, true), "bc");
	assert_equals(String("abcd").substring(1, 3), "bc");
	assert_equals(String("abcd").substring(1, 2), "b");
	check("%s"s.replace("%s", "ja") == "ja");
	check("hi %s"s.replace("%s", "ja") == "hi ja");
	check("%s ok"s.replace("%s", "ja") == "ja ok");
	check("hi %s ok"s.replace("%s", "ja") == "hi ja ok");
	auto x = "hi %s ok"s % "ja";
	check(x);
	printf("%s", x.data);
	check(x == "hi ja ok");
	check("hi %s ok"s % "ja" == "hi ja ok");

	check_eq(atoi("123x"), 123);// can crash!?!
	check_eq(" a b c  \n"s.trim(), "a b c");
	testStringConcatenation();
}


void testNilValues() {
	assert_parses("{a:null}");
	assert_equals(result["a"], NIL);

	assert_parses("{ç:null}");
	Node &node1 = result["ç"];
	assert(node1 == NIL);

	assert_parses("{ç:ø}");
	Node &node = result["ç"];
	assert(node == NIL);
}


void testConcatenationBorderCases() {
	assert_equals(Node(1, 0) + Node(3, 0), Node(1, 3, 0));// ok
	assert_equals(Node("1", 0, 0) + Node("2", 0, 0), Node("1", "2", 0));
// Border cases: {1}==1;
	assert_equals(parse("{1}"), parse("1"));
// Todo Edge case a=[] a+=1
	assert_equals(Node() + Node("1", 0, 0), Node("1", 0, 0));
	//  singleton {1}+2==1+2 = 12/3 should be {1,2}
	assert_equals(Node("1", 0, 0) + Node("x"_s), Node("1", "x", 0));
}

void testConcatenation() {
	Node node1 = Node("1", "2", "3", 0);
	check(node1.length == 3);
	check(node1.last() == "3");
	check(node1.kind == objects);
	Node other = Node("4").setType(strings);// necessary: Node("x") == reference|strings? => kind=unknown
	check(other.kind == strings);
	check(!other.isNil());
	check(!(&other == &NIL));
	check(!(other == &NIL));
	check(not(&other == &NIL));
	check(not(other == &NIL));
	check(other != NIL);
#ifndef WASM
	check(other != &NIL);
#endif
	check(&other != &NIL);


	check(not other.isNil());
	Node node1234 = node1.merge(other);
//	Node node1234=node1.merge(Node("4"));
//	Node node1234=node1.merge(new Node("4"));
	Node *four = new Node("4");
	node1.add(four);
//	node1=node1 + Node("4");
	check_eq(node1.length, 4);
	check(node1.last() == "4");
//	check(&node1234.last() == four); not true, copied!
	check(node1234.last() == four);
	check(*four == "4");
	node1234.log();

	check_eq(node1234.length, 4);

	node1234.children[node1234.length - 2].log();
	node1234.children[node1234.length - 1].log();
	node1234.last().log();
	check(node1234.last() == "4");

	assert_equals(node1, Node("1", "2", "3", "4", 0));
	Node first = Node(1, 2, 0);
	check_eq(first.length, 2);
	check_eq(first.kind, objects);
	Node node = first + Node(3);
	check_eq(node.length, 3);
	check(node.last() == 3);

	assert_equals(Node(1, 2, 0) + Node(3), Node(1, 2, 3, 0));
	assert_equals(Node(1, 2, 0) + Node(3, 4, 0), Node(1, 2, 3, 4, 0));
	assert_equals(Node("1", "2", 0) + Node("3", "4", 0), Node("1", "2", "3", "4", 0));
	assert_equals(Node(1) + Node(2), Node(3));
	assert_equals(Node(1) + Node(2.4), Node(3.4));
	assert_equals(Node(1.0) + Node(2), Node(3.0));

	skip(
			assert_equals(Node(1) + Node("a"_s), Node("1a"));
			Node bug = Node("1"_s) + Node(2);
			// AMBIGUOUS: "1" + 2 == ["1" 2] ?
			assert_equals(Node("1"_s) + Node(2), Node("12"));
			assert_equals(Node("a"_s) + Node(2.2), Node("a2.2"));
			// "3" is type unknown => it is treated as NIL and not added!
			assert_equals(Node("1", "2", 0) + Node("3"), Node("1", "2", "3", 0));// can't work ^^
	)
}


void testParamizedKeys() {
//	<label for="pwd">Password</label>

// 0. parameters accessible
	Node label0 = parse("label(for:password)");
	label0.log();
	Node &node = label0["for"];
	assert_equals(node, "password");
	assert_equals(label0["for"], "password");



// 1. paramize keys: label{param=(for:password)}:"Text"
	Node label1 = parse("label(for:password):'Passwort'");
	label1.print();
	assert_equals(label1, "Passwort");
	assert_equals(label1["for"], "password");
//	assert_equals(label1["for:password"],"Passwort");

// 2. paramize values
// TODO 1. move params of Passwort up to lable   OR 2. preserve Passwort as object in stead of making it string value of label!
	skip(
			Node label2 = parse("label:'Passwort'(for:password)");
			check(label2 == "Passwort");
			assert_equals(label2, "Passwort");
			assert_equals(label2["for"], "password");
			assert_equals(label2["for"], "password");// descend value??
			assert_equals(label2["Passwort"]["for"], "password");
	)

	skip(
//	3. relative equivalence? todo not really
			assert_equals(label1, label2);
			Node label3 = parse("label:{for:password 'Password'}");
	)
}

void testStackedLambdas() {
	Node node = parse("a{x:1}{y:2}{3}");
	node.log();
	check(node.length == 3);
	check(node[0] == parse("{x:1}"));
	check(node[0] == parse("x:1"));// grouping irrelevant
	check(node[1] == parse("{y:2}"));
	check(node[2] == parse("{3}"));
	check(node[2] != parse("{4}"));

	check(parse("a{x}{y z}") != parse("a{x,{y z}}"));
}

void testIndex() {
	assert_parses("[a b c]#2");
	result.log();
	check(result.length == 3);
	assert_is("{a b c}#2", "b");
	assert_is("(a b c)#2", "b");
	skip(
			assert_is("[a b c]#2", "b");
	)
	assert_is("{a:1 b:2}.a", 1)
	assert_is("a of {a:1 b:2}", 1)
	assert_is("a in {a:1 b:2}", 1)
	assert_is("{a:1 b:2}[a]", 1)
	assert_is("{a:1 b:2}.b", 2)
	assert_is("b of {a:1 b:2}", 2)
	assert_is("b in {a:1 b:2}", 2)
	assert_is("{a:1 b:2}[b]", 2)
}

// can be removed because noone touches List.sort algorithm!
void testSort() {
#ifndef WASM
	List<int> list = {3, 1, 2, 5, 4};
	List<int> listb = {1, 2, 3, 4, 5};
	check(list.sort() == listb)
	auto by_precedence = [](int &a, int &b) { return a * a > b * b; };
	check(list.sort(by_precedence) == listb)
	auto by_square = [](int &a) { return (float) a * a; };
	check(list.sort(by_square) == listb)
#endif
}

void testSort1() {
#ifndef WASM
	List<int> list = {3, 1, 2, 5, 4};
	List<int> listb = {1, 2, 3, 4, 5};
	auto by_precedence = [](int &a, int &b) { return a * a > b * b; };
	check(list.sort(by_precedence) == listb)
#endif
}

void testSort2() {
#ifndef WASM
	List<int> list = {3, 1, 2, 5, 4};
	List<int> listb = {1, 2, 3, 4, 5};
	auto by_square = [](int &a) { return (float) a * a; };
	check(list.sort(by_square) == listb)
#endif
}

void testRemove() {
	Node result = parse("a b c d");
	result.remove(1, 2);
	Node replaced = parse("a d");
	check(result == replaced);
}

void testRemove2() {
	Node result = parse("a b c d");
	result.remove(2, 10);
	Node replaced = parse("a b");
	check(result == replaced);
}

void testReplace() {
	Node result = parse("a b c d");
	result.replace(1, 2, new Node("x"));
	Node replaced = parse("a x d");
	check(result == replaced);
}


void testNodeConversions() {
	Node b = Node(true);
	log(typeName(b.kind));
	check(b.value.longy == 1);
	check(b.kind == bools);
	check(b == True);
	Node a = Node(1);
	check(a.kind == longs);
	check(a.value.longy == 1);
	Node a0 = Node(10l);
	check(a0.kind == longs);
	check(a0.value.longy == 10);
	Node a1 = Node(1.1);
	check_eq(a1.kind, reals);
	check(a1.kind == reals);
	check(a1.value.real == 1.1);
	Node a2 = Node(1.2f);
	check(a2.kind == reals);
	check(a2.value.real == 1.2f);
	Node as = Node('a');
	check(as.kind == strings);
	check(*as.value.string == 'a');
}


void testWasmString() {
	wasm_string x = reinterpret_cast<wasm_string>("\03abc");
	String y = String(x);
	check(y.length == 3);
	check(y == "abc"s);
}

void testGroupCascade() {
	result = parse("x='abcde';x#4='y';x#4");
	check(result.length == 3);

	Node result = parse("{ a b c, d e f; g h i , j k l \n "
	                    "a2 b2 c2, d2 e2 f2; g2 h2 i2 , j2 k2 l2}"
	                    "{a3 b3 c3, d3 e3 f3; g3 h3 i3 , j3 k3 l3 \n"
	                    "a4 b4 c4 ,d4 e4 f4; g4 h4 i4 ,j4 k4 l4}");
	result.log();
	check(result.kind == groups);// objects because {}!
	check(result.first().kind == groups);
	check(result.first().first().kind == groups);// or expression if x is op
	check(result.length == 2)// {…} and {and}
	check(result[0].length == 2) // a…  and a2…  with significant newline
	check(result[0][0].length == 2)// a b c, d e f  and  g h i , j k l
	check(result[0][0][0].length == 2)// a b c  and  d e f
	check(result[0][0] == Node("a b c, d e f; g h i , j k l"));// significant newline!
	check(result[0][1] == Node("a2 b2 c2, d2 e2 f2; g2 h2 i2 , j2 k2 l2"));// significant newline!
	check(result[0][0][0][0].length == 3)// a b c
	check(result[0][0][0][0] == Node("a b c"));
	check(result[0][0][0][0][0] == "a");
	check(result[0][0][0][0][1] == "b");
	check(result[0][0][0][0][2] == "c");
	check(result[0][0][0][1][0] == "d");
	check(result[0][0][0][1][1] == "e");
	check(result[0][0][0][1][2] == "f");
	check(result[1][1][0][1][2] == "f4");
	Node reparse = parse(result.serialize());
	check(result == reparse);

}

void testNodeBasics() {
	Node a1 = Node(1);
//	check(a1.name == "1");// debug only!
	check(a1 == 1);
	Node a11 = Node(1.1);
	check_eq(a11.name, "1.1");
	check(a11 == 1.1);

	Node a = Node("a");
	check(a.name == "a");
	check(a == "a");
	Node b = Node("c");
	check_eq(b.name, "c");
	a.add(b.clone());
	check_eq(b.name, "c");// wow, worked before, corrupted memory!!
	check_eq(a.length, 1);
	check(a.children);
	Node *b2 = b.clone();
	check_eq(b.name, "c");// wow, worked before, corrupted memory!!
	check(b == b2);
	check_eq(b, a.children[0]);

//	a["b"] = "c";
	check_eq(b, a.children[0]);
	check_eq(b.name, "c");// wow, worked before, corrupted memory!!
	check_eq(a.children[0].name, "c");
	check(a.has("c"));
	check(b == a.has("c"));

//	a["b"] = "c";
	a["d"] = "e";
	check_eq(a.length, 2);
	check(a.has("d"));
	check(a["d"] == "e");
	Node &d = a.children[a.length - 1];
	check(d.length == 0);
	check(d == "e");
	check(d.kind == keyNode);
	a.addSmart(b);// why?
}

void tests() {
	data_mode = true;// expect data unless explicit code
	testAsserts();
	testString();
	testNodeName();
	testStringConcatenation();
	testNodeBasics();
	testStringReferenceReuse();
	testWasmString();
	testTruthiness();
	testNodeConversions();
	testConcatenation();
	testMarkSimple();
	testMarkMulti();
	testMarkMulti2();
	testStackedLambdas();
	testRootLists();
	testIterate();
	testLists();
	testMarkAsMap();
	testEval();
	testParamizedKeys();
	testForEach();
	testString();
	testEmpty();
	testDiv();
	testRoot();
	testLogicPrecedence();
	testRootFloat();
	testCpp();
//	testErrors();
	testNilValues();

	testLists();
	testDeepLists();
	testGraphParams();
	testParams();
	testAddField();
	testOverwrite();
	testMapsAsLists();
	testDidYouMeanAlias();
	testNetBase();
	testRoots();
	testForEach();
	testLengthOperator();
	testLogic();
	testLogicEmptySet();
	testDeepCopyDebugBugBug();
	testDeepCopyDebugBugBug2();
	testMarkSimpleAssign();
	testMarkMultiDeep();
	testSort();
	testSort1();
	testSort2();
	testReplace();
	testRemove();
	testRemove2();
	testGraphQlQueryBug();
	testGraphQlQuery();// fails sometimes => bad pointer!?
	testGraphQlQuery2();
	testUTF();// fails sometimes => bad pointer!?
	testUnicode_UTF16_UTF32();
	testConcatenationBorderCases();
	testCall();
	testNewlineLists();
	testIndex();

	skip(
			testLogic01();// fails in WASM why
			assert_is("one plus two times three", 7);
			testMathExtra(); // fails in WASM

			testKitchensink();
			testRecentRandomBugs();
			testGroupCascade();
	)
#ifdef APPLE
	testAllSamples();
#endif
	check(NIL.value.longy == 0);// should never be modified
	log("ALL TESTS PASSED");
}


void testArrayIndices() {
	assert_is("[1 2 3]", Node(1, 2, 3, 0).setType(patterns))
	assert_is("[1 2 3]", Node(1, 2, 3, 0))
	assert_is("(1 4 3)#2", 4);//
	assert_is("x=(1 4 3);x#2", 4);
	assert_is("x=(1 4 3);x#2=5;x#2", 5);
}


void testBUG() {// move to tests() once done!
	testUTF();// fails sometimes => bad pointer!?
	testParentBUG();
}


void todos() {
	skip(
			assert_is("{a b c}#2", "b");// ok, but not for patterns:
			assert_is("[a b c]#2", "b");// patterns

			testSignificantWhitespace();
			testAngle();// fails in WASM why?

			assert_parses("ç='☺'");
			assert(eval("ç='☺'") == "☺");

			assert_parses("ç=☺");
			assert(result == "☺" or result.kind == expression);
			testDeepCopyDebugBugBug2();// SUBTLE: BUGS OUT ONLY ON SECOND TRY!!!
			testDeepCopyDebugBugBug();
			assert_eval("if(0):{3}", false);// 0:3 messy node
			testNetBase();
			testBUG();
			assert_equals(Node("1", 0) + Node("2"_s), Node("1", "2", 0));
//	log("OK %s %d"s % ("WASM",1));// only 1 handed over
			log("OK %d %d"s % (2, 1));// only 1 handed over
			testIndentAsBlock();
	)
}

#include "Wasp.h" // is_operator
#include "Paint.h"

//int dump_nr = 1;
//void dumpMemory(){
//	String file_name="dumps/dump"s+dump_nr++;
//	FILE* file=fopen(file_name,"wb");
//	size_t length = 65536*10;
//	fwrite(memory, length, 1, file);
//	fclose(file);
//}

#ifdef IMPLICIT_NODES
void testNodeImplicitConversions(){
	// looks nice, but only causes trouble and is not necessary for our runtime!
	Node b=true;
	log(typeName(b.kind));
	check(b.value.longy==1);
	check(b.kind==bools);
	check(b==True);
	Node a=1;
	check(a.kind==longs);
	check(a.value.longy=1);
	Node a0=10l;
	check(a0.kind==longs);
	check(a0.value.longy=1);
	Node a1=1.1;
	check_eq(a1.kind,reals);
	check(a1.kind==reals);
	check(a1.value.real=1.1);
	Node a2=1.2f;
	check(a2.kind==reals);
	check(a2.value.real=1.2f);
	Node as = 'a';
	check(as.kind==strings);
	check(as.value.string=='a');
}
#endif

void testSignificantWhitespace() {

	//1 + 1 ≠ 1 +1 == [1 1]
	assert(eval("1 + 1 == 2"));
	assert_is("1 +1", parse("[1 1]"));
	assert(eval("1 +1 == [1 1]"));
	assert(eval("1 +1 ≠ 1 + 1"));
}

void testUnits() {
	assert_is("1 m + 1km", Node(1001).setType("m"));
}

void testRoundFloorCeiling() {
	assert_emit("n=3;2ⁿ", 8);
	assert_emit("3²", 9);
	assert_emit("n=3.0;2.0ⁿ", 8);
	assert_emit("3.0²", 9);
	assert_emit("π", 3/*.1415926535897*/);
	assert_emit("π*1000000", 3141592/*6535897*/);

	assert_emit("π²", 9.869604401089358 /*π*π*/);

	assert_emit("n=3;2ⁿ", 8);
	assert_emit("i=-9;-i", 9);
	assert_emit("i=-9;√-i", -3);
	assert_emit("-√9", -3);
	assert_emit(".1 + .9", 1);
	assert_emit("-.1 + -.9", -1);
	assert_emit("√9", 3);
	//	assert_emit("√-9 is -3i", -3);// if «use complex numbers»
	assert_emit(".1", 0);
	assert_emit("ceil 3.7", 4);
	assert_emit("floor 3.7", 3);// todo: only if «use math» namespace
	assert_emit("ceiling 3.7", 4);// todo: only if «use math» namespace
	assert_emit("round 3.7", 4);
	assert_emit("i=3.7;.3+i", 4);// floor
	// lol "⌊3.7⌋" is cursed and is transformed into \n\t or something in wasm and IDE!
//	assert_emit("⌊3.7", 3);// floor
//	assert_emit("⌊3.7⌋", 3);// floor
//	assert_emit("3.7⌋", 3);// floor
//	//assert_emit("i=3.7;.3 + ⌊i", 3);// floor
//	//assert_emit("i=3.7;.3+⌊i⌋", 3);// floor
//	assert_emit("i=3.7;.3+i⌋", 3);// floor
//	assert_emit("i=3.7;.3+ floor i", 3);// floor
}

void testPaint() {
	init_graphics();
	while (1)requestAnimationFrame();
}

void testPaintWasm() {
//	assert_emit("i=0;j=0;while(i<10000){i++;j++};j",10000)
//	char *wasm_paint_routine = "surface=init_graphics();surface#1=0;surface#3=0;surface#4=0;surface#5=0";
	char *wasm_paint_routine = "surface=init_graphics();i=10;while(i<10000){i++;surface#i=0;}";// todo : access true  c memory from wasm!
//char *wasm_paint_routine = "surface=init_graphics;while(1) 1+1";
//char *wasm_paint_routine = "init_graphics(); while(1){requestAnimationFrame()}";// SDL bugs a bit
	assert_emit(wasm_paint_routine, 10);
	while (1)requestAnimationFrame();// help a little
}
void testCurrent() { // move to tests() once OK
//	testWasmModuleExtension();
//	assert_emit("i=8;i=i/2;i", 4);// make sure i stays a-float
//	assert_emit("i=1.0;i=3;i=i/2;i=i*4", 6.0);// make sure i stays a-float
	assert_emit("i=123.4;i", 123);// main returning int
	assert_emit("i=1.0;i", 1.0);
	assert_emit("i=3;i", 3);
	assert_emit("i=1.0;i", 1.0);

	assert_emit("3+3", (int) 6);
	assert_run("x='123';x=='123'", true);// ok

//	Node result = emit("3*3");
//	printf("!result:", result);
//	print(result);
//	Node eval1 = eval("2+2");
//	printf(eval1);
	assert_emit("3*3", (int) 9);
	assert_emit("id (3+3)", (long) 6);

//	exit(0);
//	assert_emit("i=0;while(i++ <10001);i",10000)// parsed wrongly! while(  <( ++ i 10001) i)
//	functionSignatures.setDefault(Signature());
//	Signature &signature = functionSignatures["init_graphics"].import().returns(pointer);// surface
//	if(signature.return_type!=pointer)error("WWWAAAUUUU");
//	if(functionSignatures["init_graphics"].return_type!=pointer)error("WWWAAA");

//	testRoundFloorCeiling();
//    	subtract(other complex) := re -= other.re; im -= other.im
// := is terminated by \n, not by ;!
//	exit(0);
//	assert_emit("x=3;x*=3", 9)
//	assert_emit("x={1 4 3};x[1]", 4);
//	assert_emit("x={1 4 3};x[1]=5;x[1]", 5);
//	testPaintWasm();
//	exit(0);
//	testWasmMemoryIntegrity();
#ifndef WASM
	//	testWasmModuleExtension();
	//	testWasmRuntimeExtension();
#endif
//	testPaint();
	testArrayIndicesWasm();
	testStringIndicesWasm();
//testUnits();
//	testMarkMultiDeep();

//	testWasmRuntimeExtension();
//	operator_list = List(operator_list0);
//	testRecentRandomBugs();
	tests();// make sure all still ok before changes
	testAllWasm();
	todos();// those not passing yet (skip)
	tests();// make sure all still ok after changes
	printf("OK %ld==%ld", 2l, 2l);
	check(contains("OK %ld==%ld", "%ld"));
	log("CURRENT TESTS PASSED");
}

// valgrind --track-origins=yes ./wasp