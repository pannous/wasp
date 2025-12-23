// AST Analysis Context (in Angle.cpp after Wasp.cpp parser, before wasm_emitter.cpp)
// clearAnalyzerContext(); ≠
// clearEmitterContext();

#include "Context.h"

#include "Angle.h"
#include "CharUtils.h"
#include "Map.h"
#include "Code.h"
#include "Keywords.h"
#include "wasm_reader.h"

#ifdef NATIVE_FFI
#include "ffi_signatures.h"
#endif

String &normOperator(String &alias);

Module *module; // todo: use?
bool use_interpreter = false;
Node &result = *new Node();

List<String> findAliases(String name);

Map<String, Node *> types = {100}; // builtin and defined Types
Map<String, int> call_indices; // todo keep in Function


// todo Maps don't free memory
Map<String, Function> functions = {1000};
// todo ONLY emit of main module! for funcs AND imports, serialized differently (inline for imports and extra functype section)
//Map<String, Function> library_functions; see:
List<Module *> libraries; // used modules from (automatic) import statements e.g. import math; use log; …  ≠
// functions of preloaded libraries are found WITHOUT `use` `import` statement (as in Swift) !
Map<String, Module *> native_libraries = {10}; // FFI native libraries (libc="c", libm="m", etc.)

Map<int64, bool> analyzed = {1000};
// avoid duplicate analysis (of if/while) todo: via simple tree walk, not this!


// todo : use proper context ^^ instead of:
//Map<String /*function*/, List<String> /* implicit indices 0,1,2,… */> locals;
//Map<String /*function*/, List<Valtype> /* implicit indices 0,1,2,… */> localTypes;

Map<String, Global> globals;
//List<Global> globalVariables;



// todo: clarify registerAsImport side effect
// todo: return the import, not the library function
Function *findLibraryFunction(String name, bool searchAliases) {
    if (name.empty())return 0;
    if (functions.has(name))
        return use_required_import(&functions[name]); // prevents read_wasm("lib")
    // Skip operator check for mangled C++ names (they start with _Z)
    if (name.length > 0 && is_operator(name[0], false) && !name.startsWith("_Z")) {
        return 0; // skip operators
    }
#if WASM // todo: why only in wasm?
    Module &wasp = loadRuntime();
    if (wasp.functions.has(name)) {
        if (!libraries.has(&wasp)) // on demand per test/app!
            libraries.add(&wasp);
        return use_required_import(&wasp.functions[name]);
    }
#endif
    for (Module *library: libraries)
        if (library->functions.has(name))
            return use_required_import(&library->functions[name]);
    if (contains(funclet_list, name.data)) {
#if WASM and not MY_WASM
        //				todo("getWaspFunclet get library function signature from wasp");
        warn("WASP function "s + name + " getWaspFunclet todo");
        auto pFunction = getWaspFunction(name).clone();
        pFunction->import();
        return use_required_import(pFunction);
#endif
        print("loading funclet "s + name);
        Module &funclet_module = read_wasm(findFile(name, "lib"));
        check(funclet_module.functions.has(name));
        module_cache.insert_or_assign(funclet_module.name.hash(), &funclet_module);
        Function &funclet = funclet_module.functions[name];
        print("GOT funclet "s + name);
        print(funclet.signature);
        addLibrary(&funclet_module);
        return use_required_import(&funclet);
    }
    // Load runtime on-demand for runtime functions (including mangled C++ names)
    // Check runtime even if other libraries are already loaded
    Module &runtime = loadRuntime();
    bool runtime_already_loaded = libraries.has(&runtime);

    if (runtime.functions.has(name)) {
        if (!runtime_already_loaded)
            libraries.add(&runtime);
        return use_required_import(&runtime.functions[name]);
    }

    // Add runtime to libraries if function is in function_list (for future lookups)
    if (name.in(function_list) && !runtime_already_loaded)
        libraries.add(&runtime);
    // libraries.add(&loadModule("wasp-runtime.wasm")); // on demand

    Function *function = 0;
    if (searchAliases)
        for (String alias: findAliases(name))
            function = findLibraryFunction(alias, false);
        //			use_required(function); // no, NOT required yet
    auto normed = normOperator(name);
    if (normed == name)
        return use_required_import(function);
    else
        return findLibraryFunction(normed, false);
}

void addLibrary(Module *modul) {
    if (not modul)return;
    for (auto &lib: libraries)
        if (lib->name == modul->name)return;
    libraries.add(modul); // link it later via import or use its code directly?
}

// todo merge with similar functions and aliases()
Function *use_required_import(Function *function) {
    if (!function or not function->name or function->name.empty())
        return 0; // todo how?
    addLibraryFunctionAsImport(*function);
    if (function->name == "quit")
        useFunction("proc_exit");
    if (function->name == "puts")
        useFunction("fd_write");
    // for (Function *variant: function->variants) {
    //     addLibraryFunctionAsImport(*variant);
    // }
    for (String &alias: findAliases(function->name)) {
        if (alias == function->name)continue;
        auto ali = findLibraryFunction(alias, false);
        if (ali)addLibraryFunctionAsImport(*ali);
    }
    for (auto &vari: function->variants) {
        vari->is_used = true;
    }
    //    for(Function& dep:function.required)
    //        dep.is_used = true;
    return function;
}

void useFunction(String name) {
    if (not functions.has(name)) {
        auto fun = findLibraryFunction(name, true); // search aliases
        if (not fun)
            error("function "s + name + " not found"s);
    }
    trace("useFunction: "s + name);
    functions[name].is_used = true;
}

// todo: merge ^^
bool addGlobal(Function &context, String name, Type type, bool is_param, Node *value) {
    if (isKeyword(name))
        error("keyword as global name: "s + name);
    if (name.empty()) {
        warn("empty reference in "s + context);
        return true; // 'done' ;)
    }
    if (builtin_constants.has(name))
        error(name + " is already a builtin constant"s);
    if (context.signature.has(name))
        error(name + " already declared as parameter for function "s + context.name);
    if (globals.has(name)) // ok ?
        error(name + " already declared as global"s);
    //        return true;// already declared
    if (context.locals.has(name))
        error(name + " already declared as local variable"s);
    if (isFunction(name, true))
        error(name + " already declared as function"s);
    //    if(type == int32)
    //        type = int64t; // can't grow later after global init
    if (value and value->kind == expression) {
        warn("only the most primitive expressions are allowed in global initializers => move to wasp_main!");
        //        value = new Node(0); // todo reals, strings, arrays, structs, …
    } else if (not value) {
        //        value = new Node(0);
    } else {
        //        if(type==int64t)type=int32t;// todo: dirty hack for global x=1+2 because we can't cast
    }
    Global global{.index = globals.count(), .name = name, .type = type, .value = value, .is_mutable = true};
    globals[name] = global;
    return true;
}


// currently only used for WitReader. todo: merge with addGlobal
void addGlobal(Node &node) {
    String &name = node.name;
    if (isKeyword(name))
        error("Can't add reserved keyword "s + name);
    if (globals.has(name)) {
        warn("global %s already a registered symbol: %o "s % name % globals[name].value);
    } else {
        Type type = mapType(node.type);
        globals.add(name, Global{
                        /*id*/globals.count(), name, type, node.clone(), /*mutable=*/ true
                    });
    }
}


Signature &groupFunctionArgs(Function &function, Node &params) {
    //			left = analyze(left, name) NO, we don't want args to become variables!
    List<Arg> args;

    Node *nextType = &DoubleType;
    //    Node *nextType = 0;//types preEvaluateType(params, &function);
    // todo: dynamic type in square / add1 x:=x+1;add1 3
    if (params.length == 0) {
        params = groupTypes(params, function);
        if (params.name != function.name and not params.name.empty())
            args.add({function.name, params.name, params.type ? params.type : nextType});
    } //else
    for (Node &arg: params) {
        if (arg.kind == groups) {
            arg = groupTypes(arg, function,true);
            args.add({function.name, arg.name, arg.type ? arg.type : mapType(nextType), params});
            continue;
        }
        if (isType(arg)) {
            if (args.size() > 0 and args.last().type != unknown_type)
                args.last().type = types[arg.name];
            else nextType = &arg;
        } else {
            if (arg.name != function.name)
                args.add({function.name, arg.name, arg.type ? arg.type : mapType(nextType), params});
        }
    }

    Signature signature = function.signature;
    auto already_defined = function.signature.size() > 0;
    if (function.is_polymorphic or already_defined) {
        signature = *new Signature();
    }
    for (Arg arg: args) {
        auto name = arg.name;
        if (empty(name))
            name = String("$"s + signature.size());
        //            error("empty argument name");
        if (function.locals.has(name)) {
            // TODO
            warn("Already declared as local OR duplicate argument name: "s + name);
            //			error("duplicate argument name: "s + name);
        }
        signature.add(arg.type, name); // todo: arg type, or pointer
        addLocal(function, name, arg.type, true);
    }

    Function *variant = &function;
    if (already_defined /*but not yet polymorphic*/) {
        if (signature == function.signature)
            return function.signature; // function.signature; // ok compatible
        // else split two variants
        Function new_variant = function;
        Function old_variant = function; // clone by value!
        check_is(old_variant.name, function.name);
        old_variant.signature = function.signature;
        new_variant.signature = signature; // ^^ different
        function.is_polymorphic = true;
        //			function.is_used … todo copy other attributes?
        function.signature = *new Signature(); // empty
        function.variants.add(old_variant.clone());
        function.variants.add(new_variant.clone());
    } else if (function.is_polymorphic) {
        // third … variant
        variant = new Function();
        for (Function *fun: function.variants)
            if (fun->signature == signature)
                return fun->signature; // signature;
        variant->name = function.name;
        variant->signature = signature;
        function.variants.add(variant);
    } else if (!already_defined) {
        function.signature = signature;
    }
    for (auto arg: args) {
        auto name = arg.name;
        if (empty(name)) {
            warn("empty argument name, using $n"s);
            name = String("$"s + variant->locals.size());
        }
        addLocal(*variant, name, arg.type, true);
    }
    // NOW add locals to function context:
    //	for (auto arg: variant->signature.parameters)
    //		addLocal(*variant, arg.name, arg.type, true);
    return variant->signature;
    //	return signature;
}


void clearAnalyzerContext() {
    //    clearEmitterContext()
    //	needs to be outside analyze, because analyze is recursive
#ifndef RUNTIME_ONLY
    libraries.clear(); // todo: keep runtime or keep as INACTIVE to save reparsing
    native_libraries.clear(); // Clear FFI library modules between compilations
    //    module_cache.clear(); NOO not the cache lol
    types.clear();
    globals.clear();
    call_indices.clear();
    call_indices.setDefault(-1);
    analyzed.clear(); // todo move much into outer analyze function!
    functions.clear(); // always needs to be followed by
    preRegisterFunctions(); // BUG Signature wrong cpp file
#endif
}

void addLibraryFunctionAsImport(Function &func) {
    func.is_used = true;
    if (func.is_declared)return;
    if (func.is_builtin)return;
    //	auto function_known = functions.has(func.name);

    // ⚠️ this function now lives inside Module AND as import inside "wasp_main" functions list, with different wasm_index!
    bool function_known = functions.has(func.name);
#if WASM
    Function &import = *new Function();
    if (function_known)
        import = functions[func.name];
#else
    Function &import = functions[func.name]; // copy function info from library/runtime to main module
#endif
    if (import.is_declared)return;
    if (func.is_polymorphic) {
        import.is_polymorphic = func.is_polymorphic; //
        import.variants = func.variants;
    } else {
        import.signature = func.signature;
        import.signature.parameters = func.signature.parameters; // even if polymorph?
    }
    import.signature.type_index = -1;
    import.module = func.module; // Preserve module pointer from library function
    import.ffi_library = func.ffi_library; // Preserve FFI library name for native imports
    import.is_ffi = func.is_ffi; // Preserve FFI flag
    import.is_runtime = false; // because here it is an import!
    import.is_import = true; // todo also for polymorph?
    import.is_used = true; // todo also for polymorph?
#if WASM
    if (not function_known)
        functions.add(func.name, import);
#endif
}


// return: done?
// todo return clear enum known, added, ignore ?
bool addLocal(Function &context, String name, Type type, bool is_param) {
    if (not name)
        error("addLocal: empty name");
    if (isKeyword(name))
        error("keyword as local name: "s + name);
    // todo: kotlin style context sensitive symbols!
    if (builtin_constants.has(name))
        return true;
    if (types.has(name))
        // error("type as local name: "s + name);
        return true; // already declared as type, e.g. Point
    if (context.signature.has(name)) {
        return false; // known parameter!
    }
    if (globals.has(name))
        error(name + " already declared as global"s);
    if (isFunction(name, true)) {
        // Allow re-importing FFI functions (they're idempotent)
        if (functions[name].is_ffi) {
            return false; // Already declared, skip adding again
        }
        error(name + " already declared as function"s);
    }
    if (not context.locals.has(name)) {
        int position = context.locals.size();
        context.locals.add(name, Local{.is_param = is_param, .position = position, .name = name, .type = type});
        return true; // added
    } else {
        updateLocal(context, name, type);
        return false; // already there
    }
    //#endif
}
void addWasmArrayType(Type value_type) {
    if (isGeneric(value_type)) {
        addWasmArrayType(value_type.generics.value_type);
        return;
    }
    auto array_type_name = String(typeName(value_type)) + "s"; // int-array -> “ints”
    Node array_type;
    array_type.kind = arrays;
    array_type.type = types[typeName(value_type)]; // todo more robust
    types.add(array_type_name, array_type.clone());
    // link to type index later
}

void updateLocal(Function &context, String name, Type type) {
    //#if DEBUG
    if (type == undefined)return; // no type given, ok
    Local &local = context.locals[name];
    auto type_name = typeName(type);
    auto oldType = local.type;
    if (oldType == none or oldType == unknown_type) {
        local.type = type;
    } else if (oldType != type and type != void_block and type != voids and type != unknown_type) {
        if (use_wasm_arrays) {
            // TODO: CLEANUP
            if (oldType == node) {
                addWasmArrayType(type);
                local.type = type; // make more specific!
            } else if (isGeneric(oldType)) {
                auto kind = oldType.generics.kind;
                auto valueType1 = oldType.generics.value_type;
                if (valueType1 != type) todow(
                    "generic type mismatch "s + typeName(oldType) + " vs " + type_name);
            } else if (oldType == wasmtype_array or oldType == array) {
                addWasmArrayType(type);
                local.type = genericType(array, type);
            }
        } else {
            if (!compatibleTypes(oldType, type)) {
                compatibleTypes(oldType, type);
                error("local "s + name + " in context %s already known "s % context.name + " with type " +
                    typeName(oldType) + ", ignoring new type " + type_name);
            }
        }
    }
    // ok, could be cast'able!
}

bool isGlobal(Node &node, Function &function) {
    if (not node.name.empty()) {
        if (globals.has(node.name))
            return true;
        if (builtin_constants.contains(node.name))
            return true;
    }
    if (node.first().name == "global")
        return true;
    return false;
}


// bad : we don't know which
void use_runtime(const char *function) {
    findLibraryFunction(function, false);
    //    Function &function = functions[function];
    //    if (not function.is_import and not function.is_runtime)
    //        error("can only use import or runtime "s + function);
    //    function.is_used = true;
    //    function.is_import = true;// only in this module, not in original !
}


bool is_native_library(const String & name);

Module &loadModule(String name) {
    if (name == "wasp-runtime.wasm")
        return loadRuntime();
#if WASM and not MY_WASM
    todow("loadModule in WASM: "s + name);
    return *new Module();
#else
    if(is_native_library(name))
        return *loadNativeLibrary(name);
    return read_wasm(name); // we need to read signatures!
#endif
}

