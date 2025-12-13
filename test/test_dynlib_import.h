#ifndef TEST_DYNLIB_IMPORT_H
#define TEST_DYNLIB_IMPORT_H

// Test dynamic library imports via FFI
// Implementation is in:
//   - source/ffi_loader.h (library loading)
//   - source/wasm_runner_edge.cpp (wasmedge integration)
//   - source/Angle.cpp (import parsing)
//   - source/Context.h/cpp (FFI function registry)

static void test_dynlib_import_emit() {
    tests_executed++;
    // Test FFI import and usage
    // TODO: Update syntax once parser fully supports "import native libm"
    // For now using: import sqrt from "m"
    skip(
        assert_emit("import sqrt from \"m\"\nsqrt(9.0)", 3.0);
        assert_emit("import sqrt from \"m\"\nimport pow from \"m\"\nsqrt(pow(2.0, 2.0))", 2.0);
    )
}

#endif // TEST_DYNLIB_IMPORT_H
