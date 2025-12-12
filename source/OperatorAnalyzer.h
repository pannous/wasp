#pragma once

#include "Node.h"
#include "String.h"
#include "List.h"

// Forward declarations
class Function;

// Operator analysis methods

List<String> collectOperators(Node &expression);

void checkRequiredCasts(String &op, const Node &lhs, Node &rhs, Function &context);

Node &groupOperators(Node &expression, Function &context);

Node &groupKebabMinus(Node &node, Function &context);

Node &groupOperatorCall(Node &node, Function &function);
