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
#include "FunctionAnalyzer.h"
#include "OperatorAnalyzer.h"
#include "ffi_signatures.h"

#if INCLUDE_MERGER and not RUNTIME_ONLY
#include "wasm_merger.h"
#endif


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
            tracef("Converting %s{...} to constructor\n", name.data);
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



Node &classDeclaration(Node &node, Function &function);

Node &classDeclaration(Node &node, Function &function) {
    // including class type interface struct record keywords !
    tracef("classDeclaration called\n");
    if (node.length < 2)
        error("wrong class declaration format; should be: class name{…}");
    Node &dec = node[1];
    String &kind_name = node.first().name;
    tracef("  kind_name=%s, dec.name=%s\n", kind_name.data, dec.name.data);

    String &name = dec.name;
    // If type already exists (from collectStructDeclarations), just return the existing one
    if (types.has(name)) {
        tracef("  Type '%s' already in types map (from collectStructDeclarations), returning existing\n",
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
        tracef("  Adding type '%s' to types map\n", name.data);
        types.add(name, dec.clone());
        tracef("  After add: types.has('%s')=%p\n", name.data, (void *) types.has(name));
    }
    return dec;
}





// def foo(x,y) => x+y  vs  groupFunctionDeclaration foo := x+y

// f x:=x*x  vs groupFunctionDefinition fun x := x*x






// outer analysis 3 + 3  ≠ inner analysis +(3,3)
// maybe todo: normOperators step (in angle, not wasp!)  3**2 => 3^2

// extra logic for a-b kebab-case vs minus operator

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


// todo merge with groupOperatorCall




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

    // Handle ":", "do" grouping
    if (n.has(":")) {
        condition = n.to(":");
        then = n.from(":");
    }
    if (n.has("do")) {
        condition = n.to("do");
        then = n.from("do");
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

void handleNativeImport(Node &node) {
    // Extract function and library from import node structure
    Node &func_node = node["function"];
    Node &lib_node = node["library"];
    String func_name = func_node.length > 0 ? func_node.first().name : func_node.values().name;
    String lib_name = lib_node.length > 0 ? lib_node.first().name : lib_node.values().name;

    // Get or create native library Module
    extern Map<String, Module *> native_libraries;  // Defined in Context.cpp
    Module *lib_module;
    if (!native_libraries.has(lib_name)) {
        lib_module = new Module();
        lib_module->name = lib_name;
        lib_module->is_native_library = true;
        native_libraries.add(lib_name, lib_module);
    } else {
        lib_module = native_libraries[lib_name];
    }

    // Register as FFI import in global functions map
    Function &func = functions[func_name];
    func.name = func_name;
    func.is_import = true;
    func.is_ffi = true;
    func.ffi_library = lib_name;
    func.is_used = true;
    func.module = lib_module;

    // Detect and set function signature directly (reuses existing Signature class)
    detect_ffi_signature(func_name, lib_name, func.signature);

    // Also add to native library module's functions map (if not already present)
    if (!lib_module->functions.has(func_name)) {
        lib_module->functions.add(func_name, func);
        lib_module->export_names.add(func_name);
    }
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

    if (name.in(import_keywords) || firstName.in(import_keywords)) {
        if(node.has("function") and node.has("library")) {
            handleNativeImport(node);
            return NUL;
        }

        // Handle `use <library>` for native libraries (e.g., use math, use m)
        if (node.length > 0) {
            String lib_name = node.last().name;
            // Map common library names to native library names
            if (lib_name == "math") lib_name = "m";

            // Check if it's a native library (libc="c", libm="m", etc.)
            if (lib_name == "m" || lib_name == "c") {
                extern Map<String, Module *> native_libraries;

                // Get or create native library Module
                Module *lib_module;
                if (!native_libraries.has(lib_name)) {
                    lib_module = new Module();
                    lib_module->name = lib_name;
                    lib_module->is_native_library = true;
                    native_libraries.add(lib_name, lib_module);
                } else {
                    lib_module = native_libraries[lib_name];
                }

                // Mark library as used so its functions are available
                extern List<Module *> libraries;
                if (!libraries.has(lib_module)) {
                    libraries.add(lib_module);
                }

                trace("Loaded native library: "s + lib_name);
                return NUL;
            }
        }
        //else todo ("move other import here")
    }

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

    // Handle FFI imports in expression children before grouping operators
    if (node.kind == expression && node.length > 0) {
        List<Node> filtered_children;
        for (int i = 0; i < node.length; i++) {
            Node &child = node.children[i];
            if (child.name == "import" && child.kind == functor && child.has("function") && child.has("library")) {
                handleNativeImport(child);
                // Don't add import node to filtered children (skip it)
            } else if (child.name == "\n" || child.name == " " || child.name == "\t" || child.name == "\\") {
                // Skip whitespace/newline nodes
            } else {
                // Keep non-import, non-whitespace nodes
                filtered_children.add(child);
            }
        }
        // Rebuild node.children without imports and whitespace
        if (filtered_children.size() < node.length) {
            node.length = filtered_children.size();
            for (int i = 0; i < filtered_children.size(); i++) {
                node.children[i] = filtered_children[i];
            }
        }
    }

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
#if RUNTIME_ONLY
    error("RUNTIME_ONLY");
    return -1;
#else
    String found = String(file);
    if(found.endsWith(".wat") or found.endsWith(".wast") )
        found = compileWast(file);
    let buffer = load(found);
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
