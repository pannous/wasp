// Function and declaration analysis extracted from Angle.cpp

#include "FunctionAnalyzer.h"
#include "Angle.h"
#include "Context.h"
#include "Keywords.h"
#include "CharUtils.h"

String extractFunctionName(Node &node) {
    if (not node.name.empty() and node.name != ":=")
        return node.name;
    if (node.length > 1)
        return node.first().name;
    // todo: public go home to family => go_home
    return node.name;
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
