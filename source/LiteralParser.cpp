#include "LiteralParser.h"
#include "CharUtils.h"
#include "Util.h"

namespace LiteralUtils {

char escapee(char c) {
    switch (c) {
        case '"':
            return '"'; // this is needed as we allow single quote
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
}

String renderChar(char chr) {
    return chr == '\n' ? String("\\n") : String('\'') + chr + '\'';
}

String fromCharCode(int64 uffff) {
    // todo UTF
    return String((char) (uffff));
}

int parseInt(char next_digit, int base) {
    return next_digit - '0';
}

bool isValidUrlChar(char ch) {
    return is_identifier(ch) || isDigit(ch) || ch == '-' || ch == '_' || ch == '~' ||
           ch == '/' || ch == ':' || ch == '?' || ch == '&' || ch == '=' || ch == '%' ||
           ch == '#' || ch == '.';
}

} // namespace LiteralUtils
