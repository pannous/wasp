#pragma once

// Implementation and low-level test functions

// LEB128 encoding
void testLebByteSize();

// Memory and performance
void testLeaks();
void testWasmSpeed();
void testMatrixOrder();

// Termination
void testWrong0Termination();

// Deep colon parsing
void testDeepColon();
void testDeepColon2();

// Type issues
void testStupidLongLong();

// Assert tests
void testAsserts();
void testAssert();
void testAssertRun();

// Bug tests
void testOldRandomBugs();
void testRecentRandomBugs();

// Ping
void testPing();

// 2Def
void test2Def();
