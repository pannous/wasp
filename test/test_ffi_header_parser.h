#pragma once
#include "../source/ffi_header_parser.h"

// Test that Wasp's parser can parse C function declarations
// This validates that we can reuse Wasp's parser to read C header files

static void test_parse_c_function_declaration() {
    tests_executed++;

    #if !WASM // Only run in native mode
    // Test 1: Parse simple C function declaration
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

    // Test 2: Parse function with multiple parameters
    String c_code2 = "double fmin(double x, double y);";
    Node& parsed2 = parse(c_code2);

    print("\nParsed: double fmin(double x, double y);");
    print("Result: "s + parsed2.serialize());

    // Test 3: Parse function with string parameter
    String c_code3 = "int strlen(char* str);";
    Node& parsed3 = parse(c_code3);

    print("\nParsed: int strlen(char* str);");
    print("Result: "s + parsed3.serialize());

    // Test 4: Parse extern declaration
    String c_code4 = "extern double floor(double x);";
    Node& parsed4 = parse(c_code4);

    print("\nParsed: extern double floor(double x);");
    print("Result: "s + parsed4.serialize());

    #endif
}

static void test_extract_function_signature() {
    tests_executed++;

    #if !WASM
    print("\n=== Testing Signature Extraction ===");

    // Test 1: double sqrt(double x)
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

    // Test 2: double fmin(double x, double y)
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

    // Test 3: int strlen(char* str)
    String c_code3 = "int strlen(char* str);";
    Node& parsed3 = parse(c_code3);

    // Debug: show structure
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
    // Test C type to Wasp type mapping
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

static void test_ffi_header_parser() {
    print("\n=== Testing FFI Header Parser ===\n");
    test_parse_c_function_declaration();
    test_extract_function_signature();
    test_c_type_mapping();
}
