//
// Created by pannous on 30.09.19.
//

#ifndef MARK_NODE_H
#define MARK_NODE_H

#include "String.h"
//#import  "String.h" // FFS
#include <stdarg.h> // va_list OK IN WASM???

extern bool debug;

class Node;
extern Node True;
extern Node False;
extern Node NIL;
extern Node ERROR;
extern Node Infinity;// = Node("Infinity");
extern Node NaN;// = Node("NaN");

void log(Node &);
void log(Node*);

//class String;
union Value {
//	Node node;//  incomplete type
	Node *node = 0;// todo DANGER, can be lost :( !!
//	Node **children = 0;
	String string;
	void *data;
	long longy;
//	float floaty;
	double floaty;

	Value() {}// = default;
};


class Node {
public:
//	Node(const char*);


	Node(va_list args) {

	}

	String name = empty_name;// nil_name;
	Value value;

	// TODO REFERENCES can never be changed. which is exactly what we want, so use these AT CONSTRUCTION:
//	Node &parent=NIL;
//	Node &param=NIL;
	Node *parent = nullptr;

	// a{b}(c)[d] == a{body=b}->c->d // param deep chain, attention in algorithms
	Node *param = nullptr;// LINK, not list. attributes meta modifiers decorators annotations
	Node *children = nullptr;// LIST, not link. block body content

	/* don't mix children with param, see for(i in xs) vs for(i of xs) hasOwnProperty, getOwnPropertyNames
	 * conceptual cleanup needed... => DONE?
	 * children["_body_"] => children / $html.body
	 * children["_attrib_"] => params
	 * children["_head_"] => params / params.head / $html.head
	 * children["_meta_"] => params.meta
	 * children["_name"] == name
	 *
	 * */
//	Node *next = nullptr;// NO, WE NEED TRIPLES cause objects can occur in many lists + danger: don't use for any ref/var
	Type type = unknown;
	int length = 0;// children
//	int count = 0;// use param.length for arguments / param

	Node() {
		type = objects;
//		if(debug)name = "[]";
	}

	Node& first(){
		if (length>0)return children[0];
		throw "No such element";
//		return ERROR;
	}

//	if you do not declare a copy constructor, the compiler gives you one implicitly.
//	Node( Node& other ){// copy constructor!!


	Node *clone() {// const cast?
		if (this == &NIL)return this;
		if (this == &True)return this;
		if (this == &False)return this;
		// todo ...
		Node *copy = new Node();
		*copy = *this;// copy value ok
		return copy;
	}


	explicit Node(int buffer[]) {
		value.data = buffer;
		type = buffers;
		if (debug)name = "int[]";
//			buffer.encoding = "a85";
	}

	explicit Node(char c) {
		name = c;
		value.string = c;
		type = strings;
	}

	explicit Node(double nr) {
		value.floaty = nr;
		type = floats;
		if (debug)name = itoa0(nr); // messes with setField contraction
	}


	explicit Node(float nr) {
		value.floaty = nr;
		type = floats;
		if (debug)name = String((long) nr) + String(".…");//#+"#"; // messes with setField contraction
	}

// how to find how many no. of arguments actually passed to the function? YOU CAN'T! So …
// Pass the number of arguments as the first variable
// Require the last variable argument to be null, zero or whatever
	explicit Node(int a, int b, ...) {
		type = objects;// groups list
		add(Node(a).clone());
		va_list args;
		va_start(args, b);
		int i = b;
		while (i){
			add(Node(i).clone());
			i = (int) va_arg(args, int);
		}
		va_end(args);
	}

	// why not auto null terminated on mac?
	// vargs needs to be 0 terminated, otherwise pray!
	explicit Node(char *a, char *b, ...) {
		type = objects;// groups list
		add(Node(a).clone(),false);
		va_list args;
		va_start(args, b);
		char *i = b;
		while (i){
			Node *node = Node(i).clone();
			add(node);
			i = (char *) va_arg(args, char*);
		}
		va_end(args);
	}


	explicit Node(int nr) {
		value.longy = nr;
		type = longs;
		if (debug)name = itoa0(nr); // messes with setField contraction
	}

	explicit Node(const char *name) {
		this->name = name;
//		type = strings NAH;// unless otherwise specified!
	}

	explicit Node(bool nr) {
		if (this == &NIL)
			name = "HOW";
		value.longy = nr;
		type = longs;
		if (debug)name = nr ? "✔️" : "✖️";
//		if (debug)name = nr ? "true" : "false";
//		this=True; todo
	}


//	explicit
	Node(long nr) {
		value.longy = nr;
		type = longs;
	}


	explicit Node(String s, bool identifier = false) {
//		identifier = identifier || !s.contains(" "); BULLSHIT 'hi' is strings!!
		if (identifier) {
//			if(check_reference and not symbol)...
			name = s;
			type = reference;
		}
//		else if (atoi(s) and s == itoa0(atoi(s))) {
//			value.longy = atoi(s);
//			type = longs;
//			}
//		else if (atof(s)) { value.floaty = atoi(s); }
		else {
			type = strings;
			value.string = s;
			if (name == empty_name)name = s;
		}
	}

	explicit Node(Node **pNode) {
		children = pNode[0];
		type = arrays;
		value.data = pNode[0];
	}

	explicit
	operator bool() {
		return value.longy or length > 1 or (length == 1 and this != children and (bool) (children[0]));
	}

	bool operator==(int other);

	bool operator==(long other);

	bool operator==(float other);

	bool operator==(double other);

	bool operator==(String other);

//	bool operator==(Node other);
	bool operator==(Node &other);// equals

	bool operator!=(Node other);

	Node operator+(Node other);

//	bool operator!=(Node &other);// why not auto

//	 +=, -=, *=, /=, %=, <<=, >>=, &=, |=, ^=
//	bool operator<<=(Node &other);// EVIL MAGIC ;)
//	bool operator~();// MUST BE UNITARY:(


	String string() const {
		if (type == strings)
			return value.string;
		return name;
		breakpoint_helper
				err(String("WRONG TYPE ") + typeName(type));
	}

	// moved outside because circular dependency
	Node &operator[](int i);
	Node &operator[](int i) const;

	Node &operator[](char c);

	Node &operator[](String s);
	Node &operator[](String s) const;

	Node &operator=(int i);

	Node &operator=(chars s);

	Node &set(String string, Node *node);

	Node evaluate(bool expectOperator=false);

	Node insert(Node &node, int at = -1);// non-modifying

	void add(Node node);// modifying

//	void add(Node &node);

	void add(Node *node, bool flatten=true);

	void remove(Node *node); // directly from children
	void remove(Node &node); // via compare children

	Node *begin() const;

	Node *end() const;

	Node &merge(Node &other);// non-modifying

	void log() {
		printf("Node ");
		if (this == &NIL || type == nils) {
			printf("NIL\n");
			return;
		}
//		if || name==nil_name …
		if (name.data < (char *) 0xffff) {
			printf("BUG");
			return;
		}
//		assert (name.data < (char *) 0xffff);
		if (name and name.data and name.data > (char *) 0xffff and type != objects)
			printf("name %s ", name.data);
		printf("length %i ", length);
		printf("type  %s", typeName(type).data);
		if (this == &True)
			printf("TRUE");
		if (this == &False)
			printf("FALSE");
		if (type == objects and value.data)
			printf(" value.name %s", value.string.data);// ???
		if (type == bools)
			printf(" value %s", value.longy ? "TRUE" : "FALSE");
		if (type == strings)
			printf(" value %s", value.string.data);
		if (type == longs)
			printf(" value %li", value.longy);
		if (type == floats)
			printf(" value %f", value.floaty);

		printf(" [ ");
		for (int i = 0; i < length; i++) {
			Node &node = children[i];
//			if(check(node))
			printf("%s", node.name.data);
			printf(" ");
		}
		printf("]");
		printf("\n");
	}

	float precedence(Node &operater);

	Node apply(Node left, Node op0, Node right);

	Node &setType(Type type);

	long longe() {
		return type == longs or type == bools ? value.longy : value.floaty;// danger
	}

	float floate() {
		return type == longs ? value.longy : value.floaty;// danger
	}

	Node *has(String s, bool searchParams = true);

// type conversions
	explicit operator bool() const { return value.longy; }

	explicit operator int() const { return value.longy; }

	explicit operator long() const { return value.longy; }

	explicit operator float() const { return value.floaty; }

	explicit operator String() const { return value.string; }

	explicit operator char *() const { return value.string.data; }// or name()
	Node &last();

	bool empty();// same:
	bool isEmpty();

	bool isNil();

	const char *toString();
	const char *toString() const;

	const char * serialize() const;

	const char *serializeValue() const;

	void print();
};
typedef const Node Nodec;


#endif //MARK_NODE_H
