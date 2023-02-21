//#include <v8>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>

#include "wasm.h"


#define V8_COMPRESS_POINTERS 1

#include "libplatform/libplatform.h"
#include "v8-context.h"
#include "v8-initialization.h"
#include "v8-isolate.h"
#include "v8-local-handle.h"
#include "v8-primitive.h"
#include "v8-script.h"

#define own
typedef unsigned char *bytes;
typedef long long int64;

int test_V8_cpp_Javascript() {
	// Initialize V8.
	const char *path = "";
	v8::V8::InitializeICUDefaultLocation(path);
	v8::V8::InitializeExternalStartupData(path);
	v8::Platform *platform = v8::platform::NewDefaultPlatform().release();
	v8::V8::InitializePlatform(platform);
	v8::V8::Initialize();

    // Create a new Isolate and make it the current one.
    v8::Isolate::CreateParams create_params;
	create_params.array_buffer_allocator =
			v8::ArrayBuffer::Allocator::NewDefaultAllocator();
	v8::Isolate *isolate = v8::Isolate::New(create_params);
	{
		v8::Isolate::Scope isolate_scope(isolate);

		// Create a stack-allocated handle scope.
		v8::HandleScope handle_scope(isolate);

		// Create a new context.
		v8::Local<v8::Context> context = v8::Context::New(isolate);

		// Enter the context for compiling and running the hello world script.
		v8::Context::Scope context_scope(context);

		{
			// Create a string containing the JavaScript source code.
            v8::Local<v8::String> source = v8::String::NewFromUtf8Literal(isolate, "'Hello' + ', World!'");

			// Compile the source code.
            v8::Local<v8::Script> script = v8::Script::Compile(context, source).ToLocalChecked();

			// Run the script to get the result.
			v8::Local<v8::Value> result = script->Run(context).ToLocalChecked();

			// Convert the result to an UTF8 string and print it.
			v8::String::Utf8Value utf8(isolate, result);
			printf("%s\n", *utf8);
		}

        {
            // WTF!!!!??

            // Use the JavaScript API to generate a WebAssembly module.
            //
            // |bytes| contains the binary format for the following module:
            //
            //     (func (export "add") (param i32 i32) (result i32)
            //       get_local 0
            //       get_local 1
            //       i32.add)
            //
			const char csource[] = R"(
        let bytes = new Uint8Array([
          0x00, 0x61, 0x73, 0x6d, 0x01, 0x00, 0x00, 0x00, 0x01, 0x07, 0x01,
          0x60, 0x02, 0x7f, 0x7f, 0x01, 0x7f, 0x03, 0x02, 0x01, 0x00, 0x07,
          0x07, 0x01, 0x03, 0x61, 0x64, 0x64, 0x00, 0x00, 0x0a, 0x09, 0x01,
          0x07, 0x00, 0x20, 0x00, 0x20, 0x01, 0x6a, 0x0b
        ]);
        let module = new WebAssembly.Module(bytes);
        let instance = new WebAssembly.Instance(module);
        instance.exports.add(3, 4);
			)";

			// Create a string containing the JavaScript source code.
            v8::Local<v8::String> source = v8::String::NewFromUtf8Literal(isolate, csource);

			// Compile the source code.
            v8::Local<v8::Script> script = v8::Script::Compile(context, source).ToLocalChecked();

			// Run the script to get the result.
			v8::Local<v8::Value> result = script->Run(context).ToLocalChecked();

			// Convert the result to a uint32 and print it.
			uint32_t number = result->Uint32Value(context).ToChecked();
            printf("3 + 4 = %u\n", number);
		}
	}

    // Dispose the isolate and tear down V8.
    isolate->Dispose();
    v8::V8::Dispose();
//	v8::V8::ShutdownPlatform();
    delete create_params.array_buffer_allocator;
    return 0;
}

//#define own
//
// Print a Wasm value
void wasm_val_print(wasm_val_t val) {
    switch (val.kind) {
        case WASM_I32: {
            printf("%" PRIu32, val.of.i32);
        }
            break;
        case WASM_I64: {
            printf("%" PRIu64, val.of.i64);
        }
            break;
        case WASM_F32: {
            printf("%f", val.of.f32);
        }
            break;
        case WASM_F64: {
            printf("%g", val.of.f64);
        }
            break;
        case WASM_ANYREF:
        case WASM_FUNCREF: {
            if (val.of.ref == NULL) {
                printf("null");
            } else {
                printf("ref(%p)", val.of.ref);
            }
        }
            break;
    }
}

// A function to be called from Wasm code.
own wasm_trap_t *print_callback(
        const wasm_val_vec_t *args, wasm_val_vec_t *results
) {
    printf("Calling back...\n> ");
    wasm_val_print(args->data[0]);
    printf("\n");

    wasm_val_copy(&results->data[0], &args->data[0]);
    return NULL;
}


// A function closure.
own wasm_trap_t *closure_callback(
        void *env, const wasm_val_vec_t *args, wasm_val_vec_t *results
) {
    int i = *(int *) env;
    printf("Calling back closure...\n");
    printf("> %d\n", i);

    results->data[0].kind = WASM_I32;
    results->data[0].of.i32 = (int32_t) i;
    return NULL;
}


int test_v8() {
    // Initialize.
    printf("Initializing...\n");
    wasm_engine_t *engine = wasm_engine_new();
    wasm_store_t *store = wasm_store_new(engine);

    // Load binary.
    printf("Loading binary...\n");
    FILE *file = fopen("samples/callback.wasm", "rb");
    if (!file) {
        printf("> Error loading module!\n");
        return 1;
    }
    fseek(file, 0L, SEEK_END);
    size_t file_size = ftell(file);
    fseek(file, 0L, SEEK_SET);
    wasm_byte_vec_t binary;
    wasm_byte_vec_new_uninitialized(&binary, file_size);
    if (fread(binary.data, file_size, 1, file) != 1) {
        printf("> Error loading module!\n");
        return 1;
    }
    fclose(file);

    // Compile.
    printf("Compiling module...\n");
    own wasm_module_t *module = wasm_module_new(store, &binary);
    if (!module) {
        printf("> Error compiling module!\n");
        return 1;
    }

    wasm_byte_vec_delete(&binary);

    // Create external print functions.
//    printf("Creating callback...\n");
//    own wasm_functype_t *print_type = wasm_functype_new_1_1(wasm_valtype_new_i32(), wasm_valtype_new_i32());
//    own wasm_func_t *print_func = wasm_func_new(store, print_type, print_callback);
//
//    int i = 42;
//    own wasm_functype_t *closure_type = wasm_functype_new_0_1(wasm_valtype_new_i32());
//    own wasm_func_t *closure_func = wasm_func_new_with_env(store, closure_type, closure_callback, &i, NULL);
//
//    wasm_functype_delete(print_type);
//    wasm_functype_delete(closure_type);

    // Instantiate.
    printf("Instantiating module...\n");
    wasm_extern_t *externs[] = {
//            wasm_func_as_extern(print_func), wasm_func_as_extern(closure_func)
    };
    wasm_extern_vec_t imports = WASM_ARRAY_VEC(externs);
    own wasm_instance_t *instance =
            wasm_instance_new(store, module, &imports, NULL);
    if (!instance) {
        printf("> Error instantiating module!\n");
        return 1;
    }
//
//    wasm_func_delete(print_func);
//    wasm_func_delete(closure_func);

    // Extract export.
    printf("Extracting export...\n");
    own wasm_extern_vec_t exports;
    wasm_instance_exports(instance, &exports);
    if (exports.size == 0) {
        printf("> Error accessing exports!\n");
        return 1;
    }
    const wasm_func_t *run_func = wasm_extern_as_func(exports.data[0]);
    if (run_func == NULL) {
        printf("> Error accessing export!\n");
        return 1;
    }

    wasm_module_delete(module);
    wasm_instance_delete(instance);

    // Call.
    printf("Calling export...\n");
    wasm_val_t as[2] = {WASM_I32_VAL(3), WASM_I32_VAL(4)};
    wasm_val_t rs[1] = {WASM_INIT_VAL};
    wasm_val_vec_t args = WASM_ARRAY_VEC(as);
    wasm_val_vec_t results = WASM_ARRAY_VEC(rs);
    if (wasm_func_call(run_func, &args, &results)) {
        printf("> Error calling function!\n");
        return 1;
    }

    wasm_extern_vec_delete(&exports);

    // Print result.
    printf("Printing result...\n");
    printf("> %u\n", rs[0].of.i32);

    // Shut down.
    printf("Shutting down...\n");
    wasm_store_delete(store);
    wasm_engine_delete(engine);

    // All done.
    printf("Done.\n");
    return 0;
}

namespace v8 {

    namespace api_internal {
        // Called when ToChecked is called on an empty Maybe.
        V8_EXPORT void FromJustIsNothing() {} //hack
        V8_EXPORT void ToLocalEmpty() {} //hack
    }  // namespace api_internal
}

//#include "wasm.hh"

// A function to be called from Wasm code.
//auto hello_callback(const wasm::Val args[], wasm::Val results[]) -> wasm::own<wasm::Trap*> {
//	return 0;
//}
//namespace v8{
//
//int v8_cpp_api(size_t size,byte_t* data){
//
//	auto engine = wasm::Engine::make();
//	wasm::Store *store = wasm::Store::make(engine);
//	wasm::vec<byte_t> binary((size_t)size,(byte_t*)data);
//	auto module = wasm::Module::make(store, binary);
//	//	const vec<Extern*>& imports;
//	//	wasm_extern_vec_t imports = WASM_EMPTY_VEC;
//
//	// Create external print functions.
//	auto hello_type = wasm::FuncType::make(wasm::vec<wasm::own<wasm::ValType>>::make(), wasm::vec<wasm::own<wasm::ValType>>::make());
//	wasm::Func::callback cb = &hello_callback;
//	auto hello_func = wasm::Func::make(store, hello_type, cb);
//
//	// Instantiate.
//	wasm::Extern* imports[] = {hello_func};
//
//
//	auto mod = module;
//	auto imports1 = mod->imports();
//	wasm::own<wasm::Trap> trap;
//	auto instance = wasm::Instance::make(store, mod, NULL, &trap);
//	auto exports = instance->exports();
//	wasm::vec<wasm::Val> args(1);
//	wasm::vec<wasm::Val> returns(1);
//	auto ok = exports[0]->func()->call(args,returns);
//	printf("RESULT %d", returns[0].i32());
//}
//}


bool done = 0;
wasm_engine_t *engine;
wasm_store_t *store;
//wasm_context_t *context;

void init_wasm() {
    engine = wasm_engine_new();
    assert(engine != NULL);
	store = wasm_store_new(engine);
	assert(store != NULL);
	done = 1;
}


#define own


void print_frame(wasm_frame_t *frame) {
    printf("> %p @ 0x%zx = %d.0x%zx\n",
           wasm_frame_instance(frame),
           wasm_frame_module_offset(frame),
           wasm_frame_func_index(frame),
           wasm_frame_func_offset(frame)
    );
}


extern "C" int64 run_wasm(bytes data, int size) {

//    test_V8_cpp_Javascript();
//    v8_cpp_api();
    test_v8();
//    return 3;

    if (!done)init_wasm();
    wasm_byte_vec_t binary{(size_t) size, (wasm_byte_t *) data};
    // Compile.
    printf("Compiling module...\n");
    own wasm_module_t *module = wasm_module_new(store, &binary);
    if (!module) printf("> Error compiling module!\n");
    wasm_byte_vec_delete(&binary);

    // Instantiate.
    printf("Instantiating module...\n");
    wasm_extern_vec_t imports = WASM_EMPTY_VEC;

    //	wasm_trap_t x{};// incomplete type
//	wasm_extern_t *externs[meta.import_count * 2];
//	wasm_extern_t *externs[] = {link_imports2(), link_global()};
//	linkImports(externs, meta);

    own wasm_trap_t **traps = (wasm_trap_t **) malloc(1000);
    traps[0] = (wasm_trap_t *) malloc(1000);
    own wasm_trap_t *trap = traps[0];
    own wasm_instance_t *instance = wasm_instance_new(store, module, &imports, &trap);

    if (!instance) printf("> Error instantiating module, expected trap!\n");


    wasm_module_delete(module);

    // Print result.
    printf("Printing message...\n");
    own wasm_name_t message;
    wasm_trap_message(trap, &message);
    printf("> %s\n", message.data);

    printf("Printing origin...\n");
	own wasm_frame_t *frame = wasm_trap_origin(trap);
	if (frame) {
		print_frame(frame);
		wasm_frame_delete(frame);
	} else {
        printf("> Empty origin.\n");
	}

    printf("Printing trace...\n");
	own wasm_frame_vec_t trace;
	wasm_trap_trace(trap, &trace);
	if (trace.size > 0) {
		for (size_t i = 0; i < trace.size; ++i) {
			print_frame(trace.data[i]);
		}
	} else {
        printf("> Empty trace.\n");
	}

	wasm_frame_vec_delete(&trace);
	wasm_trap_delete(trap);
	wasm_name_delete(&message);

	// Shut down.
    printf("Shutting down...\n");
    wasm_store_delete(store);
	wasm_engine_delete(engine);

	// All done.
    printf("Done.\n");
	return 0;
}

