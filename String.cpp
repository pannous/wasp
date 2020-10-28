//#pragma once
//
// Created by pannous on 24.10.18.
//

#include "String.h"
#include "NodeTypes.h"
#include "WasmHelpers.h"

#ifndef WASM
//#include <string.h> // strcpy
//#include <cstring> // strcpy doesn't work!?
#include <stdlib.h> // pulls in declaration of malloc, free
//#include <math.h> // pow
//#include "math.h"
//#include <cmath>
#endif

//extern double pow(double x, double y);
//extern "C" double pow(double x, double y);
//extern "C" double sqrt(double __a);


#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wstring-compare"
#define let auto
#define var auto


typedef void *any;
typedef unsigned char byte;
typedef const char *chars;


//extern "C" void logs(const char *,int len);
//bool debug = true;
//extern void *alloc (size_t __size);
//extern void *alloc (int __size);

#ifndef WASM
#include <cstdio>
#include <tgmath.h>

//#include <cstring>

void logs(const char *s) {
	printf("%s\n", s);
}

void logi(long i) {
	printf("%li\n", i);
}

#else
#pragma message "using wasm imports"
//void printf(const char *s){
//	while(*s)logc(*s++);
//}
#endif

class String;



//#include <stdio.h>
//#include <string.h>
#ifndef WASM
#include <cstring> //strlen
#else
//#include "string.h"
//size_t strlen(const char *x){
//	int l=0;
//	while(x[0]&&l<100){x++;l++;}
//	return l;
//}
#endif
//#define cstring
bool eq(const char *dest, const char *src) {
	if (!dest || !src)
		return false;
	int i = 0;
	if(dest=="" and src[0])return false ;
	if (strlen0(dest) != strlen0(src))return false;
	while (char c = dest[i]) {
		if (!src[i] || !c)
			return false;
		if (src[i] != c)
			return false;
		i++;
	}
	return true;
}

int strlen0(const char *x){
	int l=0;
	while(*x++)l++;
	return l;
}

// or cstring
//#ifndef cstring
void strcpy2(char *dest, const char *src, int length) {// =-1
	if (!dest || !src)
		return;
	int i = 0;
	if(length<0)length = strlen0(src);
//	if(strlen(src)<length)error(string("Illegal strcpy2 length"));// could be filled with 0 :(
//	if(strlen(dest)<length)error("Illegal strcpy2 length"_s);// could be filled with 0 :(
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


// Implementation of itoa0()


char *itoa0(long num, int base =10 ) {
	// length 22 -> log(num)/2+2 for base 10
	char *str = (char *) alloc(sizeof(char),22);// -18446744073709552000  todo: from context.names char*
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
char *itoa(long num){
	return itoa0(num, 10);
}
const char *concat(const char *a, const char *b) {
//const char* concat(char* a,char* b){// free manually!
	if (!b)return a;
	int la = (int) strlen0(a);
	int lb = (int) strlen0(b);
//	char c[la+lb];
	char *c = (char *) alloc(sizeof(char), la + lb + 1);
	strcpy2(c, a,-1);
	strcpy2(&c[la], b,-1);
	c[la + lb] = 0;
	return c;
}


//#define pow(val,exp)
const char *ftoa0(float num, int base=10, int precision=4) {/*significant digits*/
	int p=1000;//pow(base,precision);
	return concat(concat(itoa0(int(num),base),"."),itoa0(int((num-(long)num)*p),base));
}
const char* ftoa(float num){ return ftoa0(num, 10, 4); }

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


#ifndef WASM
#undef log // expanded from macro 'log' tgmath.h:245:25:
void log(long i) {
	printf("%li", i);
}
#endif



//void log(Node &node) {
//	Node node2 = node;
//	log(node2);
//}


//#ifndef WASM
//#include  <string>
//void log(std::string s) {
//	printf("%s\n", s.data());
//}
//#endif

void log(String s) {
#ifdef WASM
	log((chars)s.data);
#else
	printf("%s\n", s.data);
#endif
}

//#endif //NETBASE_STRING_CPP

#pragma clang diagnostic ignored "-Wuser-defined-literals"
//#pragma clang diagnostic ignored "-Wliteral-suffix"

//String operator "" s(const char *c, size_t) {
//	return String(c);// "bla"s  literal operator suffixes not preceded by ‘_’ are reserved for future standardization
//}

//String operator "" _s(const char *c, int x) {// invalid literal operator parameter type 'int', did you mean 'unsigned number'
//	return String(c);
//}
String operator "" _s(const char *c, unsigned long t) {// function signature contains illegal type WHYY??
	return String(c);
}
String operator "" s(const char *c, unsigned long t) {// function signature contains illegal type WHYY??
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
			return "number";
//		case ints:
//			return "int";
		case bools:
			return "bool";
		case nils:
			return "nil";
		case unknown :
			return "unknown";
		default:
			breakpoint_helper
			error(str("MISSING Type name mapping ") + t);
			return "ERROR";
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

#pragma clang diagnostic pop


void log(String *s) {
#ifdef WASM
	if(s)log(s->data);
#else
	printf(s->data);
#endif
}

#ifndef WASM
void log(chars s){
printf("%s\n",s);
}
#endif
