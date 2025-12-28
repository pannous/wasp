// Operator analysis extracted from Angle.cpp

#include "OperatorAnalyzer.h"
#include "Angle.h"
#include "Context.h"
#include "Keywords.h"
#include "CharUtils.h"

List<String> collectOperators(Node &expression) {
    List<String> operators;
    String previous;
    for (Node &op: expression) {
        String &name = op.name;
        if (not name)continue;
        //		name = normOperator(name);// times => * aliases
        if (name.endsWith("=")) // += etc
            operators.add(name);
        else if (prefixOperators.has(name.data)) {
            if (name == "-" and is_operator(previous.codepointAt(0)))
                operators.add(name + "…"); //  -2*7 ≠ 1-(2*7)!
            else
                operators.add(name);
        }
        //		WE NEED THE RIGHT PRECEDENCE NOW! -2*7 ≠ 1-(2*7)! or is it? √-i (i FIRST)  -√i √( first)
        //		else if (prefixOperators.has(op.name+"…"))// and IS_PREFIX
        //			operators.add(op.name+"…");
        else if (suffixOperators.has(name.data))
            operators.add(name);
        else if (operator_list.has(name.data))
            //			if (op.name.in(operator_list))
            operators.add(name);
        else if (op.kind == Kind::operators)
            operators.add(op.name);
        previous = name;
    }
    auto by_precedence = [](String &a, String &b) { return precedence(a) > precedence(b); };
    operators.sort(by_precedence);
    return operators;
}

void checkRequiredCasts(String &op, const Node &lhs, Node &rhs, Function &context) {
    // todo maybe add cast node here instead of in emit?
    Type left_kind = lhs.kind;
    Type right_kind = rhs.kind;
    if (right_kind == operators or right_kind == expression or right_kind == reference or right_kind == call)
        right_kind = preEvaluateType(rhs, context); // todo: use preEvaluateType for all rhs?

    if (op == "+" and left_kind == strings)
        useFunction("toString"); // todo ALL polymorphic variants! => get rid of these:
    if (op == "+" and left_kind == strings and right_kind == longs)
        useFunction("formatLong");
    if (op == "+" and left_kind == strings and (right_kind == reals or right_kind == realsF))
        useFunction("formatReal");
}

// outer analysis 3 + 3  ≠ inner analysis +(3,3)
// maybe todo: normOperators step (in angle, not wasp!)  3**2 => 3^2
Node &groupOperators(Node &expression, Function &context) {
    if (analyzed.has(expression.hash()))
        return expression;

    //    if ("a-b" and (expression.kind == reference or expression.kind == Kind::expression) and expression.name.contains('-'))
    //        return groupKebabMinus(expression, context); // extra logic for a-b kebab-case vs minus

    Function &function = context;
    List<String> operators = collectOperators(expression);
    String last = "";
    int last_position = 0;

    if (expression.kind == Kind::operators) {
        if (expression.name == "include") {
            warn(expression.serialize());
            Node &file = expression.values();
            addLibrary(&loadModule(file.name));
            return NUL; // no code, just meta
        }
        //		else todo("ungrouped dangling operator");
    }
    // inner ops get grouped first: 3*42≥2*3 => 1. group '*' : (3*42)≥(2*3)
    for (String &op: operators) {
        //        trace("operator");
        //        trace(op);
        if (op.empty())
            error("empty operator BUG");
        if (op == "else")continue; // handled in groupIf
        if (op == "module") {
            warn("todo modules");
            if (module)
                module->name = expression.last().name;
            return NUL;
        }
        if (op == "-")
            debug = true;
        if (op == "-…") op = "-"; // precedence hack

        //        todo op=use_import();continue ?
        if (op == "%")
            useFunction("modulo_float"); // no wasm i32_rem_s i64_rem_s for float/double
        if (op == "%")
            useFunction("modulo_double");
        if (op == "==" or op == "is" or op == "equals")use_runtime("eq");
        if (op == "include")
            return NUL; // todo("include again!?");
        if (op != last) last_position = 0;
        bool fromRight = rightAssociatives.has(op);
        fromRight = fromRight || isFunction(op, true);
        fromRight = fromRight || (prefixOperators.has(op.data) and op != "-"); // !√!-1 == !(√(!(-1)))
        int i = expression.index(op, last_position, fromRight);
        if (i < 0) {
            if (op == "-" or op == "‖") //  or op=="?"
                continue; // ok -1 part of number, ‖3‖ closing ?=>if
            i = expression.index(op, last_position, fromRight); // try again for debug
            expression.print();
            error("operator missing: "s + op + " in " + expression.serialize());
        }
        op = checkCanonicalName(op);

        // we can't keep references here because expression.children will get mutated later via replace(…)
        Node &node = expression.children[i];
        if (node.length)continue; // already processed
        Node &next = expression.children[i + 1];
        next = analyze(next, context);
        Node prev;
        if (i > 0) {
            prev = expression.children[i - 1];
            // if(prev.kind == Kind::groups) prev.setType(Kind::expression);
            prev = analyze(prev, context);
        }
        checkRequiredCasts(op, prev, next, context); // todo: move to analyze?

        //            prev = expression.to(op);
        //        else error("binop?");
        if (op == ".") {
            if (prev.kind == referencex) {
                useFunction("invokeExternRef"); // a.b() => invokeExternRef(a, "b")
                useFunction("getExternRefPropertyValue");
                useFunction("setExternRefPropertyValue");
            }
            if (prev.kind == (Kind) Primitive::node)
                useFunction("get");
        }


        // todo move "#" case here:
        if (isPrefixOperation(node, prev, next)) {
            // ++x -i
            // PREFIX Operators
            isPrefixOperation(node, prev, next);
            node.kind = Kind::operators; // todo should have been parsed as such!
            if (op == "-") //  -i => -(0 i)
                node.add(new Node(0)); // todo: use f64.neg
            node.add(next);
            expression.replace(i, i + 1, node);
        } else {
            if (op == "#" and not use_wasm_strings)
                findLibraryFunction("getChar", false);

            prev = analyze(prev, context);

            // Warn when comparison operators are applied to 'not' without parentheses
            if ((op == "==" or op == "!=" or op == "<" or op == ">" or op == "<=" or op == ">=" or
                 op == "is" or op == "equals") and prev.kind == Kind::operators) {
                String prev_op = prev.name;
                if (prev_op == "not" or prev_op == "!" or prev_op == "¬") {
                    warn("Ambiguous: 'not "s + prev.first().serialize() + " " + op + " " + next.serialize() +
                         "' parsed as '(not " + prev.first().serialize() + ") " + op + " " + next.serialize() +
                         "'. Use parentheses for clarity: 'not (" + prev.first().serialize() + " " + op + " " +
                         next.serialize() + ")'");
                }
            }

            auto lhs_type = preEvaluateType(prev, context);
            if (op == "+" and (lhs_type == Primitive::charp or lhs_type == Primitive::stringp or lhs_type == strings)) {
                findLibraryFunction("concat", true);
                // findLibraryFunction("_Z6concatPKcS0_", true);
            }
            if (op == "^" or op == "^^" or op == "**" or op == "exp" or op == "ℇ") {
                // todo NORM operators earlier
                findLibraryFunction("pow", false); // old rough!
                // findLibraryFunction("powd", false); // via pow
                findLibraryFunction("powi", false);
            }
            if (suffixOperators.has(op.data)) {
                // x²
                // SUFFIX Operators
                if (op == "ⁿ") {
                    findLibraryFunction("pow", false);
                    useFunction("pow"); // redirects to
                    // useFunction("powd");
                }
                if (i < 1)
                    error("suffix operator misses left side");
                node.add(prev);
                if (op == "²") {
                    node.add(prev);
                    node.name = "*"; // x² => x*x
                }
                //				analyzed.insert_or_assign(node.hash(), true);
                expression.replace(i - 1, i, node);
                i--;
                //                continue;
            } else if (op.in(function_list)) {
                // handled above!
                while (i++ < node.length)
                    node.add(expression.children[i]);
                expression.replace(i, node.length, node);
            } else if (isFunction(next)) {
                // 3 + double 8
                Node &rest = expression.from(i + 1);
                Node &args = analyze(rest, context);
                node.add(prev);
                node.add(args);
                expression.replace(i - 1, i + 1, node); // replace ALL REST
                expression.remove(i, -1);
            } else {
                auto var = prev.name;
                if ((op == "=" or op == ":=") and (prev.kind == reference or prev.kind == global)) {
                    // ONLY ASSIGNMENT! self modification handled later
                    Type inferred_type = preEvaluateType(next, context);
                    if (prev.kind == global) {
                        if (globals.has(var)) {
                            Global &global = globals[var];
                            global.type = inferred_type; // todo upgrade?
                            if (not global.value)
                                global.value = next.clone();
                        } else {
                            // later
                            //                            addGlobal(context, var, inferred_type, false, *next.clone());
                        }
                    } else if (addLocal(context, var, inferred_type, false)) {
                        // ok
                    } else {
                        // variable is known but not typed yet, or type again?
                        Local &local = function.locals[var];
                        if (local.type == unknown_type)
                            local.type = inferred_type; // mapType(next);
                    }
                } // end of '=' assignment logic
                if (not(op == "#" and prev.empty() and prev.kind != reference)) // ok for #(1,2,3) == len
                    node.add(prev);

                node.add(next);

                if (op.length > 1 and op.endsWith("=") and op[0] != ':')
                    // express *= += -= … self assignments
                    if (op[0] != '=' and op[0] != '!' and op[0] != '?' and op[0] != '<' and op[0] != '>') {
                        // *= += etc
                        node.name = String(op.data[0]);
                        Node *setter = prev.clone();
                        //					setter->setType(assignment);
                        setter->value.node = node.clone();
                        node = *setter;
                    }
                if (node.name == "?")
                    node = groupIf(node, context); // consumes prev and next
                if (i > 0)
                    expression.replace(i - 1, i + 1, node);
                else {
                    expression.replace(i, i + 1, node);
                    warn("operator missing argument");
                }
                if (expression.length == 1 and expression.kind == Kind::groups) {
                    return expression.first(); //.setType(operators);
                }
            }
        }
        last_position = i;
        last = op;
    }
    return expression;
}

// extra logic for a-b kebab-case vs minus operator
Node &groupKebabMinus(Node &node, Function &context) {
    auto name = node.name;
    auto re = name.substring(0, name.indexOf('-'));
    auto lhs = Node(re, true);
    if (context.locals.has(re) or globals.has(re)) {
        Node op = Node("-").setKind(operators, true);
        auto right = name.substring(name.indexOf('-') + 1);
        auto &rhs = analyze(*new Node(right, true), context); // todo expression!
        op.add(lhs);
        op.add(rhs);
        return *op.clone();
    }
    return node;
}

// todo merge with groupFunctionCalls
Node &groupOperatorCall(Node &node, Function &function) {
    {
        bool is_function = isFunction(node);
        findLibraryFunction(node.name, true); // sets functions[name].is_import = true;
        if (is_function and node.kind != operators) {
            node.kind = call;
        }
        Node &grouped = groupOperators(node, function); // outer analysis id(3+3) => id(+(3,3))
        if (grouped.length > 0)
            for (Node &child: grouped) {
                // inner analysis while(i<3){i++}
                child = analyze(child, function); // REPLACE with their ast
            }
        if (is_function)
            functions[node.name].is_used = true;
        return grouped;
    }
}
