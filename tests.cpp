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

bool assert_parsesx(const char *mark) {
	try {
		Mark::parse(mark);
		return true;
	} catch (SyntaxError *err) {
		printf("\nTEST FAILED WITH ERROR\n");
		printf("%s", err->data);
	}
	return false;
}

#define assert_parses(mark) if(!assert_parsesx(mark)){printf("%s:%d\n",__FILE__,__LINE__);exit(0);}

// MACRO to catch the line number. WHY NOT WITH TRACE? not precise:   testMath() + 376
#define assert_is(mark, result) {\
    printf("TEST %s==%s\n",#mark,#result);\
    bool ok=assert_isx(mark,result);\
    if(ok)printf("PASSED %s==%s\n",#mark,#result);\
    else{printf("FAILED %s==%s\n",#mark,#result);\
    printf("%s:%d\n",__FILE__,__LINE__);exit(0);}\
}

void testNetbase() {

	chars url = "http://de.netbase.pannous.com:8080/json/verbose/2";
	chars json = fetch(url);
	log(json);
	Node div = Mark::parse(json);
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

void testMarkMulti() {
	const char *source = "{a:'HIO' d:{} b:3 c:ø}";
//	const char *source = "{d:{} b:3 a:'HIO'}";
	assert_parses(source);
//		const char *source = "a='hooo'";
	Node result = Mark::parse(source);
	Node &node = result['b'];
	log("OK");
	log(result);
	log(result[0]);
	log(result[1]);
	log(result["a"]);
	log(result["a"].parent);
	log(result["b"]);
	log(result["c"]);
	log(result["a"]);
	log(result["b"]);
	log(result["c"]);
//	assert(result['d']=={})

	assert(result["b"] == 3);
	assert(result['b'] == 3);
	assert(result['a'] == "HIO");


}

void testErrors() {
	throwing = false;
	Node node = Mark::parseFile("samples/errors.mark");
	throwing = true;
}

void testSamples() {
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
//	assert_is("√42*√42", Node(42.));
	assert_is("√42*√42", 42.);
	assert_is("√42*√42", 42);

//	assert_is("√42*√42",42);// int rounding to 41 todo?
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
}

void test() {
//	testNetbase();
	testMarkSimple();
	testMarkMulti();
	testMarkAsMap();
	testDiv();
	testEval();
	testSamples();
	testErrors();

}

void testCurrent() {
	testMarkMulti();
//	printf("testCurrent OK\n NOW TESTING ALL\n");
	test();
}
