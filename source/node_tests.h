#pragma once

// Node data structure test functions

// Node basics
void testNodeBasics();
void testNodeName();
void testNodeConversions();

// Node operations
void testOverwrite();
void testAddField();
void testReplace();

// Metadata
void testMeta();
void testMetaAt();
void testMetaAt2();

// Parent and context
void testParent();
void testParentContext();

// Nil values
void checkNil();
void testNilValues();

// Errors
void testErrors();

// Node copying
void testDeepCopyBug();
void testDeepCopyDebugBugBug();
void testDeepCopyDebugBugBug2();

// Reference tests
void testExternReferenceXvalue();

// VectorShim
void testVectorShim();
