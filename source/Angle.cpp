#define _main_

#include <functional>
#include "Wasp.h"
#include "Angle.h"
#include "Node.h"
#include "Util.h"
#include "Map.h"
#include "Interpret.h"
//#import "wasm_helpers.h" // IMPORT so that they don't get mangled!
#include "wasm_helpers.h" // IMPORT so that they don't get mangled! huh?
#include "WitReader.h"
#include "CharUtils.h"
#include "Keywords.h"
#include "Context.h" // AST Analysis Context (shared with wasm_emitter.cpp)
#include "wasm_emitter.h" // to do put all dependencies into the context
#include "WitReader.h"

#if INCLUDE_MERGER and not RUNTIME_ONLY
#include "wasm_merger.h"
#endif

extern Map<int64, bool> analyzed;
void addLibrary(Module *modul);
void useFunction(String name);


extern int __force_link_parser_globals;
int *__force_link_parser_globals_ptr = &__force_link_parser_globals;



Node interpret(String code) {
    Node parsed = parse(code);
    return parsed.interpret();
}


#ifndef RUNTIME_ONLY

Code &compile(String code, bool clean = true); // exposed to wasp.js
#endif

void debug_wasm_file() {
#if not WASM and not RELEASE
    print("validate-main.sh");
    print("+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++");
    system("./validate-main.sh");
    auto errors = readFile("main.wasm.validation");
    print(errors);
    print("+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++");
    if (strlen(errors) > 0) {
        exit(1);
    }
#endif
}

// todo: merge with emit
Node eval(String code) {
#ifdef RUNTIME_ONLY
    return parsed; // no interpret, no emit => pure data  todo: WARN
#else
#ifndef WASI
    if (use_interpreter) {
        Node parsed = parse(code);
        return parsed.interpret();
    } else
#endif
    {
#if WEBAPP
        // errors are not forwarded to js, so catch them here! todo WHY NOT?
        try {
#endif
        Code &binary = compile(code, true);
        binary.save(); // to debug
#if NO_TESTS and not RELEASE
        debug_wasm_file(); // SLOW! use only to debug single file
#endif
        smart_pointer_64 results = binary.run();
        auto _resultNode = smartNode(results);
        if (!_resultNode)return ERROR;
        Node &resultNode = *_resultNode;
#if not RELEASE and not MY_WASM
        print("» %s\n"s % resultNode.serialize().data);
#endif
        return resultNode;
#if WEBAPP
        } catch (chars err) {
            print("eval FAILED WITH internal ERROR\n");
            printf("%s\n", err);
        } catch (String & err) {
            print("eval FAILED WITH ERRORs\n");
            printf("%s\n", err.data);
        } catch (SyntaxError & err) {
            print("eval FAILED WITH SyntaxError\n");
            printf("%s\n", err.data);
        } catch (...) {
            print("eval FAILED WITH UNKNOWN ERROR\n");
        }
        return ERROR;
#endif
    }
#endif
}


String extractFunctionName(Node &node) {
    if (not node.name.empty() and node.name != ":=")
        return node.name;
    if (node.length > 1)
        return node.first().name;
    // todo: public go home to family => go_home
    return node.name;
}

// if a then b else c == a and b or c
// (a op c) => op(a c)
// further right means higher prescedence/binding, gets grouped first
// todo "=" ":" handled differently?


Node &groupIf(Node n, Function &context) {
    if (n.length == 0 and !n.value.data)
        error("no if condition given");
    if (n.length == 1 and !n.value.data)
        error("no if block given");
    Node &condition = n.first();
    Node then;
    if (n.value.data)
        then = n.values();
    else if (n.kind == key) {
        condition = n.from(1);
        then = *n.value.node;
    } else if (n.length > 1)then = n[1];
    if (then == "else")
        then = condition.values();

    if (n.has("then")) {
        condition = n.to("then");
        then = n.from("then");
    }

    if (n.has(":") /*before else: */) {
        // todo remove since ":" is always parsed immediate (right?)
        condition = n.to(":");
        if (condition.has("else"))
            condition = condition.to("else"); // shouldn't happen?
        if (then.length == 0)
            then = n.from(":");
    }
    Node otherwise;
    if (n.has("else")) {
        otherwise = n["else"].values();
        if (otherwise.empty())
            otherwise = n.last();
    }
    if (then.has("then"))
        then = n.from("then");
    if (then.has("else")) {
        otherwise = then.from("else");
        then = then.to("else");
    }
    if (then.name == ":") {
        // todo remove since ":" is always parsed immediate (right?)
        if (then.length > 1)
            otherwise = then[1];
        if (then.length > 2)
            error("too many clauses for if?");
        then = then.first();
    }
    if (n.length == 3 and otherwise.isEmpty())
        otherwise = n[2];
    if (otherwise.isEmpty() and n.next) {
        if (n.next->name == "else") {
            otherwise = *n.next->next;
            //            otherwise = n.next->values();
        }
        //        else
        //            otherwise = *n.next;
    }
    Node *eff = new Node("if");
    Node &ef = *eff;
    ef.kind = expression;
    //	ef.kind = ifStatement;
    if (condition.length > 0)condition.setKind(expression); // so far treated as group!
    if (then.length > 0)then.setKind(expression);
    if (otherwise.length > 0)otherwise.setKind(expression);

    ef["condition"] = analyze(condition, context);
    ef["then"] = analyze(then, context);
    ef["else"] = analyze(otherwise, context);
    analyzed[ef.hash()] = true;
    return ef;
}

List<String> collectOperators(Node &expression) {
    List<String> operators;
    String previous;
    for (Node &op: expression) {
        String &name = op.name;
        if (not name)continue;
        //		name = normOperator(name);// times => * aliases
        if (name.endsWith("=")) // += etc
            operators.add(name);
        else if (prefixOperators.has(name)) {
            if (name == "-" and is_operator(previous.codepointAt(0)))
                operators.add(name + "…"); //  -2*7 ≠ 1-(2*7)!
            else
                operators.add(name);
        }
        //		WE NEED THE RIGHT PRECEDENCE NOW! -2*7 ≠ 1-(2*7)! or is it? √-i (i FIRST)  -√i √( first)
        //		else if (prefixOperators.has(op.name+"…"))// and IS_PREFIX
        //			operators.add(op.name+"…");
        else if (suffixOperators.has(name))
            operators.add(name);
        else if (operator_list.has(name))
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


bool maybeVariable(Node &node) {
    if (node.kind != reference and node.kind != key and !node.isSetter())
        return false;
    if (node.kind == strings)return false;
    return /*node.parent == 0 and*/ not node.name.empty() and node.name[0] >= 'A'; // todo;
}




Node &constructInstance(Node &node, Function &function);

Node &classDeclaration(Node &node, Function &function);

Node &groupTypes(Node &expression, Function &context, bool as_param) {
    // todo delete type declarations double x, but not double x=7
    // todo if expression.length = 0 and first.length=0 and not next is operator return ø
    // ways to set type:
    /*
     * int x
     * x:int
     * x=7  via pre-evaluation of rest!!!
     * x int    unstable, discouraged!
     * x::int clear verbosive not yet implemented
     * */
    if (expression.kind == declaration)
        return expression; // later
    if (types.size() == 0)initTypes();

    //	Node typed_list;
    for (int i = 0; i < expression.length; i++) {
        Node &node = expression.children[i];
        if (node.name == "as") {
            i++; // skip type declaration
            expression[i].setKind(clazz, false);
            continue;
        }
        if (not isType(node))
            continue;
        //			if (operator_list.has(typed.name))
        //				continue; // 3.3 as int …
        if (not types.has(node.name))continue;
        auto aType = types[node.name];
        if (not aType)continue;
        expression.remove(i, i); // move type to type field of instance

        Node *typed = 0;

        if (i < expression.length and not is_operator(expression.children[i].name[0])) {
            typed = &expression.children[i];
        }
        while (typed and (isPrimitive(*typed) or
                          (typed->kind == reference and typed->length == 0))) {
            // BAD criterion for next!
            typed->type = aType; // ref ok because types can't be deleted ... rIgHt?
            if (typed->kind == reference or typed->isSetter())
                addLocal(context, typed->name, mapType(aType->name), as_param);
            // HACK for double x,y,z => z.type=DoubleType !
            if (i + 1 < expression.length)
                typed = &expression[++i];
            else if (typed->next) typed = typed->next;
                // else outer group types currently not supported ((double x) y z)
            else break;
        }
    }
    //	if(isPrimitive(expression)
    return expression.flat(); // (1) => 1
}

Node &constructInstance(Node &node, Function &function) {
    Node &type = *types[node.name];
    if (node.values().name == "func") return node;
    /* todo what is this??  test-tuple: func(other: list<u8>, test-struct: test-struct, other-enum: test-enum) -> tuple<string, s64> */

    node.type = &type; // point{1 2} => Point(1,2)
    check_eq_or(node.length, type.length, "field count mismatch");
    node.kind = constructor;

    for (int i = 0; i < node.length; ++i) {
        node[i].type = type[i].type;
        //        Valtype valtype = mapType(*node[i].type);
        //        Valtype valtype1 = mapTypeToWasm(node[i].kind);
        //        check_eq_or(valtype , valtype1,
        //                    error("incompatible value for field %s of type "s % type[i].name + type.name + " in " + node.serialize()));
    }
    return node;
}

// Recursively find and convert struct constructor calls to constructor nodes
Node &groupStructConstructors(Node &node, Function &context) {
    String &name = node.name;

    if (node.length >= 2 && node.first().name == "struct")
        return node; // Don't recurse into struct declaration itself

    // Check if this node itself is a struct constructor: a{...} where 'a' is a struct type
    if (types.has(name) && node.length > 0 && node.kind != constructor) {
        Node &type = *types[name];
        if (type.kind == clazz || type.kind == structs) {
            // This is a struct constructor call
            if (debug) printf("Converting %s{...} to constructor\n", name.data);
            Node &insta = constructInstance(node, context);
            if (debug)
                printf("After constructInstance: result.kind=%d, result.serialize()=%s\n", insta.kind,
                       insta.serialize().data);
            return insta;
        }
    }

    // Recursively process children
    for (int i = 0; i < node.length; ++i) {
        node.children[i] = groupStructConstructors(node.children[i], context);
    }

    return node;
}


Node &groupGlobal(Node &node, Function &function) {
    // todo: turn wasp_main variables into global variables
    if (node.first().name == "global")
        node = node.from(1);
    if (node.length > 1) {
        node = node.flat();
        if (node.first().kind == reference)
            node.first().setKind(global, false);
        else
            error("global declaration not a reference "s + node.first());
        Node &grouped = groupOperators(node, function).flat();
        //        Node &grouped = analyze(node, function).flat();
        //        if (grouped.kind != operators)
        //            error("global declaration not an assignment "s + grouped);
        //        Node *type = grouped[1].type;
        //        Type type = mapType(grouped[1]);
        Type type = preEvaluateType(grouped[1], function);
        addGlobal(function, grouped[0].name, type, false, grouped[1].clone());
        return grouped;
    } else {
        if (not globals.has(node.name) and not builtin_constants.contains(node.name))
            addGlobal(function, node.name, unknown_type, false, 0);
        // todo update type later
    }
    node.setKind(global, false);
    //    node.setType(reference, false);
    return node;
}


Node extractReturnTypes(Node decl, Node body);

Node &classDeclaration(Node &node, Function &function);

Node &classDeclaration(Node &node, Function &function) {
    // including class type interface struct record keywords !
    if (debug) printf("classDeclaration called\n");
    if (node.length < 2)
        error("wrong class declaration format; should be: class name{…}");
    Node &dec = node[1];
    String &kind_name = node.first().name;
    if (debug) printf("  kind_name=%s, dec.name=%s\n", kind_name.data, dec.name.data);

    String &name = dec.name;
    // If type already exists (from collectStructDeclarations), just return the existing one
    if (types.has(name)) {
        if (debug) printf("  Type '%s' already in types map (from collectStructDeclarations), returning existing\n",
                          name.data);
        return *types[name];
    }

    if (kind_name == "struct") {
        //        if (use_wasm_structs) distinguish implementation later!
        dec.kind = structs;
    } else if (kind_name == "class" or kind_name == "type")
        dec.kind = clazz;
    else todo(kind_name);
    int pos = 0;
    Node *type;
    // todo other layouts and default values and class constructors, functions, getters … … …
    for (Node &field: dec) {
        if (isType(field)) {
            type = &field;
            continue;
        } else
            type = &field.values().first();
        if (not type and not isType(*type) and not types.has(type->name))
            error("class field needs type %s %s"s % type->name % node.serialize());
        field.type = types[type->name]; // int => IntegerType
        field.kind = fields;
        field.value.longy = pos;
        // field.setMeta("position", (Node)pos++);
        // field.metas()["position"] = pos++;
        field["@position"] = pos++;
    }
    if (types.has(name)) {
        if (types[name] == dec)
            warn("compatible declaration of %s already known"s % name);
        else
            error("incompatible structure %s already declared:\n"s % name + types[name]->serialize() /*+ node.line*/);
    } else {
        if (debug) printf("  Adding type '%s' to types map\n", name.data);
        types.add(name, dec.clone());
        if (debug) printf("  After add: types.has('%s')=%p\n", name.data, (void *) types.has(name));
    }
    return dec;
}


Node &funcDeclaration(String name, Node &node, Node &body, Node *returns, Module *mod) {
    //    if(functions.has(name)) polymorph
    // is_used to fake test wit signatures
    Function function = {
        .name = name, .module = mod, .is_import = true, .is_declared = not body.empty(), .is_used = true,
    }; // todo: clone!
    function.body = &body;
    function.signature = groupFunctionArgs(function, node);
    if (returns and function.signature.return_types.size() == 0)
        function.signature.returns(mapType(returns->name));
    functions.insert_or_assign(name, function);
    return Node().setKind(functor).setValue(Value{.data = &function}); // todo: clone!  todo functor => Function* !?!?
}


void discard(Node &node) {
    // nop to explicitly ignore unused-variable (for debugging) or no-discard (e.g. in emitData of emitArray )
}

Node &
groupFunctionDeclaration(String &name, Node *return_type, Node modifieres, Node &arguments, Node &body,
                         Function &context) {
    // Type return_type;
    // if(return_type0)
    //     return_type = mapType(return_type0);
    // else
    //     return_type = preEvaluateType(body, context);
    //	String &name = fun.name;
    //	silent_assert(not is_operator(name[0]));
    //	trace_assert(not is_operator(name[0]));
    if (is_operator(name[0]) and name != ":=") // todo ^^
        todo("is_operator!"s + name); // remove if it doesn't happen

    if (name and not function_operators.has(name)) {
        if (context.name != "wasp_main") {
            print("broken context");
            print(context.name);
        }
        if (not functions.has(name)) {
            Function fun{.name = name};
            functions.add(name, fun);
        }
    }
    Function &function = functions[name]; // different from context!
    function.name = name;
    function.is_declared = true;
    function.is_import = false;


    Signature &signature = groupFunctionArgs(function, arguments);
    if (signature.size() == 0 and function.locals.size() == 0 and body.has("it", false, 100)) {
        addLocal(function, "it", float64t, true);
        signature.add(float64t, "it"); // todo: any / smarti! <<<
    }
    body = analyze(body, function); // has to come after arg analysis!
    if (!return_type)
        return_type = extractReturnTypes(arguments, body).clone();
    if (return_type)
        signature.returns(mapType(return_type)); // explicit double sin(){} // todo other syntaxes+ multi
    Node &decl = *new Node(name); //node.name+":={…}");
    decl.setKind(declaration);
    decl.add(body.clone());
    function.body = body.clone(); //.flat(); // debug or use?
    //	decl["signature"]=*new Node("signature");
    if (signature.functions.size() == 0)
        signature.functions.add(&function);
    decl["@signature"].value.data = &signature;
    //    function.body= &body;
    return decl;
}

Node extractModifiers(Node &expression) {
    Node modifieres;
    for (auto &child: expression) {
        if (function_modifiers.contains(child.name)) {
            modifieres.add(child);
            expression.remove(child);
        }
    }
    return modifieres;
}


// def foo(x,y) => x+y  vs  groupFunctionDeclaration foo := x+y
Node &groupFunctionDefinition(Node &expression, Function &context) {
    auto first = expression.first();
    Node modifieres = extractModifiers(expression);
    auto kw = expression.containsAny(function_keywords, false); // todo fest='def' QUOTED!!
    if (expression.index(kw) != 0)
        error("function keywords must be first");
    expression.children++; // get rid of first 'function' keyword
    expression.length--; // get rid of first 'function' keyword
    auto &fun = expression.first();
    Node *return_type = 0;
    Node arguments = groupTypes(fun.childs(), context); // children f(x,y)
    Node body;
    auto ret = expression.containsAny(return_keywords);
    if (ret) {
        return_type = &expression.from(ret);
        expression = expression.to(ret);
        body = return_type->values(); // f(x,y) -> int { x+y }
    } else if (expression.length == 3) {
        // f(x,y) int { x+y }
        return_type = &expression[1];
        body = expression.last();
    } else if (expression.length == 2) {
        body = expression.last();
    } else body = fun.values();

    auto opa = expression.containsAny(function_operators); // fun x := x+1
    if (opa)
        body = expression.from(opa);
    return groupFunctionDeclaration(fun.name, return_type, NIL, arguments, body, context);
}

// f x:=x*x  vs groupFunctionDefinition fun x := x*x
Node &groupFunctionDeclaration(Node &expression, Function &context) {
    Node modifieres = extractModifiers(expression);
    auto op = expression.containsAny(function_operators);
    auto &left = expression.to(op);
    auto &rest = expression.from(op);
    auto fun = left.first();
    return groupFunctionDeclaration(fun.name, 0, left, left, rest, context);
}

Node &groupDeclarations(Node &expression, Function &context) {
    if (expression.kind == groups) // handle later!
        return expression;
    //    if (expression.kind != Kind::expression)return expression;// 2022-19 sure??
    if (expression.contains(":=")) {
        // add_raw
        return groupFunctionDeclaration(expression, context);
    }

    auto &first = expression.first();
    //	if (contains(functor_list, first.name))
    //		return expression;
    if (expression.kind == declaration) {
        if (isType(first)) {
            auto &fun = expression[1];
            String name = fun.name;
            Node *typ = first.clone();
            Node modifieres = NIL;
            Node &arguments = fun.values();
            Node &body = expression.last();
            return groupFunctionDeclaration(name, typ, NIL, arguments, body, context);
        } else if (isType(first.first())) {
            auto &fun = first[1];
            String name = fun.name;
            Node &typ = first.first();
            Node modifieres = NIL;
            Node &arguments = expression[1];
            Node &body = expression.last();
            return groupFunctionDeclaration(name, &typ, NIL, arguments, body, context);
        } else {
            warn("declaration"s + expression.serialize());
            return expression;
        }
    }
    for (Node &node: expression) {
        String &op = node.name;
        if (isKeyword(op))
            continue;
        if (isPrimitive(node) and node.isSetter()) {
            addLocal(context, op, preEvaluateType(node, context), false);
            continue;
        }
        if ((node.kind == reference and not node.length) or
            (node.kind == key and maybeVariable(node))) {
            // only constructors here!
            if (not globals.has(op) and not isFunction(node) and not builtin_constants.contains(op)) {
                Type evaluatedType = unknown_type;
                if (use_wasm_arrays)
                    evaluatedType = preEvaluateType(node, context); // todo turns sin π/2 into 1.5707963267948966 ;)
                addLocal(context, op, evaluatedType, false);
            }
            continue;
        } // todo danger, referenceIndices i=1 … could fall through to here:
        if (node.kind != declaration and not declaration_operators.has(op))
            continue;
        if (op.empty())continue;
        if (op == "=" or op == ":=") continue; // handle assignment via groupOperators !
        if (op == "::=") continue; // handle globals assignment via groupOperators !
        // todo: public export function jaja (a:num …) := …

        // BEGINNING OF Declaration ANALYSIS
        Node left = expression.to(node); // including public… + ARGS! :(
        Node rest = expression.from(node); // body
        String name = extractFunctionName(left);
        if (left.length == 0 and not declaration_operators.has(node.name))
            name = node.name; // todo: get rid of strange heuristics!
        if (node.length > 1) {
            // C style double sin(x) {…} todo: fragile criterion!! also why is body not child of sin??
            name = node.first().name;
            left = node.first().first(); // ARGS
            rest = node.last();
            if (rest.kind == declaration)rest = rest.values();
            context.locals.remove(name); // not a variable!
        }
        ////			todo i=1 vs i:=it*2  ok ?

        if (name.empty())
            continue;
        if (isFunction(name, true)) {
            node.kind = call;
            continue;
        }
        //			error("Symbol already declared as function: "s + name);
        // if (function.locals.has(name)) // todo double := it * 2 ; double(4)
        //				error("Symbol already declared as variable: "s + name);
        //			if (isImmutable(name))
        //				error("Symbol declared as constant or immutable: "s + name);
        return groupFunctionDeclaration(name, 0, left, left, rest, context);
    }
    return expression;
}



Node extractReturnTypes(Node decl, Node body) {
    return DoubleType; // LongType;// todo
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
        fromRight = fromRight || (prefixOperators.has(op) and op != "-"); // !√!-1 == !(√(!(-1)))
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
            if (suffixOperators.has(op)) {
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

Module &loadRuntime() {
#if WASM // and not MY_WASM
    if (not module_cache.has("wasp"s.hash()))
        error("module 'wasp' should already be loaded through js load_runtime_bytes => parseRuntime");
    Module &wasp = *module_cache["wasp"s.hash()];
    //    wasp.functions["powi"].signature.returns(int32);
    // if(!libraries.has(&wasp))// on demand per test/app!
    return wasp;
#else
    // Module &wasp = read_wasm("wasp-runtime-debug.wasm");// todo unexpected opcode …!!!
    Module &wasp = read_wasm("wasp-runtime.wasm");
    wasp.functions["getChar"].signature.returns(codepoint1);
    // addLibrary(&wasp); // BREAKS system WHY?
    // if(!libraries.has(&wasp))
    //     libraries.add(&wasp); // BREAKS system WHY?
    return wasp;
#endif
}

Type preEvaluateType(Node &node, Function &context) {
    // e.g √3 => float64t  3/2 => float64t …
    // todo: some kind of Interpret eval?
    // todo: combine with compile time eval! <<<<<
    if (node.kind == expression) {
        if (node.name == "if") // if … then (type 1) else (type 2)
            return commonType(preEvaluateType(node["then"], context), preEvaluateType(node["else"], context), "if");
        if (node.length == 1)return preEvaluateType(node.first(), context);
        node = groupOperators(node, context);
        return mapType(node.name);
    }
    if (node.kind == operators) {
        if (node.name == "√")return float64t; // todo generalize
        Node &lhs = node.first();
        auto lhs_type = preEvaluateType(lhs, context);
        if (node.length <= 1)
            return lhs_type;
        Node &rhs = node[1];
        auto rhs_type = preEvaluateType(rhs, context); // todo lol
        auto type = commonType(lhs_type, rhs_type, node.name);
        // todo: operators which are not endofunctions
        return type;
        //        if(lhs.kind==arrays)
    }
    if (isGroup(node.kind)) {
        //        arrayElementSize(node);// adds type as BAD SIDE EFFECT
        //        if (use_wasm_arrays) {// todo
        //        auto valueType = preEvaluateType(node.values(), context);
        auto valueType = commonElementType(node);
        if (valueType == unknown_type) return unknown_type;
        return genericType(node.kind, valueType);
        //        }
    }
    if (node.kind == reference) {
        if (context.locals.has(node.name)) {
            auto local = context.locals[node.name];
            return local.type;
        }
        if (node.size() == 1 and use_wasm_arrays) // todo
            return preEvaluateType(node.first(), context);
    }
    return mapType(node);
}


//Type preEvaluateType(Node &node, Function &context) {
Type preEvaluateType(Node &node, Function *context0) {
    if (!context0)context0 = new Function{.name = "ad-hoc"};
    Function &context = *context0;
    return preEvaluateType(node, context);
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


int findBestVariant(const Function &function, const Node &node, Function *context) {
    if (function.variants.size() == 1) return 0;
    if (function.variants.size() == 0) return -1;
    int best = -1;
    for (Function *variant: function.variants) {
        best++;
        auto &signature = variant->signature;
        if (signature.size() == node.length) {
            bool ok = true;
            for (int i = 0; i < signature.size(); ++i) {
                auto &sig = signature.parameters[i];
                Type aType = sig.type;
                Type pType = preEvaluateType(*node[i].clone(), context);
                if (not compatibleTypes(aType, pType)) {
                    ok = false;
                    break;
                }
            }
            if (ok)
                return best;
        }
    }
    error("no matching function variant for "s + function.name + " with "s + node.serialize());
}


// todo merge with groupOperatorCall
Node &groupFunctionCalls(Node &expressiona, Function &context) {
    if (expressiona.kind == declaration)return expressiona; // handled before
    Function *import = findLibraryFunction(expressiona.name, false);
    if (import or isFunction(expressiona)) {
        expressiona.setKind(call, false);
        if (not functions.has(expressiona.name))
            error("! missing import for function "s + expressiona.name);
        //		if (not expressiona.value.node and arity>0)error("missing args");
        functions[expressiona.name].is_used = true;
    }
    for (int i = 0; i < expressiona.length; ++i) {
        Node &node = expressiona.children[i];
        String &name = node.name;

        // if (debug) printf("  child[%d]: name=%s, kind=%s (%d), length=%d, serialize=%s\n", i, name.data, typeName(node.kind), node.kind, node.length, node.serialize().data);

        // todo: MOVE!
        if (name == "if") // kinda functor
        {
            // error("if should be treated earlier");
            auto &args = expressiona.from("if");
            Node &iff = groupIf(node.length > 0 ? node.add(args) : args, context);
            int j = expressiona.lastIndex(iff.last().next) - 1;
            if (i == 0 and j == expressiona.length - 1)return iff;
            if (j > i)
                expressiona.replace(i, j, iff); // todo figure out if a>b c d e == if(a>b)then c else d; e boundary
            if (i == 0)
                return expressiona;
            continue;
        }
        //        if (name == "for") {
        //            Node &forr = groupFor(node, context);
        //            int j = expressiona.lastIndex(forr.last().next) - 1;
        //            if (j > i)
        //                expressiona.replace(i, j, forr);
        //            continue;
        //        }
        if (name == "while") {
            // error("while' should be treated earlier");
            // todo: move into groupWhile !!
            if (node.length == 2) {
                node[0] = analyze(node[0], context);
                node[1] = analyze(node[1], context);
                continue; // all good
            }
            if (node.length == 1) {
                // while()… or …while()
                node[0] = analyze(node[0], context);
                Node then = expressiona.from("while"); // todo: to closer!?
                int remaining = then.length;
                node.add(analyze(then, context).clone());
                expressiona.remove(i + 1, i + remaining);
                continue;
            } else {
                Node n = expressiona.from("while");
                Node &iff = groupWhile(n, context); // todo: sketchy!
                int j = expressiona.lastIndex(iff.last().next) - 1; // huh?
                if (j > i)expressiona.replace(i, j, iff);
            }
        }
        // Check if this is a struct constructor (before checking isFunction)
        if (types.has(name) and node.length > 0) {
            Node &type = *types[name];
            if (type.kind == clazz) {
                // This is a struct constructor, not a function call
                if (debug) printf("Detected struct constructor: %s\n", name.data);
                node = constructInstance(node, context);
                continue; // Skip function processing for constructors
            }
        }

        if (isFunction(node)) // needs preparsing of declarations!
            node.kind = call;

        if (node.kind != call)
            continue;

        Function *ok = findLibraryFunction(name, true);
        if (not ok and not functions.has(name)) // todo load lib!
            error("missing import for function "s + name);
        Function &function = functions[name];
        function.name = name; // hack shut've Never Been Lost
        Signature &signature = function.signature;
        if (function.is_polymorphic) {
            auto &params0 = expressiona.from(i + 1);
            auto &params = analyze(params0, context);
            int variantNr = findBestVariant(function, *wrap(params), &context);
            Function *variant = function.variants[variantNr];
            signature = variant->signature; // todo hack
            variant->is_used = true;
            if (function.is_runtime and not variant->fullname.empty() and variant->fullname != function.name)
                functions.add(variant->fullname, *variant); // needs extra call index!
            addLibraryFunctionAsImport(*variant);
            print("matching function variant "s + variantNr + " of " + function.name + " with "s + signature.
                  serialize());
            node.add(params); // todo: this is all duplication of code below:
            expressiona.remove(i + 1, -1); // todo
            // expressiona.replace(i+1, -1, params);// todo
            // expressiona.replace(i+1, i + params.length, params);// todo
        }
        // return groupFunctionCallPolymorphic(node, function, expressiona, context);
        function.is_used = true;


        int minArity = signature.size(); // todo: default args!
        int maxArity = signature.size();

        if (node.length > 0) {
            //			if minArity == …
            node = analyze(node.flat(), context); //  f(1,2,3) vs f([1,2,3]) ?
            continue; // already done how
        }
        if (minArity == 0)continue;
        if (maxArity < 0)continue; // todo
        Node rest;
        if (i < expressiona.length - 1 and expressiona[i + 1].kind == groups) {
            // f(x)
            // todo f (x) (y) (z)
            // todo expressiona[i+1].length>=minArity
            rest = expressiona[i + 1];
            if (rest.length > 1)
                rest.setKind(expression);
            Node args = analyze(rest, context);
            node.add(args);
            expressiona.remove(i + 1, i + 1);
            continue;
        }
        rest = expressiona.from(i + 1);
        int arg_length = rest.length;
        if (not arg_length and rest.kind == urls) arg_length = 1;
        if (not arg_length and rest.kind == reference) arg_length = 1;
        if (not arg_length and rest.value.data) arg_length = 1;
        if (arg_length > 1)
            rest.setKind(expression); // SUUURE?
        if (rest.kind == groups or rest.kind == objects) // and rest.has(operator))
            rest.setKind(expression); // todo f(1,2) vs f(1+2)
        //		if (hasFunction(rest) and rest.first().kind != groups)
        //				warn("Ambiguous mixing of functions `ƒ 1 + ƒ 1 ` can be read as `ƒ(1 + ƒ 1)` or `ƒ(1) + ƒ 1` ");
        // per-function precedence does NOT really increase readability or bug safety
        if (rest.value.data) {
            maxArity--; // ?
            minArity--;
        }
        if (arg_length < minArity) {
            print(function.name + (String) signature);
            error("missing arguments for function %s, given %d < expected %d. "
                "defaults and currying not yet supported"s % name % arg_length % minArity);
        } else if (arg_length == 0 and minArity > 0)
            error("missing arguments for function %s, or to pass function pointer use func keyword"s % name);
        Node &args = analyze(rest, context); // todo: could contain another call!
        node.add(args);
        expressiona.remove(i + 1, i + arg_length);
    }
    return expressiona;
}




Node &groupForClassic2(Node &node, Function &context) {
    // for(…){}
    if (node.length < 2)
        error("Invalid 'for' loop structure. Expected initializer and body.");
    // Extract components
    Node &forex = node[0]; // e.g., 'for'
    Node &fore = forex.values(); // e.g., 'i = 0'
    if (fore.size() != 3)
        error("Invalid 'for' loop structure. Expected three parts like for(i=0;i<10;i++){}");

    Node initializer = fore[0];
    Node condition = fore[1];
    Node increment = fore[2];
    initializer = analyze(initializer, context);
    condition = analyze(condition, context);
    increment = analyze(increment, context);
    // todo for loop creates it's own scope!!
    Node &body = analyze(node[1], context); // Loop body
    // Create a node for the 'for' loop
    Node *forNode = new Node("fori");
    Node &grouped = *forNode;
    grouped.kind = expression; // Mark as an expression node

    // Group components into structured nodes
    grouped["initializer"] = initializer; // Store the initializer
    grouped["condition"] = condition; // Store the condition
    grouped["increment"] = increment; // Store the increment
    grouped["body"] = body; // Store the loop body

    return grouped;
}

Node &groupForClassic(Node &node, Function &context) {
    // // for i=0;i<10;i++ {}
    if (node.length == 2)return groupForClassic2(node, context);
    // if (node.length < 4)
    //     error("Invalid 'for' loop structure. Expected initializer, condition, increment, and body.");

    // Extract components
    Node &initializer = node[0]; // e.g., 'i = 0'
    Node &condition = node[1]; // e.g., 'i < 1000'
    Node &increment = node[2]; // e.g., 'i++'
    Node &body = node[3]; // Loop body

    // Validate components
    if (!initializer.value.data)
        error("Missing initializer in 'for' loop.");
    if (!condition.value.data)
        error("Missing condition in 'for' loop.");
    if (!increment.value.data)
        error("Missing increment in 'for' loop.");

    // Create a node for the 'for' loop
    Node *forNode = new Node("fori");
    Node &grouped = *forNode;
    grouped.kind = expression; // Mark as an expression node

    // Group components into structured nodes
    grouped["initializer"] = initializer; // Store the initializer
    grouped["condition"] = condition; // Store the condition
    grouped["increment"] = increment; // Store the increment
    grouped["body"] = body; // Store the loop body

    return grouped;
}

Node &groupForIn(Node &n, Function &context) {
    // for i in 0 to 10 {}
    // for i in 1 < 3 {}
    //    todo : for i < 3
    List<String> ops = {"to", "…", "upto", "<", "<=", "..", "..<", "until", ">", ">=", "...", "downto"};
    if (n[2].name != "in" or not ops.has(n[4].name))
        error("Invalid 'for' loop structure. Expected: for i in begin to end {}");
    Node &variable = n[1];
    addLocal(context, variable.name, int32t, false);
    Node &begin = n[3];
    Node &end = n[5];
    Node &body = end.values(); // for i in 1 to 5 : {print i}
    if (n.length == 6) body = n[5];
    if (n.length > 6) body = n.from(6);
    if (n.length > 6 and n[6].name == ":") body = n.from(7);

    if (body.kind != longs) // todo: hacky! put in there ?
        body.setKind(expression); // if …
    Node *foro = new Node("for");
    Node &ef = *foro;
    ef.kind = expression;
    ef["variable"] = analyze(variable, context);
    ef["begin"] = analyze(begin, context);
    ef["end"] = analyze(end, context);
    ef["body"] = analyze(body, context);
    ef["upto"] = *new Node(n[4].name == "upto" or n[4].name == "<");
    analyzed[ef.hash()] = 1;
    return ef;
}

Node &groupFor(Node &n, Function &context) {
    if (n.length < 2)
        error("Incomplete 'for' loop structure");
    if (n.length == 2) // for(…){}
        return groupForClassic2(n, context);
    if (n.length > 2 and n[2].name == "in")
        return groupForIn(n, context);
    if (n.length > 7 or n[2].name != "in")
        return groupForClassic(n, context);

    // Extract components: variable, iterable, and body
    // for i in iterable {}
    Node &variable = n.children[0];
    Node &iterable = n.children[1];
    Node body = (n.length > 2) ? n.children[2] : Node();

    addLocal(context, variable.name, int32t, false);

    // Create and structure the "for" node
    Node *foro = new Node("for");
    Node &ef = *foro;
    ef.kind = expression;
    ef["variable"] = analyze(variable, context);
    ef["iterable"] = analyze(iterable, context);
    ef["body"] = analyze(body, context);
    analyzed[ef.hash()] = 1;
    return ef;
}

Node &groupWhile(Node &n, Function &context) {
    // Check for minimum structure
    if (n.length == 0 && !n.value.data)
        error("Missing condition for while statement");
    if (n.length == 1 && !n.value.data)
        error("Missing block for while statement");

    // Extract condition and 'then' block
    Node &condition = n.children[0];
    Node then = (n.length > 1) ? n.children[1] : Node(); // Use explicit initialization

    // Handle ":" and "do" grouping
    if (n.has(":")) {
        condition = n.to(":");
        then = n.from(":");
    }

    // Handle standalone conditions and alternative grouping cases
    if (condition.value.data && !condition.next)
        then = condition.values();
    if (condition.kind == reference) {
        for (Node &child: n) {
            if (child.kind == groups || child.kind == objects) {
                condition = n.to(child);
                then = child;
                break;
            }
        }
    }
    // Create and structure the "while" node
    Node *whilo = new Node("while");
    Node &ef = *whilo;
    ef.kind = expression;
    ef["condition"] = analyze(condition, context);
    ef["then"] = analyze(then, context);
    analyzed[ef.hash()] = 1;
    return ef;
}

Node &groupOperatorCall(Node &node, Function &function);

Type guessType(Node &node, Function &function) {
    return preEvaluateType(node, function); // todo: use function context?
}


// todo move to parseTemplate
Node &groupTemplate(Node &node, Function &function) {
    for (Node &child: node) {
        Type kind = child.kind;
        if (kind != strings) {
            child = analyze(child, function); // analyze each child node
            kind = guessType(child, function);
            if (kind == expression)
                kind = child.first().kind; // todo: hacky, but works for now
            if (kind == referencex or kind == reference) useFunction("toString");
            else if (kind == undefined) useFunction("formatLong"); // TODO!
            else if (kind == long32) useFunction("formatLong"); // itoa0
            else if (kind == longs) useFunction("formatLong"); // ltoa
            else if (kind == reals or kind == realsF) useFunction("formatReal"); // ftoa
            else if (kind == doubles) useFunction("formatReal");
            else if (kind == floats) useFunction("formatReal"); // todo via auto upcast?
            else
                error("Unknown template child type: "s + typeName(kind));
        }
    }
    // useFunction("_Z12concat_charsPKcS0_");
    useFunction("concat");
    // useFunction("concat_chars");
    // functions["concat_chars"].is_used = true;
    return node;
}

Node &analyze(String code) {
    return analyze(parse(code));
}

/*
 * ☢️ ⚛ Nuclear Core ⚠️ 🚧
 * turning some knobs might yield some great powers
 * touching anything can result in disaster
 * should be replaced with elegant modular solar panels
 */
Node &analyze(Node &node, Function &function) {
    String &context = function.name;
    if (context != "global" and !functions.has(context)) {
        function.is_declared = true;
        functions.add(context, function); // index not known yet
    }
    int64 hash = node.hash();
    if (analyzed.has(hash))
        return node;

    Kind type = node.kind;
    String &name = node.name;
    auto firstName = node.first().name;

    if (firstName == "data" or firstName == "quote")
        return node; // data keyword leaves data completely unparsed, like lisp quote `()

    if (name == "html") {
        useFunction("getDocumentBody");
        useFunction("createHtml");
        useFunction("createHtmlElement");
        // also getHtmlAttribute / createHtmlAttribute / setHtmlAttribute / property
        return node; // html builder currently not parsed
    }
    if (name == "js" or name == "script" or name == "javascript") {
        useFunction("addScript");
        return node;
    }
    if (name == "if")return groupIf(node, function);
    if (name == "while")return groupWhile(node, function);
    if (name == "for" or firstName == "for")return groupFor(node, function);
    if (name == "?")return groupIf(node, function);
    if (name == "module") {
        if (!module)module = new Module();
        module->name = node.string(); // todo: use?
        return NUL;
    }
    // class/struct declaration!
    if (not firstName.empty() and class_keywords.contains(firstName))
        return classDeclaration(node, function);


    // add: func(a: float32, b: float32) -> float32
    if (node.kind == key and node.values().name == "func") // todo move edge case to witReader
        return funcDeclaration(node.name, node.values(), NUL /* no body here */ , 0, function.module);
    //        return witReader.analyzeWit(node);

    if ((type == expression and not name.empty() and not name.contains("-")))
        addLocal(function, name, int32t, false); //  todo deep type analysis x = π * fun() % 4
    if (type == key) {
        if (node.value.node /* i=ø has no node */) {
            Node &analyzed = analyze(*node.value.node, function);
            // Preserve struct constructor type information through cloning
            Node *cloned = analyzed.clone();
            if (debug && cloned->type != analyzed.type) {
                printf("WARNING: clone lost type! analyzed.type=%p, cloned->type=%p\n",
                       (void *) analyzed.type, (void *) cloned->type);
            }
            node.value.node = cloned;
        }
        if (node.length > 0) {
            // (double x, y)  (while x) : y
            auto first = node.first().first();
            if (isType(first))
                return groupTypes(node, function);
            else if (first.name == "while")
                return groupWhile(node, function);
            else if (first.name == "for")
                return groupFor(node, function);
            else if (first.name == "if")
                return groupIf(node, function);
            else if (node.length > 1)
                error("unknown key expression: "s + node.serialize());
        }
        addLocal(function, name, node.value.node ? mapType(node.value.node) : none, false);
    }
    if (type == strings and node.type == &TemplateType)
        return groupTemplate(node, function); // todo: move to witReader

    if (isGlobal(node, function))
        return groupGlobal(node, function);
    if (isPrimitive(node)) {
        if (node.kind == strings and node.next and node.next->kind == strings)
            useFunction("concat");
        if (maybeVariable(node))
            addLocal(function, name, mapType(node), false);
        return node; // nothing to be analyzed!
    }
    // function_keywords.has(node.name / values)
    if (node.containsAny(function_keywords, false))
        return groupFunctionDefinition(node, function);

    if (type == operators or type == call or isFunction(node)) //todo merge/clean
        return groupOperatorCall(node, function); // call, NOT definition

    Node &groupedTypes = groupTypes(node, function);
    Node &groupedDeclarations = groupDeclarations(groupedTypes, function);
    Node &groupedFunctions = groupFunctionCalls(groupedDeclarations, function);
    Node &grouped = groupOperators(groupedFunctions, function);
    if (analyzed[grouped.hash()])return grouped; // done!
    analyzed.insert_or_assign(grouped.hash(), 1);
    if (grouped.kind == referencex or grouped.name.startsWith("$")) {
        // external reference, e.g. html $id
        useFunction("getElementById");
        useFunction("toNode"); // todo get rid of these:
        useFunction("toString");
        useFunction("toLong");
        useFunction("toReal");
        // useFunction("toInt");
        // useFunction("toReal");
    }
    //    Node& last;
    if (isGroup(type)) {
        // handle lists, arrays, objects, … (statements)
        if (grouped.length > 0)
            for (Node &child: grouped) {
                // children of lists analyzed individually
                if (!child.name.empty() and wit_keywords.contains(child.name) and child.kind != strings /* TODO … */) {
                    WitReader witReader;
                    return witReader.analyzeWit(node); // can't MOVE there:
                }
                child = analyze(child, function); // REPLACE ref with their ast ok?
                //                last.next = child;
            }
    }
    return grouped;
}


extern "C" int64 run_wasm_file(chars file) {
    let buffer = load(String(file));
#if RUNTIME_ONLY
    error("RUNTIME_ONLY");
    return -1;
#else
    return run_wasm((bytes) buffer.data, buffer.length);
#endif
}



// emit via library merge
Node runtime_emit(String prog) {
#ifdef RUNTIME_ONLY
    printf("emit wasm not built into release runtime");
    return ERROR;
#endif
    libraries.clear(); // todo reuse
    clearAnalyzerContext();
    clearEmitterContext();
    Module &runtime = loadRuntime();
    runtime.code.needs_relocate = false;
    Code code = compile(prog, false); // should use libraries!
    code.needs_relocate = false;
    code.save("merged.wasm");
    int64 result_val = code.run(); // todo parse stdout string as node and merge with emit() !
    return *smartNode(result_val);
}
