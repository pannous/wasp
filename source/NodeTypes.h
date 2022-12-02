#pragma once
//
// Created by pannous on 19.12.19.
//

#include "smart_types.h" // todo merge

//enum Valtype;// forbids forward references to 'enum' types
class Node;

union Type;

#define error(msg) error1(msg,__FILE__,__LINE__)


// todo move these to ABI.h once it is used:
//	map_header_32 = USE Node!
#define node_header_32   0x80000000 // more complex than array!
#define array_header_32  0x40000000 // compatible with List
#define string_header_32 0x10000000 // compatible with String
#define smart_mask_32 0x70000000
#define negative_mask_32 0x80000000
// 64 bit headers occur 1. if no multi value available
#define array_header_64 0x0040000000000000L // why 0x004? because first 2 bats indicate doubles/ints!
#define string_header_64 0x0010000000000000L // todo : what happened to 0x9 smartType4bit ??

// smart_pointer_64 == 32 bits smart_type(header+payload) + 32 bit value
enum smart_pointer_masks {
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
    smart_pointer_value32_mask = 0x00000000FFFFFFFFL, // or do shift << 32 >> 32 to remove header
//	negative_long_mask_64 = 0xBFF0000000000000,
};

// 3 * sizeof(int32)  header, kind, length before *DATA !
// sizeof(List) - sizeof(S*)
#define array_header_length 12

chars typeName(const Type *t);

// types
//extern const Node Double;
//extern const Node Long;
extern Node Int; // maps to int32 in external abi! don't use internally much, only for struct
extern Node Double;
extern Node Long;
extern Node ByteType;
extern Node Bool;
extern Node Charpoint;

//#include "Util.h" // for error() :(
//#include "Code.h"

// Todo: can Type itself become a Node, making the distinction between type and kind superfluous?
// todo dangerous overlap with Valtype in Type!? OK only accessible via mapTypeToWasm
// needs to be stable: Kind is returned in multivalue and thus needs to be parsed by js!
enum Kind {// todo: merge Node.kind with Node.class(?)
    // todo smartType4bit first 16 values!!
    // plurals because of namespace clash
    // TODO add subtypes of Class:Node Variable:Node etc ^^^
    undefined = 0, // ≠ any?
    unknown = 1, //
    objects, // {…} block data with children closures
    strings = 0x9,
    call = 0x10,
    groups, // (…) meta params parameter attributes lists
    tuples = groups, // todo: with restrictions!
    patterns, // […] selectors matches, annotations! [public export extern] function mul(x,y){x*y}
    generics,// node tag or list<node>   NOT value.kind==strings !
    tags = generics,// <html>

    key, // key with value
    fields, // key in struct / class / type / prototype / interface / record (wit) possibly WITHOUT VALUE
    // todo do we really need strict schema separation from normal 'schema' of node{kind=clazz} ?

    reference, // variable identifier name x
    symbol, // one / plus / Jesus
    operators, // TODO: semantic types don't belong here! they interfere with internal structural types key etc!!
    functor, // while(xyz){abc} takes 1?/2/3 blocks if {condition} {then} {else}
    expression, // one plus one
    declaration, // x:=1
    assignment, // x = 1 // really?? needs own TYPE?
    codepoints, // boxed codepoint in value.longy field todo
    buffers, // int[]
    //	ints, // use longy field, but in wasm longs are pointers!
    bools,
    errors, // internal wasp error, NOT to be used in Angle!
    clazz,

    // each class can be made into a typed list / vector int => int[] :, // as Java class,  primitive int vs Node(kind==int) == boxed Int.
    //	vectors     use kind=array type=Any
//	vectors, // any[] vs Node[]  list of type class!  e.g.  numbers xs => xs.type=number(type=vectors)
    arrays, // Node[] vs any[]  untyped array of Nodes. Unlike vector the node is NOT a class! todo: see Classes
    linked_list, // via children or value field??
    meta = linked_list,
    nils = 0x40, // ≈ void_block for compatibility!?  ≠ undefined
    reals = 0x7C, /*  ≠ float64, just hides bugs, these concepts should not be mixed */
    longs = 0x7E, // the signature of parameters/variables is independent!
    flag_entry = longs, // special semantics at compile time for now
    enum_entry = longs, // special semantics at compile time for now
    structs,
    enums,
    flags,// just a boolean enum with “bit-boolean” values 1,2,4,8,…
    variants,
//    unions, // todo, also option
    records, // todo merge concepts with module wasp clazz?
    constructor, // special call?
    last_kind
};// Type =>  must use 'enum' tag to refer to type 'Type' NAH!


// Todo these should NOT appear as Node.kind!
// unboxed primitive raw data (list is compatible with List though!)
// todo use NodeTypes.h Type for smart-pointers :
//	https://github.com/pannous/angle/wiki/smart-pointer
enum Primitive {
	wasm_leb = 0x77,
	wasm_int32 = 0x7f,  // make sure to not confuse these with boxed Number nodes of kind longs, reals!
	wasm_f32 = 0x7d,
	wasm_int64 = 0x7E, // signed or unsigned? we don't care
	wasm_float64 = 0x7C,
//	floats = 0x1010, // only useful for main(), otherwise we can return real floats or wrapped Node[reals]
//	codepoint32 = int32, todo via map
//	pointer = int32,// 0xF0, // internal
////	node = int32, // NEEDS to be handled smartly, CAN't be differentiated from int32 now!
////	node = 0xA0,
//	angle = 0xA0,//  angle object pointer/offset versus smarti vs anyref
//	any = 0xA1,// Wildcard for function signatures, like haskell add :: a->a
////	unknown = any,
//	array = 0xAA,
//	charp = 0xC0, // vs
//	stringp = 0xC0,// use charp?  pointer? enough?? no!??
////	value = 0xA1,// wrapped node Value, used as parameter? extract and remove! / ignore
//	todoe = 0xFE, // todo
////	error_ = 0xE0, why propagate?
////	pointer = 0xF0,
////	externalPointer = 0xFE,
//	ignore = 0xAF, // truely internal, should not be exposed! e.g. Arg
////	smarti32 = 0xF3,// see smartType
////	smarti64 = 0xF6,

	byte_char = 0xB0, // when indexing byte array. todo: maybe codepoint into UTF8!?
	codepointer = 0xC0,  // when indexing int32 array
//	c_char = 0xB0, // when indexing byte array. todo: maybe codepoint into UTF8!?
	array_start = 0xA000, // careful there are different kinds of arrays/lists/List/Node[lists]
	list = 0xA100, // [len(int32), data*] compatible with List
	vector = 0xA200, // [len(leb), data…] as in wasm, should never be boxed, but converted to list, string, objects… upon parsing
	array_header = 0xA300, // [type, len, data*] or [type, len, data…] ? 0 termination for sanity (but data can be 0 too!)
//	compatible with List after shift, compatible with array_start after two shifts
	// TODO: array 0xA000 combinatorial with primitive types
	int_array = 0xA07f, // primitive, unboxed, no length!?
	long_array = 0xA07e,
	float_array = 0xA07d,
	real_array = 0xA07c,
//	longs_array= 0xA00E, todo: boxed Node{kind=longs}
//	reals_array= 0xA00C, todo: boxed Node{kind=reals}
//	byte_list = 0xB000, // not 0 terminated, length via 3rd return item??
//	byte_vector = 0xB001, // LEB encoded length header
//    block = 0xB000,
	c_string = 0xC000, // pointer to utf-8 bytes in wasm's linear memory, 0 terminated
	leb_string = 0xC001, // LEB encoded length header -> wasm_string
	utf16_string = 0xC016, // pointer to utf-16 bytes in wasm's linear memory, 0.0 terminated
	utf32_string = 0xC032, // pointer to utf-16 bytes in wasm's linear memory, 0.0 terminated
	json5_string = 0xC005,
	json_string = 0xC010,
	wasp_string = 0xC0D0, // serialized Node ≠ Node address
	wasp_data_string = 0xC0DA, // serialized Node ≠ Node address
	wasp_code_string = 0xC0DE, // charged data, see https://github.com/pannous/wasp/wiki/charged
//	maps, // our custom Map object todo: remove?

// 🔋 main type of all non-primitive objects
	nodes = 0xD000, // address of a Node in linear memory
//	data = 0xDADA, // address of a Node in linear memory
//	dada = 0xDADA, // address of a Node in linear memory

//	three3D = 0xDA3D, // binary encoded three dimensional object
	result_error = 0xE000, // different from well typed Nodes of kind 'error' and exceptions
	fointer = 0xF000, // see Addresses starting from 0x10000
//	⚠️ fointers are combinatorial:
	fointer_of_int32 = 0xF07F, // int* etc
//	fointer_of_int32 = 0xFA7F, // int[]* etc
//	ffointer = 0xFF00, // pointer pointer
//	ffointer_of_int32 = 0xFF7F, // int** etc
// for 32 bit smart pointers, as used in return of int main(){}

};


typedef int Address;
/*
 * i32 paged union Type
 * 0x00 - 0xFF zero page : Valtype / Kind
 * 0x100 - 0x1000 low page : reserved / Kind
 * 0x1000 - 0x10000 Todo Classes / pointer page ?
 * 0x10000 - string_header_32 pointer page: indirect types via address of describing Node{kind=clazz}
 * string_header_32 - 0xFFFFFFFF high page reserved (combinatorial types!) Todo e.g. person?[10] versus combinatorial primitive int[10]
 * header_64
 * check(sizeof(Type)==8) // otherwise all header structs fall apart
 * */
// for (wasm) function type signatures see Signature class!

// on 64bit systems pointers (to types)
union Type64 {//  i64 union, 8 bytes with special ranges:
    long long value = 0; // one of:
    SmartPointer64 smarty;
    Node *clazz;// same 64 bit on normal systems!!!!
//     0x10000000_00000000 - 2^64 : SmartPointer64 ≈ SmartPointer32 + 4 byte value
};

union Type {// 64 bit due to pointer! todo: i32 union, 4 bytes with special ranges:
    /* this union is partitioned in the int space:
     0x0000 - Ill defined
     0x0001 - 0x1000 : Kinds of Node MUST NOT CLASH with Valtype!?
     0x1000 - 0x10000: Classes (just the builtin ones) smarty32 " todo 150_000 classes should be enough for anyone ;)"
     // todo memory offset ok? (data 0x1000 …)
     0x10000 - … 2^60    : Addresses (including pointers to any Node including Node{type=clazz}
     //todo endianness?

     some Classes might be composing:
     0xA001 array of Type 1 ( objects )
    */

    // todo: this union is BAD because we can not READ it safely, instead we need mapType / extractors for all combinations!
    long value = 0; // one of:
//    SmartPointer64 smarty;
//    SmartPointer32 smarty;// when separating Types from values we don't need smart pointers
    Kind kind; // Node of this type
//	Valtype valtype; // doesn't make sense here but try to avoid(guarantee?) overlap with type enum for future compatibility?
    Primitive type;// c_string int_array long_array float_array etc, can also be type of value.data in boxed Node
//    Node *clazz;// same as // 64 bit on normal systems!!!!
    Address address;// pointer to Node
    Type(Kind kind) {
        this->kind = kind;
        if ((int) kind > 0x1000)error("erroneous or unsafe Type construction");
    }

	operator int() const { return this->value; }

	operator Kind() const { return this->kind; }

	operator chars() const { return typeName(this); }

	Type(Type *o) {
		value = o->value;
	}

	Type(Node *o) {
		if (!o)
			this->kind = Kind::nils;
//			this->type=Type(Valtype::voids);
//		if (o->kind)
//		if (Double==*o)
		this->kind = reals;
	}

	Type(const Node &o);

    Type() {
    }

    Type(int value) {
        this->value = value;
    }

    Type(Primitive primitive) {
        type = primitive;
    }

    bool operator==(Type other) {
        return value == other.value;
    }
//	Type(Valtype valtype){
//		value = valtype;// transparent subset equivalence
//	}
};


enum Modifiers {
	final,
	constant,
//	undefined,
	missing,
	unknowns,
	privates,
	protecteds,
	publics,
};

//#endif //MARK_NODETYPES_H

chars typeName(::Kind t);
chars typeName(::Type t);
