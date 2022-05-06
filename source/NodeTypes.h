//
// Created by pannous on 19.12.19.
//

#ifndef MARK_NODETYPES_H
#define MARK_NODETYPES_H

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
	arrays, // Node[] vs any[]  untyped array of Nodes. Unlike vector the node is NOT a class!

};// Type =>  must use 'enum' tag to refer to type 'Type' NAH!

//
enum Classes {
	integer = 0x1000,  //
	floats = 0x1010, // only useful for main(), otherwise we can return real floats or wrapped Node[reals]
	array_start = 0xA000, // careful there are different kinds of arrays/lists/List/Node[lists]
	byte_list = 0xB000, // not 0 terminated, length via 3rd return item??
	byte_vector = 0xB001, // LEB encoded length header
	c_string = 0xC000, // pointer to utf-8 bytes in wasm's linear memory, 0 terminated
	leb_string = 0xC001, // LEB encoded length header -> wasm_string
	json_string = 0xC010,
	wasp_string = 0xC0D0, // serialized Node ≠ Node address
	wasp_data_string = 0xC0DA, // serialized Node ≠ Node address
	wasp_code_string = 0xC0DE, // charged data, see https://github.com/pannous/wasp/wiki/charged
	node = 0xD000, // address of a Node in linear memory
//	three3D = 0xDA3D, // binary encoded three dimensional object
	result_error = 0xE000, // different from well typed Nodes of kind 'error'
//	fointer = 0xF000, see Addresses starting from 0x10000
//	maps, // our custom Map object todo: remove?
};

typedef int Address;

union Type {
	int value; // one of:
	Kind kind; // Node of this type
//	Valtype valtype; // doesn't make sense here but try to avoid overlap with type enum for future compatibility?
	Classes classe;// c_string etc
	Node *type;// same as
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
		if (kind > 0x1000)error("erroneous or unsafe Type construction");
	}

	operator Kind() const { return this->kind; }
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