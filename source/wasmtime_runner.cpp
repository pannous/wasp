#include "wasmtime.h"
#include "wasm_reader.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <wasm.h>
#include <wasmtime.h>
#include "Util.h"
#include <math.h>

static void exit_with_error(const char *message, wasmtime_error_t *error, wasm_trap_t *trap);

const wasm_functype_t *funcType(Signature &signature);

#define wrap(fun) static wasm_trap_t *wrap_##fun( void *env,\
wasmtime_caller_t *caller,\
const wasmtime_val_t *args,\
size_t nargs,\
wasmtime_val_t *results,\
size_t nresults\
)

typedef wasm_trap_t *(wasm_wrap)(void *, wasmtime_caller_t *, const wasmtime_val_t *, size_t, wasmtime_val_t *, size_t);
//typedef wasm_trap_t *(wasm_wrap)(void *env, wasmtime_caller_t *caller, const wasmtime_val_t *args, size_t nargs, wasmtime_val_t *results, size_t nresults);


static wasm_trap_t *hello_callback(
		void *env,
		wasmtime_caller_t *caller,
		const wasmtime_val_t *args,
		size_t nargs,
		wasmtime_val_t *results,
		size_t nresults
) {
	printf("Calling back...\n");
	printf("> Hello World!\n");
//	results=wasm_valtype_new_i32();
	int n = args[0].of.i32;
	results[0].of.i32 = n * n;
//	results[0].kind.
	return NULL;
}


wrap(square) {
	int n = args[0].of.i32;
	results[0].of.i32 = n * n;
	return NULL;
}

wrap(powd) {
	double n = args[0].of.f64;
	double x = args[1].of.f64;
	results[0].of.f64 = powd(n, x);
	return NULL;
}

wrap(powf) {
	float n = args[0].of.f32;
	float x = args[1].of.f32;
	results[0].of.f32 = pow(n, x);
	return NULL;
}

wrap(powi) {
	int n = args[0].of.i32;
	int x = args[1].of.i32;
	results[0].of.i32 = powi(n, x);
	return NULL;
}

wrap(puts) {
	int n = args[0].of.i32;
	if (wasm_memory)
		printf("%s", &((char *) wasm_memory)[n]);
	return NULL;
}

wrap(puti) {
	int i = args[0].of.i32;
	printf("%d", i);
	return NULL;
}

wrap(putf) {
	float f = args[0].of.f32;
	printf("%f", f);
	return NULL;
}

wrap(putd) {
	float f = args[0].of.f64;
	printf("%f", f);
	return NULL;
}

wrap(put_char) {
	int i = args[0].of.i32;
	printf("%c", i);
	return NULL;
}

wrap(nop) {
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


wrap(todo) {
	todo("this function should not be a wasm import, but part of the runtime!!");
	return NULL;
}


wasm_wrap *link_import(String name) {

	if (name == "__cxa_guard_acquire") return &wrap_nop;// todo!?
	if (name == "__cxa_guard_release") return &wrap_nop;// todo!?
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

	if (name == "_Z3powdd") return &wrap_powd;
	if (name == "pow") return &wrap_powd;
	if (name == "powd") return &wrap_powd;
	if (name == "powf") return &wrap_powf;
	if (name == "powi") return &wrap_powi;

	if (name == "_Z5raisePKc") return &wrap_exit;
	if (name == "_ZSt9terminatev") return &wrap_exit;
	if (name == "proc_exit") return &wrap_exit;
	if (name == "panic") return &wrap_exit;
	if (name == "raise") return &wrap_exit;
	if (name == "square") return &wrap_square;
	if (name == "memset") return &wrap_memset;

	if (name == "printf") return &wrap_puts;
	if (name == "puts") return &wrap_puts;
	if (name == "puti") return &wrap_puti;
	if (name == "putf") return &wrap_putf;
	if (name == "putd") return &wrap_putd;
	if (name == "putc") return &wrap_put_char;
	if (name == "putchar") return &wrap_put_char;// todo: remove duplicates!
	if (name == "put_char") return &wrap_put_char;// todo: remove duplicates!
	if (name == "main") return &hello_callback;
	error("unmapped import "s + name);
	return 0;
}

static void exit_with_error(const char *message, wasmtime_error_t *error, wasm_trap_t *trap) {
//	error(message);
	fprintf(stderr, "error: %s\n", message);
	wasm_byte_vec_t error_message;
	if (error != NULL) {
		wasmtime_error_message(error, &error_message);
		wasmtime_error_delete(error);
	} else {
		wasm_trap_message(trap, &error_message);
		wasm_trap_delete(trap);
	}
	fprintf(stderr, "%.*s\n", (int) error_message.size, error_message.data);
	wasm_byte_vec_delete(&error_message);
	exit(1);
}

bool done = 0;
wasm_engine_t *engine;
wasmtime_store_t *store;
wasmtime_context_t *context;

void init_wasmtime() {
	engine = wasm_engine_new();
	assert(engine != NULL);
	store = wasmtime_store_new(engine, NULL, NULL);
	assert(store != NULL);
	context = wasmtime_store_context(store);
	done = 1;
//	void free_wasmtime(){
	//	wasmtime_store_delete(store);
	//	wasm_engine_delete(engine);
}

int run_wasm(unsigned char *data, int size) {
	if (!done)init_wasmtime();
	wasmtime_error_t *error;
	wasmtime_module_t *module = NULL;

	error = wasmtime_module_new(engine, (uint8_t *) data, size, &module);
	if (error != NULL)exit_with_error("failed to compile module", error, NULL);

	wasm_trap_t *trap = NULL;
	wasmtime_instance_t instance;

	Module meta = read_wasm(data, size);// wasmtime module* sucks so we read it ourselves!
	wasmtime_extern_t imports[meta.import_count * 2];
	int i = 0;
	// LINK IMPORTS!
	for (String import_name:meta.import_names) {
		print(import_name);
		wasmtime_extern_t import;
		wasmtime_func_t link;
		Signature &signature = meta.signatures[import_name];
		wasmtime_func_new(context, funcType(signature), link_import(import_name), NULL, NULL, &link);
		import.kind = WASMTIME_EXTERN_FUNC;
		import.of.func = link;
		imports[i++] = import;
	}

	error = wasmtime_instance_new(context, module, imports, meta.import_count, &instance, &trap);
	if (error != NULL || trap != NULL) exit_with_error("failed to instantiate", error, trap);

	wasmtime_extern_t run;
	bool ok = wasmtime_instance_export_get(context, &instance, "main", 4, &run);
	assert(ok);
	assert(run.kind == WASMTIME_EXTERN_FUNC);
	wasmtime_val_t results;
	error = wasmtime_func_call(context, &run.of.func, NULL, 0, &results, 1, &trap);
	if (error != NULL || trap != NULL)exit_with_error("failed to call function", error, trap);
	int32_t result = results.of.i32;
	printf("RESULT: %d\n", result);

	wasmtime_module_delete(module);
	return result;
}

const wasm_functype_t *funcType(Signature &signature) {
	wasm_valtype_t *i = wasm_valtype_new(WASM_I32);
	wasm_valtype_t *I = wasm_valtype_new(WASM_I64);
	wasm_valtype_t *f = wasm_valtype_new(WASM_F32);
	wasm_valtype_t *F = wasm_valtype_new(WASM_F64);
	int param_count = signature.types.size();
	if (param_count == 0) {
		switch (signature.return_type) {
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
		switch (signature.types[0]) {
			case charp:
			case f32:
				return wasm_functype_new_1_0(f);
			case f64:
				switch (signature.return_type) {
					case none:
					case voids:
						return wasm_functype_new_1_0(F);
					case f64:
						return wasm_functype_new_1_1(F, F);
					default:
						break;
				}
			case int32:
				switch (signature.return_type) {
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
		switch (signature.return_type) {
			case int32:
				return wasm_functype_new_2_1(i, i, i); // printf(i32,i32)i32
			case int64:
				return wasm_functype_new_2_1(i, i, I);
			case float32:
				return wasm_functype_new_2_1(f, f, f);
			case float64:
				return wasm_functype_new_2_1(F, F, F); // powd(f64,f64)f64
			default:
				break;
		}
	}
	if (param_count == 3) return wasm_functype_new_3_1(i, i, i, i); //(char*,char*,i32,)i32 ;)
	if (param_count == 4) return wasm_functype_new_4_1(i, i, i, i, i); //(char*,char*,i32,)i32 ;)
	print(signature.format());
	error("missing signature mapping"s + signature.format());
	return 0;
}
