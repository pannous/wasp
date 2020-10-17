//#pragma once
//#define X86_64 1

bool debug=true;
#ifdef WASM
//#include <exception>
void raise(char *error){
	throw ;// wasm CAN'T throw andy object!!: undefined symbol: typeinfo for char const*
//		throw;// YAY OK!!
//		error("OK");// undefined symbol: typeinfo for char const*
//			throw ("OK");
//		throw 0;// undefined symbol: typeinfo for int
//		throw String();// vtable for __cxxabiv1::__class_type_info
//		throw nullptr;// ERR();
//		throw std::exception();// doesn't help … undefined symbol: vtable for std::exception
}
#include "String.cpp" // Todo: wasm can't link to String.cpp functions from String.h (e.g. itoa0)
#else
#include <cctype>
#include "String.h"
void raise(char *error){
	throw error;
	}
#endif

//#include "Node.cpp"
extern unsigned int *memory;
unsigned int *memory=(unsigned int *) 4096; // todo how to not handtune _data_end?
unsigned int *current = memory;

unsigned long __stack_chk_guard= 0xBAAAAAAD;
void __stack_chk_guard_setup(void) { __stack_chk_guard = 0xBAAAAAAD;/*provide some magic numbers*/ }
void __stack_chk_fail(void) { /*log("__stack_chk_fail");*/} //  Error message will be called when guard variable is corrupted

#define let auto
#define var auto
typedef void *any;
typedef unsigned char byte;
typedef const char *chars;
//bool polish_notation = true;

bool throwing = true;// otherwise fallover beautiful-soup style generous parsing

//extern "C" int isalnum(int _c);
extern bool polish_notation;

#ifdef WASM
#warning COMPILING TO WASM
#else
#warning COMPILING TO APPLE
#endif

typedef unsigned long size_t;

#ifndef WASM
#include "ErrorHandler.h"
#endif


//#import "WasmHelpers.h" // import so names are not mangled
#include "WasmHelpers.h"
#include "String.h" // variable has incomplete type
#include "Node.h"
//#import "String.cpp" // import against mangling in wasm (vs include)
#include "wasm-emitter.h"

#ifdef WASM
#ifdef X86_64
typedef number unsigned int size_t;
#else
// typedef number unsigned int size_t;
//typedef unsigned int size_t;
typedef unsigned long size_t;
#endif


extern "C" void * memset ( void * ptr, int value, size_t num ){
	int* ptr0=(int*)ptr;
	for (int i = 0; i < num; i++)
		ptr0[i]=value;
	return ptr;
}
//void* operator new[](size_t size){ // stack
//	current=memory;
//	memory+=size;
////	logs("new[]");
////	logi((number)current);
//	return current;
//}
//void* operator new(size_t size){ // stack
//	current=memory;
//	memory+=size;
////	logs("new");
////	logi((number)current);
//	return current;
//}

// WHY NOT WORKING WHEN IMPORTED? FUCKING MANGLING!
// bus error == access out of scope, e.g. logc((void*)-1000)
void log(char* s) {
#ifdef WASM
	while(*s)logc(*s++);
#else
	printf("%s\n", s);
#endif
}

//void log(chars s) {
//#ifdef WASM
//	while(*s)logc(*s++);
//#else
//	printf("%s\n", s);
//#endif
//}


void _cxa_allocate_exception(){
 log("_cxa_allocate_exception!");
}

void _cxa_throw(){
	log("_cxa_throw");
  error("OUT OF MEMORY");
}

#else // NOT WASM:

#include <zconf.h>
//#import "Backtrace.cpp"
#include "ErrorHandler.h"

#ifndef __APPLE__
#include <alloc.h>
#endif

#include <zconf.h>
#include <cstdio>

//NEEDED, else terminate called without an active exception

void err(chars error) {
#ifdef Backtrace
	Backtrace(3);
#endif
	throw error;
}

void error(chars error) {
	err(error);
}

void warn(chars warning) {
	printf("%s\n", warning);
}

void warning(chars warning) {
	printf("%s\n", warning);
}

// #include <cstdlib> // alloc too
//#include <stdio.h> // print
#endif

#ifdef WASM64
void* operator new[](unsigned number size){
	memory+=size;
	return memory;
}
void* operator new(unsigned number size){
	memory+=size;
	return memory;
}
#endif

#ifndef WASM
#import "Fetch.cpp"
//#include "Fetch.h"
#endif

#include "Node.h"
#include "String.h"

String text = EMPTY;
// raise defined in signal.h :(


class Mark {

	int at = -1;//{};            // The index of the current character PLUS ONE todo
	char previous = 0;
	char ch = 0;            // The current character
	char next = 0;
	int lineNumber{};    // The current line number
	int columnStart{};    // The index of column start char

//	Node obj = Node();
//	obj.

public:

//	Mark(){}
//	Mark(const Node &obj);
//	Mark(String source) {
//		this->text = source;
//	}

// Return the enclosed parse function. It will have access to all of the above functions and variables.
//    Node return_fuck(auto source,auto options) {
	static Node parse(String source) {
		printf("Parsing %s\n", source.data);
		return Mark().read(source);
	}

	// see 'apply' for operator eval
	static Node eval(String source) { // return by value ok, rarely used and stable
		return Mark().read(source).evaluate();
	}


	Node read(String source) {
		if (source.empty()) return NIL;
		columnStart = at = 0;
		lineNumber = 1;
		ch = ' ';
		text = String(source);
//		if (typeof options == "object" && options.format && options.format != "wasp")  // parse as other formats
//			return $convert.parse(source, options);
		var result = value(); // <<
		white();

//			while(result.type==reference)

		if (ch && ch != -1) {
			breakpoint_helper
			error("Expect end of input");
			result = ERROR;
		}
		// Mark does not support the legacy JSON reviver function todo ??
		return result;
	}


	static char *readFile(const char *filename) {
#ifndef WASM
		FILE *f = fopen(filename, "rt");
		if (!f)err("FILE NOT FOUND "_s + filename);
		fseek(f, 0, SEEK_END);
		long fsize = ftell(f);
		fseek(f, 0, SEEK_SET);  /* same as rewind(f); */
		char *s = (char *) (alloc(fsize , 2));
		fread(s, 1, fsize, f);
		fclose(f);
		return s;
#else
		return 0;
#endif
	}

	static Node parseFile(const char *filename) {
//		const char filename=replace(filename0,"~","/Users/me")
		return Mark::parse(readFile(filename));
	}

private:
	char escapee(char c) {
		switch (c) {
			case '"' :
				return '"';        // this is needed as we allows single quote
			case '\\':
				return '\\';
			case '/':
				return '/';
			case '\n':
				return '\n';       // Replace escaped newlines in strings w/ empty string
			case 'b':
				return '\b';
			case 'f':
				return '\f';
			case 'n':
				return '\n';
			case 'r':
				return '\r';
			case 't':
				return '\t';
			default:
				return 0;
		}
	};

	String renderChar(char chr) {
		return chr == '\n' ? s("\\n") : String('\'') + chr + '\'';
	};

	String backtrace2() {
#ifdef Backtrace
		return Backtrace(3);// skip to Mark::error()
#else
		return "no Backtrace compiled in";
#endif
//		String stack;
//		void *array[10];
//		size_t size;
//		size = backtrace(array, 10);
//		backtrace_symbols_fd(array, size, 2 /*STDERR_FILENO*/);
		// wasp(+0x649e)[0x55715b27e49e] hmmm
//		Compiling with -g -rdynamic gets you symbol info in your output, which glibc can use to make a nice stacktrace
	};

	String error(String m) {
		// Call error when something is wrong.
		// todo: Still to read can scan to end of line
		var columnNumber = at - columnStart;
		var msg = s("");
//				s("IN CODE:\n");
		msg = msg + m;
		msg = msg + " in line " + lineNumber + " column " + columnNumber + "\n";
		msg = msg + text + "\n";
		msg = msg + (s(" ").times(at - 1)) + "^\n";
//		msg = msg + s(" of the Mark data. \nStill to read: \n") + text.substring(at - 1, at + 30) + "\n^^ ...";
		msg = msg + backtrace2();
		var error = new SyntaxError(msg);
//		error.at = at;
//		error.lineNumber = lineNumber;
//		error.columnNumber = columnNumber;
		if (throwing)
			throw error;
		else
			return msg;
		throw msg;//error;
	};

	String s(const char string[]) {
		return {string};
	}

	char back() {
		previous = ch;
		ch = text.charAt(--at);
		at--;
		return ch;
	}

	char proceed(char c = 0) {
		// If a c parameter is provided, verify that it matches the current character.
		if (c && c != ch) {
			error(s("Expected '") + c + "' instead of " + renderChar(ch));
		}
		if (at >= text.length) {
			ch = 0;
			return -1;
		}
		// Get the next character. When there are no more characters, return the empty string.
		previous = ch;
		ch = text.charAt(at);
		if (at + 1 >= text.length)next = 0;
		else next = text.charAt(at + 1);
		at++;
		if (ch == '\n' || (ch == '\r' && next != '\n')) {
			lineNumber++;
			columnStart = at;
		}
		return ch;
	};

	bool is_bracket(char ch) {
		return ch == '(' or ch == ')' or ch == '[' or ch == ']' or ch == '{' or ch == '}';
	}

	// everything that is not an is_identifier is treated as operator/symbol/identifier?
	bool is_operator(char ch) {// todo is_KNOWN_operator
//		if (ch > 0x207C and ch < 0x2200) return true; char is signed_byte -127..127
		if (ch < 0) return true;// utf
		if (is_identifier(ch)) return false;
		if (isalnum(ch)) return false;// ANY UTF 8
		return ch > ' ' and ch != ';' and !is_bracket(ch) and ch != '\'' and ch != '"';
	}

	bool is_identifier(char ch) {
		if (ch == '#')return false;// size/count/length
		if (ch == '=')return false;
		if (ch == ' ')return false;
		if (ch == ';')return false;
		if (ch == '.')return false;
//		if (ch == '-')return false;// todo
		if (ch == "＝"[0] and next == "＝"[1])return false;// todo … !?!?
		if (ch == "√"[0] and next == "√"[1])return false;// todo … !?!?
		if (ch == "≠"[0] and next == "≠"[1])return false;// todo … !?!?
		return ('a' <= ch and ch <= 'z') or ('A' <= ch and ch <= 'Z') or ch == '_' or ch == '$' or
		       ch < 0;// ch<0: UNICODE
//		not((ch != '_' && ch != '$') && (ch < 'a' || ch > 'z') && (ch < 'A' || ch > 'Z'));
	};

	// Parse an identifier.
	String identifier() {
		// identifiers must start with a letter, _ or $.
		if (!is_identifier(ch)) {
			breakpoint_helper
			error(UNEXPECT_CHAR + renderChar(ch));
		}

		// To keep it simple, Mark identifiers do not support Unicode "letters", as in JS; if needed, use quoted syntax
		var key = String(ch);
		// subsequent characters can contain ANYTHING
		while (proceed() and is_identifier(ch))key += ch;
		// subsequent characters can contain digits
//		while (proceed() &&
//		       (('a' <= ch && ch <= 'z') || ('A' <= ch && ch <= 'Z') || ('0' <= ch && ch <= '9') || ch == '_' ||
//		        ch == '$' || ch < 0 /* UTF */ || ch == '.' || ch == '-')) {
//			 '.' and '-' are commonly used in html and xml names, but not valid JS name chars
//			key += ch;
//		}
		key += '\0';// 0x00;
		return key;
	};

	// Parse a number value.
	Node numbers() {
		let sign = '\n';
		let string = String("");
		int number0, base = 10;

		if (ch == '-' || ch == '+') {
			sign = ch;
			proceed(ch);
		}

		// support for Infinity (could tweak to allow other words):
		if (ch == 'I') {
			const Node &ok = word();
//			if(!ok)
			return Infinity;
//			return (sign == '-') ? -Infinity : Infinity;
		}

		// support for NaN
		if (ch == 'N') {
			const Node &ok = word();
//			bool ok = word();
//			if (!ok) { error('expected word to be NaN'); }
			// ignore sign as -NaN also is NaN
			return NaN;
		}

		if (ch == '0') {
			string += ch;
			proceed();
		} else {
			while (ch >= '0' && ch <= '9') {
				string += ch;
				proceed();
			}
			if (ch == '.') {
				string += '.';
				while (proceed() && ch >= '0' && ch <= '9') {
					string += ch;
				}
			}
			if (ch == 'e' || ch == 'E') {
				string += ch;
				proceed();
				if (ch == '-' || ch == '+') {
					string += ch;
					proceed();
				}
				while (ch >= '0' && ch <= '9') {
					string += ch;
					proceed();
				}
			}
		}

		if (string.contains("."))
			return Node(atof0(string.data));
		if (sign == '-') {
			number0 = -atoi0(string.data);
		} else {
			number0 = +atoi0(string.data);
		}

//		if (!isFinite(number)) {
//			error("Bad number");
//		} else {
		return Node(number0); // {number0}; //wow cPP
//		}
	};

	int parseInt(char next, int base) {
		return next - '0';
	}

	String fromCharCode(long uffff) {// todo UTF
		return String((char) (uffff));// itoa0(uffff);
	}

	Node string(char delim = '"') {
		int start = at;
		proceed();
		while (ch and ch != delim and previous != '\\')
			proceed();
		const String &substring = text.substring(start, at - 1);
		proceed();
		return Node(substring).setType(strings);// DONT do "3"==3 (here or ever)!
	}

// Parse a string value.
	Node string2(char delim = '"') {
		var hex = 0;
		var i = 0;
		var triple = false;
		var start = at;
		String string;

		// when parsing for string values, we must look for ' or " and \ characters.
		if (ch == '"' || ch == '\'') {
			delim = ch;
			if (next == delim && text[at + 1] == delim) { // got tripple quote
				triple = true;
				proceed();
				proceed();
			}
			while (proceed()) {
				if (ch == delim) {
					proceed();
					if (!triple) { // end of string
						return Node(string);
						return Node(text.substring(start, at - 2));
					} else if (ch == delim && next == delim) { // end of tripple quoted text
						proceed();
						proceed();
						return Node(text.substring(start, at - 2));
//						todo: escape
					} else {
						string += delim;
					}
					// continue
				}
				if (ch == '\\') { // escape sequence
					if (triple) { string += '\\'; } // treated as normal char
					else { // escape sequence
						proceed();
						if (ch == 'u') { // unicode escape sequence
							long uffff = 0; // unicode
							for (i = 0; i < 4; i += 1) {
								hex = parseInt(proceed(), 16);
//								if (!isFinite(hex)) { break; }
								uffff = uffff * 16 + hex;
							}
							string = string + fromCharCode(uffff);
						} else if (ch == '\r') { // ignore the line-end, as defined in ES5
							if (next == '\n') {
								proceed();
							}
						} else if (escapee(ch)) {
							string += escapee(ch);
						} else {
							break;  // bad escape
						}
					}
				}
					// else if (ch == '\n') {
					// control characters like TAB and LF are invalid in JSON, but valid in Mark;
					// break;
					// }
				else { // normal char
					string += ch;
				}
			}
		}
		error("Bad string");
		return NIL;
	};

	// Parse an inline comment
	void inlineComment() {
		// Skip an inline comment, assuming this is one. The current character should
		// be the second / character in the // pair that begins this inline comment.
		// To finish the inline comment, we look for a newline or the end of the text.
		if (ch != '/') {
			error("Not an inline comment");
		}
		do {
			proceed();
			if (ch == '\n' || ch == '\r') {
				proceed();
				return;
			}
		} while (ch);
	};

	// Parse a block comment
	void blockComment() {
		// Skip a block comment, assuming this is one. The current character should be
		// the * character in the /* pair that begins this block comment.
		// To finish the block comment, we look for an ending */ pair of characters,
		// but we also watch for the end of text before the comment is terminated.
		if (ch != '*') {
			error("Not a block comment");
		}
		do {
			proceed();
			while (ch == '*') {
				proceed('*');
				if (ch == '/') {
					proceed('/');
					return;
				}
			}
		} while (ch);

		error("Unterminated block comment");
	};

	// Parse a comment
	void comment() {
		// Skip a comment, whether inline or block-level, assuming this is one.
		// Comments always begin with a / character.
		if (ch != '/') {
			error("Not a comment");
		}
		proceed('/');
		if (ch == '/') {
			inlineComment();
		} else if (ch == '*') {
			blockComment();
		} else {
			error("Unrecognized comment");
		}
	};

	// Parse whitespace and comments.
	void white() {
		// Note that we're detecting comments by only a single / character.
		// This works since regular expressions are not valid JSON(5), but this will
		// break if there are other valid values that begin with a / character!
		while (ch) {
			if (ch == '/') {
				comment();
//				auto ws = {' ', '\t', '\r', '\n'};
			} else if (ch == ' ' || ch == '\t' || ch == '\r' || ch == '\n') {
				proceed();
			} else {
				return;
			}
		}
	};

	bool isNameStart(char i) {
		return i > 'a' && i < 'Z';
	}

	bool token(String token) {
		return ch == token[0] and suffix(++token);
	}

	bool suffix(String suffix) {
		let len = suffix.length;
		for (let i = 0; i < len; i++) {
			if (text[at + i] != suffix[i]) {
				at += i + 1;
				return false;
			}
		}
		if (isNameStart(text[at + len])) {
			at += len + 1;
			return false;
		}
		ch = text[at + len];
		at += len + 1;
		return true;
	};

	Node builtin_operator() {
		Node node = Node(ch);
		node.setType(operators);// todo ++
		proceed();
		while (ch < 0 or is_operator(ch)) {// utf8 √ …
			node.name += ch;
			proceed();
		}
		return node;
	}

	Node resolve(Node node) {
		if (node.name == "false")return False;
		if (node.name == "False")return False;
		if (node.name == "no")return False;
		if (node.name == "No")return False;
		if (node.name == "ƒ")return False;// ‽
		if (node.name == "𐄂")return False;// ‽

		//		if (node.name == "wrong")return False;
//		if (node.name == "Wrong")return False;
		if (node.name == "true")return True;
		if (node.name == "True")return True;
		if (node.name == "yes")return True;
		if (node.name == "Yes")return True;
		if (node.name == "✔\uefb88f")return True;// green
		if (node.name == "✔")return True;
		if (node.name == "🗸")return True;
		if (node.name == "✓️")return True;
		if (node.name == "✓")return True;
		if (node.name == "☑")return True;
		if (node.name == "🗹")return True;
//		if (node.name == "Right")return True;// unless class!
//		if (node.name == "right")return True;
		if (node.name == "NIL")return NIL;
		if (node.name == "null")return NIL;
		if (node.name == "nill")return NIL;
		if (node.name == "nil")return NIL;
		if (node.name == "ø")return NIL;// nil not added to lists
		return node;
	}

	Node symbol() {
		if (ch >= '0' && ch <= '9')return numbers();
		if (is_identifier(ch)) return resolve(Node(identifier(), true));// or op
		if (is_operator(ch))return builtin_operator();
		breakpoint_helper
		error(UNEXPECT_CHAR + renderChar(ch));
		return NIL;
	}

//	// {a:1 b:2} vs { x = add 1 2 }
	bool lookahead_ambiguity() {
		int braces = 0;
		int pos = at - 1;
		while (pos < text.length and text[pos] != 0 and text[pos] != '\n' and braces >= 0) {
			if (text[pos] == '{')braces++;
			if (text[pos] == ',' and braces == 0)return true;// ambiguity
			if (text[pos] == ':' and braces == 0)return true;// ambiguity
			if (text[pos] == ';' and braces == 0)return true;// end of statement!
			if (text[pos] == '=' and braces == 0)
				return text[pos + 1] != '=' and text[pos - 1] != '=' and text[pos - 1] != '!';// == != OK
			if (text[pos] == '}')braces--;
			pos++;
		}
		return false;// OK, no ambiguity
	}

	Node expression() {
		Node node = symbol();
		if (lookahead_ambiguity())
			return node;
		// {a:1 b:2} vs { x = add 1 2 }
		Node expressions = Node();
		expressions.type = Type::expression;
		expressions.add(node);
		white();
		while ((ch and is_identifier(ch)) or isalnum(ch) or is_operator(ch)) {
			node = symbol();
			expressions.add(node);
			white();
		}
//		expressions.name=map(children.name)
		if (expressions.length > 1)
			return expressions;
		else return node;
	}

	Node words() {
		bool allow_unknown_words = true;// todo depending on context
		if (allow_unknown_words)
			return expression();
		return ch >= '0' && ch <= '9' ? numbers() : word();
	}

	// Parse true, false, null, Infinity, NaN
	Node word() {
		switch (ch) {
			case 't':
				if (token("true")) { return True; }
				break;
			case 'T':
				if (token("True")) { return True; }
				break;
			case 'f':
				if (token("false")) { return False; }
				break;
			case 'F':
				if (token("False")) { return False; }
				break;
			case 'n':
				if (token("nil")) { return False; }
				if (token("nill")) { return False; }
				if (token("null")) { return False; }
				if (token("none")) { return False; }
				break;
			case 'N':
				if (token("NaN")) { return NaN; }
				if (token("Nil")) { return False; }
				if (token("Nill")) { return False; }
				if (token("Null")) { return False; }
				if (token("None")) { return False; }
				break;
			case 'I':
				if (token("Infinity")) { return Infinity; }
				break;
		}
		if (token("one")) { return True; }
		if (token("two")) { return Node(2); }
		breakpoint_helper
		const String &message = UNEXPECT_CHAR + renderChar(text.charAt(at - 1));
//		throw message;
		error(message);
	};

	Node pragma2(char prag = '\n') {// sende in wasp??
		let level = 0;
		proceed();  // skip starting '('
		while (ch) {
			if (ch == ')') {
				if (level) { level--; } // embedded (...)
				else { // end of pragma
					proceed();  // skip ending ')'
					return pragma2(prag);
				}
			} else if (ch == '(') { level++; } // store as normal char
			// else - normal char
			prag += ch;
			proceed();
		}
		breakpoint_helper
		error(UNEXPECT_END);
	};

//	value,  // Place holder for the value function.

	// Parse an array
	Node array() {
//		arr.type = arrays;
		auto *array0 = static_cast<Node *>(alloc(sizeof(Node *) , 100));// todo: GROW!
//		arr.value.node = array0;
		int len = 0;
		proceed();  // skip the starting '['
		white();
		while (ch) {
			if (ch == ']') {
				proceed();
				Node arr = Node(&array0);
				arr.length = len;
				return arr;   // Potentially empty array
			}
				// ES5 and Mark allow omitted elements in arrays, e.g. [,] and [,null]. JSON don't allow this.
//			if (ch == ',') {
//				error("Missing array element");
//			}
			else {
				Node val = value();
				array0[len++] = val;
//				array0.push(value());
			}
			white();

			// comma is optional in Mark
			if (ch == ',') { //  or ch == ';' not in list
				proceed();
				white();
			}
		}
		error("Expecting ]");
	};

	char charCodeAt(int base64) {
		return 'x';
	}

	// {: 00aacc :} base64 values todo: USE
	Node binary() {
// Parse binary value
// Use a lookup table to find the index.
		byte lookup64[128];
		byte lookup85[128];

		for (var i = 0; i < 128; i++) {
			lookup64[i] = 65;
			lookup85[i] = 86;
		}
		for (var i = 0; i < 64; i++) {
			char charCode = text.charCodeAt(i);
			if (charCode < 0) // never true: charCode > 128 or
				err(("Invalid binary charCode %d "_s % (long) charCode) + text.substring(i, i + 2) + "\n" + text);
			lookup64[charCode] = i;
		}
// ' ', \t', '\r', '\n' spaces also allowed in base64 stream
		lookup64[32] = lookup64[9] = lookup64[13] = lookup64[10] = 64;
		for (var i = 0; i < 128; i++) { if (33 <= i && i <= 117) lookup85[i] = i - 33; }
// ' ', \t', '\r', '\n' spaces also allowed in base85 stream
		lookup85[32] = lookup85[9] = lookup85[13] = lookup85[10] = 85;


		at++;  // skip the starting '{:'
		if (next == '~') { // base85
			at++;  // skip '~'
			// code based on https://github.com/noseglid/base85/blob/master/lib/base85.js
			let end = text.indexOf('}', at + 1);  // scan binary end
			if (end < 0) { error("Missing ascii85 end delimiter"); }

			// first run decodes into base85 int values, and skip the spaces
			let p = 0;
			byte base[end - at + 3];  // 3 extra bytes of padding
			while (at < end) {
				let code = lookup85[text.charCodeAt(at)];  // console.log('bin: ', next, code);
				if (code > 85) { error("Invalid ascii85 character"); }
				if (code < 85) { base[p++] = code; }
				// else skip spaces
				at++;
			}
			at = end + 2;
			proceed();  // skip '~}'
			// check length
			if (p % 5 == 1) { error("Invalid ascii85 stream length"); }

			// second run decodes into actual binary data
			let dataLength = p, padding = (dataLength % 5 == 0) ? 0 : 5 - dataLength % 5;
			int buffer[4 * dataLength / 5 - padding];
//				bytes = new DataView(buffer),
			int *bytes = buffer;// views:
			auto *bytes8 = reinterpret_cast<byte *>(buffer);
			auto *bytes16 = reinterpret_cast<short *>(buffer);
			int trail = dataLength - 4;//buffer.byteLength - 4;
			base[p] = base[p + 1] = base[p + 2] = 84;  // 3 extra bytes of padding
			// console.log('base85 byte length: ', buffer.byteLength);
			for (let i = 0, p = 0; i < dataLength; i += 5, p += 4) {
				let num = (((base[i] * 85 + base[i + 1]) * 85 + base[i + 2]) * 85 + base[i + 3]) * 85 + base[i + 4];
				// console.log("set byte to val:", p, num, String.fromCodePoint(num >> 24), String.fromCodePoint((num >> 16) & 0xff),
				//	String.fromCodePoint((num >> 8) & 0xff), String.fromCodePoint(num & 0xff));
				// write the uint32 value
				if (p <= trail) { // bulk of bytes
					bytes[p] = num; // big endian
				} else { // trailing bytes
					switch (padding) {
						case 1:
							bytes8[p + 2] = (num >> 8) & 0xff;  // fall through
						case 2:
							bytes16[p] = num >> 16;
							break;
						case 3:
							bytes8[p] = num >> 24;
					}
				}
			}
			return Node(buffer);// {buffer};
		} else { // base64
			// code based on https://github.com/niklasvh/base64-arraybuffer
			let end = text.indexOf('}', at), bufEnd = end, pad = 0;  // scan binary end
			if (end < 0) { error("Missing base64 end delimiter"); }
			// strip optional padding
			if (text[bufEnd - 1] == '=') { // 1st padding
				bufEnd--;
				pad = 1;
				if (text[bufEnd - 1] == '=') { // 2nd padding
					bufEnd--;
					pad = 2;
				}
			}
			// console.log('binary char length: ', bufEnd - at);

			// first run decodes into base64 int values, and skip the spaces
			byte base[bufEnd - at];
			let p = 0;
			while (at < bufEnd) {
				let code = lookup64[text.charCodeAt(at)];  // console.log('bin: ', next, code);
				if (code > 64) { error("Invalid base64 character"); }
				if (code < 64) { base[p++] = code; }
				// else skip spaces
				at++;
			}
			at = end + 1;
			proceed();  // skip '}'
			// check length
			if ((pad && (p + pad) % 4 != 0) || (!pad && p % 4 == 1)) {
				error("Invalid base64 stream length");
			}

			// second run decodes into actual binary data
			let len = int(p * 0.75);
			int code1, code2, code3, code4 = 0;
			int buffer[len];
			auto *bytes = reinterpret_cast<byte *>(buffer);// views:
			// console.log('binary length: ', len);
			for (let i = 0, p = 0; p < len; i += 4) {
				code1 = base[i];
				code2 = base[i + 1];
				code3 = base[i + 2];
				code4 = base[i + 3];
				bytes[p++] = (code1 << 2) | (code2 >> 4);
				// extra undefined bytes casted into 0 by JS binary operator
				bytes[p++] = ((code2 & 15) << 4) | (code3 >> 2);
				bytes[p++] = ((code3 & 3) << 6) | (code4 & 63);
			}
			// console.log('binary decoded length:', p);
//			buffer.encoding = "b64";
			return Node(buffer);
		}
	};

	bool isDigit(char next) {
		return next >= '0' and next <= '9';
	}

	Node &setField(Node &key) { // a:{b}
		Node &val = *value(' ', &key).clone();// applies to WHOLE expression
		if ((val.type == groups or val.type == patterns or val.type == objects) and val.length == 1 and
		    val.name.empty())
			val = val.last();// singleton
		if (val.value.numbery and val.type != objects and val.name.empty()) {
			if (&key == &NIL or key.isNil() or key == NIL or val.value.floaty == 6.4807)
				if (key.name == nil_name)
					warn("impossible");
			key.value = val.value;// direct copy value SURE?? what about meta data... ?
			key.type = val.type;
		} else {
			key.type = keyNode;
			if (!key.children and val.name.empty() and val.length > 1) { // deep copy why?
				key.children = val.children;
				key.length = val.length;
				key.type = val.type;
			} else if (!val.isNil())
				key.value.node = &val;// clone?
		}
		return key;
	}

// ":" is short binding a b:c d == a (b:c) d
// "=" is number-binding a b=c d == (a b)=(c d)   todo a=b c=d
// special : close=' ' : single value in a list {a:1 b:2} ≠ {a:(1 b:2)} BUT a=1,2,3 == a=(1 2 3)
// special : close=';' : single expression a = 1 + 2
// significant whitespace a {} == a,{}{}
// todo a:[1,2] ≠ a[1,2] but a{x}=a:{x}? OR better a{x}=a({x}) !? but html{...}
	Node value(char close = 0, Node *parent = 0) {
		// A JSON value could be an object, an array, a string, a number, or a word.
		Node list;
		Node current;
		current.parent = parent;
		current.setType(groups);// may be changed later, default (1 2)==1,2
		var length = text.length;
		int start = at;
		loop:
		proceed();
		white();
		while (ch and at <= length) {
//			white();// sets ch todo 1+1 != 1 +1
			if (previous == '\\') {
				proceed();
				continue;
			}// escape ANYTHING
			if (ch == close or ((close == ' ' or close == ',') and
			                    (ch == ';' or ch == ',' or ch == '\n'))) {// todo: a=1,2,3
				proceed();
				break;
			}// inner match ok
			if (ch == '}' or ch == ']' or ch == ')') { // todo: ERROR if not opened before!
				if (ch != close) // cant debug wth?
					if (!current.parent)
						return ERROR;// throwing? error("NOT MATCHING" : ERROR);
//				break;
				return current;
			}// outer match unresolved so far
			switch (ch) {
				case '=':
				case ':': {
					// todo {a b c:d} vs {a:b c:d}
					Node &key = current.last();
					setField(key);
					break;
				}
				case '{': {
					Node &object = value('}').setType(Type::objects);
					current.last().add(object);
					break;
				}
				case '[': {
					Node &pattern = value(']').setType(Type::patterns);
					current.add(pattern);
					break;
				}
				case '(': {
					Node &group = value(')').setType(Type::groups);
					current.last().add(group);
					break;
				}// lists handled by ' '!
				case '}':
				case ')':
				case ']':
//					break loop;// not in c++
					error("wrong closing bracket");
				case '-':
				case '+':
				case '.':
					if (isDigit(next))
						return numbers();
					else
						return words();
				case '/':
					if (next == '/' or next == '*') {
						comment();
						break;
					}
				case '"':
				case '\'':
				case '`': {
					if (previous == '\\')continue;// escape
					if (close != ch) {
						// open string
						current.add(string(ch));
						break;
					}
					Node id = Node(text.substring(start, at));
					id.setType(Type::strings);// todo "3" could have be resolved as number? DONT do js magifuckery
					current.add(id);
					break;
				}
				case '\t':
				case '\n':
				case ';':
				case ',':
				case ' ': {
					if (previous == ',' or previous == close or close == '"' or close == '\'' or close == '`') {
						proceed();
						continue;
					}
					const Node &sub = value(ch, &current);
					current.add(sub);// space is list operator
					break;
				}
				default: {
					Node node = words();
					if(polish_notation && current.length == 0)
						current=node;
					else
						current.add(node);
				}
			}
		}
		bool keepBlock = close == '}';
		if (current.length == 0 and current.param) {
			current.children = current.param->children;
			current.length = current.param->length;
			current.param = nullptr;; // delete after copy to current.param . todo: what about extra meta info?
			current.type = groups;// todo: lose type if unbound?
		}
		if (current.length == 1) {
//			if (current.value.node == &current.children[0])return *current.value.node;
//			if (current.value.numbery and current.children[0].value.numbery)
//				if (current.value.node->value.numbery != current.children[0].value.numbery)
//					error("ambiguous values");
//			if (current.value.numbery)
//				current.children[0].value = current.value;
			return current.children[0];
		}
		return current;
	};
//	int $parent{};
};

void ok() {
	error("WHAAA");
}

#ifdef BACKTRACE
void handler(int sig) {
	void *array[10];
	size_t size;

	// get void*'s for all entries on the stack
	size = backtrace(array, 10);

	// print out all the frames to stderr
	fprintf(stderr, "Error: signal %d:\n", sig);
	backtrace_symbols_fd(array, size, STDERR_FILENO);
	exit(1);
}
#endif

struct TTT {
	int x;
	chars y;
	char wtf[1000];
};


//void assert_is(char *wasp, Node result);

void init() {
	NIL.type = nils;
	NIL.value.numbery = 0;
	False.type = bools;
	False.value.numbery = 0;
	True.type = bools;
	True.value.numbery = 1;
}

#import "tests.cpp"

static Node parse(String source) {
	printf("Parsing %s\n", source.data);
	return Mark().read(source);
}

// Mark/wasp has clean syntax with FULLY-TYPED data model (like JSON or even better)
// Mark/wasp is generic and EXTENSIBLE (like XML or even better)
// Mark/wasp has built-in MIXED CONTENT support (like HTML5 or even better)
// Mark/wasp supports HIGH-ORDER COMPOSITION (like S-expressions or even better)



class String;
void print(String s){
	log(s.data);
}

//char* operator "" _ss(const char *c, unsigned number t) {// function signature contains illegal type WHYY??
//	return "NOO";
//}
String& operator "" _ss(const char *c, unsigned long t) {// function signature contains illegal type WHYY??
	String string;
	String *pString = &string;
//	String *pString = new String("NOO");
	return *pString;
}

//wasm-ld: error: wasp.o: undefined symbol: vtable for __cxxabiv1::__class_type_info

// 2020: WASI does not yet support C++ exceptions. C++ code is supported only with -fno-exceptions for now.
// https://github.com/WebAssembly/wasi-sdk
//struct ERR{
//public:
//	ERR() = default;
//	virtual ~ERR() = default;
//};
char newline = '\n';
#ifndef _main_
int main(int argp, char **argv) {

#ifdef register_global_signal_exception_handler
	register_global_signal_exception_handler();
#endif

	try {
//		String args=String((char*)0);// RuntimeError: unreachable
#ifdef WASM
		String args;// hack: written to by wasmx
		args.data[0] = '{';
		log(args);
		current += strlen(args);
#endif
//		logi(reinterpret_cast<long>(args.data));// 0
//		logi(reinterpret_cast<long>(memory));// 4096
		log("Hello");
		log("Hello "_s+"WASM"+"!!!");
		log("OK format %d"_s.format(123));
		log("OK format %s???"_s.replace("%s","WASM!"));
		Node node = Node("123");// type mismatch in implicit return, expected [i32] but got [] FUUUCK
		log(node);
//		if(1>0)
//		raise("test_error");
//		tests();
//		testCurrent();
		return 43;
	} catch (chars err) {
		printf("\nERROR\n");
		printf("%s", err);
	} catch (String err) {
		printf("\nERROR\n");
		printf("%s", err.data);
	} catch (SyntaxError *err) {
		printf("\nERROR\n");
		printf("%s", err->data);
	}
	usleep(1000000000);
	return -1;
}

#endif
