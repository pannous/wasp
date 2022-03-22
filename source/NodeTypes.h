//
// Created by pannous on 19.12.19.
//

#ifndef MARK_NODETYPES_H
#define MARK_NODETYPES_H
// Todo: can Type itself become a Node, making the distinction between type and kind superfluous?
enum Type {// todo: merge Node.kind with Node.class(?)
	// plurals because of namespace clash
	// TODO add subtypes of Class:Node Variable:Node etc ^^^
	nils = 0,
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

	// each class can be made into a typed list / vector int => int[] :
	classe, // as Java class,  primitive int vs Node(kind==int) == boxed Int.
	//	vectors     use kind=array type=Any
//	vectors, // any[] vs Node[]  list of type class!  e.g.  numbers xs => xs.type=number(type=vectors)
	arrays, // Node[] vs any[]  untyped array of Nodes. Unlike vector the node is NOT a class!

	unknown //7
};// Type =>  must use 'enum' tag to refer to type 'Type' NAH!

#endif //MARK_NODETYPES_H

