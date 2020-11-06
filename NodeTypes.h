//
// Created by pannous on 19.12.19.
//

#ifndef MARK_NODETYPES_H
#define MARK_NODETYPES_H

enum Type {
// plurals because of namespace clash
	// TODO add subtypes of Class:Node Variable:Node etc ^^^
	nils = 0,
	objects, // {…} block data with children closures
	groups, // (…) meta params parameter attributes lists
	patterns, // […] selectors matches, annotations! [public export extern] function mul(x,y){x*y}
	keyNode, // key with value
	reference,// variable identifier name x
	symbol,// one / plus / Jesus
	operators,// or just symbol?
	expressions,// one plus one
	strings,
	arrays,// Node[] vs any[]? vs
	buffers,// int[]
	floats,
	longs, // the signature of parameters/variables is independent!
//	ints,// use longy field, but in wasm longs are pointers!
	bools,
	errors,// internal wasp error, NOT to be used in Angle!
	function,
	unknown = 20 //7
};// Type =>  must use 'enum' tag to refer to type 'Type' NAH!

#endif //MARK_NODETYPES_H
