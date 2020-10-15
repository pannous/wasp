//
// Created by pannous on 24.10.18.
//

//#ifndef NETBASE_STRING_CPP
//#define NETBASE_STRING_CPP
//#include "String.h" prefering one big WASM object

//extern double pow(double x, double y);
extern "C" double pow(double x, double y);
extern "C" double sqrt(double __a);
#include "String.h"
#include "WasmHelpers.h"
#include <stdlib.h> // pulls in declaration of malloc, free
#include <math.h> // pow
//#include <tgmath.h> // pow
//#include <cmath> // MISSING ON MAC WTF
//#include <string.h> // strcpy
//#include <cstring> // strcpy doesn't work!?
void* alloc(long size){
	return 	malloc(size);
}
#endif

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wstring-compare"
#define let auto
#define var auto

#include "NodeTypes.h"

typedef void *any;
typedef unsigned char byte;
typedef const char *chars;


extern "C" void logs(const char *);
bool debug = true;
//extern void *alloc (size_t __size);
//extern void *alloc (int __size);

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
#pragma message "using wasm imports"
void printf(const char *s){
	while(*s)logc(*s++);
}
#endif

class String;



//#include <stdio.h>
//#include <string.h>
#ifndef WASM
#include <cstring> //strlen
#else
//#include "string.h"
size_t  strlen(const char *x){
	int l=0;
	while(x++)l++;
	return l;
}
#endif
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
void strcpy2(char *dest, const char *src, int length) {// =-1
	if (!dest || !src)
		return;
	int i = 0;
	if(length<0)length = strlen(src);
//	if(strlen(src)<length)throw "Illegal strcpy2 length"; could be filled with 0 :(
//	if(strlen(dest)<length)throw "Illegal strcpy2 length"; could be filled with 0 :(
	while (char c = src[i]) {
		if (length-- == 0)break;
		dest[i] = c;
		i++;
	}
}
void strcpy2(char *dest, const char *src){
	strcpy2(dest, src, -1);
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
class Node; // can't pre-access properties, BUT can use functions:
String toString(Node &node);

void reverse(char *str, int len);

// Implementation of itoa0()


char *itoa0(long num, int base ) {
	char *str = (char *) alloc(100);// todo: from context.names char*
	int len = 0;
	bool isNegative = false;
	/* Handle 0 explicitely, otherwise empty string is printed for 0 */
	if (num == 0) {
		str[len++] = '0';
		str[len] = '\0';
		return str;
	}
	// In standard itoa0(), negative numbers are handled only with
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
char *itoa0(long num){
	return itoa0(num, 10);
}
const char *concat(const char *a, const char *b) {
//const char* concat(char* a,char* b){// free manually!
	if (!b)return a;
	int la = (int) strlen(a);
	int lb = (int) strlen(b);
//	char c[la+lb];
	char *c = (char *) alloc((la + lb + 1) * sizeof(char) );
	strcpy2(c, a,-1);
	strcpy2(&c[la], b,-1);
	c[la + lb] = 0;
	return c;
}

const char *ftoa(float num, int base=10, int precision=4) {/*significant digits*/
	return concat(concat(itoa0(int(num),base),"."),itoa0(int(num*pow(base,precision)),base));
}

void reverse(char *str, int len) {
	for (int i = 0; i < len / 2; i++) {
		char t = str[len - 1 - i];
		str[len - 1 - i] = str[i];
		str[i] = t;
	}
}

//char itoa0(byte i) {
//	return (char) (i + 0x30);
//}

//String String::operator++() {
//	this->data++;// self modifying ok?
//	return *this;
//}


//class NodeMap{
//
//};

class Node;

//String object_name = "<object>";

//#import "Node.cpp"


void log(long i) {
#ifdef WASM
	logi((int) i);
#else
	printf("%li", i);
#endif
}



//void log(Node &node) {
//	Node node2 = node;
//	log(node2);
//}


#ifndef WASM

//#import <string>
#include  <string>

void log(std::string s) {
	printf("%s\n", s.data());
}

#endif

void log(String s) {
	printf("%s\n", s.data);
}

//#endif //NETBASE_STRING_CPP

#pragma clang diagnostic ignored "-Wuser-defined-literals"
//#pragma clang diagnostic ignored "-Wliteral-suffix"

//String operator "" s(const char *c, size_t) {
//	return String(c);// "bla"s  literal operator suffixes not preceded by ‘_’ are reserved for future standardization
//}

String operator "" _s(const char *c, size_t) {
	return String(c);
}

String operator "" _(const char *c, unsigned long t) {
	return String(c);
}


bool eq(String &dest, const char *src) {
	return dest.data == src;
}

void err(String error){
	err(error.data);
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


String UNEXPECT_END = "Unexpected end of input";
String UNEXPECT_CHAR = "Unexpected character ";


String nil_name = "nil";
String empty_name = "";
String object_name = "{…}";
String groups_name = "(…)";
String patterns_name = "[…]";
String EMPTY = String('\0');

unsigned int *memory = (unsigned int *) 4096; // todo how to not handtune _data_end?

#pragma clang diagnostic pop

