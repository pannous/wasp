#pragma once

// Parser and syntax test functions

// Basic parsing
void testRandomParse();
void testNoBlock();

// Data mode and representations
void testDataMode();
void testRepresentations();

// Significant whitespace
void testSignificantWhitespace();
void testEmptyLineGrouping();
void testSuperfluousIndentation();
void testIndentAsBlock();

// Comments
void testComments();

// Dedentation
void testDedent();
void testDedent2();

// Mark (data notation) tests
void testMarkSimple();
void testMarkMulti();
void testMarkMulti2();
void testMarkMultiDeep();
void testMarkAsMap();

// GraphQL parsing
void testGraphSimple();
void testGraphQlQueryBug();
void testGraphQlQuery();
void testGraphQlQuery2();
void testGraphQlQuerySignificantWhitespace();
void testGraphParams();

// Division parsing
void testDiv();
void testDivMark();
void testDivDeep();

// Group and cascade
void testGroupCascade();
void testGroupCascade0();
void testGroupCascade1();
void testGroupCascade2();

// Root parsing
void testRoots();
void testRootLists();

// Parameters
void testParams();

// Serialization
void testSerialize();
