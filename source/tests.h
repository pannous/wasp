#pragma once
extern Node &result;
extern "C" void testCurrent();
#if not RUNTIME_ONLY
void testAllEmit();

void testAllWasm();

void testAllAngle();

void testWasmGC();

#endif