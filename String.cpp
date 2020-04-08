//
// Created by pannous on 24.10.18.
//

//#ifndef NETBASE_STRING_CPP
//#define NETBASE_STRING_CPP
//#include "String.h" prefering one big WASM object

//void *malloc(int size);

//#include <malloc.h>
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wstring-compare"
#define let auto
#define var auto

#include "NodeTypes.h"

typedef void *any;
typedef unsigned char byte;
typedef const char *chars;
//typedef int size_t;

extern unsigned int *memory;
extern unsigned int *&heap;

extern "C" void logs(const char *);
bool debug = true;
//extern void *malloc (size_t __size);
//extern void *malloc (int __size);

#ifndef WASM

#include <cstdio>
#include <tgmath.h>
//#include <cstring>

void err(chars error);

void todo(chars error);

void logs(const char *s) {
	printf("%s\n", s);
}

void logi(long i) {
	printf("%li\n", i);
}

#else

extern "C" void print (const char *);// no \n newline
extern "C" void logs (const char *);
extern "C" void logc(char s);
extern "C" void logi(int i);
void printf(const char *s);
void printf(const char *format, int i);
void printf(const char *format, chars i);
//  extern "C" int printf (const char *__restrict __format, ...);
#endif

class String;

String typeName(Type t);

String str(const char *&s);


int atoi0(const char *__nptr);


//#include <stdio.h>
//#include <string.h>
#include <cstring> //strlen

//#define cstring
bool eq(const char *dest, const char *src) {
	if (!dest || !src)
		return false;
	int i = 0;
	if(dest=="" and src[0])return false ;
	if (strlen(dest) != strlen(src))return false;
	while (char c = dest[i]) {
		if (!src[i] || !c)
			return false;
		if (src[i] != c)
			return false;
		i++;
	}
	return true;
}

// or cstring
//#ifndef cstring
void strcpy2(char *dest, const char *src, int length = -1) {
	if (!dest || !src)
		return;
	int i = 0;
	while (char c = src[i]) {
		if (length-- == 0)break;
		dest[i] = c;
		i++;
	}
}
//#endif

int atoi0(const char *p) {
	int k = 0;
	while (*p) {
		int n = (*p) - '0';
		if (n < 0 or n > 9)
			return k;
		k = (k << 3) + (k << 1) + n;
		p++;
	}
	return k;
}

double atof0(const char *string) {
	double result = 0.0;
	if (!string) return result;

	double multiplier = 1;
	double divisor = 1.0;
	int integer_portion = 0;

	integer_portion = atoi0(string);

	result = (double) integer_portion;
	if (*string == '-') {
		result *= -1;
		multiplier = -1;
	}

	while (*string && (*string != '.'))
		string++;
	if (*string)
		string++;
	while (*string) {
		if (*string < '0' || *string > '9') return result;
		divisor *= 10.0;
		result += (double) (*string - '0') / divisor;
		string++;
	}
	return result * multiplier;
}

class String;

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

void reverse(char *str, int len);

// Implementation of itoa()

char *itoa(long num, int base = 10) {
	char *str = (char *) malloc(100);// todo: from context.names char*
	int len = 0;
	bool isNegative = false;
	/* Handle 0 explicitely, otherwise empty string is printed for 0 */
	if (num == 0) {
		str[len++] = '0';
		str[len] = '\0';
		return str;
	}
	// In standard itoa(), negative numbers are handled only with
	// base 10. Otherwise numbers are considered unsigned.
	if (num < 0 && base == 10) {
		isNegative = true;
		num = -num;
	}
	// Process individual digits
	while (num != 0) {
		int rem = num % base;
		str[len++] = (rem > 9) ? (rem - 10) + 'a' : rem + '0';
		num = num / base;
	}
	// If number is negative, append '-'
	if (isNegative)
		str[len++] = '-';
	str[len] = '\0'; // Append string terminator
	// Reverse the string
	reverse(str, len);
	return str;
}

void reverse(char *str, int len) {
	for (int i = 0; i < len / 2; i++) {
		char t = str[len - 1 - i];
		str[len - 1 - i] = str[i];
		str[i] = t;
	}
}

//char itoa(byte i) {
//	return (char) (i + 0x30);
//}
#ifndef __APPLE__
#include <malloc.h>
#endif
class String {

public:
	char *data{};
	int length = -1;

	String() {
		data = static_cast<char *>(calloc(sizeof(char), 2));
		length = 0;
	}

	String(char c) {
		data = static_cast<char *>(calloc(sizeof(char), 2));
		data[0] = c;
		data[1] = 0;
		length = 1;
	}

	String(const char string[]) {
		data = const_cast<char *>(string);
		length = len(data);
	}


//		String operator+(Type e){
	String(Type e) : String(typeName(e)) {}

	explicit String(int c) {
		data = itoa(c);
		length = len(data);
	}

	explicit String(long c) {
		data = itoa(c);
		length = len(data);
	}

#ifndef WASM

	String(std::string basicString) {
		data = const_cast<char *>(basicString.data());// copy?
	}

#endif

	char charAt(int i) {
		if (i >= length)
			err((String("IndexOutOfBounds at ") + itoa(i) + " in " + data).data);
		return data[i];
	}

	char charCodeAt(int i) {
		if (i >= length)
			throw IndexOutOfBounds(data, i);
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
		if (to <= from)return "";
		if (to < 0 or to > length)to = length;
		int len = (to - from) + 1;
		auto *neu = static_cast<char *>(malloc((sizeof(char)) * len));
//#ifdef cstring
//		strcpy(neu, &data[from]);
//#else
		strcpy2(neu, &data[from], to - from);
//#endif
		neu[to - from] = 0;
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
		if (!data)data = static_cast<char *>(malloc(sizeof(char) * 2));
		if (data + length + 1 == (char *) memory) {// just append recent
			data[length++] = c;
			data[length] = 0;
			memory += 2;
		} else {
			auto *neu = static_cast<char *>(malloc(sizeof(char) * length + 5));
			if (data)strcpy(neu, data);
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
		b.replace("%s", c);
		return b;
	}

	String operator%(chars &c) {
		String b = this->clone();
		b.replace("%s", c);
		return b;
	}


	String operator%(char *c) {
		return this->replace("%s", c);
	}

	String operator%(long d) {
		return this->replace("%d", itoa(d));
	}

	String operator%(double f) {
		String formated = String() + itoa(f) + "." + itoa((f - int(f)) * 10000);
		return this->replace("%f", formated);
	}

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
		while (c++)append(c[0]);
		return this;
	}

	String *operator+=(char c) {
		append(c);
		return this;
	}

	String operator+(String c) {
		if (c.length <= 0)
			return *this;
		auto *neu = static_cast<char *>(malloc(length + c.length + 1));
#ifdef cstring
		if (data)strcpy(neu, data);
		if (c.data)strcpy(neu + length, c.data);
#else
		if (data)strcpy2(neu, data, length);
		if (c.data)strcpy2(neu + length, c.data, c.length);
#endif
		neu[length + c.length] = 0;
		return String(neu);
	}

	String operator+(const char x[]) {
		return this->operator+(String(x));
	}
//	String operator+(float i) {
//		return this->operator+(String(i));
//	}

	String operator+(bool b) {
		return this->operator+(b ? " true" : " false");
	}

	String operator+(long i) {
		return this->operator+(String(i));
	}

	String operator+(int i) {
		return this->operator+(String(i));
	}

	String operator+(char c) {
		return this->operator+(String(c));
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
		return !length==0  && data  && data[0] == c && data[1] == '\0';
	}

	bool operator==(String &s) {// const
		if (this->empty())return s.empty();
		if (s.empty())return this->empty();
		return eq(data, s.data);
	}

	bool operator==(char *c) const {
		if (!this)
			return false;// how lol e.g. me.children[0].name => nil.name
		return eq(data, c);
	}

	bool operator!=(char *c) {
		return eq(data, c);
	}

	char operator[](int i) {
		return data[i];
	}

	bool empty() {
		return this==0 || length==0  ||  !data || data=="" || data=="ø" || data=="[…]"  || data=="(…)"  || data=="{…}" ||  data[0] == 0;
	}

	int indexOf(chars string) {
		int l = len(string);
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

	String replace(chars string, chars with) {
		int i = this->indexOf(string);
		if (i >= 0)
			return String(substring(0, i) + with + substring(i + len(string), length));
		else
			return *this;
	}

	String times(short i) {
		if (i < 0)
			return "";
		String concat = "";
		while (i-- > 0)
			concat += this;
		return concat;
	}

// type conversions

	explicit operator int() { return atoi(data); }

//	 operator char*()  { return data; }
	explicit operator int() const { return atoi(data); }

	operator char *() const { return data; }

	bool isNumber() {
		return atoi0(data);
	}
};
//String String::operator++() {
//	this->data++;// self modifying ok?
//	return *this;
//}


//class NodeMap{
//
//};

class Node;

String nil_name = "nil";
String empty_name = "";
String object_name = "{…}";
String groups_name = "(…)";
String patterns_name = "[…]";
//String object_name = "<object>";

#import "Node.cpp"

void log(long i) {
	printf("%li", i);
}

void log(chars s) {
	printf("%s\n", s);
}

void log(Node &n) {
	n.log();
}


//void log(Node &node) {
//	Node node2 = node;
//	log(node2);
//}


#ifndef WASM

#import <string>

void log(std::string s) {
	printf("%s\n", s.data());
}

#endif

void log(String s) {
	printf("%s\n", s.data);
}

void log(Node *n0) {
	if (!n0)
		return;
	Node n = *n0;
	log(n);
}

#ifdef WASM
void printf(const char *s) {
	print(s);
}
void printf(const char *format, int i) {
	print(String(format).replace("%d", String(i)).replace("%i", String(i)).replace("%li", String(i)));
}
void printf(const char *format, long i) {
	print(String(format).replace("%d", String(i)).replace("%i", String(i)).replace("%li", String(i)));
}
void printf(const char *format, chars value) {
	print(String(format).replace("%s", value));
}

void printf(const char *format, void* value) {
	print(String(format).replace("%p", String((long)value)));
}
#endif

//#endif //NETBASE_STRING_CPP

#pragma clang diagnostic ignored "-Wuser-defined-literals"
#pragma clang diagnostic ignored "-Wliteral-suffix"

String operator "" s(const char *c, size_t) {
	return String(c);// "bla"s  literal operator suffixes not preceded by ‘_’ are reserved for future standardization
}

String operator "" _s(const char *c, size_t) {
	return String(c);
}

String operator "" _(const char *c, unsigned long t) {
	return String(c);
}


String string(const char *&s) {
	return String(s);
}

String str(const char *&s) {
	return String(s);
}

String str(char *s) {
	return String(s);
}

String s(const char *&s) {
	return String(s);
}


String typeName(Type t) {
	switch (t) {
		case objects:
			return "object";
		case groups:
			return "group";
		case patterns:
			return "pattern";
		case keyNode:
			return "node";
		case reference:
			return "reference";
		case symbol:
			return "symbol";
		case operators:
			return "operator";
		case expression:
			return "expression";
		case strings:
			return "string";
		case arrays:
			return "array";
		case buffers:
			return "buffer";
		case floats:
			return "float";
		case longs:
			return "long";
		case ints:
			return "int";
		case bools:
			return "bool";
		case nils:
			return "nil";
		case unknown :
			return "unknown";
		default:
			breakpoint_helper
			throw str("MISSING Type name mapping ") + t;
	}
}


void todo(chars error) {
	breakpoint_helper
	err(str("TODO ") + error);
}

class SyntaxError : String {
public:
	char *data;
public:
	SyntaxError(String &error) {
		this->data = error.data;
	}
};

#pragma clang diagnostic pop