//#pragma once
#include "Node.h"
#include "Wasp.h"
#include "String.h" // variable has incomplete type
#include "Backtrace.h" // header ok in WASM
#include "wasm_helpers.h"
#include "wasm_emitter.h"
#include "wasm_runner.h"

#ifndef RUNTIME_ONLY

#include "Interpret.h"

#endif

#ifdef WASM
// NEEDS TO BE IN Wasp because __wasm_call_ctors !
//unsigned int *memory=0;// NOT USED without wasm! static_cast<unsigned int *>(malloc(1000000));
char *__heap_base = (char *) memory;
//unsigned char* __heap_base=0;
char *memoryChars = (char *) memory;
//int HEAP_OFFSET=65536/2; // todo: how done right? if too low, internal data gets corrupted ("out of bounds table access" etc)
//#define HEAP_OFFSET 65536
//int memory_size=1048576-HEAP_OFFSET; // todo set in CMake !
char *current = (char *) HEAP_OFFSET;
#endif

#ifndef WASM

#include <cctype>
#include <thread>
#include "ErrorHandler.h"

#endif

bool data_mode = true;// todo ooo! // tread '=' as ':' instead of keeping as expression operator  WHY would we keep it again??

chars operator_list0[] = {"+", "-",
                          "*", "/", ":=", "else", "then" /*pipe*/ , "is", "equal", "equals", "==", "!=", "≠", "not",
                          "|",
                          "and", "or", "&", "++", "--", "to", "xor", "be", "…", "...", "..<" /*range*/,
                          "<=", ">=", "≥", "≤", "<", ">", "less", "bigger", "⁰", "¹", "²", "³", "⁴", "×", "⋅", "⋆", "÷",
                          "^", "√", "∈", "∉", "⊂", "⊃", "in", "of",
                          "from", "#", "$", "ceil", "floor", "round", 0, 0, 0, 0}; // "while" ...
//∧  or  & and ∨ or ¬  or  ~ not → implies ⊢ entails, proves ⊨ entails, therefore ∴  ∵ because
// ⊃ superset ≡ iff  ∀ universal quantification ∃ existential  ⊤ true, tautology ⊥ false, contradiction
#ifdef WASI
List<chars> operator_list;
#else
List<chars> operator_list(operator_list0);
#endif

//	bool is_identifier(char ch) {
bool is_identifier(codepoint ch) {
	if (ch == '#')return false;// size/count/length
	if (ch == '=')return false;
	if (ch == ':')return false;
	if (ch == ' ')return false;
	if (ch == ';')return false;
	if (ch == '.')return false;
	if (ch == '-')return false;// todo
	if (ch < 0 or ch > 128)return true;// all UTF identifier todo ;)
	return ('a' <= ch and ch <= 'z') or ('A' <= ch and ch <= 'Z') or ch == '_' or ch == '$';// ch<0: UNICODE
//		not((ch != '_' and ch != '$') and (ch < 'a' or ch > 'z') and (ch < 'A' or ch > 'Z'));
};


bool is_bracket(char ch) {
	return ch == '(' or ch == ')' or ch == '[' or ch == ']' or ch == '{' or ch == '}';
}

//	List<String> operators; // reuse functions!
//	if(is_grapheme_modifier(ch))error("multi codepoint graphemes not");
// everything that is not an is_identifier is treated as operator/symbol/identifier?
// NEEDs complete codepoint, not just leading char because	☺ == e2 98 ba  √ == e2 88 9a
bool is_operator(codepoint ch) {// todo is_KNOWN_operator todo Julia
//	0x0086	134	<control>: START OF SELECTED AREA	†
	if (ch == U'∞')return false;// or can it be made as operator!?
	if (ch == U'⅓')return false;// numbers are implicit operators 3y = 3*y
	if (ch == U'∅')return false;// Explicitly because it is part of the operator range 0x2200 - 0x2319
//		0x20D0	8400	COMBINING LEFT HARPOON ABOVE	⃐
//		0x2300	8960	DIAMETER SIGN	⌀
	if (0x207C < ch and ch <= 0x208C) return true; // ⁰ … ₌
	if (0x2190 < ch and ch <= 0x21F3) return true; // ← … ⇳
	if (0x2200 < ch and ch <= 0x2319) return true; // ∀ … ⌙

	if (ch == u'√')return true;// 0x221A redundant
	if (ch == u'＝')return true;
	if (ch == u'≠')return true;
	if (ch == u'#')return true;

//		if(ch=='=') return false;// internal treatment
	if (ch > 0x80)
		return false;// utf NOT enough: ç. can still be a reference!
	if (is_identifier(ch)) return false;
	if (isalnum0(ch)) return false;// ANY UTF 8
	return ch > ' ' and ch != ';' and !is_bracket(ch) and ch != '\'' and ch != '"';
}


bool closing(char ch, char closer) {
	if (ch == closer)return true;
	if (ch == '}' or ch == ']' or ch == ')') { // todo: ERROR if not opened before!
//				if (ch != close and close != ' ' and close != '\t' /*???*/) // cant debug wth?
		return true;
	}// outer match unresolved so far
	if (precedence(ch) <= precedence(closer))
		return true;
	return false;
}

String &text = EMPTY;

class Wasp {

	int at = -1;//{};            // The index of the current character PLUS ONE todo

	char lastNonWhite = 0;
//	char previous = 0;
//	char ch = 0;            // The current character
//	char* point = 0;            // The current character
//	char next = 0; // set in proceed()

	codepoint ch = 0;            // The current character
	codepoint next = 0; // set in proceed()
	codepoint previous = 0;

	String line;
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
		printf("Parsing: %s\n", source.data);
		return Wasp().read(source);
	}

	// see 'apply' for operator eval
	static Node eval(String source) { // return by value ok, rarely used and stable
		Node parsed = parse(source);
		parsed.log();
#ifndef RUNTIME_ONLY
		return parsed.interpret();
#else
		error("RUNTIME_ONLY, no interpret!");
		return parsed;// DANGER!!
#endif
	}


	Node &read(String source) {
		if (source.empty()) return const_cast<Node &>(NIL);
		columnStart = 0;
		at = -1;
		lineNumber = 1;
		ch = 0;
		text = source;
		proceed();// at=0
		Node result = valueNode(); // <<
		white();
		if (ch and ch != -1) {
			printf("UNEXPECTED CHAR %c", ch);
			error("Expect end of input");
			result = ERROR;
		}
		// Mark does not support the legacy JSON reviver function todo ??
		return *result.clone();
	}


	static char *readFile(chars filename) {
#ifndef WASM
		FILE *f = fopen(filename, "rt");
		if (!f)error("FILE NOT FOUND "_s + filename);
		fseek(f, 0, SEEK_END);
		long fsize = ftell(f);
		fseek(f, 0, SEEK_SET);  /* same as rewind(f); */
		char *s = (char *) (alloc(fsize, 2));
		fread(s, 1, fsize, f);
		fclose(f);
		return s;
#else
		return 0;
#endif
	}

	static Node parseFile(chars filename) {
//		const char filename=replace(filename0,"~","/Users/me")
		return Wasp::parse(readFile(filename));
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

	String pointer() {
		auto columnNumber = at - columnStart;
		String msg;
//				s("IN CODE:\n");
		msg = msg + " at position " + at + " in line " + lineNumber + " column " + columnNumber + "\n";
		msg = msg + line + "\n";
		msg = msg + (s(" ").times(columnNumber - 1)) + "^\n";
		return msg;
	}

	String err(String m) {
		// Call error when something is wrong.
		// todo: Still to read can scan to end of line
		String msg = m;
		msg += pointer();
//		msg = msg + s(" of the Mark data. \nStill to read: \n") + text.substring(at - 1, at + 30) + "\n^^ ...";
//		msg = msg + backtrace2();
		auto error = new SyntaxError(msg);
		error->at = at;
		error->lineNumber = lineNumber;
		error->columnNumber = at - columnStart;
		if (throwing)
			raise(msg);
		else
			return msg;
		return msg;
	};

	String s(const char string[]) {
		return {string};
	}

	char back() {
		ch = previous;
		at--;
		return ch;
	}

	char proceed(char c = 0) {
		if (not ch and at >= 0) {
			warn("end of code");
			return ch;
		}
		// If a c parameter is provided, verify that it matches the current character.
		if (c and c != ch) {
			error(s("Expected '") + c + "' instead of " + renderChar(ch));
		}
		// Get the next character. When there are no more characters, return the empty string.
		previous = ch;
		if (ch != ' ' and ch != '\n' and ch != '\r' and ch != '\t')
			lastNonWhite = ch;
		short step = utf8_byte_count(previous);
		at = at + step;
		if (at >= text.length) {
			ch = 0;
//			point = 0;
			return -1;
		}
		ch = decode_unicode_character(text.data + at);// charAt(at);
		short width = utf8_byte_count(ch);
		if (at + width >= text.length)next = 0;
		else next = decode_unicode_character(text.data + at + width);
//		point = text.data + at;
		if (ch == '\n' or (ch == '\r' and next != '\n')) {
			lineNumber++;
			columnStart = at;
		}
		if (previous == '\n' or previous == 0)
			line = text.substring(columnStart, text.indexOf('\n', columnStart));
		return ch;
	};

	// Parse an identifier.
	String identifier() {
		// identifiers must start with a letter, _ or $.
		if (!is_identifier(ch)) {
			error("Unexpected identifier character "s + renderChar(ch));
		}

		// To keep it simple, Mark identifiers do not support Unicode "letters", as in JS; if needed, use quoted syntax
		auto key = String(ch);
		// subsequent characters can contain ANYTHING
		while ((proceed() and is_identifier(ch)) or isDigit(ch))key += ch;
		// subsequent characters can contain digits
//		while (proceed() and
//		       (('a' <= ch and ch <= 'z') or ('A' <= ch and ch <= 'Z') or ('0' <= ch and ch <= '9') or ch == '_' or
//		        ch == '$' or ch < 0 /* UTF */ or ch == '.' or ch == '-')) {
//			 '.' and '-' are commonly used in html and xml names, but not valid JS name chars
//			key += ch;
//		}
		key += '\0';// 0x00;
		return key;
	};

	// Parse a number value.
	Node numbero() {
		auto sign = '\n';
		auto string = String("");
		int number0, base = 10;
		if (ch == '+')warn("unnecessary + sign or missing whitespace 1 +1 == [1 1]");
		if (ch == '-' or ch == '+') {
			sign = ch;
			proceed(ch);
		}
		// support for Infinity (could tweak to allow other words):
		if (ch == 'I') {
			const Node &ok = word();
			return (sign == '-') ? NegInfinity : Infinity;
		}

		// support for NaN
		if (ch == 'N') {
			const Node &ok = word();
//			bool ok = word();
//			if (!ok) { error('expected word to be NaN'); }
			// ignore sign as -NaN also is NaN
			return NaN;
		}

//		if (ch == '0') {
//			string += ch;
//			proceed();
//		} else {
		while (ch >= '0' and ch <= '9') {
			string += ch;
			proceed();
		}
		if (ch == '.') {
			string += '.';
			while (proceed() and ch >= '0' and ch <= '9') {
				string += ch;
			}
		}
		if (ch == 'e' or ch == 'E') {
			string += ch;
			proceed();
			if (ch == '-' or ch == '+') {
				string += ch;
				proceed();
			}
			while (ch >= '0' and ch <= '9') {
				string += ch;
				proceed();
			}
		}

		if (string.contains(".")) {
			if (sign == '-') return Node(-atof0(string.data));
			else return Node(atof0(string.data));
		}
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
		proceed();
		int start = at;
		while (ch and ch != delim and previous != '\\')
			proceed();
//		const String &substring = text.substring(start, at - 1);
		String substring = text.substring(start, at);
		proceed();
		return Node(substring).setType(strings);// DONT do "3"==3 (here or ever)!
	}

// Parse a string value.
	Node string2(char delim = '"') {
		auto hex = 0;
		auto i = 0;
		auto triple = false;
		auto start = at;
		String string;

		// when parsing for string values, we must look for ' or " and \ characters.
		if (ch == '"' or ch == '\'') {
			delim = ch;
			if (next == delim and text[at + 1] == delim) { // got tripple quote
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
					} else if (ch == delim and next == delim) { // end of tripple quoted text
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
			if (ch == '\n' or ch == '\r') {
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
	bool comment() {

		// Skip a comment, whether inline or block-level, assuming this is one.
		// Comments always begin with a / character.
		char preserveLast = lastNonWhite;
		if (ch != '/') {
			error("Not a comment");
		}
		if (next == '/') {
			proceed('/');
			inlineComment();
			previous = lastNonWhite = preserveLast;
			return true;
		} else if (next == '*') {
			proceed('/');
			blockComment();
			previous = lastNonWhite = preserveLast;
			return true;
		} else {
			return false; // not a comment
			// division handled elsewhere
//			error("Unrecognized comment");
		}
	};

	// Parse whitespace and comments.
	void white() {
		// Note that we're detecting comments by only a single / character.
		// This works since regular expression are not valid JSON(5), but this will
		// break if there are other valid values that begin with a / character!
		while (ch) {
			if (ch == '/') {
				if (not comment()) return; // else loop on
//				auto ws = {' ', '\t', '\r', '\n'};
// or ch == '\r' or ch == '\n' NEWLINE IS NOT A WHITE LOL, it has semantics
			} else if (ch == ' ' or ch == '\t') {
				proceed();
			} else
				return;
		}
	};

	bool isNameStart(char i) {
		return i > 'a' and i < 'Z';
	}

	bool token(String token) {
		return ch == token[0] and suffix(++token);
	}

	bool suffix(String suffix) {
		auto len = suffix.length;
		for (auto i = 0; i < len; i++) {
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


	Node any_operator() {
		Node node = Node(ch);
		node.setType(operators);// todo ++
		proceed();
		// annoying extra logic: x=* is parsed (x = *) instead of (x =*)
		while ((ch < 0 or is_operator(ch)) and (previous != '=' or ch == '=')) {// utf8 √ …
			node.name += ch;
			proceed();
		}
		return node;
	}


	Node &resolve(Node &node) {
		String &symbol = node.name;
		if (symbol == "false")return False;
		if (symbol == "False")return False;
		if (symbol == "no")return False;
		if (symbol == "No")return False;
		if (symbol == "ƒ")return False;// ‽
		if (symbol == "⊥")return False;//
//		if (node.name == "𐄂")return False; ambiguous: multiplication 𐄂 + / check 𐄂
//		if (node.name == "wrong")return False;
//		if (node.name == "Wrong")return False;
		if (symbol == "⊤")return True; // + vs -
		if (symbol == "true")return True;
		if (symbol == "True")return True;
		if (symbol == "yes")return True;
		if (symbol == "Yes")return True;
		if (symbol == "✔")return True;
		if (symbol == "✔\uefb88f")return True;// green ✔️ ~ ✔
		if (symbol == "✔️")return True;
		if (symbol == "🗸")return True;
		if (symbol == "✓️")return True;
		if (symbol == "☑")return True;
		if (symbol == "🗹")return True;
//		if (node.name == "Right")return True;// unless class!
//		if (node.name == "right")return True;
		if (symbol == "NIL")return NIL;
		if (symbol == "null")return NIL;
		if (symbol == "nill")return NIL;
		if (symbol == "nil")return NIL;
		if (symbol == "ø")return NIL;// nil not added to lists
//		if (node.name.in(operator_list))
		if (operator_list.has(symbol))
			node.setType(operators, false); // later: in angle!? NO! HERE: a xor {} != a xxx{}
//		log("resolve NOT FOUND");
//		log(symbol);
		return node;
	}

	Node symbol() {
		if (ch >= '0' and ch <= '9')return numbero();
		if (is_operator(ch))
			return any_operator();
		if (is_identifier(ch))return resolve(*new Node(identifier(), true));// or op
		error("Unexpected symbol character "s + String((char) text[at]) + String((char) text[at + 1]) +
		      String((char) text[at + 2]));
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
				return text[pos + 1] != '=' and text[pos - 1] != '=' and not is_operator(text[pos - 1]);// == != OK
			if (text[pos] == '}')braces--;
			pos++;
		}
		return false;// OK, no ambiguity
	}

	bool is_known_functor(Node node) {
		if (precedence(node))return true;
//		else if (functor_list.has(node.name))return true;
		else return false;
	}

	Node &expressione(bool stop_at_space) {
		Node node = symbol();
		if (lookahead_ambiguity())
			return *node.clone();
		// {a:1 b:2} vs { x = add 1 2 }
		Node expressionas;
		// set kind = expression only if it contains operator, otherwise keep it as list!!!
		expressionas.add(node);
		if (node.kind == operators)expressionas.kind = expression;
		if (stop_at_space and ch == ' ')return *expressionas.clone();
		white();
		while (ch and (is_identifier(ch) or isalnum0(ch) or is_operator(ch))) {
			node = symbol();// including operators `=` ...
			if (node.kind == operators)expressionas.kind = expression;
			expressionas.add(&node);
			white();
		}
//		expression.name=map(children.name)
		if (expressionas.length > 1)
			return *expressionas.clone();
		else return *node.clone();
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
			default:
				break;
		}
		if (token("one")) { return True; }
		if (token("two")) { return Node(2); }
		error("Unexpected character "s + renderChar(text.charAt(at - 1)));// throws, but WASM still needs:
		return ERROR;
	};

//	void pragma2(char prag = '\n') {// sende in wasp??
//		auto level = 0;
//		proceed();  // skip starting '('
//		while (ch) {
//			if (ch == ')') {
//				if (level) { level--; } // embedded (...)
//				else { // end of pragma
//					proceed();  // skip ending ')'
//					return pragma2(prag);
//				}
//			} else if (ch == '(') { level++; } // store as normal char
//			// else - normal char
//			prag += ch;
//			proceed();
//		}
//
//		error(UNEXPECT_END);
//	};

//	value,  // Place holder for the value function.

	// Parse an array
	Node array() {
//		arr.type = arrays;
		auto *array0 = static_cast<Node *>(alloc(sizeof(Node *), 100));// todo: GROW!
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
				Node val = valueNode();// copy by value!
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
		return ERROR;
	};

	// {: 00aacc :} base64 values todo: USE
	Node binary() {
// Parse binary value
// Use a lookup table to find the index.
		byte lookup64[128];
		byte lookup85[128];

		for (auto i = 0; i < 128; i++) {
			lookup64[i] = 65;
			lookup85[i] = 86;
		}
		for (auto i = 0; i < 64; i++) {
			char charCode = text.charCodeAt(i);
			if (charCode < 0) // never true: charCode > 128 or
				error(("Invalid binary charCode %d "_s % (long) charCode) + text.substring(i, i + 2) + "\n" + text);
			lookup64[charCode] = i;
		}
// ' ', \t', '\r', '\n' spaces also allowed in base64 stream
		lookup64[32] = lookup64[9] = lookup64[13] = lookup64[10] = 64;
		for (auto i = 0; i < 128; i++) { if (33 <= i and i <= 117) lookup85[i] = i - 33; }
// ' ', \t', '\r', '\n' spaces also allowed in base85 stream
		lookup85[32] = lookup85[9] = lookup85[13] = lookup85[10] = 85;


		at++;  // skip the starting '{:'
		if (next == '~') { // base85
			at++;  // skip '~'
			// code based on https://github.com/noseglid/base85/blob/master/lib/base85.js
			auto end = text.indexOf('}', at + 1);  // scan binary end
			if (end < 0) { error("Missing ascii85 end delimiter"); }

			// first run decodes into base85 int values, and skip the spaces
			auto p = 0;
			byte base[end - at + 3];  // 3 extra bytes of padding
			while (at < end) {
				auto code = lookup85[text.charCodeAt(at)];  // console.log('bin: ', next, code);
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
			auto dataLength = p, padding = (dataLength % 5 == 0) ? 0 : 5 - dataLength % 5;
			int buffer[4 * dataLength / 5 - padding];
//				bytes = new DataView(buffer),
			int *bytes = buffer;// views:
			auto *bytes8 = reinterpret_cast<byte *>(buffer);
			auto *bytes16 = reinterpret_cast<short *>(buffer);
			int trail = dataLength - 4;//buffer.byteLength - 4;
			base[p] = base[p + 1] = base[p + 2] = 84;  // 3 extra bytes of padding
			// console.log('base85 byte length: ', buffer.byteLength);
			for (auto i = 0, p = 0; i < dataLength; i += 5, p += 4) {
				auto num = (((base[i] * 85 + base[i + 1]) * 85 + base[i + 2]) * 85 + base[i + 3]) * 85 + base[i + 4];
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
						default:
							break;
					}
				}
			}
			return Node(buffer);// {buffer};
		} else { // base64
			// code based on https://github.com/niklasvh/base64-arraybuffer
			auto end = text.indexOf('}', at), bufEnd = end, pad = 0;  // scan binary end
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
			auto p = 0;
			while (at < bufEnd) {
				auto code = lookup64[text.charCodeAt(at)];  // console.log('bin: ', next, code);
				if (code > 64) { error("Invalid base64 character"); }
				if (code < 64) { base[p++] = code; }
				// else skip spaces
				at++;
			}
			at = end + 1;
			proceed();  // skip '}'
			// check length
			if ((pad and (p + pad) % 4 != 0) or (!pad and p % 4 == 1)) {
				error("Invalid base64 stream length");
			}

			// second run decodes into actual binary data
			auto len = int(p * 0.75);
			int code1, code2, code3, code4 = 0;
			int buffer[len];
			auto *bytes = reinterpret_cast<byte *>(buffer);// views:
			// console.log('binary length: ', len);
			for (auto i = 0, p = 0; p < len; i += 4) {
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

	Node &setField(Node &key, Node &val) { // a:{b}
		if ((val.kind == groups or val.kind == patterns or val.kind == objects) and val.length == 1 and
		    empty(val.name))
			val = val.last();// singleton
		val.parent = &key;// todo bug: might get lost!
		bool deep_copy = empty(val.name) or !debug or (key.kind == reference and empty(val.name));
		if (debug) {// todo make sure all works even with nested nodes! x="123" (node 'x' (child value='123')) vs (node 'x' value="123")
			deep_copy = deep_copy or (val.kind == Type::longs and val.name == itoa(val.value.longy));
			deep_copy = deep_copy or (val.kind == Type::reals and val.name == ftoa(val.value.real));
			deep_copy = deep_copy or (val.kind == Type::bools and
			                          (val.name == "True" or val.name == "False"));// todo why check name?
//			if(val.kind == Type::strings)
//				debug = 1;
//			deep_copy = deep_copy or (val.kind == Type::strings and not val.name.empty() and key.kind==reference); and ... ?
		} // shit just for debug labels. might remove!!
// last part to preserve {deep{a:3,b:4,c:{d:'hi'}}} != {deep{a:3,b:4,c:'hi'}}

		if (val.value.longy and val.kind != objects and deep_copy) {
			if (&key == &NIL or key.isNil() or key == NIL or val.value.real == 6.4807)
				if (key.name == nil_name)
					warn("impossible");
			key.value = val.value;// direct copy value SURE?? what about meta data... ?
			key.kind = val.kind;
		} else {
			key.kind = keyNode;
			if (!key.children and empty(val.name) and val.length > 1) { // deep copy why?
				key.children = val.children;
				key.length = val.length;
				key.kind = val.kind;
			} else if (!val.isNil())
				key.value.node = &val;// clone?
		}
		return key;
	}


	bool checkAmbiguousBlock(Node current, Node *parent) {
		// wait, this should be part of case ' ', no?
		//						a of {a:1 b:2}
		return previous == ' ' and current.last().kind != operators and current.last().kind != call and
		       (!parent or (parent and parent->last().kind != operators and parent->last().kind != call)) and
		       lastNonWhite != ':' and lastNonWhite != '=' and lastNonWhite != ',' and lastNonWhite != ';' and
		       lastNonWhite != '{' and lastNonWhite != '(' and lastNonWhite != '[' and
		       lastNonWhite != '}' and lastNonWhite != ']' and lastNonWhite != ')';

	}

	bool skipBorders(char ch) {// {\n} == {}
		if (lastNonWhite == ':')return true;
		if (lastNonWhite == '{' or next == '}')return true;// todo: nextNonWhite
		if (lastNonWhite == '(' or next == ')')return true;
		if (lastNonWhite == '[' or next == ']')return true;
		if (ch == ',' and next == ';')return true;// 1,2,3,; => 1,2,3;
		if (ch == ',' and next == '\n')return true;// 1,2,3,\n => 1,2,3;
		if (ch == ';' and next == '\n')return true;// 1,2,3,\n => 1,2,3;
		return false;
	}

	bool isFunctor(Node &node) {
		return node.name.in(functor_list);
	}

	bool is_grouper(codepoint previous) {
		codepoint *point = grouper_list;
		do {
			if (previous == *point)
				return true;
		} while (*point++);
		return false;
	}

// ":" is short binding a b:c d == a (b:c) d
// "=" is number-binding a b=c d == (a b)=(c d)   todo a=b c=d
// special : close=' ' : single value in a list {a:1 b:2} ≠ {a:(1 b:2)} BUT a=1,2,3 == a=(1 2 3)
// special : close=';' : single expression a = 1 + 2
// significant whitespace a {} == a,{}{}
// todo a:[1,2] ≠ a[1,2] but a{x}=a:{x}? OR better a{x}=a({x}) !? but html{...}
	Node &valueNode(codepoint close = 0, Node *parent = 0) {
		// A JSON value could be an object, an array, a string, a number, or a word.
		Node current;
		current.parent = parent;
		current.setType(groups);// may be changed later, default (1 2)==1,2
		auto length = text.length;
		int start = at;
		loop:
		white();// insignificant whitespace HERE
		while (ch and at <= length) {
//			white()  significant whitespace   1+1 != 1 +1 = [1 1]
			if (previous == '\\') {// escape ANYTHING
				proceed();
				continue;
			}
			if (ch == close) { // (…) {…} «…» ...
				if (ch == '}' or ch == ']' or ch == ')')
					proceed();
				break;
			}
			if (closing(ch, close)) { // 1,2,3;  «;» closes «,» list
				break;
			}// inner match ok
			switch (ch) {
//				https://en.wikipedia.org/wiki/ASCII#Control_code_chart
//				https://en.wikipedia.org/wiki/ASCII#Character_set
				case '\x0E': // Shift Out close='\x0F' Shift In
				case u'⸨': // '⸩'
				case '{': {
					if (checkAmbiguousBlock(current, parent)) {
						log(pointer());
						breakpoint_helper
						warn("Ambiguous reading a {x} => Did you mean a{x} or a:{x} or a , {x}");
					}
					bool asListItem =
							lastNonWhite == ',' or lastNonWhite == ';' or (previous == ' ' and lastNonWhite != ':');
					proceed();
					Node object = Node().setType(Type::objects);
					Node objectValue = valueNode('}', parent ? parent : &current.last());
					object.addSmart(objectValue);
					object = object.flat();
					if (asListItem)
						current.add(object);
					else
						current.addSmart(object);
					break;
				}
				case U'［': // FULLWIDTH ｛ ｢ ｣
				case '[': {
					proceed();
					Node pattern = Node().setType(Type::patterns);
					Node patternValue = valueNode(']', &current.last());
					pattern.add(patternValue);
					if (patternValue.kind == expression or patternValue.kind == groups)
						pattern = patternValue.setType(patterns, false);
					current.add(pattern);// a[b] ≠ (a b)
//					current.addSmart(pattern);// a[b] ≠ (a b) always preserve pattern (?)
					break;
				}
				case '(': {
					// checkAmbiguousBlock? x (1) == x(1) or [x 1] ?? todo too bad!!!
					proceed();
					Node group = Node().setType(Type::groups);
					Node groupValue = valueNode(')', &current.last());
					group.addSmart(groupValue);
					if (groupValue.kind == objects)
						group = groupValue.setType(groups);// flatten hack
					current.addSmart(group);
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
					if (isDigit(next) and (previous == 0 or is_grouper(previous) or is_operator(previous)))
						current.addSmart(numbero());// (2+2) != (2 +2) !!!
					else if (ch == '-' and next == '.')
						current.addSmart(numbero()); // -.9 -0.9 border case :(
					else {
						Node *op = any_operator().clone();
						current.add(op);
						current.kind = expression;
					}
					break;
				case '"':
				case '\'': /* don't use modifiers ` ˋ ˎ */
				case u'«': // «…»
				case u'‘':// ‘𝚗𝚊𝚖𝚎’
				case u'“':// “…” Character too large for enclosing character literal type
				case '`': {
					if (previous == '\\')continue;// escape
					bool matches = close == ch;
					matches = matches or (close == u'‘' and ch == u'’');
					matches = matches or (close == u'’' and ch == u'‘');
					matches = matches or (close == u'“' and ch == u'”');
					matches = matches or (close == u'”' and ch == u'“');
					if (!matches) { // open string
						if (current.last().kind == expression)
							current.last().addSmart(string(ch));
						else
							current.add(string(ch).clone());
						break;
					}
					Node id = Node(text.substring(start, at));
					id.setType(Type::strings);// todo "3" could have be resolved as number? DONT do js magifuckery
					current.add(id);
					break;
				}
				case U'≝':
				case U'≔': // ≕ =:
				case U'＝':
				case U'﹦':
				case U'：':
				case ':':
				case '=': {
					// todo {a b c:d} vs {a:b c:d}
					Node &key = current.last();
					bool add_raw = current.kind == expression or key.kind == expression or
					               (current.last().kind == groups and current.length > 1);
					if (is_operator(previous))
						add_raw = true;// == *=
					Node op = any_operator();// extend *= ...
					if (not(op.name == ":" or (data_mode and op.name == "=")))
						add_raw = true;// todo: treat ':' as implicit constructor and all other as expression for now!
					if (op.name.length > 1)
						add_raw = true;
					if (current.kind == expression)
						add_raw = true;
					if (add_raw) {
						current.add(op.setType(operators)).setType(expression);
//						continue; noo why continue??
					}
					Node &val = *valueNode(' ', &key).clone();// applies to WHOLE expression
					if (add_raw) {  // complex expression are not simple maps
						current.add(val);
					} else {
						setField(key, val);
					}
					break;
				}
				case '\n': // groupCascade
				case '\t': // only in tables
				case ';': // indent 􀋵  ☞ 𒋰 𒐂 ˆ ˃
				case ',': {
					if (skipBorders(ch)) {
						proceed();
						continue;
					}
					// closing ' ' handled above
					// ambiguity? 1+2;3  => list (1+2);3 => list  ok!
					if (current.grouper != ch) {// and current.length > 1
						// x;1+2 needs to be grouped (x (1 + 2)) not (x 1 + 2))!
						if (current.length > 1 or current.kind == expression) {
							Node neu;// wrap x,y => ( (x y) ; … )
							neu.kind = groups;
							neu.parent = parent;
							//						neu.grouper = ch;
							neu.add(current);
							current = neu;
						}
						current.grouper = ch;
						char closer = ch;// need to keep troughout loop!
						int i = 0;
						//						closing(ch, closer) and not closing(ch, close) and ch!='}' and ch!=')' and ch!=']' and ch!=0
						while (ch == closer) {
							proceed();
							Node element = valueNode(closer);// todo stop copying!
							current.add(element.clone());
						}
						break;
					}
					// else fallthough!
					current.grouper = ch;
				}
				case ' ': // possibly significant whitespace not consumed by white()
				{
					proceed();
					white();
					break;
				}
				case '/':
					if (ch == '/' and (next == '/' or next == '*')) {
						comment();
						warn("comment should have been handled before!?");
						continue;
					}// else fall through to default … expressione
				default: {
					// a:b c != a:(b c)
					// {a} ; b c vs {a} b c vs {a} + c
					bool addFlat = lastNonWhite != ';' and previous != '\n';
					Node node = expressione(close == ' ');//word();
					if (precedence(node) or operator_list.has(node.name))
						node.kind = operators;
					if (node.kind == operators and ch != ':') {
						if (isFunctor(node))
							node.kind = functor;// todo: earlier
						else current.kind = expression;
					}
					if (node.length > 1 and addFlat) {
						for (Node arg:node)current.add(arg);
						current.kind = node.kind;// was: expression
					} else {
						current.add(&node);
					}
				}
			}
		}

		bool keepBlock = close == '}';
		Node &result = current.flat();
		return *result.

				clone();
	};
//	int $parent{};
};


float precedence(char group) {
	if (group == 0)return 1;
	if (group == '}')return 1;
	if (group == ']')return 1;
	if (group == ')')return 1;
	if (0 < group and group < 0x20)return 1.5;
	if (group == '\n')return 2;
	if (group == ';')return 3;
	if (group == ',')return 4;
	if (group == ' ')return 5;
	if (group == '_')return 6;
//error("unknown precedence for symbol: "s+group);
	return 999;
}


// test functions to check wasm->runtime interaction
int ok() {
	return 42;
}

int oki(int i) {// used in wasm runtime test
	return 42 + i;
}

float okf(float f) {
	return 42 + f;
}

float okf5(float f) {
	return 41.5 + f;
}


void not_ok() {
	error1("WHAAA");
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


//void assert_is(char *wasp, Node result);

//void init() {
//	NIL.kind = nils;
//	NIL.value.number = 0;
//	False.kind = bools;
//	False.value.number = 0;
//	True.kind = bools;
//	True.value.number = 1;
//}

#ifndef NO_TESTS // RUNTIME_ONLY

#import "tests.cpp"
#include "WebServer.hpp"

#endif

Node run(String source) {
	return emit(source);
}

//static
Node parse(String source) {
	operator_list = List<chars>(operator_list0);// wasm hack
	// WE HAVE A GENERAL PROBLEM:
	// 1. top level objects are not constructed True
	// 2. even explicit construction seems to be PER object scope (.cpp file) HOW!

	printf("Parsing: %s\n", source.data);
	if (!source.data)return NIL;
	return Wasp().read(source);
}

// Mark/wasp has clean syntax with FULLY-TYPED data model (like JSON or even better)
// Mark/wasp is generic and EXTENSIBLE (like XML or even better)
// Mark/wasp has built-in MIXED CONTENT support (like HTML5 or even better)
// Mark/wasp supports HIGH-ORDER COMPOSITION (like S-expression or even better)



class String;

#ifndef WASM

void print(String s) {
	if (!s.shared_reference)
		log(s.data);
	else {
		char tmp = s.data[s.length];
		s.data[s.length] = 0;// hack not thread-safe
		log(s.data);
		s.data[s.length] = tmp;
	}
}

#endif

struct Exception {
};
//wasm-ld: error: wasp.o: undefined symbol: vtable for __cxxabiv1::__class_type_info

// 2020: WASI does not yet support C++ exceptions. C++ code is supported only with -fno-exceptions for now.
// https://github.com/WebAssembly/wasi-sdk
//struct ERR{
//public:
//	ERR() = default;
//	virtual ~ERR() = default;
//};
char newline = '\n';
//#ifndef _main_
#define __MAIN__

// called AFTER __wasm_call_ctors() !!!
//#ifndef RUNTIME_ONLY
//Undefined symbols for architecture arm64:
//"_main", referenced from:
//implicit entry/start for main executable

Node &compile(String &file) {

}


int run_wasm_file(chars file) {
	FILE *ptr;
	ptr = fopen(file, "rb");  // r for read, b for binary
	if (!ptr)error("File not found "s + file);
	fseek(ptr, 0L, SEEK_END);
	int sz = ftell(ptr);
	unsigned char buffer[sz];
	fread(buffer, sizeof(buffer), 1, ptr); // read 10 bytes to our buffer
	return run_wasm(buffer, sz);
}

void usage() {
	print("wasp is a new compiler and programming language");
	print("wasp [repl]            open interactive programming environment");
	print("wasp <file.wasp>       compile wasp to wasm or native and execute");
	print("wasp <file.wasm>       compile wasm to native and execute");
	print("wasp help              see https://github.com/pannous/wasp/wiki");
	print("wasp tests             ");

}

// wasmer etc DO accept float/double return, just not from main!
int main(int argp, char **argv) {
#ifdef ErrorHandler
	register_global_signal_exception_handler();
#endif
	try {
		log("Hello Wasp 🐝");
		if (argp >= 1) {
			String arg = argv[0];
			if (arg.endsWith(".wasp"))
				compile(arg);
			if (arg.endsWith(".wasm"))
				run_wasm_file(arg);
			if (arg == "test" or arg == "tests")
				testCurrent();
			if (arg == "app" or arg == "start" or arg == "webview" or arg == "browser" or arg == "run" or
			    arg == "repl") {
//				start_server(9999);
				init_graphics();
			}
			if (arg.contains("serv"))
				start_server(9999);
			if (arg.contains("help"))
				print("detailed documentation can be found at https://github.com/pannous/wasp/wiki ");
//			return 42;
		} else {
			usage();
		}
#ifdef WASM
		initSymbols();
//		String args(current);
		String args((char*)alloc(1,1));// hack: written to by wasmx
//		args.data[0] = '{';
		log(args);
		current += strlen0(args)+1;
#endif
#ifdef WEBAPP
		log("\nWEBAPP!");
		// handing over to V8, we need to call testCurrent() from there!
		std::thread go(start_server, 9999);
		init_graphics(); // startApp();
//		start_server(9999);
#endif
#ifndef NO_TESTS // RUNTIME_ONLY

		testCurrent();
#endif
		return 42;
	} catch (Exception e) {
		printf("\nException WOOW\n");
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
//	usleep(1000000000);
	return 1; //EXIT_FAILURE;
}

//#endif

#ifndef WASI
//#ifndef RUNTIME_ONLY
//extern int main(int argp, char **argv);
extern "C" int _start() { // for wasm-ld
	return -42;// wasm-ld dummy should not be called
//	return main(0, 0);
}
//#endif
#endif


