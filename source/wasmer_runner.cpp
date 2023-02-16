//
// Created by me on 29.09.21.
//

#include <stdio.h>
#include <cmath>
#include "wasmer.h"
#include "wasm_runner.h"
#include "wasm_helpers.h"
#include "wasm_reader.h"

#define error(msg) error1(msg,__FILE__,__LINE__)

#define own

bool done = 0;
wasm_engine_t *engine;
wasm_store_t *store;


const wasm_functype_t *funcType(Signature &signature);

typedef wasm_trap_t *(wasm_caller_t)(const wasm_val_vec_t *, wasm_val_vec_t *);

typedef wasm_trap_t *(wasm_wrap)(const wasm_val_vec_t *, wasm_val_vec_t *);
//typedef wasm_trap_t*(wasmtime_wrap)(void *, wasm_caller_t *, const wasm_val_t *, size_t, wasm_val_t *, size_t);

// ⚠️  'fun' needs to be a Locally accessible symbol (via include, ...)
#define wrap_any(fun) static wasm_trap_t *wrap_##fun( void *env,\
wasm_caller_t *caller,\
const wasm_val_t *args,\
size_t nargs,\
wasm_val_t *results,\
size_t nresults\
){ fun();return NULL;}

#define wrap(fun) static wasm_trap_t *wrap_##fun(const wasm_val_vec_t *args, wasm_val_vec_t *results)

// could be unified with wasmtime via getter defines get(0,i32) => args[0].data->of.i32 / args[0].of.i32
wrap(square) {
	int n = args[0].data->of.i32;
	results->data->of.i32 = n * n;
	return NULL;
}


wrap(log) {
    double n = args[0].data->of.float64;
    results[0].data->of.float64 = log(n);
    return NULL;
}


wrap(powd) {
    double n = args[0].data->of.float64;
    double x = args[1].data->of.float64;
    results[0].data->of.float64 = powd(n, x);
    return NULL;
}

wrap(puts) {
	int n = args[0].data->of.i32;
	if (wasm_memory)
		printf("%s\n", &((char *) wasm_memory)[n]);
	else
        printf("puts / printf can't access null wasm_memory at %d (internal error!)", n);
	return NULL;
}

wrap(puti) {
	int i = args[0].data->of.i32;
    printf("%d", i);
	return NULL;
}

wrap(putf) {
	float f = args[0].data->of.float32;
    printf("%f", f);
	return NULL;
}

wrap(putd) {
    float f = args[0].data->of.float64;
    printf("%f", f);
    return NULL;
}

wrap(putc) {// put_char
    int i = args[0].data->of.i32;
    printf("%c", i);
    return NULL;
}

wrap(nop_) {
    return NULL;
}

wrap(atoi) {
    int n = args[0].data->of.i32;
    auto a = (chars) ((char *) wasm_memory) + n;
    int i = parseLong(a);
    results[0].data->of.i32 = i;
    return NULL;
}

wrap(exit) {
    printf("exit, lol");
//	exit(42);
	return NULL;
}

wrap(memset) {
	todo("memset");
	return NULL;
}


wrap(calloc) {
	todo("calloc");
	return NULL;
}

wrap(todo) {
	todo("this function should not be a wasm import, but part of the runtime!!");
	return NULL;
}

wrap(absi) {
//	todo("this function should not be a wasm import, but part of the runtime!!");
	int64 i = args[0].data->of.i32;
	results[0].data->of.i32 = i > 0 ? i : -1;
	return NULL;
}

wrap(absf) {
//	todo("this function should not be a wasm import, but part of the runtime!!");
    double i = args[0].data->of.float32;
    results[0].data->of.float32 = i > 0 ? i : -1;
    return NULL;
}


void test_lambda() {
	print("requestAnimationFrame lambda");
};

//wrap_any(requestAnimationFrame);
wrap_any(test_lambda);

#define wrap_fun(fun) [](void *, wasm_caller_t *, const wasm_val_t *, size_t, wasm_val_t *, size_t)->wasm_trap_t*{fun();return NULL;};

wasm_wrap *link_import(String name) {
	if (name == "memset") return &wrap_todo;// should be provided by wasp!!
	if (name == "calloc") return &wrap_todo;
	if (name == "_Z5abs_ff") return &wrap_absf;// why??
// todo get rid of these again!
	if (name == "_Z7consolev") return &wrap_nop;

	if (name == "__cxa_guard_acquire") return &wrap_nop;// todo!?
	if (name == "__cxa_guard_release") return &wrap_nop;// todo!?
	if (name == "_Z13init_graphicsv") return &wrap_nop;
//	if (name == "_Z21requestAnimationFramev") return wrap_fun(test_lambda);
//	if (name == "_Z21requestAnimationFramev") return  wrap_fun(requestAnimationFrame);

//	if (name == "_Z8typeName7Valtype") return &wrap_nop;// todo!?
//	if (name == "_Z8run_wasmPhi") return &wrap_nop;
//	if (name == "_Z11testCurrentv") return &wrap_nop;// hmmm self test?

	if (name == "fopen") return &wrap_nop;// todo!?
	if (name == "fseek") return &wrap_nop;// todo!?
	if (name == "ftell") return &wrap_nop;// todo!?
	if (name == "fread") return &wrap_nop;// todo!?
	if (name == "system") return &wrap_nop;// danger!

	if (name == "__cxa_begin_catch") return &wrap_nop;
	if (name == "_ZdlPv") return &wrap_nop;// delete


	// todo: merge!
	if (name == "_Z5raisePKc") return &wrap_exit;
	if (name == "_ZSt9terminatev") return &wrap_exit;
	if (name == "__cxa_atexit") return &wrap_exit;

	if (name == "__cxa_demangle") return &wrap_nop;
	if (name == "proc_exit") return &wrap_exit;
	if (name == "panic") return &wrap_exit;
	if (name == "raise") return &wrap_exit;
    if (name == "square") return &wrap_square;
    if (name == "printf") return &wrap_puts;
    if (name == "print") return &wrap_puts;
    if (name == "logs") return &wrap_puts;
    if (name == "logi") return &wrap_puti;
    if (name == "logc") return &wrap_putc;
    if (name == "puti") return &wrap_puti;
    if (name == "puts") return &wrap_puts;
    if (name == "putf") return &wrap_putf;
    if (name == "putd") return &wrap_putd;
    if (name == "log") return &wrap_log;
    if (name == "putc") return &wrap_putc;
//	if (name == "putchar") return &wrap_putc;// todo: remove duplicates!
    if (name == "put_char") return &wrap_putc;// todo: remove duplicates!
//	if (name == "wasp_main") return &hello_callback;
    error("unmapped import "s + name);
    return 0;
}


// Print a Wasm value
void wasm_val_print(wasm_val_t val) {
	switch (val.kind) {
		case WASM_I32:
            printf("%d", val.of.i32);
			break;
		case WASM_I64:
            printf("%lld", val.of.i64);
			break;
		case WASM_F32:
            printf("%f", val.of.float32);
			break;
		case WASM_F64:
            printf("%g", val.of.float64);
			break;
        case WASM_ANYREF:
        case WASM_FUNCREF:
            if (val.of.ref == NULL) printf("null");
            else printf("ref(%p)", val.of.ref);
			break;
	}
}

// A function to be called from Wasm code.
own wasm_trap_t *print_callback(const wasm_val_vec_t *args, wasm_val_vec_t *results) {
    printf("Calling back...\n> ");
    wasm_val_print(args->data[0]);
    printf("\n");
    wasm_val_copy(&results->data[0], &args->data[0]);
	return NULL;
}

// A function to be called from Wasm code.
own wasm_trap_t *callback(const wasm_val_vec_t *args, wasm_val_vec_t *results) {
    assert(args->data[0].kind == WASM_I32);
    printf("> Thread %d running\n", args->data[0].of.i32);
	return NULL;
}

// see tests/wasm-c-api/example/hostref.cc for advanced shit

wasm_extern_t *link_imports2() {
	// Create imports.
//	own wasm_functype_t *print_type = wasm_functype_new_1_1(wasm_valtype_new_i32(), wasm_valtype_new_i32());
//	own wasm_func_t *print_func = wasm_func_new(store, print_type, print_callback);
	own wasm_functype_t *func_type = wasm_functype_new_1_0(wasm_valtype_new_i32());
	own wasm_func_t *func = wasm_func_new(store, func_type, callback);
	wasm_functype_delete(func_type);
	return wasm_func_as_extern(func);
}

wasm_extern_t *link_global() {
    wasm_val_t val = WASM_I32_VAL(1);// value value or ID?
    own wasm_globaltype_t *global_type = wasm_globaltype_new(wasm_valtype_new_i32(), WASM_CONST);
    own wasm_global_t *global = wasm_global_new(store, global_type, &val);
    wasm_globaltype_delete(global_type);
    return wasm_global_as_extern(global);
}

void init_wasmer() {
	engine = wasm_engine_new();
	store = wasm_store_new(engine);
	done = 1;
}

void print_name(const wasm_name_t *name) {
    printf("%.*s : ", (int) name->size, name->data);
}


void print_mutability(wasm_mutability_t mut) {
	switch (mut) {
		case WASM_VAR:
            printf("var");
			break;
		case WASM_CONST:
            printf("const");
			break;
	}
}

void print_limits(const wasm_limits_t *limits) {
    printf("%ud", limits->min);
    if (limits->max < wasm_limits_max_default) printf(" %ud", limits->max);
}

void print_valtype(const wasm_valtype_t *type) {
	switch (wasm_valtype_kind(type)) {
		case WASM_I32:
            printf("i32");
			break;
		case WASM_I64:
            printf("i64");
			break;
		case WASM_F32:
            printf("f32");
			break;
		case WASM_F64:
            printf("f64");
			break;
		case WASM_ANYREF:
            printf("anyref");
			break;
		case WASM_FUNCREF:
            printf("funcref");
			break;
	}
}

void print_valtypes(const wasm_valtype_vec_t *types) {
	bool first = true;
	for (size_t i = 0; i < types->size; ++i) {
		if (first) {
			first = false;
		} else {
            printf(" ");
		}
		print_valtype(types->data[i]);
	}
}

void print_externtype(const wasm_externtype_t *type) {
	switch (wasm_externtype_kind(type)) {
		case WASM_EXTERN_FUNC: {
			const wasm_functype_t *functype =
					wasm_externtype_as_functype_const(type);
            printf("func ");
            print_valtypes(wasm_functype_params(functype));
            printf(" -> ");
            print_valtypes(wasm_functype_results(functype));
		}
			break;
		case WASM_EXTERN_GLOBAL: {
			const wasm_globaltype_t *globaltype =
					wasm_externtype_as_globaltype_const(type);
            printf("global ");
            print_mutability(wasm_globaltype_mutability(globaltype));
            printf(" ");
            print_valtype(wasm_globaltype_content(globaltype));
		}
			break;
		case WASM_EXTERN_TABLE: {
			const wasm_tabletype_t *tabletype =
					wasm_externtype_as_tabletype_const(type);
            printf("table ");
            print_limits(wasm_tabletype_limits(tabletype));
            printf(" ");
            print_valtype(wasm_tabletype_element(tabletype));
		}
			break;
		case WASM_EXTERN_MEMORY: {
			const wasm_memorytype_t *memorytype =
					wasm_externtype_as_memorytype_const(type);
            printf("memory ");
            print_limits(wasm_memorytype_limits(memorytype));
		}
			break;
	}
    printf("\n");
}


void print_frame(wasm_frame_t *frame) {
    printf("..");
//	printf("> %p @ 0x%zx = %u.0x%zx\n",
//	       wasm_frame_instance(frame),
//	       wasm_frame_module_offset(frame),
//	       wasm_frame_func_index(frame),
//	       wasm_frame_func_offset(frame)
//	);
}

void print_trace(wasm_trap_t *trap) {
    printf("Printing trace...\n");
	own wasm_frame_vec_t trace;
	wasm_trap_trace(trap, &trace);
	if (trace.size > 0) {
		for (size_t i = 0; i < trace.size; ++i) {
			print_frame(trace.data[i]);
		}
	}
}


//void linkImports(wasmer_named_extern_t *externs, Module meta){
void linkImports(wasm_extern_t **externs, Module meta) {
	// LINK IMPORTS!
	int i = 0;

	for (String import_name: meta.import_names) {
		Signature &signature = meta.signatures[import_name];
		const wasm_functype_t *func_type = funcType(signature);
		wasm_wrap *wrap = link_import(import_name);
//		const wasm_func_callback_t callback=link_import(import_name);
//		wasm_func_t link*=wasm_func_new(store, funcType(signature), link_import(import_name));
//		wasm_func_t *import = wasm_func_new(store, func_type, wrap);
		wasm_func_t *import = wasm_func_new(store, func_type, wrap);
//		import.k ->kind = WASM_EXTERN_FUNC;
//		import->of.import = link;
		wasm_extern_t *ex = wasm_func_as_extern(import);
		wasmer_named_extern_t *ex2;//=ex;

		externs[i++] = ex;
//		return wasm_func_as_extern(import);
		print(import_name);
//		wasm_func_t link*=wasm_func_new_with_env(store, funcType(signature), link_import(import_name), NULL, NULL);
//		wasm_func_new(store, funcType(signature), link_import(import_name), NULL, NULL, &link);
	}
}

wasm_func_t *findFunction(wasm_extern_vec_t exports, wasm_exporttype_vec_t export_types) {
	for (int i = 0; i < exports.size; i++) {
		wasm_func_t *exporte = wasm_extern_as_func(exports.data[i]);
		print_name(wasm_exporttype_name(export_types.data[i]));
		print_externtype(wasm_exporttype_type(export_types.data[i]));
//		if(exporte.name=="wasp_main") // uh todo!?
		if (exporte)return exporte;
	}
	error("> Failed to get the `main` function!\n");
	return 0;
}


extern "C" int64 run_wasm(bytes wasm_bytes, int len) {
    if (!done)init_wasmer();
    wasm_byte_vec_t wasmBytes = {(size_t) len, (char *) wasm_bytes};
    wasm_module_t *module = wasm_module_new(store, &wasmBytes);
    if (!module) error("> Error compiling module!\n");
    wasm_byte_vec_delete(&wasmBytes);

    Module meta = read_wasm(wasm_bytes, len);// wasm module* sucks so we read it ourselves!

    wasm_extern_vec_t imports = WASM_EMPTY_VEC;
    wasm_extern_t *externs[meta.import_count * 2];
//	wasm_extern_t *externs[] = {link_imports2(), link_global()};
	linkImports(externs, meta);
	wasm_extern_vec_new(&imports, meta.import_count * 2, externs);
	wasm_trap_t *trap = 0;//wasm_trap_new(store, (wasm_message_t*)"Error instantiating module!\n\0");
	wasm_instance_t *instance = wasm_instance_new(store, module, &imports, &trap);
	if (!instance) {
		int i = wasmer_last_error_length();
		char error_buffer[1000];
        wasmer_last_error_message(error_buffer, i);
        printf("%s\n", error_buffer);
//		print_trace(trap);
		error("Error instantiating module!\n");
	}
	wasm_extern_vec_t exports;
	own wasm_exporttype_vec_t export_types;
	wasm_module_exports(module, &export_types);
	wasm_instance_exports(instance, &exports);
	if (exports.size == 0) error("> Error accessing exports!\n");

    printf("Retrieving the `main` function...\n");
	wasm_func_t *sum_func = findFunction(exports, export_types);
	if (sum_func == NULL) error("> Failed to get the `main` function!\n");
//	printf("Calling `sum` function...\n");
//wasm_val_t args_val[2] = {WASM_I32_VAL(3), WASM_I32_VAL(4)};
	wasm_val_t args_val[0];// our main takes no args!
	wasm_val_t results_val[1] = {WASM_INIT_VAL};
	wasm_val_vec_t args = WASM_ARRAY_VEC(args_val);
	wasm_val_vec_t results = WASM_ARRAY_VEC(results_val);

    // wasmer is only good for calling utterly tested code, otherwise it gives ZERO info on what went wrong!
    if (wasm_func_call(sum_func, &args, &results)) error("> Error calling the `main` function!\n");

//	int nresults=1;//results_val
//	int64_t result = results_val->of.i64;
//	if (nresults > 1) {
//		wasm_val_t results2 = *(&results + 1);
//		Type type = results2->of.i32;
//		if (type >= undefined and type <= arrays)
//			printf("TYPE: %s\n", (chars) type);
//		else printf("Unknown type %d\n", (int) type);
//	}

    int32_t result = results_val->of.i32;
    printf("Wasmer Result: %d\n", result);

    wasm_func_delete(sum_func);
    wasm_module_delete(module);
//	wasm_extern_vec_delete(&exports);// SIG_KILL why?
    wasm_instance_delete(instance);
//	wasm_store_delete(store);
//	wasm_engine_delete(engine);
	return result;
}

// same in wasmtime_runner.cpp … so far duplication ok
const wasm_functype_t *funcType(Signature &signature) {
	wasm_valtype_t *i = wasm_valtype_new(WASM_I32);
	wasm_valtype_t *I = wasm_valtype_new(WASM_I64);
	wasm_valtype_t *f = wasm_valtype_new(WASM_F32);
	wasm_valtype_t *f1 = wasm_valtype_new(WASM_F32);
	wasm_valtype_t *F = wasm_valtype_new(WASM_F64);
    int param_count = signature.parameter_types.size();
	// todo multi-value
	auto returnType = signature.return_types.last(none);
	if (param_count == 0) {
		switch (returnType) {
			case none:
			case voids:
				return wasm_functype_new_0_0();
			case int32:
				return wasm_functype_new_0_1(i);
			default:
				break;
		}
	}
	if (param_count == 1) {
        switch (signature.parameter_types[0]) {
            case charp:
            case float32:
                switch (returnType) {
                    case none:
                    case voids:
                        return wasm_functype_new_1_0(f);
                    case float32:
                        return wasm_functype_new_1_1(f, f1);
                    default:
                        break;
                }
            case float64:
                switch (returnType) {
                    case none:
                    case voids:
                        return wasm_functype_new_1_0(F);
                    case float64:
                        return wasm_functype_new_1_1(F, F);
                    default:
                        break;
                }
            case int32:
				switch (returnType) {
					case none:
					case voids:
						return wasm_functype_new_1_0(i);
					case int32:
						return wasm_functype_new_1_1(i, i);
					default:
						break;
				}
			default:
				break;
		}
	}
	if (param_count == 2) {
		switch (returnType) {
            case int32:
                return wasm_functype_new_2_1(i, i, i); // printf(i32,i32)i32
            case i64:
                return wasm_functype_new_2_1(i, i, I);
            case float32:
                return wasm_functype_new_2_1(f, f, f);
            case float64:
                return wasm_functype_new_2_1(F, F, F); // powd(f64,f64)f64
            default:
                break;
        }
	}
	// todo unhack!
	if (param_count == 3) return wasm_functype_new_3_1(i, i, i, i); //(char*,char*,i32,)i32 ;)
	if (param_count == 4) return wasm_functype_new_4_1(i, i, i, i, i); //(char*,char*,i32,)i32 ;)
	print(signature.format());
	error("missing signature mapping"s + signature.format());
	return 0;
}
