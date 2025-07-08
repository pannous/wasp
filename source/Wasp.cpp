//#pragma once
#include "Node.h"
#include "Wasp.h"
#include "Util.h"
#include "String.h" // variable has incomplete type
#include "Backtrace.h" // header ok in WASM
#include "wasm_helpers.h"
#include "wasm_emitter.h"
#include "wasm_runner.h"
#include "console.h"
//#include "tests.h"
#if WASM or LINUX
bool isnumber(char c) { return c >= '0' and c <= '9'; }
// why cctype no work?
#else

#include <cctype> // isnumber

#endif

#include <cstdlib> // OK in WASM!


#ifndef PURE_WASM

#include "stdio.h" // FILE

#endif

#include <math.h>


#ifndef RUNTIME_ONLY

#include "WebServer.hpp"
//#include "wasm_merger_wabt.h"
#include "WebApp.h"

#endif

#include "WitReader.h"

int __force_link_parser_globals = 1; // put this in other .cpp files to force linking parser globals:
// extern int __force_link_parser_globals;
// int* __force_link_parser_globals_ptr = &__force_link_parser_globals;


int SERVER_PORT = 1234;
//bool eval_via_emit = false;// not all tests yet
bool eval_via_emit = true; // << todo!  assert_is(…)

// WE DON'T NEED THIS in main, we can just use CANONICAL ABI lowering, e.g. for strings: [i32, i32]
// WE DO NEED THIS for easier WASM to js calls, avoiding new_string
bool use_wasm_strings = false; // stringref in wat // used to work with wasm-as but Chrome removed flag?
bool use_wasm_structs = false; // struct in wat
bool use_wasm_arrays = false; // array in wat


// get home dir :
/*#include <unistd.h>
#include <sys/types.h>
#include <pwd.h>*/

#ifndef RUNTIME_ONLY

#include "Interpret.h"

#endif


#ifndef WASM

#include <thread>
#include "ErrorHandler.h"
#include "Paint.h"

#endif

#define err(m) err1("%s:%d\n%s"s%__FILE__%__LINE__%m)
#define parserError(m) err1("%s:%d\n%s"s%__FILE__%__LINE__%m)

bool isalpha0(codepoint c) {
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
}


bool data_mode = true;
// todo ! // tread '=' as ':' instead of keeping as expression operator  WHY would we keep it again??

Node &wrapPattern(Node &n) {
    // y[1] => y:[1]
    if (n.kind == patterns)return n;
    if (n.kind == groups or n.kind == objects)return n.setKind(patterns, false);
    Node &wrap = *new Node(patterns);
    wrap.add(n);
    return wrap;
}

List<String> falseKeywords = {"false", "False", "no", "No", "⊥", "✖", "✖\uFE0F", "wrong", "Wrong"};
// 𐄂 vs times! ƒ is function
List<String> trueKeywords = {"true", "True", "yes", "Yes", "⊤", "ok", "OK", "✔", "☑", "✓", "✓\uFE0F", "✔\uFE0F"};
// correct, right, valid, ok, good, correct, proven
List<String> nilKeywords = {
    "NULL", "nil", "null", "none", "None", "nothing", "Nothing", "⊥", "∅", "∅\uFE0F", "ø", "empty"
};

//List<codepoint>
//List<chars> circumfixOperators/*Left*/ = {"‖", 0};
//codepoint circumfixOperators[] = {u'‖', 0};
// minus common ones u'(', u'{', u'[',


codepoint opening_special_brackets[] = {
    u'‖', u'⟨', u'﴾', u'﹙', u'（', u'⁽', u'⸨', u'﹛', u'｛', u'﹝', u'〔', u'〘',
    u'〚', u'〖', u'【', u'『', u'「', u'｢', u'⁅', u'«', u'《', u'〈',
    u'︷', u'︵', u'﹁', u'﹃', u'︹', u'︻', u'︽', 0
};
//codepoint closing_special_brackets[] = {}

codepoint separator_list[] = {' ', ',', ';', ':', '\n', '\t', 0};
// todo:
//codepoint grouper_list[] = {'(', ')', '{', '}', '[', ']', u'«', u'»', 0, };
codepoint grouper_list[] = {' ', ',', ';', ':', '\n', '\t', '(', ')', '{', '}', '[', ']', u'«', u'»', 0};
// () ﴾ ﴿ ﹙﹚（ ） ⁽ ⁾  ⸨ ⸩
// {} ﹛﹜｛｝    ﹝﹞〔〕〘〙  ‖…‖
// [] 〚〛〖〗【】『』「」｢｣ ⁅⁆
// <> ⟨⟩ «» 《》〈〉〈〉
// ︷ ︵ ﹁ ﹃ ︹ ︻ ︽
// ︸ ︶ ﹂ ﹄ ︺ ︼ ︾

const char *validUrlSchemes[] = {"http", "https", "ftp", "file", "mailto", "tel", "//", nullptr}; /// CAREFUL

// predicates in of on from to
// todo split keywords into binops and prefix functors
chars import_keywords[] = {"use", "require", "import", "include", "using", 0};
// todo aliases need NOT be in this list:
// todo library functions need NOT be in this list (loaded when though?) "log10", "log₁₀", "log₂", "ln", "logₑ",
// todo special UTF signs need NOT be in this list, as they are identified as operators via utf range
//chars operator_list0[] =  // "while" ...
// todo ∨ ~ v ~ versus! "³", "⁴", define inside wasp
//  or  & and ∨ or ¬  or  ~ not → implies ⊢ entails, proves ⊨ entails, therefore ∴  ∵ because
// ⊃ superset ≡ iff  ∀ universal quantification ∃ existential  ⊤ true, tautology ⊥ false, contradiction
//#if WASI
//List<chars> operator_list;
//#else
List<chars> operator_list = {
    "return", "+", "-", "*", "/", ":=", "≔", "else", "then" /*pipe*/ ,
    "is", "equal", "equals", "==", "!=", "≠", "#", "=", "." /*attribute operator!*/,
    "not", "!", "¬", "|", "and", "or", "&", "++", "--", "to", "xor", "be", "?", ":", "nop",
    "pass", "typeof",
    "upto", "…", "...", "..", "..<" /*range*/,
    "%", "mod", "modulo", "⌟", "2⌟", "10⌟", "⌞", "⌞2", "⌞10",
    // "plus", "times", "add", "minus", // todo via aliases.wasp / SPO PSO verb matching
    "use", "using", "include", "require", "import", "module",
    "<=", ">=", "≥", "≤", "<", ">", "less", "bigger", "⁰", "¹", "²", "×", "⋅", "⋆", "÷",
    "^", "∨", "¬", "√", "∈", "∉", "⊂", "⊃", "in", "of", "by", "iff", "on", "as", "^^", "^",
    "**",
    "from", "#", "$", "ceil", "floor", "round", "∧", "⋀", "⋁", "∨", "⊻",
    "abs" /* f64.abs! */, /* "norm", "‖" acts as GROUP, not as operator (when parsing) */
    // norm ‖…‖ quite complicated for parser! ‖x‖ := √∑xᵢ²
};
//#endif


Map<String, List<String> > aliases;
Map<int64/*hash*/, String *> hash_to_normed_alias;

bool aliases_loaded = true; // DON't load aliases!
//bool aliases_loaded = false;

void load_aliases() {
    //    aliases.setDefault(List<String>());// uff!?
    hash_to_normed_alias.setDefault(new String());
    data_mode = true;
    auto list = parseFile("lib/aliases.wasp");
    for (auto key: list) {
        auto normed = key.name;
        aliases[normed] = key.toList();
        for (auto alias: key) {
            auto variant = alias.name;
            hash_to_normed_alias[variant.hash()] = &normed.clone();
        }
    }
    // check(hash_to_normed_alias["times"s.hash()] == "*"s);
    //	check(hash_to_normed_alias["mod_d"s.hash()]=="mod"s);
    aliases_loaded = true;
}

String &normOperator(String &alias) {
    if (alias.empty())return alias;
    if (not aliases_loaded)load_aliases();
    auto hash = alias.hash();
    if (not hash_to_normed_alias.has(hash))
        return alias; // or NIL : no alias
    auto normed = hash_to_normed_alias[hash];
    if (not normed->empty() and alias != normed)
        trace(alias + " operator normed to " + normed);
    if (alias == "is")return string("eq");
    return *normed;
}

//	bool is_identifier(char ch) {
bool is_identifier(codepoint ch) {
    if (ch == '_' or ch == '$' or ch == '@')return true;
    if (ch == '-' or ch == u'‖' or ch == L'‖' or ch == '/')return false;
    if (is_operator(ch, false))
        return false;
    if (ch < 0 or ch > 128)return true; // all UTF identifier todo ;)
    return ('a' <= ch and ch <= 'z') or ('A' <= ch and ch <= 'Z'); // ch<0: UNICODE
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
        case '<':
            return '>'; // tags / generics
        case '\x0E':
            return '\x0F'; // Shift In closes Shift Out??
        case '\x0F':
            return '\x0E'; // Shift Out closes '\x0F' Shift In
        case u'⟨':
            return u'⟩';
        case u'‖':
            return u'‖';
        case u'⸨':
            return u'⸩';
        case u'﹛':
            return u'﹜';
        case u'｛':
            return u'｝'; //  ︷
        case u'﹝':
            return u'﹞'; // ︸
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
        case u'"':
            return u'"';
        case u'`':
            return u'`';
        case u'\'':
            return u'\'';
        default:
            error("unknown bracket "s + bracket);
    }

    return 0;
}


//	List<String> operators; // reuse functions!
//	if(is_grapheme_modifier(ch))parseError("multi codepoint graphemes not");
// everything that is not an is_identifier is treated as operator/symbol/identifier?
// NEEDs complete codepoint, not just leading char because	☺ == e2 98 ba  √ == e2 88 9a
bool is_operator(codepoint ch, bool check_identifiers /*= true*/) {
    // todo is_KNOWN_operator todo Julia
    if (ch == '-')return true; // ⚠️ minus vs hyphen!
    if (check_identifiers && is_identifier(ch))
        return false;
    //	0x0086	134	<control>: START OF SELECTED AREA	†
    //    if (ch == '_' or ch == '$' or ch == '@')return false; // part of identifier in VARIABLEs
    if (ch == U'∞')return false; // or can it be made as operator!?
    if (ch == U'⅓')return false; // numbers are implicit operators 3y = 3*y
    if (ch == U'∅')return false; // Explicitly because it is part of the operator range 0x2200 - 0x2319
    //		0x20D0	8400	COMBINING LEFT HARPOON ABOVE	⃐
    //		0x2300	8960	DIAMETER SIGN	⌀
    if (0x207C < ch and ch <= 0x208C) return true; // ⁰ … ₌
    if (0x2190 < ch and ch <= 0x21F3) return true; // ← … ⇳
    if (0x2200 < ch and ch <= 0x2319) return true; // ∀ … ⌙
    if (ch == u'¬')return true;
    if (ch == u'＝')return true;
    //    if (ch == u'#' and prev=='\n' or next == ' ')return false;
    if (ch == u'#') return true; // todo: # is NOT an operator, but a special symbol for size/count/length
    if (operator_list.has(String(ch)))
        return true;

    //		if(ch=='=') return false;// internal treatment
    if (ch > 0x80)
        return false; // utf NOT enough: ç. can still be a reference!
    if (isalnum0(ch)) return false; // ANY UTF 8
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
    if (not elem)return false;
    do {
        if (eq(match, *elem))
            return true;
    } while (*elem++);
    return false;
}


class Wasp {
    String text = EMPTY;
    String file = EMPTY; // possible source
    int at = -1; //{};            // The index of the current character PLUS ONE todo

    char lastNonWhite = 0;
    //	char previous = 0;
    //	char ch = 0;            // The current character
    //	char* point = 0;            // The current character
    //	char next = 0; // set in proceed()

    codepoint ch = 0; // The current character
    codepoint next = 0; // set in proceed()
    codepoint previous = 0;

    String line;
    int lineNumber{}; // The current line number
    int columnStart{}; // The index of column start char

    ParserOptions parserOptions;

    int indentation_level = 0;
    bool indentation_by_tabs = false; // not a compiler option but inferred from indentation!

#define INDENT 0x0F // 	SI 	␏ 	^O 		Shift In
#define DEDENT 0x0E //  SO 	␎ 	^N 		Shift Out

    //    void parserError(String message) {
    //        String msg = message;
    //        msg += position();
    //        auto error = new SyntaxError(msg);
    //        error->at = at;
    //        error->lineNumber = lineNumber;
    //        error->columnNumber = at - columnStart;
    //        error->file = file.data;
    //        err(msg);
    //    }
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
            if (indentation_level > 0 and not indentation_by_tabs) {
                char proceed1 = proceed();
                if (proceed1)
                    parserError("mixing tabs and spaces for indentation");
            }
            indentation_by_tabs = true;
            tabs++;
            offset++;
        }
        if (tabs > 0 and text[offset] == ' ')
            parserError("ambiguous indentation, mixing tabs and spaces");
        while (text[offset] == ' ') {
            if (indentation_level > 0 and indentation_by_tabs)
                if (proceed())
                    parserError("mixing tabs and spaces for indentation");
            indentation_by_tabs = false;
            tabs = tabs + 1. / spaces_per_tab;
            offset++;
        }
        if (tabs > 0 and text[offset] == '\t')
            parserError("ambiguous indentation, mixing tabs and spaces");
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
        if (closer == '>')
            return ch == '>' or ch == '\n'; // nothing else closes!
        if (closer == ' ' and ch == '>' and parserOptions.use_generics) // todo better
            return true;
        if (ch == closer)
            return true;
        if (group_precedence(ch) <= group_precedence(closer))
            return true;
        if (ch == INDENT)
            return false; // quite the opposite
        if (ch == DEDENT and not(closer == '}' or closer == ']' or closer == ')'))
            return true;
        if (ch == '}' or ch == ']' or ch == ')') {
            // todo: ERROR if not opened before!
            //				if (ch != close and close != ' ' and close != '\t' /*???*/) // cant debug wth?
            return true;
        } // outer match unresolved so far

        if (group_precedence(ch) <= group_precedence(closer))
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

    Wasp &setParserOptions(ParserOptions p) {
        this->parserOptions = p;
        return *this;
    }

    // Return the enclosed parse function. It will have access to all of the above functions and variables.
    //    Node return_fuck(auto source,auto options) {
    // YUCK static magically applies to new() objects too!?!


    // todo: flatten the parse->parse->read branch!!
    Node &parse(chars source0, ParserOptions options) {
        String source = source0;
        if (!source0) {
            warn("parse on empty source");
            return NUL;
        }
        parserOptions = options;
        if ((source.endsWith(".wasp") or source.endsWith(".wit")) and not source.contains("")) {
            setFile(source);
            source = readFile(findFile(source, parserOptions.current_dir));
        }
#ifndef RELEASE
        put_chars("Parsing: ");
        println(source.data);
#endif
        columnStart = 0;
        at = -1;
        lineNumber = 1;
        indentation_level = 0;
        ch = 0;
        text = source;
        while (empty(ch) and (ch or at < 0))
            proceed(); // at=0
        previous = 0;
        Node &result = valueNode(); // <<
        white();
        if (ch and ch != -1 and ch != DEDENT) {
            printf("UNEXPECTED CHAR %c", ch);
            print(position());
            parserError("Expect end of input");
            result = ERROR;
        }
        // Mark does not support the legacy JSON reviver function todo ??
        return result;
        //		return *result.clone();
    }


    Wasp &setFile(String file) {
        this->file = file;
        parserOptions.current_dir = extractPath(file);
        return *this;
    }

private:
    char escapee(char c) {
        switch (c) {
            case '"':
                return '"'; // this is needed as we allows single quote
            case '\\':
                return '\\';
            case '/':
                return '/';
            case '\n':
                return '\n'; // Replace escaped newlines in strings w/ empty string
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
        msg = msg + (s(" ").times(columnNumber - 1)) + "^^^";
        if (not file.empty()) msg = msg + "" + file + ":" + lineNumber;
        //		print(msg);
        return msg;
    }

    String err1(String m) {
        // Call error when something is wrong.
        // todo: Still to read can scan to end of line
        String msg = m;
        msg += position();
        //		msg = msg + s(" of the Mark data. \nStill to read: ") + text.substring(at - 1, at + 30) + "^^ ...";
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
            parserError("end of code");
            return ch;
        }
        // If a c parameter is provided, verify that it matches the current character.
        if (c and c != ch) {
            // todo: debug only / who cares?
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
        ch = decode_unicode_character(text.data + at); // charAt(at);
        short width = utf8_byte_count(ch);
        if (at + width >= text.length)next = 0;
        else next = decode_unicode_character(text.data + at + width);
        //		point = text.data + at;
        if (ch == '\n' or (ch == '\r' and next != '\n')) {
            lineNumber++;
            columnStart = at; // including indent
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
        if (!is_identifier(ch))
            parserError("Unexpected identifier character "s + renderChar(ch));
        int start = at;
        // subsequent characters can contain ANYTHING except operators
        while ((proceed() and is_identifier(ch)) or isDigit(ch) or isKebabBridge())
            if (is_operator(ch) and not isKebabBridge())
                break;
        int to = at;
        while (to > 0 and empty(text[to - 1]))to--;
        String key = String(text.data + start, to - start, !debug);
        return key;
    };

    // Helper: Check if character is valid in a URL.
    bool isValidUrlChar(char ch) {
        return is_identifier(ch) || isDigit(ch) || ch == '-' || ch == '_' || ch == '~' || ch == '/' || ch == ':' ||
               ch == '?' ||
               ch == '&' || ch == '=' || ch == '%' || ch == '#' || ch == '.';
    }

    // Parse a URL.
    String url() {
        // URLs should start with a valid scheme (e.g., http, https).
        //        if (!starts_with_scheme(ch))
        //            parserError("Unexpected start of URL: "s + renderChar(ch));
        int start = at;

        // Process characters valid in a URL (letters, digits, -, _, ~, /, :, ?, &, =, %, #).
        while (proceed() && (isValidUrlChar(ch)))
            if (isWhite(ch))
                break;

        int to = at;
        while (to > 0 && empty(text[to - 1])) to--;

        String url = String(text.data + start, to - start, !debug);
        return url;
    }

    Node &hexadecimal_number() {
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
        return *new Node(val);
    }

    // Parse a number value.
    Node &numbero() {
        auto sign = '\n';
        auto string = String("");
        short base = 10;
        int64 number0;
        if (ch == '0' and (next == 'x' or next == 'X'))return hexadecimal_number(); // base=16;
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

        // todo include ⅓ for consistency but 3⅓=3+⅓ ⅓π=⅓*π … !!
        while (atoi1(ch) >= 0) {
            // -1 if not
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

        if (string.contains(".") or string.contains("e-") or string.contains("E-")) {
            if (sign == '-') return *new Node(-parseDouble(string.data));
            else return *new Node(parseDouble(string.data));
        }
        if (sign == '-') {
            number0 = -parseLong(string.data);
        } else {
            number0 = +parseLong(string.data);
        }
        //		if (!isFinite(number)) {
        //			parseError("Bad number");
        //		}
        if (base != 10) todo("base "s + base);
        Node &number_node = *new Node(number0);
        if (not is_identifier(ch)) {
            return number_node;
        } else {
            //            print("IMPLICIT MULTIPLICATION!?"); // YES! now what?
            //            ch!=' ' and not is_operator(ch) and not isWhiteSpace(ch) and not is_bracket(ch) and not is_grouper(ch) and not  and not is_operator(ch) and isDigit(ch)
            Node &mult = *new Node("*");
            mult.kind = operators; // binops;
            mult.add(number_node);
            mult.add(Node(identifier()).setKind(reference).clone());
            return mult;
        }
    };

    int parseInt(char next_digit, int base) {
        return next_digit - '0';
    }

    String fromCharCode(int64 uffff) {
        // todo UTF
        return String((char) (uffff)); // itoa0(uffff);
    }

    bool end_of_text() {
        return at >= text.length;
    }

    Node &parseTemplate() {
        // chunk `xyz…$abc…uvw…${expr}…xyz` into "xyz…", "$abc","…uvw…","${expr}", "…xyz"
        // List<Node> chunks;
        Node &chunks = *new Node();
        chunks.setType(&TemplateType);
        chunks.setKind(strings); // todo:
        // chunks.setKind(templatex);  // much cleaner!
        proceed(); // skip `
        while (ch and ch != '`' and previous != '\\') {
            if (ch == '$') {
                if (next == '{') {
                    proceed(); // skip $
                    proceed(); // skip {
                    auto nodes = valueNode('}');
                    chunks.add(nodes); //.setKind(expression)); // ${1+2}
                } else {
                    chunks.add(Node(identifier()).setKind(referencex)); // $test
                }
            } else {
                int start = at;
                while (ch and ch != '`' and previous != '\\' and ch != '$')
                    proceed();
                if (start == at)break;
                String tee = text.substring(start, at);
                chunks.add(Node(tee).setKind(strings));
                // chunks.add(Node(tee.clone()).setKind(strings));
            }
        }
        proceed(); // skip ` done
        if (end_of_text())
            parserError("Unterminated string");
        return chunks;
    }

    Node quote(codepoint delim = '"') {
        if (ch == '`') return parseTemplate();
        proceed();
        int start = at;
        while (ch and ch != delim and previous != '\\')
            proceed();
        if (end_of_text())
            parserError("Unterminated string");
        String substring = text.substring(start, at);
        proceed();
        return Node(substring).setKind(strings); // DONT do "3"==3 (here or ever)!
    }

    //// Parse a string value.
    //    [[maybe_unused]] Node string2(char delim = '"') {
    //		auto hex = 0;
    //		auto i = 0;
    //		auto triple = false;
    //		auto start = at;
    //		String string;
    //
    //		// when parsing for string values, we must look for ' or " and \ characters.
    //		if (ch == '"' or ch == '\'') {
    //			delim = ch;
    //			if (next == delim and text[at + 1] == delim) { // got tripple quote
    //				triple = true;
    //				proceed();
    //				proceed();
    //			}
    //			while (proceed()) {
    //				if (ch == delim) {
    //					proceed();
    //					if (!triple) { // end of string
    //						return Node(string);
    //						return Node(text.substring(start, at - 2));
    //					} else if (ch == delim and next == delim) { // end of tripple quoted text
    //						proceed();
    //						proceed();
    //						return Node(text.substring(start, at - 2));
    ////						todo: escape
    //					} else {
    //						string += delim;
    //					}
    //					// continue
    //				}
    //				if (ch == '\\') { // escape sequence
    //					if (triple) { string += '\\'; } // treated as normal char
    //					else { // escape sequence
    //						proceed();
    //						if (ch == 'u') { // unicode escape sequence
    //							int64 uffff = 0; // unicode
    //							for (i = 0; i < 4; i += 1) {
    //								hex = parseInt(proceed(), 16);
    ////								if (!isFinite(hex)) { break; }
    //								uffff = uffff * 16 + hex;
    //							}
    //							string = string + fromCharCode(uffff);
    //						} else if (ch == '\r') { // ignore the line-end, as defined in ES5
    //							if (next == '\n') {
    //								proceed();
    //							}
    //						} else if (escapee(ch)) {
    //							string += escapee(ch);
    //						} else {
    //							break;  // bad escape
    //						}
    //					}
    //				}
    //					// else if (ch == '\n') {
    //					// control characters like TAB and LF are invalid in JSON, but valid in Mark;
    //					// break;
    //					// }
    //				else { // normal char
    //					string += ch;
    //				}
    //			}
    //		}
    //		parseError()("Bad string");
    //		return NIL;
    //	};

    // Parse an inline comment
    void inlineComment() {
        // Skip an inline comment, assuming this is one. The current character should
        // be the second / character in the // pair that begins this inline comment.
        // To finish the inline comment, we look for a newline or the end of the text.
        if (ch != '/' and ch != '#') {
            parserError("Not an inline comment");
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
        //		if (ch != '*') {
        //			parseError()("Not a block comment");
        //		}
        do {
            proceed();
            if (ch == '*' or ch == '#') {
                proceed(); // closing */ or ##
                if (ch == '/' or ch == '#') {
                    proceed();
                    return;
                }
            }
        } while (ch);
        parserError("Unterminated block comment");
    };

    // Parse a comment
    bool comment() {
        // Skip a comment, whether inline or block-level, assuming this is one.
        char preserveLast = lastNonWhite;
        //		if (ch == ';' and next == ';') { // and mode = stupid wast comments   or columnStart==0
        //		    inlineComment();
        //			return true;
        //		}
        //		if (ch == '-' and next == '-') { // and mode = stupid applescript comments
        //		    inlineComment();
        //			return true;
        //		}
        // Comments always begin with a # or / character.
        if (ch == '#') {
            if (not(empty(previous)))
                return false;
            if (empty(next) or previous == '\n' or previous == '\r' or previous == 0)
                inlineComment();
            else if (next == '*' or next == '#') {
                // #* or ### are block comments !
                proceed('#');
                blockComment();
            } else
                return false;
            previous = lastNonWhite = preserveLast;
            return true;
        }
        if (ch != '/')
            parserError("Not a comment");
        if (next == '/') {
            proceed('/');
            inlineComment();
            previous = lastNonWhite = preserveLast;
            return true;
        } else if (next == '*' or next == '#') {
            proceed('/');
            blockComment();
            previous = lastNonWhite = preserveLast;
            return true;
        } else {
            return false; // not a comment
            // division handled elsewhere
            //			parseError("Unrecognized comment");
        }
    };

    // Parse whitespace and comments.
    void white(bool skip_whitespace = false) {
        // Note that we're detecting comments by only a single / character.
        // This works since regular expression are not valid JSON(5), but this will
        // break if there are other valid values that begin with a / character!
        while (ch) {
            if (ch == '/' or ch == '#') {
                if (not comment()) return; // else loop on
                // NEWLINE IS NOT A WHITE , it has semantics
            } else if (ch == ' ' or ch == '\t' or (skip_whitespace and (ch == '\r' or ch == '\n'))) {
                proceed();
            } else
                return;
        }
    };

    static bool isNameStart(char i) {
        return (i >= 'A' and i <= 'Z') or (i >= 'a' and i <= 'z') or i == '_' or i == '$' or i == '@';
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


    Node &operatorr() {
        Node &node = *new Node(ch);
        node.value.longy = 0;
        node.setKind(operators); // todo ++
        proceed();
        while (ch == '=' or ch == previous) {
            // allow *= += ++ -- **  …
            node.name += ch;
            proceed();
        }
        if (previous == '=' and ch == '>')
            node.name += ch; // =>

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


    //	const
    static Node resolve(Node node) {
        String &symbol = node.name;
        if (falseKeywords.has(symbol)) return False;
        if (trueKeywords.has(symbol)) return True;
        if (nilKeywords.has(symbol)) return NIL;
        //		if (node.name.in(operator_list))
        if (operator_list.has(symbol))
            node.setKind(operators, false); // later: in angle!? NO! HERE: a xor {} != a xxx{}
        //		put("resolve NOT FOUND");
        //		put(symbol);
        return node;
    }

    Node &symbol() {
        if (isDigit(ch))
            return numbero();
        if (ch == '.' and (isDigit(next)))
            return numbero();
        // todo simplify?
        if ((ch == '-' or ch == '+') and (isDigit(next) or next == '.') and previous != u'‖' and
            (empty(previous) or is_operator(previous) or next == '.')) // -1 √-1 but not 2-1 x-1!
            return numbero();
        if (ch == u'‖') {
            proceed(); // todo: better ;)
            return (*new Node("‖")).add(valueNode(u'‖').clone()).setKind(operators, false);
            //			return (*new Node("abs")).setType(Kind::call, false);
        }
        if (ch == '$' and parserOptions.dollar_names and is_identifier(next)) {
            proceed(); // $name
            return *resolve(Node(identifier()).setKind(referencex)).clone(); // or op
        }
        if (is_operator(ch))
            return operatorr();
        if (is_identifier(ch))
            return *resolve(Node(identifier(), true)).clone(); // or op
        parserError("Unexpected symbol character "s + String((char) text[at]) + String((char) text[at + 1]) +
            String((char) text[at + 2]));
        return (Node &) NIL;
    }

    //	// {a:1 b:2} vs { x = add 1 2 }
    bool lookahead_ambiguity() {
        int braces = 0;
        int pos = at - 1;
        while (pos < text.length and text[pos] != 0 and text[pos] != '\n' and braces >= 0) {
            if (text[pos] == '{')braces++;
            // handle lists elsewhere! not in expression
            if (text[pos] == ',' and braces == 0)
                return true; // ambiguity because expression (1 , 2) vs ((expression 1), 2)
            if (text[pos] == ':' and braces == 0)return true; // ambiguity
            if (text[pos] == ';' and braces == 0)return true; // end of statement!
            if (text[pos] == '=' and braces == 0)
                return text[pos + 1] != '=' and text[pos - 1] != '=' and not is_operator(text[pos - 1]); // == != OK
            if (text[pos] == '}')
                braces--;
            pos++;
        }
        return false; // OK, no ambiguity
    }

    bool is_known_functor(Node node) {
        if (precedence(node))return true;
            //		else if (functor_list.has(node.name))return true;
        else return false;
    }

    Node &expressione(codepoint closer) {
        Node &node = symbol();
        if (lookahead_ambiguity())
            return node;
        // {a:1 b:2} vs { x = add 1 2 }
        Node &expressionas = *new Node();
        // set kind = expression only if it contains operator, otherwise keep it as list!!!
        expressionas.add(node);
        if (node.kind == operators) expressionas.kind = expression; //
        //		if (contains(import_keywords,node.name))
        //			closer =0;// get rest of line;
        if (closing(ch, closer)) // stop_at_space, keep it for further analysis (?)
            return expressionas;
        white();
        if (node.kind != operators) expressionas.kind = groups;
        bool tag = parserOptions.use_generics || parserOptions.use_tags; // todo, allow IFF ' < ' surrounded by spaces!
        while (ch and ch != closer and
               (is_identifier(ch) or isalnum0(ch) or (is_operator(ch) and (not tag or (ch != '<' and ch != '>'))))) {
            node = symbol(); // including operators `=` ...
            if (node.kind == operators)expressionas.kind = expression;
            expressionas.add(&node);
            white();
        }
        //		expression.name=map(children.name)
        if (expressionas.length > 1)
            return expressionas;
        else return node;
    }

    bool isDigit(codepoint c) {
        return (c >= '0' and c <= '9') or atoi1(c) != -1;
    }

    Node &setField(Node &key, Node &val) {
        // a:{b}
        if ((val.kind == groups or val.kind == patterns or val.kind == objects) and val.length == 1 and
            empty(val.name))
            val = val.last(); // singleton
        val.parent = &key; // todo bug: might get lost!
        bool deep_copy = empty(val.name) or !debug or (key.kind == reference and empty(val.name));
        if (debug) {
            // todo make sure all works even with nested node_pointer! x="123" (node 'x' (child value='123')) vs (node 'x' value="123")
            deep_copy = deep_copy or (val.kind == Kind::longs and val.name == formatLong(val.value.longy));
            deep_copy = deep_copy or (val.kind == Kind::reals and val.name == ftoa(val.value.real));
            deep_copy = deep_copy or (val.kind == Kind::bools and
                                      (val.name == "True" or val.name == "False")); // todo why check name?
            //			if(val.kind == Type::strings)
            //				debug = 1;
            //			deep_copy = deep_copy or (val.kind == Type::strings and not val.name.empty() and key.kind==reference); and ... ?
        } // shit just for debug labels. might remove!!
        // last part to preserve {deep{a:3,b:4,c:{d:'hi'}}} != {deep{a:3,b:4,c:'hi'}}

        if (val.value.longy and val.kind != objects and deep_copy) {
            if (&key == &NIL or key.isNil() or key == NIL)
                if (key.name == nil_name)
                    warn("impossible"); // if ø:3
            key.value = val.value; // direct copy value SURE?? what about meta data... ?
            key.kind = val.kind;
            check_silent(NIL.value.longy == 0)
        } else {
            key.setKind(Kind::key, true);
            if (!key.children and empty(val.name) and val.length > 1) {
                // deep copy why?
                key.children = val.children;
                key.length = val.length;
                key.kind = val.kind;
            } else if (!val.isNil())
                key.value.node = &val; // clone?
        }
        return key;
    }


    bool checkAmbiguousBlock(Node current, Node *parent) {
        // wait, this should be part of case ' ', no?
        //						a of {a:1 b:2}
        // todo : group operator as expression a ( b … c)
        return previous == ' ' and current.last().kind != operators and current.last().kind != call and
               (!parent or (parent and parent->last().kind != operators and parent->last().kind != call)) and
               lastNonWhite != ':' and lastNonWhite != '=' and lastNonWhite != ',' and lastNonWhite != ';' and
               lastNonWhite != '{' and lastNonWhite != '(' and lastNonWhite != '[' and
               lastNonWhite != '}' and lastNonWhite != ']' and lastNonWhite != ')';
        //todo simplify: and not is_grouper(lastNonWhite)
    }

    bool skipBorders(char ch) {
        // {\n} == {}
        if (next == 0)return true;
        if (lastNonWhite == ':')return true;
        if (lastNonWhite == '{' or next == '}')
            return true; // todo: nextNonWhite
        if (lastNonWhite == '(' or next == ')')return true;
        if (lastNonWhite == '[' or next == ']')return true;
        if (ch == ',' and next == ';')return true; // 1,2,3,; => 1,2,3;
        if (ch == ',' and next == '\n')return true; // 1,2,3,\n => 1,2,3;
        if (ch == ';' and next == '\n')return true; // 1,2,3,\n => 1,2,3;
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


    Kind getType(codepoint bracket) {
        switch (bracket) {
            //				https://en.wikipedia.org/wiki/ASCII#Control_code_chart
            //				https://en.wikipedia.org/wiki/ASCII#Character_set
            case '\x0E': // Shift Out close='\x0F' Shift In
            case u'﹛': // ﹜
            case u'｛': // ｝
            case '{':
                return Kind::objects;
            case u'⸨': // '⸩'
            case '(':
                return Kind::groups;
            case u'﹝': // ﹞
            case u'〔': // 〕
            case U'［': // ］ FULLWIDTH
            case '[':
                return patterns;
            case '<':
                return generics;
        }
        parserError("unknown bracket type "s + bracket);
        return errors;
    }

    Node &direct_include(Node &current, Node &node) {
        // todo: this old c-style include is not really what we want.
        // todo: instead handle `use / include / import / require` in Angle.cpp analyze!
        // especially if file.name is lib.wasm ;)
        // import IF not in data mode
        String lib;
        if (current.first() == "from")
            lib = current[1].name;
        else if (node.empty()) {
            white();
            if (ch == '"' or ch == '\'' or ch == '<')proceed(); // include "c-style" // include <cpp-style>
            lib = (identifier());
            if (ch == '"' or ch == '\'' or ch == '>')proceed();
        } else
            lib = (node.last().name);
        //		node.values(). first().name
        if (lib == "memory")
            return node; // todo ignore memory includes???
        if (not file.empty() and file.endsWith(".wit")) // todo file from where ??
            lib.replaceAllInPlace('-', '_'); // stupid kebab case!
        if (!lib.empty()) // creates 'include' node for wasm …
            node = parseFile(lib, parserOptions);
        return node;
    }

    // ":" is short binding a b:c d == a (b:c) d
    // "=" is int64-binding a b=c d == (a b)=(c d)   todo a=b c=d
    // "-" is post binding operator (analyzed in angle) OR short-binding in kebab-case
    // special : close=' ' : single value in a list {a:1 b:2} ≠ {a:(1 b:2)} BUT a=1,2,3 == a=(1 2 3)
    // special : close=';' : single expression a = 1 + 2
    // significant whitespace a {} == a,{}{}
    // todo a:[1,2] ≠ a[1,2] but a{x}=a:{x}? OR better a{x}=a({x}) !? but html{...}
    // reason for strange name is better IDE findability, todo rename to readNode() or parseNode()?
    Node &valueNode(codepoint close = 0, Node *parent = 0) {
        // A JSON value could be an object, an array, a string, a number, or a word.
        Node &actual = *new Node(); // current already used in super context
        actual.parent = parent;
        actual.setKind(groups); // may be changed later, default (1 2)==1,2
#if DEBUG
        if (line != "}") // why?
            actual.line = &line;
        actual.lineNumber = lineNumber;
        actual.column = columnStart; // todo
        actual.file = &file;
#endif
        auto length = text.length;
        int start = at; // line, expression, group, … start
        //		loop:
        white(); // insignificant whitespace HERE
        while (ch and at <= length) {
            //			white()  significant whitespace   1+1 != 1 +1 = [1 1]
            if (previous == '\\') {
                // escape ANYTHING
                proceed();
                continue;
            }
            if (ch == close) {
                // (…) {…} «…» ... “‘ part of string
                if (ch == 0 or /*ch == 0x0E or*/ ch == ' ' or ch == '\n' or ch == '\t' or ch == ';' or ch == ',');
                    // keep ';' ',' ' ' for further analysis (?)
                else // drop brackets
                    proceed(); // what else??
                close = 0; // ok, we are done
                break;
            } // todo: merge <>
            if (closing(ch, close)) {
                // 1,2,3;  «;» closes «,» list
                close = 0; // ok, we are done
                break;
            } // inner match ok


            if (contains(opening_special_brackets, ch)) {
                // overloadable grouping operators, but not builtin (){}[]
                let grouper = ch;
                proceed();
                auto body = valueNode(closingBracket(grouper));
                Node group(grouper);
                group.setKind(operators, false); // name==« (without »)
                group.add(body);
                //				group.type = type("group")["field"]=grouper;
                actual.add(group);
                continue;
            }
            switch (ch) {
                //				https://en.wikipedia.org/wiki/ASCII#Control_code_chart
                //				https://en.wikipedia.org/wiki/ASCII#Character_set
                //                case 'Ü':
                //                    print("Ü");
                case '@':
                case '$':
                    if (parserOptions.dollar_names or parserOptions.at_names)
                        actual.add(Node(identifier()).setKind(referencex));
                    else
                        actual.add(operatorr());
                    break;
                case '<':
                    if (text.substring(at, at + 5) == "<html") {
                        int to = text.find("</html>", at);
                        if (to < 0) to = text.length; // warn("unclosed html tag");
                        auto html = Node("html", strings);
                        html.value.string = &text.substring(text.find('>', at + 5) + 1, to).clone();
                        actual.add(html);
                        at = to + 7;
                        previous = '>';
                        proceed();
                        break;
                    }
                    if (text.startsWith("<script", at)) {
                        int to = text.find("</script>", at);
                        if (to < 0) to = text.length; // warn("unclosed script tag");
                        auto html = Node("script", strings);
                        html.value.string = &text.substring(text.find('>', at + 5) + 1, to).clone();
                        actual.add(html);
                        at = to + 9;
                        previous = '>';
                        proceed();
                        break;
                    }
                case '>':
                    if (not(parserOptions.use_tags or parserOptions.use_generics) or
                        (previous == ' ' and next == ' ')) {
                        Node &op = operatorr();
                        actual.add(op);
                        actual.kind = expression;
                        continue;
                    } else if (ch == '>') {
                        //                        actual.setType(parserOptions.use_generics ? generics : tags, false); NOT ON ELEMENT!
                        return actual;
                    } else {
                        if (next == '/') todo("closing </tags>");
                    } //fall through:
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
                    codepoint bracket = ch;
                    auto type = getType(bracket);
#if not RUNTIME_ONLY
                    bool flatten = not isFunction(actual.last());
#else
                    bool flatten = true;
#endif
                    bool addToLast = false;
                    bool asListItem =
                            lastNonWhite == ',' or lastNonWhite == ';' or (previous == ' ' and lastNonWhite != ':');
                    if (checkAmbiguousBlock(actual, parent)) {
                        if (parserOptions.space_brace) {
                            addToLast = true; // a b {c}; => a b{c}
                            asListItem = false;
                        } else
                            warn("Ambiguous reading could mean a{x} or a:{x} or a , {x}"s + position());
                    }
                    bool specialDeclaration = false;
                    if (type == patterns) {
                        asListItem = false;
                        flatten = false;
                    }
                    if ((lastNonWhite == ')' and bracket == '{')) {
                        // declare (){} as special syntax in wiki!
                        // careful! using (1,2) {2,3} may yield hidden bug!
                        asListItem = false;
                        flatten = false;
                        specialDeclaration = true; // (){}
                        auto name = actual.first().name;
                        if (name == "while" or name == "if") // todo cleaner!
                            specialDeclaration = false; // if (){} is not a declaration!
                    }
                    proceed();
                    // wrap {x} … or todo: just don't flatten before?
                    Node &object = *new Node();
                    Node &objectValue = valueNode(closingBracket(bracket), parent ? parent : &actual.last());
                    //                    if(bracket=='[' and not data_mode )
                    //                        objectValue = wrapPattern(objectValue);
                    object.addSmart(objectValue);
                    //						object.add(objectValue);
                    if (flatten) object = object.flat();
                    object.setKind(type, false);
                    object.separator = objectValue.separator;
#if DEBUG
                    if (line != "}")
                        object.line = &line;
#endif
                    if (asListItem)
                        actual.add(object);
                    else if (addToLast)
                        actual.last().last().addSmart(object);
                        //                    else if (actual.last().kind == operators)
                        //                        actual.last().add(object);
                    else
                        actual.addSmart(object);
                    //					current.addSmart(&object,flatten);
                    if (specialDeclaration)
                        actual.kind = declaration;
                    break;
                }
                //			}// lists handled by ' '!
                case '}':
                case ')':
                case ']': // ..
                    //					break loop;// not in c++
                    parserError("wrong closing bracket");
                // case '+': // todo WHO writes +1 ?
                case '-':
                    if (parserOptions.arrow and next == '>') {
                        // a->b immediate key:value
                        proceed();
                        proceed();
                        white();
                        Node &node = valueNode(' '); // f: func() -> tuple<int,int>
                        //                        Node &node = *new Node(identifier()); // ok for now
                        Node *last = &actual.last(); // don't ref here, else actual.last gets overwritten!
                        while (last->value.node and last->kind == key)
                            last = last->value.node; // a:b:c:d
                        last->setValue({.node = &node}).setKind(key, false);
                        break; //
                        continue;
                    }
                    if (isKebabBridge())
                        parserError("kebab case should be handled in identifier");
                    if (next == '>') {
                        // -> => ⇨
                        next = u'⇨';
                        proceed();
                    }
                case '.':
                    if (isDigit(next) and
                        (previous == 0 or contains(separator_list, previous) or is_operator(previous)))
                        actual.addSmart(numbero()); // (2+2) != (2 +2) !!!
                    else if (ch == '-' and next == '.') // todo bad criterion 1-.9 is BINOP!
                        actual.addSmart(numbero()); // -.9 -0.9 border case :(
                    else {
                        Node *op = operatorr().clone();
                        actual.add(op);
                        actual.kind = expression;
                    }
                    break;
                case '"':
                case '\'': /* don't use modifiers ` ˋ ˎ */
                case u'«': // «…»
                case u'‘': // ‘𝚗𝚊𝚖𝚎’
                case u'“': // “…” Character too large for enclosing character literal type
                case '`': {
                    // strings and templates
                    if (previous == '\\')continue; // escape
                    bool matches = close == ch;
                    codepoint closer = closingBracket(ch);
                    matches = matches or (close == u'‘' and ch == u'’');
                    matches = matches or (close == u'’' and ch == u'‘');
                    matches = matches or (close == u'“' and ch == u'”');
                    matches = matches or (close == u'”' and ch == u'“');
                    if (!matches) {
                        // open string
                        if (actual.last().kind == expression)
                            actual.last().addSmart(quote(closer));
                        else
                            actual.add(quote(closer).clone());
                        break;
                    } else {
                        close = 0; // ok, we are done
                    }
                    Node id = Node(text.substring(start, at));
                    id.setKind(Kind::strings); // todo "3" could have be resolved as number? DONT do js magifuckery
                    id.setType(&TemplateType);
                    actual.add(id);
                    break;
                }
                case ':':
                case U'：':
                case U'≝':
                case U'≔': // vs ≕ =:
                case U'＝':
                case U'﹦':
                case u'←': // in apl assignment is a left arrow
                case u'⇨': // ??
                case '=': {
                    // assignments, declarations and map key-value-pairs
                    // todo {a b c:d} vs {a:b c:d}
                    Node &key = actual.last();

                    if (ch == ':' and next == '/' and key.name.in(validUrlSchemes)) {
                        key.name = key.name + url();
                        key.setKind(Kind::urls, false); // todo: Kind::urls ?
                        continue;
                        //                        break;
                    }
                    bool add_raw = actual.kind == expression or key.kind == expression or
                                   (actual.last().kind == groups and actual.length > 1);
                    bool add_to_whole_expression = false;
                    if (previous == ' ' and (next == ' ' or next == '\n') and not parserOptions.colon_immediate)
                        add_to_whole_expression = true; // a b : c => (a b):c  // todo: symbol :a as in ruby?
                    if (is_operator(previous))
                        add_raw = true; // == *=
                    if(previous==')' and function_keywords.has(actual.first().name)) {
                        actual.setKind(declaration, false);
                        add_raw= true; // fun():body
                    }
                    //					char prev = previous;// preserve
                    Node op = operatorr(); // extend *= ...
                    if (next == '>' and parserOptions.arrow)
                        proceed(); // =>
                    if (not(op.name == ":" or (parserOptions.data_mode and op.name == "=")))
                        add_raw = true;
                    if (op.name.length > 1)
                        add_raw = true;
                    if (actual.kind == expression)
                        add_raw = true;
                    if (add_raw) {
                        actual.add(op.setKind(operators)).setKind(expression);
                    }
                    char closer; // significant whitespace:
                    if (ch == '\n') closer = ';'; // a: b c == a:(b c) newline or whatever!
                    else if (op == ":" and parserOptions.colon_immediate)
                        closer = ' '; // immediate a:b c == (a:b),c
                    else if (ch == INDENT) {
                        closer = DEDENT;
                        if (not actual.separator)
                            actual.separator = '\n'; // because!
                        proceed();
                        white();
                    } else if (ch == ' ') closer = ';'; // a: b c == a:(b c) newline or whatever!
                    else closer = ' ';
                    Node &val = valueNode(closer, &key); // applies to WHOLE expression
                    if (add_to_whole_expression and actual.length > 1 and not add_raw) {
                        if (actual.value.node) todo("multi-body a:{b}{c}");
                        actual.setKind(Kind::key, false); // lose type group/expression etc ! ok?
                        // todo: might still be expression!
                        //						object.setType(Type::valueExpression);
                        actual.value.node = &val;
                    } else if (add_raw) {
                        // complex expression are not simple maps
                        actual.add(val);
                    } else {
                        setField(key, val);
                    }
                    break;
                }
                case INDENT: {
                    proceed();
                    if (actual.separator == ',') {
                        warn("indent block within list");
                        ch = '\n'; // we assume it was not desired;)
                    } else {
                        Node element = valueNode(DEDENT); // todo stop copying!
                        actual.addSmart(element.flat());
                        if (not actual.separator)
                            actual.separator = '\n'; // because
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
                    if (actual.separator != ch) {
                        // and current.length > 1
                        // x;1+2 needs to be grouped (x (1 + 2)) not (x 1 + 2))!
                        if (actual.length > 1 or actual.kind == expression) {
                            Node neu; // wrap x,y => ( (x y) ; … )
                            //							neu.kind = current.kind;// or groups;
                            neu.kind = groups;
                            neu.parent = parent;
                            neu.separator = ch;
                            neu.add(actual);
                            actual = neu;
                        } else
                            actual.separator = ch;
                        char sep = ch; // need to keep troughout loop!
                        while (ch == sep and not closing(ch, close)) {
                            // same separator a , b , c
                            proceed();
                            Node &element = valueNode(sep);
                            actual.add(element.flat());
                        }
                        break;
                    }
                    // else fallthough!
                    actual.separator = ch;
                }
                case ' ': // possibly significant whitespace not consumed by white()
                {
                    if (not actual.separator)
                        actual.separator = ch;
                    proceed();
                    white();
                    break;
                }
                case '#':
                    if (next == ' ') {
                        comment();
                        continue;
                    } else {
                        actual.add(operatorr());
                        continue;
                    }
                case '/':
                    if (ch == '/' and (next == '/' or next == '*')) {
                        comment();
                        warn("comment should have been handled before!?");
                        continue;
                    } // else fall through to default … expressione
                case '%': // escape keywords for names in wit
                    if (parserOptions.percent_names) {
                        // and…
                        proceed();
                        actual.add(Node(identifier())); // todo make sure not to mark as operator …
                        continue;
                    }
                default: {
                    // a:b c != a:(b c)
                    // {a} ; b c vs {a} b c vs {a} + c
                    // todo: what a flimsy criterion:
                    bool addFlat = lastNonWhite != ';' and previous != '\n';
                    Node &node = expressione(close); //word();
#if DEBUG
                    node.line = &line; // else via text offset ok
#endif
                    if (contains(import_keywords, (chars) node.first().name.data)) {
                        //  use, include, require …
                        node = direct_include(actual, node);
                    }
#ifndef RUNTIME_ONLY // precedence??
                    if (precedence(node) or operator_list.has(node.name)) {
                        node.kind = operators;
                        //						if(not isPrefixOperation(node))
                        //						if(not contains(prefixOperators,node))
                    }
#endif
                    if (node.kind == operators and ch != ':') {
                        if (isFunctor(node))
                            node.kind = functor; // todo: earlier
                        else actual.kind = expression;
                    }
                    if (node.length > 1 and addFlat) {
                        for (Node arg: node)actual.add(arg);
                        actual.kind = node.kind; // was: expression
                    } else {
                        if (actual.last().kind == operators)
                            actual.addSmart(&node.flat());
                        else
                            actual.add(&node.flat());
                    }
                }
            }
        }
        if (close and not isWhite(close) and close != ';' and close != ',') {
            // todo remember opening pair line
            parserError("unclosed pair "s + close);
        }
        return actual.flat();
    };

    bool isWhite(codepoint c) {
        return c == ' ' or c == '\t' or c == '\n' or c == '\r' or c == u''; // shift out
    }

    bool isKebabBridge() {
        // isHyphen(Bridge) e.g. a-b in special ids like in component model
        if (not is_identifier(next))return false; // i-- i-1
        if (parserOptions.kebab_case_plus and ch == '-')return true;
        return parserOptions.kebab_case and ch == '-' and isalpha0(previous) and not isnumber(next) and next != '=';
    }
};


float group_precedence(char group) {
    // ≠ float precedence(String operators);
    if (group == 0)return 1;
    if (group == '}')return 1;
    if (group == ']')return 1;
    if (group == ')')return 1;
    if (group == 0x0E or group == 0x0F) // indent, dedent
        return 1.1;
    if (0 < group and group < 0x20) return 1.5; // ascii control keys why?
    if (group == '\n')return 2;
    if (group == ';')return 3;
    if (group == ',')return 4;
    if (group == ' ')return 5;
    if (group == '_')return 6;
    //	parseError("unknown precedence for symbol: "s+group);
    return 999;
}


// test functions to check wasm->runtime interaction
int test42() {
    return 42;
}

int test42i(int i) {
    // used in wasm runtime test
    return 42 + i;
}

float test42f(float f) {
    return 42 + f;
}

// default args don't work in wasm! (how could they?)
float test41ff(float f = 0) {
    return 41.4 + f;
}


void not_ok() {
    error1(""); // assert_throws test
}

#if BACKTRACE
void handler(int sig) {
    void *array[10];
    size_t size;

    // get void*'s for all entries on the stack
    size = backtrace(array, 10);

    // print out all the frames to stderr
    fprintf(stderr, "Error: signal %d:", sig);
    backtrace_symbols_fd(array, size, STDERR_FILENO);
    proc_exit(0);
*/
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

// todo WE HAVE A GENERAL PROBLEM:
// 1. top level objects are not constructed
// 2. even explicit construction seems to be PER object scope (.cpp file) HOW!
void load_parser_initialization() {
    // todo: remove thx to __wasm_call_ctors
    if (operator_list.size() == 0)
        //		warn("operator_list should have been constructed in __wasm_call_ctors @ _start");
        error("operator_list should have been constructed in __wasm_call_ctors @ _start");
    //	operator_list = List<chars>(operator_list0);// wasm hack
    //	load_aliases();
}


// Mark/wasp has clean syntax with FULLY-TYPED data model (like JSON or even better)
// Mark/wasp is generic and EXTENSIBLE (like XML or even better)
// Mark/wasp has built-in MIXED CONTENT support (like HTML5 or even better)
// Mark/wasp supports HIGH-ORDER COMPOSITION (like S-expression or even better)


//struct Exception {};
//wasm-ld: error: wasp.o: undefined symbol: vtable for __cxxabiv1::__class_type_info

// 2020: WASI does not yet support C++ exceptions. C++ code is supported only with -fno-exceptions for now.
// https://github.com/WebAssembly/wasi-sdk
//struct ERR{
//public:
//	ERR() = default;
//	virtual ~ERR() = default;
//};
//char newline = '\n';
//#ifndef _main_
#define __MAIN__

// called AFTER __wasm_call_ctors() !!!
//#ifndef RUNTIME_ONLY
//Undefined symbols for architecture arm64:
//"_main", referenced from:
//implicit entry/start for main executable

//static
Node parseFile(String filename, ParserOptions options) {
    String found = findFile(filename, options.current_dir);
    if (not found)
        error("file not found "s + filename);
    else
        info("found "s + found);
    if (found.endsWith("wast") or found.endsWith("wat")) {
#ifndef WASM
        system("/usr/local/bin/wat2wasm "s + found);
#endif
        found = found.replace("wast", "wasm");
        // and use it:
    }
    if (found.endsWith("wasm")) {
        // handle in Angle.cpp analysis, not in valueNode
        //			read_wasm(found);
        auto import = Node("include").setKind(operators);
        import.add(new Node(found));
        return import;
    } else if (found.endsWith("wasp"))
        return Wasp().setFile(found).parse(readFile(found), options);
    else if (found.endsWith("wit") or found.endsWith("wai"))
        return WitReader().read(found);
    else if (not found.contains(".")) {
        found = findFile(filename + ".wasp", options.current_dir);
        if (found)return parseFile(found);
        found = findFile(filename + ".wasm", options.current_dir);
        if (found)return parseFile(found);
        else
            error("Can't find module "s + filename);
    } else
        error("Unknown extension in file "s + filename);
    return NIL;
}

void usage() {
    //    print("𓆤 Wasp is a new compiled programming language");
    print("🐝 Wasp is a new compiled programming language");
    print("wasp [console]         open interactive programming environment");
    //	print("wasp <file.wasp>       compile wasp to wasm or native and execute");
    //	print("wasp <file.wasm>       compile wasm to native and execute");
    //	print("wasp <file.html>       compile standalone webview app and execute");// bundle all wasm
    print("wasp <files>           compile and link files into binary and execute");
    print("wasp eval <code>       run code and print result");
    print("wasp serve <file/code/ø>       compile and link code into binary and execute");
    //	print("wasp compile <files>   compile and link files into binary");
    //	print("wasp test <files> 				");
    //	print("compile <files> 				");
    //	print("wasp combine <files> 				");// via combine
    //	print("wasp optimize <files>");// aka strip, via compile optimize(d)
    //	print("wasp remove <file> <functions>");// manually strip
    //	print("wasp rename <file> <function/global/module> <old> <new-name>");
    //	print("wasp move (really?) <files> 				");
    print("wasp help              see https://github.com/pannous/wasp/wiki");
    print("wasp tests ");
}

// provide stack space either through -Wl,-z,stack-size=1179648 or :
// byte stack_hack[80000];// it turns out c++ DOES somehow need space before __heap_end
// if value is too small we get all kinds of errors:
// memory access out of bounds
// THE SMALLER THE VALUE THE EARLIER IT FAILS

// wasmer etc DO accept float/double return, just not from main!
//extern "C"
// Code &compile(String code, bool clean = true); // exposed to wasp.js


int main(int argc, char **argv) {
    if (getenv("SERVER_SOFTWARE"))
        printf("Content-Type: text/plain\n\n"); // todo html
    String args;
    for (int i = 1; i < argc; ++i) args += i > 1 ? String(" ") + argv[i] : String(argv[i]);
    String path = argv[0];
    print("Wasp 🐝 "s + wasp_version);
    //   String arg=extractArg(argv,argc);

#if WASM
    initSymbols(); // todo still necessary ??
    // String args((char*)alloc(1,1));// hack: written to by wasmx todo ??
    // heap_end += strlen(args)+1; // todo WHAT IS THIS??
#endif

#if ErrorHandler
    register_global_signal_exception_handler();
#endif
    try {
        if (argc == 1) {
            // no args, just program name
            usage();
            console();
            return 0;
            //			return 42; // funny, but breaks IDE chaining
        } // else
        else if (args.endsWith(".html") or args.endsWith(".htm")) {
#if WEBAPP
            //				start_server(SERVER_PORT);
            std::thread go(start_server, SERVER_PORT);
            auto arg = "http://localhost:"s + SERVER_PORT + "/" + args;
            print("Serving "s + arg);
            open_webview(arg);
            //				arg.replaceMatch(".*\/", "http://localhost:SERVER_PORT/");
#else
            print("wasp compiled without webview");
#endif
        } else if (args.endsWith(".wasp") or args.endsWith(".angle")) {
            String wasp_code = load(args);
            return eval(wasp_code).value.longy;
        } else if (args.endsWith(".wasm")) {
            if (argc >= 3) {
#if WABT_MERGE
                merge_files(--argc, ++argv);
#else
                todo("linking files needs compilation with WABT_MERGE")
#endif
            } else
                run_wasm_file(args);
        } else if (args == "test" or args == "tests")
#if NO_TESTS
            print("wasp release compiled without tests");
#else
            testCurrent();
#endif
        else if (args.startsWith("eval")) {
            Node results = eval(args.from(" "));
            print("» "s + results.serialize());
        } else if (args == "repl" or args == "console" or args == "start" or args == "run") {
            console(); // todo args == "run" ambiguous run <file> or run repl? not if <file> empty OK
        } else if (args == "2D" or args == "2d" or args == "SDL" or args == "sdl") {
#if GRAFIX
            init_graphics();
#else
            print("wasp compiled without sdl/webview"); // todo grafix host function?
#endif
        } else if (args == "app" or args == "webview" or args == "browser") {
#if not WEBAPP
            print("must compile with WEBAPP support");
            return -1;
#endif
#if GRAFIX
            init_graphics();
#else
            print("wasp compiled without sdl/webview");
#endif
        } else if (args.startsWith("serv") or args == "server") {
#if SERVER
            std::thread go(start_server, 9999);
//				start_server(9999);
#else
            printf("Content-Type: text/plain\n\n");
            String prog = args.from(" ");
            if (fileExists(prog)) prog = load(prog); // todo: static content?
            Node result = eval(prog); // todo give args as context
            if (prog.length > 0)
                print(result.serialize());
            else
                print("Wasp compiled without server OR no program given!");
#endif
        } else if (args.contains("help"))
            print("detailed documentation can be found at https://github.com/pannous/wasp/wiki ");
        else if (args.contains("compile") or args.contains("build") or args.contains("link")) {
            Node results = eval(args.from(" ")); // todo: dont run, just compile!
            // Code &binary = compile(args.from(" "), true);
            // binary.save(); // to debug
        }
        else {
            // run(args);
            Node results = eval(args);
            // print("» "s + results.serialize()); // todo: (?) ( already in eval )
        }
        return 0; // EXIT_SUCCESS;
        //			return 42; // funny, but breaks IDE chaining
        //    } catch (Exception e) { // struct Exception {};
        //        print("Exception (…?)");
        // } catch (chars err) {
    } catch (char const *err) {
        print("ERROR");
        print(err);
    } catch (String err) {
        print("ERROR");
        print(err);
    } catch (const Error &err) {
        print("ERROR");
        print(err.message);
    } catch (SyntaxError *err) {
        print("ERROR");
        print(err->data);
    }
    //	usleep(1000000000);
    return 1; //EXIT_FAILURE;
}

//#endif

#ifndef WASI
//#ifndef RUNTIME_ONLY
//extern int main(int argp, char **argv);
//extern "C" int _start() { // for wasm-ld
//    initSymbols();
//    return -42;// wasm-ld dummy should not be called, ok to test run_wasm("wasp.wasm")
////	return main(0);
//}
//#endif
#endif


// precedence logic sits in between raw Wasp structure and concrete Angle syntax
// precedence should be in the Wasp header but the implementation is subject to moving
// like c++ here HIGHER up == lower value == more important
float precedence(String name) {
    if (eq(name, "abs"))return 0.08;
    if (eq(name, "‖"))return 0.10; // norms / abs

    if (eq(name, "."))return 0.5;
    if (eq(name, "of"))return 0.6;
    if (eq(name, "in"))return 0.7;
    if (eq(name, "from"))return 0.8;

    if (eq(name, "not"))return 1;
    if (eq(name, "¬"))return 1;
    if (eq(name, "-…"))return 1; // unary operators are immediate, no need for prescidence
    if (eq(name, "!"))return 1;
    if (eq(name, "√"))return 1; // !√1 √!-1
    if (eq(name, "^"))return 2; // todo: ambiguity? 2^3+1 vs 2^(x+1)
    if (eq(name, "**"))return 2; //
    if (eq(name, "^^"))return 2; // how did it work without??

    if (eq(name, "#"))return 3; // count
    if (eq(name, "++"))return 3;
    //	if (eq(node.name, "+"))return 3;//
    if (eq(name, "--"))return 3;

    if (eq(name, "/"))return 4.9;
    if (eq(name, "÷"))return 4.9;

    if (eq(name, "times"))return 5;
    if (eq(name, "*"))return 5;
    if (eq(name, "×"))return 5;
    if (eq(name, "add"))return 6;
    if (eq(name, "plus"))return 6;
    if (eq(name, "+"))return 6;
    if (eq(name, "minus"))return 6;
    if (eq(name, "-"))return 6;
    if (eq(name, "%"))
        return 6.1;
    if (eq(name, "rem"))return 6.1;
    if (eq(name, "modulo"))return 6.1;
    if (eq(name, "upto"))return 6.3; // range
    if (eq(name, "…"))return 6.3;
    if (eq(name, "..."))return 6.3;
    if (eq(name, ".."))return 6.3; // excluding range
    if (eq(name, "..<"))return 6.3; // excluding range
    if (eq(name, "<"))return 6.5;
    if (eq(name, "<="))return 6.5;
    if (eq(name, ">="))return 6.5;
    if (eq(name, ">"))return 6.5;
    if (eq(name, "≥"))return 6.5;
    if (eq(name, "≤"))return 6.5;
    if (eq(name, "≈"))return 6.5;
    if (eq(name, "=="))return 6.6;
    if (eq(name, "is"))return 6.6; // careful, use 'be' for := assignment
    if (eq(name, "eq"))return 6.6;
    if (eq(name, "equals"))return 6.6;
    if (eq(name, "is not"))return 6.6; // ambiguity: a == !b vs a != b
    if (eq(name, "isnt"))return 6.6;
    if (eq(name, "isn't"))return 6.6;
    if (eq(name, "equal"))return 10;
    if (eq(name, "≠"))return 10;
    if (eq(name, "!="))return 10;

    if (eq(name, "and"))return 7.1;
    if (eq(name, "&&"))return 7.1;
    if (eq(name, "&"))return 7.1;
    if (eq(name, "∧"))return 7.1; // ⚠️ todo this is POWER for non-boolean! NEVER bitwise and  1^0==0 vs 1^0==1 ⚠ WARN!
    if (eq(name, "⋀"))return 7.1;


    if (eq(name, "xor"))return 7.2;
    if (eq(name, "^|"))return 7.2;
    if (eq(name, "⊻"))return 7.2;

    if (eq(name, "or"))return 7.2;
    if (eq(name, "||"))return 7.2;
    if (eq(name, "∨"))return 7.2;
    if (eq(name, "⋁"))return 7.2;
    //	if (eq(name, "|"))return 7.2;// todo pipe special

    if (eq(name, ":"))return 7.5; // todo:
    if (eq(name, "?"))return 7.6;

    if (name.in(function_list)) // f 1 > f 2
        return 8; // 1000;// function calls outmost operation todo? add 3*square 4+1


    if (eq(name, "⇒"))return 11; // lambdas
    if (eq(name, "=>"))return 11;
    if (eq(name, "::"))return 11; // todo lambda symbol? square = x :: x*x

    //	if (eq(name, ":"))return 12;// construction
    if (eq(name, "="))return 12; // declaration
    if (eq(name, ":="))return 13;
    if (eq(name, "be"))return 13; // counterpart 'is' for ==
    if (eq(name, "::="))return 14; // globals setter
    //	if (eq(name, "is"))return 13;// careful, could be == (6.6)

    if (eq(name, "else"))return 13.09;
    if (eq(name, "then"))return 13.15;
    if (eq(name, "if"))return 100;
    if (eq(name, "while"))return 101;
    //	if (eq(name, "once"))return 101;
    //	if (eq(name, "go"))return 101;
    if (name.in(functor_list)) // f 1 > f 2
        return function_precedence; // if, while, ... statements calls outmost operation todo? add 3*square 4+1

    if (eq(name, "return"))return 1000;
    return 0; // no precedence
}


static Wasp wasp_parser; // todo: why can't we use instances in wasm?

Node &parse(String source, ParserOptions parserOptions) {
    if (operator_list.size() == 0)
        load_parser_initialization();
    return wasp_parser.parse(source, parserOptions);
}

extern "C" Node *Parse(chars data) {
    return &wasp_parser.parse(data, {.data_mode = true});
}

Node &parse(chars source) {
    return wasp_parser.parse(source, {});
}
