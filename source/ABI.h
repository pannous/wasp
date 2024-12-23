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
//    unsigned long long type; // -Wreturn-type-c-linkage OK
    Type type; // -Wno-return-type-c-linkage
};

struct Retol {
    int64 value;
    Type type;
};

// example of multi-value return in C/C++ :
// extern "C" Reto swap(int a, int b);
//(func (export "add_and_subtract") (param i32 i32) (result i32 i32)
//local.get 1
//local.get 0
//)




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


// todo move these to ABI.h once it is used:
//	map_header_32 = USE Node!
#define array_header_length 16 // 4*4 to align with int64
//#include "ABI.h"
#define string_header_32 0x10000000 // compatible with String
#define array_header_32  0x40000000 // compatible with List
#define buffer_header_32  0x44000000 // incompatible with List!
//#define map_header_32    0x46000000 // compatible with Map
#define map_header_32    0x50000000 // compatible with Map
#define ref_header_32    0x60000000 // index of externref == js object ! in table (export "externref_table") 1 externref
//#define smart_mask_32    0x70000000 ??
#define node_header_32   0x80000000 // more complex than array!
#define negative_mask_32 0x80000000 // todo ^^ ?
#define kind_header_32   0xDD000000
// 64 bit headers occur 1. if no multi value available

#define string_header_64 0x0010000000000000L // todo : what happened to 0x9 smartType4bit ??
#define array_header_64 0x0040000000000000L // why 0x004? because first 2 bats indicate doubles/ints!
#define ref_header_64 0x0060000000000000L // why 0x004? because first 2 bats indicate doubles/ints!
#define node_header_64 0x0A000000000000000L // todo undup
#define codepoint_header_64 0x00C0000000000000L

// smart_pointer_64 == 32 bits smart_type(header+payload) + 32 bit value
//enum smart_pointer_masks {
static int64
//	float_header_64 = 0x0020000000000000, not needed, use:
smart_type_64_mask = 0xFFFFFFFF00000000L,
        double_mask_64 = 0x7F00000000000000L,
        smart_mask_64 = 0x00FF000000000000L,
//	negative_mask_64 = 0x8000000000000000,
negative_mask_64 = 0xFF00000000000000L,
        type_mask_64_word = 0xFFFF000000000000L,
        smart_pointer_value60_mask = 0x0FFFFFFFFFFFFFFFL,
        smart_pointer_value56_mask = 0x00FFFFFFFFFFFFFFL,
        smart_pointer_value52_mask = 0x000FFFFFFFFFFFFFL,
        smart_pointer_value48_mask = 0x0000FFFFFFFFFFFFL,
        smart_pointer_value32_mask = 0x00000000FFFFFFFFL; // or do shift << 32 >> 32 to remove header
//	negative_long_mask_64 = 0xBFF0000000000000,

//};

// 3 * sizeof(int32)  header, kind, length before *DATA !
// sizeof(List) - sizeof(S*)


smartType4bit getSmartType(smart_pointer_32 spo);
//smartType4bit getSmartType(smart_pointer_64 spo);

static smart_pointer_64 smart_pointer_type_mask = 0xFFFFFFFF00000000L; // full : smartType4bit plus 12 specifier bit plus 16 bit type payload
static smart_pointer_64 smart_pointer_header_mask = 0xFFFF000000000000L; // smartType4bit plus 12 specifier  todo maybe less F?
static smart_pointer_64 smart_pointer_payload_mask = 0x0000FFFF00000000L; // todo maybe less F?
//todo stringa = 0x9 vs
#define string_header_64 0x0010000000000000L // todo : what happened to 0x9 smartType4bit ??
#define array_header_64  0x0040000000000000L // why 0x004? because first 2 bats indicate doubles/ints!

static smart_pointer_64 smart_pointer_node_signature = 0xA000000000000000L;// ((smart_pointer_64)nodep) << 63; // 0xA0…0

//    short smart_type_payload = (short)(smartPointer64 & 0x0000FFFF00000000L)>>16;// type payload including length (of array)

