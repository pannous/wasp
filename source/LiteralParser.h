#pragma once
#include "Node.h"
#include "String.h"

// Literal parsing utilities - helper functions for parsing numbers, strings, templates, URLs
// Extracted from Wasp.cpp for better code organization
// The actual parsing methods remain in Wasp class as they need parser context

namespace LiteralUtils {
    // Character escape handling
    char escapee(char c);
    String renderChar(char chr);

    // Number utilities
    String fromCharCode(int64 uffff);
    int parseInt(char next_digit, int base);

    // URL validation
    bool isValidUrlChar(char ch);
}
