//#pragma once
//
// Created by pannous on 24.10.18.
//

#include "String.h"
#include "NodeTypes.h"
#include "wasm_helpers.h"
#include "Map.h"

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
//#define let auto
//#define var auto


//typedef void *any;
typedef unsigned char byte;
typedef chars chars;


//extern "C" void logs(chars ,int len);
//bool debug = true;
//extern void *alloc (size_t __size);
//extern void *alloc (int __size);
char *empty_string = "";
#ifndef WASM

#include <cstdio>

//#include <cstring>


#else
#pragma message "using wasm imports"
//void printf(chars s){
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
//size_t strlen(chars x){
//	int l=0;
//	while(x[0]&&l<100){x++;l++;}
//	return l;
//}
#endif
//#define cstring

bool eq(chars dest, chars src, int length) {
	if (!dest || !src)
		return false;
	int i = 0;
	if (dest == "" and src[0])return false;
	if (length<0 and strlen0(dest) != strlen0(src))
		return false;
//	if length>0 it forces comparison of reference strings terminated by length, not by 0!
	while (char c = dest[i]) {
		if(length>=0 and i>=length)
			break;
		char d = src[i];
		if (!d || !c)
			return false;
		if (d != c)
			return false;
		i++;
	}
	return true;
}

int strlen0(chars x) {
	if (!x)return 0;
	int l = 0;
	if ((long) x > MEMORY_SIZE) {
		logs(x);
		logi((int) (long) x);// 0x1000000 16777216
		error("corrupt string");
	}
	while (l < MAX_STRING_LENGTH and *x++)l++;
	return l;
}

// or cstring
//#ifndef cstring
void strcpy2(char *dest, chars src, int length) {// =-1
	if (!dest || !src)
		return;
	int i = 0;
	if (length < 0)length = strlen0(src);
	if (length <= 0)return ;
//	if(strlen(src)<length)error(string("Illegal strcpy2 length"));// could be filled with 0 :(
//	if(strlen(dest)<length)error("Illegal strcpy2 length"_s);// could be filled with 0 :(
	while (char c = src[i]) {
		if (length-- == 0)break;
		dest[i] = c;
		i++;
	}
}

void strcpy2(char *dest, chars src) {
	strcpy2(dest, src, -1);
}


//#endif

int atoi0(chars p) {
	if (!p)return 0;
	while(*p=='+')p++;
	short sig=1;
	if(*p=='-'){ sig=-1;p++; }
	int k = 0;
	while (*p) {
		int n = (*p) - '0';
		if (n < 0 or n > 9)
			return k;
		k = (k << 3) + (k << 1) + n;
		p++;
	}
	return sig * k;
}

double atof0(chars string) {
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


char *itoa0(long num, int base = 10) {
	// length 22 -> log(num)/2+2 for base 10
	char *str = (char *) alloc(sizeof(char), 22+1);// -18446744073709552000  todo: from context.names char*
//	int addr=(int)(long)str;
//	if(addr<0 or addr>memory_size)
//		error("OUT OF MEMORY");
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
	while (num != 0 and len<22) {
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

char *itoa0(long num) {
	return itoa0(num, 10);
}

char *itoa(long num) {
	return itoa0(num, 10);
}

chars concat(chars a, chars b) {
//chars concat(char* a,char* b){// free manually!
	if (!b)return a;
	int la = (int) strlen0(a);
	int lb = (int) strlen0(b);
//	char c[la+lb];
	char *c = (char *) alloc(sizeof(char), la + lb + 1);
	strcpy2(c, a, -1);
	strcpy2(&c[la], b, -1);
	c[la + lb] = 0;
	return c;
}


//#define pow(val,exp)
chars ftoa0(float num, int base = 10, int precision = 4) {/*significant digits*/
	int p = 1000;//pow(base,precision);
	char* f=(char*)concat(concat(itoa0(int(num), base), "."), itoa0(int((num - (long) num) * p), base));
	int len=strlen0(f);// cut trailing 0 : 1.1000 -> 1.1
	for (int i = 1; i < len; ++i) {
		if(f[len-i]=='0')f[len-i]=0;
		else break;
	}
	return f;
}

chars ftoa(float num) { return ftoa0(num, 10, 4); }

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


//#endif //NETBASE_STRING_CPP

#pragma clang diagnostic ignored "-Wuser-defined-literals"
//#pragma clang diagnostic ignored "-Wliteral-suffix"

//String operator "" s(chars c, size_t) {
//	return String(c);// "bla"s  literal operator suffixes not preceded by ‘_’ are reserved for future standardization
//}

//String operator "" _s(chars c, int x) {// invalid literal operator parameter type 'int', did you mean 'unsigned number'
//	return String(c);
//}
String operator "" _s(chars c, unsigned long t) {// function signature contains illegal type WHYY??
	return String(c);
}

String operator "" s(chars c, unsigned long t) {// function signature contains illegal type WHYY??
	return String(c);
}

String operator "" _(chars c, unsigned long t) {
	return String(c);
}


bool eq(String &dest, chars src) {
	return dest.data == src;
}

String string(chars &s) {
	return String(s);
}

String str(chars &s) {
	return String(s);
}

String str(char *s) {
	return String(s);
}

String s(chars &s) {
	return String(s);
}


//String
chars typeName(Type t) {
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
		case expressions:
			return "expression";
		case strings:
			return "string";
		case arrays:
			return "array";
		case buffers:
			return "buffer";
		case reals:
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
		case call:
			return "function";
		case declaration:
			return "declaration";
		case errors:
			return "error";
		default:
			error(str("MISSING Type name mapping ") + t);
			return "ERROR";
	}
}

String nil_name = "nil";
String empty_name = "";
String object_name = "{…}";
String groups_name = "(…)";
String patterns_name = "[…]";
String EMPTY = String('\0');

#pragma clang diagnostic pop

bool String::empty() const {//this==0 in testMarkMulti!
//#ifdef WASM
//	if(memory_size and data and (long) data > memory_size/*bug!*/)
//		error("CORRUPT String pointer");
////		return true;
//#endif
	if(this == 0)return true;
	return length == 0 || !data || data[0] == 0 || data == object_name.data;
//		|| data=="" || data=="ø" || data=="[…]"  || data=="(…)"  || data=="{…}"  TODO
}


codepoint *String::begin() {
	if (!codepoints)extractCodepoints();
	return &codepoints[0];
}

codepoint *String::end() {
	if (!codepoints)extractCodepoints();
	return &codepoints[codepoint_count];
}


size_t utf8_strlen(char *utf8bytes) {
	size_t len = 0;
	for (chars p = utf8bytes; *p; ++p)
		if ((*p & 0xc0) != 0x80)
			++len;
	return len;
}

codepoint decode_unicode_character(char *text, int *len) {
	if ((text[0] & 0b10000000) == 0) {
//		if(len)*len=1; // 1 byte code point, ASCII
		return (text[0] & 0b01111111);
	} else if ((text[0] & 0b11100000) == 0b11000000) {
//		if(len)*len=2; // 2 byte code point
		return (text[0] & 0b00011111) << 6 | (text[0 + 1] & 0b00111111);
	} else if ((text[0] & 0b11110000) == 0b11100000) {
//		if(len)*len=3; // 3 byte code point
		return (text[0] & 0b00001111) << 12 | (text[0 + 1] & 0b00111111) << 6 | (text[0 + 2] & 0b00111111);
	} else {
//		if(len)*len=4; // 4 byte code point
		return (text[0] & 0b00000111) << 18 | (text[0 + 1] & 0b00111111) << 12 | (text[0 + 2] & 0b00111111) << 6 | (text[0 + 3] & 0b00111111);
	}
}


void encode_unicode_character(char *buffer, wchar_t ucs_character) {
	int offset = 0;
	if (ucs_character <= 0x7F) {
		// Plain single-byte ASCII.
		buffer[offset++] = (char) ucs_character;
	} else if (ucs_character <= 0x7FF) {
		// Two bytes.
		buffer[offset++] = 0xC0 | (ucs_character >> 6);
		buffer[offset++] = 0x80 | ((ucs_character >> 0) & 0x3F);
	} else if (ucs_character <= 0xFFFF) {
		// Three bytes.
		buffer[offset++] = 0xE0 | (ucs_character >> 12);
		buffer[offset++] = 0x80 | ((ucs_character >> 6) & 0x3F);
		buffer[offset++] = 0x80 | ((ucs_character >> 0) & 0x3F);
	} else if (ucs_character <= 0x1FFFFF) {
		// Four bytes.
		buffer[offset++] = 0xF0 | (ucs_character >> 18);
		buffer[offset++] = 0x80 | ((ucs_character >> 12) & 0x3F);
		buffer[offset++] = 0x80 | ((ucs_character >> 6) & 0x3F);
		buffer[offset++] = 0x80 | ((ucs_character >> 0) & 0x3F);
	} else if (ucs_character <= 0x3FFFFFF) {
		// Five bytes.
		buffer[offset++] = 0xF8 | (ucs_character >> 24);
		buffer[offset++] = 0x80 | ((ucs_character >> 18) & 0x3F);
		buffer[offset++] = 0x80 | ((ucs_character >> 12) & 0x3F);
		buffer[offset++] = 0x80 | ((ucs_character >> 6) & 0x3F);
		buffer[offset++] = 0x80 | ((ucs_character >> 0) & 0x3F);
	} else if (ucs_character <= 0x7FFFFFFF) {
		// Six bytes.
		buffer[offset++] = 0xFC | (ucs_character >> 30);
		buffer[offset++] = 0x80 | ((ucs_character >> 24) & 0x3F);
		buffer[offset++] = 0x80 | ((ucs_character >> 18) & 0x3F);
		buffer[offset++] = 0x80 | ((ucs_character >> 12) & 0x3F);
		buffer[offset++] = 0x80 | ((ucs_character >> 6) & 0x3F);
		buffer[offset++] = 0x80 | ((ucs_character >> 0) & 0x3F);
	} else {
		warn("Invalid char; don't encode anything.");
	}
}

// Taken from boost internals
inline short utf8_byte_count(char c0) {
	unsigned int c=c0;
	if((c&0x80u)==0x00)return 1;
	if((c&0xE0u)==0xC0)return 2;
	if((c&0xF0u)==0xE0)return 3;
//	if(c&0xF0==0xF0)return 4;
	// redundant:
	if ((c & 0b10000000u) == 0) return 1;
	if ((c & 0b11100000u) == 0b11000000) return 2;
	if ((c & 0b11110000u) == 0b11100000) return 3;
	return 4;
}

// utf8_byte_count on the first byte of a codepoint represented as utf8 is fine, however AUTO-CASTING U'√' to char is NOT fine!
short utf8_byte_count(codepoint c) {
	if(c==0)return 1;
	return String(c).length;
}

//List<codepoint> split_by_codepoint(String input) {
//	List<codepoint> ret;
//	auto it = input.begin();
//	while (it != input.end()) {
//		uint8_t count = utf8_byte_count(*it);
//		ret.add(input.codepointAt(it));
//		it += count;
//	}
//	return ret;
//}


codepoint *String::extractCodepoints(bool again) {
	if (codepoints and not again)
		return codepoints;
	codepoint_count = 0;
	codepoints = (codepoint *) calloc(length, sizeof(codepoint));
	for (int i = 0; i < length;) {
		short count = utf8_byte_count(data[i]);
		codepoints[codepoint_count++] = decode_unicode_character(&data[i]);
		i += count;
	}
	return codepoints;

}
String EMPTY_STRING0="";
String& EMPTY_STRING=EMPTY_STRING0;
void error1(String message, chars file, int line){
	error1(message.data, file, line);
}


// 0 = NO, 1 = yes at #1

//int String::in(List<String> list) {
//	for (int i = 0; i < list.size(); ++i) {
//		if (list[i] == this)
//			return i+1;
//	}
//	return 0;
//}


bool empty(String &s) { return s.empty(); }

bool empty(String *s) { return not s or s->empty(); }

bool empty(chars s) { return not s or strlen0(s) == 0; }

#undef log // expanded from macro 'log' tgmath.h:245:25:


void log(chars s) {
	logs(s);
}

void log(String *s) {
	if (s->shared_reference)s = s->clone();// add \0 !!
	if (s)logs(s->data);
}

void log(String s) {
	log(&s);
}

void log(long i) {
	logi(i);
}

void log(int i) {
	logi(i);
}

