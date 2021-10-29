//#pragma once
//
// Created by pannous on 24.10.18.
//

#include "String.h"
#include "NodeTypes.h"
#include "wasm_helpers.h"
#include "Map.h"
#include "Code.h"
#include "Util.h"

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


//int ord(codepoint c);// identity!
//	return (int) c;

bool eq(chars dest, chars src, int length) {
	if (!dest || !src)
		return false;
	int i = 0;
	if (dest == "" and src[0])return false;
	if (length < 0 and strlen0(dest) != strlen0(src))
		return false;
//	if length>0 it forces comparison of reference strings terminated by length, not by 0!
	while (char c = dest[i]) {
		if (length >= 0 and i >= length)
			break;
		char d = src[i];
		if (!d || !c)
			return false;
		if (d != c)
			return false;
		i++;
	}
	if (length > 0 and src[i] and not dest[i])// reference strings must be ≠0 for whole length
		return false;
	return true;
}

int strlen0(chars x) {
	if (!x)return 0;
	int l = 0;
	if ((long) x >= MEMORY_SIZE || (long) x == 0x200000000) {
		logs(x);
		logi((int) (long) x);// 0x1000000 16777216
		error("corrupt string");
	}
	if ((long) x == 0x1ffffffff || (long) x >= 0xffffffff00000000 ||
	    ((long) x >= 0x100000000 and (long) x <= 0x100100000))
		return false;// todo: valgrind debug corruption, usually because of not enough memory
	while (l < MAX_STRING_LENGTH and (long) x < MEMORY_SIZE - 1 and *x++)l++;
	return l;
}

// INCORRECT! use decode_unicode_character(str, &len) !
//size_t utf8_strlen(chars utf8bytes) {
//	size_t len = 0;
//	for (chars p = utf8bytes; *p; ++p)
//		if ((*p & 0xc0) != 0x80)
//			++len;
//	return len;
//}


// or cstring
//#ifndef cstring
// needs manual 0 termination, or copy with length + 1
void strcpy2(char *dest, chars src, int length) {// =-1
	if (!dest || !src)
		return;
	int i = 0;
	if (length < 0)length = strlen0(src);
	if (length <= 0)return;
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


/*		(*p) - 0x11052 // …
 *
Some start with 1…9 missing 0
 0x278A	10122	DINGBAT NEGATIVE CIRCLED SANS-SERIF DIGIT ONE	➊
 0x2780	10112	DINGBAT CIRCLED SANS-SERIF DIGIT ONE	➀
 0x2776	10102	DINGBAT NEGATIVE CIRCLED DIGIT ONE	❶
 0x2488	9352	DIGIT ONE FULL STOP	⒈   1.st ≠ 1!
0x2474	9332	PARENTHESIZED DIGIT ONE	⑴
 0x2460	9312	CIRCLED DIGIT ONE	①
 𑁒	11052	BRAHMI NUMBER ONE
  𐄇	10107	AEGEAN NUMBER ONE
 𐋪 	102EA	COPTIC EPACT NUMBER TEN
 𐡘	67672	𐡘	10858	IMPERIAL ARAMAIC NUMBER ONE
 𐤖	67862	𐤖	10916	PHOENICIAN NUMBER ONE
 0x1372	4978	ETHIOPIC NUMBER TEN	፲
 0x2469	9321	CIRCLED NUMBER TEN	⑩
 𐌠	66336	𐌠	10320	OLD ITALIC NUMERAL ONE
0x3021	12321	HANGZHOU NUMERAL ONE	〡
0x2170	8560	SMALL ROMAN NUMERAL ONE	ⅰ
0x2160	8544	ROMAN NUMERAL ONE	Ⅰ
𝍠	119648	𝍠	1D360	COUNTING ROD UNIT DIGIT ONE
 𐋡	66273	𐋡	102E1	COPTIC EPACT DIGIT ONE
8160	𐩀	10A40	KHAROSHTHI DIGIT ONE ... only to 4 !

 some start with 0…9 !
 0xFF10	65296	FULLWIDTH DIGIT ZERO	０
𝟶	120822	𝟶	1D7F6	MATHEMATICAL MONOSPACE DIGIT ZERO
𝟬	120812	𝟬	1D7EC	MATHEMATICAL SANS-SERIF BOLD DIGIT ZERO
𝟢	120802	𝟢	1D7E2	MATHEMATICAL SANS-SERIF DIGIT ZERO
 𝟘	120792	𝟘	1D7D8	MATHEMATICAL DOUBLE-STRUCK DIGIT ZERO
 𝟎	120782	𝟎	1D7CE	MATHEMATICAL BOLD DIGIT ZERO
 𑁦	69734	𑁦	11066	BRAHMI DIGIT ZERO
 0x1810	6160	MONGOLIAN DIGIT ZERO	᠐
 0x17E0	6112	KHMER DIGIT ZERO	០
 0x1040	4160	MYANMAR DIGIT ZERO	၀
 0x0F20	3872	TIBETAN DIGIT ZERO	༠
 0x0ED0	3792	LAO DIGIT ZERO	໐
 0x0E50	3664	THAI DIGIT ZERO	๐
 0x0CE6	3302	KANNADA DIGIT ZERO	೦
 0x0C66	3174	TELUGU DIGIT ZERO	౦
 0x0BE7	3047	TAMIL DIGIT ONE	௧
 0x0B66	2918	ORIYA DIGIT ZERO	୦
 0x0AE6	2790	GUJARATI DIGIT ZERO	૦
 0x09E6	2534	BENGALI DIGIT ZERO	০
 0x0966	2406	DEVANAGARI DIGIT ZERO	०
 0x06F0	1776	EXTENDED ARABIC-INDIC DIGIT ZERO	۰
 0x0660	1632	ARABIC-INDIC DIGIT ZERO	٠
 */
// todo
int zeros[] = {0x1D7F6, 0x1D7EC, 0x1D7E2, 0x1D7D8, 0x1D7CE, 0x11066, 0x1810, 0x17E0, 0x1040, 0x0F20, 0x0ED0, 0x0E50,
               0x0CE6, 0x0C66, 0x0BE7, 0x0B66, 0x0AE6, 0x09E6, 0x0966, 0x06F0, 0x0660, 0};
int ones[] = {0x278A, 0x2780, 0x2776, 0x2488, 0x2474, 0x2460, 0x11052, 0x10107, 0x102EA, 0x10858, 0x10916, 0x10320,
              0x1D360, 0x102E1, 0x3021, 0x2170, 0x2160, 0x1372, 0x2469, 0};

int atoi0(codepoint c) {
	int offset = -1;
	if (c >= '0' and c <= '9')return c - '0';
	while (zeros[++offset]) {
		int k = zeros[offset];
		short n = c - k;
		if (n >= 0 and n <= 9)
			return n;
	}
	offset = -1;
	while (ones[++offset]) {
		int k = ones[offset];
		short n = c - k + 1;
		if (n >= 1 and n <= 9)// most dont have #10
			return n;
	}
	return -1;
}

int atoi0(chars p) {
	if (!p)return 0;
	while (*p == '+')p++;
	short sig = 1;
	if (*p == '-') {
		sig = -1;
		p++;
	}
	int k = 0;
	while (*p) {
		int n;
		if (*p > 0x7F or *p < 0) {
			// some utf8
			short len;
			n = atoi0(decode_unicode_character(p, &len));
			p += len;
		} else {
			n = (*p) - '0';
			p++;
		}
		if (n < 0 or n > 9)
			return k;
		k = (k << 3) + (k << 1) + n;
	}
	if (*p == 'e' or *p == 'E')
		k *= powf(10, atoi0(++p));// we need float for 1/100. …
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
		if (*string == 'e' or *string == 'E')
			return result * powf(10, atoi0(++string));
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
	char *str = (char *) alloc(sizeof(char), 22 + 1);// -18446744073709552000  todo: from context.names char*
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
	while (num != 0 and len < 22) {
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
	char *f = (char *) concat(concat(itoa0(int(num), base), "."), itoa0(abs(int((num - (long) num) * p)), base));
	int len = strlen0(f);// cut trailing 0 : 1.1000 -> 1.1
	for (int i = 1; i < len; ++i) {
		if (f[len - i] == '0')f[len - i] = 0;
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


#ifndef WASM
//relocation R_WASM_MEMORY_ADDR_SLEB cannot be used against symbol nil_name; recompile with -fPIC
String nil_name = "nil";// ␀ ø
String empty_name = "";
String object_name = "{…}";
String groups_name = "(…)";
String patterns_name = "[…]";
String EMPTY = String('\0');
#else
String nil_name;
String empty_name;
String object_name;
String groups_name;
String patterns_name;
String EMPTY;
#endif

#pragma clang diagnostic pop

bool String::empty() const {//this==0 in testMarkMulti!
//#ifdef WASM
//	if(memory_size and data and (long) data > memory_size/*bug!*/)
//		error("CORRUPT String pointer");
////		return true;
//#endif
	if (this == 0)return true;
	if ((long) data == 0x1ffffffff || (long) data >= 0xffffffff00000000 ||
	    ((long) data >= 0x100000000 and (long) data <= 0x100100000))
		return false;// todo: valgrind debug corruption, usually because of not enough memory
	return length == 0 || !data || (long) data > MEMORY_SIZE || data[0] == 0 || data == object_name.data;
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


codepoint decode_unicode_character(chars text, short *len) {
	if ((text[0] & 0b10000000) == 0) {
		if (len)*len = 1; // 1 byte code point, ASCII
		return (text[0] & 0b01111111);
	} else if ((text[0] & 0b11100000) == 0b11000000) {
		if (len)*len = 2; // 2 byte code point
		return (text[0] & 0b00011111) << 6 | (text[0 + 1] & 0b00111111);
	} else if ((text[0] & 0b11110000) == 0b11100000) {
		if (len)*len = 3; // 3 byte code point
		return (text[0] & 0b00001111) << 12 | (text[0 + 1] & 0b00111111) << 6 | (text[0 + 2] & 0b00111111);
	} else {
		if (len)*len = 4; // 4 byte code point
		return (text[0] & 0b00000111) << 18 | (text[0 + 1] & 0b00111111) << 12 | (text[0 + 2] & 0b00111111) << 6 |
		       (text[0 + 3] & 0b00111111);
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

// utf8_byte_count on the first byte of a codepoint represented as utf8 is fine, however AUTO-CASTING U'√' to char is NOT fine!
short utf8_byte_count(codepoint ucs_character) {
	if (ucs_character <= 0x7F) return 1;
	if (ucs_character <= 0x7FF) return 2;
	if (ucs_character <= 0xFFFF) return 3;
	if (ucs_character <= 0x1FFFFF) return 4;
	if (ucs_character <= 0x3FFFFFF) return 5;
// if (ucs_character <= 0x7FFFFFFF)
	return 6;
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
		short count;// = utf8_byte_count(data[i]);
		codepoints[codepoint_count++] = decode_unicode_character(&data[i], &count);
		i += count;
	}
	return codepoints;
}

bool String::startsWith(chars string) {
	return indexOf(string) == 0;
}

bool String::endsWith(const char *string) {
	int len1 = len(string);
	return len1 <= length and eq(data + length - len1, string, len1);
}

String String::to(const char *string) {
	return substring(0, indexOf(string), true);
}

List<String> String::split(const char *string) {
	List<String> parts;
	int len1 = len(string);
	int start = 0;
	for (int i = 0; i < length; i++) {
		if (eq(data + i, string, len1)) {
			const String &part = String(data + start, i - start, !debug);
			parts.add(part);
			start = i + len1;
		}
	}
	const String &rest = String(data + start, -1, true);
	parts.add(rest);
	return parts;
}

String String::trim() {
	int start = 0;
	while (start < length and (data[start] == ' ' or data[start] == '\t' or data[start] == '\n'))start++;
	int end = length - 1;
	while (0 <= end and (data[end] == ' ' or data[end] == '\n'))end--;
	return String(data + start, end - start + 1, true);// share ok?
}

long String::hash() {
	return wordHash(data, min(length, 20));
//	return (long)data;// only conflict: shared substring(0,i);
}


String &String::lower() {
	String &clone1 = clone();
	lowerCase(clone1.data, length);
	return clone1;
}

String EMPTY_STRING0 = "";
String &EMPTY_STRING = EMPTY_STRING0;

void error1(String message, chars file, int line) {
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

bool empty(codepoint s) {
	return 0 <= s and s <= ' ';// not s or s == ' ' or s == '\n' or s == '\t' or s==0x0F or s==0x0E;
}


bool contains(chars str, chars match) {
	int l = strlen0(match);
	for (int i = 0; str[i] != 0; i++) {
		bool ok = true;
		for (int j = 0; j < l; j++) {
			if (str[i + j] != match[j]) {
				ok = false;
				break;
			}
		}
		if (ok)return true;
	}
	return false;
}

#undef log // expanded from macro 'log' tgmath.h:245:25:


void log(chars s) {
	logs(s);
#ifndef WASM    // console.log adds newline
	logs("\n");
#endif
}

void print(const Node node) {
	print(node.string());
}

void log(String *s) {
	if (s->shared_reference)log(s->clone());// add \0 !!
	else if (s)log(s->data);
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

void log(char c) {
	logc(c);
}


