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
groupFunctionDeclaration(String &name, Node *return_type, Node modifieres, Node &arguments, Node &body, Function &context) {
    if (name and not function_operators.has(name.data)) {
        check (context.name == "wasp_main")
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
    for (auto &child: expression)
        if (function_modifiers.contains(child.name)) {
            modifieres.add(child);
            expression.remove(child);
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
    // if(expression.kind == declaration and expression["params"]) {} no extra logic in Wasp.cpp !
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
        if (node.kind != declaration and not declaration_operators.has(op.data))
            continue;
        if (op.empty())continue;
        if (op == "=" or op == ":=") continue; // handle assignment via groupOperators !
        if (op == "::=") continue; // handle globals assignment via groupOperators !
        // todo: public export function jaja (a:num …) := …

        // BEGINNING OF Declaration ANALYSIS
        Node left = expression.to(node); // including public… + ARGS! :(
        Node rest = expression.from(node); // body
        String name = extractFunctionName(left);
        if (left.length == 0 and not declaration_operators.has(node.name.data))
            name = node.name; // todo: get rid of strange heuristics!
        if (node.length > 1) {
            // C style double sin(x) {…} todo: fragile criterion!! also why is body not child of sin??
            name = node.first().name;
            left = node.first().first(); // ARGS
            rest = node.last();
            if (rest.kind == declaration)rest = rest.values();
            String &nam = name;
            context.locals.remove(nam); // not a variable!
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

Node &parseWhileExpression(Node &node, Node &expressiona, int i, Function &context) {
    if (node.length == 2) {
        node[0] = analyze(node[0], context);
        node[1] = analyze(node[1], context);
        return node; // all good
    }
    if (node.length == 1) {
        // while()… or …while()
        node[0] = analyze(node[0], context);
        Node then = expressiona.from("while"); // todo: to closer!?
        int remaining = then.length;
        node.add(analyze(then, context).clone());
        expressiona.remove(i + 1, i + remaining);
        return node;
    }
    Node n = expressiona.from("while"s);
    Node &iff = groupWhile(n, context); // todo: sketchy!
    int j = expressiona.lastIndex(iff.last().next) - 1; // huh?
    if (j > i)
        expressiona.replace(i, j, iff);
    return iff;
}

// todo this function needs some serious refactor namely get rid of the "if" matching …
Node &groupFunctionCalls(Node &node, Function &context) {
    if (node.kind == declaration)return node; // handled before
    Function *import = findLibraryFunction(node.name, false);
    import = findLibraryFunction(node.name, false); // DEBUG!
    if (import or isFunction(node)) {
        node.setKind(call, false);
        if (not functions.has(node.name))
            error("! missing import for function "s + node.name);
        //		if (not expressiona.value.node and arity>0)error("missing args");
        functions[node.name].is_used = true;
    }
    if (node.kind != expression and not isFunction(node.first()) and
        node.first().name != "if" and
        node.first().name != "while")
        return node;
    for (int i = 0; i < node.length; ++i) {
        Node &child = node.children[i];
        String &name = child.name;
        // todo: MOVE to analyze()!
        if (name == "if") // kinda functor
        {
            if(i>0)error("if should be first in expression");
            // error("if should be treated earlier");
            auto &args = node.from("if");
            return groupIf(child.length > 0 ? child.add(args) : args, context);
        }
        if (name == "while") {// ok different:
        // todo: MOVE to analyze()!
            // i=1;while(i<9)i++;i+1 has leftovers!! IGNORE EDGE CASE! or use condition.next
            // i=1;while i<10 do {i++};i
            // error("while' should be treated earlier");
            parseWhileExpression(child, node, i, context);
            continue;
        }
        // Check if this is a struct constructor (before checking isFunction)
        if (types.has(name) and child.length > 0) {
            Node &type = *types[name];
            if (type.kind == clazz) {
                // This is a struct constructor, not a function call
                if (debug) printf("Detected struct constructor: %s\n", name.data);
                child = constructInstance(child, context);
                continue; // Skip function processing for constructors
            }
        }

        if (isFunction(child)) // needs preparsing of declarations!
            child.kind = call;

        if (child.kind != call)
            continue;

        Function *ok = findLibraryFunction(name, true);
        if (not ok and not functions.has(name)) // todo load lib!
            error("missing import for function "s + name);
        Function &function = functions[name];
        function.name = name; // hack shut've Never Been Lost
        Signature &signature = function.signature;
        if (function.is_polymorphic) {
            auto &params0 = node.from(i + 1);
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
            child.add(params); // todo: this is all duplication of code below:
            node.remove(i + 1, -1); // todo
            // expressiona.replace(i+1, -1, params);// todo
            // expressiona.replace(i+1, i + params.length, params);// todo
        }
        // return groupFunctionCallPolymorphic(node, function, expressiona, context);
        function.is_used = true;


        int minArity = signature.size(); // todo: default args!
        int maxArity = signature.size();

        if (child.length > 0) {
            //			if minArity == …
            child = analyze(child.flat(), context); //  f(1,2,3) vs f([1,2,3]) ?
            continue; // already done how
        }
        if (minArity == 0)continue;
        if (maxArity < 0)continue; // todo
        Node rest;
        if (i < node.length - 1 and node[i + 1].kind == groups) {
            // f(x)
            // todo f (x) (y) (z)
            // todo expressiona[i+1].length>=minArity
            rest = node[i + 1];
            if (rest.length > 1)
                rest.setKind(expression);
            Node args = analyze(rest, context);
            child.add(args);
            node.remove(i + 1, i + 1);
            continue;
        }
        rest = node.from(i + 1);
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
        child.add(args);
        node.remove(i + 1, i + arg_length);
    }
    return node;
}
