#pragma once

// String-related test functions

// String concatenation
void testEmitStringConcatenation();
void testStringConcatenation();
void testConcatenationBorderCases();
void testConcatenation();
void testStringConcatWasm();

// String interpolation
void testStringInterpolation();

// String operations
void testString();
void testReplaceAll();
void testStringReferenceReuse();
void testStringIndicesWasm();

// External strings
void testExternString();

// WASM string tests
void testWasmString();

// Unicode and encoding
void testUTFinCPP();
void testUnicode_UTF16_UTF32();
void testUTF();
void testUpperLowerCase();
