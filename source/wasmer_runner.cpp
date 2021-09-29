//
// Created by me on 29.09.21.
//

#include <stdio.h>
#include "wasmer.h"
#include "wasm_runner.h"
#include "wasm_helpers.h"

#define error(msg) error1(msg,__FILE__,__LINE__)

#define own

bool done = 0;
wasm_engine_t *engine;
wasm_store_t *store;

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
			printf("%f", val.of.f32);
			break;
		case WASM_F64:
			printf("%g", val.of.f64);
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
	wasm_val_t val = WASM_I32_VAL(1);// init value or ID?
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

int run_wasm(bytes wasm_bytes, int len) {
	if (!done)init_wasmer();
	wasm_byte_vec_t wasmBytes = {(size_t) len, (char *) wasm_bytes};
	wasm_module_t *module = wasm_module_new(store, &wasmBytes);
	if (!module) error("> Error compiling module!\n");
	wasm_byte_vec_delete(&wasmBytes);
	//	wasm_extern_vec_t imports = WASM_EMPTY_VEC;
//	wasm_extern_vec_t import_object = link_imports();

	wasm_extern_t *externs[] = {link_imports2(), link_global()};
	wasm_extern_vec_t imports = WASM_ARRAY_VEC(externs);
	wasm_instance_t *instance = wasm_instance_new(store, module, &imports, NULL);
	if (!instance) error("> Error instantiating module!\n");
	wasm_extern_vec_t exports;
	own wasm_exporttype_vec_t export_types;
	wasm_module_exports(module, &export_types);
	wasm_instance_exports(instance, &exports);
	if (exports.size == 0) error("> Error accessing exports!\n");
//	printf("Retrieving the `main` function...\n");
	wasm_func_t *sum_func;
	for (int i = 0; i < exports.size; i++) {
		wasm_func_t *exporte = wasm_extern_as_func(exports.data[i]);
		print_name(wasm_exporttype_name(export_types.data[i]));
		print_externtype(wasm_exporttype_type(export_types.data[i]));
//		if(exporte.name=="main")
		if (exporte)
			sum_func = exporte;
	}
	if (sum_func == NULL) error("> Failed to get the `main` function!\n");
//	printf("Calling `sum` function...\n");
//wasm_val_t args_val[2] = {WASM_I32_VAL(3), WASM_I32_VAL(4)};
	wasm_val_t args_val[0];// our main takes no args!
	wasm_val_t results_val[1] = {WASM_INIT_VAL};
	wasm_val_vec_t args = WASM_ARRAY_VEC(args_val);
	wasm_val_vec_t results = WASM_ARRAY_VEC(results_val);

	// wasmer is only good for calling utterly tested code, otherwise it gives ZERO info on what went wrong!
	if (wasm_func_call(sum_func, &args, &results)) error("> Error calling the `main` function!\n");

	int32_t result = results_val[0].of.i32;
	printf("!!!Result: %d\n", result);

	wasm_func_delete(sum_func);
	wasm_module_delete(module);
//	wasm_extern_vec_delete(&exports);// SIG_KILL why?
	wasm_instance_delete(instance);
//	wasm_store_delete(store);
//	wasm_engine_delete(engine);
	return result;
}
