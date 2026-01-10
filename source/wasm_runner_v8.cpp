// V8 WebAssembly Runner - uses V8's native JavaScript WebAssembly API
// This avoids the need for wasm-c-api which requires V8 internals

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#include <sstream>

#define V8_COMPRESS_POINTERS 1

#include "libplatform/libplatform.h"
#include "v8-context.h"
#include "v8-initialization.h"
#include "v8-isolate.h"
#include "v8-local-handle.h"
#include "v8-primitive.h"
#include "v8-script.h"
#include "v8-array-buffer.h"

typedef unsigned char *bytes;
typedef long long int64;

static bool v8_initialized = false;
static v8::Isolate* global_isolate = nullptr;
static v8::ArrayBuffer::Allocator* array_buffer_allocator = nullptr;
static std::unique_ptr<v8::Platform> platform;

void init_v8() {
    if (v8_initialized) return;

    printf("Initializing V8 engine...\n");

    const char *path = "";
    v8::V8::InitializeICUDefaultLocation(path);
    v8::V8::InitializeExternalStartupData(path);
    platform = v8::platform::NewDefaultPlatform();
    v8::V8::InitializePlatform(platform.get());
    v8::V8::Initialize();

    v8::Isolate::CreateParams create_params;
    array_buffer_allocator = v8::ArrayBuffer::Allocator::NewDefaultAllocator();
    create_params.array_buffer_allocator = array_buffer_allocator;
    global_isolate = v8::Isolate::New(create_params);

    v8_initialized = true;
    printf("V8 initialized.\n");
}

void shutdown_v8() {
    if (!v8_initialized) return;

    global_isolate->Dispose();
    v8::V8::Dispose();
    v8::V8::DisposePlatform();
    delete array_buffer_allocator;

    v8_initialized = false;
    global_isolate = nullptr;
}

// Convert WASM bytes to a JavaScript Uint8Array literal string
static std::string bytes_to_js_array(const unsigned char* data, int size) {
    std::ostringstream oss;
    oss << "new Uint8Array([";
    for (int i = 0; i < size; i++) {
        if (i > 0) oss << ",";
        oss << (int)data[i];
    }
    oss << "])";
    return oss.str();
}

extern "C" int64 run_wasm(bytes data, int size) {
    init_v8();

    v8::Isolate::Scope isolate_scope(global_isolate);
    v8::HandleScope handle_scope(global_isolate);
    v8::Local<v8::Context> context = v8::Context::New(global_isolate);
    v8::Context::Scope context_scope(context);

    // Build JavaScript code to instantiate and run the WASM module
    std::string bytes_array = bytes_to_js_array(data, size);

    // JavaScript code that:
    // 1. Creates a WebAssembly.Module from the bytes
    // 2. Creates an instance with empty imports (or basic imports)
    // 3. Calls main/wasp_main and returns the result
    std::string js_code = R"(
        (function() {
            try {
                let bytes = )" + bytes_array + R"(;
                let module = new WebAssembly.Module(bytes);

                // Get required imports from module
                let importObject = WebAssembly.Module.imports(module);

                // Create basic import object for common imports
                let memory = new WebAssembly.Memory({ initial: 256 });
                let imports = {
                    env: {
                        memory: memory,
                        put: function(x) { return x; },
                        puti: function(x) { return x; },
                        puts: function(ptr) { return 0; },
                        putc: function(c) { return c; },
                        panic: function(msg) { return 0; },
                        log: function(x) { return x; },
                        printi: function(x) { return x; },
                        printf: function(x) { return x; },
                    },
                    wasi_snapshot_preview1: {
                        fd_write: function(fd, iovs, iovs_len, nwritten) { return 0; },
                        fd_read: function(fd, iovs, iovs_len, nread) { return 0; },
                        fd_close: function(fd) { return 0; },
                        fd_seek: function(fd, offset, whence, newoffset) { return 0; },
                        environ_sizes_get: function(count, buf_size) { return 0; },
                        environ_get: function(environ, environ_buf) { return 0; },
                        args_sizes_get: function(argc, argv_buf_size) { return 0; },
                        args_get: function(argv, argv_buf) { return 0; },
                        proc_exit: function(code) { return 0; },
                        clock_time_get: function(id, precision, time) { return 0; },
                        random_get: function(buf, buf_len) { return 0; },
                    }
                };

                // Add missing imports dynamically
                for (let imp of importObject) {
                    if (!imports[imp.module]) {
                        imports[imp.module] = {};
                    }
                    if (!imports[imp.module][imp.name]) {
                        if (imp.kind === 'function') {
                            imports[imp.module][imp.name] = function() { return 0; };
                        } else if (imp.kind === 'memory') {
                            imports[imp.module][imp.name] = memory;
                        } else if (imp.kind === 'global') {
                            imports[imp.module][imp.name] = new WebAssembly.Global({value: 'i32', mutable: true}, 0);
                        } else if (imp.kind === 'table') {
                            imports[imp.module][imp.name] = new WebAssembly.Table({initial: 1, element: 'anyfunc'});
                        }
                    }
                }

                let instance = new WebAssembly.Instance(module, imports);

                // Debug: list exports
                let exportNames = Object.keys(instance.exports);

                // Try to find and call main function
                let result = 0;

                if (instance.exports.wasp_main) {
                    result = instance.exports.wasp_main();
                } else if (instance.exports.main) {
                    result = instance.exports.main();
                } else if (instance.exports._start) {
                    instance.exports._start();
                    result = 0;
                } else {
                    // No main found, return export count as debug info
                    return -(1000 + exportNames.length);
                }

                // If result is BigInt, convert to number
                if (typeof result === 'bigint') {
                    return Number(result);
                }
                // Debug: if not a number, return type info encoded
                if (typeof result !== 'number') {
                    // Return -2000 - type code
                    let typeCode = 0;
                    if (result === undefined) typeCode = 1;
                    else if (result === null) typeCode = 2;
                    else if (typeof result === 'boolean') typeCode = 3;
                    else if (typeof result === 'string') typeCode = 4;
                    else if (typeof result === 'object') typeCode = 5;
                    else if (typeof result === 'function') typeCode = 6;
                    return -(2000 + typeCode);
                }
                return result;
            } catch (e) {
                // Return -3000 for exceptions
                return -3000;
            }
        })()
    )";

    v8::Local<v8::String> source = v8::String::NewFromUtf8(
        global_isolate, js_code.c_str(), v8::NewStringType::kNormal).ToLocalChecked();

    v8::Local<v8::Script> script;
    if (!v8::Script::Compile(context, source).ToLocal(&script)) {
        printf("V8: Failed to compile JavaScript\n");
        return -1;
    }

    v8::TryCatch try_catch(global_isolate);
    v8::Local<v8::Value> result;
    if (!script->Run(context).ToLocal(&result)) {
        if (try_catch.HasCaught()) {
            v8::String::Utf8Value exception(global_isolate, try_catch.Exception());
            printf("V8 Exception: %s\n", *exception);
            v8::Local<v8::Message> message = try_catch.Message();
            if (!message.IsEmpty()) {
                v8::String::Utf8Value msg(global_isolate, message->Get());
                printf("V8 Message: %s\n", *msg);
            }
        }
        printf("V8: Failed to run JavaScript\n");
        return -1;
    }

    // Handle Promise result from async function
    if (result->IsPromise()) {
        v8::Local<v8::Promise> promise = result.As<v8::Promise>();

        // Run microtasks to resolve the promise
        while (promise->State() == v8::Promise::kPending) {
            global_isolate->PerformMicrotaskCheckpoint();
        }

        if (promise->State() == v8::Promise::kFulfilled) {
            v8::Local<v8::Value> value = promise->Result();
            if (value->IsNumber()) {
                return value->IntegerValue(context).FromMaybe(0);
            }
        } else if (promise->State() == v8::Promise::kRejected) {
            v8::Local<v8::Value> reason = promise->Result();
            v8::String::Utf8Value utf8(global_isolate, reason);
            printf("V8: Promise rejected: %s\n", *utf8);
            return -1;
        }
    }

    if (result->IsNumber()) {
        int64 val = result->IntegerValue(context).FromMaybe(0);
        printf("V8 result: %lld\n", val);
        return val;
    }

    // Debug: print what type we got
    v8::String::Utf8Value type_str(global_isolate, result->TypeOf(global_isolate));
    printf("V8 result type: %s\n", *type_str);

    return 0;
}

// Simple test function
int test_v8_simple() {
    init_v8();

    v8::Isolate::Scope isolate_scope(global_isolate);
    v8::HandleScope handle_scope(global_isolate);
    v8::Local<v8::Context> context = v8::Context::New(global_isolate);
    v8::Context::Scope context_scope(context);

    // Simple add(3, 4) WASM module
    const char js_code[] = R"(
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

    v8::Local<v8::String> source = v8::String::NewFromUtf8Literal(global_isolate, js_code);
    v8::Local<v8::Script> script = v8::Script::Compile(context, source).ToLocalChecked();
    v8::Local<v8::Value> result = script->Run(context).ToLocalChecked();

    uint32_t number = result->Uint32Value(context).ToChecked();
    printf("V8 test: 3 + 4 = %u\n", number);

    return number == 7 ? 0 : 1;
}
