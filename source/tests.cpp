#include "Node.h"
#include "Wasp.h"
#include "Angle.h"
#include "String.h"
#include "Map.h"
#include "tests.h"
#include "Paint.h"

#include "test_angle.cpp"
#include "test_wast.cpp"
#include "test_wasm.cpp"

//void testLogarithmInRuntime(){
//	float ℯ = 2.7182818284590;
//	//	assert_equals(ln(0),-∞);
//	assert_equals(log(100000),5.);
//	assert_equals(log(10),1.);
//	assert_equals(log(1),0.);
//	assert_equals(ln(ℯ*ℯ),2.);
//	assert_equals(ln(1),0.);
//	assert_equals(ln(ℯ),1.);
//}
void testLogarithm() {
	float ℯ = 2.7182818284590;
	//	assert_equals(ln(0),-∞);
	assert_emit("log(100000)", 5.);
	assert_emit("log(10)", 1.);
	assert_emit("log(1)", 0.);
	assert_emit("ℯ = 2.7182818284590;ln(ℯ*ℯ)", 2.);
	assert_emit("ln(1)", 0.);
	assert_emit("ℯ = 2.7182818284590;ln(ℯ)", 1.);
}

void testUpperLowerCase() {
	char string[] = "ABC";
	lowerCase(string, 0);
	assert_equals(string, "abc");
	skip(
			char string[] = "ÄÖÜ";
			lowerCase(string, 0);
			assert_equals(string, "äöü");
			char string[] = "ÂÊÎÔÛ ÁÉÍÓÚ ÀÈÌÒÙ AÖU";// String literals are read only!
			lowerCase(string, 0);
			assert_equals(string, "âêîôû áéíóú àèìòù aöu");
			char *string2 = (char *) u8"ÂÊÎÔÛ ÁÉÍÓÚ ÀÈÌÒÙ AÖU";
			lowerCase(string2, 0);
			assert_equals(string2, "âêîôû áéíóú àèìòù aöu");
			chars string3 = "ÂÊÎÔÛ ÁÉÍÓÚ ÀÈÌÒÙ AÖU";
	)
//	g_utf8_strup(string);
}

void testPrimitiveTypes() {
	assert_emit("double 2", 2);
	assert_emit("float 2", 2);
	assert_emit("int 2", 2);
	assert_emit("long 2", 2);
	assert_emit("8.33333333332248946124e-03", 0);
	assert_emit("8.33333333332248946124e+01", 83);
	assert_emit("S1  = -1.6666", -1);
	assert_emit("double S1  = -1.6666", -1);
//	assert_emit("double\n"
//	            "\tS1  = -1.6666", -1);

	assert_emit("grow(double z):=z*2;grow 5", 10);
	assert_emit("grow(z):=z*2;grow 5", 10);
	assert_emit("int grow(double z):=z*2;grow 5", 10);
	assert_emit("double grow(z):=z*2;grow 5", 10);
	assert_emit("int grow(int z):=z*2;grow 5", 10);
	assert_emit("double grow(int z):=z*2;grow 5", 10);
	assert_emit("double\n"
	            "\tS1  = -1.66666666666666324348e01, /* 0xBFC55555, 0x55555549 */\n"
	            "\tS2  =  8.33333333332248946124e03, /* 0x3F811111, 0x1110F8A6 */\n\nS1", -16);
	assert_emit("double\n"
	            "\tS1  = -1.66666666666666324348e01, /* 0xBFC55555, 0x55555549 */\n"
	            "\tS2  =  8.33333333332248946124e01, /* 0x3F811111, 0x1110F8A6 */\n\nS2", 83);
	assert_equals(ftoa(8.33333333332248946124e-03), "0.0083");
//	assert_equals(ftoa2(8.33333333332248946124e-03), "8.333E-3");
	assert_emit("S1 = -1.66666666666666324348e-01;S1*100", -16);
	assert_emit("S1 = 8.33333333332248946124e-03;S1*1000", 8);
	skip(
			assert_emit("(2,4) == (2,4)", 1);// todo: array creation/ comparison
			assert_emit("(float 2, int 4.3)  == 2,4", 1);//  PRECEDENCE needs to be in valueNode :(
			assert_emit("float 2, int 4.3  == 2,4", 1);//  PRECEDENCE needs to be in valueNode :(
	//	float  2, ( int ==( 4.3 2)), 4
	)
}

// One of the few tests which can be removed because who will ever change the sin routine?
void test_sin() {
	assert_equals(sin(0), 0);
	assert_equals(sin(pi / 2), 1);
	assert_equals(sin(-pi / 2), -1);
	assert_equals(sin(pi), 0);
	assert_equals(sin(2 * pi), 0);
	assert_equals(sin(3 * pi / 2), -1);

	assert_equals(cos(-pi / 2 + 0), 0);
	assert_equals(cos(0), 1);
	assert_equals(cos(-pi / 2 + pi), 0);
	assert_equals(cos(-pi / 2 + 2 * pi), 0);
	assert_equals(cos(pi), -1);
	assert_equals(cos(-pi), -1);
}


void testModulo() {
	assert_equals(mod_d(10007.0, 10000.0), 7)
	assert_emit("10007%10000", 7);
	assert_emit("10007.0%10000", 7);
	assert_emit("10007.0%10000.0", 7);
	assert_emit("10007%10000.0", 7);
	assert_emit("i=10007;x=i%10000", 7);
	assert_emit("i=10007.0;x=i%10000.0", 7);
	assert_emit("i=10007.1;x=i%10000.1", 7);
}

void testRepresentations() {
	result = parse("a{x:1}");
	auto result2 = parse("a:{x:1}");
	assert_equals(result.kind, reference);
	assert_equals(result2.kind, key);
//	a{x:1} ==
}

void testSignificantWhitespace() {
	result = parse("a b (c)");
	check(result.length == 3);
	result = parse("a b(c)");
	check(result.length == 2 or result.length == 1);
	result = parse("a b:c");
	check(result.length == 2);// a , b:c
	check(result.last().kind == key);// a , b:c
	result = parse("a: b c d");
	check(result.length == 3);
	check(result.name == "a"); // "a"(b c d), NOT ((a:b) c d)
	check(result.kind == groups);// not key!
	result = parse("a b : c");
	check(result.length == 1 or result.length == 2);// (a b):c
	assert_equals(result.kind, key);
	let x = data_mode;
	data_mode = false;
	result = parse("a b=c");
	check(result.length == 4);// a b = c
	print(result);
	result = analyze(result);
	print(result);
	skip(
			check(result.length == 1);// todo  todo => (a b)=c => =( (a b) c)
			data_mode = true;// HTML MODE!
			result = parse("a href=link.html");
			check(result.length == 1);// a(b=c)
	)
	data_mode = x;
	//1 + 1 ≠ 1 +1 == [1 1]
	assert(eval("1 + 1 == 2"));
	assert_is("1 +1", Node(1, 1, 0));
//	assert_is("1 +1", parse("[1 1]"));
	skip(
			assert_emit("1 +1 == [1 1]", 1);
			assert(eval("1 +1 == [1 1]"));
	)
	assert_emit("1 +1 ≠ 1 + 1", 1);
	assert(eval("1 +1 ≠ 1 + 1"));
}


void testComments() {
	let c = "from wasp import * # to silence python warnings;)\n y/* yeah! */=0 // really";
	result = parse(c);
	check(result.length == 2);
	check(result[0].length == 4);
	check(result[1].length == 3);
}

void testEmptyLineGrouping() {
	auto indented = R"(
a:
  b
  c

d
e
	)";
	auto groups = parse(indented);
	//	auto groups = parse("a:\n b\n c\n\nd\ne\n");
	check(groups.length == 3);// a(),d,e
	auto parsed = groups.first();
	check(parsed.length == 2);
	check(parsed[1] == "c");
	check(parsed.name == "a");
}


//[[maybe_used]]
[[nodiscard("replace generates a new string to be consumed!")]]
//__attribute__((__warn_unused_result__))
int testNodiscard() {
	return 54;
}


void testSerialize() {
	const char *inprint = "green=256*255";
	//	const char *inprint = "blue=255;green=256*255;maxi=3840*2160/2;init_graphics();surface=(1,2,3);i=10000;while(i<maxi){i++;surface#i=blue;};10";
	assertSerialize(inprint);
}


void testDedent2() {
	auto indented = R"(
a:
  b
  c
d
e
	)";
	auto groups = parse(indented);
	//	auto groups = parse("a:\n b\n c\nd\ne\n");
	print(groups.serialize());
	print(groups.length);
	check(groups.length == 3);// a(),d,e
	auto parsed = groups.first();
	check(parsed.name == "a");
	check(parsed.length == 2);
	print(parsed[1]);
	check(parsed[1].name == "c");
}

void testDedent() {
	auto indented = R"(
a
  b
  c
d
e
	)";
	auto groups = parse(indented);
	//	auto groups = parse("a:\n b\n c\nd\ne\n");
	print(groups.serialize());
	print(groups.length);
	check(groups.length == 3);// a(),d,e
	auto parsed = groups.first();
	check(parsed.name == "a");
	check(parsed.length == 2);
	print(parsed[1]);
	check(parsed[1].name == "c");
}

/*
void testWasmSpeed() {
	struct timeval stop, start;
	gettimeofday(&start, NULL);
	time_t s, e;
	time(&s);
	// todo: let compiler comprinte constant expressions like 1024*65536/4
	//out of bounds memory access if only one Memory page!
	//	assert_emit("i=0;k='hi';while(i<1024*65536/4){i++;k#i=65};k[1]", 65)// wow SLOOW!!!
	assert_emit("i=0;k='hi';while(i<16777216){i++;k#i=65};k[1]", 65)// still slow, but < 1s
	//	assert_emit("i=0;k='hi';while(i<16){i++;k#i=65};k[1]", 65)// still slow, but < 1s
	//	70 ms PURE C -O3   123 ms  PURE C -O1  475 ms in PURE C without optimization
	//  141 ms wasmtime very fast (similar to wasmer)
	//  150 ms wasmer very fast!
	//  546 ms in WebKit (todo: test V8/WebView2!)
	//	465 - 3511 ms in WASM3  VERY inconsistent, but ok, it's an interpreter!
	//	1687 ms wasmx (node.js)
	//  1000-3000 ms in wasm-micro-runtime :( MESSES with system clock! // wow, SLOWER HOW!?
	//	so we can never draw 4k by hand wow. but why? only GPU can do more than 20 frames per second
	//	sleep(1);
	gettimeofday(&stop, NULL);
	time(&e);

	printf("took %ld sec\n", e - s);
	printf("took %lu ms\n", ((stop.tv_sec - start.tv_sec) * 100000 + stop.tv_usec - start.tv_usec) / 100);

	exit(0);
}*/

void testImport() {
	assert_is("import fourty_two", 42);
	assert_is("include fourty_two", 42);
	assert_is("require fourty_two", 42);
	assert_is("import fourty_two;ft*2", 42 * 2);
	assert_is("include fourty_two;ft*2", 42 * 2);
	assert_is("require fourty_two;ft*2", 42 * 2);
}

void testWaspInitializationIntegrity() {
	check(not contains(operator_list0, "‖"))// it's a grouper!
}

void testColonLists() {
	auto parsed = parse("a: b c d");
	check(parsed.length == 3);
	check(parsed[1] == "c");
	check(parsed.name == "a");
}


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
	result.print();
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


void testHex() {
	assert_is("0xFF", 255);
	assert_is("0x100", 256);
}


void testNetBase() {
	warn("NETBASE OFFLINE");
	if (1 > 0)return;
	chars url = "http://de.netbase.pannous.com:8080/json/verbose/2";
//	print(url);
	chars json = fetch(url);
//	print(json);
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
	node.print();
	assert(div["span"].length == 2);
	assert(div["span"]["class"] == "bold");
}

void testDivMark() {
	polish_notation = true;
	Node div = parse("{div {span class:'bold' 'text'} {br}}");
	Node &span = div["span"];
	span.print();
	assert(span.length == 2);
	assert(span["class"] == "bold");
	polish_notation = false;
}

void testDiv() {
	Node result = parse("div{ class:'bold' 'text'}");
	result.print();
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
	data_mode = true; // else [ a , = , 3 ]
	assert_parses("a=3");
	Node &a = result["a"];
	assert(a == 3);
}

void testMarkSimple() {
	print("testMarkSimple");
	Node a = assert_parses("{aa:3}");
	assert_equals(a.value.longy, (long) 3);
	assert_equals(a, long(3));
	assert(a == 3);
	assert(a.kind == longs or a.kind == key and a.value.node->kind == longs);
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
//	print(character);
//	print(hanzi);
//	print(word);
	print(sizeof(char32_t));// 32 lol
	print(sizeof(wchar_t));

	assert_parses("ç='☺'");
	assert(eval("ç='☺'") == "☺");

	assert_parses("ç=☺");
	assert(result == "☺" or result.kind == expression);
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
	print(x3);
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
	print(result['a']);
	print(result['b']);
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
	// use assert_throws
	assert_throws("0/0");
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
	Node node = /*Wasp::*/parseFile("samples/errors.wasp");
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
			Mark::/*Wasp::*/parseFile(file.path().string().data());
	}
}
#endif
#endif
#endif

void testSample() {
	Node node = /*Wasp::*/parseFile("samples/comments.wasp");
}

void testNewlineLists() {
	Node node = parse("  c: \"commas optional\"\n d: \"semicolons optional\"\n e: \"trailing comments\"");
	assert(node['d'] == "semicolons optional");
}

void testKitchensink() {
	Node node = /*Wasp::*/parseFile("samples/kitchensink.wasp");
	node.print();
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
	Node empty;
	bool nothing = true;
	for (Node &child :empty)nothing = false;
	check(nothing);
	Node liste = parse("{1 2 3}");
	liste.print();
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
	result.print();
	assert_equals(result.length, 3);
	assert_equals(result.kind, patterns);
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


	assert_is("¬ 1", 0);
	assert_is("¬ 0", 1);

	assert_is("0 ⋁ 0", 0);
	assert_is("0 ⋁ 1", 1);
	assert_is("1 ⋁ 0", 1);
	assert_is("1 ⋁ 1", 1);

	assert_is("0 ⊻ 0", 0);
	assert_is("0 ⊻ 1", 1);
	assert_is("1 ⊻ 0", 1);
	assert_is("1 ⊻ 1", 0);

	assert_is("1 ∧ 1", 1);
	assert_is("1 ∧ 0", 0);
	assert_is("0 ∧ 1", 0);
	assert_is("0 ∧ 0", 0);
}

// use the bool() function to determine if a value is truthy or falsy.
void testTruthiness() {
	Node node = parse("true");
//	print("TRUE:");
	nl();
	print(node.name);
	nl();
	print(node.value.longy);
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
	result.print();
	Node &data = result["data"];
	data.print();
	Node &hero = data["hero"];
	hero.print();
	Node &height = data["hero"]["height"];
	height.print();
	Node &id = hero["id"];
	id.print();
	assert(id == "R2-D2");
	assert(height == 5.6430448);
//	assert(height==5.643);
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
	hero.print();
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
// 0x0E 	SO 	␎ 	^N 		Shift Out
// 0x0F 	SI 	␏ 	^O 		Shift In
	//	indent/dedent  0xF03B looks like pause!?   0xF032…  it does, what's going on CLion? Using STSong!
	//	https://fontawesome.com/v4.7/icon/outdent looks more like it, also matching context  OK in font PingFang HK?
}// 􀖯􀉶𠿜🕻🗠🂿	𝄉

void testParentContext() {
	chars source = "{a:'HIO' d:{} b:3 c:ø}";
	assert_parses(source);
	result.print();
	Node &a = result["a"];
	a.print();
	assert_equals(a.kind, strings);
	assert_equals(a.value.string, "HIO");
	assert_equals(a.string(), "HIO");// keyNodes go to values!
	assert(a == "HIO");
//	assert_equals(a.name,"a" or"HIO");// keyNodes go to values!
	skip(
			assert_equals(a.kind, key);
			assert(a.name == "HIO");
	)
}

void testParentBUG() {
//	chars source = "{a:'HIO' d:{} b:3 c:ø}";
	chars source = "{a:'HIO'}";
	assert_parses(source);
	Node &a = result["a"];
	check(a.kind == key or a.kind == strings);
	check(a == "HIO");
	check(a.parent == 0);// key a is highest level
	Node *parent = a.value.node->parent;
	check(parent);
	print(parent);// BROKEN, WHY?? let's find out:
	check(*parent == result);
	skip(
	// pointer identity broken by flat() ?
			check(parent == &result);
	)
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
	assert_is("١٢٣", 123);
	assert_equals(atoi0("١٢٣"), 123);
	check_eq(atoi0("123"), 123);// can crash!?!
//	assert_equals( atoi1(u'₃'),3);// op
	assert_equals(atoi0("0"), 0);
	assert_equals(atoi0("x"), 0);// todo side channel?
	assert_equals(atoi0("3"), 3);
	assert_equals(atoi1('x'), -1);
	assert_equals(atoi1('3'), 3);
	check_eq(" a b c  \n"s.trim(), "a b c");
	assert_equals("     \n   malloc"s.trim(), "malloc");
	assert_equals("     \n   malloc     \n   "s.trim(), "malloc");
	assert_equals("malloc     \n   "s.trim(), "malloc");
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
	//	address of 'NIL' will always evaluate to 'true' because NIL is const now!
//	check(!(other == &NIL));
//	check(not(&other == &NIL));
//	check(not(other == &NIL));
	check(other != NIL);
#ifndef WASM
//	check(other != &NIL);
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
	node1234.print();

	check_eq(node1234.length, 4);

	node1234.children[node1234.length - 2].print();
	node1234.children[node1234.length - 1].print();
	node1234.last().print();
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
	label0.print();
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
	node.print();
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
	result.print();
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
	print(typeName(b.kind));
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

void testGroupCascade0() {
	result = parse("x='abcde';x#4='y';x#4");
	check(result.length == 3);
}

void testGroupCascade1() {
	Node result0 = parse("a b; c d");
	check(result0.length == 2);
	check(result0[1].length == 2);
	Node result = parse("{ a b c, d e f }");
	Node result1 = parse("a b c, d e f ");
	assert_equals(result1, result);
	Node result2 = parse("a b c; d e f ");
	assert_equals(result2, result1);
	assert_equals(result2, result);
	Node result3 = parse("a,b,c;d,e,f");
	assert_equals(result3, result2);
	assert_equals(result3, result1);
	assert_equals(result3, result);
	Node result4 = parse("a, b ,c; d,e , f ");
	assert_equals(result4, result3);
	assert_equals(result4, result2);
	assert_equals(result4, result1);
	assert_equals(result4, result);
}

void testGroupCascade2() {
	result = parse("{ a b , c d ; e f , g h }");
	Node result1 = parse("{ a b , c d \n e f , g h }");
	print(result1.serialize());
	assert_equals(result1, result);
	Node result2 = parse("a b ; c d \n e f , g h ");
	assert_equals(result1, result2);
	assert_equals(result2, result);
}

void testSuperfluousIndentation() {
	result = parse("a{\n  b,c}");
	Node result1 = parse("a{b,c}");
	check(result1 == result);
}

void testGroupCascade() {
//	testGroupCascade2();
//	testGroupCascade0();
//	testGroupCascade1();

	result = parse("{ a b c, d e f; g h i , j k l \n "
	               "a2 b2 c2, d2 e2 f2; g2 h2 i2 , j2 k2 l2}"
	               "{a3 b3 c3, d3 e3 f3; g3 h3 i3 , j3 k3 l3 \n"
	               "a4 b4 c4 ,d4 e4 f4; g4 h4 i4 ,j4 k4 l4}");
	result.print();
	assert_equals(result.kind, groups);// ( {} {} ) because 2 {}!
	auto first = result.first();
	assert_equals(first.kind, objects);// { a b c … }
	assert_equals(first.first().kind, groups);// or expression if x is op
	assert_equals(result.length, 2)// {…} and {and}
	assert_equals(result[0].length, 2) // a…  and a2…  with significant newline
	assert_equals(result[0][0].length, 2)// a b c, d e f  and  g h i , j k l
	assert_equals(result[0][0][0].length, 2)// a b c  and  d e f
	assert_equals(result[0][0], parse("a b c, d e f; g h i , j k l"));// significant newline!
	assert_equals(result[0][1], parse("a2 b2 c2, d2 e2 f2; g2 h2 i2 , j2 k2 l2"));// significant newline!
	assert_equals(result[0][0][0][0].length, 3)// a b c
	assert_equals(result[0][0][0][0], parse("a b c"));
	assert_equals(result[0][0][0][0][0], "a");
	assert_equals(result[0][0][0][0][1], "b");
	assert_equals(result[0][0][0][0][2], "c");
	assert_equals(result[0][0][0][1][0], "d");
	assert_equals(result[0][0][0][1][1], "e");
	assert_equals(result[0][0][0][1][2], "f");
	assert_equals(result[1][1][0][1][2], "f4");
	Node reparse = parse(result.serialize());
	print(reparse.serialize());
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
	check(d.kind == key);
	a.addSmart(b);// why?
}

void testLogicOperators() {
	assert_is("¬ 0", 1);
	assert_is("¬ 1", 0);

	assert_is("0 ⋁ 0", 0);
	assert_is("0 ⋁ 1", 1);
	assert_is("1 ⋁ 0", 1);
	assert_is("1 ⋁ 1", 1);

	assert_is("0 ⊻ 0", 0);
	assert_is("0 ⊻ 1", 1);
	assert_is("1 ⊻ 0", 1);
	assert_is("1 ⊻ 1", 0);

	assert_is("1 ∧ 1", 1);
	assert_is("1 ∧ 0", 0);
	assert_is("0 ∧ 1", 0);
	assert_is("0 ∧ 0", 0);
}

void testBUG();

void tests() {
//	data_mode = true;// expect data unless explicit code
	testBUG();
	testSignificantWhitespace();
	testUpperLowerCase();
	testSerialize();
	testPrimitiveTypes();
	test_sin();
	testModulo();
	testRecentRandomBugs();
	testIndentAsBlock();
	testDeepCopyDebugBugBug2();// SUBTLE: BUGS OUT ONLY ON SECOND TRY!!!
	testDeepCopyDebugBugBug();
	testComments();
	testEmptyLineGrouping();
	testWaspInitializationIntegrity();
	testRoundFloorCeiling();
	testSwitch();
	testAsserts();
	testSuperfluousIndentation();
	testString();
	testEmptyLineGrouping();
	testColonLists();
	testGraphParams();
	testNodeName();
	testStringConcatenation();
	testNodeBasics();
	testStringReferenceReuse();
	testWasmString();// with length as header
	testTruthiness();
	testNodeConversions();
	testConcatenation();
	testMarkSimple();
	testMarkMulti();
	testMarkMulti2();
	testDedent2();
	testDedent();
	testGroupCascade0();
	testGraphQlQuery();
	//	testNodiscard();// works NOW!
	print(testNodiscard());
	testGroupCascade();
	testNewlineLists();
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
	testHex();
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
	testAddField();
	testOverwrite();
	testMapsAsLists();
	testDidYouMeanAlias();
	testNetBase();
	testRoots();
	testForEach();
	testLengthOperator();
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
	testLogic01();// fails in WASM why
	testMathExtra(); // fails in WASM
	testRecentRandomBugs();
	testGroupCascade();
	testLogic();
	testLogicOperators();
	assert_is("one plus two times three", 7);
	data_mode = true;
	testParams();
	skip(
			testKitchensink();
	)
#ifdef APPLE
	testAllSamples();
#endif
	check(NIL.value.longy == 0);// should never be modified
	print("ALL TESTS PASSED");
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
			assert_emit("(2,4) == (2,4)", 1);// todo: array creation/ comparison
			assert_emit("2,4 == 2,4", 1);
			assert_emit("(2 as float, 4.3 as int)  == 2,4", 1);
			assert_emit("(2 as float, 4.3 as int)  == 2,4", 1);
	)
	skip(
			assert_emit("‖-2^2 - -2^3‖", 4);// Too many args for operator ‖,   a - b not grouped!
			data_mode = false;testParams();
			run("circle.wasp");
			assert_emit("1 +1 == [1 1]", 1);
			assert_emit("1 +1 ≠ 1 + 1", 1);
			testWasmMutableGlobal();
	)
	skip( // todo soon

	// while without body
			assert_emit("i=0;while(i++ <10001);i", 10000)// parsed wrongly! while(  <( ++ i 10001) i)
			assert_emit("1 - 3 - square 3+4", (long) -51);// OK!
			assert_emit("1 -3 - square 3+4", (long) -51);// warn "mixing math op with list items (1, -3 … ) !
			assert_emit("1 - - 3", 4);// -1 uh ok?  warn "what are you doning?"
			assert_emit("use math;⅓ ≈ .3333333 ", 1);
			assert_emit("precision = 3 digits; ⅓ ≈ .333 ", 1);
			assert_throws("i*=3");// well:
			assert_emit("i*=3", (long) 0);
			globals.setDefault(new Node());
			globals["y"] = new Node();
			// todo: ERRORS when cogs don't match! e.g. remove ¬ from prefixOperators!
			assert_throws("ceiling 3.7");
			// default bug!
			//    	subtract(other complex) := re -= other.re; im -= other.im
			// := is terminated by \n, not by ;!
			assert_throws("xyz 3.7"); // todo SHOULD THROW unknown symbol!
	)
	testSignificantWhitespace();
	skip(
			assert_eval("if(0):{3}", false);// 0:3 messy node
			assert_equals(Node("1", 0) + Node("2"_s),
			              Node("1", "2", 0));// 1+2 => 1:2  stupid border case because 1 not group (1)
			assert_is("{a b c}#2", "b");// ok, but not for patterns:
			assert_is("[a b c]#2", "b");// patterns
			assert_is("i=3;i--", 2);// todo bring variables to interpreter
			assert_is("i=3.7;.3+i", 4);// todo bring variables to interpreter
			assert_is("i=3;i*-1", -3);// todo bring variables to interpreter
			testAngle();// fails in WASM why?

			testNetBase();
//	print("OK %s %d"s % ("WASM",1));// only 1 handed over
			print("OK %d %d"s % (2, 1));// only 1 handed over
	)

}

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
	print(typeName(b.kind));
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

void testUnits() {
	assert_is("1 m + 1km", Node(1001).setType("m"));
}

void testPaint() {
#ifdef SDL
	init_graphics();
	while (1)paint(-1);
#endif
}


/*void print_timestamp() {
	time_t rawtime;
	struct tm *timeinfo;
	time(&rawtime);
	timeinfo = localtime(&rawtime);
	char buffer[30];
	strftime(buffer, 30, "%G/%m/%d-%H:%M:%S.%ffff", timeinfo);
	prints(buffer);
//	return reinterpret_cast<char *>(buffer);
}*/

#include <sys/time.h>

void testPaintWasm() {
#ifdef GRAFIX
	struct timeval stop, start;
	gettimeofday(&start, NULL);
	// todo: let compiler comprinte constant expressions like 1024*65536/4
//	assert_emit("i=0;k='hi';while(i<1024*65536/4){i++;k#i=65};k[1]", 65)// wow SLOOW!!!
//out of bounds memory access if only one Memory page!
//	assert_emit("i=0;k='hi';while(i<16777216){i++;k#i=65};paint()", 0)// still slow, but < 1s
	// wow, SLOWER in wasm-micro-runtime HOW!?
//	exit(0);

//(√((x-c)^2+(y-c)^2)<r?0:255)
//(x-c)^2+(y-c)^2
//	assert_emit("h=100;r=10;i=100;c=99;r=99;x=i%w;y=i/h;k=‖(x-c)^2+(y-c)^2‖<r",1);
////char *wasm_paint_routine = "surface=(1,2);i=0;while(i<1000000){i++;surface#i=i*(10-√i);};paint";
	char *wasm_paint_routine = "w=1920;c=500;r=100;surface=(1,2);i=0;"
							   "while(i<1000000){"
							   "i++;x=i%w;y=i/w;surface#i=(x-c)^2+(y-c)^2"
							   "};paint";
//((x-c)^2+(y-c)^2 < r^2)?0x44aa88:0xffeedd
//char *wasm_paint_routine = "surface=(1,2);i=0;while(i<1000000){i++;surface#i=i;};paint";
//assert_emit(wasm_paint_routine, 0);
//	char *wasm_paint_routine = "maxi=3840*2160/4/2;init_graphics();surface=(1,2,3);i=0;while(i<maxi){i++;surface#i=i*(10-√i);};";
	emit(wasm_paint_routine);
//	paint(0);
	gettimeofday(&stop, NULL);
//	printf("took %lu µs\n", (stop.tv_sec - start.tv_sec) * 100000 + stop.tv_usec - start.tv_usec);
	printf("took %lu ms\n", ((stop.tv_sec - start.tv_sec) * 100000 + stop.tv_usec - start.tv_usec) / 100);
//	exit(0);
//char *wasm_paint_routine = "init_graphics(); while(1){paint()}";// SDL bugs a bit
	while (1)paint(0);// help a little
#endif
}

void testNodesInWasm() {
	assert_emit("{b:c}", parse("{b:c}"));
	assert_emit("a{b:c}", parse("a{b:c}"));
}


void testSubGroupingIndent() {
	result = parse("x{\ta\n\tb,c,\n\td;\n\te");
	assert_equals(result.length, 3);
	assert_equals(result.first(), "a");
	assert_equals(result.last(), "e");
}

void testSubGrouping() {// dangling , should make '\n' not close
//	result=parse("a\nb,c,\nd;e");
	result = parse("a\n"
	               "b,c,\n"
	               "d;\n"
	               "e");
	assert_equals(result.length, 3);
	assert_equals(result.first(), "a");
	assert_equals(result.last(), "e");
	testSubGroupingIndent();
}

// 2021-10 : 40 sec for Wasm3
// 2021-10 : 10 sec in Webapp!
void testCurrent() {
	//	throwing = false;// shorter stack trace
	//	panicking = true;//
	data_mode = true; // a=b => a{b}
//	data_mode = false; // a=b => a,=,b before analysis
	clearContext();
//	testLogarithm();
//	assert_emit("1 +1 == [1 1]", 1);
//	testSubGrouping();
	check(parse("false and false").length == 3);
	assert_emit("false and false", false);

	assert_emit("grow:=it*2; grow 3*4", 24)

	auto uff = "r = 1 + 2*3 + 1";
	assert_emit(uff, 8);
//	auto uff = "r = 1 + 2*3 + 4";// DIFFERENT BUG EHHHW
//	auto uff = "double sin(x){a}";
//auto uff = "r = S2 + z + z*w*(S5 + z*S6)";
//	auto uff = "double sin(double x){\n\tz=1;\n"
//	           "\tdouble\n"
//	           "\tS1  = -1.66666666666666324348e-01, /* 0xBFC55555, 0x55555549 */\n"
//	           "\tS2  =  8.33333333332248946124e-03, /* 0x3F811111, 0x1110F8A6 */\n"
//	           "\tw = z*z;\n"
//	           "\tr = S2 + z*(S3 + z*S4) + z*w*(S5 + z*S6);}";

//	assertSerialize(uff);

//	assert_emit("use sin;sin π/2", 1);
//	assert_emit("use sin;sin π", 0);

//testNodesInWasm();
//testSubGrouping();
//	testMergeWabt();
//	run( "wasp.wasm");
//	testPaintWasm();
	//	assert_run("render html{'test'}", 4);
//	testImportWasm();
//	testImport();
//	exit(1);
	//	return;// let the webview show!
	todos();// those not passing yet (skip)
	testAllWasm();
	tests();// make sure all still ok before changes
	print("CURRENT TESTS PASSED");
}

// valgrind --track-origins=yes ./wasp