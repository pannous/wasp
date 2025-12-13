#ifndef TEST_FFI_IMPORT_PATTERN_H
#define TEST_FFI_IMPORT_PATTERN_H

// Test FFI Import Pattern Recognition
// Verifies that "import funcname from library" syntax is properly parsed

static void test_import_from_pattern_parse() {
    tests_executed++;

    #if !WASM // Only run in native mode
    // Test parsing of import-from pattern

    // Parse: import abs from "c"
    String code1 = "import abs from \"c\"";
    Node parsed1 = parse(code1);

    print("Parsed import abs from \"c\":");
    print(parsed1.serialize());

    // Check if it's recognized as import
    check_is(parsed1.name, "import");

    // Parse: import sqrt from "m"
    String code2 = "import sqrt from \"m\"";
    Node parsed2 = parse(code2);

    print("Parsed import sqrt from \"m\":");
    print(parsed2.serialize());

    #endif
}

static void test_import_from_pattern_emit() {
    tests_executed++;

    // Test that import-from actually works in emission
    // These tests are currently skipped because FFI isn't fully integrated yet
    skip(
        // Basic import and call
        assert_emit("import abs from \"c\"\nabs(-42)", 42);

        // Multiple imports from same library
        assert_emit("import floor from \"m\"\nimport ceil from \"m\"\nceil(floor(3.7))", 3.0);

        // Import and use in expression
        assert_emit("import sqrt from \"m\"\nsqrt(16)", 4.0);
    )
}

static void test_import_from_vs_include() {
    tests_executed++;

    #if !WASM
    // Verify difference between "import from" (FFI) and "import" (file include)

    // FFI import: import abs from "c"
    String ffi_import = "import abs from \"c\"";
    Node ffi_node = parse(ffi_import);

    print("FFI import node:");
    print(ffi_node.serialize());

    // File import: import "myfile.wasp"
    // This would load myfile.wasp

    // TODO: Verify that FFI imports are properly distinguished from file imports
    #endif
}

static void test_ffi_import_pattern() {
    test_import_from_pattern_parse();
    test_import_from_pattern_emit();
    test_import_from_vs_include();
}

#endif // TEST_FFI_IMPORT_PATTERN_H
