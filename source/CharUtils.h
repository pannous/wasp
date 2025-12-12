#pragma once
#include "String.h"
#include "List.h"

// Character classification and bracket utilities
// Extracted from Wasp.cpp for better code organization

bool isalpha0(codepoint c);
bool is_identifier(codepoint ch);
bool is_bracket(char ch);
bool is_operator(codepoint ch, bool check_identifiers = true);
codepoint closingBracket(codepoint bracket);
bool isDigit(codepoint c);

// List utilities for character arrays (0-terminated)
template<class S>
bool contains(S list[], S match) {
    S *elem = list;
    do {
        if (match == *elem)
            return true;
    } while (*elem++);
    return false;
}

bool contains(chars list[], chars match);

// External character/operator lists referenced by these functions
extern List<chars> operator_list;
extern codepoint opening_special_brackets[];
extern codepoint separator_list[];
extern codepoint grouper_list[];
