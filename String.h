#pragma once
//
// Created by pannous on 19.12.19.
//
//#include <c++/v1/cstdlib>
#include "WasmHelpers.h"
#include "NodeTypes.h"
#ifndef WASM
#include <cstdlib>
#include <cstdio> // printf
#endif

void reverse(char *str, int len);
char *itoa0(long num, int base);
char *itoa0(long num);
char *itoa(long num);
int atoi0(const char *__nptr);
double atof0(const char *string);

void err(chars error);

void todo(chars error);


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

#pragma once // needs to be on top
//#include "Node.h"

//#ifndef WASP_STRING
//#define WASP_STRING
class Node;
typedef const char *chars;

#ifndef WASM
//#include <string>
//#include <stdlib.h> // pulls in declaration of malloc, free
#else
#endif
//void* alloc(number size);// wasm | linux
extern unsigned int *memory;

extern void err(chars error);
extern void error(chars error);
extern void warn(chars error);
extern void warning(chars error);

extern chars fetch(chars url);

class String;
String toString(Node &node);
String str(const char *&s);
String str(char *string);
String s(const char *&s);

bool eq(const char *dest, const char *src);
void strcpy2(char *dest, const char *src);
void strcpy2(char *dest, const char *src, int length);
int strlen0(const char *x);
//size_t   strlen(const char *__s);


//const char *ftoa(float num, int base = 10, int precision = 4);
const char *ftoa0(float num, int base, int precision);
const char *ftoa(float num);

class Error {
public:
	char *message;
//	Error(String *string) {
//		message = string->data;
//	}
};

class IndexOutOfBounds : Error {
public:
//	IndexOutOfBounds(String *string1) : Error(string1){}
	IndexOutOfBounds(char *data, int i) : Error() {}
};

//char *empty_string = "";
String typeName(Type t);
//char null_value[]={0};// todo make sure it's immutable!!
class String{

#ifdef WASM
//#define size_t unsigned number
//	void* calloc(size_t s,int idk);
#else
#ifndef __APPLE__
//#include <alloc.h>
#endif
#endif

public:
	char *data{};
	int length = -1;

	String() {
//		assert(null_value[0] == 0);
		data = "";
//		data = empty_string;
//		data =  {0};//null_value;
//		data = static_cast<char *>(calloc(1, 1));
		length = 0;
	}

	void* operator new(unsigned long size){
		return static_cast<String *>(calloc(sizeof(String),size));// WOW THAT WORKS!!!
	}
	void operator delete (void*){}// Todo ;)

//	~String()=default;

	explicit String(char c) {
		data = static_cast<char *>(alloc(sizeof(char),2));
		data[0] = c;
		data[1] = 0;
		length = 1;
	}

//	explicit
	String(const char string[]) {
		data = const_cast<char *>(string);// heap may disappear, use copy!
		length = strlen0(string);
//		data = static_cast<char *>(alloc(sizeof(char), length));
//		strcpy2(data, string, length+1);
	}


//		String operator+(Type e){
	explicit String(Type e) : String(typeName(e)) {}

	explicit String(int c) {
		data = itoa(c);// wasm function signature contains illegal type WHYYYY
		length = len(data);
	}

	explicit String(long c) {
		data = itoa0(c);
		length = len(data);
	}


	explicit String(double c) {
		int max_length=4;
		data = itoa0(c);
		length = len(data);
//		itof :
		append('.');
		c = c - (long(c));
		while(length<max_length){
			c= (c - long(c)) * 10;
			if(int(c)==0)break;
			append(int(c)+0x30);
		}
	}

#ifdef std
	String(std::string basicString) {
		data = const_cast<char *>(basicString.data());// copy?
	}
#endif

	char charAt(int i) {
		if (i >= length)
			err((String("IndexOutOfBounds at ") + itoa0(i) + " in " + data).data);
		return data[i];
	}

	char charCodeAt(int i) {
//		if (i >= length)
//			raise(IndexOutOfBounds(data, i).message);
//		String("IndexOutOfBounds at ") + i + " in " + data;
//			throw new IndexOutOfBounds(String(" at ") + i + " in " + data);
		return data[i];
	}

	int indexOf(char c, int i = 0) {
		for (int j = i; j < length; j++) {
			if (data[j] == c)return j;
		}
		return -1;
	}

//	operator std::string() const { return "Hi"; }

	String substring(int from, int to = -1) { // excluding to
		if (to < 0 or to > length)to = length;
		if (to <= from)return String();
		int len = (to - from) + 1;
		auto neu = static_cast<char *>(alloc((sizeof(char)) , len+1));
//#ifdef cstring
//		strcpy(neu, &data[from]);
//#else
		strcpy2(neu, &data[from], to - from);
//#endif
		neu[to - from] = 0;
		neu[len]=0;
		return String(neu);
//		free(neu);
	}

	int len(const char *data) {
		if (!data)data = this->data;
		if (!data || data[0] == 0)
			return 0;
		int MAX = 100000;
		for (int i = 0; i < MAX; ++i) {
			if (!data[i])return i;
		}
		return -1;//error
	}

	String &append(char c) {
		if (!data)data = static_cast<char *>(alloc(sizeof(char), 2));
		if (data + length + 1 == (char *) current) {// just append recent
			data[length++] = c;
			data[length] = 0;
			current += 2;
		} else {
			auto *neu = static_cast<char *>(alloc(sizeof(char) , length + 5));
			if (data)strcpy2(neu, data, length);
			neu[length++] = c;
			data = neu;
			data[length] = 0;
		}
		return *this;
	}

	String clone() {
		return String(this->data);
	}


	String operator%(String &c) {
		if (!contains("%s"))
			return *this + c;
		String b = this->clone();
		String d=b.replace("%s", c);
		return d;
	}


	String operator%(Node &c) {
		if (!contains("%s"))
			return *this + toString(c);
		String b = this->clone();
		String d=b.replace("%s", toString(c));
		return d;
	}

	String operator%(chars &c) {
		String b = this->clone();
		b.replace("%s", c);
		return b;
	}

	String operator%(const char *c) {
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
//		todo %l ??
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
		while (c[0] && c++)append(c[-1]);
		return this;
	}

	String *operator+=(chars c) {
		while (c[0] && c++)append(c[-1]);
		return this;
	}

	String *operator+=(char c) {
		append(c);
		return this;
	}

	String operator+(String c) {
		if (c.length <= 0)
			return *this;
//		log("1.");
//		log(this);
//		log(" + 2.");
//		log(c);
//		log("\n");
		auto *neu = static_cast<char *>(alloc(sizeof(char), length + c.length + 1));
#ifdef cstring
		if (data)strcpy(neu, data);
		if (c.data)strcpy(neu + length, c.data);
#else
		if (data)strcpy2(neu, data, length);
		if (c.data)strcpy2(neu + length, c.data, c.length);
#endif
		neu[length + c.length] = 0;
//		log(neu);
//		String* ok=new String(neu);
//		log("3.");
//		log(ok);
//		ok.length = length + c.length;
//		log("-----------------");
		return String(neu);
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

	String operator+(String* s) {
		if(!s or !s->data)return *this;
		return this->operator+(s->data);
	}

	String operator++() {
		this->data++;// self modifying ok?
		length--;
		return *this;
	}

	String operator++(int postfix) {//
		this->data += 1 + postfix;// self modifying ok?
		length -= 1 + postfix;
		return *this;
	}

	String operator+(char *c) {
		return this->operator+(String(c));
	}

	bool operator==(char c) {
		return length!=0  && data  && data[0] == c && data[1] == '\0';
	}

	bool operator==(chars c) {
		return length != 0 && data && eq(data, c);
	}

	bool operator==(char* c) {
		return length != 0 && data && eq(data, c);
	}



	bool operator!=(String &s) {// const
		if (this->empty())return !s.empty();
		if (s.empty())return !this->empty();
		return !eq(data, s.data);
	}

//	bool operator==(const String other ) {
//		return length == other.length && eq(data, other.data);
//	}  ambiguous with

	bool operator==(String &s) {// const
		if (this->empty())return s.empty();
		if (s.empty())return this->empty();
		return eq(data, s.data);
	}

	bool operator==(String *s) {// const
		if (this->empty() and not s)return true;
		if (this->empty())return s->empty();
		if (s->empty())return this->empty();
		return eq(data, s->data);
	}

	bool operator==(char *c) const {
//		if (!this)return false;// how lol e.g. me.children[0].name => nil.name
		return eq(data, c);
	}

	bool operator!=(char *c) {
		return eq(data, c);
	}

	char operator[](int i) {
		return data[i];
	}
	bool empty() const;

	int indexOf(chars string) {
		int l = strlen0(string);
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

	String replace(chars string, chars with) {// first only!
		int i = this->indexOf(string);
		if (i >= 0) {
			unsigned int from = i + strlen0(string);
			return substring(0, i) + with + substring(from, -1);
		}
		else {
			return *this;
		}
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

//	otherwise String("abc") == "abc"  is char* comparison hence false
//	explicit
	operator char *() const { return data; }

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
		return this->replace("%f" , ftoa(f));
	}
	String format(char *string) {
		return this->replace("%s", string);
	}

// return position(==index+1), NOT index!
	int in(String array[]) {// array NEEDS to be 0 terminated!!!!
		int i = 0;
//		for(String x:array){}
		String dis = *this;
		while (array[i]){
			if(array[i] == dis) {
				return i+1;
			}
			i++;
		}
		return 0;
	}

//	bool in(String* array) {
//		return false;
//	}
};


class SyntaxError : String {
public:
	char *data;
public:
	void* operator new(unsigned long size){
		return static_cast<Node *>(calloc(sizeof(SyntaxError),size));// WOW THAT WORKS!!!
	}
	void operator delete (void*){}
	~SyntaxError()= default;
	explicit SyntaxError(String &error) {
		this->data = error.data;
	}
};

#define breakpoint_helper printf("\n%s:%d breakpoint_helper\n",__FILE__,__LINE__);

String operator ""_(const char* c, unsigned long );
String operator ""_s(const char* c, unsigned long );
String operator ""s(const char* c, unsigned long );



extern String UNEXPECT_END;// = "Unexpected end of input";
extern String UNEXPECT_CHAR;// = "Unexpected character ";
extern String empty_name;
extern String nil_name;// = "nil";
//extern String empty_name;// = "";
extern String object_name;// = "{…}";
extern String groups_name;// = "(…)";
extern String patterns_name;// = "[…]";
extern String EMPTY;// = String('\0');

//String operator "" s(const char *c, unsigned long );// wasm function signature contains illegal type
//String operator "" _(const char *c, unsigned long );
//String operator "" _s(const char *c, unsigned long );
void log(String *s);
void log(chars s);
//#endif