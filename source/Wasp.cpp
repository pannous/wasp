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
#include "CharUtils.h"
#include "Keywords.h"
#include "LiteralParser.h"
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
#include <unistd.h> // isatty

#endif

#include <math.h>


#ifndef RUNTIME_ONLY
#include "WebServer.hpp"
//#include "wasm_merger_wabt.h"
#include "WebApp.h"
#endif

#include "WitReader.h"
#include "lsp_server.h"

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

// isalpha0 moved to CharUtils.cpp


bool data_mode = true;
// todo ! // tread '=' as ':' instead of keeping as expression operator  WHY would we keep it again??

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
    for (auto &key: list) {
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

// is_identifier() moved to CharUtils.cpp


// is_bracket() moved to CharUtils.cpp


// closingBracket() moved to CharUtils.cpp


// is_operator() moved to CharUtils.cpp


// contains() template moved to CharUtils.h

// contains() moved to CharUtils.cpp


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

    //    void error(String message) {
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
                    error("mixing tabs and spaces for indentation");
            }
            indentation_by_tabs = true;
            tabs++;
            offset++;
        }
        if (tabs > 0 and text[offset] == ' ')
            error("ambiguous indentation, mixing tabs and spaces");
        while (text[offset] == ' ') {
            if (indentation_level > 0 and indentation_by_tabs)
                if (proceed())
                    error("mixing tabs and spaces for indentation");
            indentation_by_tabs = false;
            tabs = tabs + 1. / spaces_per_tab;
            offset++;
        }
        if (tabs > 0 and text[offset] == '\t')
            error("ambiguous indentation, mixing tabs and spaces");
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
    bool closing(char c, char closer) {
        if (closer == '>')
            return c == '>' or c == '\n'; // nothing else closes!
        if (closer == ' ' and c == '>' and parserOptions.use_generics) // todo better
            return true;
        if (c == closer)
            return true;
        if (group_precedence(c) <= group_precedence(closer))
            return true;
        if (c == INDENT)
            return false; // quite the opposite
        if (c == DEDENT and not(closer == '}' or closer == ']' or closer == ')'))
            return true;
        if (c == '}' or c == ']' or c == ')') {
            // todo: ERROR if not opened before!
            //				if (ch != close and close != ' ' and close != '\t' /*???*/) // cant debug wth?
            return true;
        } // outer match unresolved so far

        if (group_precedence(c) <= group_precedence(closer))
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
            source = readFile(findFile(source, parserOptions.current_dir).data);
        }
#ifndef RELEASE
        if (not options.data_mode and options.debug) {
            // todo: e.g. c-header parser
            put_chars("Parsing: ");
            println(source.data);
        }
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
        Node &result = parseNode(); // <<
        white();
        if (ch and ch != -1 and ch != DEDENT) {
            printf("UNEXPECTED CHAR %c", ch);
            print(position());
            error("Expect end of input");
            result = ERROR;
        }
        // Mark does not support the legacy JSON reviver function todo ??
        return result;
        //		return *result.clone();
    }


    Wasp &setFile(String file0) {
        this->file = file0;
        parserOptions.current_dir = extractPath(file);
        return *this;
    }

private:
    // Switch case handlers for valueNode() - extracted for better readability
    // Return true if handled (break from switch), false if not handled (continue loop)
    bool parseDollarAt(Node &actual);

    bool parseHtmlTag(Node &actual);

    bool parseAngleBracket(Node &actual);

    bool parseBracketGroup(Node &actual, codepoint close, Node *parent);

    bool parseString(Node &actual, int start, codepoint &close);

    bool parseAssignment(Node &actual);

    bool parseIndent(Node &actual);

    bool parseListSeparator(Node &actual, codepoint close, Node *parent);

    bool parseMinusDot(Node &actual);

    Node& directInclude(Node & node);

    void parseExpression(Node &actual, codepoint close);

    // escapee() and renderChar() moved to LiteralParser.cpp (LiteralUtils namespace)

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
        String msg = m;
        msg += String(position()).data;
        auto error = new SyntaxError(msg);
        error->at = at;
        error->lineNumber = lineNumber;
        error->columnNumber = at - columnStart;
        error->file = file.data;
        if (throwing)
            raise(msg.data);
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
            error("end of code");
            return ch;
        }
        // If a c parameter is provided, verify that it matches the current character.
        if (c and c != ch) {
            // todo: debug only / who cares?
            // err(s("Expected '") + c + "' instead of " + LiteralUtils::renderChar(ch));
        }
        // Get the next character. When there are no more characters, return the empty string.
        previous = ch;
        if (ch != ' ' and ch != '\n' and ch != '\r' and ch != '\t')
            lastNonWhite = ch;
        short step = utf8_byte_count(previous);
        at = at + step;
        if (at >= text.length) {
            ch = 0;
            return -1;
        }
        ch = decode_unicode_character(text.data + at);
        short width = utf8_byte_count(ch);
        if (at + width >= text.length)next = 0;
        else next = decode_unicode_character(text.data + at + width);
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
    String parseIdentifier() {
        // identifiers must start with a letter, _ or $.
        if (!is_identifier(ch))
            error("Unexpected identifier character "s + LiteralUtils::renderChar(ch));
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

    // Parse a URL.
    String url() {
        int start = at;
        // Process characters valid in a URL (letters, digits, -, _, ~, /, :, ?, &, =, %, #).
        while (proceed() && (LiteralUtils::isValidUrlChar(ch)))
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
            mult.add(Node(parseIdentifier()).setKind(reference).clone());
            return mult;
        }
    };

    // parseInt() and fromCharCode() moved to LiteralParser.cpp

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
                    auto nodes = parseNode('}');
                    chunks.add(nodes); //.setKind(expression)); // ${1+2}
                } else {
                    chunks.add(Node(parseIdentifier().clone()).setKind(referencex)); // $test
                }
            } else {
                int start = at;
                while (ch and ch != '`' and previous != '\\' and ch != '$')
                    proceed();
                if (start == at)break;
                String tee = text.substring(start, at);
                // chunks.add(Node(tee).setKind(strings));
                chunks.add(Node(tee.clone()).setKind(strings));
            }
        }
        if (end_of_text())
            error("Unterminated string");
        proceed(); // skip ` done
        return chunks;
    }

    Node quote(codepoint delim = '"') {
        if (ch == '`') return parseTemplate();
        proceed();
        int start = at;
        while (ch and ch != delim and previous != '\\')
            proceed();
        if (end_of_text())
            error("Unterminated string");
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
            error("Not an inline comment");
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
        String start = line;
        int startLine = lineNumber;
        do {
            proceed();
            while (ch == '*' or ch == '#') {
                proceed(); // closing */ or ##
                if (ch == '/' or ch == '#') {
                    proceed();
                    return;
                }
            }
        } while (ch);
        error("Unterminated block comment starting at line "s + startLine + ": " + start);
    };

    // Parse a comment
    bool parseComment() {
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
            error("Not a comment");
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
                if (not parseComment()) return; // else loop on
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


    Node &parseOperator() {
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
        if (operator_list.has(symbol.data))
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
            return (*new Node("‖")).add(parseNode(u'‖').clone()).setKind(operators, false);
            //			return (*new Node("abs")).setType(Kind::call, false);
        }
        if (ch == '$' and parserOptions.dollar_names and is_identifier(next)) {
            proceed(); // $name
            return *resolve(Node(parseIdentifier()).setKind(referencex)).clone(); // or op
        }
        if (is_operator(ch))
            return parseOperator();
        if (is_identifier(ch))
            return *resolve(Node(parseIdentifier(), true)).clone(); // or op
        error("Unexpected symbol character ‘"s + String((char) text[at]) + "’" + String((char) text[at + 1]) +
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
        if(function_keywords.has(node.name.data))
            return parseDeclaration(closer);
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

    // isDigit() moved to CharUtils.cpp

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

    bool skipBorders(char c) {
        // {\n} == {}
        if (next == 0)return true;
        if (lastNonWhite == ':')return true;
        if (lastNonWhite == '{' or next == '}')
            return true; // todo: nextNonWhite
        if (lastNonWhite == '(' or next == ')')return true;
        if (lastNonWhite == '[' or next == ']')return true;
        if (c == ',' and next == ';')return true; // 1,2,3,; => 1,2,3;
        if (c == ',' and next == '\n')return true; // 1,2,3,\n => 1,2,3;
        if (c == ';' and next == '\n')return true; // 1,2,3,\n => 1,2,3;
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
        error("unknown bracket type "s + bracket);
        return errors;
    }


    void parseGrouping(Node &actual) {
        // overloadable grouping operators, but not builtin (){}[]
        let grouper = ch;
        proceed();
        auto &body = parseNode(closingBracket(grouper));
        Node group(grouper);
        group.setKind(operators, false); // name==« (without »)
        group.add(body);
        //				group.type = type("group")["field"]=grouper;
        actual.add(group);
        return;
    }

    // ":" is short binding a b:c d == a (b:c) d
    // "=" is long binding a b=c d == (a b)=(c d)   todo a=b c=d
    // "-" is post binding operator (analyzed in angle) OR short-binding in kebab-case
    // special : close=' ' : single value in a list {a:1 b:2} ≠ {a:(1 b:2)} BUT a=1,2,3 == a=(1 2 3)
    // special : close=';' : single expression a = 1 + 2
    // significant whitespace a {} == a,{}{}
    // todo a:[1,2] ≠ a[1,2] but a{x}=a:{x}? OR better a{x}=a({x}) !? but html{...}
    // reason for strange name is better IDE findability, todo rename to readNode() or parseNode()?
    Node &parseNode(codepoint close = 0, Node *parent = 0) {
        Node &actual = *new Node(); // current already used in super context
        actual.parent = parent;
        actual.setKind(groups);
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
                // (…) {…} «…» ... “‘ part of string keep ';' ',' ' ' for further analysis (?)
                if (ch == 0 or /*ch == 0x0E or*/ ch == ' ' or ch == '\n' or ch == '\t' or ch == ';' or ch == ',');
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
                parseGrouping(actual);
                continue;
            }
            switch (ch) {
                case '@':
                case '$':
                    if (parseDollarAt(actual)) break;
                    continue;
                case '<':
                    if (parseHtmlTag(actual)) break;
                // else Fall through to angle bracket handling
                case '>':
                    if (ch == '>' and (parserOptions.use_tags or parserOptions.use_generics)) return actual;
                    if (parseAngleBracket(actual)) break;
                    continue;
                case u'﹝': // ﹞
                case u'〔': // 〕
                case U'［': // ］ FULLWIDTH
                case '[':
                case u'⸨': // '⸩'
                case '(':
                case '\x0E': // Shift Out close='\x0F' Shift In
                case u'﹛': // ﹜
                case u'｛': // ｝
                case '{':
                    if (parseBracketGroup(actual, close, parent)) break;
                    continue;
                case '}':
                case ')':
                case ']':
                    error("wrong closing bracket");
                // case '+': // todo WHO writes +1 ? ;)
                case '-':
                case '.':
                    if (parseMinusDot(actual)) break;
                    continue;
                case '"':
                case '\'': /* don't use modifiers ` ˋ ˎ */
                case u'«': // «…»
                case u'\u2018': // '𝚗𝚊𝚖𝚎'
                case u'\u201C': // "…" Character too large for enclosing character literal type
                case '`':
                    if (parseString(actual, start, close)) break;
                    continue;
                case ':':
                case U'：':
                case U'≝':
                case U'≔': // vs ≕ =:
                case U'＝':
                case U'﹦':
                case u'←': // in apl assignment is a left arrow
                case u'⇨': // ??
                case '=':
                    if (parseAssignment(actual)) break;
                    continue;
                case INDENT:
                    if (parseIndent(actual)) break;
                    continue;
                case '\n': // groupCascade
                case '\t': // only in tables
                case ';': //
                case ',':
                case ' ':
                    if (parseListSeparator(actual, close, parent)) break;
                    continue;
                case '#':
                    if (next == ' ') {
                        parseComment();
                        continue;
                    } else {
                        actual.add(parseOperator());
                        continue;
                    }
                case '/':
                    if (ch == '/' and (next == '/' or next == '*')) {
                        parseComment();
                        warn("comment should have been handled before!?");
                        continue;
                    } // else fall through to default … expressione
                case '%': // escape keywords for names in wit
                    if (parserOptions.percent_names) {
                        // and…
                        proceed();
                        actual.add(Node(parseIdentifier())); // todo make sure not to mark as operator …
                        continue;
                    }
                default:
                    parseExpression(actual, close);
                    break;
            }
        }
        if(close == ':')
            error("':' as close");
        if (close and not isWhite(close) and close != ';' and close != ',')
            error("unclosed pair "s + close); // todo remember opening pair line
        Node &result = actual.flat();
        return result;
    };


    Node& parseDeclaration(codepoint closer) {
        proceed();
        Node &fun = symbol();
        fun.setKind(declaration, false);
        Node args=NIL;
        List<chars> block_markers={":",":=","=","is","be","do","\n"};//," of "," as "," in "," is "," like "}; "then" for if
        if(ch=='(') {
            proceed('(');
            args=parseNode(')');
        }else {
            int first_pos = line.length;
            chars first_marker;
            for(chars block_marker : block_markers) {
                int pos = line.indexOf(block_marker, at);
                if(pos!=-1 and pos<first_pos) {
                    first_pos=pos;
                    first_marker = block_marker;
                }
            }
            // if(not first_marker)first_marker="\n";
            String argString = line.to(first_pos).trim();
            at += argString.length;
            if(first_marker)parseOperator();
            // proceed(); // to skip space or marker
            args = parse(argString.data,parserOptions);
            args = parseNode(first_marker[0]); // until first block marker
        }
        if(ch=='{') {
            proceed('{');
            closer = '}';
        }
        if(ch==':' or ch=='=') {
            proceed();
            closer = ';';
        }
        if(ch=='\n') {
            proceed();
            closer = DEDENT;
        }
        // Node block=parseBlock();
        Node block=parseNode(closer);
        fun["params"]=args;
        fun["body"]=block;
        return fun;
        // return *fun.clone();
    }



    bool isKebabBridge() {
        // isHyphen(Bridge) e.g. a-b in special ids like in component model
        if (not is_identifier(next))return false; // i-- i-1
        if (parserOptions.kebab_case_plus and ch == '-')return true;
        return parserOptions.kebab_case and ch == '-' and isalpha0(previous) and not isnumber(next) and next != '=';
    }
};

// Implementation of parse handler methods (extracted from valueNode switch statement)
#include "WaspParsers.cpp"


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

#include "../test/test_functions.h"

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
    *
    /
}
#endif

void load_parser_initialization() {
    // todo: remove thx to __wasm_call_ctors
    if (operator_list.size() == 0)
        error("operator_list should have been constructed in __wasm_call_ctors @ _start");
}


// Mark/wasp has clean syntax with FULLY-TYPED data model (like JSON or even better)
// Mark/wasp is generic and EXTENSIBLE (like XML or even better)
// Mark/wasp has built-in MIXED CONTENT support (like HTML5 or even better)
// Mark/wasp supports HIGH-ORDER COMPOSITION (like S-expression or even better)


//struct Exception {};
//wasm-ld: error: wasp.o: undefined symbol: vtable for __cxxabiv1::__class_type_info


Node parseFile(String filename, ParserOptions options) {
    String found = findFile(filename, options.current_dir);
    if (not found)
        error("file not found "s + filename);
    else
        info((chars)("found "s + found));
    if (found.endsWith("wast") or found.endsWith("wat"))
        found = compileWast(found.data); // and use it:
    if (found.endsWith("wasm")) {
        // handle in Angle.cpp analysis, not in valueNode
        //			read_wasm(found);
        auto import = Node("include").setKind(operators);
        import.add(new Node(found));
        return import;
    } else if (found.endsWith("wasp"))
        return Wasp().setFile(found).parse(readFile(found.data), options);
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

#define __MAIN__

#if WABT_MERGE or INCLUDE_MERGER
void merge_files(int argc, char **argv);
#endif

#if not CTESTS
int main(int argc, char **argv) {
    executable_path = argv[0];
    if (getenv("SERVER_SOFTWARE"))
        printf("Content-Type: text/plain\n\n"); // todo html
    String args;
    for (int i = 1; i < argc; ++i) args += i > 1 ? String(" ") + argv[i] : String(argv[i]);
    //   String arg=extractArg(argv,argc);
#if WASM
    initSymbols(); // todo still necessary ??
#endif
#if ErrorHandler
    register_global_signal_exception_handler();
#endif
    try {
        if (argc == 1) { // no args, just program name
#if not WASM // todo but wasi pipes?
            // Check if stdin is a pipe (not a terminal)
            if (!isatty(STDIN_FILENO)) {
                // Read from stdin and evaluate
                String input;
                char buffer[4096];
                while (fgets(buffer, sizeof(buffer), stdin)) {
                    input += buffer;
                }
                if (input.length > 0) {
                    Node result = eval(input);
                    print(result.serialize());
                    return 0;
                }
            }
#endif
            print("Wasp 🐝 "s + wasp_version);
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
#if WABT_MERGE or INCLUDE_MERGER
                merge_files(--argc, ++argv);
#else
                todo("linking files needs compilation with WABT_MERGE")
#endif
            } else {
                int64 result = run_wasm_file(args.data);
                print(result);
            }
        } else if (args == "test" or args == "tests")
#if NO_TESTS
        print("wasp release compiled without tests");
#else
            testCurrent();
#endif
        else if (args == "home" or args == "wiki" or args == "docs" or args == "documentation") {
            print("Wasp documentation can be found at https://github.com/pannous/wasp/wiki");
            system("open https://github.com/pannous/wasp/");
        } else if (args.startsWith("eval")) {
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
        } else if (args == "lsp") {
#if not WASM
            return lsp_main(); // maybe later ;)
#endif
        } else if (args.contains("help"))
            print("detailed documentation can be found at https://github.com/pannous/wasp/wiki ");
        else if (args == "version" or args == "--version" or args == "-v")
            print("Wasp 🐝 "s + wasp_version);
        else if (args.contains("compile") or args.contains("build") or args.contains("link")) {
            Node results = eval(args.from(" ")); // todo: dont run, just compile!
            // Code &binary = compile(args.from(" "), true);
            // binary.save(); // to debug
        } else {
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
#endif

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


static Wasp wasp_parser; // todo: why can't we use instances in wasm?

Node &parse(String source, ParserOptions parserOptions) {
    if (operator_list.size() == 0)
        load_parser_initialization();
    return wasp_parser.parse(source.data, parserOptions);
}

extern "C" Node *Parse(chars data) {
    return &wasp_parser.parse(data, {.data_mode = true});
}

Node &parse(chars source) {
    return wasp_parser.parse(source, {});
}
