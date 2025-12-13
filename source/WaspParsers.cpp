// Wasp parser helper methods - extracted logic from valueNode() switch cases
// These methods are part of the Wasp class defined in Wasp.cpp
// They contain the parsing logic while control flow (continue/break/return) stays in valueNode()

#include "Wasp.h"
#include "CharUtils.h"
#include "LiteralParser.h"

// @, $ - dollar names and meta attributes
bool Wasp::parseDollarAt(Node &actual) {
    if ((ch == '$' and parserOptions.dollar_names) or (ch == '@' and parserOptions.at_names)) {
        actual.add(Node(parseIdentifier()).setKind(referencex));
        return true;
    }

    if (ch == '@' and parserOptions.meta_attributes) {
        proceed(); // skip @
        auto meta = parseIdentifier();
        Node* value = 0;
        if(ch=='(') {
            proceed();
            value = &valueNode(')');
        }
        auto node = valueNode();
        auto key = new Node(meta);
        if(value) {
            key->setKind(::key);
            key->value.node = value;
        }
        node.addMeta(key);
        actual.add(node);
        return true;
    }

    actual.add(parseOperator());
    return true;
}

// <html> and <script> tags
bool Wasp::parseHtmlTag(Node &actual) {
    if (text.substring(at, at + 5) == "<html") {
        int to = text.find("</html>", at);
        if (to < 0) to = text.length;
        auto html = Node("html", strings);
        html.value.string = &text.substring(text.find('>', at + 5) + 1, to).clone();
        actual.add(html);
        at = to + 7;
        previous = '>';
        proceed();
        return true;
    }

    if (text.startsWith("<script", at)) {
        int to = text.find("</script>", at);
        if (to < 0) to = text.length;
        auto html = Node("script", strings);
        html.value.string = &text.substring(text.find('>', at + 5) + 1, to).clone();
        actual.add(html);
        at = to + 9;
        previous = '>';
        proceed();
        return true;
    }

    // Not an HTML tag, continue with angle bracket handling
    return false;
}

// < > angle brackets for tags/generics or comparison operators
bool Wasp::parseAngleBracket(Node &actual) {
    if (not(parserOptions.use_tags or parserOptions.use_generics) or (previous == ' ' and next == ' ')) {
        Node &op = parseOperator();
        actual.add(op);
        actual.kind = expression;
        return true;
    }

    if (next == '/') todo("closing </tags>");
    return false; // Fall through to bracket group
}

// {, [, ( - brackets for objects, patterns, groups
bool Wasp::parseBracketGroup(Node &actual, codepoint close, Node *parent) {
    codepoint bracket = ch;
    auto type = getType(bracket);
#if not RUNTIME_ONLY
    bool flatten = not isFunction(actual.last());
#else
    bool flatten = true;
#endif
    bool addToLast = false;
    bool asListItem = lastNonWhite == ',' or lastNonWhite == ';' or (previous == ' ' and lastNonWhite != ':');

    if (checkAmbiguousBlock(actual, parent)) {
        if (parserOptions.space_brace) {
            addToLast = true;
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
        asListItem = false;
        flatten = false;
        specialDeclaration = true;
        auto name = actual.first().name;
        if (name == "while" or name == "if")
            specialDeclaration = false;
    }

    proceed();
    Node &object = *new Node();
    Node &objectValue = valueNode(closingBracket(bracket), parent ? parent : &actual.last());
    object.addSmart(objectValue);
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
    else
        actual.addSmart(object);

    if(bracket == '{')
        actual.last().setKind(objects, false);
    if (specialDeclaration)
        actual.kind = declaration;

    return true;
}

// ", ', `, « - string literals
bool Wasp::parseString(Node &actual, int start, codepoint &close) {
    if (previous == '\\') return false; // escape

    bool matches = close == ch;
    codepoint closer = closingBracket(ch);
    matches = matches or (close == u'\u2018' and ch == u'\u2019');
    matches = matches or (close == u'\u2019' and ch == u'\u2018');
    matches = matches or (close == u'\u201C' and ch == u'\u201D');
    matches = matches or (close == u'\u201D' and ch == u'\u201C');

    if (!matches) {
        // open string
        if (actual.last().kind == expression)
            actual.last().addSmart(quote(closer));
        else
            actual.add(quote(closer).clone());
        return true;
    }

    // Closing string delimiter
    close = 0; // ok, we are done
    Node id = Node(text.substring(start, at));
    id.setKind(Kind::strings);
    id.setType(&TemplateType);
    actual.add(id);
    return true;
}

// :, =, ←, ⇨ - assignments and key:value pairs
bool Wasp::parseAssignment(Node &actual) {
    Node &key = actual.last();

    // URL detection
    if (ch == ':' and next == '/' and key.name.in(validUrlSchemes)) {
        key.name = key.name + url();
        key.setKind(Kind::urls, false);
        return false;
    }

    bool add_raw = actual.kind == expression or key.kind == expression or
                   (actual.last().kind == groups and actual.length > 1);
    bool add_to_whole_expression = false;

    if (previous == ' ' and (next == ' ' or next == '\n') and not parserOptions.colon_immediate)
        add_to_whole_expression = true;
    if (is_operator(previous))
        add_raw = true;
    if(previous==')' and function_keywords.has(actual.first().name)) {
        actual.setKind(declaration, false);
        add_raw= true;
    }

    Node op = parseOperator();
    if (next == '>' and parserOptions.arrow)
        proceed();
    if (not(op.name == ":" or (parserOptions.data_mode and op.name == "=")))
        add_raw = true;
    if (op.name.length > 1)
        add_raw = true;
    if (actual.kind == expression)
        add_raw = true;

    if (add_raw) {
        actual.add(op.setKind(operators)).setKind(expression);
        return true;
    }

    char closer;
    if (ch == '\n') closer = ';';
    else if (op == ":" and parserOptions.colon_immediate)
        closer = ' ';
    else if (ch == INDENT) {
        closer = DEDENT;
        if (not actual.separator)
            actual.separator = '\n';
        proceed();
        white();
    } else if (ch == ' ') closer = ';';
    else closer = ' ';

    Node &val = valueNode(closer, &key);

    if (add_to_whole_expression and actual.length > 1 and not add_raw) {
        if (actual.value.node) todo("multi-body a:{b}{c}");
        actual.setKind(Kind::key, false);
        actual.value.node = &val;
    } else if (add_raw) {
        actual.add(val);
    } else {
        setField(key, val);
    }

    return true;
}

// INDENT token
bool Wasp::parseIndent(Node &actual) {
    proceed();
    if (actual.separator == ',') {
        warn("indent block within list");
        ch = '\n';
        return true;
    }

    Node element = valueNode(DEDENT);
    actual.addSmart(element.flat());
    if (not actual.separator)
        actual.separator = '\n';
    return false;
}

// \n, \t, ;, , - list separators (and space)
bool Wasp::parseListSeparator(Node &actual, codepoint close, Node *parent) {
    // Handle separators first
    if (ch != ' ') {
        if (skipBorders(ch)) {
            proceed();
            return false;
        }

        if (actual.separator != ch) {
            if (actual.length > 1 or actual.kind == expression) {
                Node neu;
                neu.kind = groups;
                neu.parent = parent;
                neu.separator = ch;
                neu.add(actual);
                actual = neu;
            } else
                actual.separator = ch;

            char sep = ch;
            while (ch == sep and not closing(ch, close)) {
                proceed();
                Node &element = valueNode(sep);
                actual.add(element.flat());
            }
            return true;
        }

        // Same separator continues - fall through to space handling below
        actual.separator = ch;
    }

    // Space handling (or fall-through from separator cases)
    if (not actual.separator)
        actual.separator = ch;
    proceed();
    white();
    return true;
}

// - and . for negative numbers, arrows, or operators
bool Wasp::parseMinusDot(Node &actual) {
    // Handle -> arrow operator
    if (ch == '-' and parserOptions.arrow and next == '>') {
        proceed();
        proceed();
        white();
        Node &node = valueNode(' ');
        Node *last = &actual.last();
        while (last->value.node and last->kind == key)
            last = last->value.node;
        last->setValue({.node = &node}).setKind(key, false);
        return false; // Continue
    }

    if (ch == '-' and isKebabBridge())
        parserError("kebab case should be handled in identifier");

    if (ch == '-' and next == '>') {
        next = u'⇨';
        proceed();
    }

    // Check for number
    if (isDigit(next) and (previous == 0 or contains(separator_list, previous) or is_operator(previous)))
        actual.addSmart(numbero());
    else if (ch == '-' and next == '.')
        actual.addSmart(numbero());
    else {
        Node *op = parseOperator().clone();
        actual.add(op);
        actual.kind = expression;
    }

    return true;
}

// Default: expressions, identifiers, imports
void Wasp::parseExpression(Node &actual, codepoint close) {
    bool addFlat = lastNonWhite != ';' and previous != '\n';
    Node &node = expressione(close);

#if DEBUG
    node.line = &line;
#endif

    // Handle imports
    if (contains(import_keywords, (chars) node.first().name.data)) {
        node = direct_include(actual, node);
    }

#ifndef RUNTIME_ONLY
    // Don't convert FFI imports to operators - they need to stay as functors
    bool is_ffi_import = (node.name == "import" && node.kind == functor);

    if (!is_ffi_import && (precedence(node) or operator_list.has(node.name))) {
        node.kind = operators;
    }
#endif

    if (node.kind == operators and ch != ':') {
        if (isFunctor(node))
            node.kind = functor;
        else actual.kind = expression;
    }

    // FFI imports need to be added directly without flattening
    if (is_ffi_import) {
        // Add import node directly without flattening
        actual.add(&node);
    } else if (node.length > 1 and addFlat) {
        for (Node arg: node)actual.add(arg);
        actual.kind = node.kind;
    } else {
        if (actual.last().kind == operators)
            actual.addSmart(&node.flat());
        else
            actual.add(&node.flat());
    }
}
