#include "CharUtils.h"
#include "List.h"
#include "Util.h"

bool isalpha0(codepoint c) {
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
}

bool is_identifier(codepoint ch) {
    if (ch == '_' or ch == '$' or ch == '@')return true;
    if (ch == '-' or ch == u'‖' or ch == L'‖' or ch == '/')return false;
    if (is_operator(ch, false))
        return false;
    if (ch < 0 or ch > 128)return true; // all UTF identifier todo ;)
    return ('a' <= ch and ch <= 'z') or ('A' <= ch and ch <= 'Z'); // ch<0: UNICODE
}

bool is_bracket(char ch) {
    return ch == '(' or ch == ')' or ch == '[' or ch == ']' or ch == '{' or ch == '}';
}

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
        case '"':
            return u'"';
        case u'\u2018':
            return u'\u2019';
        case u'«':
            return u'»';
        case u'\u201C':
            return u'\u201D';
        case u'\u201D':
            return u'\u201D';
        case u'`':
            return u'`';
        case u'\'':
            return u'\'';
        default:
            error((chars)"unknown bracket "s + bracket);
    }
    return 0;
}

// NEEDs complete codepoint, not just leading char because ☺ == e2 98 ba  √ == e2 88 9a
bool is_operator(codepoint ch, bool check_identifiers /*= true*/) {
    // todo is_KNOWN_operator todo Julia
    if (ch == '-')return true; // ⚠️ minus vs hyphen!
    if (check_identifiers && is_identifier(ch))
        return false;
    if (ch == U'∞')return false; // or can it be made as operator!?
    if (ch == U'⅓')return false; // numbers are implicit operators 3y = 3*y
    if (ch == U'∅')return false; // Explicitly because it is part of the operator range 0x2200 - 0x2319
    if (0x207C < ch and ch <= 0x208C) return true; // ⁰ … ₌
    if (0x2190 < ch and ch <= 0x21F3) return true; // ← … ⇳
    if (0x2200 < ch and ch <= 0x2319) return true; // ∀ … ⌙
    if (ch == u'¬')return true;
    if (ch == u'＝')return true;
    if (ch == u'#') return true; // todo: # is NOT an operator, but a special symbol for size/count/length
    if (operator_list.has(String(ch).data))
        return true;

    if (ch > 0x80)
        return false; // utf NOT enough: ç. can still be a reference!
    if (isalnum0(ch)) return false; // ANY UTF 8
    return ch > ' ' and ch != ';' and !is_bracket(ch) and ch != '\'' and ch != '"';
}

bool isDigit(codepoint c) {
    return (c >= '0' and c <= '9') or atoi1(c) != -1;
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



bool isWhite(codepoint c) {
    return c == ' ' or c == '\t' or c == '\n' or c == '\r' or c == u''; // shift out
}

