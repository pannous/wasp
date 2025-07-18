//#pragma once
////
//// Created by pannous on 30.09.19.
////

#include "Node.h"
#include "List.h"
#include "Code.h"


#include <cstdarg> // va_list OK in WASM!!
#include <cstdlib> // OK in WASM!

#ifndef PURE_WASM

#include <cstdio>

#endif


//#include <cmath>
//#include <tgmath.h> // sqrt macro
#include "String.h"
#include "NodeTypes.h"
#include "Util.h"


//void todo(chars error) {
//	breakpoint_helper
//#ifdef DEBUG
//	error("TODO "s + error);
//#else
//	warn(str("TODO ") + error);
//#endif
//}

//
//String operator "" s(chars c, size_t){
//return String(c);// "bla"s
//}

//new String();
//auto &ws = {' ', '\t', '\r', '\n'};
//NIL=0;
//Node NIL;
//const Node NIL = Node(nil_name).setType(nils).setValue(0);// non-existent. NOT a value, but a keyword!
//const Node Unknown = Node("unknown").setType(nils).setValue(0); // maybe-existent
//const Node Undefined = Node("undefined").setType(nils).setValue(0); // maybe-existent, maybe error
//const Node Missing = Node("missing").setType(nils).setValue(0); // existent but absent. NOT a value, but a keyword!
//
//const Node ERROR = Node("ERROR").setType(errors);// internal error ≠ Error class ≠ NIL
//const Node True = Node("True").setType(bools).setValue(true);
//const Node False = Node("False").setType(bools);

//const Node Infinity = Node("Infinity");
//const Node NegInfinity = Node("-Infinity");
//const Node NaN = Node("NaN");

// must never be used in non-const references!
const Node NIL = Node(new String(nil_name, false)).setKind(nils).setValue(0); // non-existent. NOT a value, but a keyword!
Node Unknown = Node("unknown").setKind(nils).setValue(0); // maybe-existent
Node Undefined = Node("undefined").setKind(nils).setValue(0); // maybe-existent, maybe error
Node Missing = Node("missing").setKind(nils).setValue(0); // existent but absent. NOT a value, but a keyword!
//
Node ERROR = Node("ERROR").setKind(errors); // internal error ≠ Error class ≠ NIL
Node True = Node("True").setKind(bools).setValue(true);
Node False = Node("False").setKind(bools).setValue(false);
Node Ignore = Node("Ignore");
Node Infinity = Node("Infinity");
Node NegInfinity = Node("-Infinity");
Node Nan = Node("NaN");


Node *reconstructArray(int *array_struct);

extern "C" void __wasm_call_ctors();

void initSymbols() {
    print("initSymbols");
    ((Node&) NIL).name = nil_name;
#if WASM
		if (True.kind == bools)
			error("Wasm DOES init symbols!?");
		__wasm_call_ctors();
#endif
}

Node &Node::operator=(int i) {
    value.longy = i;
    kind = longs;
    if (name.empty() or name.isNumber())
        name = String(formatLong(i));
    return *this;
}

Node &Node::operator=(chars c) {
    value.string = new String(c);
    kind = strings;
    if (name == EMPTY)name = *value.string;
    return *this;
}

// getter only / can't set unknown fields
Node &Node::operator[](int i) {
    if (i >= length) {
        error(String("out of range index[] ") + i + " >= length " + length);
    }
    // todo: allow insertion of unknown indices? prefered method: not
    return children[i];
}

Node &Nodec::operator[](int i) const {
    if (i >= length)
        error(String("out of range index[] ") + i + " >= length " + length);
    return children[i];
}

Node &Node::operator[](String *s) {
    if (!s)return *new Node(); // NIL.copy();
    return operator[](s->data);
}

//Node &Node::operator[](String s) {
Node &Node::operator[](chars s) {
    if (s[0] == '@') {
        return metas()[s + 1];
    }
    Node *found = has(s);
    if (s[0] == '.') {
        s++;
        found = has(s);
    }
    //	if (found and found->kind==key and found->value.node)return *found->value.node;
    // ^^ DON'T DO THAT! a:{b:{c:'hi'}} => a["b"] must be c, not "hi"
    if (found)return *found;
    if (name == s) {
        // me.me == me ? really? let's see if it's a stupid idea…
        if (kind == key and value.node)
            return *value.node;
        return (Node &) *this;
    }
    if (length == 1)
        if (children[0].has(s))return children[0][s];
    if (meta and meta->has(s)) {
        return (*meta)[s];
    }
    Node &neu = set(s, 0); // for n["a"]=b // todo: return DANGLING/NIL
    neu.kind = key; //nils; // until ref is set! but neu never knows when its set!! :(
    neu.parent = (Node *) this;
    neu.length = 0;
    neu.children = 0;
    if (neu.value.node)
        return *neu.value.node;
    else return neu;
}

//Node &Nodec::operator[](String s) const {
//	return (*this)[s];
//}

// CAREFUL: iterate by value or by reference ?!
//for (Node &child : liste) { // DOES effect liste
//for (Node child : liste) { // Does NOT effect liste
Node *Node::begin() const {
    if (length <= 0 or !children)return 0;
    return children;
}

Node *Node::end() const {
    if (length <= 0 or !children) return 0;
    if (length < capacity);
    else {
        puti(length);
        puti(capacity);
    }
    check_silent(length < capacity);
    return children + length;
}

// non-modifying
Node &Node::merge(Node &other) {
    if (kind == objects or kind == groups)
        if (length == 0)return other; // ()+x == x
    if (other.isNil()) {
        return *this;
    }
    Node &neu = *clone(); // non-modifying
    if (other.length == 0) {
        neu.add(other);
    } // else
    for (Node &item: other) {
        neu.add(item);
    }
    return neu;
} // non-modifying
Node &Node::merge(Node *other) {
    return merge(*other);
}

Node &Node::operator[](char c) {
    return (*this)[String(c)];
}


//Node *all = 0;// = (Node *)calloc capacity, (sizeof(Node * maxNodes);


bool typesCompatible(Node &one, Node &other) {
    if (one.kind == other.kind)return true;
    if (one.kind == objects or one.kind == groups or one.kind == patterns or one.kind == expression)
        return other.kind == objects or other.kind == groups or other.kind == patterns or other.kind == expression or
               other.kind == unknown;
    if (one.kind != key and other.kind != key) return false;
    return false;
}


// super wasteful, for debug
Node &Node::set(String string, Node *node) {
    //	if (!children)children = static_cast<Node *>(alloc(capacity));
    if (capacity == 0)capacity = NODE_DEFAULT_CAPACITY; // how ;) copy constructor bug? todo
    if (!children) {
        init_children();
        //        if (name == nil_name)name = "ø";
    }
    if (length >= capacity / 2) todo("GROW children");
    //	children = static_cast<Node *>(alloc(1000));// copy old
    Node &entry = children[length];
    if (length > 0) {
        //		Node &current = children[length - 1];
        //		current.next = &entry;// WE NEED TRIPLES cause objects can occur in many lists
        //		entry.previous=current;
    }
    entry.parent = this;
    entry.name = string;
    if (!node) {
        //		entry.value.node=&entry;// HACK to set reference to self!
        entry.kind = key;
        entry.value.node = &children[capacity - length - 1]; //  HACK to get key and value node dummy from children
        //		 todo: reduce capacity per node
        entry.value.node->name = string;
        entry.value.node->kind = Kind::unknown;
        entry.value.node->parent = &entry;
        //		entry.value.node=Node();// dangling ref to be set
    } else {
        entry = *node; // copy by value OK
        entry.parent = this;
    }
    if (length > 0)
        children[length - 1].next = &entry;
    length++;
    return entry;
}

//Node::Node(chars string) {
//	value.string = &String(string);
//	type = strings;
//}

bool Node::operator==(String other) {
    //	return (*this == other.data); // todo unify/simplify
    //	if (this == 0)return other.empty();
    //	if (kind == objects or kind == key)objects={…} NOT have value!  return *value.node == other or value.string == other;
    if (kind == key) return other == name or (value.node and *value.node == other); // todo: a=3 a=="a" ??? really?
    if (kind == longs)
        return other == formatLong(value.longy); // "3" == 3   php style ARE YOU SURE? ;) only if otherwise consistent!
    if (kind == reals) return other == ftoa(value.real); // parseFloat(other)==value.real

    if (kind == reference) return other == name or (value.node and *value.node == other);
    if (kind == unknown) return other == name;
    if (kind == operators) return other == name;
    if (kind == strings)
        return other == value.string;
    return false;
}

bool Node::operator==(Node *other) {
    if (other == 0)return isNil();
    return *this == *other;
}

bool Node::operator==(bool other) {
    return other == operator bool();
}

bool Node::operator==(char other) {
    if (kind == codepoint1)return (codepoint) value.longy == other;
    return kind == strings and *value.string == other;
}

bool Node::operator==(codepoint other) {
    if (kind == codepoint1)return (codepoint) value.longy == other;
    return kind == strings and *value.string == other;
}

bool Node::operator==(chars other) {
    if (kind == strings and value.data)
        if (eq(value.string->data, other, value.string->shared_reference ? value.string->length : -1)) return true;
    if (eq(name.data, other, name.shared_reference ? name.length : -1))return true; // todo really name==other?
    if (kind == key and value.node and *value.node == other)return true;
    return false;
}

bool Node::operator==(wchar_t other) {
    if (kind == longs)return other == value.longy;
    if (kind == codepoint1)return other == value.longy;
    if (kind == strings)return other == value.string->codepointAt(0);
    error("operator== on wrong kind");
    // return name == other;
    // return false;
}

bool Node::operator==(int other) {
    //	if (this == 0)return false;// HOW?
    if (kind == longs and value.longy == other) return true;
    if (kind == reals and value.real == other)return true;
    if (kind == reals and (int) value.real == other)return true; // rounding equivalence only in tests!
    if (kind == reals)return similar(value.real, other);
    if (kind == bools)return other == value.longy;
    if (kind == codepoint1)return other == value.longy; // permissive here only!
    if (kind == key and value.node and *value.node == other)return true;
    if (kind == strings and parseLong(value.string->data) == other)return true;
    if (parseLong(name) == other)return true;
    if (length == 1 and (isGroup(kind)))
        return last() == other;
    //	if (type == objects)return value.node->numbere()==other;// WTF
    return false;
}

bool Node::operator==(int64 other) {
    if ((kind == strings or kind == unknown) and name.length == 0 and length == 0)return other == 0; // ""==0==ø
    if (kind == key and value.node and value.node->value.longy == other)return true;
    return (kind == longs and value.longy == other) or (kind == reals and value.real == other) or
           (kind == bools and value.longy == other);
}

bool Node::operator==(double other) {
    if (kind == key and value.node and value.node->value.real == other)return true;
    return (kind == reals and similar(value.real, other)) or
           //			(kind == reals and (float )value.real == (float)other) or // lost precision
           (kind == longs and value.longy == other) or
           (kind == longs and value.longy == (int64) other) or // rounding ok?
           (kind == bools and value.longy == other);
}

bool Node::operator==(float other) {
    if (kind == key and value.node and value.node->value.real == other)return true;
    return (kind == reals and value.real == other) or
           (kind == longs and value.longy == other);
}

bool Node::operator==(const Node &other) {
    return *this == (Node) other;
}

void debugNode(Node &n) {
    print("Node.name");
    print(n.name);
    print("Node.kind");
    print(typeName(n.kind));
    print((int) n.kind);
    print("Node.length");
    print(n.length);
    print("Node.value.longy");
    print(n.value.longy);
    print("Node.children");
    print(n.children);
    print("Node.isNil");
    print(n.isNil());
    print("Node == Nil");
    print(n == NIL);
    print("Node == ERROR");
    print(n == ERROR);
    print("Node.serialize");
    print(n.serialize());
}

//bool Node::operator===(const Node &other) {
//	return hash() == (Node) other.hash();
//}
// are {1,2} and (1,2) the same here? objects, params, groups, blocks
bool Node::operator==(Node &other) {
    if (kind == clazz or other.kind == clazz)
        return name.lower() == other.name.lower();
    //	other = other.flat();// todo this.flat() too!
    if (kind == errors)return other.kind == errors;
    if (other.kind == errors)return kind == errors;

    if (this == &other)return true; // same pointer!

    if (kind == longs and other.kind == longs)
        return value.longy == other.value.longy;
    if (kind == longs and other.kind == reals)
        return value.longy == other.value.real or value.longy == (int64) other.value.real; // rounding!
    if (kind == reals and other.kind == reals)
        return value.real == other.value.real or similar(value.real, other.value.real);
    if (kind == reals and other.kind == longs)
        return (int64) value.real == other.value.longy or similar(value.real, other.value.longy);;
    if (kind == bools or other.kind == bools) // 1 == true
        return value.longy == other.value.longy; // or (value.data!= nullptr and other.value.data != nullptr a);
    //	if (kind.type == int_array and other.kind.type == int_array)
    //		return value.real == other.value.longy;

    //	auto &a1 = isNil();
    //	auto &a2 = other.isNil();

    if (name == NIL.name.data or name == False.name.data or name == "")
        if (other.name == NIL.name.data or other.name == False.name.data or other.name == "") {
            trace("NILS!");
            return true; // TODO: SHOULD already BE SAME by engine!
        }
    // todo ^^==::
    if (isNil() and other.isNil()) {
        trace("NILS!");
        return true;
    }

    if (kind != strings and other.kind != strings and isEmpty() and other.isEmpty())
        // todo: THIS IS NOT ENOUGH!!! "plus" symbol  a!=b ,  "false and false" != "and false"
        return true;
    if (kind != operators and value.node == &other)
        return true; // todo when is same value EVER enough??
    if (kind != operators and other.value.node and other.kind == key and this == other.value.node) {
        //        println("SELF REFERENCE");
        //        debugNode(*other.value.node);
        return true; // reference ~= its value
    }
    if (kind == key and value.node and *value.node == other)return true; // todo again?
    if (kind == nils and other.kind == longs)return other.value.longy == 0;
    if (other.kind == nils and kind == longs)return value.longy == 0;

    if (other.kind == unknown and name == other.name)
        return true; // weak criterum for dangling unknowns!! TODO ok??

    if ((isGroup(kind)) and length == 1 and children[0] == other)
        return true; // (x)==x

    /*Node flattened=this->flat();// too expensive? but we want (x)=x ! other way: if kind==group and length==1
    if(flattened.hash()!=hash() and flattened==other)
        return true;*/


    if (kind == strings) {
        if (other.kind == codepoint1)
            return *value.string == (codepoint) other.value.longy;
        if (other.kind == strings)
            return *value.string == *other.value.string or *value.string == other.name or
                   name == other.value.string; // !? match by name??
        return false; // no string->int etc conversion in angle!
    }

    if (not typesCompatible(*this, other))
        return false;


    if (length != other.length)
        return false;


    // if ... compare fields independent of type object {}, group [] ()
    for (int i = 0; i < length; i++) {
        Node field = children[i].flat();
        Node val = other.children[i].flat();
        if (field != val and !field.name.empty()) {
            auto otherField = other.has(field.name);
            if (!otherField)return false;
            else val = *otherField;
        }
        if (field != val) {
            if ((field.kind != key and field.kind != nils) or !field.value.node) {
                trace("CHILD MISMATCH");
                return false;
            }
            Node deep = *field.value.node;
            if (deep != val) {
                trace("CHILD MISMATCH");
                return false;
            }
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
        if (isGroup(kind))
            return length > other.value.longy;
    }
    if (isGroup(other.kind)) {
        if (kind == longs)return value.longy > other.length;
        if (isGroup(kind))
            return length > other.value.longy;
    }
    if (!has("compare") and !has("greater") and !has("less"))
        error("Missing compare functions for objects %s > %s ?"s % name % other);
    return false;
}

// non-modifying
Node Node::operator+(Node other) {
    if (kind == strings and other.kind == longs)
        return Node(*value.string + other.value.longy);
    if (kind == strings and other.kind == reals)
        return Node(*value.string + other.value.real);
    if (kind == strings and other.kind == strings)
        return Node(*value.string + other.value.string);
    if (kind == longs and other.kind == longs)
        return Node(value.longy + other.value.longy);
    if (kind == reals and other.kind == longs)
        return Node(value.real + other.value.longy);
    if (kind == longs and other.kind == reals)
        return Node(value.longy + other.value.real);
    if (kind == reals and other.kind == reals)
        return Node(value.real + other.value.real);
    if (kind == longs and other.kind == strings)
        return Node(String(value.longy) + other.value.string);
    //	if(type==floats and other.type==strings)
    //		return Node(value.real + other.value.string);
    if (kind == unknown or kind == objects or kind == groups /*or kind == patterns*/)
        return merge(other);
    if (other.kind == objects)
        return other.insert(*this, 0);
    error("Operator + not supported for node types");
    // error("Operator + not supported for node types %s and %s"s % typeName(kind) % typeName(other.kind));
    return ERROR;
};

void Node::remove(Node *node) {
    if (!children)return; // directly from pointer
    if (length == 0)return;
    if (node < children or node > children + length)return;
    for (int64 j = node - children; j < length - 1; j++) {
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

Node &Node::add(const Node *node) {
    if ((int64) node > MAX_MEM)
        error("node Out of Memory");
    if (!node)return *this;
    if (this == &NIL) {
        warn("adding to NIL!");
        return *this;
    }
    if (node->kind == groups and node->length == 0 and node->name.empty())
        return *this;
    if (kind == longs or kind == reals)
        warn("can't modify primitives, only their referenceIndices"); // e.g.  a=7 a.nice=yes
    if (capacity == 0)capacity = NODE_DEFAULT_CAPACITY; // how ;) copy constructor bug? todo
    if (length >= capacity - 1) {
        warn("Out of node capacity "s + capacity + " in " + name);
        capacity *= 2;
        Node *new_children = new Node[capacity];  // Use new[] to call constructors
        // Copy existing children using assignment operator
        for (int i = 0; i < length; i++) {
            new_children[i] = children[i];
        }
        children = new_children;
    }
    if (lastChild >= MAX_NODE_CAPACITY)
        error("Out of global Node memory");
    init_children();
    if (length > 0)
        children[length - 1].next = &children[length];
    ((Node *) node)->parent = this; // not const lol. allow to set and ignore NIL.parent
    children[length] = *node; // invokes memcpy, so add(Node(1)) with stack value is ok!
    length++;
    return *this;
}

Node &Node::add(const Node &node) {
    //	if (&node == 0)return *this;
    return add(&node);
}

void Node::addSmart(Node node) {
    // merge?
    addSmart(&node, false, true);
}

void Node::addSmart(Node *node, bool flatten, bool clutch) {
    // flatten AFTER construction!
    // todo cleanup clutch LOL!, and or merge with flat()
    if (!node)return;
    if (this == &NIL) {
        warn("adding smart to NIL!");
        return;
    }
    if (node->kind == generics and kind == groups)
        kind = generics;
    if (use_polish_notation and node->length > 0) {
        if (name.empty())
            name = (*node)[0].name;
        else
            parent->add(node); // REALLY?
        Node args = node->from(node[0]);
        add(args);
        return;
    }
    if (clutch) {
        // a{x:1} != a {x:1} but {x:1} becomes child of a
        // a{x:1} == a:{x:1} ?
        Node &letzt = last();

        if (node->kind == generics) {
            letzt.addMeta(node);
            letzt.metas()["generics"] = *node;
            if (letzt.kind == reference)
                letzt.kind = generics; // todo NOT ON THE reference!, on the value (?!?)
        }
        // do NOT use letzt for node.kind==patterns: {a:1 b:2}[a]
        //	only prefixOperators
        if (letzt.kind == functor and letzt.length == 0) {
            letzt.add(node);
            return;
        }
        // f (x) == f(x) ~= f x
        if (letzt.kind == reference or letzt.kind == key or
            letzt.name == "while" /*todo: functors, but not operators?*/)
            //          or  letzt.name == "for" ?
            letzt.addSmart(node);
        else if (name.empty() and kind != expression and kind != groups) // last().kind==reference)
            letzt.addSmart(node);
        else
            add(node); // don't loop to addSmart lol
        return;
    }
    if (node->isNil() and ::empty(node->name) and node->kind != longs)
        return; // skipp nils!  (NIL) is unrepresentable and always ()! todo?
    node->parent = this;
    if (node->length == 1 and flatten and ::empty(node->name))
        if (not(node->kind == patterns)) //  and data_mode
            node = &node->last();

    //  or node->kind == patterns  DON'T flatten patterns!
    if (not children and (node->kind == objects or node->kind == groups) and
        ::empty(node->name)) {
        children = node->children;
        length = node->length;
        for (Node &child: *this)
            child.parent = this;
        if (kind != groups) kind = node->kind; // todo: keep kind if … ?
    } else {
        add(node);
    }
}

//non-modifying
Node Node::insert(Node &node, int at) {
    if (this == &NIL) {
        warn("insert to NIL!");
        return *this;
    }
    if (length == 0)return node; //  todo: rescue value,name?
    while (at < 0)at = length + at;
    if (at >= length - 1) {
        Node *clon = clone();
        clon->add(node);
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
Node *Node::has(String s, bool searchMeta, short searchDepth) const {
    if (searchDepth < 0)return 0;
    if ((kind == key) and value.node and s == value.node->name)
        return value.node;
    for (int i = 0; i < length; i++) {
        Node &entry = children[i];
        if (s == entry.name) {
            if ((entry.kind == key or entry.kind == nils) and entry.value.node)
                return entry.value.node;
            else // danger overwrite a["b"]=c => a["b"].name == "c":
                return &entry;
        }
    }
    if (s == name.data)
        return const_cast<Node *>(this);
    //	if (meta and searchMeta and searchDepth == 0) {// todo: dont search leaves when searchDepth-->0
    //		Node *found = meta->has(s);
    //		if (found)return found;
    //	}


    if (searchDepth > 0) {
        for (int i = 0; i < length; i++) {
            Node *found = children[i].has(s, searchMeta, searchDepth--);
            if (found)return found;
        }
        if (kind == key)return value.node->has(s, searchMeta, searchDepth--);
    }
    return 0; // NIL
}

Node &Node::last() {
    return length > 0 ? children[length - 1] : *this;
}

//bool Node::empty() {// nil!
//	return isEmpty();
//}

bool Node::isEmpty() {
    // not required here: name.empty()
    return (length == 0 and value.longy == 0) or isNil();
}

// todo : [x y]+[z] = [x y z] BUT z isNil() ??  Node("z").kind==unknown ! empty referenceIndices ARE NIL OR NOT?? x==nil?
bool Node::isNil() const {
    // required here: name.empty()
    return this == &NIL or kind == nils or
           ((kind == key or kind == unknown or name.empty()) and length == 0 and value.data == nullptr);
}

// todo hide : use serialize() for true deep walk
chars Node::serializeValue(bool deep) const {
    // DON'T BE FOOLED!! CORRUPTION IN ONE PART HERE MAY CAUSE CORRUPTION MUCH LATER!!
    //    if (!this)return "";
    //    return "XXX";// DOES NOT EVEN HELP! so the issue must be in serialize() then !!!?!
    //	String wasp = "";
    Value val = value;
    //	switch (kind.type) {
    //		case int_array:
    //			wasp = "[";
    //			for (int i = 0; i < length; i++) {
    //				wasp=wasp+((int*)value.data)[i]+", ";
    //			}
    //			wasp += "]";
    //			return wasp;
    //		default:
    //			;// pass through
    //	}

    switch (kind) {
        case 0:
            return "ø";
        case fields:
            return "??";
        //            return val.node->name;
        case strings:
            return val.data ? "\""s + val.string + "\"" : "";
        //		case ints:
        case errors:
            return val.node ? val.node->name : "<ERROR>";
        case codepoint1:
            return String((codepoint) val.longy);
        case flags: {
            // todo: distinguish flags class from flags variables!
            if (type) {
                if (type->kind != flags)
                    error("flags type ≠ flags?");
                String flag_list;
                for (auto &flag: *type)
                    if (val.longy && flag.value.longy)
                        flag_list = flag.name + " | ";
                return flag_list;
            }
        }
        case longs:
        case long32:
            return formatLong(val.longy);
        case reals:
        case realsF:
            return ftoa(val.real);
        case nils:
            return "ø";
        case generics:
            return deep ? "" : "<"s + val.node->name + ">"; // "<…>";
        case structs:
        case records:
        case clazz:
        case variants:
        case enums:
            return name;
        //            return ""s + name + "{…" + "}";// todo fields
        case objects:
            return deep ? "" : "{…}"; // useful for debugging, but later return "" for
        case linked_list:
            return deep ? "" : "(:)";
        case groups:
            return deep ? "" : "(…)";
        case patterns:
            return deep ? "" : "[…]";
        case key:
            if (deep)
                return val.node ? val.node->serialize() : ""; // val.node->serialize();
            else
                return val.node ? val.node->name : ""; // val.node->serialize();

        case reference:
            return val.data ? val.node->name : "ø";
        case symbol:
            return *val.string;
        case bools:
            return val.longy > 0 ? "true" : "false";
        case arrays:
            return "[…]"; //val.data type?
        case buffers:
            return "int[]"; //val.data lenght?
        case call:
        case operators:
        case constructor:
        case functor:
        case urls:
        case modul:
            return name; // +"!"
        case declaration:
        case expression:
        case assignment:
        case unknown:
            return "";
        case last_kind:
            return "";
        case kind_padding:
            error("kind_padding is not a Kind");
        case referencex:
            return "ref";
        //        case wasmtype_struct:
        //        case wasmtype_array:
        //            return name;
        case number:
            return formatLong(val.longy);
        default:
            breakpoint_helper
            return "MISSING CASE";
            break;
    }
    error("MISSING CASE serializeValue for "s + (int) kind + " " + typeName(kind));
}

// todo: (x)=>x when root
String Node::serialize() const {
    //    return "serialize currently has a bug";
    // >>>>>>>>>>>> name != serializedValue <<<<<<<<<<  THIS causes BUG!
    // DON'T BE FOOLED!! CORRUPTION IN ONE PART HERE e.g. serializeValue() MAY CAUSE CORRUPTION MucH LATER!!
    // Start TRACE in RUN mode (NOT debug!!) to see AddressSanitizer output
    // DON'T BE FOOLED!! just because all tests pass now doesn't mean the bug here (in serializeValue?) is fixed!
    //    if (not this)return "";
    if (eq(name, "html"))
        return "<html>"s + value.string + "</html>"s;

    String wasp = "";
    if(this->meta) {
        for(Node& meta: *this->meta) {
            auto val = meta.values().serialize();
            if(meta.kind==key)val= meta.value.node->serializeValue();
            // wasp += "@"s + meta.serialize() + " ";// todo THIS IS THE WAY!
            // wasp += "@"s + meta.name + "(" + meta.serializeValue() + ") ";
            wasp += "@"s + meta.name + "(" + val + ") ";
        }
    }
    if (not use_polish_notation or length == 0) {
        if (not name.empty()) wasp += name;
        //        String serializedValue=serializeValue();// todo IS THIS A GENERAL STRING BUG!? can't return "XXX" => String !?!
        //        const String &serializedValue = "";
        const String &serializedValue = *new String(serializeValue());
        if (kind == longs or kind == reals)
            if (not parseLong(name) and name and name.data and name.data[0] != '0')
                return ""s + name + ":" + serializedValue;
        if (kind == strings and name and (name.empty() or name == value.string))
            return serializedValue; // not text:"text", just "text"
        if (kind == longs and name and (name.empty() or name == formatLong(value.longy)))
            return serializedValue; // not "3":3
        if (kind == reals) // and name and (name.empty() or name==itoa(value.longy)))
            return serializedValue; // not "3":3.14

        //   and !eq(serializedValue, "{…}") and
        //            !eq(serializedValue, "?")

        // >>>>>>>>>>>> name != serializedValue <<<<<<<<<<  THIS causes BUG! (or before???)
        if ((value.data or kind == longs) and serializedValue and not(name == serializedValue)) {
            if (name)
                wasp += ":";

            wasp += serializedValue;
            if (kind != longs) wasp += " ";
        }
    }

    if (length >= 0) {
        if (kind == expression and name.empty())wasp += ":";

        if ((length >= 1 or kind == patterns or kind == objects)) {
            // skip single element braces: a == (a)
            if (kind == groups and (not separator or separator == ' ')) wasp += "(";
            else if (kind == generics or kind == tags)wasp += "<";
            else if (kind == objects)wasp += "{";
            else if (kind == patterns)wasp += "[";
            else if (not separator) wasp += "("; // default and not…
        }

        if (use_polish_notation and not name.empty()) wasp += name;
        int i = 0;
        if (length > 0)
            if (kind == operators) wasp += " ";

        for (const Node &child: *this) {
            if (!child.node_header)
                continue; // broken child (e.g. in skipped reconstruct)
            if (length == 0)
                break; // how on earth is that possible??
            if (i++ > 0) wasp += separator ? separator : ' ';
            //            auto chil = "";// ok FOR NOW?? THIS 'fixes' it
            const String &chil = new String(
                child.serialize()); // <<< HERE is the BUG! still makes it fail soon or LATER!
            wasp += chil; // THIS results in test fail later!
            //            println(chil);// THIS results in MISSING Type LATER!! WTF C++
            //            wasp += *new String(chil);// ok FOR NOW??
        }

        if (length > 0 or kind == patterns or kind == objects) {
            if (kind == groups and (not separator or separator == ' ')) wasp += ")";
            else if (kind == generics or kind == tags) wasp += ">";
            else if (kind == objects)wasp += "}";
            else if (kind == patterns)wasp += "]";
            else if (not separator) wasp += ")"; // default
        }
        //        if (name == (char*)"‖") wasp += name; // THIS LINE BREAKS IT!?! impossible!
    }
    return wasp;
    //	return name.empty()? string() : name;
    //	return empty(node.name)? node.string() : node.name;
}

chars Node::toString() {
    return serialize();
}

chars Node::toString() const {
    return serialize();
}

String toString(Node &node) {
    return node.serialize();
}


Node &Node::setValue(const Value v) {
    //	::print("setValue");
    //	::print(v.longy);
    value = v;
    return *this;
}


Node &Node::setValue(int64 v) {
    value.longy = v;
    return *this;
}

// rest of node children split
Node &Node::from(Node &match) {
    return from(match.name);
}


Node &Node::from(int pos) {
    // inclusive
    Node &lhs = *new Node();
    for (int i = pos; i < length; i++) {
        lhs.add(children[i]);
    }
    return lhs.flat();
}

Node &Node::from(String match) {
    Node lhs = *new Node();
    bool start = false;
    for (Node child: *this) {
        if (start)lhs.add(&child);
        if (child.name == match)start = true;
    }
    if (lhs.length == 0)
        for (Node child: *this)
            if (child.name == match)return child.values();
    if (kind != call and kind != declaration)
        lhs.kind = kind;
    return lhs.flat();
}

Node &Node::to(String match) {
    Node rhs = *new Node();
    for (Node &child: *this) {
        if (child.name == match)
            break;
        rhs.add(&child);
    }
    //	if(rhs.length==0)// no match
    //		return *this;
    if (kind != call and kind != declaration)
        rhs.kind = kind;
    return rhs.flat();
}

Node &Node::to(Node match) {
    return to(match.name);
}

//	Node& flatten(Node &current){
Node &Node::flat() {
    //	if (kind == call)return *this;//->clone();
    if (kind == patterns or kind == modul or kind == declaration)
        return *this; // never flatten patterns x=[] "hi"[1] …
    if (length == 0 and kind == key and name.empty() and value.node)return *value.node;
    if (length == 1) {
        Node &child = children[0];
        if (child.kind == patterns and kind != groups) // huh?
            return *this; // never flatten patterns x=[] "hi"[1] …
        if (value.node == &child and name.empty()) // todo remove redundancy
            return *value.node;
        if (kind == key and name.empty()) {
            // (x):y => x:y
            if (not child.value.node and child.kind == reference) {
                child.value.node = value.node;
                child.kind = key;
            }
            return child;
        }
        if ((int64) children < MAX_MEM and not value.data and name.empty()) {
            child.parent = parent;
            return child.flat();
        }
        if (child.length > 0 and not child.value.data and child.name.empty()) {
            // double flatten ((x y))=>x y   todo: via recursion
            child.children[0].parent = this;
            children = child.children;
            length = child.length;
            return *this;
        }
    }
    // return *this; //->clone();
    return *this->clone();
}

Node &Node::setName(chars name0) {
    name = name0;
    return *this;
}

// extract value from this (remove name to avoid emit-setter )
Node &Node::values() {
    if (kind == key)return *value.node;
    if (kind == longs)return *new Node(value.longy);
    if (kind == reals)return *new Node(value.real);
    if (kind == strings)return *new Node(*value.string);
    if (kind == codepoint1)return *new Node((codepoint) value.longy);
    if (kind == bools)return value.data ? True : False;
    if (length == 1 and not value.data) return children[0]; // todo: reaaaly?
    Node &val = clone()->setName("");
    //	val.children = 0;
    return val;
}

bool Node::isSetter() {
    // todo BAD HEURISTIC!!
    // todo properly via expression i=1 == (set i 1)
    // todo proper constructor i:1 == (construct i (1))
    // todo i=0 == i.empty ?  that is: should null value construction be identical to NO value?
    // if(isNumber(name.data[0]))
    //     return false; // todo : enable and fix if 0:{3} else 4

    if (kind == bools)return not(name == True.name.data) and not(name == False.name.data);
    if (kind == longs || kind == reals) // || kind==bools)
        return not name.empty() and (not parseLong(name) and not name.contains('.')); // todo WTF hack
    if (kind == key and value.data) return true;
    if (kind == strings and name == value.string) return false; // todo x="x" '123'="123" redundancy bites us here
    if (kind == strings and value.data)
        return true;
    if (kind == global and value.data) return true;
    if (kind == reference and value.data) return true;
    return length > 0; // i:4
}

int Node::index(String string, int start, bool reverse) {
    if (reverse)return lastIndex(string, start);
    for (int i = start; i < length; ++i) {
        if (children[i].name == string)
            return i;
    }
    return -1; // throw "not found"
}

int Node::lastIndex(String &string, int start) {
    if (start <= 0)start = length;
    for (int i = start - 1; i >= 0; i--) {
        if (children[i].name == string)
            return i;
    }
    return -1; // throw "not found"
}

int Node::lastIndex(Node *node, int start) {
    if (start <= 0)start = length;
    for (int i = start; i >= 0; --i) {
        if (children[i] == node)
            return i;
    }
    return -1; // throw "not found"
}

// inclusive from…to todo make to exclusive like py?
// ⚠️ DANGER! any references pointing to children become INVALID!
//[[modifying]]
void Node::replace(int from, int to, Node *node) {
    if (to < 0)to = length;
    if (to < from)
        error("Node::replace from>to : "s + from + ">" + to);
    if (from < 0 or from >= length)
        error("Node::replace from<0 or from>=length");
    if (to >= length) {
        warn("Node::replace to>=length");
        to = length - 1;
    }
    if (!children)
        error("can't replace without children");
    children[from] = *node;
    int i = 0;
    while (to + i++ <= length) {
        children[from + i] = children[to + i]; // ok if beyond length
        children[from + i - 1].next = &children[from + i];
    }
    length = length - (to - from); //  + 1 if not inclusive;
    if (length > 0)children[length - 1].next = 0;
}

// INCLUDING to: [a b c d].remove(1,2)==[a d]
// todo  ⚠️ HIGH DANGER !!! check for bugs
[[modifying]] // todo  ⚠️ DANGER! any references pointing to children become INVALID!   ⚠️
void Node::remove(int from, int to) {
    // including
    if (to < 0)to = length;
    if (to < from)to = from;
    if (to >= length)to = length - 1;
    int i = -1;
    while (to + ++i < length) {
        if (!children)
            error("can't remove without children");
        children[from + i] = children[to + i + 1]; // ok if beyond length
        if (children[from + i].next)
            children[from + i].next = children + from + 1;
    }
    length = length - (to - from) - 1;
}


void Node::replace(int from, int to, Node &node) {
    replace(from, to, &node);
}

//Node &Node::metas() {
//	if (!meta) {
//		meta = new Node();
//		meta->setType(patterns);// naja!
//	}
//	return *meta;
//}

void print(Node &n) {
    n.print();
}

//void print(const Node &n0) {
//	print(&n0);
//}


void print(Node *n0) {
    if (!n0)return;
    print(*n0);
}


Node &Node::setKind(Kind kin, bool check) {
    if (kind == kin)return *this;
    if (kind == modul and kin == key)
        return *this; // todo   import host: host-funcs     module{.name=host}.value=host-funcs
    if (kind == 0)check = false;
    if (kind == operators and kin == expression)return *this; // don't generalize
    if (kind == declaration and kin == expression)return *this; // don't generalize
    // todo remove these check = false hacks and call setType() with check = false !
    if (kind == codepoint1 and kin == operators)check = false; // and name==(codepoint)value.longy
    if (kind == groups and (kin == expression or kin == functor))check = false;
    if (kind == reference and kin == key)check = false;
    if (kind == referencex and kin == key)check = false; // careful we lose $ semantics!
    if (kind == groups and kin == key)check = false;
    //	if (kind == groups and kin == global)check = false;
    if (kind == expression and kin == declaration)check = false;
    if (kind == operators and kin == key)check = false; // todo: allow operators as keys in maps {for:me while:day} ??
    if (kind == declaration and kin == assignment)check = false; // todo wait who changes x:=7 to x=7 ??
    if (check) {
        if (kind != unknown and kind != objects and
            kind != strings) // strings is default type after construction, ok to keep it in name
            error("Node already has semantic type "s + typeName(kind) + "! Can't change to " + typeName(kin));
    }
    if (value.data and (kin == groups or kin == objects))
        return *this;
    if (kind == nils and not value.data)
        return *this;
    kind = kin;
    //	if(name.empty() and debug){
    //		if(type==objects)name = object_name;
    //		if(type==groups)name = groups_name;
    //		if(type==patterns)name = patterns_name;
    //	}
    return *this;
}

List<String> &Node::toList() {
    List<String> &values = *new List<String>();
    for (Node &child: *this)
        values.add(child.name);
    return values;
}

bool Node::empty() {
    // we don't care about name here
    return length == 0 and value.data == 0;
}

void Node::clear() {
    next = 0;
    length = 0;
    children = 0;
    //	children[0] = 0; // messes with shared list!
    if (kind == key)
        error("can't clear key(?)");
    value.data = 0;
}

String *Node::Line() {
#if DEBUG
    if (line)return line;
    if (parent)return parent->Line();
#endif
    return new String("<missing line information>");
}

void Node::addMeta(Node *pNode) {
    metas().add(pNode);
}

bool Node::contains(const char *string) {
    for (Node &chile: *this)
        if (chile.name == string)return true;
    return false;
}


chars Node::containsAny(List<chars> strings, bool allow_quoted/*=true*/) {
    for (Node &chile: *this)
        for (chars string: strings)
            if (chile.name == string and (allow_quoted or chile.kind != Kind::strings))
                return string;
    return 0;
}

int Node::size() {
    return length;
}

Node &Node::childs() {
    Node &neu = *new Node();
    if (isGroup(kind))
        neu.kind = kind;
    else
        neu.kind = groups;
    neu.children = children;
    neu.length = length;
    return neu;
}


//Node &node(Type t, int64 value, char *name) {
//    return (*new Node(name)).setValue(value).setType(t, false);
//}
//
//Type::Type(const Node &o) {
//
//}


int ord(Node &p) {
    if (p.kind != codepoint1)
        p.invoke("ord", 0); // self
    return p.value.longy;
}


// todo: make constructor
extern "C" Node *smartNode(smart_pointer_64 smartPointer64) {
    //    tracef("smartNode(smartPointer64 : %llx\n", (int64) smartPointer64);
    if (smartPointer64 == 0)return &False; //const_cast<Node *>(&NIL);
    if (smartPointer64 == 0x8000000000000000)return new Node(0);
    //    if (!isSmartPointer(smartPointer64))
    //        return Node(smartPointer64);
    if ((smartPointer64 & negative_mask_64) == negative_mask_64) {
        return new Node((int64) smartPointer64);
    }
    if ((type_mask_64_word & smartPointer64) == 0) {
        int64 pure_long_60 = (int64) smartPointer64;
        return new Node(pure_long_60);
    } // as number
    if ((smartPointer64 & smart_pointer_header_mask) == smart_pointer_node_signature) {
        smart_pointer_64 pointer = smartPointer64 & smart_pointer_value60_mask;
        if (pointer < 0x100000000L) {
            // danger zone: unsafe reconstruction of Node from wasm memory
            Node &reconstruct = *reconstructWasmNode((int) pointer);
            return &reconstruct;
        }
        return (Node *) pointer;
    }

    //        if ((smartPointer64 & double_mask_64) == double_mask_64)
    if (smartPointer64 & double_mask_64) {
        // ok no other match since 0xFF already checked
        // todo rare cases, where doubles don't match 0x7F…
        double val = *(double *) &smartPointer64;
        return new Node(val);
    }

    auto value = smartPointer64 & 0xFFFFFFFF; // data part
    int64 smart_type64 = smartPointer64 & 0xFFFFFFFF00000000; // type part
    unsigned int smart_type_32 = smart_type64 >> 32;
    byte smart_type_4 = (smartPointer64 & 0xF000000000000000) >> 63; // type part
    //    short smart_type_payload = (short)(smartPointer64 & 0x0000FFFF00000000L)>>16;// type payload including length (of array)

    if (smart_type64 == string_header_64 or smart_type_4 == stringa or smart_type_32 == string_header_32) {
        // smart pointer for string
        //        if(smart_type_payload&string_meta::share)
        //        return new Node(new String((char *) wasm_memory) + value, false/*copy!*/);
        //        else
        if (not wasm_memory)
            error("wasm_memory not linked");
        char *string = ((char *) wasm_memory) + value;
        String *pString = new String(string, true /*copy!*/);
        Node &pNode = *new Node(pString, false /* not identifier*/);
        pNode.setKind(strings);
        return &pNode;
    }
    if (smart_type64 == codepoint_header_64)
        return new Node((codepoint) value);

    if (smart_type64 == array_header_64 /* and abi=wasp */) {
        auto arrayStruct = (int *) (((char *) wasm_memory) + value);
        return reconstructArray(arrayStruct);
    }

    breakpoint_helper
    printf("smartPointer64 : %llx\n", (int64) smartPointer64);
    error1("missing smart pointer type %x "s % smart_type64 + " “" + typeName(Type(smart_type64)) + "”");
    return new Node();
}

Node *reconstructArray(int *array_struct) {
    // smart pointer to smart array
    int kind = *array_struct++;
    if (kind == array_header_32 or kind == node_header_32) // todo we know it's array but still. plz fix
        kind = *array_struct++;
    int len = *array_struct++; // todo: leb128 vector later
    wasm_node_index type = *array_struct++;
    int stack_Item_Size = stackItemSize(kind, false); // mapType(type);
    Primitive value_kind;
    if (type) {
        Node &typ = *reconstructWasmNode(type);
        stack_Item_Size = stackItemSize(typ);
        value_kind = mapTypeToPrimitive(typ);
    }
    if (stack_Item_Size < 0 or stack_Item_Size > 1000)
        error("maybe internal emit out of sync. implausible stack_Item_Size for "s % typeName(kind));


    //        int capacity = *array_struct++;// OR:
    //        wasm_node_index type = *array_struct++;
    Node *arr = new Node();
    arr->kind = objects;
    //        arr->kind = kind;
    int pos = 0;
    while (len-- > 0) {
        // array_struct has advanced to continuous array item list
        char *val = (((char *) array_struct) + stack_Item_Size * pos++);
        Node *chile;
        if (value_kind == Primitive::byte_char)chile = new Node((codepoint) *val);
        else if (value_kind == Primitive::byte_i8)chile = new Node((int64) *val);
        else if (value_kind == Primitive::codepoint32)chile = new Node((codepoint) *(int64 *) val);
        else if (value_kind == wasm_int32)chile = new Node(*(int *) val);
        else if (value_kind == int16t)chile = new Node(*(short *) val);
        else if ((int) value_kind == longs)chile = new Node(*(int64 *) val);
        else if ((int) value_kind == reals)chile = new Node(*(double *) val);
        else todo("smartNode of array with element kind "s + typeName((Kind)value_kind));
        arr->add(chile);
    }
    return arr;
}


// ⚠️ Despite sanity checks, there's still a chance that parts here are unsafe!
// todo put sanity checks for node / string in extra function!
Node *reconstructWasmNode(wasm_node_index pointer) {
    if (pointer == 0)
        return &NUL; // we NEVER have node_pointer at 0
    if (pointer > 0x1000000 and debug) // todo proper memory bound check including data/runtime_offset
        warn("pointer>10000"); // todo remove (in)sanity check
        // error("pointer>10000"); // todo remove (in)sanity check
    if ((int64) pointer > MAX_MEM)
        error("wasm_node_index outside wasm bounds %x>%x"s % (int) pointer % (int64) MAX_MEM);
#if WASM
    warn("WASM direct Node* from pointer");
	return (Node *) (int64) pointer; // INSIDE wasm code INSIDE wasm linear wasm_memory
#endif
    if (not wasm_memory)
        error("no attached wasm memory");
    // Host 64 bit pointer layout is different than wasm 32 bit pointer layout! Can NOT reconstruct objects directly!
    // we don't want to mess with wasm_memory anyways, so do NOT use reference here
    Node &reconstruct = *new Node();
    // we must NOT mess with Node object in wasm_memory, with internal pointers and layout
    bool wasm32bit = true; // PER wasm INSTANCE, has to be set by run_wasm
    if (wasm32bit) {
        wasm32_node_struct nodeStruct = *(wasm32_node_struct *) ((int64) wasm_memory + (int64) pointer);
        if (not nodeStruct.node_header) {
            warn("reconstruct node sanity check failed");
            return &reconstruct;
            error("reconstruct node sanity check failed");
        }
        reconstruct.length = nodeStruct.length;
        reconstruct.value = nodeStruct.value;
        reconstruct.type = nodeStruct.node_type_pointer ? reconstructWasmNode(nodeStruct.node_type_pointer) : 0;
        if (nodeStruct.name_pointer > 0) // and nodeStruct.name_pointer < MAX_MEM)
            reconstruct.name = String(((char *) wasm_memory) + nodeStruct.name_pointer);
        //        else
        //            error("bad name");
        //        if (reconstruct.name.kind) {
        //            check_is(reconstruct.name.kind, string_header_32);// todo or similar
        //            if ((int64) reconstruct.name.data < 0 or (int64) reconstruct.name.data > MEMORY_SIZE)
        //                error("invalid string in smartPointer");
        //            reconstruct.name = String(((char *) wasm_memory) + (int64) reconstruct.name.data);// copy!
        //        } else reconstruct.name = "";// uh cheap fix?
        reconstruct.kind = nodeStruct.kind;
        reconstruct.meta = nodeStruct.meta_pointer ? reconstructWasmNode(nodeStruct.meta_pointer) : 0;
        if (nodeStruct.child_pointer >= 0) {
            // -1 means no children (debug/bug)

            reconstruct.children = new Node[reconstruct.length]; // Use new[] to call constructors
            reconstruct.capacity = reconstruct.length; // can grow later
            int *child_pointers = (int *) (((char *) wasm_memory) + nodeStruct.child_pointer);
            for (int i = 0; i < reconstruct.length; ++i) {
                int64 wasm_child_pointer = child_pointers[i];
                reconstruct.children[i] = *reconstructWasmNode(wasm_child_pointer);
                reconstruct.children[i].parent = &reconstruct;
                if (i > 0)reconstruct.children[i - 1].next = &reconstruct.children[i];
            }
        }
    } else {
        // 64 bit wasm
        // object has same layout, but we still need to fix pointers later
        reconstruct = *(Node *) ((int64) wasm_memory + (int64) pointer);
        for (int i = 0; i < reconstruct.length; ++i) {
            int64 wasm_child_pointer = (int64) reconstruct.children[i];
            reconstruct.children[i] = *reconstructWasmNode(wasm_child_pointer);
        }
    }
    if ((int64) reconstruct.name.data < 0 or (int64) reconstruct.name.data > MAX_MEM)
        error("invalid string in smartPointer");
    check_is((int64) reconstruct.name.kind, (int64) string_header_32);
    if (reconstruct.name.length < 0 or reconstruct.name.length > MAX_NODE_CAPACITY)
        error("reconstruct node sanity check failed for length");
    check_is((int64) reconstruct.node_header, (int64) node_header_32)
    if (reconstruct.length < 0 or reconstruct.length > reconstruct.capacity or reconstruct.length > MAX_NODE_CAPACITY)
        error("reconstruct node sanity check failed for length");
    return &reconstruct;
}


smart_pointer_64 toSmartPointer(Node *n) {
    return n->toSmartPointer();
}

//extern "C" Node* getField(Node* n, Node* field);
extern "C" Node *getField(Node *n, smart_pointer_64 field) {
    if (*(int *) field == node_header_32) {
        Node &nod = *(Node *) field;
        String &name = nod.name;
        return &n->operator[](name);
    }
    if (*(int *) field == string_header_32) {
        String &name = *(String *) field;
        return &n->operator[](name);
    }
    return &n->operator[](*(int *) field);
}


// this is NOT reinterpret cast, but a real cast, e.g. from 2 to '2'
extern "C"
Node cast(const Node &from, Type to_type) {
    if (from.kind == to_type.kind)return from;
    if (from.kind == reals and to_type.kind == longs)return Node((int64) from.value.real); // boring, done by wasm?
    if (from.kind == longs and to_type.kind == reals)return Node((double) from.value.longy);
    if (from.kind == longs and to_type.kind == bools)return Node((bool) from.value.longy);
    // REAL CASTS "2" to '2' to 2
    if (from.kind == longs and to_type.kind == strings)
        return Node(String(formatLong(from.value.longy)), false);
    if (from.kind == reals and to_type.kind == strings)
        return Node(String(formatRealWithBaseAndPrecision(from.value.real, 10, 2)), false);
    if (from.kind == longs and to_type.kind == codepoint1) {
        // digit to char! 2 => '2'
        // DANGER: user intention unclear!? 2 => '2' or 0x20 => ' '
        if (from.value.longy < 0 or from.value.longy > 9)
            error("int to char cast only for 0 to 9");
        if (from.value.longy > 9) return Node((codepoint) from.value.longy); // reinterpret cast
        return Node(getChar(formatLong(from.value.longy), 1));
    }
    if (from.kind == strings and to_type.kind == codepoint1)return Node(getChar(*from.value.string, 1)); // "a" => 'a'
    if (from.kind == strings and to_type.kind == bools) // "False" => false "nil" => false
        return Node(not falseKeywords.has(*from.value.string) and not nilKeywords.has(*from.value.string));
    if (from.kind == codepoint1 and to_type.kind == bools) {
        codepoint c = from.value.longy;
        if (empty(c))return Node(false);
        if (c == '0' or c == 'f' or c == 'F' or c == 'n' or c == 'N' or c == u'ø')return Node(false);
        return Node(atoi1(c) != 0);
    }
    if (from.kind == codepoint1 and to_type.kind == longs)return Node(from.value.longy);
    if (from.kind == codepoint1 and to_type.kind == strings)return Node(String((codepoint) from.value.longy), false);
    todo("cast "s + from.serialize() + " to " + typeName(to_type));
    return ERROR;
}


// only wrap if not already wrapped
Node *wrap(Node &node) {
    if (node.size() > 0 and isGroup(node.kind))
        return &node;
    Node *wrapped = new Node();
    wrapped->setKind(groups);
    wrapped->add(node);
    return wrapped;
}
