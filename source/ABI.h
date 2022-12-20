#pragma once
// todo: move smartpointer stuff here
#include "NodeTypes.h"
//#include "Util.h"
#define ABI_64 sizeof(void*)==8
// Any_Header
// ⚠️ pointer size is not abi stable between 32 and 64 bit, so sizeof(Header) itself is not abi stable!
struct Header { // ⚠️ the first fields are shared among all Wasp classes, the payload includes extra data!!
    int header; // internal type node, string, list/array todo: merge with type!?
    int length; // of array types, sizeOf(…) otherwise
    Type type;   // type of node / array ≈ generics
    int capacity; // in lists, child_pointer in wasm32_node_struct
//	int refcount; // todo? or via wasm 2.0 ? see  Memory manager / Garbage collector info
//	int size; static sizeOf(…) not needed for each instance!! for unknown classes use length field
//	int size; todont:  add to all classes 1. to align 4*8 bytes 2. to allow extra fields after 'array' 3. total size or extra fields??
//    Header *meta; // memory, debug, … can be anything via smart header  todo:  add to all classes
    void *data;// 0  pointer to data[] vs direct data in Array_Header! Payload starts here  e.g. chars of string, items of list, children of node
    void *extra; // extra fields in classes are only inferable through reflection/inspection. The size field of WaspData sets the limit
};

// see wasm32_node_struct vs Node
// see wasm32_list_struct vs List


// same for C_String but not for String which has extra fields and pointer to data
struct Array_Header { /*Any_Header*/
    int header;
    Type kind;
    int length;
    Type type; // useless, just for alignment padding
    void *data;// ⚠️ data… continuation in memory, not pointer to different data!
    // void *extra_fields…; // in objects without data continuation
};

// https://github.com/WebAssembly/component-model/blob/main/design/mvp/CanonicalABI.md
struct Canonical_ABI_string {
    int length;
    int pointer;
};

struct RustData {
};
//https://faultlore.com/blah/rust-layouts-and-abis/



// allows boxed return of bigint via Address to longy Node …
struct Reto {
	int value;
	Type type;
};

struct Retol {
	int64 value;
    Type type;
};


typedef int usize;
typedef int u32;
struct AssemblyScriptData {
	usize mmInfo;// 	-20 	usize 	Memory manager info
	usize gcInfo;// 	-16 	usize 	Garbage collector info
	usize gcInfo2;// 	-12 	usize 	Garbage collector info
	u32 rtId;// 	-8 	u32 	Unique id of the concrete class
	u32 rtSize;// 	-4 	u32 	Size of the data following the header
	void *payload;// 0 	Payload starts here
};
