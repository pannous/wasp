#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-parameter"
#pragma clang diagnostic ignored "-Wwritable-strings"
#pragma once
//
// Created by pannous on 19.12.19.
//
#include "wasm_helpers.h"
#include "Util.h"
#include "NodeTypes.h"
//#include "Map.h" recursive include error Node.h:60:9: error: field has incomplete type 'String'

#include <cstdlib> // OK in WASM!

#ifndef WASM

#include <cstdio> // printf

#endif
typedef size_t codepoint_offset;
#define MAX_STRING_LENGTH 100000 // only for strlen()
//#define MAX_WASM_DATA_LENGTH 0x1000000 // 16 MB
//#define MAX_WASM_DATA_LENGTH 0x80000 // 1/2 MB
#define MAX_WASM_DATA_LENGTH 0x40000 // 1/4 MB
//#define MAX_WASM_DATA_LENGTH 0x20000 // 1/8 MB

typedef chars chars;
typedef const unsigned char *wasm_string;// wasm strings start with their LEB encoded length and do NOT end with 0 !! :(

class String;

class Node;

template<class S>
class List;

#ifdef STD_STRING
#warning STD_STRING defeats the purpose of using a lightweight unicode-aware String class
#endif


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

/* WASM defines:
 * https://github.com/WebAssembly/stringref/blob/main/proposals/stringref/Overview.md
    codepoint: An integer in the range [0,0x10FFFF].
    surrogate: A codepoint in the range [0xD800,0xDFFF].
    unicode scalar value: A codepoint that is not a surrogate.
    character: An imprecise concept that we try to avoid in this document.
    code unit: An indivisible unit of an encoded unicode scalar value. For UTF-8 encodings, an integer in the range [0,0xFF] (a byte); for UTF-16 encodings, an integer in the range [0,0xFFFF]; for UTF-32, the unicode scalar value itself.
    high surrogate: A surrogate in the range [0xD800,0xDBFF].
    low surrogate: A surrogate which is not a high surrogate.
    surrogate pair: A sequence of a high surrogate followed by a low surrogate, used by UTF-16 to encode a codepoint in the range [0x10000,0x10FFFF].
    isolated surrogate: Any surrogate which is not part of a surrogate pair.
 * */

//typedef Image glyph; image, usually stored in a font (which is a collection of glyphs), used to represent graphemes
//https://stackoverflow.com/questions/27331819/whats-the-difference-between-a-character-a-code-point-a-glyph-and-a-grapheme
// '\uD83D\uDC0A' UTF-16 code units expressing a single code point (U+1F40A)  char(0x1F40A) == '🐊'
//https://github.com/foliojs/grapheme-breaker/blob/master/src/GraphemeBreaker.js


void newline();

String &hex(int64 d, bool include_0x = true, bool upper_case = false);

enum sizeMeasure {
    by_char8s,// bytes
//	char16s,
    by_codepoints,
    by_graphemes
};

void reverseInPlace(char *str, int len);

codepoint decode_unicode_character(chars utf8bytes, short *len = 0);

short utf8_byte_count(codepoint c);

//decode_codepoint
char *formatLongWithBase(int64 num, int base);

char *formatLong(int64 num);

char *formatLong(int64 num);

int atoi1(codepoint c);// usually byte 0…10, but with special signs for CYRILLIC MILLIONS ҉ we stick with int

int64 parseLong(chars str);

//float64
double parseDouble(chars string);

int encode_unicode_character(char *buffer, wchar_t ucs_character);

String toString(Node &node);

String str(chars &s);

String str(char *string);

String s(chars &s);

//int ord(codepoint c);// identity!

bool eq(chars dest, chars src, int length = -1);

void strcpy2(char *dest, chars src);

void strcpy2(char *dest, chars src, int length);

#if EMSCRIPTEN
extern "C" size_t strlen(const char *);
#elif WASM
extern "C" size_t strlen(const char *) __attribute__((__nothrow__, __leaf__, __pure__, __nonnull__(1)));
#else
extern "C" size_t strlen(const char *s);
#endif
//int strlen(chars x);
//size_t   strlen(chars __s);


//chars ftoa(float num, int base = 10, int precision = 4);
chars formatRealWithBaseAndPrecision(double num, int base, int precision);

chars ftoa2(double num, int significant_digits = 4);

chars ftoa(double num);

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

//extern char *empty_string;// = "";

//duplicate symbol '_empty_string'


//char null_value[]={0};// todo make sure it's immutable!!

// todo: make String struct-compatible with Node!?

//enum StringKind{
//    Primitives
//};

String *empty_string();
//class CodepointList{
//
//};
// following general Header struct / wasm32_node_struct

// https://github.com/WebAssembly/stringref
class String {
//    A UTF-8 environment can use non-synchronized continuation bytes as base64: 0b10   Base58 avoids lookalikes

public:
    char *data{};// UTF-8 sequence
    int length = -1;
    Primitive kind = (Primitive) string_header_32;  // post header ;) static const
    codepoint *codepoints = 0;//  todo reuse data field after decode? nah, breaks fd_write  extract on demand from data or via constructor
    int codepoint_count = -1;// 'type' field in list, node and array_header, semi compatible
    bool shared_reference = false;// length terminated substrings! copy on modify if shared views. // todo: move to header?
    // todo is shared_reference sufficient for (im)mutable final const keywords?
// memory layout different to chars with leb length header!
    // todo add UTF-16 representation as codepoint union?
private:
//    static String* EMPTY_STRING;

public:
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

    // can also be directly cast (String)c_io_vector, BUT need to set codepoint_count=-1 after!
    String(c_io_vector ciov, bool shared = true) {
        data = (char *) ciov.string;
        length = ciov.length;
        shared_reference = shared;
    }

    String(String *copy_move_assignment) {
        data = copy_move_assignment->data;
        length = copy_move_assignment->length;
        shared_reference = true;// shared;// better safe than sorry
    }

//#ifndef WASM
////	Error while importing "env"."_Znwm": unknown import.
    void *operator new(size_t size) {
        return static_cast<String *>(calloc(size, 1));// WOW THAT WORKS!!!
    }
//#endif

    void operator delete(void *) {
//        check(header == string_header_32);
        /*lol*/} // Todo ;)

//	~String()=default;

#ifdef STD_STRING
    String(const std::string str):String(str.c_str){
        // defeats the purpose of using a lightweight unicode-aware String class
    }
#endif

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

    String(const char string[], int length0) {// todo signature for merger experimental!
//		shared_reference= true;
        length = length0;
//		data =(char*) reinterpret_cast<const char *>(string);
        data = (char *) (alloc(sizeof(char), length + 1));
        memcpy(data, string, length);
        data[length] = 0;
    }

//	explicit
    String(const char string[], bool copy = true/*false AFTER all is tested, for efficiency*/) {
        // todo, make Node(string) copy = true in many situations?
        // todo (maybe dont) mark data as to-free on destruction once copy = false AND bool malloced = true
//		data = const_cast<char *>(string);// todo heap may disappear, use copy!
        length = string == 0 ? 0 : strlen(string);
        if (length == 0)data = 0;//SUBTLE BUGS if setting data="" data=empty_string !!!;//0;//{data[0]=0;}
        else {
            if (copy) {
                data = (char *) (alloc(sizeof(char), length + 1));
                strcpy2(data, string, length);
                data[length] = 0;
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
        if (length < 0 or length >= 128) todo("decode full LEB128");
        data = (char *) s + length_bytes;
    }

//		String operator+(Type e){
//	explicit String(::Kind type) : String(typeName(type)) {}// lil hack to get String of specific enums
//	explicit String(Type type) : String(typeName(type)) {}// lil hack to get String of specific enums

    explicit String(int integer) {
        data = formatLong(integer);// wasm function signature contains illegal type WHYYYY
        length = strlen(data);
    }

    explicit String(uint integer) {
        data = formatLong(integer);// wasm function signature contains illegal type WHYYYY
        length = strlen(data);
    }

    explicit String(int64 number) {
        data = formatLong(number);
        length = len();
    }

    explicit String(char16_t utf16char) {
        data = (char *) (calloc(sizeof(char16_t), 4));// 2byte can be unrolled into 3(+??) bytes, e.g. u'☺'
        length = encode_unicode_character(data, utf16char);
        data[length] = 0;
    }

//	explicit String(char16_t* chars){
//		while ... data = (char*)(calloc(sizeof(char16_t),len(chars)));
//		length+ = encode_unicode_characters(data,);
//	}

// char32_t same as codepoint!
    explicit String(char32_t utf32char) {// conflicts with int
        auto byteCount = utf8_byte_count(utf32char);
        data = (char *) (calloc(sizeof(char32_t), byteCount + 1));
        encode_unicode_character(data, utf32char);
        length = byteCount;// at most 4 bytes
        data[length] = 0;// be sure
    }

    explicit String(wchar_t wideChar) {
        data = (char *) (calloc(sizeof(wchar_t), 2));
        length = encode_unicode_character(data, wideChar);
        data[length] = 0;// be sure
    }

    explicit String(double real) {
        int max_length = 4;
        data = formatLong(real);
        length = len();
//		itof :
        append('.');
        real = real - (int64(real));
        while (length < max_length) {
            real = (real - int64(real)) * 10;
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
            if (codepoint_count < 0)
                extractCodepoints();
            return codepoint_count;
        }
        if (by == by_graphemes) {
            todo("graphemes");
        }
        todo("unknown sizeMeasure");
        return -1;
    }

    size_t len() {
        return length >= 0 ? length : !shared_reference and strlen(data);
    }

    char charAt(int position) {
        if (position >= length)
            error((String("IndexOutOfBounds at ") + formatLong(position) + " in " + data).data);
        return data[position];
    }

    char charCodeAt(int position) {
//		if (position >= length)
//			raise(IndexOutOfBounds(data, position).message);
//		String("IndexOutOfBounds at ") + i + " in " + data;
//			throw new IndexOutOfBounds(String(" at ") + i + " in " + data);
        return data[position];
    }

    int indexOf(char c, int from = 0, bool reverse = false) {
        for (int j = from; j < length and j < MAX_STRING_LENGTH; j++) {
            if (reverse and data[j] == c)return j;
            if (!reverse and data[j] == c)return j;
        }
        return -1;
    }

    int find(char c, int from = 0, bool reverse = false) { return indexOf(c, from, reverse); }

//	operator std::string() const { return "Hi"; }

// excluding to
// todo ref param is confusing as one can expect it to be 'include = true/false'
    String substring(int from, int to = -1, bool ref = false /* true after all is tested*/) { // excluding to
        if (from < 0 or (from == 0 and (to == length or to == -1))) return *this;
        if (to < 0) to = length + to + 1;// -2 : skip last character
        if (to > length or to < -length) to = length;
        if (to <= from) return "";//EMPTY_STRING;
        if (from >= length) return "";//EMPTY_STRING;
        int len = to - from;
        return String(data + from, len, ref);
    }
//
//    int len(chars data) {
//        if (!data)data = this->data;
//        if (!data || data[0] == 0)
//            return 0;
//        int MAX = 10000;
//        for (int i = 0; i < MAX; ++i) {
//            if (!data[i])return i;
//        }
//        return -1;//error
//    }


    String &append(chars c, int byteCount = -1) {
        if (byteCount < 0) byteCount = strlen(c);
        if (!data) {
            data = (char *) (alloc(sizeof(char), byteCount + 1));
#if WASM
            } else if (data + length + 1 == (char*) heap_end) {// just append recent
                heap_end += byteCount + 1;
#endif
        } else {
            auto *neu = (char *) (alloc(sizeof(char), length + byteCount + 1));
            if (data)strcpy2(neu, data, length);
            data = neu;
        }
        strcpy2(data + length, c, byteCount);
        length += byteCount;
        data[length] = 0;
        return *this;
    }

    String &append(codepoint c) {
        auto byteCount = utf8_byte_count(c);
        if (!data) {
            data = (char *) (alloc(sizeof(char), byteCount + 1));
#if WASM
            } else if (data + length + 1 == (char*) heap_end) {// just append recent
                heap_end += byteCount + 1;
#endif
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
        String b = this->clone();
        const String &serial = toString(c);
        if (contains("%@"))
            b = b.replace("%@", serial);
        else if (contains("%o"))
            b = b.replace("%o", serial);
        else if (contains("%s"))
            b = b.replace("%s", serial);
        else {
            warn("string interpolation missing %s / %o / %@ slot for node");
            return *this + serial;
        }
        return b;
    }

    String operator%(Node *c) {
        return *this % *c;
    }

//    String operator%(chars &c) {
//        return this->replace("%s", c);
//    }

    String operator%(chars c) {
        if (!this->contains("%s"))return this->append(c);
        return this->replace("%s", c);
    }

    String operator%(char *c) {
        return this->replace("%s", c);
    }

    String operator%(char c) {
        return this->replace("%c", String(c).data);
    }

    String operator%(int d) {
        if (contains("%d"))
            return this->replace("%d", formatLong(d));
        if (contains("%x"))
            return this->replace("%x", hex(d));
        if (contains("%i"))
            return this->replace("%i", formatLong(d));
        if (contains("%li"))
            return this->replace("%li", formatLong(d));
        if (contains("%ld"))
            return this->replace("%ld", formatLong(d));
        if (contains("%l"))
            return this->replace("%l", formatLong(d));
        if (contains("%zu"))
            return this->replace("%zu", formatLong(d));
        if (contains("%c"))
            return this->replace("%c", String((codepoint) d));
        if (contains("%C"))
            return this->replace("%C", String((codepoint) d));
        put_chars("ERROR\nmissing placeholder %d in string modulo operation s%d:\n");
        put_chars(this->data, this->length);
        put_chars(" value:");
        put_chars(formatLong(d));
        proc_exit(-1);
        return "«ERROR»";
    }

    String operator%(unsigned int d) {
        return this->operator%((int) d);
    }

    String operator%(codepoint d) {
        return this->operator%((int) d);
    }

    String operator%(uint64 d) {
        return this->operator%((int64) d);
    }

    String operator%(int64 d) {
        if (contains("%lld"))
            return this->replace("%lld", formatLong(d));
        else if (contains("%ld"))
            return this->replace("%ld", formatLong(d));
        else if (contains("%llx"))
            return this->replace("%llx", hex(d));
        else if (contains("%lx"))
            return this->replace("%lx", hex(d));
        else if (contains("%l"))
            return this->replace("%l", formatLong(d));
        else if (contains("%d"))
            return this->replace("%d", formatLong(d));
        else if (contains("%x"))
            return this->replace("%x", hex(d));
        print("FORMAT:");
        printf("%s", data);
        printf("int64 arg:");
        print(formatLong(d));
        error("missing placeholder %d in string modulo operation s%d");
        return "«ERROR»";
    }

    String operator%(size_t s) {
        return *this % (int64) s;
    }


    String operator%(double f) {
        String formated = String() + formatLong(f) + "." + formatLong((f - int(f)) * 10000);
        return this->replace("%f", formated);
    }
//
//	String operator%(float f) {
//		String formated = String() + itoa0(f) + "." + itoa0((f - int(f)) * 10000);
//		return this->replace("%f", formated);
//	}

// ambiguous (with operand types 'String' and 'String&') yet x+=name converts to char* WHY???
//	String *operator+=(String c) {
//		this->data = (*this + c).data;
//		this->length += c.length;
//		return this;
//	}

    String *operator+=(String &c) {
        append(c.data, c.length);
        return this;
    }

    String *operator+=(String *c) {
        if (c)append(c->data, c->length);
        return this;
    }

    String *operator+=(char *c) {
        append(c);
        return this;
    }

//    operator+() should not return a reference type as it is a new (locally declared) instance that holds the result of the operation.
    String *operator+=(chars c) {
        append(c);
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
    String &operator+(Type type) {
        return this->append(typeName(type));
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
        String *ok = new String(neu);// never to be freed => buffer overflow sometime?
        ok->length = length + c.length;
        return *ok;
    }

    String operator+(const char x[]) {
        return this->operator+(String((char *) x));
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

    String operator+(size_t i) {
        return this->operator+(int64(i));
    }

    String operator+(int i) {
        return this->operator+(String(i));
    }

    String operator+(uint i) {
        return this->operator+(String(i));
    }

    String operator+(int64 i) {
        return this->operator+(String((int64) i));
    }

    String operator+(uint64 i) {
        return this->operator+(String((int64) i));
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

    // self modifying ok?
    String operator++(int postfix) {//
        if (length - postfix <= 0)return "";
        this->data += 1 + postfix;
        length -= 1 + postfix;
        return *this;
    }

    String operator+(char *c) {
        return this->operator+(String(c));
    }


    bool operator==(char16_t c) {
        return this->operator==(String(c));
    }

//    bool operator==(const String s) {
//        return s == this;
//    }

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
        return eq(data, c, length);
    }

    bool operator==(char *c) {
        return eq(data, c, length);
    }

// need name == (char*)"‖" WTH
//    use of overloaded operator '==' is ambiguous WHY??
//    bool operator==(const char *c) const{
//        return eq(data, c, length);
//    }

    bool operator==(String *c) const {
        if (!c)return this->empty();
        if (this->empty())return not c or c->empty();
        return eq(data, c->data, length);
    }

//	bool operator!=(const String s) {// const
//		return this != &s;
//	}

//	bool operator!=(String s) {// const
//		return this != &s;
//	}

    bool operator!=(String &s) {// const
        if (this->empty())return !s.empty();
        if (s.empty())return !this->empty();
        if (s.length != length)return true;
        return !eq(data, s.data, length);
    }

//	bool operator==(const String other ) {
//		return length == other.length && eq(data, other.data, shared_reference? length:-1);
//	}  ambiguous with

    bool operator==(String &s) {// const
        if (this->empty())return s.empty();
        if (s.empty())return this->empty();
        if (s.length != length)return false;
        return eq(data, s.data, length);
    }

    bool operator==(String *s) {// const
        if (!s or s->empty())return empty();
        if (this->empty())return not s or s->empty();
        if (s->length != length)return false;
        return eq(data, s->data, length);
    }

    bool operator==(char *c) const {
        return eq(data, c, length);
    }

    bool operator!=(char *c) {
        return !eq(data, c);
    }

    bool operator!=(const String &c) {
        if (c.length != length)return false;
        return !eq(data, c.data, length);
    }

//#define min(a, b) (a < b ? a : b)

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

    String &operator||(String &s) {// const
        if (this->empty())return s;
        return *this;
    }

    codepoint operator[](codepoint_offset i) {
        return codepointAt(i);
    }

//    grapheme operator[](grapheme_offset i) {
//        return graphemeAt(i);
//    }


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


    int indexOf(chars string, bool reverse = false) {
        int l = strlen(string);
//        if ((int64) data + l > MAX_MEM)
//            error("corrupt string"); // let it fail / auto-grow!?
        for (int i = 0; i <= length - l; i++) {
            bool ok = true;
            int i0 = reverse ? length - i : i;
            for (int j = 0; j < l; j++) {
                if (data[i0 + j] != string[j]) {
                    ok = false;
                    break;
                }
            }
            if (ok)
                return i0;
        }
        return -1;//
    }

    int lastIndexOf(const char *string) {
        return indexOf(string, true);
    }

    bool contains(chars string) {
        return indexOf(string) >= 0;
    }

    bool contains(char chr) {
        return indexOf(chr) >= 0;
    }


//    [[non-modifying]]
    [[nodiscard("replace generates a new string to be consumed!")]]
    __attribute__((__warn_unused_result__))
    String &replace(chars string, chars with) {// first only!
        int i = this->indexOf(string);
        if (i >= 0) {
            unsigned int from = i + strlen(string);
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
        if (with.contains(part)) todo("incremental replaceAll");
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

    explicit operator int() { return parseLong(data); }

//	 operator char*()  { return data; }
    explicit operator int() const { return parseLong(data); }

//	MUST BE explicit, otherwise String("abc") != "abc"  : char* comparison hence false
//	explicit // cast // todo ^^^ HUH!?
    operator char *() const { return data; }

    explicit // if(String("0")) OK, BUT bool x=String("0") calls operator char *() !
    operator bool() const { return data and length; } // better safe then sorry!!
//    operator char *() const { return this ? data : 0; }

//	operator codepoint *() { return extractCodepoints(); }


    [[nodiscard]] bool isNumber() const {
        return data[0] == '0' or parseLong(data);
    }

    String format(int i) {
        return this->replace("%d", formatLong(i));
    }

    String format(int64 i) {
        return this->replace("%d", formatLong(i));
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

    int64 hash() const;

    String &lower();

//	void lower();
    void shift(int i);

    String from(const char *string);
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

String operator ""_(chars c, unsigned long);// invalid literal operator parameter type uint64

String operator ""_s(chars c, unsigned long);

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wuser-defined-literals"

String operator ""s(chars c, unsigned long);

#pragma clang diagnostic pop


//#define UNEXPECT_END "Unexpected end of input"
//#define UNEXPECT_CHAR "Unexpected character "

static String EMPTY = "";// = String('\0');
static chars nil_name = "nil";

//String operator "" s(chars c, uint64 );// wasm function signature contains illegal type
//String operator "" _(chars c, uint64 );
//String operator "" _s(chars c, uint64 );
void print(String *s);

void print(void *s);

void print(char const *s);

//void print(char *s);
//void print(const char *s);
void print(String);

void print(Type);
void print(Type32);
void println(Node &s);

void println(String); // ==

void println(int64);

void put(String);

extern "C" int puts(const char *);// int return needed for stdio compatibilty !
//void puts(const char *);

//#endif
//unsigned  == unsigned int!
//inline
short utf8_byte_count(char c);


bool empty(String &s);

bool empty(String *s);

bool empty(chars);

bool empty(codepoint);// todo: rename whitespace (and braces??)

bool contains(chars str, chars match);


//extern "C"  // only pointers!
String &string(chars);

extern "C"  // only pointers!
String *str(chars);


#ifdef WEBAPP
// todo expensive!
#include <sstream>
void render(Node &node, std::stringstream *html = 0);
#endif


#pragma clang diagnostic pop

codepoint getChar(chars string, int nr);

