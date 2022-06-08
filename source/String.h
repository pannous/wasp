#pragma once
//
// Created by pannous on 19.12.19.
//
#include "wasm_helpers.h"
#include "Util.h"
//#include "NodeTypes.h"
#include <cstdlib> // OK in WASM!

#define MAX_STRING_LENGTH 10000
#define MAX_DATA_LENGTH 0x1000000

//#include "Map.h" recursive include error Node.h:60:9: error: field has incomplete type 'String'

#ifndef WASM

#include <cstdlib>
#include <cstdio> // printf

#endif


typedef const unsigned char *wasm_string;// wasm strings start with their LEB encoded length and do NOT end with 0 !! :(

class String;

template<class S>
class List;

extern String &EMPTY_STRING;

//What 'char' and 'wchar_t' represent are completely ambiguous.
//You might think that they represent a "character", but depending on the encoding, that might not be true.
//typedef byte char;// overloaded term that can mean many things but throws a compiler warning
// character in the real world is LESS ambivalent then in the UTF world! on the other hand a new term like icon/grapheme cant hurt
//typedef wchar_t character;// overloaded term that can mean many things:
typedef char32_t codepoint;// ☃ is a single code point but 3 UTF-8 code units (char's), and 1 UTF-16 code unit (char16_t)
//typedef codepoint operator_char;
//class Codepoint{ char32_t nr; };// to avoid wrong autocast codepoint x=string[i]
//typedef char* grapheme;// sequence of one or more code points that are displayed as a single 'character' ä may be two code points a¨, or one ä
typedef String grapheme;// sequence of one or more code points that are displayed as a single 'character' ☀️=☀+_ e2 98 80 + ef b8 8f
//typedef String grapheme;// usually codepoint + color or something. no need yet? boycott idea?
// TODO IS IT SAFE IF WE USE char IN WASP as synonym for codepoint, by merging graphemes and ignoring modifiers? we can return a¨ as  ä!!
// ⚠ color is an invisible control character like in ascii ⚠️=⚠ + ef b8 8f

//typedef Image glyph; image, usually stored in a font (which is a collection of glyphs), used to represent graphemes
//https://stackoverflow.com/questions/27331819/whats-the-difference-between-a-character-a-code-point-a-glyph-and-a-grapheme
// '\uD83D\uDC0A' UTF-16 code units expressing a single code point (U+1F40A)  char(0x1F40A) == '🐊'
//https://github.com/foliojs/grapheme-breaker/blob/master/src/GraphemeBreaker.js

enum sizeMeasure {
	by_char8s,// bytes
//	char16s,
	by_codepoints,
	by_graphemes
};

void reverse(char *str, int len);

codepoint decode_unicode_character(chars utf8bytes, short *len = 0);

short utf8_byte_count(codepoint c);

//decode_codepoint
char *itoa0(long num, int base);

char *itoa0(long num);

char *itoa(long num);

int atoi1(codepoint c);

long atoi0(chars str);

double atof0(chars string);

void encode_unicode_character(char *buffer, wchar_t ucs_character);

//void* calloc(int i);
//extern "C" void* calloc(int size,int count);
//extern "C" void* calloc(int size);
//extern "C"

//void *calloc(size_t nitems, size_t size);// __result_use_check __alloc_size(1,2);
//void *calloc(size_t nitems, size_t size){
//	void *mem = alloc(nitems*size);
//	while (nitems > 0) { ((char *) mem)[--nitems] = 0; }
//	return mem;
//}

//#include "Node.h"

//#ifndef WASP_STRING
//#define WASP_STRING
class Node;

typedef chars chars;

#ifndef WASM
//#include <string>
//#include <stdlib.h> // pulls in declaration of malloc, free
#else
#endif
//void* alloc(number size);// wasm | linux


//extern void error1(chars message, chars file = 0, int line = 0);

//void error1(String message, chars file = 0, int line = 0);

class String;

String toString(Node &node);

String str(chars &s);

String str(char *string);

String s(chars &s);

//int ord(codepoint c);// identity!

bool eq(chars dest, chars src, int length = -1);

void strcpy2(char *dest, chars src);

void strcpy2(char *dest, chars src, int length);

int strlen0(chars x);
//size_t   strlen(chars __s);


//chars ftoa(float num, int base = 10, int precision = 4);
chars ftoa0(float num, int base, int precision);

chars ftoa2(float num, int significant_digits = 4);

chars ftoa(float num);

class Error {
public:
	char *message;

	Error() {
		message = "error";
	}

	Error(char *string) {
		message = string;
	}
//		Error(String *string) {
//		member access into incomplete type 'String'
//		message = string->data;
//	}
};

class IndexOutOfBounds : ::Error {
public:
//	IndexOutOfBounds(String *string1) : Error(string1){}
	IndexOutOfBounds(char *data, int i) : Error() {}
};

extern char *empty_string;// = "";

//duplicate symbol '_empty_string'


//char null_value[]={0};// todo make sure it's immutable!!

// todo: make String struct-compatible with Node!?
class String {
// sizeof(Node) == 20 == 5 * 4 int(*)

#ifdef WASM
	//#define size_t unsigned number
	//	void* calloc(size_t s,int idk);
#else
#ifndef __APPLE__
	//#include <alloc.h>
#endif
#endif

private:
	codepoint *codepoints = 0;// extract on demand from data
	int codepoint_count = -1;

public:
	// memory layout different to chars with leb length header!
	char *data{};
	int length = -1;
	bool shared_reference = false;// length terminated substrings!

	String() {
//		assert(null_value[0] == 0);
//		data = 0;
		data = "";
//		data = empty_string;
//		data =  {0};//null_value;
//		data = (char *)calloc(1, 1);
//		data = (char*)(calloc(1, 1));
		length = 0;
	}

//#ifndef WASM
////	Error while importing "env"."_Znwm": unknown import.
	void *operator new(size_t size) {
		return static_cast<String *>(calloc(size, 1));// WOW THAT WORKS!!!
	}
//#endif

	void operator delete(void *) {/*lol*/} // Todo ;)

//	~String()=default;

	explicit String(char *datas, int len, bool share) {
		data = datas;
		length = len;
		shared_reference = share;
		if (!share) {
			data = (char *) alloc(sizeof(char), length + 1);// including \0
			if (length > 0)
				memcpy(data, datas, length);
			else
				strcpy2(data, datas, length);
			data[length] = 0;
		}
	}

	explicit String(char byte_character) {
		if (byte_character == 0) {
			length = 0;
			data = 0;//SUBTLE BUGS if setting data=""; data=empty_string;
			return;
		}
		data = (char *) (alloc(sizeof(char), 2));
		data[0] = byte_character;
		data[1] = 0;
		length = 1;
	}

//	explicit
	String(const char string[], bool copy = true/*false AFTER all is tested, for efficiency*/) {
		// todo (maybe dont) mark data as to-free on destruction once copy = false AND bool malloced = true
//		data = const_cast<char *>(string);// todo heap may disappear, use copy!
		length = strlen0(string);
		if (length == 0)data = 0;//SUBTLE BUGS if setting data="" data=empty_string !!!;//0;//{data[0]=0;}
		else {
			if (copy) {
				data = (char *) (alloc(sizeof(char), length + 1));
				strcpy2(data, string, length);
			} else {
				shared_reference = true;
				data = (char *) string;
			}
		}
	}

	explicit
	String(wasm_string s) {
		shared_reference = true;
		short length_bytes = 1;// LEB128 length encoding header
		length = s[0];
		if (length < 0 or length >= 128)
			todo("decode full LEB128");
		data = (char *) s + length_bytes;
	}

//		String operator+(Type e){
//	explicit String(::Kind type) : String(typeName(type)) {}// lil hack to get String of specific enums
//	explicit String(Type type) : String(typeName(type)) {}// lil hack to get String of specific enums

	explicit String(int integer) {
		data = itoa(integer);// wasm function signature contains illegal type WHYYYY
		length = len(data);
	}

	explicit String(long number) {
		data = itoa0(number);
		length = len(data);
	}


	explicit String(char16_t utf16char) {
		data = (char *) (calloc(sizeof(char16_t), 2));
		encode_unicode_character(data, utf16char);
		length = len(data);// at most 2 bytes
	}

//	explicit String(char16_t* chars){
//		data = (char*)(calloc(sizeof(char16_t),len(chars)));
//		encode_unicode_characters(data, chars);
//		length = len(data);
//	}

// char32_t same as codepoint!
	explicit String(char32_t utf32char) {// conflicts with int
		data = (char *) (calloc(sizeof(char32_t), utf8_byte_count(utf32char) + 1));
		encode_unicode_character(data, utf32char);
		length = len(data);// at most 4 bytes
		data[length] = 0;// be sure
	}

	explicit String(wchar_t wideChar) {
		data = (char *) (calloc(sizeof(wchar_t), 2));
		encode_unicode_character(data, wideChar);
		length = len(data);
	}

	explicit String(double real) {
		int max_length = 4;
		data = itoa0(real);
		length = len(data);
//		itof :
		append('.');
		real = real - (long(real));
		while (length < max_length) {
			real = (real - long(real)) * 10;
			if (int(real) == 0)break;// todo 0.30303
			append(int(real) + '0');// = '0'+1,2,3
		}
	}

#ifdef std
	String(std::string basicString) {
		data = const_cast<char *>(basicString.data());// copy?
	}
#endif


	codepoint codepointAt(int i) {
		if (!codepoints)extractCodepoints();
		if (i > codepoint_count)
			error("index > codepoint count");
		return codepoints[i];
	}

//	grapheme graphemeAt(int i) {
//		todo("grapheme");
//	}

	codepoint *extractCodepoints(bool again = false);

	int size(enum sizeMeasure by = by_codepoints) {
		if (by == by_char8s)return length;
		if (by == by_codepoints) {
			if (codepoint_count < 0)extractCodepoints();
			return codepoint_count;
		}
		if (by == by_graphemes) {
			todo("graphemes");
		}
		todo("unknown sizeMeasure");
		return -1;
	}

	char charAt(int position) {
		if (position >= length)
			error((String("IndexOutOfBounds at ") + itoa0(position) + " in " + data).data);
		return data[position];
	}

	char charCodeAt(int position) {
//		if (position >= length)
//			raise(IndexOutOfBounds(data, position).message);
//		String("IndexOutOfBounds at ") + i + " in " + data;
//			throw new IndexOutOfBounds(String(" at ") + i + " in " + data);
		return data[position];
	}

	int indexOf(char c, int from = 0) {
		for (int j = from; j < length and j < MAX_STRING_LENGTH; j++) {
			if (data[j] == c)return j;
		}
		return -1;
	}

//	operator std::string() const { return "Hi"; }

// excluding to
	String substring(int from, int to = -1, bool ref = false /* true after all is tested*/) { // excluding to
		if (from < 0 or (from == 0 and (to == length or to == -1))) return *this;
		if (to < 0) to = length + to + 1;// -2 : skip last character
		if (to > length or to < -length) to = length;
		if (to <= from) return EMPTY_STRING;
		if (from >= length) return EMPTY_STRING;
		int len = to - from;
		return String(data + from, len, ref);
//		auto *neu = new String(data + from, len, ref);
//		return *neu;
	}

	int len(chars data) {
		if (!data)data = this->data;
		if (!data || data[0] == 0)
			return 0;
		int MAX = 10000;
		for (int i = 0; i < MAX; ++i) {
			if (!data[i])return i;
		}
		return -1;//error
	}


	String &append(codepoint c) {
		auto byteCount = utf8_byte_count(c);
		if (!data) {
			data = (char *) (alloc(sizeof(char), byteCount + 1));
		} else if (data + length + 1 == (char *) current) {// just append recent
			current += byteCount + 1;
		} else {
			auto *neu = (char *) (alloc(sizeof(char), length + 5));// we need 4 bytes because *(int*)…=c
			if (data)strcpy2(neu, data, length);
			data = neu;
		}
		encode_unicode_character(data + length, c);
		length += byteCount;
		data[length] = 0;
		return *this;
	}

	String &clone() {
		return *new String(this->data, this->length, false);
	}


	String operator%(String &c) {
		if (!contains("%s"))
			return *this + c;
		String b = this->clone();
		String d = b.replace("%s", c);
		return d;
	}

	String operator%(String *c) {
		if (!c)return *this;
		if (!contains("%s"))
			return *this + c;
		String b = this->clone();
		String d = b.replace("%s", *c);
		return d;
	}


	String operator%(Node &c) {
		if (!contains("%s"))
			return *this + toString(c);
		String b = this->clone();
		String d = b.replace("%s", toString(c));
		return d;
	}

	String operator%(chars &c) {
		return this->replace("%s", c);
	}

	String operator%(chars c) {
		return this->replace("%s", c);
	}

	String operator%(char *c) {
		return this->replace("%s", c);
	}

	String operator%(char c) {
		return this->replace("%c", String(c).data);
	}

	String operator%(int d) {
		return this->replace("%d", itoa0(d));
	}

	String operator%(long d) {
//		todo %l %ld %lld ??
		if (contains("%x"))
			return this->replace("%x", hex(d));
		else
			return this->replace("%d", itoa0(d));
	}

	String operator%(double f) {
		String formated = String() + itoa0(f) + "." + itoa0((f - int(f)) * 10000);
		return this->replace("%f", formated);
	}
//
//	String operator%(float f) {
//		String formated = String() + itoa0(f) + "." + itoa0((f - int(f)) * 10000);
//		return this->replace("%f", formated);
//	}

	String *operator+=(String &c) {
		this->data = (*this + c).data;
		this->length += c.length;
		return this;
	}

	String *operator+=(String *c) {
		this->data = (*this + c).data;
		this->length += c->length;
		return this;
	}

	String *operator+=(char *c) {
		while (c and c[0] && c++)append(c[-1]);
		return this;
	}

	String *operator+=(chars c) {
		while (c and c[0] && c++)append(c[-1]);
		return this;
	}

//	 DANGER string + ' ' + " " yields NONSENSE!!!
//	String *operator+=(char c) {
//		append(c);
//		return this;
//	}

	String *operator+=(codepoint c) {
		append(c);
		return this;
	}

	String &operator+(codepoint c) {
		append(c);
		return *this;
	}

	// todo self-modifying lol
	String &operator+(wchar_t c) {
		append(c);
		return *this;
	}

	[[nodiscard]]
	String &operator+(String c) {
		if (c.length <= 0)
			return *this;
		auto *neu = (char *) alloc(sizeof(char), length + c.length + 1);
#ifdef cstring
		if (data)strcpy(neu, data);
		if (c.data)strcpy(neu + length, c.data);
#else
		if (data)strcpy2(neu, data, length);
		if (c.data)strcpy2(neu + length, c.data, c.length);
#endif
		neu[length + c.length] = 0;
//		put(neu);
		String *ok = new String(neu);
		ok->length = length + c.length;
		return *ok;
	}

	String operator+(const char x[]) {
		return this->operator+(String(x));
	}
//	String operator+(float i) {
//		return this->operator+(String(i));
//	}

	String operator+(bool b) {
		return this->operator+(b ? "✔️" : "✖️");//✓
//		return this->operator+(b ? " true" : " false");
	}

	String operator+(double i) {
		return this->operator+(String(i));
	}

	String operator+(int i) {
		return this->operator+(String(i));
	}

	String operator+(long i) {
		return this->operator+(String(i));
	}

	String operator+(char c) {
		return this->operator+(String(c));
	}

	String operator+(String *s) {
		if (!s or !s->data)return *this;
		return this->operator+(s->data);
	}

	String operator++() {
		if (length <= 0)return "";
		this->data++;// self modifying ok?
		length--;
		return *this;
	}

	String operator++(int postfix) {//
		if (length <= 0)return "";
		this->data += 1 + postfix;// self modifying ok?
		length -= 1 + postfix;
		return *this;
	}

	String operator+(char *c) {
		return this->operator+(String(c));
	}


	bool operator==(char16_t c) {
		return this->operator==(String(c));
	}

//	check(U'牛' == "牛"s );// owh wow it works reversed
	bool operator==(char32_t c) {
		return this->operator==(String(c));
	}

	bool operator==(wchar_t c) {
		return this->operator==(String(c));
	}

	bool operator==(char c) {
		return length != 0 && data && data[0] == c && data[1] == '\0';
	}

	bool operator==(chars c) {
		return length != 0 && data && eq(data, c, shared_reference ? length : -1);
	}

	bool operator==(char *c) {
		return length != 0 && data && eq(data, c, shared_reference ? length : -1);
	}


	bool operator==(String *c) const {
		if (!c)return this->empty();
		if (this->empty())return not c or c->empty();
		return eq(data, c->data, shared_reference ? length : -1);
	}

	String &operator||(String &s) {// const
		if (this->empty())return s;
		return *this;
	}

	bool operator!=(String &s) {// const
		if (this->empty())return !s.empty();
		if (s.empty())return !this->empty();
		return !eq(data, s.data, shared_reference ? length : -1);
	}

//	bool operator==(const String other ) {
//		return length == other.length && eq(data, other.data, shared_reference? length:-1);
//	}  ambiguous with

	bool operator==(String &s) {// const
		if (this->empty())return s.empty();
		if (s.empty())return this->empty();
		return eq(data, s.data, shared_reference ? length : -1);
	}

	bool operator==(String *s) {// const
		if (this->empty() and not s)return true;
		if (this->empty())return s->empty();
		if (s->empty())return this->empty();
		return eq(data, s->data, shared_reference ? length : -1);
	}

	bool operator==(char *c) const {
		if (!this)return false;// how lol e.g. me.children[0].name => nil.name
		return eq(data, c, shared_reference ? length : -1);
	}

	bool operator!=(char *c) {
		return eq(data, c);
	}


#define min(a, b) (a < b ? a : b)

	bool operator>(String other) {
		for (int i = 0; i < min(length, other.length); ++i) {
			if (data[i] < other.data[i])return false;
		}
		return length >= other.length;
	}

	bool operator>=(String other) {
		for (int i = 0; i < min(length, other.length); ++i) {
			if (data[i] < other.data[i])return false;
		}
		return length >= other.length;
	}

	bool operator<=(String other) {
		for (int i = 0; i < min(length, other.length); ++i) {
			if (data[i] > other.data[i])return false;
		}
		return length <= other.length;
	}

	bool operator<(String other) {
		for (int i = 0; i < min(length, other.length); ++i) {
			if (data[i] > other.data[i])return false;
		}
		return length <= other.length;
	}
	/*
	char16_t character = u'牛';
	char32_t hanzi = U'牛';
	wchar_t word = L'牛';
	exposed usage? */
//	codepoint operator[](int i) {
//		return codepointAt(i);
//	}

	// internal usage
	char operator[](int i) {
		if (!data or i < 0 or i > length)
			return 0;// -1? todo: throw?
		return data[i];
	}
	// expensive
//	String operator[](int i);
//	grapheme operator[](int i);

	bool empty() const;

	int indexOf(chars string) {
		int l = strlen0(string);
		if ((long) data + l > MEMORY_SIZE)
			error("corrupt string");
		for (int i = 0; i <= length - l; i++) {
			bool ok = true;
			for (int j = 0; j < l; j++) {
				if (data[i + j] != string[j]) {
					ok = false;
					break;
				}
			}
			if (ok)
				return i;
		}
		return -1;//
	}

	bool contains(chars string) {
		return indexOf(string) >= 0;
	}

	bool contains(char chr) {
		return indexOf(chr) >= 0;
	}

	[[nodiscard]]
	__attribute__((__warn_unused_result__))
	String &replace(chars string, chars with) {// first only!
		int i = this->indexOf(string);
		if (i >= 0) {
			unsigned int from = i + strlen0(string);
			return substring(0, i) + with + substring(from, -1);
		} else {
			return *this;
		}
	}

	[[nodiscard("replace generates a new string to be consumed!")]]
	__attribute__((__warn_unused_result__))
	String &replace(chars string, String with) {// first only!
		return replace(string, with.data);
	}

	[[nodiscard]]
	__attribute__((__warn_unused_result__))
	String &replaceAll(String part, String with) {
		String &done = *this;
		if (with.contains(part))todo("incremental replaceAll");
		while (done.contains(part))
			done = done.replace(part, with);
		return done;
	}

	String times(short i) {
		if (i < 0)
			return String();
		String concat = String();
		while (i-- > 0)
			concat += this;
		return concat;
	}

// type conversions

	explicit operator int() { return atoi0(data); }

//	 operator char*()  { return data; }
	explicit operator int() const { return atoi0(data); }

//	MUST BE explicit, otherwise String("abc") != "abc"  : char* comparison hence false
//	explicit cast
	operator char *() const { return this ? data : 0; }

//	operator codepoint *() { return extractCodepoints(); }


	[[nodiscard]] bool isNumber() const {
		return atoi0(data);
	}

	String format(int i) {
		return this->replace("%d", itoa0(i));
	}

	String format(long i) {
		return this->replace("%d", itoa0(i));
	}

	String format(double f) {
		return this->replace("%f", ftoa(f));
	}

	String format(char *string) {
		return this->replace("%s", string);
	}

//	int in(List<chars> liste);
	int in(chars array[]) {// array NEEDS to be 0 terminated!!!!
		int i = 0;
		while (array[i]) {
			if (eq(array[i], data))
				return i + 1;
			i++;
		}
		return 0;
	}

	// 0 = NO, 1 = yes at #1
	int in(String array[]) {// array NEEDS to be 0 terminated!!!!
		int i = 0;
//		for(String x:array){}
		String dis = *this;
		while (not array[i].empty()) {
			if (array[i] == dis) {
				return i + 1;
			}
			i++;
		}
		return 0;
	}

//	bool in(String* array) {
//		return false;
//	}
	codepoint *begin();

	codepoint *end();

	bool startsWith(chars string);

	bool endsWith(const char *string);

	String to(const char *string);

	List<String> split(const char *string);

	String trim();

	long hash();

	String &lower();
//	void lower();
};


class SyntaxError : String {
public:
	char *data;
	char *file;
	int lineNumber;
	int columnNumber;
	int at;
public:

	void operator delete(void *) {}

	~SyntaxError() = default;

	void *operator new(size_t size) {
		return static_cast<String *>(calloc(size, 1));// WOW THAT WORKS!!!
	}

	explicit SyntaxError(String &error) {
		this->data = error.data;
	}
};

#define breakpoint_helper printf("\n%s:%d breakpoint_helper\n",__FILE__,__LINE__);

String operator ""_(chars c, unsigned long);

String operator ""_s(chars c, unsigned long);

String operator ""s(chars c, unsigned long);


extern String UNEXPECT_END;// = "Unexpected end of input";
extern String UNEXPECT_CHAR;// = "Unexpected character ";
extern String empty_name;
extern String nil_name;// = "nil";
//extern String empty_name;// = "";
extern String object_name;// = "{…}";
extern String groups_name;// = "(…)";
extern String patterns_name;// = "[…]";
extern String EMPTY;// = String('\0');

//String operator "" s(chars c, unsigned long );// wasm function signature contains illegal type
//String operator "" _(chars c, unsigned long );
//String operator "" _s(chars c, unsigned long );
void print(String *s);

void print(String s);

int puts(const char *);// int return needed for stdio compatibilty !
//void puts(const char *);

//#endif
//unsigned  == unsigned int!
inline short utf8_byte_count(char c);


bool empty(String &s);

bool empty(String *s);

bool empty(chars s);

bool empty(codepoint s);// todo: rename whitespace (and braces??)

bool contains(chars str, chars match);


#ifdef WEBAPP

#include <sstream>

void render(Node &node, std::stringstream *html = 0);

#endif