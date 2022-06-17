//
// Created by pannous on 19.12.19.
//

#ifndef MARK_NODETYPES_H
#define MARK_NODETYPES_H

union Type;

chars typeName(const Type *t);


//#include "Util.h" // for error() :(
//#include "Code.h"

// Todo: can Type itself become a Node, making the distinction between type and kind superfluous?
enum Kind {// todo: merge Node.kind with Node.class(?)
	// plurals because of namespace clash
	// TODO add subtypes of Class:Node Variable:Node etc ^^^
	undefined = 0, // ≠ any?
	unknown = 1, //
	nils = 0x10, // ≠ undefined
	objects, // {…} block data with children closures
	groups, // (…) meta params parameter attributes lists
	patterns, // […] selectors matches, annotations! [public export extern] function mul(x,y){x*y}
	key, // key with value
	reference, // variable identifier name x
	symbol, // one / plus / Jesus
	operators, // TODO: semantic types don't belong here! they interfere with internal structural types key etc!!
	functor, // while(xyz){abc} takes 1?/2/3 blocks if {condition} {then} {else}
	expression, // one plus one
	declaration, // x:=1
	assignment, // x = 1 // really?? needs own TYPE?
	strings,
	codepoints,
	buffers, // int[]
	reals,
	longs, // the signature of parameters/variables is independent!
	//	ints, // use longy field, but in wasm longs are pointers!
	bools,
	errors, // internal wasp error, NOT to be used in Angle!
	call,
	clazz,

	// each class can be made into a typed list / vector int => int[] :, // as Java class,  primitive int vs Node(kind==int) == boxed Int.
	//	vectors     use kind=array type=Any
//	vectors, // any[] vs Node[]  list of type class!  e.g.  numbers xs => xs.type=number(type=vectors)
	arrays, // Node[] vs any[]  untyped array of Nodes. Unlike vector the node is NOT a class! todo: see Classes
};// Type =>  must use 'enum' tag to refer to type 'Type' NAH!


// Todo these should NOT appear as Node.kind!
// unboxed primitive raw data (list is compatible with List though!)
enum Primitive {
	integer = 0x7f,  // todo: merge with Valtype.int32 = 0x7f
	byte_char = 0xB0, // when indexing byte array. todo: maybe codepoint into UTF8!?
	floats = 0x1010, // only useful for main(), otherwise we can return real floats or wrapped Node[reals]
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
	array_header_32 = 0x40000000,
	string_header_32 = 0x10000000,
	smart_mask_32 = 0x70000000,
	negative_mask_32 = 0x80000000,
	array_header_64 = 0x0040000000000000,
	string_header_64 = 0x0010000000000000,
};

typedef int Address;

/*
 * i32 paged union Type
 * 0x00 - 0xFF zero page : Valtype / Kind
 * 0x100 - 0x1000 low page : reserved / Kind
 * 0x1000 - 0x10000 Todo Classes / pointer page ?
 * 0x10000 - string_header_32 pointer page: indirect types via address of describing Node{kind=clazz}
 * string_header_32 - 0xFFFFFFFF high page reserved (combinatorial types!) Todo e.g. person?[10] versus combinatorial primitive int[10]
 * */
union Type {
	// ⚠️ Kind is short, not int!
	int value = 0; // one of:
	Kind kind; // Node of this type
	Primitive type;// c_string int_array long_array float_array etc, can also be type of value.data in boxed Node
//	Valtype valtype; // doesn't make sense here but try to avoid overlap with type enum for future compatibility?
	Node *clazz;// same as
	Address address;// pointer to Node
	/* this union is partitioned in the int space:
	 0x0000 - Ill defined
	 0x0001 - 0x1000 : Kinds of Node
	 0x1000 - 0x10000: Classes
	 0x10000 - …     : Addresses
	 some Classes might be composing:
	 0xA001 array of Type 1 ( objects )
	*/
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

	Type() {
	}

	Type(int value) {
		this->value = value;
	}

	Type(Primitive primitive) {
		type = primitive;
	}
};


struct Array_header {
	Type typ;
	int len;
	std::byte data;// ⚠️ data… continuation in memory, not pointer to different data!
//	void* data;// ⚠️ data… continuation in memory, not pointer to different data!
};

// allows boxed return of bigint via Address to longy Node …
struct Reto {
	int value;
	Type type;
};

struct Retol {
	long value;
	Type type;
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

#endif //MARK_NODETYPES_H

chars typeName(::Kind t);
chars typeName(::Type t);
