////
//// Created by me on 30.09.19.
////
//
////#include "Node.h"
#include "String.h"

struct Node;
extern Node True;
extern Node False;
extern Node NIL;


union Value {
	Node *node = 0;// todo DANGER, can be lost :( !!
//	Node node;// incomplete type
	String string;
	long longy;
//	float floaty;
	double floaty;
	void *data;

	Value() {}// = default;
};


class Node {
public:
//	Node(const char*);

	String name = empty_name;// nil_name;
	Value value;
	Node *parent = nullptr;
	Node *params = nullptr;// attributes meta modifiers decorators annotations
	Node *children = nullptr;// body content
	/* don't mix children with params, see for(i in xs) vs for(i of xs) hasOwnProperty, getOwnPropertyNames
	 * CONCEPTUAL CLEANUP NEEDED!
	 * children["_meta_"]
	 * children["_head_"]
	 * children["_body_"]
	 * children["_attrib_"]
	 * children["_name"] == this.name
	 *
	 * */
	Node *next = nullptr;// in children list, danger: don't use for any ref/var
	Type type = unknown;
	int length = 0;

	Node() {
		type = objects;
//		if(debug)name = "[]";
	}

//	if you do not declare a copy constructor, the compiler gives you one implicitly.
//	Node( Node& other ){// copy constructor!!


	Node *clone() {// const cast?
		Node *copy= new Node();
		*copy=*this;// copy value ok
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
		if (debug)name = itoa(nr);
	}


	explicit Node(float nr) {
		if (debug)name = String((long) nr) + String(".…");//#+"#";
		value.floaty = nr;
		type = floats;
	}

	explicit Node(int nr) {
		value.longy = nr;
		type = longs;
		if (debug)name = itoa(nr);
	}

	explicit Node(const char *name) {
		this->name = name;
	}

	explicit Node(bool nr) {
		if (this == &NIL)
			name = "HOW";
		value.longy = nr;
		type = longs;
		if (debug)name = nr ? "true" : "false";
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
//		else if (atoi(s) and s == itoa(atoi(s))) {
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
		this->children = pNode[0];
		type = arrays;
		value.data = pNode[0];
	}

	explicit
	operator bool() {
		return value.longy or length > 1 or (length == 1 and this != this->children and (bool) (this->children[0]));
	}

	bool operator==(int other);

	bool operator==(long other);

	bool operator==(float other);

	bool operator==(double other);

	bool operator==(String other);

//	bool operator==(Node other);
	bool operator==(Node &other);// equals

	bool operator!=(Node &other);// why not auto

//	 +=, -=, *=, /=, %=, <<=, >>=, &=, |=, ^=
//	bool operator<<=(Node &other);// EVIL MAGIC ;)
//	bool operator~();// MUST BE UNITARY:(


//	Node(bool b) {
//		Value(b ? 0 : 1);
//	}

	String string() {
		if (type == strings)
			return value.string;
//		return name;
		breakpoint_helper
		err(String("WRONG TYPE ") + typeName(type));
	}

	// moved outside because circular dependency
	Node &operator[](int i);

	Node &operator[](char c);

	Node &operator[](String s);

	Node &operator=(int i);

	Node &operator=(chars s);

	Node &set(String string, Node *node);

	Node evaluate();

	void add(Node &node);

	void add(Node *node);

	void remove(Node *node); // directly from children
	void remove(Node &node); // via compare children

	Node *begin();

	Node *end();

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

	Node& setType(Type type);

	long longe() {
		return type == longs or type==bools? value.longy : value.floaty;// danger
	}

	float floate() {
		return type == longs ? value.longy : value.floaty;// danger
	}

	Node *has(String s);

// type conversions
	explicit operator bool() const { return this->value.longy; }

	explicit operator int() const { return this->value.longy; }

	explicit operator long() const { return this->value.longy; }

	explicit operator float() const { return this->value.floaty; }

	explicit operator String() const { return this->value.string; }

	explicit operator char *() const { return this->value.string.data; }// or name()
};

//
//String operator "" s(const char* c, size_t){
//return String(c);// "bla"s
//}

//String operator ""_s(const char* c, size_t){
//	return String(c);// "bla"_s
//}

//new String();
//auto ws = {' ', '\t', '\r', '\n'};
Node Infinity = Node("Infinity");
String EMPTY = String('\0');
Node NaN = Node("NaN");
//NIL=0;
//Node NIL;
Node NIL = Node("NIL");
Node ERR = Node("ERR");// ≠ NIL
Node True = Node("True");
Node False = Node("False");

Node &Node::operator=(int i) {
	this->value.longy = i;
	this->type = longs;
	this->name = i;
	return *this;
}

Node &Node::operator=(chars c) {
	this->value.string = String(c);
	this->type = strings;
	if (name == empty_name)name = this->value.string;
	return *this;
}

Node &Node::operator[](int i) {
	if (i >= length) {
		breakpoint_helper;
		err(String("out of range index[] ") + i + " >= length " + length);
	}
	return this->children[i];
}

Node &Node::operator[](String s) {
	Node *found = has(s);
	if (found)return *found;
	if (name == s)// me.me == me ? really? let's see if it's a stupid idea…
		return *this;
	if (length == 1)
		if (children[0].has(s))return children[0][s];

	Node &neu = set(s, 0);// for n["a"]=b // todo: return DANGLING/NIL
	neu.type = nils; // until ref is set!
	neu.parent = this;
	if (neu.value.node) return *neu.value.node;
	else return neu;
}

Node *Node::begin() {
	return children;
}

Node *Node::end() {
	return children + length;
}

Node &Node::operator[](char c) {
	return (*this)[String(c)];
}

int capacity = 100;
int maxNodes = 10000;
int last = 0;

void *calloc(int i);

Node *all = static_cast<Node *>(calloc(capacity * maxNodes));

void *calloc(int i) {
	void *mem = malloc(i);
	while (i > 0) { ((char *) mem)[--i] = 0; }
	return mem;
}

// super wasteful, for debug
Node &Node::set(String string, Node *node) {
//	if (!children)children = static_cast<Node *>(malloc(capacity));

	if (!children) {
		children = &all[capacity * last++];
		if (this->name == nil_name)this->name = object_name;
	}
	if (this->length >= capacity / 2)todo("GROW children");
//	children = static_cast<Node *>(malloc(1000));// copy old
	Node &entry = children[this->length];
	if (this->length > 0) {
		Node &current = children[this->length - 1];
		current.next = &entry;
//		entry.previous=current;
	}
	entry.parent = this;
	entry.name = string;
	if (!node) {
//		entry.value.node=&entry;// HACK to set reference to self!
		entry.type = keyNode;
		entry.value.node = &children[capacity - this->length - 1];//  HACK to get key and value node dummy from children
//		 todo: reduce capacity per node
		entry.value.node->name = string;
		entry.value.node->type = Type::unknown;
		entry.value.node->parent = &entry;
//		entry.value.node=Node();// dangling ref to be set
	} else {
		entry = *node;// copy by value OK
		entry.parent = this;
	}
	this->length++;
	return entry;
}

//Node::Node(const char *string) {
//	this->value.string = String(string);
//	this->type = strings;
//}

bool Node::operator==(String other) {
	if (this->type == objects)return *this->value.node == other;
	if (this->type == longs) return other == itoa(this->value.longy);
	return this->type == strings and other == this->value.string;
}

bool Node::operator==(int other) {
	if (this == 0)return false;// HOW?
	if ((this->type == longs and this->value.longy == other) or (this->type == floats and value.floaty == other))
		return true;
	if (this->type == keyNode and value.node and *value.node == other)return true;
	if (this->type == strings and atoi(value.string) == other)return true;
	if (atoi(name) == other)return true;
	return false;
}

bool Node::operator==(long other) {
	return (this->type == longs and this->value.longy == other) or (this->type == floats and value.floaty == other);
}

bool Node::operator==(double other) {
	return (this->type == floats and this->value.floaty == ((float) other)) or
	       (this->type == longs and this->value.longy == other);
}

bool Node::operator==(float other) {
	return (this->type == floats and this->value.floaty == other) or
	       (this->type == longs and this->value.longy == other);
}

bool Node::operator==(Node &other) {
	if (this == &other)return true;// same pointer!
	if (this->value.node == &other)return true;// same value enough?
	if (this == other.value.node)return true;// same value enough?

	if (&other == &NIL and type == nils and length == 0 and value.data == 0)return true;
	if (type != other.type and this->type != unknown)
		if (type != keyNode and other.type != keyNode) return false;
	if (type == longs)
		return this->value.longy == other.value.longy;
	if (type == strings)
		return value.string == other.value.string;
	if (type == floats)
		return value.floaty == other.value.floaty;
	// if ...
	for (int i = 0; i < length; i++) {
		Node &field = this->children[i];
		Node &val = other[field.name];
		if (field != val) {
			if ((field.type != keyNode and field.type != nils) or !field.value.node)
				return false;
			Node deep = *field.value.node;
			return deep == val;
		}
	}
//	if(name==other.name and length>0 and length==other.length)
//		return true;
	return false;
}

bool Node::operator!=(Node &other) {
	return not(*this == other);
}

void log(Node &);

bool recursive = true;

Node values(Node n) {
	if (eq(n.name, "one"))return Node(1l);
	if (eq(n.name, "two"))return Node(2l);
	if (eq(n.name, "three"))return Node(3l);
	return n;
}

Node Node::evaluate() {
	if (this->length == 0)return values(*this);
	if (this->length == 1)return values(children[0]);
//	if (this->type != expression and this->type != keyNode)
//		return *this;
	float max = 0; // do{
	Node right;
	Node left;
	for (Node &node : *this) {
		float p = precedence(node);
		if (p > max) max = p;
		node.log();
	}
	if (max == 0) {
		error(String("could not find operator:") % name);
		return *this;
	}
	Node *op = 0;
	for (Node &n : *this) {
		float p = precedence(n);
		if (p == max and not op) {
			op = &n;
		} else if (op)
			right.add(n);
		else left.add(n);
	}
//		this->remove(&op);// fucks up pointers?
	if (recursive)
		return apply(left, *op, right);
//	};// while (max > 0);
	return *this;
}

void Node::remove(Node *node) {
	if (!children)return;// directly from pointer
	if (length == 0)return;
	if (node < children or node > children + length)return;
	for (int j = node - children; j < length - 1; j++) {
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

void Node::add(Node *node) {
	if (not children or (length == 0 and not value.node))
		value.node = node;
	if (!children) children = &all[capacity * last++];
	children[length++] = *node;
}

void Node::add(Node &node) {
//	if (not children or (length == 0 and not value.node) and type==nils)
//		value.node = &node;
	if (!children)
		children = &all[capacity * last++];
	children[length++] = node;
}

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

bool leftAssociativity(Node &operater) {
	return false;
//	https://icarus.cs.weber.edu/~dab/cs1410/textbook/2.Core/operators.html
}

float Node::precedence(Node &operater) {
	// like c++ here HIGHER up == lower value == more important
//	switch (node.name) nope
	String &name = operater.name;
	if (operater.type == strings)// and name.empty()
		name = operater.value.string;
	if (eq(name, "not"))return 1;
	if (eq(name, "¬"))return 1;
	if (eq(name, "!"))return 1;
	if (eq(name, "and"))return 1.1;
	if (eq(name, "&&"))return 1.1;
	if (eq(name, "&"))return 1.1;
	if (eq(name, "xor"))return 1.2;
	if (eq(name, "or"))return 1.2;
	if (eq(name, "||"))return 1.2;

	if (eq(name, "√"))return 3;
	if (eq(name, "++"))return 3;
//	if (eq(node.name, "+"))return 3;//
	if (eq(name, "--"))return 3;
	if (eq(name, "-"))return 3;// 1 + -x

	if (eq(name, "times"))return 5;
	if (eq(name, "*"))return 5;
	if (eq(name, "add"))return 6;
	if (eq(name, "plus"))return 6;
	if (eq(name, "+"))return 6;
	if (eq(name, "minus"))return 6;
	if (eq(name, "-"))return 6;
	if (eq(name, "="))return 10;
	if (eq(name, "=="))return 10;
	if (eq(name, "≠"))return 10;
	if (eq(name, "!="))return 10;
	if (eq(name, "equals"))return 10;
	if (eq(name, "equal"))return 10;
	return 0;// no precedence
}

/*
0x2218	8728	RING OPERATOR	∘
 */
Node Node::apply(Node left, Node op0, Node right) {
	left = left.evaluate();
	String &op = op0.name;
	bool lazy = (op == "or") and (bool) left;
	if (!lazy)
		right = right.evaluate();

	if (op == "not" or op == "¬" or op == "!") {
		bool x = not left.evaluate();
		return Node(x);
	}
	if (op == "#") {
		return right.length;// or right["size"] or right["count"]  or right["length"]
	}
	if (op == "√") {
		Node &unary_result = NIL;
		if (right.type == floats)
			unary_result = Node(sqrt(right.value.floaty));
		if (right.type == longs)
			unary_result = Node(sqrt(right.value.longy)).setType(floats);
		left.add(unary_result);
		return left.evaluate();
	}

//	if(!is_KNOWN_operator(op0))return call(left, op0, right);

	if (op == "|") {
		if (left.type == strings or right.type == strings) return Node(left.string() + right.string());
		if (left.type == longs and right.type == longs) return Node(left.value.longy | right.value.longy);
		// pipe todo
	}

	if (op == "&") {
		if (left.type == strings or right.type == strings) return Node(left.string() + right.string());
		return Node(left.value.longy & right.value.longy);
	}

	if (op == "xor" or op == "^|") {
		if (left.type == strings or right.type == strings) return Node(left.string() + right.string());
		return Node(left.value.longy ^ right.value.longy);
	}

	if (op == "and" or op == "&&") {
		if (left.type == strings or right.type == strings) return Node(left.string() + right.string());
		return Node(left.value.longy and right.value.longy);
	}// bool?

	if (op == "or" or op == "||" or op == "&") {
		if (left.type == strings or right.type == strings) return Node(left.string() + right.string());
		return Node(left.value.longy or right.value.longy);
	}// bool?

	if (op == "==" or op == "equals") {
		return left == right;
	}

	if (op == "!=" or op == "^=" or op == "≠" or op == "is not") {
		return left != right;
	}

	if (op == "+" or op == "add" or op == "plus") {
		if (left.type == strings or right.type == strings) return Node(left.string() + right.string());
		if (left.type == floats and right.type == floats) return Node(left.value.floaty + right.value.floaty);
		if (left.type == floats and right.type == longs) return Node(left.value.floaty + right.value.longy);
		if (left.type == longs and right.type == floats) return Node(left.value.longy + right.value.floaty);
		if (left.type == longs and right.type == longs) return Node(left.value.longy + right.value.longy);
//		if(left.type==arrays …
	}

	// todo: 2 * -x
	if (op == "-" or op == "minus" or op == "subtract") {
		if (left.type == floats and right.type == floats) return Node(left.value.floaty - right.value.floaty);
		if (left.type == longs and right.type == floats) return Node(left.value.longy - right.value.floaty);
		if (left.type == floats and right.type == longs) return Node(left.value.floaty - right.value.longy);
		if (left.type == longs and right.type == longs) return Node(left.value.longy - right.value.longy);
	}

	if (op == "*" or op == "⋆" or op == "×" or op == "∗" or op == "times") {// ⊗
		if (left.type == strings or right.type == strings) return Node(left.string().times(right.value.longy));
		if (left.type == floats and right.type == floats) return Node(left.value.floaty * right.value.floaty);
		if (left.type == longs and right.type == floats) return Node(left.value.longy * right.value.floaty);
		if (left.type == floats and right.type == longs) return Node(left.value.floaty * right.value.longy);
		if (left.type == longs and right.type == longs) return Node(left.value.longy * right.value.longy);
		todo(op + " operator NOT defined for types %s and %s ");

//		if (right.type == longs) return Node(left.value.longy * right.value.longy);
	}
	todo(op + " is NOT a builtin operator ");
	return NIL;
//	log("NO builtin operator "+op0+" calling…")
//	return call(left, op0, right);
}

Node& Node::setType(Type type) {
	this->type = type;
	return *this;
}

// Node* OK? else Node&
Node *Node::has(String s) {
	for (int i = 0; i < length; i++) {
		Node &entry = this->children[i];
		if (s == entry.name)
			if ((entry.type == keyNode or entry.type == nils) and entry.value.node)
				return entry.value.node;
			else // danger overwrite a["b"]=c => a["b"].name == "c":
				return &entry;
	}
	return 0;// NIL
}
