#pragma once

#include "Node.h"
#include "String.h"

// Forward declarations
class Function;

// Function and declaration analysis methods

String extractFunctionName(Node &node);

Node &funcDeclaration(String name, Node &node, Node &body, Node *returns, Module *mod);

Node &groupFunctionDeclaration(String &name, Node *return_type, Node modifieres, Node &arguments, Node &body,
                               Function &context);

Node &groupFunctionDeclaration(Node &expression, Function &context);

Node &groupFunctionDefinition(Node &expression, Function &context);

Node &groupDeclarations(Node &expression, Function &context);

Node &groupFunctionCalls(Node &expressiona, Function &context);

Node extractModifiers(Node &expression);

Node extractReturnTypes(Node decl, Node body);

void discard(Node &node);
