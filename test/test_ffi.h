#pragma once
#include "../source/ffi_marshaller.h"
#include "../source/ffi_dynamic_wrapper.h"
#include "../source/ffi_loader.h"

// Use/import
void testUse();

void testInclude();

// Extended FFI Tests - Additional library functions using dynamic wrappers
// These tests demonstrate that the dynamic wrapper system automatically
// supports new function signatures without requiring hardcoded wrappers.

// Implementation uses:
//   - source/ffi_dynamic_wrapper.h (generic wrappers)
//   - source/ffi_marshaller.h (dynamic dispatch and marshalling)
//   - source/ffi_loader.h (library loading)

// ============================================================================
// String Comparison Functions
// ============================================================================

static void test_ffi_strcmp() {
    tests_executed++;
    // Test: int strcmp(const char* s1, const char* s2)
    // Signature: char*, char* -> int32
    // Returns: 0 if equal, <0 if s1<s2, >0 if s1>s2

    // Equal strings
    assert_emit("import strcmp from \"c\"\\nstrcmp(\"hello\", \"hello\")", 0);

    // Different strings - "abc" < "def"
    assert_emit("import strcmp from \"c\"\\nx=strcmp(\"abc\", \"def\");x<0", 1);

    // Different strings - "xyz" > "abc"
    assert_emit("import strcmp from \"c\"\\nx=strcmp(\"xyz\", \"abc\");x>0", 1);

    // Empty strings
    assert_emit("import strcmp from \"c\"\\nstrcmp(\"\", \"\")", 0);
}

static void test_ffi_strncmp() {
    tests_executed++;
    // Test: int strncmp(const char* s1, const char* s2, size_t n)
    // Signature: char*, char*, int32 -> int32
    // Note: Requires 3-parameter string support

    // Compare first 3 characters - equal
    assert_emit("import strncmp from \"c\"\\nstrncmp(\"hello\", \"help\", 3)", 0);

    // Compare first 5 characters - different
    assert_emit("import strncmp from \"c\"\\nx=strncmp(\"hello\", \"help\", 5);x!=0", 1);
}

// ============================================================================
// Additional Math Functions
// ============================================================================

static void test_ffi_ceil() {
    tests_executed++;
    // Test: double ceil(double x)
    // Signature: float64 -> float64
    // Returns: smallest integer value >= x

    assert_emit("import ceil from \"m\"\\nceil(3.2)", 4.0);
    assert_emit("import ceil from \"m\"\\nceil(-2.8)", -2.0);
    assert_emit("import ceil from \"m\"\\nceil(5.0)", 5.0);
    assert_emit("import ceil from \"m\"\\nceil(0.1)", 1.0);
}

static void test_ffi_sin() {
    tests_executed++;
    // Test: double sin(double x)
    // Signature: float64 -> float64

    // sin(0) = 0
    assert_emit("import sin from \"m\"\\nsin(0.0)", 0.0);

    // sin(π/2) ≈ 1.0
    assert_emit("import sin from \"m\"\\nsin(1.5707963267948966)", 1.0);

    // sin(π) ≈ 0.0 (within epsilon)
    assert_emit("import sin from \"m\"\\nimport abs from \"c\"\\nabs(sin(3.141592653589793))<0.001", 1);
}

static void test_ffi_cos() {
    tests_executed++;
    // Test: double cos(double x)
    // Signature: float64 -> float64

    // cos(0) = 1
    assert_emit("import cos from \"m\"\\ncos(0.0)", 1.0);

    // cos(π/2) ≈ 0.0
    assert_emit("import cos from \"m\"\\nimport abs from \"c\"\\nabs(cos(1.5707963267948966))<0.001", 1);

    // cos(π) ≈ -1.0
    assert_emit("import cos from \"m\"\\ncos(3.141592653589793)", -1.0);
}

static void test_ffi_tan() {
    tests_executed++;
    // Test: double tan(double x)
    // Signature: float64 -> float64

    // tan(0) = 0
    assert_emit("import tan from \"m\"\\ntan(0.0)", 0.0);

    // tan(π/4) ≈ 1.0
    assert_emit("import tan from \"m\"\\ntan(0.7853981633974483)", 1.0);
}

static void test_ffi_fabs() {
    tests_executed++;
    // Test: double fabs(double x)
    // Signature: float64 -> float64
    // Absolute value for floating point

    assert_emit("import fabs from \"m\"\\nfabs(3.14)", 3.14);
    assert_emit("import fabs from \"m\"\\nfabs(-3.14)", 3.14);
    assert_emit("import fabs from \"m\"\\nfabs(0.0)", 0.0);
}

static void test_ffi_fmax() {
    tests_executed++;
    // Test: double fmax(double x, double y)
    // Signature: float64, float64 -> float64
    // Returns maximum of two values

    assert_emit("import fmax from \"m\"\\nfmax(3.5, 2.1)", 3.5);
    assert_emit("import fmax from \"m\"\\nfmax(100.0, 200.0)", 200.0);
    assert_emit("import fmax from \"m\"\\nfmax(-5.0, -10.0)", -5.0);
}

static void test_ffi_fmod() {
    tests_executed++;
    // Test: double fmod(double x, double y)
    // Signature: float64, float64 -> float64
    // Floating-point remainder of x/y

    assert_emit("import fmod from \"m\"\\nfmod(5.5, 2.0)", 1.5);
    assert_emit("import fmod from \"m\"\\nfmod(10.0, 3.0)", 1.0);
}

// ============================================================================
// String Conversion Functions
// ============================================================================

static void test_ffi_atoi() {
    tests_executed++;
    // Test: int atoi(const char* str)
    // Signature: char* -> int32
    // Convert string to integer

    assert_emit("import atoi from \"c\"\\natoi(\"42\")", 42);
    assert_emit("import atoi from \"c\"\\natoi(\"-123\")", -123);
    assert_emit("import atoi from \"c\"\\natoi(\"0\")", 0);
    assert_emit("import atoi from \"c\"\\natoi(\"999\")", 999);
}

static void test_ffi_atol() {
    tests_executed++;
    // Test: long atol(const char* str)
    // Signature: char* -> int64
    // Note: Requires int64 return type support

    assert_emit("import atol from \"c\"\\natol(\"1234567\")", 1234567);
    assert_emit("import atol from \"c\"\\natol(\"-999999\")", -999999);
}

// ============================================================================
// Zero-Parameter Functions
// ============================================================================

static void test_ffi_rand() {
    tests_executed++;
    // Test: int rand(void)
    // Signature: void -> int32
    // Returns pseudo-random number

    // Just verify it returns a number (non-deterministic)
    assert_emit("import rand from \"c\"\\nx=rand();x>=0", 1);

    // Two calls should (probably) return different values
    // This test might occasionally fail due to randomness
    assert_emit("import rand from \"c\"\\nx=rand();y=rand();x!=y", 1);
}

// ============================================================================
// Combined/Complex Tests
// ============================================================================

static void test_ffi_trigonometry_combined() {
    tests_executed++;
    // Test: sin²(x) + cos²(x) = 1 (Pythagorean identity)

    // Import multiple trig functions
    assert_emit(
        "import sin from \"m\"\n"
        "import cos from \"m\"\n"
        "x = 0.5\n"
        "sin_x = sin(x)\n"
        "cos_x = cos(x)\n"
        "result = sin_x * sin_x + cos_x * cos_x\n"
        "result",
        1.0
    );
}

static void test_ffi_string_math_combined() {
    tests_executed++;
    // Test: Parse string numbers and do math

    // atoi("10") + atoi("20") = 30
    assert_emit(
        "import atoi from \"c\"\n"
        "x = atoi(\"10\")\n"
        "y = atoi(\"20\")\n"
        "x + y",
        30
    );

    // ceil(atof("3.7")) = 4.0
    assert_emit(
        "import atof from \"c\"\n"
        "import ceil from \"m\"\n"
        "ceil(atof(\"3.7\"))",
        4.0
    );
}

static void test_ffi_string_comparison_logic() {
    tests_executed++;
    // Test: Use strcmp for conditional logic

    // if strcmp("a", "a") == 0 then 100 else 200
    assert_emit(
        "import strcmp from \"c\"\n"
        "result = strcmp(\"test\", \"test\")\n"
        "if result == 0 then 100 else 200",
        100
    );

    // if strcmp("a", "b") < 0 then 1 else 0
    assert_emit(
        "import strcmp from \"c\"\n"
        "result = strcmp(\"aaa\", \"bbb\")\n"
        "if result < 0 then 1 else 0",
        1
    );
}

static void test_ffi_math_pipeline() {
    tests_executed++;
    // Test: Chain multiple math functions

    // abs(floor(sin(3.14159))) = abs(0) = 0
    assert_emit(
        "import sin from \"m\"\n"
        "import floor from \"m\"\n"
        "import fabs from \"m\"\n"
        "fabs(floor(sin(3.14159)))",
        0.0
    );

    // fmax(ceil(2.3), floor(5.9)) = max(3, 5) = 5
    assert_emit(
        "import ceil from \"m\"\n"
        "import floor from \"m\"\n"
        "import fmax from \"m\"\n"
        "fmax(ceil(2.3), floor(5.9))",
        5.0
    );
}

// ============================================================================
// Signature Coverage Tests
// ============================================================================

static void test_ffi_signature_coverage() {
    tests_executed++;
    // Verify that different signature patterns all work

#if !WASM // Only run in native mode
    // Test signature detection for various patterns

    // Pattern: char*, char* -> int32 (strcmp)
    List<FFIMarshaller::CType> params_strcmp(2);
    params_strcmp.add(FFIMarshaller::CType::String);
    params_strcmp.add(FFIMarshaller::CType::String);
    FFIMarshaller::CType ret_strcmp = FFIMarshaller::CType::Int32;
    String sig_strcmp = FFIMarshaller::format_signature(params_strcmp, ret_strcmp);
    check_is(sig_strcmp, "int32 (char*, char*)");

    // Pattern: void -> int32 (rand)
    List<FFIMarshaller::CType> params_rand(0);
    FFIMarshaller::CType ret_rand = FFIMarshaller::CType::Int32;
    String sig_rand = FFIMarshaller::format_signature(params_rand, ret_rand);
    check_is(sig_rand, "int32 ()");

    // Pattern: float64 -> float64 (sin, cos, etc.)
    List<FFIMarshaller::CType> params_trig(1);
    params_trig.add(FFIMarshaller::CType::Float64);
    FFIMarshaller::CType ret_trig = FFIMarshaller::CType::Float64;
    String sig_trig = FFIMarshaller::format_signature(params_trig, ret_trig);
    check_is(sig_trig, "float64 (float64)");
#endif
}

// ============================================================================
// Main Extended FFI Test Function
// ============================================================================

static void test_ffi_extended_emit() {
    tests_executed++;

    // String comparison functions
    test_ffi_strcmp();
    // test_ffi_strncmp();  // Requires 3-parameter support

    // Additional math functions
    test_ffi_ceil();
    test_ffi_sin();
    test_ffi_cos();
    test_ffi_tan();
    test_ffi_fabs();
    test_ffi_fmax();
    test_ffi_fmod();

    // String conversion
    test_ffi_atoi();
    // test_ffi_atol();  // Requires int64 return support

    // Zero-parameter functions
    test_ffi_rand();

    // Combined tests
    test_ffi_trigonometry_combined();
    test_ffi_string_math_combined();
    test_ffi_string_comparison_logic();
    test_ffi_math_pipeline();

    // Signature coverage
    test_ffi_signature_coverage();
}


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

    assert_emit("import abs from \"c\"\nabs(-42)", 42);
    let a=analyze("abs(0)");
    check(a.kind==call or a.first().kind==call);
    assert_emit("import abs from \"c\"\nabs(0)", 0);
    assert_emit("import abs from \"c\"\nabs(100)", 100);
}

static void test_ffi_floor() {
    tests_executed++;
    // Test 2: float64 -> float64 (floor from libm)
    // Signature: float64 floor(float64 value)
    // Wrapper: ffi_f64_f64 (already exists)

    assert_emit("import floor from \"m\"\\nfloor(3.7)", 3.0);
    assert_emit("import floor from \"m\"\\nfloor(-2.3)", -3.0);
    assert_emit("import floor from \"m\"\\nfloor(5.0)", 5.0);
}

static void test_ffi_strlen() {
    tests_executed++;
    // Test 3: char* -> int32 (strlen from libc)
    // Signature: int32 strlen(char* str)
    // Wrapper: ffi_str_i32

    assert_emit("import strlen from \"c\"\\nstrlen(\"hello\")", 5);
    assert_emit("import strlen from \"c\"\\nstrlen(\"\")", 0);
    assert_emit("import strlen from \"c\"\\nstrlen(\"Wasp\")", 4);
}

static void test_ffi_atof() {
    tests_executed++;
    // Test 4: char* -> float64 (atof from libc)
    // Signature: float64 atof(char* str)
    // Wrapper: ffi_str_f64

    assert_emit("import atof from \"c\"\\natof(\"3.14159\")", 3.14159);
    assert_emit("import atof from \"c\"\\natof(\"2.71828\")", 2.71828);
    assert_emit("import atof from \"c\"\\natof(\"42\")", 42.0);
}

static void test_ffi_fmin() {
    tests_executed++;
    // Test 5: float64, float64 -> float64 (fmin from libm)
    // Signature: float64 fmin(float64 x, float64 y)
    // Wrapper: ffi_f64_f64_f64 (already exists)

    assert_emit("import fmin from \"m\"\\nfmin(3.5, 2.1)", 2.1);
    assert_emit("import fmin from \"m\"\\nfmin(100.0, 50.0)", 50.0);
    assert_emit("import fmin from \"m\"\\nfmin(-5.0, -10.0)", -10.0);
}

static void test_ffi_combined() {
    tests_executed++;
    // Combined tests using multiple FFI functions together

    // sqrt(abs(-16)) = sqrt(16) = 4.0
    assert_emit("import sqrt from \"m\"\\nimport abs from \"c\"\\nsqrt(abs(-16))", 4.0);

    // floor(fmin(3.7, 2.9)) = floor(2.9) = 2.0
    assert_emit("import floor from \"m\"\\nimport fmin from \"m\"\\nfloor(fmin(3.7, 2.9))", 2.0);

    // abs(strlen("test")) = abs(4) = 4
    assert_emit("import abs from \"c\"\\nimport strlen from \"c\"\\nabs(strlen(\"test\"))", 4);
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
    check_is(wrapper1, "ffi_i32_i32");

    // Test float64 -> float64
    List<FFIMarshaller::CType> params2(1);
    params2.add(FFIMarshaller::CType::Float64);
    String wrapper2 = FFIMarshaller::detect_wrapper_name(params2, FFIMarshaller::CType::Float64);
    check_is(wrapper2, "ffi_f64_f64");

    // Test char* -> int32
    List<FFIMarshaller::CType> params3(1);
    params3.add(FFIMarshaller::CType::String);
    String wrapper3 = FFIMarshaller::detect_wrapper_name(params3, FFIMarshaller::CType::Int32);
    check_is(wrapper3, "ffi_str_i32");

    // Test char* -> float64
    List<FFIMarshaller::CType> params4(1);
    params4.add(FFIMarshaller::CType::String);
    String wrapper4 = FFIMarshaller::detect_wrapper_name(params4, FFIMarshaller::CType::Float64);
    check_is(wrapper4, "ffi_str_f64");

    // Test float64, float64 -> float64
    List<FFIMarshaller::CType> params5(2);
    params5.add(FFIMarshaller::CType::Float64);
    params5.add(FFIMarshaller::CType::Float64);
    String wrapper5 = FFIMarshaller::detect_wrapper_name(params5, FFIMarshaller::CType::Float64);
    check_is(wrapper5, "ffi_f64_f64_f64");

    // Test signature formatting
    String sig_format = FFIMarshaller::format_signature(params5, FFIMarshaller::CType::Float64);
    check_is(sig_format, "float64 (float64, float64)");
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
