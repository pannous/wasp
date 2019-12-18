Node result;

bool assert_equals(double a, double b, char *context = "") {
	if (a != b)// err
		puts("FAILED assert_equals! %f should be %f in %s"s % a % b % context);
	else puts("OK %f==%f in %s"s % a % b % context);
	return a == b;
}

bool assert_equals(String a, String b, char *context = "") {
	if (a != b)// err
		puts("FAILED assert_equals! %s should be %s in %s"s % a % b % context);
	else puts("OK %s==%s in %s"s % a % b % context);
	return a == b;
}

bool assert_equals(long a, long b, char *context) {
	if (a != b)// err
		puts("FAILED assert_equals! %d should be %d in %s"s % a % b % context);
	else puts("OK %d==%d in %s"s % a % b % context);
	return a == b;
}

bool assert_isx(char *mark, Node result) {
	try {
		Node left = Mark::eval(mark);
		if (left.type == floats or result.type == floats)
			return assert_equals(left.floate(), result.floate(), mark);
		if (left.type == longs or result.type == longs)
			return assert_equals(left.longe(), result.longe(), mark);
		return left == result;
	} catch (SyntaxError *err) {
		printf("\nTEST FAILED WITH ERROR\n");
		printf("%s", err->data);
	}
	return false;
}

Node assert_parsesx(const char *mark) {
	try {
		return Mark::parse(mark);
	} catch (chars err) {
		printf("\nTEST FAILED WITH ERROR\n");
		printf("%s", err);
	} catch (String err) {
		printf("\nTEST FAILED WITH ERROR\n");
		printf("%s", err.data);
	} catch (SyntaxError *err) {
		printf("\nTEST FAILED WITH ERROR\n");
		printf("%s", err->data);
	}
	return 0;
}

#define assert_parses(mark) result=assert_parsesx(mark);if(!result){printf("\n%s:%d\n",__FILE__,__LINE__);exit(0);}else log(result);

// MACRO to catch the line number. WHY NOT WITH TRACE? not precise:   testMath() + 376
#define assert_is(mark, result) {\
    printf("TEST %s==%s\n",#mark,#result);\
    bool ok=assert_isx(mark,result);\
    if(ok)printf("PASSED %s==%s\n",#mark,#result);\
    else{printf("FAILED %s==%s\n",#mark,#result);\
    printf("%s:%d\n",__FILE__,__LINE__);exit(0);}\
}

void testC();

void testBUG();

void testLists();

void testNetBase() {
	chars url = "http://de.netbase.pannous.com:8080/json/verbose/2";
	chars json = fetch(url);
	log(json);
	Node result = Mark::parse(json);
	assert(result["query"] == "2");
	assert(result["count"] == "1");
	assert(result["count"] == 1);
	Node results = result["results"];
	Node Erde = results[0];
	assert(Erde["name"] == "Erde");
//	assert(Erde.name == "Erde");
	assert(Erde["id"] == 2);
//	assert(Erde.id==2);
	Node &statements = Erde["statements"];
	assert(statements.length >= 1);
}

void testDiv() {
	Node div = Mark::parse("{div {span class:'bold' 'text'} {br}}");
	div["span"];
	div["span"]["class"];
}


void testMarkAsMap() {
	Node compare = Node();
//	compare["d"] = Node();
	compare["b"] = 3;
	compare["a"] = "HIO";
	Node &node = compare["a"];
	assert(node == "HIO");
	const char *source = "{b:3 a:'HIO'}";// d:{}
	Node marked = Mark::parse(source);
	Node &node1 = marked["a"];
	assert(node1 == "HIO");
	assert(marked["a"] == compare["a"]);
	assert(marked["b"] == compare["b"]);
	assert(compare == marked);
}

void testMarkSimple() {
	log("primitives");
	assert(Mark::parse("{a:3}"));
//	assert(Mark::parse("a=3")["a"] == 3);
	Node a = Mark::parse("{a:3}");
	Node &a3 = a["a"];
	assert(a3 == 3);
	assert(a3.type == longs);
	assert("a"s == a3.name);
//	assert(a3.name == "a"s);// todo? cant


	Node &b = a["b"];
	a["b"] = a3;
	assert(a["b"] == a3);
	assert(a["b"] == a3);
	assert(a["b"] == 3);

	assert(Mark::parse("3.") == 3.);
	assert(Mark::parse("3.") == 3.f);
//	assert(Mark::parse("3.1") == 3.1); // todo epsilon
//	assert(Mark::parse("3.1") == 3.1f);// todo epsilon
	assert(Mark::parse("'hi'") == "hi");
	assert(Mark::parse("3") == 3);
//		const char *source = "{a:3,b:4,c:{d:'hi'}}";
}


void testUTFinCPP() {
	// char = byte % 128   char<0 => utf or something;)
//	using namespace std;
//	const auto str = u8"عربى";
//	wstring_convert<codecvt_utf8<char32_t>, char32_t> cv;
//	auto str32 = cv.from_bytes(str);
//	for(auto c : str32)
//		cout << uint_least32_t(c) << '\n';
//		char a = '☹';// char (by definition) is one byte WTF WTF WTF WTF WTF WTF WTF WTF
//		wchar_t  a = '☹';// NOPE
//		char32_t a = '☹';// NOPE
//		__wchar_t__ a = '☹';// NOPE
//int a= '☹';// NOPE
//char* a='☹';// NOPE
//		char[10] a='☹';// NOPE
	char *a = "☹"; // OK
	byte *b = reinterpret_cast<byte *>(a);
//	a[0] = {char} -30 '\xe2'
//	a[1] = {char} -104 '\x98'
//	a[2] = {char} -71 '\xb9'
//	b[0] = {byte} 226 '\xe2'
//	b[1] = {byte} 152 '\x98'
//	b[2] = {byte} 185 '\xb9'
//	b[3] = {byte} 0 '\0'
}

//testUTFø  error: stray ‘\303’ in program
void testUTF() {
	testUTFinCPP();

	assert_parses("{ç:111}");
	assert(result["ç"] == 111);

	assert_parses("{ç:☺}");
	assert(result["ç"] == "☺");

	assert_parses("{ç:ø}");
	Node &node = result["ç"];
	node.log();
//	assert(node == "ø"); => OK
//	assert(node == NIL);
}


void testMarkMulti() {
//	const char *source = "{a:'HIO' d:{} b:3 c:ø}";
	const char *source = "{a:'HIO' d:{} b:3}";
	assert_parses(source);
	Node &node = result['b'];
	log(result['a']);
	log(result['b']);
	log(result['d']);
	log(result["a"]);
	log(result["b"]);
	log(result["d"]);
	assert(result['a'] == "HIO");
	assert(result["b"] == 3);
	assert(result['b'] == 3);
}

void testErrors() {
	throwing = false;
	Node node = Mark::parseFile("samples/errors.mark");
	throwing = true;
}

void testSamples() {
//	ln -s /me/dev/script/wasm/mark/samples /me/dev/script/wasm/mark/cmake-build-debug/
//	Node node= Mark::parseFile("samples/comments.mark");
	Node node = Mark::parseFile("samples/kitchensink.mark");

	assert(node['a'] == "classical json");
	assert(node['b'] == "quotes optional");
	assert(node['c'] == "commas optional");
	assert(node['d'] == "semicolons optional");
	assert(node['e'] == "trailing comments"); // trailing comments
	assert(node["f"] == /*inline comments*/ "inline comments");
}

void testEval3() {
	auto math = "one plus two";
	Node result = Mark::eval(math);
	assert(result == 3);
}

bool assert_equals(float a, float b, char *context) {
	float epsilon = fabs(a + b) / 100000.;// 𝕚𝚤:=-1
	bool ok = a == b or fabs(a - b) <= epsilon;
	if (!ok)
		puts("FAILED assert_equals! %f should be %f in %s"s % a % b % context);
//		 err("FAILED assert_equals! %d should be %d in %s"s % a % b % context);
	else puts("OK %f==%f in %s"s % a % b % context);
	return ok;
}


void testMath() {
	auto math = "one plus two times three";
	Node result = Mark::eval(math);
	assert(result == 7);
}

void testRoot() {
//	assert_is("√4", 2);
	assert_is("√4+40", 42);
	assert_is("40 + √4", 42);
	assert_is("40+√4", 42);
}

void testRootFloat() {
	assert_is("√42*√42", 42.);
//	assert_is("√42*√42", Node(42.));
//	assert_is("√42*√42", 42);

//	assert_is("√42*√42",42);// int rounding to 41 todo?
}

void testLists() {
	assert_parses("[1,2,3]");
	assert(result.length == 3);
	assert(result[2] == 3);
	assert(result[0] == 1);
	assert(result[0] == "1");
//	assert_is("[1,2,3]",1);
}


void testLogic() {
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


}

void testEval() {
	assert_is("√4", 2);
	testLogic();
	testMath();
	testRoot();
	testRootFloat();
}


void testC() {
	assert(1 < 2 < 3);
	assert('a' < 'b' < 'c');
	char b = 'b';
	assert('a' < b < 'c');
	assert_equals(String("abcd").substring(1, 2), "bc");
}


void test() {
	testMarkSimple();
	testMarkMulti();
	testMarkAsMap();
	testBUG();
	testC();
	testUTF();
	testDiv();
	testEval();
	testSamples();
	testErrors();
	testNetBase();
	testLists();
}

void testBUG() {
	const char *source = "{a:'HIO' d:{} b:3 c:ø}";
	assert_parses(source);
	log(result["a"].parent);// BROKEN, WHY??
}

void testCurrent() {
	testUTF();
//	printf("testCurrent OK\n NOW TESTING ALL\n");
	test();
}

