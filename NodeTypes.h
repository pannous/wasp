//
// Created by me on 19.12.19.
//

#ifndef MARK_NODETYPES_H
#define MARK_NODETYPES_H

enum Type {
// plurals because of namespace clash
	// TODO add subtypes of Class:Node Variable:Node etc ^^^
			nils = 0,
	objects, // {…} children
	groups, // (…) meta parameter attributes lists
	patterns, // […] selectors matches, annotations! [public export extern] function mul(x,y){x*y}
	keyNode, // key with value
	reference,// variable identifier name x
	symbol,// one / plus / Jesus
	operators,// or just symbol?
	expression,// one plus one
	strings,
	arrays,// same as:
	buffers,
	floats,
	longs,
	ints,
	bools,
	unknown = 20 //7
};

#endif //MARK_NODETYPES_H
