#pragma once
#include "../source/ffi_marshaller.h"
#include "../source/ffi_dynamic_wrapper.h"
#include "../source/ffi_loader.h"
#include "../source/ffi_header_parser.h"

// Comprehensive FFI Tests
// All FFI-related tests consolidated from:
//   - test_dynlib_import.h
//   - test_ffi_comprehensive.h
//   - test_ffi_header_parser.h
//   - test_ffi_import_pattern.h
//
// Implementation uses:
//   - source/ffi_loader.h (library loading)
//   - source/ffi_marshaller.h (type conversion and signature detection)
//   - source/ffi_dynamic_wrapper.h (generic wrappers)
//   - source/ffi_header_parser.h (C header parsing)
//   - source/wasm_runner_edge.cpp (wasmedge FFI integration)
//   - source/wasmtime_runner.cpp (wasmtime FFI integration)
//   - source/Angle.cpp (import parsing)
//   - source/Context.h/cpp (FFI function registry)

// ============================================================================
// Dynamic Library Import Tests (using 'use' keyword)
// ============================================================================

static void test_dynlib_import_emit() {
    tests_executed++;
#ifdef NATIVE_FFI
    // Test FFI import and usage with 'use' keyword
    // These are actual C library functions, not WASM builtins
    // Note: FFI only works in native mode, not when compiled to WASM

    // Math library functions (libm) - using functions that work reliably
    assert_is("use m; ceil(3.2)", 4.0);
    assert_is("use math; floor(3.7)", 3.0);
    assert_is("use m; fmin(3.5, 2.1)", 2.1);
    assert_is("use m; fmax(1.5, 2.5)", 2.5);
#else
    print("SKIPPING test_dynlib_import_emit - NATIVE_FFI not available");
#endif
}

// ============================================================================
// Basic FFI Tests - Core functionality
// ============================================================================

static void test_ffi_abs() {
    tests_executed++;
    // Test: int32 -> int32 (abs from libc)
    assert_emit("import abs from \"c\"\nabs(-42)", 42);
    assert_emit("import abs from \"c\"\nabs(100)", 100);
}

static void test_ffi_floor() {
    tests_executed++;
    // Test: float64 -> float64 (floor from libm)
    assert_emit("import floor from 'm'\nfloor(3.7)", 3.0);
    assert_emit("import floor from 'm'\nfloor(-2.3)", -3.0);
    assert_emit("import floor from 'm'\nfloor(5.0)", 5.0);
}

static void test_ffi_strlen() {
    tests_executed++;
    // Test: char* -> int32 (strlen from libc)
    assert_emit("import strlen from \"c\"\nstrlen(\"hello\")", 5);
    assert_emit("import strlen from \"c\"\nstrlen(\"\")", 0);
    assert_emit("import strlen from \"c\"\nstrlen(\"Wasp\")", 4);
}

static void test_ffi_atof() {
    tests_executed++;
    // Test: char* -> float64 (atof from libc)
    assert_emit("import atof from \"c\"\natof(\"3.14159\")", 3.14159);
    assert_emit("import atof from \"c\"\natof(\"2.71828\")", 2.71828);
    assert_emit("import atof from \"c\"\natof(\"42\")", 42.0);
}

static void test_ffi_fmin() {
    tests_executed++;
    // Test: float64, float64 -> float64 (fmin from libm)
    assert_emit("import fmin from 'm'\nfmin(3.5, 2.1)", 2.1);
    assert_emit("import fmin from 'm'\nfmin(100.0, 50.0)", 50.0);
    assert_emit("import fmin from 'm'\nfmin(-5.0, -10.0)", -10.0);
}

static void test_ffi_combined() {
    tests_executed++;
    // Combined tests using multiple FFI functions together

    // sqrt(abs(-16)) = sqrt(16) = 4.0
    assert_emit("import sqrt from 'm'\nimport abs from \"c\"\nsqrt(abs(-16))", 4.0);

    // floor(fmin(3.7, 2.9)) = floor(2.9) = 2.0
    assert_emit("import floor from 'm'\nimport fmin from 'm'\nfloor(fmin(3.7, 2.9))", 2.0);

    // abs(strlen("test")) = abs(4) = 4
    assert_emit("import abs from \"c\"\nimport strlen from \"c\"\nabs(strlen(\"test\"))", 4);
}

// ============================================================================
// String Comparison Functions
// ============================================================================

static void test_ffi_strcmp() {
    tests_executed++;
    // Test: int strcmp(const char* s1, const char* s2)
    assert_emit("import strcmp from \"c\"\nstrcmp(\"hello\", \"hello\")", 0);
    assert_emit("import strcmp from \"c\"\nx=strcmp(\"abc\", \"def\");x<0", 1);
    assert_emit("import strcmp from \"c\"\nx=strcmp(\"xyz\", \"abc\");x>0", 1);
    assert_emit("import strcmp from \"c\"\nstrcmp(\"\", \"\")", 0);
}

static void test_ffi_strncmp() {
    tests_executed++;
    // Test: int strncmp(const char* s1, const char* s2, size_t n)
    assert_emit("import strncmp from \"c\"\nstrncmp(\"hello\", \"help\", 3)", 0);
    assert_emit("import strncmp from \"c\"\nx=strncmp(\"hello\", \"help\", 5);x!=0", 1);
}

// ============================================================================
// Additional Math Functions
// ============================================================================

static void test_ffi_ceil() {
    tests_executed++;
    // Test: double ceil(double x)
    assert_emit("import ceil from 'm'\nceil(3.2)", 4.0);
    assert_emit("import ceil from 'm'\nceil(-2.8)", -2.0);
    assert_emit("import ceil from 'm'\nceil(5.0)", 5.0);
    assert_emit("import ceil from 'm'\nceil(0.1)", 1.0);
}

static void test_ffi_sin() {
    tests_executed++;
    // Test: double sin(double x)
    assert_emit("import sin from 'm'\nsin(0.0)", 0.0);
    assert_emit("import sin from 'm'\nsin(1.5707963267948966)", 1.0);
    assert_emit("import sin from 'm'\nimport abs from \"c\"\nabs(sin(3.141592653589793))<0.001", 1);
}

static void test_ffi_cos() {
    tests_executed++;
    // Test: double cos(double x)
    assert_emit("import cos from 'm'\ncos(0.0)", 1.0);
    assert_emit("import cos from 'm'\nimport abs from \"c\"\nabs(cos(1.5707963267948966))<0.001", 1);
    assert_emit("import cos from 'm'\ncos(3.141592653589793)", -1.0);
}

static void test_ffi_tan() {
    tests_executed++;
    // Test: double tan(double x)
    assert_emit("import tan from 'm'\ntan(0.0)", 0.0);
    assert_emit("import tan from 'm'\ntan(0.7853981633974483)", 1.0);
}

static void test_ffi_fabs() {
    tests_executed++;
    // Test: double fabs(double x)
    assert_emit("import fabs from 'm'\nfabs(3.14)", 3.14);
    assert_emit("import fabs from 'm'\nfabs(-3.14)", 3.14);
    assert_emit("import fabs from 'm'\nfabs(0.0)", 0.0);
}

static void test_ffi_fmax() {
    tests_executed++;
    // Test: double fmax(double x, double y)
    assert_emit("import fmax from 'm'\nfmax(3.5, 2.1)", 3.5);
    assert_emit("import fmax from 'm'\nfmax(100.0, 200.0)", 200.0);
    assert_emit("import fmax from 'm'\nfmax(-5.0, -10.0)", -5.0);
}

static void test_ffi_fmod() {
    tests_executed++;
    // Test: double fmod(double x, double y)
    assert_emit("import fmod from 'm'\nfmod(5.5, 2.0)", 1.5);
    assert_emit("import fmod from 'm'\nfmod(10.0, 3.0)", 1.0);
}

// ============================================================================
// String Conversion Functions
// ============================================================================

static void test_ffi_atoi() {
    tests_executed++;
    // Test: int atoi(const char* str)
    assert_emit("import atoi from \"c\"\natoi(\"42\")", 42);
    assert_emit("import atoi from \"c\"\natoi(\"-123\")", -123);
    assert_emit("import atoi from \"c\"\natoi(\"0\")", 0);
    assert_emit("import atoi from \"c\"\natoi(\"999\")", 999);
}

static void test_ffi_atol() {
    tests_executed++;
    // Test: long atol(const char* str)
    assert_emit("import atol from \"c\"\natol(\"1234567\")", 1234567);
    assert_emit("import atol from \"c\"\natol(\"-999999\")", -999999);
}

// ============================================================================
// Zero-Parameter Functions
// ============================================================================

static void test_ffi_rand() {
    tests_executed++;
    // Test: int rand(void)
    assert_emit("import rand from \"c\"\nx=rand();x>=0", 1);
    assert_emit("import rand from \"c\"\nx=rand();y=rand();x!=y", 1);
}

// ============================================================================
// Combined/Complex Tests
// ============================================================================

static void test_ffi_trigonometry_combined() {
    tests_executed++;
    // Test: sin²(x) + cos²(x) = 1 (Pythagorean identity)
    assert_emit(
        "import sin from 'm'\n"
        "import cos from 'm'\n"
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
    assert_emit(
        "import atoi from \"c\"\n"
        "x = atoi(\"10\")\n"
        "y = atoi(\"20\")\n"
        "x + y",
        30
    );

    assert_emit(
        "import atof from \"c\"\n"
        "import ceil from 'm'\n"
        "ceil(atof(\"3.7\"))",
        4.0
    );
}

static void test_ffi_string_comparison_logic() {
    tests_executed++;
    // Test: Use strcmp for conditional logic
    assert_emit(
        "import strcmp from \"c\"\n"
        "result = strcmp(\"test\", \"test\")\n"
        "if result == 0 then 100 else 200",
        100
    );

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
    assert_emit(
        "import sin from 'm'\n"
        "import floor from 'm'\n"
        "import fabs from 'm'\n"
        "fabs(floor(sin(3.14159)))",
        0.0
    );

    assert_emit(
        "import ceil from 'm'\n"
        "import floor from 'm'\n"
        "import fmax from 'm'\n"
        "fmax(ceil(2.3), floor(5.9))",
        5.0
    );
}

// ============================================================================
// Signature Detection and Coverage Tests
// ============================================================================

static void test_ffi_signature_detection() {
    tests_executed++;
#if !WASM
    List<FFIMarshaller::CType> params1(1);
    params1.add(FFIMarshaller::CType::Int32);
    String wrapper1 = FFIMarshaller::detect_wrapper_name(params1, FFIMarshaller::CType::Int32);
    check_is(wrapper1, "ffi_i32_i32");

    List<FFIMarshaller::CType> params2(1);
    params2.add(FFIMarshaller::CType::Float64);
    String wrapper2 = FFIMarshaller::detect_wrapper_name(params2, FFIMarshaller::CType::Float64);
    check_is(wrapper2, "ffi_f64_f64");

    List<FFIMarshaller::CType> params3(1);
    params3.add(FFIMarshaller::CType::String);
    String wrapper3 = FFIMarshaller::detect_wrapper_name(params3, FFIMarshaller::CType::Int32);
    check_is(wrapper3, "ffi_str_i32");

    List<FFIMarshaller::CType> params4(1);
    params4.add(FFIMarshaller::CType::String);
    String wrapper4 = FFIMarshaller::detect_wrapper_name(params4, FFIMarshaller::CType::Float64);
    check_is(wrapper4, "ffi_str_f64");

    List<FFIMarshaller::CType> params5(2);
    params5.add(FFIMarshaller::CType::Float64);
    params5.add(FFIMarshaller::CType::Float64);
    String wrapper5 = FFIMarshaller::detect_wrapper_name(params5, FFIMarshaller::CType::Float64);
    check_is(wrapper5, "ffi_f64_f64_f64");

    String sig_format = FFIMarshaller::format_signature(params5, FFIMarshaller::CType::Float64);
    check_is(sig_format, "float64 (float64, float64)");
#endif
}

static void test_ffi_signature_coverage() {
    tests_executed++;
#if !WASM
    List<FFIMarshaller::CType> params_strcmp(2);
    params_strcmp.add(FFIMarshaller::CType::String);
    params_strcmp.add(FFIMarshaller::CType::String);
    FFIMarshaller::CType ret_strcmp = FFIMarshaller::CType::Int32;
    String sig_strcmp = FFIMarshaller::format_signature(params_strcmp, ret_strcmp);
    check_is(sig_strcmp, "int32 (char*, char*)");

    List<FFIMarshaller::CType> params_rand(0);
    FFIMarshaller::CType ret_rand = FFIMarshaller::CType::Int32;
    String sig_rand = FFIMarshaller::format_signature(params_rand, ret_rand);
    check_is(sig_rand, "int32 ()");

    List<FFIMarshaller::CType> params_trig(1);
    params_trig.add(FFIMarshaller::CType::Float64);
    FFIMarshaller::CType ret_trig = FFIMarshaller::CType::Float64;
    String sig_trig = FFIMarshaller::format_signature(params_trig, ret_trig);
    check_is(sig_trig, "float64 (float64)");
#endif
}

// ============================================================================
// Import Pattern Tests
// ============================================================================

static void test_import_from_pattern_parse() {
    tests_executed++;
#if !WASM
    String code1 = "import abs from \"c\"";
    Node parsed1 = parse(code1);
    print("Parsed import abs from \"c\":");
    print(parsed1.serialize());
    check_is(parsed1.name, "import");

    String code2 = "import sqrt from \"m\"";
    Node parsed2 = parse(code2);
    print("Parsed import sqrt from \"m\":");
    print(parsed2.serialize());
#endif
}

static void test_import_from_pattern_emit() {
    tests_executed++;
    skip(
        assert_emit("import abs from \"c\"\nabs(-42)", 42);
        assert_emit("import floor from \"m\"\nimport ceil from \"m\"\nceil(floor(3.7))", 3.0);
        assert_emit("import sqrt from \"m\"\nsqrt(16)", 4.0);
    )
}

static void test_import_from_vs_include() {
    tests_executed++;
#if !WASM
    String ffi_import = "import abs from \"c\"";
    Node ffi_node = parse(ffi_import);
    print("FFI import node:");
    print(ffi_node.serialize());
#endif
}

// ============================================================================
// C Header Parser Tests
// ============================================================================

static void test_parse_c_function_declaration() {
    tests_executed++;
#if !WASM
    String c_code1 = "double sqrt(double x);";
    Node& parsed1 = parse(c_code1);
    print("Parsed: double sqrt(double x);");
    print("Result: "s + parsed1.serialize());
    print("Kind: "s + typeName(parsed1.kind));
    print("Name: "s + parsed1.name);
    print("Length: "s + (int)parsed1.length);

    if (parsed1.length > 0) {
        print("First child name: "s + parsed1.first().name);
        print("First child kind: "s + typeName(parsed1.first().kind));
    }

    if (parsed1.length > 1) {
        print("Second child name: "s + parsed1[1].name);
        print("Second child kind: "s + typeName(parsed1[1].kind));
        print("Second child length: "s + (int)parsed1[1].length);
    }

    String c_code2 = "double fmin(double x, double y);";
    Node& parsed2 = parse(c_code2);
    print("\nParsed: double fmin(double x, double y);");
    print("Result: "s + parsed2.serialize());

    String c_code3 = "int strlen(char* str);";
    Node& parsed3 = parse(c_code3);
    print("\nParsed: int strlen(char* str);");
    print("Result: "s + parsed3.serialize());

    String c_code4 = "extern double floor(double x);";
    Node& parsed4 = parse(c_code4);
    print("\nParsed: extern double floor(double x);");
    print("Result: "s + parsed4.serialize());
#endif
}

static void test_extract_function_signature() {
    tests_executed++;
#if !WASM
    print("\n=== Testing Signature Extraction! ===");

    String c_code1 = "double sqrt(double x);";
    Node& parsed1 = parse(c_code1);
    FFIHeaderSignature sig1;
    sig1.library = "m";

    if (extractFunctionSignature(parsed1, sig1)) {
        print("✓ Extracted: "s + sig1.name);
        print("  Return: "s + sig1.return_type);
        print("  Params: "s + (int)sig1.param_types.size());
        for (int i = 0; i < sig1.param_types.size(); i++) {
            print("    ["s + i + "] " + sig1.param_types[i]);
        }
        print("  Generated: "s + generateFFISignatureLine(sig1));
    } else {
        print("✗ Failed to extract sqrt signature");
    }

    String c_code2 = "double fmin(double x, double y);";
    Node& parsed2 = parse(c_code2);
    FFIHeaderSignature sig2;
    sig2.library = "m";

    if (extractFunctionSignature(parsed2, sig2)) {
        print("\n✓ Extracted: "s + sig2.name);
        print("  Return: "s + sig2.return_type);
        print("  Params: "s + (int)sig2.param_types.size());
        for (int i = 0; i < sig2.param_types.size(); i++) {
            print("    ["s + i + "] " + sig2.param_types[i]);
        }
        print("  Generated: "s + generateFFISignatureLine(sig2));
    } else {
        print("✗ Failed to extract fmin signature");
    }

    String c_code3 = "int strlen(char* str);";
    Node& parsed3 = parse(c_code3);

    print("\nDEBUG strlen structure:");
    print("  parsed3: "s + parsed3.serialize());
    print("  parsed3.length: "s + (int)parsed3.length);
    for (int i = 0; i < parsed3.length; i++) {
        Node& child = parsed3[i];
        print("  child["s + i + "]: name='" + child.name + "' kind=" + typeName(child.kind) + " len=" + (int)child.length);
        if (child.length > 0) {
            for (int j = 0; j < child.length; j++) {
                Node& grandchild = child[j];
                print("    ["s + j + "] name='" + grandchild.name + "' kind=" + typeName(grandchild.kind) + " len=" + (int)grandchild.length);
                if (grandchild.length > 0) {
                    for (int k = 0; k < grandchild.length; k++) {
                        print("      ["s + k + "] '" + grandchild[k].name + "' (" + typeName(grandchild[k].kind) + ")");
                    }
                }
            }
        }
    }

    FFIHeaderSignature sig3;
    sig3.library = "c";

    if (extractFunctionSignature(parsed3, sig3)) {
        print("\n✓ Extracted: "s + sig3.name);
        print("  Return: "s + sig3.return_type);
        print("  Params: "s + (int)sig3.param_types.size());
        for (int i = 0; i < sig3.param_types.size(); i++) {
            print("    ["s + i + "] " + sig3.param_types[i]);
        }
        print("  Generated: "s + generateFFISignatureLine(sig3));
    } else {
        print("✗ Failed to extract strlen signature");
    }
#endif
}

static void test_c_type_mapping() {
    tests_executed++;
#if !WASM
    check((int)mapCTypeToWasp("double") == (int)float64t);
    check((int)mapCTypeToWasp("float") == (int)float32t);
    check((int)mapCTypeToWasp("int") == (int)int32t);
    check((int)mapCTypeToWasp("long") == (int)i64);
    check((int)mapCTypeToWasp("char*") == (int)charp);
    check((int)mapCTypeToWasp("const char*") == (int)charp);
    check((int)mapCTypeToWasp("void") == (int)nils);
    print("C type mapping tests passed!");
#endif
}

// ============================================================================
// Main Test Runners
// ============================================================================

static void test_ffi_extended_emit() {
    tests_executed++;
    test_ffi_strcmp();
    test_ffi_ceil();
    test_ffi_sin();
    test_ffi_cos();
    test_ffi_tan();
    test_ffi_fabs();
    test_ffi_fmax();
    test_ffi_fmod();
    test_ffi_atoi();
    test_ffi_rand();
    test_ffi_trigonometry_combined();
    test_ffi_string_math_combined();
    test_ffi_string_comparison_logic();
    test_ffi_math_pipeline();
    test_ffi_signature_coverage();
}

static void test_ffi_import_pattern() {
    tests_executed++;
    test_import_from_pattern_parse();
    test_import_from_pattern_emit();
    test_import_from_vs_include();
}

static void test_ffi_header_parser() {
    tests_executed++;
    print("\n=== Testing FFI Header Parser ===\n");
    test_parse_c_function_declaration();
    test_extract_function_signature();
    test_c_type_mapping();
}

static void test_ffi_all() {
    tests_executed++;
    // Main comprehensive test function that runs all FFI tests
    test_ffi_abs();
    test_ffi_floor();
    test_ffi_fmin();
    test_ffi_strlen();
    test_ffi_atof();
    test_ffi_combined();
    test_ffi_signature_detection();
}
