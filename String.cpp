//
// Created by pannous on 24.10.18.
//

//#ifndef NETBASE_STRING_CPP
//#define NETBASE_STRING_CPP
//#include "String.h" prefering one big WASM object

//void *malloc(int size);

//#include <malloc.h>
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wstring-compare"
#define let auto
#define var auto
typedef void *any;
typedef unsigned char byte;
typedef const char *chars;
//typedef int size_t;

extern unsigned int *memory;
extern unsigned int *&heap;

extern "C" void logs(const char *);
extern bool debug = true;
//extern void *malloc (size_t __size);
//extern void *malloc (int __size);

#ifndef WASM

#include <cstdio>
#include <tgmath.h>

void err(chars error);

void todo(chars error);

void logs(const char *s) {
	printf("%s\n", s);
}

void logi(long i) {
	printf("%li\n", i);
}

#else

extern "C" void print (const char *);// no \n newline
extern "C" void logs (const char *);
extern "C" void logc(char s);
extern "C" void logi(int i);
void printf(const char *s);
void printf(const char *format, int i);
void printf(const char *format, chars i);
//  extern "C" int printf (const char *__restrict __format, ...);
#endif

// NodeType todo move
enum Type {
// plurals because of namespace clash
			objects = 0,
	reference,// variable identifier name x
	symbol,// one / plus / Jesus
	operators,// or just symbol?
	expression,// one plus one
	strings,
	arrays,// same as:
	buffers,
	nodes,
	floats,
	longs,
	ints,
	bools,
	nils,
	unknown = 20 //7
};

class String;

String str(const char *&s);

String typeName(Type t);

int atoi0(const char *__nptr);


bool eq(const char *dest, const char *src) {
	if (!dest || !src)
		return false;
	int i = 0;
	while (char c = dest[i]) {
		if (!src[i] || !c)
			return false;
		if (src[i] != c)
			return false;
		i++;
	}
	if (src[i])
		return false;// different length!
//	while (char c = src[i]) {
//		if (!dest[i] || !c)
//			return false;
//		if (dest[i] != c)
//			return false;
//		i++;
//	}
	return true;
}


void strcpy(char *dest, const char *src, int length = -1) {
	if (!dest || !src)
		return;
	int i = 0;
	while (char c = src[i]) {
		if (length-- == 0)break;
		dest[i] = c;
		i++;
	}
}


int atoi0(const char *p) {
	int k = 0;
	while (*p) {
		int n = (*p) - '0';
		if (n < 0 or n > 9)
			return k;
		k = (k << 3) + (k << 1) + n;
		p++;
	}
	return k;
}

double atof0(const char *string) {
	double result = 0.0;
	if (!string) return result;

	double multiplier = 1;
	double divisor = 1.0;
	int integer_portion = 0;

	integer_portion = atoi0(string);

	result = (double) integer_portion;
	if (*string == '-') {
		result *= -1;
		multiplier = -1;
	}

	while (*string && (*string != '.'))
		string++;
	if (*string)
		string++;
	while (*string) {
		if (*string < '0' || *string > '9') return result;
		divisor *= 10.0;
		result += (double) (*string - '0') / divisor;
		string++;
	}
	return result * multiplier;
}

class String;

class Error {
public:
	char *message;
//	Error(String *string) {
//		message = string->data;
//	}
};

class IndexOutOfBounds : Error {
public:
//	IndexOutOfBounds(String *string1) : Error(string1){}
	IndexOutOfBounds(char *data, int i) : Error() {}
};

void reverse(char *str, int len);

// Implementation of itoa()
char *itoa(long num, int base = 10) {
	char *str = new char[100];
	int len = 0;
	bool isNegative = false;
	/* Handle 0 explicitely, otherwise empty string is printed for 0 */
	if (num == 0) {
		str[len++] = '0';
		str[len] = '\0';
		return str;
	}
	// In standard itoa(), negative numbers are handled only with
	// base 10. Otherwise numbers are considered unsigned.
	if (num < 0 && base == 10) {
		isNegative = true;
		num = -num;
	}
	// Process individual digits
	while (num != 0) {
		int rem = num % base;
		str[len++] = (rem > 9) ? (rem - 10) + 'a' : rem + '0';
		num = num / base;
	}
	// If number is negative, append '-'
	if (isNegative)
		str[len++] = '-';
	str[len] = '\0'; // Append string terminator
	// Reverse the string
	reverse(str, len);
	return str;
}

void reverse(char *str, int len) {
	for (int i = 0; i < len / 2; i++) {
		char t = str[len - 1 - i];
		str[len - 1 - i] = str[i];
		str[i] = t;
	}
}

//char itoa(byte i) {
//	return (char) (i + 0x30);
//}

class String {

public:
	char *data{};
	int length = -1;

	String() {
		data = static_cast<char *>(malloc(sizeof(char) * 2));
		length = 0;
	}

	String(char c) {
		data = static_cast<char *>(malloc(sizeof(char) * 2));
		data[0] = c;
		data[1] = 0;
		length = 1;
	}

	String(const char string[]) {
		data = const_cast<char *>(string);
		length = len(data);
	}


//		String operator+(Type e){
	String(Type e) : String(typeName(e)) {}

	explicit String(int c) {
		data = itoa(c);
		length = len(data);
	}

	explicit String(long c) {
		data = itoa(c);
		length = len(data);
	}

#ifndef WASM

	String(std::string basicString) {
		data = const_cast<char *>(basicString.data());// copy?
	}

#endif

	char charAt(int i) {
		if (i >= length)
			err((String("IndexOutOfBounds at ") + itoa(i) + " in " + data).data);
		return data[i];
	}

	char charCodeAt(int i) {
		if (i >= length)
			throw IndexOutOfBounds(data, i);
//		String("IndexOutOfBounds at ") + i + " in " + data;
//			throw new IndexOutOfBounds(String(" at ") + i + " in " + data);
		return data[i];
	}

	int indexOf(char c, int i = 0) {
		for (int j = i; j < length; j++) {
			if (data[j] == c)return j;
		}
		return -1;
	}

//	operator std::string() const { return "Hi"; }

	String substring(int from, int to = -1) { // excluding to
		if (to < 0 or to > length)to = length;
		auto *neu = static_cast<char *>(malloc((sizeof(char)) * (to - from)+1));
		strcpy(neu, &data[from], to - from);
		neu[to - from] = 0;
		return String(neu);
	}

	int len(const char *data) {
		if (!data)data = this->data;
		if (!data || data[0] == 0)
			return 0;
		int MAX = 100000;
		for (int i = 0; i < MAX; ++i) {
			if (!data[i])return i;
		}
		return -1;//error
	}

	String &append(char c) {
		if(!data)data = static_cast<char *>(malloc(sizeof(char) * 2));
		if (data + length + 1 == (char *) memory) {// just append recent
			data[length++] = c;
			data[length] = 0;
			memory += 2;
		} else {
			auto *neu = static_cast<char *>(malloc(sizeof(char) * length + 5));
			if (data)strcpy(neu, data);
			neu[length++] = c;
			data = neu;
			data[length] = 0;
		}
		return *this;
	}

	String clone() {
		return String(this->data);
	}


	String operator%(String &c) {
		String b = this->clone();
		b.replace("%s", c);
		return b;
	}

	String operator%(char *c) {
		return this->replace("%s", c);
	}

	String operator%(long d) {
		return this->replace("%d", itoa(d));
	}

	String operator%(double f) {
		String formated = String() + itoa(f) + "." + itoa((f - int(f)) * 10000);
		return this->replace("%f", formated);
	}

	String *operator+=(String &c) {
		this->data = (*this + c).data;
		this->length += c.length;
		return this;
	}

	String *operator+=(String *c) {
		this->data = (*this + c).data;
		this->length += c->length;
		return this;
	}

	String *operator+=(char *c) {
		while (c++)append(c[0]);
		return this;
	}

	String *operator+=(char c) {
		append(c);
		return this;
	}

	String operator+(String c) {
		if (c.length <= 0)
			return *this;
		auto *neu = static_cast<char *>(malloc(length + c.length + 1));
		if (data)strcpy(neu, data, length);
		if (c.data)strcpy(neu + length, c.data, c.length);
		neu[length + c.length] = 0;
		return String(neu);
	}

	String operator+(const char x[]) {
		return this->operator+(String(x));
	}
//	String operator+(float i) {
//		return this->operator+(String(i));
//	}

	String operator+(bool b) {
		return this->operator+(b ? " true" : " false");
	}

	String operator+(long i) {
		return this->operator+(String(i));
	}

	String operator+(int i) {
		return this->operator+(String(i));
	}

	String operator+(char c) {
		return this->operator+(String(c));
	}

	String operator++() {
		this->data++;// self modifying ok?
		length--;
		return *this;
	}

	String operator++(int postfix) {//
		this->data += postfix;// self modifying ok?
		length -= postfix;
		return *this;
	}

	String operator+(char *c) {
		return this->operator+(String(c));
	}

	bool operator==(char c) {
		return !empty() && this[0] == c && this[1] == '\0';
	}

	bool operator==(String &s) {
		return eq(data, s.data);
	}

	bool operator==(char *c) {
		return eq(data, c);
	}

	bool operator!=(char *c) {
		return eq(data, c);
	}

	char operator[](int i) {
		return data[i];
	}

	operator chars() {// !
		return data;
	}

	bool empty() {
		return !data || data[0] == 0;
	}

	int indexOf(chars string) {
		int l = len(string);
		for (int i = 0; i <= length - l; i++) {
			bool ok = true;
			for (int j = 0; j < l; j++) {
				if (data[i + j] != string[j]) {
					ok = false;
					break;
				}
			}
			if (ok)
				return i;
		}
		return -1;//
	}

	bool contains(chars string) {
		return indexOf(string) >= 0;
	}

	String replace(chars string, chars with) {
		int i = this->indexOf(string);
		if (i >= 0)
			return String(substring(0, i) + with + substring(i + len(string), length));
		else
			return *this;
	}

	String times(short i) {
		if (i < 0)
			return "";
		String concat = "";
		while (i-- > 0)
			concat += this;
		return concat;
	}

};
//String String::operator++() {
//	this->data++;// self modifying ok?
//	return *this;
//}


//class NodeMap{
//
//};

class Node;

union Value {
	Node *node = 0;// todo DANGER, can be lost :( !!
//	Node node;// incomplete type
	String string;
	long longy;
	float floaty;
	void *data;

	Value() {}// = default;
};

struct Node;
String nil_name = "nil";
String empty_name = "";
String object_name = "<object>";
extern Node True;
extern Node False;
extern Node NIL;

class Node {
public:
//	Node(const char*);

	String name = empty_name;// nil_name;
	Value value;
	Node *parent = nullptr;
	Node *children = nullptr;
	Node *next = nullptr;// in children list, danger: don't use for any ref/var
	Type type = unknown;
	int length = 0;

	Node() {
		type = objects;
//		if(debug)name = "[]";
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

	explicit Node(bool nr) {
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
		} else {
			type = strings;
			value.string = s;
		}
	}

	explicit Node(Node **pNode) {
		type = arrays;
		value.data = pNode;
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

	bool operator==(Node other);

	bool operator!=(Node other);


//	Node(bool b) {
//		Value(b ? 0 : 1);
//	}

	String string() {
		if (type == strings)
			return value.string;
//		return name;
		throw "WRONG TYPE";
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
		if (name.data < (char *) 0xffff)
			err("BUG");
		if (name and name.data and name.data > (char *) 0xffff and type != objects)
			printf("name %s ", name.data);
		printf("length %i ", length);
		printf("type  %s", typeName(type).data);
		if (this == &True)
			printf("TRUE");
		if (this == &False)
			printf("FALSE");
		if (type == objects and value.data)
			printf(" name %s", value.string.data);
		if (type == bools)
			printf(" value %s", value.longy ? "TRUE" : "FALSE");
		if (type == strings)
			printf(" value %s", value.string.data);
		if (type == longs)
			printf(" value %li", value.longy);
		if (type == floats)
			printf(" value %f", value.floaty);
		printf("\n");
	}

	float precedence(Node &node);

	Node apply(Node left, Node op0, Node right);

	Node setType(Type type);

	long longe() {
		return type == longs ? value.longy : value.floaty;// danger
	}

	float floate() {
		return type == longs ? value.longy : value.floaty;// danger
	}
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
	return *this;
}

Node &Node::operator[](int i) {
	if (i >= length)throw String("out of range ") + i + " > " + length;
	return this->children[i];
}

Node &Node::operator[](String s) {
	for (int i = 0; i < length; i++) {
		Node &entry = this->children[i];
		if (s == entry.name)
			if ((entry.type == nodes or entry.type == nils) and entry.value.node)
				return *entry.value.node;
			else // danger overwrite a["b"]=c => a["b"].name == "c":
				return entry;
	}
	Node &neu = set(s, 0);// for n["a"]=b // todo: return DANGLING/NIL
	neu.type = nils; // until ref is set!
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
Node *all = static_cast<Node *>(malloc(capacity * maxNodes));// super wasteful, for debug
Node &Node::set(String string, Node *node0) {
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
	if (!node0) {
//		entry.value.node=&entry;// HACK to set reference to self!
		entry.value.node = &children[capacity - this->length - 1];//  HACK to get key and value node dummy from children
//		 todo: reduce capacity per node
		entry.value.node->name = string;
		entry.value.node->type = nodes;// todo value
		entry.value.node->parent = &entry;
		entry.type = nodes;
//		entry.value.node=Node();// dangling ref to be set
	} else {
		Node node = *node0;
		entry.type = node.type;
//	if(node.type==) // ...
		entry.value = node.value; // copy by ref
		if (node.type == unknown) {
			entry.type = nodes;
			entry.value.node = &node;
		}
	}
	this->length++;
	return entry;
}

//Node::Node(const char *string) {
//	this->value.string = String(string);
//	this->type = strings;
//}

bool Node::operator==(String other) {
	return this->type == strings and this->value.string == other;
}

bool Node::operator==(int other) {
	return (this->type == longs and this->value.longy == other) or (this->type == floats and value.floaty == other);
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

bool Node::operator==(Node other) {
	if (type != other.type)
		if (type != nodes and other.type != nodes) return false;
	if (type == longs)
		return this->value.longy = other.value.longy;
	if (type == strings)
		return value.string == other.value.string;
	if (type == floats)
		return value.floaty == other.value.floaty;
	// if ...
	for (int i = 0; i < length; i++) {
		Node &field = this->children[i];
		Node &val = other[field.name];
		if (field != val) {
			if ((field.type != nodes and field.type != nils) or !field.value.node)
				return false;
			Node deep = *field.value.node;
			return deep == val;
		}
	}
	return false;
}

bool Node::operator!=(Node other) {
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
//	if (this->type != expression and this->type != nodes)
//		return *this;
	float max = 0; // do{
	Node right;
	Node left;
	for (Node &node : *this) {
		float p = precedence(node);
		if (p > max) max = p;
		node.log();
	}
	if (max == 0)
		return *this;
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

float Node::precedence(Node &node) {
	// like c++ here HIGHER up == lower value == more important
//	switch (node.name) nope
	if (eq(node.name, "not"))return 1;
	if (eq(node.name, "¬"))return 1;
	if (eq(node.name, "!"))return 1;
	if (eq(node.name, "and"))return 1.1;
	if (eq(node.name, "&&"))return 1.1;
	if (eq(node.name, "&"))return 1.1;
	if (eq(node.name, "xor"))return 1.2;
	if (eq(node.name, "or"))return 1.2;
	if (eq(node.name, "||"))return 1.2;

	if (eq(node.name, "√"))return 3;
	if (eq(node.name, "++"))return 3;
//	if (eq(node.name, "+"))return 3;//
	if (eq(node.name, "--"))return 3;
	if (eq(node.name, "-"))return 3;// 1 + -x

	if (eq(node.name, "times"))return 5;
	if (eq(node.name, "*"))return 5;
	if (eq(node.name, "add"))return 6;
	if (eq(node.name, "plus"))return 6;
	if (eq(node.name, "+"))return 6;
	if (eq(node.name, "minus"))return 6;
	if (eq(node.name, "-"))return 6;
	if (eq(node.name, "-"))return 6;
	if (eq(node.name, "-"))return 6;


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

Node Node::setType(Type type) {
	this->type = type;
	return *this;
}


void log(long i) {
	printf("%li", i);
}

void log(chars s) {
	printf("%s\n", s);
}

void log(Node &n) {
	n.log();
}


//void log(Node &node) {
//	Node node2 = node;
//	log(node2);
//}


#ifndef WASM

#import <string>

void log(std::string s) {
	printf("%s\n", s.data());
}

#endif

void log(String s) {
	printf("%s\n", s.data);
}

void log(Node *n0) {
	if (!n0)
		return;
	Node n = *n0;
	log(n);
}

#ifdef WASM
void printf(const char *s) {
	print(s);
}
void printf(const char *format, int i) {
	print(String(format).replace("%d", String(i)).replace("%i", String(i)).replace("%li", String(i)));
}
void printf(const char *format, long i) {
	print(String(format).replace("%d", String(i)).replace("%i", String(i)).replace("%li", String(i)));
}
void printf(const char *format, chars value) {
	print(String(format).replace("%s", value));
}

void printf(const char *format, void* value) {
	print(String(format).replace("%p", String((long)value)));
}
#endif

//#endif //NETBASE_STRING_CPP

#pragma clang diagnostic ignored "-Wuser-defined-literals"
#pragma clang diagnostic ignored "-Wliteral-suffix"

String operator "" s(const char *c, size_t) {
	return String(c);// "bla"s  literal operator suffixes not preceded by ‘_’ are reserved for future standardization
}

String operator "" _s(const char *c, size_t) {
	return String(c);
}

String operator "" _(const char *c, unsigned long t) {
	return String(c);
}


String string(const char *&s) {
	return String(s);
}

String str(const char *&s) {
	return String(s);
}

String str(char *s) {
	return String(s);
}

String s(const char *&s) {
	return String(s);
}


String typeName(Type t) {
	switch (t) {
		case objects:
			return "object";
		case reference:
			return "reference";
		case symbol:
			return "symbol";
		case operators:
			return "operator";
		case expression:
			return "expression";
		case strings:
			return "string";
		case arrays:
			return "array";
		case buffers:
			return "buffer";
		case nodes:
			return "node";
		case floats:
			return "float";
		case longs:
			return "long";
		case ints:
			return "int";
		case bools:
			return "bool";
		case nils:
			return "nil";
		case unknown :
			return "unknown";
		default:
			throw str("MISSING Type name mapping ") + t;
	}
}


void todo(chars error) {
	err(str("TODO ") + error);
}

class SyntaxError : String {
public:
	char *data;
public:
	SyntaxError(String &error) {
		this->data = error.data;
	}
};

#pragma clang diagnostic pop