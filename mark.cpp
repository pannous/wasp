//#define WASM 1
//#define X86_64 1
#define let auto
#define var auto
typedef void *any;
typedef unsigned char byte;
typedef const char *chars;

extern "C" void raise(chars error);
extern "C" chars fetch(chars url);

bool throwing = true;// otherwise fallover beautiful-soup style generous parsing

unsigned int *memory = (unsigned int *) 4096; // todo how to not handtune _data_end?
#ifdef WASM
#ifdef X86_64
typedef long unsigned int size_t;
#else
// typedef long unsigned int size_t;
typedef unsigned int size_t;
#endif

unsigned int *current;
extern "C" void logs (const char *);
extern "C" void logc(char s);
extern "C" void logi(int i);

extern "C" void * memset ( void * ptr, int value, size_t num ){
	int* ptr0=(int*)ptr;
	for (int i = 0; i < num; i++)
		ptr0[i]=value;
	return ptr;
}
void* operator new[](size_t size){ // stack
	current=memory;
	memory+=size;
//	logs("new[]");
//	logi((long)current);
	return current;
}
void* operator new(size_t size){ // stack
	current=memory;
	memory+=size;
//	logs("new");
//	logi((long)current);
	return current;
}

void *malloc(int size) { // heap
//	logs("malloc");
//	logi((long)current);
	current = memory;//heap;
	memory += size * 2 + 1;
	return current;
}

void _cxa_allocate_exception(){
 logs("_cxa_allocate_exception");
}

void _cxa_throw(){
	logs("_cxa_throw");
  throw "OUT OF MEMORY";
}

#else // NOT WASM:

void raise(chars error) { throw error; }

#include <malloc.h>
#include <assert.h>
// #include <cstdlib> // malloc too
//#include <stdio.h> // printf
#endif

#ifdef WASM64
void* operator new[](unsigned long size){
	memory+=size;
	return memory;
}
void* operator new(unsigned long size){
	memory+=size;
	return memory;
}
#endif


#import "String.cpp" // import against mangling in wasm (vs include)

#ifndef WASM

#import "Fetch.cpp"

#endif

//#include "String.h"
void raise(String error) {
	raise(error.data);
}

class Mark {
	String text = EMPTY;
	String UNEXPECT_END = "Unexpected end of input";
	String UNEXPECT_CHAR = "Unexpected character ";

	int at{},            // The index of the current character
			lineNumber{},    // The current line number
			columnStart{};    // The index of column start char
	char ch = 0;            // The current character

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
		return Mark().read(source);
	}

	static Node eval(String source) {
		return Mark().read(source).evaluate();
	}

	Node read(String source) {
		if (source.empty()) return NIL;
		columnStart = at = 0;
		lineNumber = 1;
		ch = ' ';
		text = String(source);
//		if (typeof options == "object" && options.format && options.format != "mark")  // parse as other formats
//			return $convert.parse(source, options);
		var result = value(); // <<
		white();
		if (ch && ch != -1) error("Expect end of input");
		// Mark does not support the legacy JSON reviver function todo ??
		return result;
	}

	static char *readFile(const char *filename) {
		FILE *f = fopen(filename, "rt");
		fseek(f, 0, SEEK_END);
		long fsize = ftell(f);
		fseek(f, 0, SEEK_SET);  /* same as rewind(f); */
		char *s = static_cast<char *>(malloc(fsize + 1));
		fread(s, 1, fsize, f);
		fclose(f);
		return s;
	}

	static Node parseFile(const char *filename) {
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
		return chr == '\n' ? s("-1") : String('\'') + chr + '\'';
	};

	String error(String m) {
		// Call error when something is wrong.
		// todo: Still to read can scan to end of line
		var columnNumber = at - columnStart;
		var msg = m + "\n at line " + lineNumber + " column " + columnNumber;
		msg = msg + s(" of the Mark data. Still to read: ") + text.substring(at - 1, at + 30) + "...";
//		var error = new SyntaxError(msg);
//		error.at = at;
//		error.lineNumber = lineNumber;
//		error.columnNumber = columnNumber;
		throw msg;//error;
		return m;
	};

	String s(const char string[]) {
		return {string};
	}

	char next(char c = 0) {
		// If a c parameter is provided, verify that it matches the current character.
		if (c && c != ch) {
			error(s("Expected '") + c + "' instead of " + renderChar(ch));
		}
		if (at >= text.length) {
			ch = 0;
			return -1;
		}
		// Get the next character. When there are no more characters, return the empty string.
		ch = text.charAt(at);
		at++;
		if (ch == '\n' || (ch == '\r' && text[at] != '\n')) {
			lineNumber++;
			columnStart = at;
		}
		return ch;
	};

	// Parse an identifier.
	String identifier() {
		// To keep it simple, Mark identifiers do not support Unicode "letters", as in JS; if needed, use quoted syntax
		var key = String(ch);

		// identifiers must start with a letter, _ or $.
		if ((ch != '_' && ch != '$') && (ch < 'a' || ch > 'z') && (ch < 'A' || ch > 'Z')) {
			error(UNEXPECT_CHAR + renderChar(ch));
		}
		// subsequent characters can contain digits
		while (next() &&
		       (('a' <= ch && ch <= 'z') || ('A' <= ch && ch <= 'Z') || ('0' <= ch && ch <= '9') || ch == '_' ||
		        ch == '$' ||
		        // '.' and '-' are commonly used in html and xml names, but not valid JS name chars
		        ch == '.' || ch == '-')) {
			key += ch;
		}
		key += 0x00;
		return key;
	};

	// Parse a number value.
	Node number() {
		let sign = '\n';
		let string = String("");
		int number0, base = 10;

		if (ch == '-' || ch == '+') {
			sign = ch;
			next(ch);
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
			next();
		} else {
			while (ch >= '0' && ch <= '9') {
				string += ch;
				next();
			}
			if (ch == '.') {
				string += '.';
				while (next() && ch >= '0' && ch <= '9') {
					string += ch;
				}
			}
			if (ch == 'e' || ch == 'E') {
				string += ch;
				next();
				if (ch == '-' || ch == '+') {
					string += ch;
					next();
				}
				while (ch >= '0' && ch <= '9') {
					string += ch;
					next();
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

	String fromCharCode(long uffff) {
		return (char) (uffff);// itoa(uffff);
	}

// Parse a string value.
	Node string() {
		var hex = 0;
		var i = 0;
		var string = String();
		var triple = false;
		var delim = '"';      // double quote or single quote
		long uffff;

		// when parsing for string values, we must look for ' or " and \ characters.
		if (ch == '"' || ch == '\'') {
			delim = ch;
			if (text[at] == delim && text[at + 1] == delim) { // got tripple quote
				triple = true;
				next();
				next();
			}
			while (next()) {
				if (ch == delim) {
					next();
					if (!triple) { // end of string
						return Node(string);
					} else if (ch == delim && text[at] == delim) { // end of tripple quoted text
						next();
						next();
						return Node(string);
					} else {
						string += delim;
					}
					// continue
				}
				if (ch == '\\') { // escape sequence
					if (triple) { string += '\\'; } // treated as normal char
					else { // escape sequence
						next();
						if (ch == 'u') { // unicode escape sequence
							uffff = 0;
							for (i = 0; i < 4; i += 1) {
								hex = parseInt(next(), 16);
//								if (!isFinite(hex)) { break; }
								uffff = uffff * 16 + hex;
							}
							string = string + fromCharCode(uffff);
						} else if (ch == '\r') { // ignore the line-end, as defined in ES5
							if (text[at] == '\n') {
								next();
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
			next();
			if (ch == '\n' || ch == '\r') {
				next();
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
			next();
			while (ch == '*') {
				next('*');
				if (ch == '/') {
					next('/');
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
		next('/');
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
				next();
			} else {
				return;
			}
		}
	};

	bool isNameStart(char i) {
		return i > 'a' && i < 'Z';
	}

	bool isSuffix(String suffix) {
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

	// Parse true, false, null, Infinity, NaN
	Node word() {
		switch (ch) {
			case 't':
				if (isSuffix("rue")) { return True; }
				break;
			case 'f':
				if (isSuffix("alse")) { return False; }
				break;
			case 'n':
				if (isSuffix("ull")) { return False; }
				break;
			case 'I':
				if (isSuffix("nfinity")) { return Infinity; }
				break;
			case 'N':
				if (isSuffix("aN")) { return NaN; }
		}
		throw error(UNEXPECT_CHAR + renderChar(text.charAt(at - 1)));
	};

	Node pragma(char prag = '\n') {
		let level = 0;
		next();  // skip starting '('
		while (ch) {
			if (ch == ')') {
				if (level) { level--; } // embedded (...)
				else { // end of pragma
					next();  // skip ending ')'
					return pragma(prag);
				}
			} else if (ch == '(') { level++; } // store as normal char
			// else - normal char
			prag += ch;
			next();
		}
		throw error(UNEXPECT_END);
	};

//	value,  // Place holder for the value function.

	// Parse an array
	Node array() {
//		arr.type = arrays;
		auto *array0 = static_cast<Node *>(malloc(100));
//		arr.value.node = array0;
		Node arr = Node(&array0);
		int len = 0;
		next();  // skip the starting '['
		white();
		while (ch) {
			if (ch == ']') {
				next();
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
				next();
				white();
			}
		}
		throw error("Expecting ]");
	};

	char charCodeAt(int base64) {
		return 'x';
	}

	// {: 00aacc :} base64 values
	Node binary() {
// Parse binary value
// Use a lookup table to find the index.
		byte lookup64[128];
		byte lookup85[128];

		for (var i = 0; i < 128; i++) {
			lookup64[i] = 65;
			lookup85[i] = 86;
		}
		for (var i = 0; i < 64; i++) { lookup64[text.charCodeAt(i)] = i; }
// ' ', \t', '\r', '\n' spaces also allowed in base64 stream
		lookup64[32] = lookup64[9] = lookup64[13] = lookup64[10] = 64;
		for (var i = 0; i < 128; i++) { if (33 <= i && i <= 117) lookup85[i] = i - 33; }
// ' ', \t', '\r', '\n' spaces also allowed in base85 stream
		lookup85[32] = lookup85[9] = lookup85[13] = lookup85[10] = 85;


		at++;  // skip the starting '{:'
		if (text[at] == '~') { // base85
			at++;  // skip '~'
			// code based on https://github.com/noseglid/base85/blob/master/lib/base85.js
			let end = text.indexOf('}', at + 1);  // scan binary end
			if (end < 0) { error("Missing ascii85 end delimiter"); }

			// first run decodes into base85 int values, and skip the spaces
			let p = 0;
			byte base[end - at + 3];  // 3 extra bytes of padding
			while (at < end) {
				let code = lookup85[text.charCodeAt(at)];  // console.log('bin: ', text[at], code);
				if (code > 85) { error("Invalid ascii85 character"); }
				if (code < 85) { base[p++] = code; }
				// else skip spaces
				at++;
			}
			at = end + 2;
			next();  // skip '~}'
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
				let code = lookup64[text.charCodeAt(at)];  // console.log('bin: ', text[at], code);
				if (code > 64) { error("Invalid base64 character"); }
				if (code < 64) { base[p++] = code; }
				// else skip spaces
				at++;
			}
			at = end + 1;
			next();  // skip '}'
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

	// Parse an object, pragma or binary
////	class Object {
//	Node obj = Node();
//	String key;        // property key
//	bool extended = false;    // whether the is extended Mark object or legacy JSON object
//	int index = 0;

	void putText(String text, Node obj) {
		obj["text"] = Node(text);
		if (!obj.value.string)
			obj.value.string = text.data;
		else
			log(obj.value.string);
//			todo("BUGy");
//		if (index > 0 && obj[index - 1]->type == strings) {
//			// merge with previous text
//			obj.set(index - 1, obj[index - 1]->string() + text);
////			obj[index - 1] = obj[index - 1].string() + text;
//		} else {
//			obj.set(index, text);
////			obj[index] = text;
//			index++;
//		}
	};

	void todo(const char string[]) {
		raise(string);
		throw string;
	}

	void parseContent(Node obj) {//context
		while (ch) {
			if (ch == '{' || ch == '(') { // child object
				let child = (ch == '(') ? pragma() : (text[at] == ':' ? binary() : object());
				obj.set(obj.length, &child);// NR as key!
			} else if (ch == '"' || ch == '\'') { // text node
				let str = string();
				// only output non empty text
//					if (str.string())
				todo("?");
				putText(str.string(), obj);
			} else if (ch == '}') {
				next();
//				obj.set($length, index);
//				obj[$length] = index;
				return;
			} else {
				error(UNEXPECT_CHAR + renderChar(ch));
			}
			white();
		}
		error(UNEXPECT_END);
	};

	bool isNaN(String key) {
		return key[0] < '0' || key[0] > '9';//todo
	}

	Node object() {
		next();
		white();  // skip the starting '{'
		String key = EMPTY;        // property key
		bool extended = false;    // whether the is extended Mark object or legacy JSON object
		int index = 0;
		Node obj;
		while (ch && ch != -1) {
			if (ch == '}') { // end of the object
				next();
				if (extended) {
					log("TODO");// TODO
//					obj.set($length, index);
//					obj[$length] = index;
				}
				return obj;   // could be empty object
			}
			// scan the key
			if (ch == '{' || ch == '(') { // child object or pragma
				if (extended) {
					parseContent(obj);
					return obj;
				}
				error(UNEXPECT_CHAR + "'{'");
			}
			if (ch == '"' || ch == '\'') { // quoted key
				var str = string();
				white();
				if (ch == ':' or ch == '=') { // property or JSON object
					key = str.string();
				} else {
					if (extended) { // already got type name
						// only output non-empty text
//						if (str)
						putText(str.string(), obj);
						parseContent(obj);
						return obj;
					} else if (key.empty()) { // at the starting of the object
						// create the object
						obj = str;// MARK(str, null, null);
						extended = true;  // key = str;
						continue;
					} else {
						error(UNEXPECT_CHAR + renderChar(ch));
					}
				}
			} else {
				// if (ch=='_' || ch=='$' || 'a'<=ch && ch<='z' || 'A'<=ch && ch<='Z')
				// Mark unquoted key, which needs to be valid JS identifier.
				var ident = identifier();
				white();
				if (ch == ':' or ch == '=') { // property value
					key = ident;
				} else {
					if (!extended) { // start of Mark object
						obj = Node(ident); //MARK(ident, null, null);
						extended = true;  // key = ident;
						continue;
					}
					error(UNEXPECT_CHAR + renderChar(ch));
				}
			}

			// key-value pair
			next(); // skip ':'
			var val = value();
			if (extended && !isNaN(key)) { // any numeric key is rejected for Mark object
				error("Numeric key not allowed as Mark property name");
			}
			Node &child = obj[key];
			if (child.length != 0) {//} && obj[key].type != "function") {
				log(child);
				child = obj[key];// debug
				error(s("Duplicate key not allowed: ") + key);
			}
//			child.value.node=&val;
//			child.type=val.type;
			child = val;// SET reference!
			child.parent = &obj;
			child.name = key;
//			obj.set(key, val);
//			obj[key] = val;
//			assert(obj[key].value == &val);
			white();
			// ',' is optional in Mark
			if (ch == ',' or ch == ';') {
				next();
				white();
			}
		}
		throw error(UNEXPECT_END);
	};

//Mark::Mark(const Node &obj) : obj(obj) {}

// Parse a JSON value.
	Node value() {
		// A JSON value could be an object, an array, a string, a number, or a word.
		white();
		switch (ch) {
			case '{':
				return (text[at] == ':' or text[at] == '=') ?
				       binary() : object();
			case '[':
				return array();
			case '"':
			case '\'':
				return string();
			case '(':
				return pragma();
			case '-':
			case '+':
			case '.':
				return number();
			default:
				return ch >= '0' && ch <= '9' ? number() : word();
		}
	};;
	int $length{};
	int $convert{};
	int $parent{};
};

void ok() {
	throw "WHAAA";
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


void init() {
	NIL.type = nils;
	True.value.longy = 1;
	True.type = bools;
	False.value.longy = 0;
	False.type = bools;
}

void testNetbase() {

	chars url = "http://de.netbase.pannous.com:8080/json/verbose/2";
	chars json = fetch(url);
	log(json);
	Node div = Mark::parse(json);
}

void testDiv() {
	Node div = Mark::parse("{div {span class:'bold' 'text'} {br}}");
	div["span"];
	div["span"]["class"];
}


void testMarkAsMap() {
	Node compare = Node();
//	compare["d"] = Node();
	compare["b"] = 3;
	compare["a"] = "HIO";
	Node &node = compare["a"];
	assert(node == "HIO");
	const char *source = "{b:3 a:'HIO'}";// d:{}
	Node marked = Mark::parse(source);
	Node &node1 = marked["a"];
	assert(node1 == "HIO");
	assert(marked["a"] == compare["a"]);
	assert(marked["b"] == compare["b"]);
	bool ok = compare == marked;
	assert(ok);
	assert(compare == marked);
}

void testMark() {
	log("primitives");
//	assert(Mark::parse("a=3") == 3);
//	assert(Mark::parse("a=3")["a"] == 3);
	Node a = Mark::parse("{a:3}");
	Node &a3 = a["a"];
	assert(a3 == 3);
	assert(a3.type == longs);
	assert(a3.name == "a");

	Node &b = a["b"];
	a["b"] = a3;
	assert(a["b"] == a3);
	assert(a["b"] == 3);

	assert(Mark::parse("3.") == 3.);
	assert(Mark::parse("3.") == 3.f);
//	assert(Mark::parse("3.1") == 3.1); // todo epsilon
//	assert(Mark::parse("3.1") == 3.1f);// todo epsilon
	assert(Mark::parse("'hi'") == "hi");
	assert(Mark::parse("3") == 3);
//		const char *source = "{a:3,b:4,c:{d:'hi'}}";
	const char *source = "{d:{} b:3 a:'HIO'}";
//		const char *source = "a='hooo'";
	Node result = Mark::parse(source);
	Node &node = result['b'];
	log("OK");
	log(result);
	log(result[0]);
	log(result[1]);
	log(result["a"]);
	log(result["a"].parent);
	log(result["b"]);
	log(result["c"]);
	log(result["a"]);
	log(result["b"]);
	log(result["c"]);
//	assert(result['d']=={})

	assert(result["b"] == 3);
	assert(result['b'] == 3);
	assert(result['a'] == "HIO");


}

void testErrors() {
	throwing = false;
	Node node = Mark::parseFile("samples/errors.mark");
	throwing = true;
}

void testSamples() {
//	Node node= Mark::parseFile("samples/comments.mark");
	Node node = Mark::parseFile("samples/kitchensink.mark");

	assert(node['a'] == "classical json");
	assert(node['b'] == "quotes optional");
	assert(node['c'] == "commas optional");
	assert(node['d'] == "semicolons optional");
	assert(node['e'] == "trailing comments"); // trailing comments
	assert(node["f"] == /*inline comments*/ "inline comments");
}

void testEval() {
	auto math = "one plus one";
	Mark::parse(math);
}

void test() {
//	testNetbase();

	testMark();
	testMarkAsMap();
	testDiv();
	testEval();
	testSamples();
	testErrors();
}

//String operator ++(const char*);


int main(int argp, char **argv) {
	auto s = "hello world"_;
	init();
	log(String("OK %s").replace("%s", "WASM"));
	try {
		test();
//		log(Node("hello"_));
	} catch (chars err) {
		printf("\nERROR\n");
		printf("%s", err);
	} catch (String err) {
		printf("\nERROR\n");
		printf("%s", err.data);
	}

	return 42;
}


