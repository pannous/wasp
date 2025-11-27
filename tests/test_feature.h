#pragma once

// Language feature test functions

// WIT (WebAssembly Interface Types)
void testWit();
void testWitInterface();
void testWitExport();
void testWitFunction();
void testWitImport();

// Pattern matching
void testPattern();

// Flags and enums
void testFlags();
void testFlags2();
void testFlagSafety();
void testEnumConversion();
void testBitField();

// Classes
void testClass();

// Bindings
void testEqualsBinding();
void testColonImmediateBinding();
void bindgen(Node &n);

// Use/import
void testUse();
void testInclude();

// Exceptions
void testExceptions();

// Naming conventions
void testHyphenUnits();
void testHypenVersusMinus();
void testKebabCase();
void testDidYouMeanAlias();

// WGSL (WebGPU Shading Language)
void testWGSL();

// Operators
void testLengthOperator();
void testMinusMinus();

// Evaluation
void testEval();
void testEval3();

// Empty/nil
void testEmpty();

// Modern C++ features
void testModernCpp();
void testCpp();

// Special values
void testEqualities();
int testNodiscard();
