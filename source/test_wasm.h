#pragma once

// WebAssembly test functions

// Range tests
void testRange();

// Merge and linking tests
void testMergeGlobal();
void testMergeMemory();
void testMergeRuntime();
void testMergeOwn();
void testMergeWabt();
void testMergeWabtByHand();

// Emitter tests
void testEmitter();

// Math and operations
void testImplicitMultiplication();
void testMathPrimitives();
void testFloatOperators();
void testNorm2();
void testNorm();
void testMathOperators();
void testMathOperatorsRuntime();
void testMathExtra();
void testMathLibrary();

// Globals and locals
void testGlobals();
void test_get_local();
void testWasmTypedGlobals();
void testWasmMutableGlobal();
void testWasmMutableGlobal2();
void testWasmMutableGlobalImports();

// Function tests
void testWasmFunctionDefiniton();
void testWasmFunctionCalls();

// Ternary and conditionals
void testWasmTernary();
void testLazyEvaluation();

// Return tests
void testConstReturn();

// Print tests
void testPrint();

// Comparison tests
void testComparisonMath();
void testComparisonId();
void testComparisonIdPrecedence();
void testComparisonPrimitives();

// Logic tests
void testWasmLogicPrimitives();
void testWasmLogicUnaryVariables();
void testWasmLogicUnary();
void testWasmLogicOnObjects();
void testWasmLogic();
void testWasmLogicNegated();
void testWasmLogicCombined();

// Variable tests
void testWasmVariables0();
void testWasmIncrement();
void testSelfModifying();

// Control flow
void testWasmIf();
void testWasmWhile();

// Memory tests
void testWasmMemoryIntegrity();

// Square and exponentiation
void testSquarePrecedence();
void testSquares();
void testSquareExpWasm();

// Bug tests
void testOldRandomBugs();
void testRecentRandomBugs();

// Runtime tests
void testWasmRuntimeExtension();

// String tests
void testStringConcatWasm();
void testStringIndicesWasm();
void testWasmString();

// Object and array tests
void testObjectPropertiesWasm();
void testArrayIndicesWasm();

// Miscellaneous WASM
void testWasmStuff();

// Math functions
void testRoundFloorCeiling();
void testLogarithm();
void testLogarithm2();
void testRoot();
void testRootFloat();

// Custom operators
void testCustomOperators();

// Index tests
void testIndexWasm();

// Import tests
void testImportWasm();

// Smart return tests
void testSmartReturnHarder();
void testSmartReturn();

// Struct tests
void testStruct();
void testStruct2();

// Garbage collection
void testWasmGC();

// Multi-value return
void testMultiValue();

// Assert tests
void testAssertRun();
void testAssert();

// For loop tests
void testForLoopClassic();
void testForLoops();

// Data sections
void testNamedDataSections();

// Auto type tests
void testAutoSmarty();

// Arguments
void testArguments();

// Fibonacci
void testFibonacci();

// Host interaction
void testHostDownload();

// Trigonometry
void testSinus2();
void testSinus();

// Emit tests
void testEmitBasics();

// Node tests
void testNodeDataBinaryReconstruction();

// Browser-specific
void testFixedInBrowser();
void testTodoBrowser();

// Comprehensive WASM tests
void testAllWasm();
