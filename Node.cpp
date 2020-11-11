//#pragma once
////
//// Created by pannous on 30.09.19.
////
//
#include "Node.h"

#ifndef WASM

#include <cstdarg>
#include <stdio.h>
#include <stdarg.h> // va_list

#endif

bool polish_notation = false;// f(a,b) => (f a b) also : lisp mode  a(1)(2)==a{1 2}
bool throwing = true;// otherwise fallover beautiful-soup style generous parsing

bool debug = true;
extern "C" double sqrt(double);


unsigned int *memory = (unsigned int *) 1024; // <?> memoryBase set in wasmx !?!?   todo how to not handtune _data_end?
unsigned int *current = memory;

unsigned long __stack_chk_guard = 0xBAAAAAAD;

void __stack_chk_guard_setup(void) { __stack_chk_guard = 0xBAAAAAAD;/*provide some magic numbers*/ }

void
__stack_chk_fail(void) { /*log("__stack_chk_fail");*/} //  Error message will be called when guard variable is corrupted


//#include <cmath>
//#include <tgmath.h> // sqrt macro
#include "String.h"
#include "NodeTypes.h"


void todo(chars error) {
	breakpoint_helper
	warn(str("TODO ") + error);
}

//
//String operator "" s(const char* c, size_t){
//return String(c);// "bla"s
//}

//new String();
//auto ws = {' ', '\t', '\r', '\n'};
Node Infinity = Node("Infinity");
Node NegInfinity = Node("-Infinity");
Node NaN = Node("NaN");
//NIL=0;
//Node NIL;
Node NIL = Node(nil_name).setType(nils).setValue(0);// non-existent
Node Unknown = Node("unknown").setType(nils).setValue(0); // maybe-existent
Node Undefined = Node("undefined").setType(nils).setValue(0); // maybe-existent, maybe error
Node Missing = Node("missing").setType(nils).setValue(0); // existent but absent

Node ERROR = Node("ERROR").setType(errors);// internal error ≠ Error class ≠ NIL
Node True = Node("True").setType(bools).setValue(true);
Node False = Node("False").setType(bools);

Node &Node::operator=(int i) {
	value.longy = i;
	kind = longs;
	if (name.empty() or name.isNumber())
		name = String(itoa(i));
	return *this;
}

Node &Node::operator=(chars c) {
	value.string = String(c);
	kind = strings;
	if (name == empty_name)name = value.string;
	return *this;
}

Node &Node::operator[](int i) {
	if (i >= length) {
		breakpoint_helper;
		err(String("out of range index[] ") + i + " >= length " + length);
		// todo: allow insertion of unknown indices? prefered method: not
	}
	return children[i];
}

Node &Nodec::operator[](int i) const {
	if (i >= length) {
		breakpoint_helper;
		err(String("out of range index[] ") + i + " >= length " + length);
	}
	return children[i];
}

Node &Node::operator[](String s) {
	Node *found = has(s);
	if (s[0] == '.') {
		s++;
		found = has(s);
	}
//	if (found and found->kind==keyNode and found->value.node)return *found->value.node;
// ^^ DON'T DO THAT! a:{b:{c:'hi'}} => a["b"] must be c, not "hi"
	if (found)return *found;
	if (name == s) {// me.me == me ? really? let's see if it's a stupid idea…
		if (kind == objects and value.node)
			return *value.node;
		return *this;
	}
	if (length == 1)
		if (children[0].has(s))return children[0][s];

	Node &neu = set(s, 0);// for n["a"]=b // todo: return DANGLING/NIL
	neu.kind = keyNode;//nils; // until ref is set! but neu never knows when its set!! :(
	neu.parent = this;
	if (neu.value.node) return *neu.value.node;
	else return neu;
}

Node &Nodec::operator[](String s) const {
	return (*this)[s];
}

// CAREFUL: iterate by value or by reference ?!
//for (Node &child : liste) { // DOES effect liste
//for (Node child : liste) { // Does NOT effect liste
Node *Node::begin() const {
	return children;
}

Node *Node::end() const {
	return children + length;
}

// non-modifying
Node Node::merge(Node &other) {
	if (other.isNil())return *this;
	if (other.length == 0) {
		return this->insert(other);
	}
	Node &neu = *clone();// non-modifying
	for (Node &item:other)
		neu.addRaw(item);
	return neu;
}// non-modifying


Node &Node::operator[](char c) {
	return (*this)[String(c)];
}

int capacity = 100;// todo !!! lol lists>100 elements;)
int maxNodes = 10000;
int lastChild = 0;


Node *all = static_cast<Node *>(calloc(sizeof(Node), capacity * maxNodes));


bool typesCompatible(Node &one, Node &other) {
	if (one.kind == other.kind)return true;
	if (one.kind == objects or one.kind == groups or one.kind == patterns or one.kind == expressions)
		return other.kind == objects or other.kind == groups or other.kind == patterns or other.kind == expressions;
	if (one.kind != keyNode and other.kind != keyNode) return false;
	return false;
}


// super wasteful, for debug
Node &Node::set(String string, Node *node) {
//	if (!children)children = static_cast<Node *>(alloc(capacity));

	if (!children) {
		children = &all[capacity * lastChild++];
		if (name == nil_name)name = object_name;
	}
	if (length >= capacity / 2)todo("GROW children");
//	children = static_cast<Node *>(alloc(1000));// copy old
	Node &entry = children[length];
	if (&entry == &NIL)
		error("IMPOSSIBLE");
	if (length > 0) {
		Node &current = children[length - 1];
//		current.next = &entry;// WE NEED TRIPLES cause objects can occur in many lists
//		entry.previous=current;
	}
	entry.parent = this;
	entry.name = string;
	if (!node) {
//		entry.value.node=&entry;// HACK to set reference to self!
		entry.kind = keyNode;
		entry.value.node = &children[capacity - length - 1];//  HACK to get key and value node dummy from children
//		 todo: reduce capacity per node
		entry.value.node->name = string;
		entry.value.node->kind = Type::unknown;
		entry.value.node->parent = &entry;
//		entry.value.node=Node();// dangling ref to be set
	} else {
		entry = *node;// copy by value OK
		entry.parent = this;
	}
	length++;
	return entry;
}

//Node::Node(const char *string) {
//	value.string = String(string);
//	type = strings;
//}

bool Node::operator==(String other) {
	if (this == 0)return other.empty();
	if (kind == objects or kind == keyNode)return *value.node == other or value.string == other;
	if (kind == keyNode) return other == name or value.node and *value.node == other;// todo: a=3 a=="a" ??? really?
	if (kind == longs) return other == itoa(value.longy);
	if (kind == reference) return other == name or value.node and *value.node == other;
	if (kind == unknown) return other == name;
	if (kind == operators) return other == name;
	return kind == strings and other == value.string;
}

bool Node::operator==(int other) {
//	if (this == 0)return false;// HOW?
	if ((kind == longs and value.longy == other) or (kind == reals and value.real == other))
		return true;
	if (kind == keyNode and value.node and *value.node == other)return true;
	if (kind == strings and atoi0(value.string) == other)return true;
	if (atoi0(this->name) == other)return true;
	if (kind == objects and length == 1)return last() == other;
//	if (type == objects)return value.node->numbere()==other;// WTF
	return false;
}

bool Node::operator==(long other) {
	if (kind == keyNode and value.node and value.node->value.longy == other)return true;
	return (kind == longs and value.longy == other) or (kind == reals and value.real == other) or
	       (kind == bools and value.longy == other);
}

bool Node::operator==(double other) {
	if (kind == keyNode and value.node and value.node->value.real == other)return true;
	return (kind == reals and value.real == ((float) other)) or
	       (kind == longs and value.longy == other);
}

bool Node::operator==(float other) {
	if (kind == keyNode and value.node and value.node->value.real == other)return true;
	return (kind == reals and value.real == other) or
	       (kind == longs and value.longy == other);
}

bool namesCompatible(Node a, Node b) {

}

// are {1,2} and (1,2) the same here? objects, params, groups, blocks
bool Node::operator==(Node &other) {
	if (this->kind == errors)return other.kind == errors;
	if (other.kind == errors)return this->kind == errors;

	if (this == &other)return true;// same pointer!

	if (kind == longs and other.kind == longs)
		return value.longy == other.value.longy;
	if (kind == longs and other.kind == reals)
		return value.longy == other.value.real;
	if (kind == reals and other.kind == reals)
		return value.real == other.value.real;
	if (kind == reals and other.kind == longs)
		return value.real == other.value.longy;

	if (isNil() and other.isNil())
		return true;
	if (isEmpty() and
	    other.isEmpty()) // todo: THIS IS NOT ENOUGH!!! "plus" symbol  a!=b ,  "false and false" != "and false"
		return true;
	if (name == NIL.name or name == False.name or name == "")
		if (other.name == NIL.name or other.name == False.name or other.name == "")
			return true;// TODO: SHOULD already BE SAME by engine!
	if (value.node == &other)return true;// same value enough?
	if (this == other.value.node)return true;// reference ~= its value

	if (kind == keyNode and this->value.node and *this->value.node == other)return true;// todo again?
	if (kind == nils and other.kind == longs)return other.value.longy == 0;
	if (other.kind == nils and kind == longs)return value.longy == 0;

	if (other.kind == unknown and name == other.name)
		return true; // weak criterum for dangling unknowns!! TODO ok??
	if (kind == bools or other.kind == bools) // 1 == true
		return value.longy == other.value.longy;// or (value.data!= nullptr and other.value.data != nullptr a);
	if (not typesCompatible(*this, other))
		return false;
//	CompileError: WebAssembly.Module(): Compiling function #53:"Node::operator==(Node&)" failed: expected 1 elements on the stack for fallthru to @3, found 0 @+5465
//	or (other != NIL and other != False) or

	if (kind == strings)
		return value.string == other.value.string or value.string == other.name;// !? match by name??


	if (length != other.length)
		return false;

#ifdef WASM
	todo("Node::operator==(Node &other) in WASM");
	return false;
#endif


	// if ... compare fields independent of type object {}, group [] ()
	for (int i = 0; i < length; i++) {
		Node &field = children[i];
		Node &val = other.children[i];
		if (field != val and !field.name.empty())
			val = other[field.name];
		if (field != val) {
			if ((field.kind != keyNode and field.kind != nils) or !field.value.node)
				return false;
			Node deep = *field.value.node;
			return deep == val;
		}
	}


	if (name == other.name)
		return true;
	return false;
}

//bool Node::operator!=(Node &other) {
//	return not(*this == other);
//}
//use of overloaded operator '!=' is ambiguous
//expected 1 elements on the stack for fallthru  BAD somehow!?
bool Node::operator!=(Node other) {
	return not(*this == other);
}

bool Node::operator>(Node other) {
	if (kind == strings and other.kind == strings) {
		return value.string > other.value.string;
	}
	if (kind == longs) {
		if (other.kind == longs)return value.longy > other.value.longy;
		if (other.kind == reals)return value.longy > other.value.real;
	}
	if (kind == reals) {
		if (other.kind == longs)return value.real > other.value.longy;
		if (other.kind == reals)return value.real > other.value.real;
	}
	if (other.kind == longs) {
		if (kind == longs)return value.longy > other.value.longy;
		if (kind == reals)return value.real > other.value.longy;
		if (kind == objects or kind == groups or kind == patterns)
			return length > other.value.longy;
	}
	if (other.kind == objects or other.kind == groups or other.kind == patterns) {
		if (kind == longs)return value.longy > other.length;
		if (kind == objects or kind == groups or kind == patterns)
			return length > other.value.longy;
	}
	if (!has("compare") and !has("greater") and !has("less"))
		error("Missing compare functions for objects %s > %s ?"s % name % other);
}

Node Node::operator+(Node other) {
	if (kind == strings and other.kind == longs)
		return Node(value.string + other.value.longy);
	if (kind == strings and other.kind == reals)
		return Node(value.string + other.value.real);
	if (kind == strings and other.kind == strings)
		return Node(value.string + other.value.string);
	if (kind == longs and other.kind == longs)
		return Node(value.longy + other.value.longy);
	if (kind == reals and other.kind == longs)
		return Node(value.real + other.value.longy);
	if (kind == longs and other.kind == reals)
		return Node(value.longy + other.value.real);
	if (kind == reals and other.kind == reals)
		return Node(value.real + other.value.real);
	if (kind == longs and other.kind == strings)
		return Node(value.longy + other.value.string);
//	if(type==floats and other.type==strings)
//		return Node(value.real + other.value.string);
	if (kind == objects)
		return this->merge(other);
	if (other.kind == objects)
		return other.insert(*this, 0);
	error(str("Operator + not supported for node types %s and %s") % typeName(kind) % typeName(other.kind));
	return ERROR;
};

void Node::remove(Node *node) {
	if (!children)return;// directly from pointer
	if (length == 0)return;
	if (node < children or node > children + length)return;
	for (long j = node - children; j < length - 1; j++) {
		children[j] = children[j + 1];
	}
	children[length - 1] = 0;
	length--;
}

void Node::remove(Node &node) {
	if (!children)return;
	for (int i = 0; i < length; i++) {
		if (children[i] == node) {
			for (int j = i; j < length; j++) {
				children[j] = children[j + 1];
			}
			length--;
		}
	}
}

void Node::addRaw(Node *node) {
	if (length >= capacity - 1)
		error("Out of node Memory");
	if (lastChild >= maxNodes)
		error("Out of global Memory");
	if (!children) children = &all[capacity * lastChild++];
	if (length > 0)
		children[length - 1].next = &children[length];
	node->parent = this;
	children[length++] = *node;
}

Node &Node::addRaw(Node &node) {
	if (length >= capacity - 1)
		error("Out of node Memory");
	if (lastChild >= maxNodes)
		error("Out of global Memory");
	if (!children) children = &all[capacity * lastChild++];
	if (length > 0)
		children[length - 1].next = &children[length];
	children[length++] = node;
	node.parent = this;
	return *this;
}

void Node::add(Node &node) {
	return add(&node);
}

void Node::add(Node *node, bool flatten) { // flatten AFTER construction!
	if (node->isNil() and node->name.empty() and node->kind != longs)
		return;// skipp nils!  (NIL) is unrepresentable and always ()! todo?
	node->parent = this;
	if (node->length == 1 and flatten and node->name.empty())
		node = &node->last();

	if (not children and (node->kind == objects or node->kind == groups or node->kind == patterns) and
	    node->name.empty()) {
		children = node->children;
		length = node->length;
		for (Node &child:*this)
			child.parent = this;
		if (kind != groups) kind = node->kind; // todo: keep kind if … ?
	} else {
		addRaw(node);
	}
// todo a{x}{y z} => a{x,{y z}} BAD
}

void Node::addSmart(Node node) {// merge?
	if (polish_notation and node.length > 0) {
		if (name.empty())
			name = node[0].name;
		else
			parent->addRaw(node);// REALLY?
//			todo("polish_notation how?");
		Node args = node.from(node[0]);
		addRaw(args);
		return;
	}
	// a{x:1} != a {x:1} but {x:1} becomes child of a
	// a{x:1} == a:{x:1} ?
	if (last().kind == operators) {
		addRaw(node);
		return;
	}

	if (last().kind == reference or last().kind == keyNode or name.empty() and
	    not kind == expressions)// last().kind==reference)
		last().add(&node);
	else
		add(&node);
}

//non-modifying
Node Node::insert(Node &node, int at) {
	if (length == 0)return node;//  todo: rescue value,name?
	while (at < 0)at = length + at;
	if (at >= length - 1) {
		Node *clon = this->clone();
		clon->addRaw(node);
		return *clon;
	}
	if (at == 0)return node + *this;
	if (at > 0)
		error("Not implemented: insert at offset");
	return ERROR;
}
//
//void Node::add(Node &node) {
//	add(&node);
//}

// like c++ here HIGHER up == lower number evaluated earlier , except 0 WTF
/*
1 	:: 	Scope resolution 	Left-to-right

2 	a++   a-- 	Suffix/postfix increment and decrement
type()   type{} 	Functional cast
a() 	Function call
a[] 	Subscript
.   -> 	Member access

3
++a   --a 	Prefix increment and decrement 	Right-to-left
+a   -a 	Unary plus and minus
!   ~ 	Logical NOT and bitwise NOT
new   new[] 	Dynamic memory allocation
delete   delete[] 	Dynamic memory deallocation
4 	.*   ->* 	Pointer-to-member 	Left-to-right
5 	a*b   a/b   a%b 	Multiplication, division, and remainder
6 	a+b   a-b 	Addition and subtraction
7 	<<   >> 	Bitwise left shift and right shift
8 	<=> 	Three-way comparison operator (since C++20)
9 	<   <= 	For relational operators < and ≤ respectively
>   >= 	For relational operators > and ≥ respectively
10 	==   != 	For relational operators = and ≠ respectively
11 	& 	Bitwise AND
12 	^ 	Bitwise XOR (exclusive or)
13 	| 	Bitwise OR (inclusive or)
14 	&& 	Logical AND
15 	|| 	Logical OR
16 	a?b:c 	Ternary conditional[note 2] 	Right-to-left
throw 	throw operator
co_yield 	yield-expression (C++20)
= 	Direct assignment (provided by default for C++ classes)
+=   -= 	Compound assignment by sum and difference
*=   /=   %= 	Compound assignment by product, quotient, and remainder
<<=   >>= 	Compound assignment by bitwise left shift and right shift
&=   ^=   |= 	Compound assignment by bitwise AND, XOR, and OR
17 	, 	Comma 	Left-to-right
	 */

//bool leftAssociativity(Node &operater) {
//	return false;
////	https://icarus.cs.weber.edu/~dab/cs1410/textbook/2.Core/operators.html
//}

// Node* OK? else Node&
Node *Node::has(String s, bool searchMeta) const {
	if ((kind == objects or kind == keyNode) and value.node and s == value.node->name)
		return value.node;
	if (!children)return 0;
	for (int i = 0; i < length; i++) {
		Node &entry = children[i];
		if (s == entry.name)
			if ((entry.kind == keyNode or entry.kind == nils) and entry.value.node)
				return entry.value.node;
			else // danger overwrite a["b"]=c => a["b"].name == "c":
				return &entry;
	}
	if (s == name.data)
		return const_cast<Node *>(this);
	if (meta and searchMeta)
		return meta->has(s);
	return 0;// NIL
}

Node &Node::last() {
	return length > 0 ? children[length - 1] : *this;
}

bool Node::empty() {// nil!
	return isEmpty();
}

bool Node::isEmpty() {// not required here: name.empty()
	return (length == 0 and value.longy == 0) or isNil();
}

bool Node::isNil() { // required here: name.empty()
	return this == &NIL or kind == nils or
	       ((kind == keyNode or kind == unknown or name.empty()) and length == 0 and value.data == nullptr);
}

const char *Node::serializeValue() const {
	String wasp = "";
	Value val = value;
	switch (kind) {
		case strings:
			return "'"s + val.string + "'";
//		case ints:
		case longs:
			return itoa(val.longy);
		case reals:
			return ftoa(val.real);
		case nils:
			return "ø";
		case objects:
			return object_name;
		case groups:
			return groups_name;
		case patterns:
			return patterns_name;
		case keyNode:
			return val.node ? val.node->name : "";// val.node->serialize();
		case reference:
			return val.data ? val.node->name : "ø";
		case symbol:
			return val.string;
		case bools:
			return val.longy > 0 ? "true" : "false";
		case arrays:
			return "[…]";//val.data type?
		case buffers:
			return "int[]";//val.data lenght?
		case function:
			return "{…}";
		case operators:
		case expressions:
		case unknown:
			return "?";
		default:
			breakpoint_helper
			error("MISSING CASE");
			return "MISSING CASE";
	}
}

const char *Node::serialize() const {
	String wasp = "";
	if (not polish_notation or this->length == 0) {
		if (not this->name.empty()) wasp += this->name;
		const char *serializedValue = serializeValue();
		if (this->value.data and !eq(name, serializedValue) and !eq(serializedValue, "{…}") and
		    !eq(serializedValue, "?")) {
			wasp += ":";
			wasp += serializedValue;
			wasp += " ";
		}
	}
	if (this->length > 0) {
		wasp += (this->kind == groups ? "(" : "{");
		if (polish_notation and not this->name.empty())wasp += this->name;
		for (Node &node : *this) {
			wasp += " ";
			wasp += node.serialize();
		}
		wasp += (this->kind == groups ? " )" : " }");
	}
	return wasp;
//	return this->name.empty()? this->string() : this->name;
//	return node.name.empty()? node.string() : node.name;
}

const char *Node::toString() {
	return this->serialize();
}

const char *Node::toString() const {
	return this->serialize();
}

String toString(Node &node) {
	return node.serialize();
}

void Node::print() {
	printf(this->serialize());
}

Node &Node::setValue(Value v) {
	value = v;
	return *this;
}


// rest of node children split
Node Node::from(Node match) {
	return from(match.name);
}

Node Node::from(String match) {
	Node lhs;
	bool start = false;
	for (Node child:*this) {
		if (start)lhs.addRaw(&child);
		if (child.name == match)start = true;
	}
	if (lhs.length == 0)
		for (Node child:*this)
			if (child.name == match)return child.values();
	if (kind != function)
		lhs.kind = kind;
	return lhs.flat();
}

Node Node::to(String match) {
	Node rhs;
	for (Node child:*this) {
		if (child.name == match)
			break;
		rhs.addRaw(&child);
	}
//	if(rhs.length==0)// no match
//		return *this;
	if (kind != function)
		rhs.kind = kind;
	return rhs.flat();
}

Node Node::to(Node match) {
	return to(match.name);
}

//	Node& flatten(Node &current){
Node &Node::flat() {
	if (kind == function)return *this->clone();
	if (length == 0 and kind == keyNode and name.empty() and value.node)return *value.node;
	if (length == 1 and value.node == &children[0])// todo remove redundancy
		return *value.node;
	if (length == 1 and not value.data and name.empty()) {
		children[0].parent = parent;
		return children[0].flat();
	}
	return *this->clone();
}

Node &Node::setName(char *name0) {
	name = name0;
	return *this;
}

// extract value from this (remove name)
Node Node::values() {
	if (kind == longs)return Node(value.longy);
	if (kind == reals)return Node(value.real);
	if (kind == strings)return Node(value.string);
	if (kind == bools)return value.data ? True : False;
	if (kind == keyNode)return *value.node;
	Node &val = clone()->setName("");
//	val.children = 0;
	return val;
}

void log(Node &n) {
	n.log();
}


void log(Node *n0) {
	if (!n0)return;
	Node n = *n0;
	log(n);
}


void printf(Node &) {
	log("void printf(Node&);");
}