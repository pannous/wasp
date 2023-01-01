//#pragma once
//
// Created by pannous on 24.10.18.
//

#include "String.h"
#include "wasm_helpers.h"
#include "List.h"
#include "Code.h"
#include "Util.h"

//SET(CMAKE_SYSROOT /opt/wasm/wasi-sdk/share/wasi-sysroot/) # also for WASM include!
#include <stdlib.h> // pulls in declaration of malloc, free
#include <math.h> // pow


#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wstring-compare"

//String* String::EMPTY_STRING=new String();

//#ifndef WASM
#include <cstdio>
#include <cstring>
//#else
//#endif


typedef unsigned char byte;
typedef chars chars;

class String;

//char *empty_string = "";

bool eq(chars dest, chars src, int length) {
    if (!dest || !src)
        return false;
    if (length == 0)return !src[0] and !dest[0];
    if (dest[0] == 0)return !src[0];
    if (src[0] == 0)return !dest[0];
//	be sure check to src.length == dest.length before:
//	length>0 forces comparison of reference strings terminated by length, not by 0!
    if (length < 0 and strlen(dest) != strlen(src))
        return false;
    int i = 0;
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

// needs manual 0 termination, or copy with length + 1
void strcpy2(char *dest, chars src, int length) {// =-1
    if (!dest || !src)
        return;
    int i = 0;
    if (length < 0)length = strlen(src);
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


/*		UNICODE NUMERALS 𝟎=0 etc
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
// todo 0x2080, 0x2070 subscript OPERATOR vs x²⁺³ == x⁵
// unicode ranges 0...9
static int zeros[] = {0x1D7F6, 0x1D7EC, 0x1D7E2, 0x1D7D8, 0x1D7CE, 0x11066, 0x1810, 0x17E0, 0x1040, 0x0F20, 0x0ED0,
                      0x0E50,
                      0x0CE6, 0x0C66, 0x0BE7, 0x0B66, 0x0AE6, 0x09E6, 0x0966, 0x06F0, 0x0660, 0, 0};
// unicode ranges 1...9
static int ones[] = {0x278A, 0x2780, 0x2776, 0x2488, 0x2474, 0x2460, 0x11052, 0x10107, 0x102EA, 0x10858, 0x10916,
                     0x10320,
                     0x1D360, 0x102E1, 0x3021, 0x2170, 0x2160, 0x1372, 0x2469, 0, 0};

// unlike atoi0 returns -1 if not a digit!
int atoi1(codepoint c) {
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

// todo: 0 ambiguous "0" or "NaN"  use smartType!
int64 parseLong(chars str) {
    if (!str)return 0;
    while (*str == '+')str++;
    short sig = 1;
    if (*str == '-') {
        sig = -1;
        str++;
    }
    int64 k = 0;
    while (*str) {
        int64 n;
        short len;
        n = atoi1(decode_unicode_character(str, &len));// inline!
        str += len;
        if (n < 0 or n > 9)break;
        if (k < 0)k = 0;
        k = (k << 3) + (k << 1) + n;
    }

    if (k > 0 and (*str == 'e' or *str == 'E'))
        k *= powd(10, parseLong(++str));// we need float for E-10 == 1/10 …
    return sig * k;
}

//extern "C"
int atoi(char *s) {
    return parseLong(s);
}

extern double parseDouble(chars string) {
// __extenddftf2
    double result = 0.0;
    if (!string) return result;

    double multiplier = 1;
    double divisor = 1.0;
    int64 integer_portion = parseLong(string);
    result = (double) integer_portion;
    if (*string == '-') {
        result *= -1;
        multiplier = -1;
    }

    while (*string && (*string != '.') && (*string != 'e') && (*string != 'E'))
        string++;
    if (*string and (*string == '.'))
        string++;
    while (*string) {
        if (*string == 'e' or *string == 'E') {
#if MY_WASI
            todo("EXP")
#else
            return result * (double) powl(10, parseLong(++string));
#endif
        }
        if (*string < '0' || *string > '9') return result;
        divisor *= 10.0;
        result += (double) (*string - '0') / divisor;
        string++;
    }

    return result * multiplier;
}

class Node; // can't pre-access properties, BUT can use functions:
String toString(Node &node);

// Implementation of itoa0()
char *formatLongWithBase(int64 num, int base = 10) {
    if (base == 16)return hex(num);
    // length 22 -> put(num)/2+2 for base 10
    static char str[23];
//    char *str = (char *) alloc(sizeof(char), 22 + 1);// -18446744073709552000  todo: from context.names char*
//	int addr=(int)(int64)str;
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

char *formatLong(int64 num) {
    return formatLongWithBase(num, 10);
}

char *ltoa(int64 num) {
    return formatLongWithBase(num, 10);
}

char *itoa0(int64 num) {// todo remove once you know the right call
    return formatLongWithBase(num, 10);
}

//#define pow(val,exp)
chars formatRealWithBaseAndPrecision(double num, int base = 10, int digits_after_zero = 4) {/*significant_digits*/
//	int p = powi(base,digits_after_zero+1);
    auto remainder = abs(num) - abs(int64(num));
//	auto remainder = abs_f(num) - abs_l(int64(num));
//	auto remainder = itoa0(abs(int((num - (int64) num) * p)), base);
    chars f = concat(formatLongWithBase(int(num), base), ".");
//	significant_digits-=strlen(f)-1
    while (digits_after_zero-- > 0) {
        remainder *= base;
        auto digit = formatLongWithBase(int(remainder), base);
        remainder -= int(remainder);
        f = concat(f, digit);
    }
    int len = strlen(f);// cut trailing 0 : 1.1000 -> 1.1
    for (int i = 1; i < len; ++i) {
        if (f[len - i] == '0') ((char *) f)[len - i] = 0;
        else break;
    }
    return f;
}

chars formatRealWithSignificantDigits(double num, int digits_after_zero = 4) {/*significant_digits*/
    return formatRealWithBaseAndPrecision(num, 10, digits_after_zero);
}

// todo: alias elsewhere
extern "C" chars formatReal(double num) {/*significant_digits*/
    return formatRealWithBaseAndPrecision(num, 10, 4);
}

chars ftoa(double num) { return formatRealWithBaseAndPrecision(num, 10, 4); }


// -123.4E-100\0
chars ftoa2(float num, int significant_digits) {
    int exp = 0;
    while (num > 1 or num < -1) {
        num /= 10;
        exp++;
    }
    while (num < 1 and num > -1) {
        num *= 10;
        exp--;
    }
    char *f = static_cast<char *>(malloc(significant_digits + 8));// -123.4E-100\0
    int pos = 0;
    if (num < 0)f[pos++] = '-';
    f[pos++] = '0' + int(num);
    f[pos++] = '.';
    while (significant_digits-- > 1) {
        num -= int(num);
        num *= 10;
        f[pos++] = '0' + int(num);
    }
    while (pos > 0 and f[pos] == '0')f[pos--] = 0;
    f[pos++] = 'E';
//	strcpy2(&f[pos],"×10^");
    strcpy2(&f[pos], formatLong(exp));
    return f;
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





//void put(Node &node) {
//	Node node2 = node;
//	put(node2);
//}


//#ifndef WASM
//#include  <string>
//void put(std::string s) {
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
    return String(c, (int) t);
}

String operator "" s(chars c, unsigned long t) {// function signature contains illegal type WHYY??
    return String(c, (int) t);
}

String operator "" _(chars c, unsigned long t) {
    return String(c, (int) t);
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


#pragma clang diagnostic pop

// can mostly be omitted via operator bool(){} / operator char *() const { return data; }!!
bool String::empty() const {//this==0 in testMarkMulti!
//	if(memory_size and data and (int64) data > memory_size/*bug!*/)
////		return true;
    if (this == 0)return true;
    if ((int64) this < 8)return true;// zero page broken object hack
    if (length == 0)return true;
    if (this->data == 0)return true;
    return false;
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


int encode_unicode_character(char *buffer, wchar_t ucs_character) {
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
    return offset;
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
    int len1 = strlen(string);
    length = strlen(data);// todo LOST WHEN?
    return len1 <= length and eq(data + length - len1, string, len1);
}

String String::to(const char *string) {
    return substring(0, indexOf(string), true).clone();
}

List<String> String::split(const char *string) {
    List<String> parts;
    int len1 = strlen(string);
    int start = 0;
    for (int i = 0; i < length; i++) {
        if (eq(data + i, string, len1)) {
            String part = String(data + start, i - start, !debug);
            parts.add(part);// copy by value, hopefully c++ optimizer is smart enough ;)
            start = i + len1;
        }
    }
    String rest = String(data + start, -1, true);
    parts.add(rest);
    return parts;//.clone();  return by copy value ok (list items[] separate)
}

String String::trim() {
    int start = 0;
    while (start < length and (data[start] == ' ' or data[start] == '\t' or data[start] == '\n'))start++;
    int end = length - 1;
    while (0 <= end and (data[end] == ' ' or data[end] == '\n'))end--;
    return String(data + start, end - start + 1, true);// share ok?
}

int64 String::hash() const {
    return wordHash(data, min(length, 20));
//	return (int64)data;// only conflict: shared substring(0,i);
}


String &String::lower() {
    String &clone1 = clone();
    lowerCase(clone1.data, length);
    return clone1;
}

void String::shift(int i) {
    while (length > 0 and i-- > 0) {
        data++;
        length--;
    }
}

String String::from(const char *string) {
    return substring(this->indexOf(string) + strlen(string));
}

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

bool empty(chars s) { return not s or strlen(s) == 0; }

bool empty(codepoint s) {
    return 0 <= s and s <= ' ';// not s or s == ' ' or s == '\n' or s == '\t' or s==0x0F or s==0x0E;
}


bool contains(chars str, chars match) {
    if (!str)
        return false;
    int l = strlen(match);
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

void put(chars s) {
    put_chars(s);
    newline();
}

void print(const Node node) {
    print(node.string());
}


//bool skip_newline = false;
//void print(int l){
//    put_chars(formatLong(l));
//}
//void print(long l) {
//    put_chars(formatLong(l));
//}
void print(int64 l) {
    put_chars(formatLong(l));
}
//void print(size_t l) {
//    put_chars(formatLong(l));
//}

void print(char c) {
    put_char(c);
    newline();
}

void print(char const *s) {
    put_chars(s, strlen(s));
}

void print(String *s) {
    if (s->shared_reference)print(s->clone());// add \0 !!
    else if (s)put(s->data);
}

void print(void *s) {
    put_chars(hex((int64) s));
}

void print(String s) {
    put_chars(s.data, s.length);
//    if (tracing)
//        newline();
}

void println(String s) {
    put_chars(s.data, s.length);
    newline();
}


void println(int64 s) {
    put_chars(formatLong(s));
    newline();
}


void println(Node &s) {
    println(s.serialize());
}

// don't extern "C", else demangle can't reflect … see put_string
void put(String s) {
    put_chars(s.data, s.length);
    newline();
}

void print(char *str) {
#if MY_WASI
    puts(str);
#else
    printf("%s", str);
#endif
}

String *EMPTY_STRING;

String *empty_string() {
    if (!EMPTY_STRING)EMPTY_STRING = new String();
    return EMPTY_STRING;
}


// starting with 1!
//inline haha you can't inline wasm
[[maybe_unused]]
codepoint getChar(chars string, int nr) {
    return String(string).codepointAt(nr - 1);
    // todo codepoint
//    int len = strlen(string);
//    if (nr < 1)error("#index starts with 1, use [] if you want 0 indexing");
//    if (nr > len)error("index out of bounds %i>%i "s % nr % len);
//    return string[nr - 1 % len];
}

//extern "C"  // only pointers!
String &string(chars chars) { return *new String(chars); }

extern "C"  // only pointers!
String *str(chars chars) {
    return new String(chars);
}
