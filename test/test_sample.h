#pragma once

// Sample and integration test functions

// Sample tests
void testAllSamples();
void testSample();
void testKitchensink();

// Main test runners
extern "C" void testCurrent();
void testAllEmit();
void testAllWasm();
void testAllAngle();
void testWasmGC();

// External references
extern Node &result;

#if WEBAPP
void console_log(const char *s);
#endif
