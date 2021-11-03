//#pragma once
#include "Node.h"
#include "Wasp.h"
#include "Util.h"
#include "String.h" // variable has incomplete type
#include "Backtrace.h" // header ok in WASM
#include "wasm_helpers.h"
#include "wasm_emitter.h"
#include "wasm_runner.h"

void testCurrent();
// get home dir :
/*#include <unistd.h>
#include <sys/types.h>
#include <pwd.h>*/
#include "stdio.h" // FILE

//#define err(m) printf("\n%s:%d\n",__FILE__,__LINE__);err1(m)
#define err(m) err1("\n%s:%d\n%s"s%__FILE__%__LINE__%m)

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


//List<codepoint>
//List<chars> circumfixOperators/*Left*/ = {"‖", 0};
//codepoint circumfixOperators[] = {u'‖', 0};
// minus common ones u'(', u'{', u'[',
codepoint opening_special_brackets[] = {u'‖', u'﴾', u'﹙', u'（', u'⁽', u'⸨', u'﹛', u'｛', u'﹝', u'〔', u'〘',
                                        u'〚', u'〖', u'【', u'『', u'「', u'｢', u'⁅', u'«', u'《', u'〈',
                                        u'︷', u'︵', u'﹁', u'﹃', u'︹', u'︻', u'︽', 0};
//codepoint closing_special_brackets[] = {}

codepoint separator_list[] = {' ', ',', ';', ':', '\n', '\t', 0};
// todo:
//codepoint grouper_list[] = {'(', ')', '{', '}', '[', ']', u'«', u'»', 0, };
codepoint grouper_list[] = {' ', ',', ';', ':', '\n', '\t', '(', ')', '{', '}', '[', ']', u'«', u'»', 0};
// () ﴾ ﴿ ﹙﹚（ ） ⁽ ⁾  ⸨ ⸩
// {} ﹛﹜｛｝    ﹝﹞〔〕〘〙  ‖…‖
// [] 〚〛〖〗【】『』「」｢｣ ⁅⁆
// «» 《》〈〉〈〉
// ︷ ︵ ﹁ ﹃ ︹ ︻ ︽
// ︸ ︶ ﹂ ﹄ ︺ ︼ ︾


// predicates in of on from to
// todo split keywords into binops and prefix functors
chars import_keywords[] = {"use", "require", "import", "include", "using", 0};
// todo aliases need NOT be in this list:
chars operator_list0[] = {"+", "-", "*", "/", ":=", "else", "then" /*pipe*/ , "is", "equal", "equals", "==", "!=", "≠",
                          "not",
                          "¬", "|", "and", "or", "&", "++", "--", "to", "xor", "be", "?", ":", "…", "...", "%", "mod",
                          "modulo",
                          "..<" /*range*/,
                          "upto", "use", "include", "require", "import", "module",
                          "<=", ">=", "≥", "≤", "<", ">", "less", "bigger", "⁰", "¹", "²", "×", "⋅", "⋆", "÷",
                          "^", "∨", "¬", "√", "∈", "∉", "⊂", "⊃", "in", "of", "by", "iff", "on", "as", "^^", "^", "**",
                          "from", "#", "$", "ceil", "floor", "round", "∧", "⋀", "⋁", "∨", "⊻",
                          "abs", /* "norm", "‖" acts as GROUP, not as operator (when parsing) */
		// norm ‖…‖ quite complicated for parser! ‖x‖ := √∑xᵢ²
		                  0, 0, 0,
		                  0}; // "while" ...
// todo ∨ ~ v ~ versus! "³", "⁴", define inside wasp
//  or  & and ∨ or ¬  or  ~ not → implies ⊢ entails, proves ⊨ entails, therefore ∴  ∵ because
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


// ︷
// ︸﹛﹜｛｝﹝﹞〔〕〘〙〚〛〖〗【】『』「」｢｣《》〈〉〈〉⁅⁆ «»
// ︵  ﴾ ﴿ ﹙ ﹚ （ ） ⁽ ⁾  ⸨⸩ see grouper_list
// ︶
codepoint closingBracket(codepoint bracket) {
	switch (bracket) {
		case '\x0E':
			return '\x0F'; // Shift In closes Shift Out??
		case '\x0F':
			return '\x0E'; // Shift Out closes '\x0F' Shift In
		case u'‖':
			return u'‖';
		case u'⸨':
			return u'⸩';
		case u'﹛':
			return u'﹜';
		case u'｛':
			return u'｝';//  ︷
		case u'﹝':
			return u'﹞';// ︸
		case u'〔':
			return u'〕';
		case u'〘':
			return u'〙';
		case u'〚':
			return u'〛';
		case u'〖':
			return u'〗';
		case u'【':
			return u'】';
		case u'『':
			return u'』';
		case u'「':
			return u'」';
		case u'｢':
			return u'｣';
		case u'《':
			return u'》';
		case u'〈':
			return u'〉';
		case u'⁅':
			return u'⁆';
		case '{':
			return u'}';
		case '(':
			return u')';
		case '[':
			return u']';
		case u'‘':
			return u'’';
		case u'«':
			return u'»';
		case u'“':
			return u'”';
		default:
			error("unknown bracket "s + bracket);
	}

	return 0;
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
	if (ch == u'¬')return true;
	if (ch == u'＝')return true;
	if (ch == u'#')return true;
	if (operator_list.has(String(ch)))
		return true;

	//		if(ch=='=') return false;// internal treatment
	if (ch > 0x80)
		return false;// utf NOT enough: ç. can still be a reference!
	if (is_identifier(ch)) return false;
	if (isalnum0(ch)) return false;// ANY UTF 8
	return ch > ' ' and ch != ';' and !is_bracket(ch) and ch != '\'' and ch != '"';
}


// list HAS TO BE 0 terminated! Dangerous C!! ;)
template<class S>
bool contains(S list[], S match) {
	S *elem = list;
	do {
		if (match == *elem)
			return true;
	} while (*elem++);
	return false;
}

bool contains(chars list[], chars match) {
	chars *elem = list;
	do {
		if (eq(match, *elem))
			return true;
	} while (*elem++);
	return false;
}


class Wasp {

	String text = EMPTY;
	String file = EMPTY; // possible source
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

	int indentation_level = 0;
	bool indentation_by_tabs = false;
#define INDENT 0x0F // 	SI 	␏ 	^O 		Shift In
#define DEDENT 0x0E //  SO 	␎ 	^N 		Shift Out

	//	indent 􀋵 (increase.indent) ☞ 𒋰 𒐂 ˆ ˃
	int indentation() {
		if (is_grouper(lastNonWhite)) {
			// todo: ignore indent in x{…} BUT keep and restore previous indent!
		}
		float tabs = 0;
		int spaces_per_tab = 2;
		int offset = at;
//		if(text[offset]==end_block) 0x0b / vertical tab
//			return indentation_level-1;
		if (text[offset] == '\n' and text[offset + 1] == '\n') {
//			// double newline dedent. really? why not keep track via indent?
//			print(position());
//			if(text[offset+2] == '\n')
//				return indentation_level--;// todo close ALL?
//			else return indentation_level; //ignore simple newlines
		}
		if (text[offset] == '\n' or text[offset] == '\r')offset++;
		while (text[offset] == '\t') {
			if (indentation_level > 0 and not indentation_by_tabs)
				proceed() ? err("mixing tabs and spaces for indentation") : 0;
			indentation_by_tabs = true;
			tabs++;
			offset++;
		}
		if (tabs > 0 and text[offset] == ' ')
			err("ambiguous indentation, mixing tabs and spaces");
		while (text[offset] == ' ') {
			if (indentation_level > 0 and indentation_by_tabs)
				proceed() ? err("mixing tabs and spaces for indentation") : 0;
			indentation_by_tabs = false;
			tabs = tabs + 1. / spaces_per_tab;
			offset++;
		}
		if (tabs > 0 and text[offset] == '\t')err("ambiguous indentation, mixing tabs and spaces");
//		while(next==' ' or next=='\t')proceed();// but keep last ch as INDENT!
		if (text[offset] == '\n')
			return indentation_level; // careful empty lines if next indentation == last one : just hangover spacer!
		if (text[offset] == 0)return 0; // no more indentation.
		return floor(tabs);
	}

// Ascii control for indent/dedent: perfect!
//  0x0B    VT  ␋     vertical tab => end_block
//  0x0E 	SO 	␎ 	^N 		Shift Out
//  0x0F 	SI 	␏ 	^O 		Shift In
//	0x1C 	S4 	FS 	␜ 	^\ 		File Separator
//	0x1D 	S5 	GS 	␝ 	^] 		Group Separator
//	0x1E 	S6 	RS 	␞ 	^^[k] 		Record Separator
//	0x1F 	S7 	US 	␟ 	^_ 		Unit Separator
// ␙
//0x2403	9219	SYMBOL FOR END OF TEXT	␃
//0x2404	9220	SYMBOL FOR END OF TRANSMISSION	␄
//0x2419	9241	SYMBOL FOR END OF MEDIUM	␙
//0x241B	9243	SYMBOL FOR ESCAPE	␛
// U+0085 <control-0085> (NEL: NEXT LINE) ␤ NewLine
// ‘Language Tag character’ (U+E0001) + en-us …
	bool closing(char ch, char closer) {
		if (ch == closer)
			return true;
		if (precedence(ch) <= precedence(closer))
			return true;
		if (ch == INDENT)
			return false;// quite the opposite
		if (ch == DEDENT and not(closer == '}' or closer == ']' or closer == ')'))
			return true;
		if (ch == '}' or ch == ']' or ch == ')') { // todo: ERROR if not opened before!
			//				if (ch != close and close != ' ' and close != '\t' /*???*/) // cant debug wth?
			return true;
		}// outer match unresolved so far

		if (precedence(ch) <= precedence(closer))
			return true;
		return false;
	}

public:

//	Mark(){}
//	Mark(const Node &obj);
//	Mark(String source) {
//		this->text = source;
//	}
//	Wasp() : lineNumber(0) {
//		at = -1;
//	}

// Return the enclosed parse function. It will have access to all of the above functions and variables.
//    Node return_fuck(auto source,auto options) {
// YUCK static magically applies to new() objects too!?!
	Node parse(String source) {
		printf("Parsing: %s\n", source.data);
		return read(source);
	}

	// see 'apply' for operator eval
	static Node eval(String source) { // return by value ok, rarely used and stable
		Node parsed = Wasp().parse(source);
		parsed.log();
#ifndef RUNTIME_ONLY
		return parsed.interpret();
#else
		error("RUNTIME_ONLY, no interpret!");
		return parsed;// DANGER!!
#endif
	}


	// todo: flatten the parse->parse->read branch!!
	Node &read(String source) {
		if (source.endsWith(".wasp") and not source.contains("\n")) {
			setFile(source);
			source = readFile(findFile(source));
		}
		if (source.empty()) return const_cast<Node &>(NIL);
		columnStart = 0;
		at = -1;
		lineNumber = 1;
		ch = 0;
		text = source;
		while (empty(ch) and (ch or at < 0))
			proceed();// at=0
		Node result = valueNode(); // <<
		white();
		if (ch and ch != -1 and ch != DEDENT) {
			printf("UNEXPECTED CHAR %c", ch);
			print(position());
			error("Expect end of input");
			result = ERROR;
		}
		// Mark does not support the legacy JSON reviver function todo ??
		return *result.clone();
	}


	Wasp &setFile(String string) {
		file = string;
		return *this;
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

	[[nodiscard]]
	String position() {
		auto columnNumber = at - columnStart;
		String msg;
		msg = msg + " in line " + lineNumber + " column " + columnNumber + " (char#" + at + ")\n";
		msg = msg + line + "\n";
		msg = msg + (s(" ").times(columnNumber - 1)) + "^";
		if (not file.empty()) msg = msg + "\n" + file + ":" + lineNumber;
//		print(msg);
		return msg;
	}

	String err1(String m) {
		// Call error when something is wrong.
		// todo: Still to read can scan to end of line
		String msg = m;
		msg += position();
//		msg = msg + s(" of the Mark data. \nStill to read: \n") + text.substring(at - 1, at + 30) + "\n^^ ...";
//		msg = msg + backtrace2();
		auto error = new SyntaxError(msg);
		error->at = at;
		error->lineNumber = lineNumber;
		error->columnNumber = at - columnStart;
		error->file = file.data;
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


	// one char at a time, NO JUMPING OVER ' ' here!
	char proceed(char c = 0) {
		if (not ch and at >= 0) {
			warn("end of code");
			return ch;
		}
		// If a c parameter is provided, verify that it matches the current character.
		if (c and c != ch) {
			err(s("Expected '") + c + "' instead of " + renderChar(ch));
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
			columnStart = at;// including indent
			int new_indentation_level = indentation();
			if ((lastNonWhite == ':' or not is_grouper(lastNonWhite)) and previous != '"')
				if (new_indentation_level > indentation_level)
					ch = INDENT;
			if (new_indentation_level < indentation_level)
				ch = DEDENT;
			indentation_level = new_indentation_level;
			//			at = columnStart;// restore to be sure todo remove
		}
		if (previous == '\n' or previous == INDENT or previous == DEDENT or previous == 0) {
			auto to = text.indexOf('\n', at);
			line = text.substring(at, to);
		}
		return ch;
	};

	// Parse an identifier.
	String identifier() {
		// identifiers must start with a letter, _ or $.
		if (!is_identifier(ch)) err("Unexpected identifier character "s + renderChar(ch));
		int start = at;
		// subsequent characters can contain ANYTHING
		while ((proceed() and is_identifier(ch)) or isDigit(ch))
			if (is_operator(ch))
				break;
		int to = at;
		while (to > 0 and empty(text[to - 1]))to--;
		String key = String(text.data + start, to - start, !debug);
		return key;
	};

	Node hexadecimal_number() {
		if (ch == '0')proceed();
		if (ch == 'x' or ch == 'X')proceed();
		int val = 0;
		while (true) {
			if (ch >= '0' and ch <= '9') val = val * 16 + (ch - '0');
			else if (ch >= 'a' and ch <= 'f') val = val * 16 + (ch - 'a' + 10);
			else if (ch >= 'A' and ch <= 'F') val = val * 16 + (ch - 'A' + 10);
			else break;
			proceed();
		}
		return Node(val);
	}

	// Parse a number value.
	Node numbero() {
		auto sign = '\n';
		auto string = String("");
		int number0, base = 10;
		if (ch == '0' and (next == 'x' or next == 'X'))return hexadecimal_number();// base=16;
		if (ch == '0' and (next == 'o' or next == 'O')) {
			todo("octal");
			base = 8;
		} // todo
		if (ch == '+' and not is_operator(previous))
			warn("unnecessary + sign or missing whitespace 1 +1 == [1 1]");
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

		while (atoi1(ch) >= 0) { // -1 if not
			//	ch >= '0' and ch <= '9'
			string += ch;
			proceed();
		}
		if (ch == '.') {
			string += '.';
			while (proceed() and atoi1(ch) >= 0) {
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
			while (atoi1(ch) != -1) {
				//				ch >= '0' and ch <= '9'
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
		err("Bad string");
		return NIL;
	};

	// Parse an inline comment
	void inlineComment() {
		// Skip an inline comment, assuming this is one. The current character should
		// be the second / character in the // pair that begins this inline comment.
		// To finish the inline comment, we look for a newline or the end of the text.
		if (ch != '/' and ch != '#') {
			err("Not an inline comment");
		}
		do {
			proceed();
			if (ch == '\r' or ch == '\n' or ch == 0) {
//				proceed();
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
			err("Not a block comment");
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

		err("Unterminated block comment");
	};

	// Parse a comment
	bool comment() {
		// Skip a comment, whether inline or block-level, assuming this is one.
		// Comments always begin with a # or / character.
		char preserveLast = lastNonWhite;
		if (ch == '#') {
			inlineComment();
			previous = lastNonWhite = preserveLast;
			return true;
		}
		if (ch != '/') err("Not a comment");
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
				// NEWLINE IS NOT A WHITE , it has semantics
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
		while (ch == '=' or ch == previous) {// allow *= += ++ -- **  …
			node.name += ch;
			proceed();
		}
		// NO OTHER COMBINATIONS for now!

		/*// annoying extra logic: x=* is parsed (x = *) instead of (x =*)
		while ((ch < 0 or is_operator(ch)) and (previous != '=' or ch == '=')) {// utf8 √ …
			if (ch != '-' and previous == '-')
				break;// no combinations with - : -√
			if (ch == '-' and previous != '-')
				break;// no combinations with -  √- *- etc
			node.name += ch;
			proceed();
		}*/
		return node;
	}


	const Node &resolve(Node &node) {
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
//		put("resolve NOT FOUND");
//		put(symbol);
		return node;
	}

	Node symbol() {
		if (isDigit(ch))
			return numbero();
		if (ch == '.' and (isDigit(next)))
			return numbero();
		// todo simplify?
		if ((ch == '-' or ch == '+') and (isDigit(next) or next == '.') and previous != u'‖' and
		    (empty(previous) or is_operator(previous) or next == '.')) // -1 √-1 but not 2-1 x-1!
			return numbero();
		if (ch == u'‖') {
			proceed();// todo: better ;)
			return (*new Node("‖")).add(valueNode(u'‖')).setType(operators, false);
		}
		if (is_operator(ch))
			return any_operator();
		if (is_identifier(ch))
			return resolve(*new Node(identifier(), true));// or op
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

	Node &expressione(codepoint closer) {
		Node node = symbol();
		if (lookahead_ambiguity())
			return *node.clone();
		// {a:1 b:2} vs { x = add 1 2 }
		Node expressionas;
		// set kind = expression only if it contains operator, otherwise keep it as list!!!
		expressionas.add(node);
		if (node.kind == operators) expressionas.kind = expression;//
//		if (contains(import_keywords,node.name))
//			closer =0;// get rest of line;
		if (closing(ch, closer))// stop_at_space, keep it for further analysis (?)
			return *expressionas.clone();
		white();
		if (node.kind != operators) expressionas.kind = groups;
		while (ch and ch != closer and (is_identifier(ch) or isalnum0(ch) or is_operator(ch))) {
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
		err("Expecting ]");
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
				err(("Invalid binary charCode %d "_s % (long) charCode) + text.substring(i, i + 2) + "\n" + text);
			lookup64[(short) charCode] = i;// todo: what is this?
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
			if (end < 0) { err("Missing ascii85 end delimiter"); }

			// first run decodes into base85 int values, and skip the spaces
			auto p = 0;
			byte base[end - at + 3];  // 3 extra bytes of padding
			while (at < end) {
				auto code = lookup85[(short) text.charCodeAt(at)];  // console.put('bin: ', next, code);
				if (code > 85) { err("Invalid ascii85 character"); }
				if (code < 85) { base[p++] = code; }
				// else skip spaces
				at++;
			}
			at = end + 2;
			proceed();  // skip '~}'
			// check length
			if (p % 5 == 1) { err("Invalid ascii85 stream length"); }

			// second run decodes into actual binary data
			auto dataLength = p, padding = (dataLength % 5 == 0) ? 0 : 5 - dataLength % 5;
			int buffer[4 * dataLength / 5 - padding];
//				bytes = new DataView(buffer),
			int *bytes = buffer;// views:
			auto *bytes8 = reinterpret_cast<byte *>(buffer);
			auto *bytes16 = reinterpret_cast<short *>(buffer);
			int trail = dataLength - 4;//buffer.byteLength - 4;
			base[p] = base[p + 1] = base[p + 2] = 84;  // 3 extra bytes of padding
			// console.put('base85 byte length: ', buffer.byteLength);
			for (auto i = 0, p = 0; i < dataLength; i += 5, p += 4) {
				auto num = (((base[i] * 85 + base[i + 1]) * 85 + base[i + 2]) * 85 + base[i + 3]) * 85 + base[i + 4];
				// console.put("set byte to val:", p, num, String.fromCodePoint(num >> 24), String.fromCodePoint((num >> 16) & 0xff),
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
			if (end < 0) { err("Missing base64 end delimiter"); }
			// strip optional padding
			if (text[bufEnd - 1] == '=') { // 1st padding
				bufEnd--;
				pad = 1;
				if (text[bufEnd - 1] == '=') { // 2nd padding
					bufEnd--;
					pad = 2;
				}
			}
			// console.put('binary char length: ', bufEnd - at);

			// first run decodes into base64 int values, and skip the spaces
			byte base[bufEnd - at];
			auto p = 0;
			while (at < bufEnd) {
				auto code = lookup64[(short) text.charCodeAt(at)];  // console.put('bin: ', next, code);
				if (code > 64) { err("Invalid base64 character"); }
				if (code < 64) { base[p++] = code; }
				// else skip spaces
				at++;
			}
			at = end + 1;
			proceed();  // skip '}'
			// check length
			if ((pad and (p + pad) % 4 != 0) or (!pad and p % 4 == 1)) {
				err("Invalid base64 stream length");
			}

			// second run decodes into actual binary data
			auto len = int(p * 0.75);
			int code1, code2, code3, code4 = 0;
			int buffer[len];
			auto *bytes = reinterpret_cast<byte *>(buffer);// views:
			// console.put('binary length: ', len);
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
			// console.put('binary decoded length:', p);
//			buffer.encoding = "b64";
			return Node(buffer);
		}
	};

	bool isDigit(codepoint c) {
		return (c >= '0' and c <= '9') or atoi1(c) != -1;
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
		//todo simplify: and not is_grouper(lastNonWhite)
	}

	bool skipBorders(char ch) {// {\n} == {}
		if (next == 0)return true;
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
#if RUNTIME_ONLY
		return false;
#else
		return node.name.in(functor_list);
#endif
	}

	// todo ill-conceived separator
	bool is_grouper(codepoint bracket) {
		return contains(separator_list, bracket) or contains(grouper_list, bracket);
	}


	Type getType(codepoint bracket) {
		switch (bracket) {
			//				https://en.wikipedia.org/wiki/ASCII#Control_code_chart
			//				https://en.wikipedia.org/wiki/ASCII#Character_set
			case '\x0E': // Shift Out close='\x0F' Shift In
			case u'﹛': // ﹜
			case u'｛': // ｝
			case '{':
				return Type::objects;
			case u'⸨': // '⸩'
			case '(':
				return Type::groups;
			case u'﹝': // ﹞
			case u'〔': // 〕
			case U'［': // ］ FULLWIDTH
			case '[':
				return patterns;
		}
		err("unknown bracket type "s + bracket);
		return errors;
	}

// ":" is short binding a b:c d == a (b:c) d
// "=" is number-binding a b=c d == (a b)=(c d)   todo a=b c=d
// special : close=' ' : single value in a list {a:1 b:2} ≠ {a:(1 b:2)} BUT a=1,2,3 == a=(1 2 3)
// special : close=';' : single expression a = 1 + 2
// significant whitespace a {} == a,{}{}
// todo a:[1,2] ≠ a[1,2] but a{x}=a:{x}? OR better a{x}=a({x}) !? but html{...}
// reason for strange name import instead of parse is better IDE findability, todo rename to parseNode()?
	Node &valueNode(codepoint close = 0, Node *parent = 0) {
		// A JSON value could be an object, an array, a string, a number, or a word.
		Node current;
		current.parent = parent;
		current.setType(groups);// may be changed later, default (1 2)==1,2
#ifdef DEBUG
		current.line = &line;
#endif
		auto length = text.length;
		int start = at;// line, expression, group, … start
//		loop:
		white();// insignificant whitespace HERE
		while (ch and at <= length) {
//			white()  significant whitespace   1+1 != 1 +1 = [1 1]
			if (previous == '\\') {// escape ANYTHING
				proceed();
				continue;
			}
			if (ch == close) { // (…) {…} «…» ... “‘ part of string
				if (ch == 0 or /*ch == 0x0E or*/ ch == ' ' or ch == '\n' or ch == '\t' or ch == ';' or ch == ',');
					// keep ';' ',' ' ' for further analysis (?)
				else // drop brackets
					proceed(); // what else??
				break;
			}// todo: merge <>
			if (closing(ch, close)) { // 1,2,3;  «;» closes «,» list
				break;
			}// inner match ok

			if (contains(opening_special_brackets, ch)) {
				// overloadable grouping operators, but not builtin (){}[]
				let grouper = ch;
				proceed();
				auto body = valueNode(closingBracket(grouper));
				Node group(grouper);
				group.setType(operators, false);// name==« (without »)
				group.add(body);
//				group.type = type("group")["field"]=grouper;
				current.add(group);
				continue;
			}
			switch (ch) {
//				https://en.wikipedia.org/wiki/ASCII#Control_code_chart
//				https://en.wikipedia.org/wiki/ASCII#Character_set

				case u'﹝': // ﹞
				case u'〔': // 〕
				case U'［': // ］ FULLWIDTH
				case '[':
				case u'⸨': // '⸩'
				case '(':
				case '\x0E': // Shift Out close='\x0F' Shift In
				case u'﹛': // ﹜
				case u'｛': // ｝
				case '{': {
					let bracket = ch;
					auto type = getType(bracket);
					bool asListItem =
							lastNonWhite == ',' or lastNonWhite == ';' or (previous == ' ' and lastNonWhite != ':');
					if (checkAmbiguousBlock(current, parent)) {
						warn("Ambiguous reading could mean a{x} or a:{x} or a , {x}"s + position());
					}
					if (type == patterns)asListItem = false;
					proceed();
					Node object = Node().setType(type);
#ifdef DEBUG
					object.line = &line;
#endif
					// wrap {x} … or todo: just don't flatten before?
					Node objectValue = valueNode(closingBracket(bracket), parent ? parent : &current.last());
					object.addSmart(objectValue);
					if (type != patterns)
						object = object.flat();
					object.setType(type, false);
					object.separator = objectValue.separator;
					if (asListItem)
						current.add(object);
					else
						current.addSmart(object);
					break;
				}
//					if (checkAmbiguousBlock(current, parent))
//						warn(position() + "\nAmbiguous reading could mean a{x} or a:{x} or a , {x}");
//					proceed();
//					Node pattern = Node().setType(Type::patterns);
//					Node patternValue = valueNode(']', &current.last());
//					pattern.add(patternValue);
//					if (patternValue.kind == expression or patternValue.kind == groups)
//						pattern = patternValue.setType(patterns, false);
//					current.add(pattern);// a[b] ≠ (a b)
////					current.addSmart(pattern);// a[b] ≠ (a b) always preserve pattern (?)
//					break;
//				}
//
//					bool add_to_last = true;// a b(c) vs a b (c)
//					if (checkAmbiguousBlock(current, parent)) {
//						warn("Ambiguous reading could mean a{x} or a:{x} or a , {x}"s + position());
//						add_to_last = false;
//					}
//					proceed();
//					Node group = Node().setType(Type::groups);
//					Node groupValue = valueNode(')', &current.last());
//					group.addSmart(groupValue);
//					if (groupValue.kind == objects)
//						group = groupValue.setType(groups);// flatten hack
//					if (add_to_last)
//						current.addSmart(group);
//					else
//						current.add(group);
//					break;
//			}// lists handled by ' '!
				case '}':
				case ')':
				case ']':// ..
//					break loop;// not in c++
					err("wrong closing bracket");
//				case '+': // todo WHO writes +1 ?
				case '-':
				case '.':
					if (isDigit(next) and
					    (previous == 0 or contains(separator_list, previous) or is_operator(previous)))
						current.addSmart(numbero());// (2+2) != (2 +2) !!!
					else if (ch == '-' and next == '.')// todo bad criterion 1-.9 is BINOP!
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
				case '`': {// strings and templates
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
				case '=': { // assignments, declarations and map key-value-pairs
					// todo {a b c:d} vs {a:b c:d}
					Node &key = current.last();
					bool add_raw = current.kind == expression or key.kind == expression or
					               (current.last().kind == groups and current.length > 1);
					bool add_to_whole_expression = false; // a b : c => (a b):c  // todo: symbol :a !
					if (previous == ' ' and (next == ' ' or next == '\n'))// and lastNonWhite !=':' …
						add_to_whole_expression = true;
					if (is_operator(previous))
						add_raw = true;// == *=

					char prev = previous;// preserve
					Node op = any_operator();// extend *= ...
					if (not(op.name == ":" or (data_mode and op.name == "=")))
						add_raw = true;// todo: treat ':' as implicit constructor and all other as expression for now!
					if (op.name.length > 1)
						add_raw = true;
					if (current.kind == expression)
						add_raw = true;
					if (add_raw) {
						current.add(op.setType(operators)).setType(expression);
					}
					char closer;// significant whitespace:
					if (ch == '\n') closer = ';';// a: b c == a:(b c) newline or whatever!
					else if (ch == INDENT) {
						closer = DEDENT;
						if (not current.separator)
							current.separator = '\n';// because!
						proceed();
						white();
					} else if (ch == ' ') closer = ';';// a: b c == a:(b c) newline or whatever!
					else closer = ' ';// immediate a:b c == (a:b),c
					Node &val = *valueNode(closer, &key).clone();// applies to WHOLE expression
					if (add_to_whole_expression and current.length > 1 and not add_raw) {
						if (current.value.node)todo("multi-body a:{b}{c}");
						current.setType(Type::keyNode, false);// lose type group/expression etc ! ok?
						// todo: might still be expression!
//						object.setType(Type::valueExpression);
						current.value.node = &val;
					} else if (add_raw) {  // complex expression are not simple maps
						current.add(val);
					} else {
						setField(key, val);
					}
					break;
				}
				case INDENT: {
					proceed();
					if (current.separator == ',') {
						warn("indent block within list");
						ch = '\n';// we assume it was not desired;)
					} else {
						Node element = valueNode(DEDENT);// todo stop copying!
						current.addSmart(element.flat());
						if (not current.separator)
							current.separator = '\n';// because
						continue;
					}
				}
				case '\n': // groupCascade
				case '\t': // only in tables
				case ';': //
				case ',': {
					if (skipBorders(ch)) {
						proceed();
						continue;
					}
					// ambiguity? 1+2;3  => list (1+2);3 => list  ok!
					if (current.separator != ch) {// and current.length > 1
						// x;1+2 needs to be grouped (x (1 + 2)) not (x 1 + 2))!
						if (current.length > 1 or current.kind == expression) {
							Node neu;// wrap x,y => ( (x y) ; … )
//							neu.kind = current.kind;// or groups;
							neu.kind = groups;
							neu.parent = parent;
							neu.separator = ch;
							neu.add(current);
							current = neu;
						} else
							current.separator = ch;
						char closer = ch;// need to keep troughout loop!
						while (ch == closer) {// same separator a , b , c
							proceed();
							Node element = valueNode(closer);// todo stop copying!
							current.add(element.flat());
						}
						break;
					}
					// else fallthough!
					current.separator = ch;
				}
				case ' ': // possibly significant whitespace not consumed by white()
				{
					if (not current.separator)
						current.separator = ch;
					proceed();
					white();
					break;
				}
				case '#':
					if (next == ' ') {
						comment();
						continue;
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
					// todo: what a flimsy criterion:
					bool addFlat = lastNonWhite != ';' and previous != '\n';
					Node node = expressione(close);//word();
#ifdef DEBUG
					node.line = &line;
#endif
					if (contains(import_keywords, (chars) node.first().name.data)) { //  use, include, require …
						// import IF not in data mode
						if (current.first() == "from")
							node = parseFile(current[1].name);
						else if (node.empty()) {
							white();
							node = parseFile(identifier());
						} else
							node = parseFile(node.last().name);
//							node = parseFile(node.values().first().name);// todo
					}
#ifndef RUNTIME_ONLY
					if (precedence(node) or operator_list.has(node.name)) {
						node.kind = operators;
//						if(not isPrefixOperation(node))
//						if(not contains(prefixOperators,node))
					}
#endif
					if (node.kind == operators and ch != ':') {
						if (isFunctor(node))
							node.kind = functor;// todo: earlier
						else current.kind = expression;
					}
					if (node.length > 1 and addFlat) {
						for (Node arg:node)current.add(arg);
						current.kind = node.kind;// was: expression
					} else {
						if (current.last().kind == operators)
							current.addSmart(&node.flat());
						else
							current.add(&node.flat());
					}
				}
			}
		}

		Node &result = current.flat();
		return *result.

				clone();
	};

};


float precedence(char group) {
	if (group == 0)return 1;
	if (group == '}')return 1;
	if (group == ']')return 1;
	if (group == ')')return 1;
	if (group == 0x0E or group == 0x0F)
		return 1.1;
	if (0 < group and group < 0x20)return 1.5;
	if (group == '\n')return 2;
	if (group == ';')return 3;
	if (group == ',')return 4;
	if (group == ' ')return 5;
	if (group == '_')return 6;
//	error("unknown precedence for symbol: "s+group);
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
	error1("");// assert_throws test
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

#ifndef NO_TESTS
#ifndef RUNTIME_ONLY

#import "tests.cpp"
#include "WebServer.hpp"

#endif
#endif

// todo: merge + cleanup all these eval parse run compile emit interpret
Node run(String source) {
#if RUNTIME_ONLY
	error("RUNTIME_ONLY");
	return NIL;
#else
	return emit(source);
#endif
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
		put(s.data);
	else {
		char tmp = s.data[s.length];
		s.data[s.length] = 0;// hack not thread-safe
		put(s.data);
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

String load(String file) {
#if WASM
	return "";
#else
	FILE *ptr;
	ptr = fopen(file, "rb");  // r for read, b for binary
	if (!ptr)error("File not found "s + file);
	fseek(ptr, 0L, SEEK_END);
	int size = ftell(ptr);
	unsigned char buffer[size];
	fseek(ptr, 0L, SEEK_SET);
	int ok = fread(buffer, sizeof(buffer), size, ptr);
	if (!ok)error("Empty file or error reading "s + file);
	String *binary = new String((char *) buffer, size, false);
//	assert_equals(binary->length, size);
	return *binary;
#endif
}

Node compile(String file) {
	String code = load(file);
#if RUNTIME_ONLY
	return Node("Wasp compiled without emitter");
#else
	return emit(code);
#endif
}

int run_wasm_file(chars file) {
	let buffer = load(String(file));
#if RUNTIME_ONLY
	error("RUNTIME_ONLY");
	return -1;
#else
	return run_wasm((bytes) buffer.data, buffer.length);
#endif
}


//static
Node parseFile(String filename) {
	String found = findFile(filename);
	if (not found)error("file not found "s + filename);
	else info("found "s + found);
	if (found.endsWith("wast") or found.endsWith("wat")) {
#ifndef WASM
		system("/usr/local/bin/wat2wasm "s + found);
#endif
		found = found.replace("wast", "wasm");
		// and use it:
	}
	if (found.endsWith("wasm")) {// handle in analysis, not in valueNode
		//			read_wasm(found);
		auto import = Node("include").setType(operators);
		import.add(new Node(found));
		return import;
	} else if (found.endsWith("wasp"))
		return Wasp().setFile(found).parse(readFile(found));
	else if (not found.contains(".")) {
		found = findFile(filename + ".wasp");
		if (found)return parseFile(filename + ".wasp");
		found = findFile(filename + ".wasm");
		if (found)return parseFile(filename + ".wasm");
		else
			error("Can't find module "s + filename);
	} else
		error("Unknown extension in file "s + filename);
	return NIL;
}

void usage() {
	print("wasp is a new compiled programming language");
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
		put("Hello Wasp 🐝");
		if (argp >= 2) {
			String arg = argv[1];
			if (arg.endsWith(".wasp")) {
				return compile(arg).value.longy;
			}
			if (arg.endsWith(".wasm"))
				run_wasm_file(arg);
#ifndef NO_TESTS
			if (arg == "test" or arg == "tests")
				testCurrent();
#endif
			if (arg == "app" or arg == "start" or arg == "webview" or arg == "browser" or arg == "run" or
			    arg == "repl") {
//				start_server(9999);
#ifdef GRAFIX
				init_graphics();
#else
				print("wasp compiled without sdl/webview");
#endif
			}
#ifdef SERVER
			if (arg.contains("serv"))
				start_server(9999);
#endif
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
		put(args);
		current += strlen0(args)+1;
#endif
#ifdef SERVER
		//		start_server(9999);
				std::thread go(start_server, 9999);
#endif
#ifdef WEBAPP
		put("\nWEBAPP!");
		// handing over to V8, we need to call testCurrent() from there!
		init_graphics(); //
#endif
#ifndef NO_TESTS // RUNTIME_ONLY
		testCurrent();// needs init_graphics in WEBAPP to run wasm!
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
	return -42;// wasm-ld dummy should not be called, ok to test run_wasm("wasp.wasm")
//	return main(0, 0);
}
//#endif
#endif

