#pragma once
//
// Created by pannous on 30.09.19.
//

#include "String.h"
#include "smart_types.h"
#include "NodeTypes.h"

//#ifndef WASI
//SOMETIMES IT WORKS with WASI, sometimes it doesnt!? ./build-wasm.sh fails as of 2021/2
#include <stdarg.h> // va_list OK IN WASM???
//#include <cstdarg> // va_list ok in wasm even without wasi!
#include <cstdlib> // OK in WASM!

#ifndef WASM
#include <initializer_list> // allow List x={1,2,3};
#endif

#define NODE_DEFAULT_CAPACITY 100

typedef char const *chars;
typedef unsigned char byte;//!

extern bool debug;
extern bool throwing;// false for error tests etc
extern bool panicking;// false for error tests, webview, etc

// todo: wrap parser-options
static bool use_polish_notation;// prefix notation, s-expression parser flag  (html (body)) vs html{body{}}

// todo
enum class NodeFlags { // class makes it typesafe
    is_mutable = 1, // reference / value ≠ let final , constant
    is_required = 2, // not_nullable=2, // not_optional  strange default in 2022 but 'easy' unsafe c interaction ;)
    is_static = 4, //
    is_global = 8,
//    is_writable=2,
//    is_readable=4,//?
};

//The "correct" way is to define bit operators for the enum, as:
//inline AnimalFlags operator|(AnimalFlags a, AnimalFlags b)
//{
//    return static_cast<AnimalFlags>(static_cast<int>(a) | static_cast<int>(b));
//}



struct ParserOptions { // not just for parser but also for serialize!!

//    todo move polish_notation here! … how? polish_notation is also used in serialize() !
//    bool polish_notation= false;// prefix notation, s-expression parser flag  (html (body)) vs html{body{}}

// parse x[1] as x:1 reference with immediate value or x:[1] reference with pattern
// parse x={y} as x:{y} as x{y} or keep operator =
    bool data_mode = false;
    bool arrow = false; // treat -> arrow as map operator :
    bool dollar_names = false;// $name as in wat,wit
    bool at_names = false;// @interface as in wat,wit
    bool use_tags = false;// <html> or
    bool use_generics = false;// generic list<abc> , "less than" requires spaces, a<b can still be resolved as 'smaller' in analyzer

    bool kebab_case = true;//  false;// kebab-case means: parse "-" as hypen instead of minus, or 1900 - 2000AD (easy with units)
    // a-b can still be resolved as minus in analyzer

    bool space_brace = false;// resolve a {x} as a{x}
    String current_dir = "./"; // where to look for includes
};

//extern unsigned char __heap_base;
//unsigned int bump_pointer = &__heap_base;

typedef unsigned char byte;
typedef chars chars;

//extern "C" int isalnum(int _c);

#ifdef WASM
// message in CMAKE!
//#warning COMPILING TO WASM
#else
//#warning COMPILING TO APPLE
#endif


class Node;

//extern const Node True;
//extern const Node False;
//extern const Node NIL;
//extern const Node ERROR;
//extern const Node Infinity;// = Node("Infinity");
//extern const Node NegInfinity;// = Node("Infinity");
//extern const Node NaN;// = Node("NaN");
//

extern const Node NIL;
static Node &NUL = const_cast<Node &>(NIL);
extern Node True;
extern Node False;
extern Node ERROR;
extern Node Ignore;
extern Node Infinity;// = Node("Infinity");
extern Node NegInfinity;// = Node("Infinity");
extern Node NaN;// = Node("NaN");

void print(Node &n);

void print(Node *n0);
//void print(const Node &);


//class String;
union Value {
//	sizeof(Value)==8 (long)
	Node *node = 0;// todo DANGER, pointer can be lost :(   // todo same as child
//	Node *child = 0; //todo DANGER child and next are NOT REDUNDANT! (a:b c:d) == a(value=b next=c(value=d))
//	Node **children = 0;// keep children separate for complex key nodes (a b c):(d e f)
	String *string;// todo: wasm_chars*
	void *data;// any bytes
	long long longy;
//	codepoint chary;// use longy
	double real;

	Value() {}// = default;
	Value(int i) { longy = i; }

	Value(long i) { longy = i; }

	Value(bool b) { longy = b; }

	Value(double r) { real = r; }

//	Value(String s) { string = &s; }

	Value(String &s) { string = &s; }

	Value(String *s) { string = s; }

//	~Value() = default;
};

//
//struct TypedValue {
//	Kind type;// 1 byte!
//	//	Kind kind;
//	//	ValueKind kind;
////	Node* type; forseeing? but:
//	Value value;// node can have its own type
//};
//
//struct TypedNode {
//	Node *type;
//	Node *node;
//};

// The order of Type,Value is reverse to the Wasp ABI return tuple Value(int32), Type(int32)
class Node {
	// todo: sizeof(Node) can be reduced later by: shrinking header, merging type&kind, let *children own its length, make name String* offset
    // sizeof(Node) == 64 (20 for name,
    //	short _node_header_ = 0xDADA; //
//	static
    int node_header = node_header_32;
public:
    Node *type = 0;// variable/reference type or object class?
    int length = 0;// children
    Node *children = nullptr;// LIST, not link. block body content
    ::Kind kind = unknown;// improved from 'undefined' upon construction
    Value value;// value.node and next are NOT REDUNDANT  label(for:password):'Passwort' but children could be merged!?
//	Type kind = unknown;// improved from 'undefined' upon construction

    String name = empty_name;// nil_name;
    Node *meta = 0;// LINK, not list. attributes meta modifiers decorators annotations
    Node *parent = nullptr;
    // todo rename and alias:
    // lets lads lats lates: lets because a=b;c=d …; lads children; lats laterals; lates delayed evaluation
    Node *next = 0; // in children list, redundant with children[i+1] => for debugging only
    char separator = 0;// " " ";" ","
//	char grouper = 0;// "()", "{}", "[]" via kind!  «…» via type Group("«…»")

    int capacity = NODE_DEFAULT_CAPACITY;
    long _hash = 0;// set by hash(); should copy! on *x=node / clone()
#ifdef DEBUG
// int code_position; // hash to external map
//	int lineNumber;
	String *line = 0;// debug! EXPENSIVE for non ast nodes!
#endif

// TODO REFERENCES can never be changed. which is exactly what we want, so use these AT CONSTRUCTION:
//	Node &parent=NIL;
//	Node &param=NIL;

//	int count = 0;// use param.length for arguments / param

// a{b}(c)[d] == a{body=b}->c->d // param deep chain, attention in algorithms
//	Node *param = nullptr;// LINK, not list. attributes meta modifiers decorators annotations

	/* don't mix children with param, see for(i in xs) vs for(i of xs) hasOwnProperty, getOwnPropertyNames
	 * conceptual cleanup needed... => DONE?
	 * children["_body"] => children / $html.body
	 * children["_attrib"] => params
	 * children["_head"] => params / params.head / $html.head
	 * children["_meta"] => params.meta
	 * children["_name"] == name
	 *
	 * */
//	Node *next = nullptr;// NO, WE NEED TRIPLES cause objects can occur in many lists + danger: don't use for any ref/var


	//	Node(chars);
//	Node(va_list args) {
//	}

	void *operator new(size_t size) {
		return (Node *) (calloc(size, 1));// WOW THAT WORKS!!!
	}

	void operator delete(void *a) {
		todo("delete Node");
	}
//	~Node()= default; // destructor
//	virtual ~Node() = default;


	Node() {
		kind = objects;
//		if(debug)name = "[]";
	}


	explicit Node(String *args) {// initiator list C style {x,y,z,0} ZERO 0 ø TERMINATED!!
		while (args[length] and length < MAX_DATA_LENGTH) {
			children[length] = Node(args[length]);
			length++;
		}
		kind = groups;
    }

    explicit Node(int buffer[]) {
        value.data = buffer;
        kind = buffers;
//		todo ("type of array");
//		if (debug)name = "int[]";
//			buffer.encoding = "a85";
    }

    explicit
    Node(long long value, Type64 type64) {
        this->value.longy = value;// can also be double bytes:
        this->kind = (Kind) type64.value; // todo other types!
    }


    explicit
    Node(char c, bool withValue = false /*todo why not, ever? */ ) {
        name = String(c);
        if (withValue) {
            value.longy = c;
            kind = codepoints;
        }
        // todo ^^ keep!
    }

    explicit
    Node(double nr) {
		value.real = nr;
		kind = reals;
		if (debug) name = String(ftoa(nr)); // messes with setField contraction
//			name = String(itoa0(nr, 10)); // messes with setField contraction
	}

    explicit Node(int nr) : Node((long long) nr) {}

    explicit Node(Kind type) : Node() { kind = type; }

	explicit Node(float nr) : Node((double) nr) {}

// how to find how many no. of arguments actually passed to the function? YOU CAN'T! So …
// Pass the number of arguments as the first variable OR
// Require the last variable argument to be null, zero or whatever
// BOXED
	explicit Node(int a, int b, ...) {
		kind = objects;// groups list
		add(Node(a).clone());
		va_list args;// WORK WITHOUT WASI!!
		va_start(args, b);
		int i = b;
		while (i) {
			addSmart(Node(i).clone());
			i = (int) va_arg(args, int);
		}
		va_end(args);
	}

// stupid little helper function to create ad-hoc arrays,
// MUST NOT CONTAIN 0, as it is
// 0 terminated!! MUST END WITH 0
// Node(1,2,3,0) == [1,2,3]
//	explicit Node(int a, int b, ...) {
//		kind=buffers;//int_array;
//		value.data= malloc(10*sizeof(int));
//		((int*)value.data)[length++]=a;
//		// ⚠️ don't abuse node length for array length !?! children[length-1] might CRASH!
//		va_list args;// WORK WITHOUT WASI!!
//		va_start(args, b);// needed to distinguish Node(int nr)
//		int i = b;
//		while (i) {
//			((int*)value.data)[length++]=i;
//			i = (int) va_arg(args, int);
//			if(length%10==0)value.data= malloc(10*sizeof(int));
//		}
//		va_end(args);
//		length=0;// no children! but where is length??
//	}

#ifndef WASM
	Node(const std::initializer_list<String> &_items) : Node() {
		for (const String &s: _items) {
			add(new Node(s));
		}
	}
#endif

	// why not auto null terminated on mac?
	// vargs needs to be 0 terminated, otherwise pray!
	explicit
	Node(char *a, char *b, ...) {
		kind = objects;// groups list
		add(Node(a).clone());
		va_list args;
		va_start(args, b);
		char *i = b;
		while (i) {
			Node *node = Node(i).clone();
			add(node);
			i = (char *) va_arg(args, char*);
		}
		va_end(args);
	}


	explicit
	Node(long long nr) { // stupild
		value.longy = nr;
		kind = longs;
		if (debug)name = String(itoa(nr)); // messes with setField contraction
	}

	explicit
	Node(long nr) { // stupild
		value.longy = nr;
		kind = longs;
		if (debug)name = String(itoa(nr)); // messes with setField contraction
	}

	explicit // wow without explicit everything breaks WHY?
	Node(bool yes) {
		if (yes)
			*this = True;
		else
			*this = False;
	}


	explicit Node(chars name) {
		this->name = *new String(name);
//		type = strings NAH;// unless otherwise specified!
	}

	Node(String name, ::Kind type) {
		this->name = name;
		this->kind = type;
	}
//	explicit Node(bool truth) {
//		error("DONT USE CONSTRUCTION, USE ok?True:False"); // todo : can't we auto-cast?  Node &bool::operator(){return True;}
//		if (this == &NIL)
//			name = "HOW";
//		value.number = truth;
//		type = numbers;
//		if (debug)name = truth ? "✔️" : "✖️";
////		if (debug)name = nr ? "true" : "false";
////		this=True; todo
//	}


	explicit Node(String s, bool identifier = false) {
//		identifier = identifier || !s.contains(" "); BULLSHIT 'hi' is strings!!
		if (identifier) {
//			if(check_reference and not symbol)...
            name = s;
            kind = reference;// kind / type must be concretized once value type is known!
        }
//		else if (atoi(s) and s == itoa0(atoi(s))) {
//			value.number = atoi(s);
//			type = numbers;
//			}
//		else if (atof(s)) { value.real = atoi(s); }
		else {
			kind = unknown;
//			kind = strings;
			value.string = new String(s.data, s.length, true);// todo COPY AGAIN!?
			if (name == empty_name)name = s;
		}
	}

	explicit Node(Node **pNode) {
		children = pNode[0];
		kind = arrays;
		value.data = pNode[0];
	}

	explicit
	Node(codepoint c) {
		name = String(c);
		value.longy = c;
		kind = codepoints;
//		value.string = &name;// todo uh, no, and danger! change name=>change value? hell no!
//		kind = strings;
	}

	explicit
	Node(smart_pointer_32 spo) {
		smartType4bit type4Bit = getSmartType(spo);
		int payload = spo << 4 >> 4; // delete 4 bit type header
//		if (type != int28 and type != float28 )payload = spo << 8 >> 8;
		switch (type4Bit) {
			case int28:
			case sint28:
				value.longy = (int) spo;
				kind = longs;
				break;
			case float28:
				value.real = payload;// todo!
				kind = reals;
				break;
			case anys:
				*this = memory[payload];// todo
				break;
			case symbola:
				name = String(&memoryChars[payload]); // todo 0x10 ... 0x1F or length header
				kind = reference;
				break;
			case stringa:
				value.string = new String(&memoryChars[payload]);
				name = *value.string;
				kind = strings;
				break;
			case codes:
			case utf8char:
				value.string = new String((wchar_t) payload);
				name = *value.string;
				kind = strings;
				break;
			default:
				error("unknown or unimplemented smart type");
        }
    }

    long hash() {
        static int _object_count = 1;
//		if (not _hash) _hash = random();//  expensive?
        if (not _hash)_hash = _object_count++;// //  (long) (void *) this; could conflict on memory reuse
        return _hash;
    }

    Node *invoke(String function, Node *arguments) {
        todo("dynamic dispatch");
        // i32.const fun
        // call_indirect(type,table)
    }

    Node &first() {
        if (length > 0)return children[0];
        if (children)return children[0]; // hack for missing length!
        if (kind == assignment and value.node)return *value.node;// todo sure??, could be direct type!?
        if (kind == operators and next)return *next;// todo remove hack
        return *this;// (x)==x   danger: loops
//		error("No such element");
//		return ERROR;
    }

//	if you do not declare a copy constructor, the compiler gives you one implicitly.
//	Node( Node& other ){// copy constructor!!

	Node *clone(bool childs = true) {// const cast?
		if (this == &NIL)return this;
		if (this == &True)return this;
		if (this == &False)return this;
		// todo ...
		Node *copy = new Node();
		*copy = *this;// ok copies all values
		copy->type = type;//

// Todo: deep cloning whole tree? definitely clone children
		if (childs) {
            if (kind == key and value.data)
                copy->value.node = value.node->clone(false);
            copy->children = 0;
            copy->length = 0;
            if (length > 0)
                for (Node &n: *this)
                    copy->add(n);// necessary, else children is the same pointer!
        }
		return copy;
	}

	//	 explicit copy operator not neccessary
//	Node& operator=(Node val){
//		this->name = val.name;
//		this->value = val.value;
//		this->kind = val.kind;
//	}

//	Node &operator=(Node& n);
//	 DOESN'T work like this: referenceIndices are always set via internal reference mechanism, this is for value copy!
//	Node& operator=(Node& val){
//		if(name and kind==nils){
//			this->kind = reference;
//			this->value.node = &val;
//		} else{
//			this->name = val.name;
//			this->value = val.value;
//			this->kind = val.kind;
//		}
//		return *this;
//	}

	bool operator==(bool other);

	bool operator==(char other);

	bool operator==(int other);

	bool operator==(long other);

	bool operator==(float other);

	bool operator==(double other);

	bool operator==(chars other);

	bool operator==(String other);

//	bool operator==(Node other);
	bool operator==(Node &other);// equals
	bool operator==(Node *other);// equals

	bool operator==(const Node &other);// equals


	bool operator!=(Node other);

	bool operator>(Node other);

	Node operator+(Node other);

//	bool operator!=(Node &other);// why not auto

//	 +=, -=, *=, /=, %=, <<=, >>=, &=, |=, ^=
//	bool operator<<=(Node &other);// EVIL MAGIC ;)
//	bool operator~();// MUST BE UNITARY:(


	String string() const {
		if (kind == strings)
			return *value.string;
		return name;
		error((char *) (String("WRONG TYPE ") + String(kind)));
	}

	// moved outside because circular dependency
	Node &operator[](int i);

	Node &operator[](int i) const;

	Node &operator[](char c);

	Node &operator[](chars s);

//	Node &operator[](String s);
	Node &operator[](String *s);

//	Node &operator[](String s) const;

	Node &operator=(int i);

	Node &operator=(chars s);


	Node &set(String string, Node *node);

	Node interpret(bool expectOperator = false);

	Node insert(Node &node, int at = -1);// non-modifying


	//	Node &add(Node node);  call to member function 'add' is ambiguous
	Node &add(const Node *node);

	Node &add(const Node &node);

//	void addSmart(Node &node);// modifying
	void addSmart(Node node);

	void addSmart(Node *node, bool flatten = true, bool clutch = false);

	void remove(Node *node); // directly from children
	void remove(Node &node); // via compare children

	// danger: iterate by value or by reference?
	[[nodiscard]] Node *begin() const;

	[[nodiscard]] Node *end() const;

	Node &merge(Node &other);// non-modifying
	Node &merge(Node *other);

	void print(bool internal_representation = false) {
		printf("%s\n", serialize().data);
		if (internal_representation) {
			printf("node{");
			if (this == &NIL || kind == nils) {
				printf("NIL\n");
				return;
			}
			if (name.data)
				printf("name:%s", name.data);
			printf(" length:%d", length);
			if (kind < unknown)
				printf(" type:%s", typeName(kind));
			const String &string1 = serializeValue(false);
			printf(" value:%s\n", string1.data);// NEEDS "%s", otherwise HACKABLE
			printf(" children:[");// flat, non-recursive
			for (int i = 0; i < min(length, 10); i++) {
				Node &node = children[i];
				if (!node.name.empty()) {
					printf("%s", node.name.data);
					printf(" ");
				} else printf("{…} ");// overview
			}
			printf("]");
			printf("}\n");
		}
	}

	Node apply_op(Node left, Node op0, Node right);


	long long numbere() {
		return kind == longs or kind == bools ? value.longy : value.real;// danger
	}

	float floate() {
		return kind == longs ? value.longy : value.real;// danger
	}

	Node *has(String s, bool searchMeta = true, short searchDepth = 0) const;


//		https://github.com/pannous/angle/wiki/truthiness
//		if(name=="nil")return false;
//		if(name=="0")return false;
	explicit operator bool() {// TRUTHINESS operator, implicit in if, while
		return value.longy or length > 1 or (length == 1 and this != children and (bool) (children[0]));
	}

	// type conversions
	explicit operator bool() const {
		return value.longy or length > 1 or (length == 1 and this != children and (bool) (children[0]));
	}

	explicit operator int() const { return value.longy; }

	explicit operator long() const { return value.longy; }

	explicit operator float() const { return value.real; }

	explicit operator String() const { return *value.string; }

	explicit operator char *() const { return kind == strings ? value.string->data : name.data; }// todo: unsafe BS

	Node &last();

//	bool empty();// same:
	bool isEmpty();

	bool isNil() const;

	chars toString();

	chars toString() const;

	String serialize() const;

	String serializeValue(bool deep = true) const;


	Node &setValue(Value v);


	Node &from(Node &node);// exclusive
	Node &from(String match);

	Node &from(int pos);

	Node &to(Node match);// exclusive
	Node &to(String match);

	Node &flat();

	Node &setName(char *name0);

	Node &values();

	bool isSetter();

	int lastIndex(String &string, int start = 0);

    int lastIndex(Node *node, int start = 0);

    int index(String string, int start = 0, bool reverse = false);

    void replace(int from, int to, Node &node);

    void replace(int from, int to, Node *node);

    void remove(int at, int to);

    Node &metas() {
        // todo List<Node> ?
        if (!meta)meta = new Node(Kind::meta);
        return *meta;
    }

    Node &setType(::Kind type, bool check = true);

    Node &setType(const char *string) {// setClass
//		type = &Node(string).setType(classe);
        return *this;
    }

    Node &setType(Node *_type) {
//		type = &_type->setType(classe);
        return *this;
    }

    List<String> &toList();

    bool empty();

    void clear();

    String *Line();

    void addMeta(Node *pNode);
};

typedef const Node Nodec;

void initSymbols();// wasm doesn't do it why!?
class [[maybe_unused]] BoolBridge {
	bool _b;
public:
//	Single-argument constructors must be marked explicit to avoid unintentional implicit conversions
// but this is INTENTIONAL! doesn't work though
	BoolBridge(bool b) { _b = b; }

	operator Node &() { return _b ? True : False; }
};

Node interpret(Node &n);

Node eval(String n);


struct smart_value{
    Type smartType;
    Value value;
};
