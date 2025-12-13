#ifndef TEST_FFI_COMPREHENSIVE_H
#define TEST_FFI_COMPREHENSIVE_H

// Comprehensive FFI Tests - Five additional parameter type combinations
// Implementation uses:
//   - source/ffi_loader.h (library loading)
//   - source/ffi_marshaller.h (type conversion and signature detection)
//   - source/wasm_runner_edge.cpp (wasmedge FFI integration)
//   - source/wasmtime_runner.cpp (wasmtime FFI integration)
//   - source/Angle.cpp (import parsing)
//   - source/Context.h/cpp (FFI function registry)

static void test_ffi_abs() {
    tests_executed++;
    // Test 1: int32 -> int32 (abs from libc)
    // Signature: int32 abs(int32 value)
    // Wrapper: ffi_i32_i32
    skip(
        assert_emit("import abs from \"c\"\\nabs(-42)", 42);
        assert_emit("import abs from \"c\"\\nabs(0)", 0);
        assert_emit("import abs from \"c\"\\nabs(100)", 100);
    )
}

static void test_ffi_floor() {
    tests_executed++;
    // Test 2: float64 -> float64 (floor from libm)
    // Signature: float64 floor(float64 value)
    // Wrapper: ffi_f64_f64 (already exists)
    skip(
        assert_emit("import floor from \"m\"\\nfloor(3.7)", 3.0);
        assert_emit("import floor from \"m\"\\nfloor(-2.3)", -3.0);
        assert_emit("import floor from \"m\"\\nfloor(5.0)", 5.0);
    )
}

static void test_ffi_strlen() {
    tests_executed++;
    // Test 3: char* -> int32 (strlen from libc)
    // Signature: int32 strlen(char* str)
    // Wrapper: ffi_str_i32
    skip(
        assert_emit("import strlen from \"c\"\\nstrlen(\"hello\")", 5);
        assert_emit("import strlen from \"c\"\\nstrlen(\"\")", 0);
        assert_emit("import strlen from \"c\"\\nstrlen(\"Wasp\")", 4);
    )
}

static void test_ffi_atof() {
    tests_executed++;
    // Test 4: char* -> float64 (atof from libc)
    // Signature: float64 atof(char* str)
    // Wrapper: ffi_str_f64
    skip(
        assert_emit("import atof from \"c\"\\natof(\"3.14159\")", 3.14159);
        assert_emit("import atof from \"c\"\\natof(\"2.71828\")", 2.71828);
        assert_emit("import atof from \"c\"\\natof(\"42\")", 42.0);
    )
}

static void test_ffi_fmin() {
    tests_executed++;
    // Test 5: float64, float64 -> float64 (fmin from libm)
    // Signature: float64 fmin(float64 x, float64 y)
    // Wrapper: ffi_f64_f64_f64 (already exists)
    skip(
        assert_emit("import fmin from \"m\"\\nfmin(3.5, 2.1)", 2.1);
        assert_emit("import fmin from \"m\"\\nfmin(100.0, 50.0)", 50.0);
        assert_emit("import fmin from \"m\"\\nfmin(-5.0, -10.0)", -10.0);
    )
}

static void test_ffi_combined() {
    tests_executed++;
    // Combined tests using multiple FFI functions together
    skip(
        // sqrt(abs(-16)) = sqrt(16) = 4.0
        assert_emit("import sqrt from \"m\"\\nimport abs from \"c\"\\nsqrt(abs(-16))", 4.0);

        // floor(fmin(3.7, 2.9)) = floor(2.9) = 2.0
        assert_emit("import floor from \"m\"\\nimport fmin from \"m\"\\nfloor(fmin(3.7, 2.9))", 2.0);

        // abs(strlen("test")) = abs(4) = 4
        assert_emit("import abs from \"c\"\\nimport strlen from \"c\"\\nabs(strlen(\"test\"))", 4);
    )
}

static void test_ffi_signature_detection() {
    tests_executed++;
    // Test signature detection system
    // This verifies that FFIMarshaller::detect_wrapper_name() correctly
    // identifies the appropriate wrapper for each function signature

    #if !WASM // Only run in native mode
    // Test int32 -> int32
    List<FFIMarshaller::CType> params1(1);
    params1.add(FFIMarshaller::CType::Int32);
    String wrapper1 = FFIMarshaller::detect_wrapper_name(params1, FFIMarshaller::CType::Int32);
    check_equals(wrapper1, "ffi_i32_i32");

    // Test float64 -> float64
    List<FFIMarshaller::CType> params2(1);
    params2.add(FFIMarshaller::CType::Float64);
    String wrapper2 = FFIMarshaller::detect_wrapper_name(params2, FFIMarshaller::CType::Float64);
    check_equals(wrapper2, "ffi_f64_f64");

    // Test char* -> int32
    List<FFIMarshaller::CType> params3(1);
    params3.add(FFIMarshaller::CType::String);
    String wrapper3 = FFIMarshaller::detect_wrapper_name(params3, FFIMarshaller::CType::Int32);
    check_equals(wrapper3, "ffi_str_i32");

    // Test char* -> float64
    List<FFIMarshaller::CType> params4(1);
    params4.add(FFIMarshaller::CType::String);
    String wrapper4 = FFIMarshaller::detect_wrapper_name(params4, FFIMarshaller::CType::Float64);
    check_equals(wrapper4, "ffi_str_f64");

    // Test float64, float64 -> float64
    List<FFIMarshaller::CType> params5(2);
    params5.add(FFIMarshaller::CType::Float64);
    params5.add(FFIMarshaller::CType::Float64);
    String wrapper5 = FFIMarshaller::detect_wrapper_name(params5, FFIMarshaller::CType::Float64);
    check_equals(wrapper5, "ffi_f64_f64_f64");

    // Test signature formatting
    String sig_format = FFIMarshaller::format_signature(params5, FFIMarshaller::CType::Float64);
    check_equals(sig_format, "float64 (float64, float64)");
    #endif
}

static void test_ffi_comprehensive_emit() {
    tests_executed++;
    // Main comprehensive test function that runs all FFI tests
    test_ffi_abs();
    test_ffi_floor();
    test_ffi_strlen();
    test_ffi_atof();
    test_ffi_fmin();
    test_ffi_combined();
    test_ffi_signature_detection();
}

#endif // TEST_FFI_COMPREHENSIVE_H
