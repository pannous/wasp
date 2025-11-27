#pragma once

// WebAssembly test functions

// WASM emitter and basics
void testEmitter();
void testEmitBasics();
void testWasmStuff();

// WASM merging and linking
void testMergeGlobal();
void testMergeMemory();
void testMergeRuntime();
void testMergeOwn();
void testMergeWabt();
void testMergeWabtByHand();

// WASM globals and locals
void testGlobals();
void test_get_local();
void testWasmTypedGlobals();
void testWasmMutableGlobal();
void testWasmMutableGlobal2();
void testWasmMutableGlobalImports();

// WASM functions
void testWasmFunctionDefiniton();
void testWasmFunctionCalls();
void testRenameWasmFunction();

// WASM variables
void testWasmVariables0();
void testWasmIncrement();
void testSelfModifying();

// WASM control flow
void testWasmTernary();
void testWasmIf();
void testWasmWhile();
void testForLoopClassic();
void testForLoops();

// WASM logic
void testWasmLogicPrimitives();
void testWasmLogicUnaryVariables();
void testWasmLogicUnary();
void testWasmLogicOnObjects();
void testWasmLogic();
void testWasmLogicNegated();
void testWasmLogicCombined();

// WASM comparisons
void testComparisonMath();
void testComparisonId();
void testComparisonIdPrecedence();
void testComparisonPrimitives();

// WASM returns
void testConstReturn();

// WASM lazy evaluation
void testLazyEvaluation();

// WASM memory
void testWasmMemoryIntegrity();

// WASM runtime
void testWasmRunner();
void testWasmRuntimeExtension();
void testWasmSpeed();

// WASM garbage collection
void testWasmGC();
int test_wasmedge_gc();

// WASM multi-value
void testMultiValue();

// WASM imports/exports
void testImportWasm();
void testImport42();

// WASM data sections
void testNamedDataSections();

// WASM host interaction
void testHostDownload();

// WASM node operations
void testNodeDataBinaryReconstruction();

// WASM print
void testPrint();
void test_fd_write();

// Comprehensive WASM tests
void testAllWasm();
void testAllEmit();

// WASM range tests
void testRange();

// WASM arguments
void testArguments();

// WAST (WebAssembly Text format)
void testParse();
void testWast();
