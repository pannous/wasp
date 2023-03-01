#pragma once
//
// Created by pannous on 30.09.19.
//

#include "String.h"
#include "smart_types.h"
#include "NodeTypes.h"
#include "ABI.h"
#include "Config.h"

#include <cstdarg> // va_list OK IN WASM even without wasi!
#include <cstdlib> // OK in WASM!
#include <cstdio>

#ifndef WASM
#include <initializer_list> // allow List x={1,2,3};
#endif

#define NODE_DEFAULT_CAPACITY 100

typedef char const *chars;
typedef unsigned char byte;//!

//extern bool use_wasm_reference_types; :
// WE DON'T NEED THIS, we can just use CANONICAL ABI lowering, e.g. for strings: [i32, i32]
extern bool use_wasm_structs;
extern bool use_wasm_strings;
extern bool use_wasm_arrays;


#define MAX_NODE_CAPACITY 100000 // debug only, let it run out of memory naturally!
static int lastChild = 1;

Node *reconstructWasmNode(wasm_node_index pointer);

// todo
enum class NodeFlags { // class makes it typesafe (??)
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



typedef unsigned char byte;
typedef chars chars;



class Node;

extern const Node NIL;
static Node &NUL = const_cast<Node &>(NIL);
extern Node True;
extern Node False;
extern Node ERROR;
extern Node Ignore;
extern Node Infinity;// = Node("Infinity");
extern Node NegInfinity;// = Node("Infinity");
extern Node Nan;// = Node("NaN");

// math.h:
//#define NAN       __builtin_nanf("")
//#define INFINITY  __builtin_inff()
//#define NAN       (0.0f/0.0f)
//#define INFINITY  1e5000f

void print(Node &n);
void print(Node *n0);
//void print(const Node &);

#undef Function
//class Function readline.h:40:15: note: 'Function' declared here WTF

class Module;
//class String;

// ⚠️ A Value is only useful with accompanying Type!
union Value { // node_pointer can contain ANYTHING, especially types known in wasp
//	sizeof(Value)==8 (int64)
    Node *node = 0;// todo DANGER, pointer can be lost :(   // todo same as child
//	Node *child = 0; //todo DANGER child and next are NOT REDUNDANT! (a:b c:d) == a(value=b next=c(value=d))
//	Node **children = 0;// keep children separate for complex key node_pointer (a b c):(d e f)
    String *string;// todo: wasm_chars*
//    Function *function;
    Module *module;

    void *data;// any bytes
    int64 longy;
    codepoint codepoint;//chary;// use longy
    double real;

//	Value() {}// = default;
//	explicit Value(int i) { longy = i; }
//
//	explicit Value(int64 i) { longy = i; }
//
//	Value(bool b) { longy = b; }
//
//	Value(double r) { real = r; }
//
////	Value(String s) { string = &s; }
//
//	Value(String &s) { string = &s; }
//
//	Value(String *s) { string = s; }

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

struct debug_struct {
    byte a1;
    byte a12;
    byte a13;
    byte a14;
    byte a15;
    byte a16;
    byte a17;
    byte a11;
    byte a21;
    byte a31;
    byte a41;
    byte a51;
    byte a61;
    byte a71;
    int64 rest1;
    int64 rest2;
    int64 rest3;
};

// in wasm32 pointers are int instead of int64, use this struct to transfer between both Node* spaces
// reconcilable with Node INSIDE wasm. ALWAYS PAIR ints so that they align with int64, in 32 AND 64 bit modes!
struct wasm32_node_struct {
    int node_header = node_header_32;
    int length; // number of children ≠ sizeof(Node)
    int node_type_pointer;// int64 in 64 bit systems!
    int child_pointer;    // int64 in 64 bit systems!
    Value value;// int64 ok
    Kind kind;  // int32 TODO needs padding in Node ( 64 bit ) how? PUT IN node_type(pointer). MIX GENERICS complicated :(
    int meta_pointer;
    // previous fields must be aligned to int64!
    int name_pointer;
    int name_length;
//    …
//    String name;// ok 64 bit compatible (except data which needs to be relocated to/fro wasm_memory anyways ) !
    // ignore rest for now!
};

// The order of Type,Value is reverseInPlace to the Wasp ABI return tuple Value(int32), Type(int32)
// hovering over it yields size(120), copy/move : construction, assignment
class Node {
    // todo: sizeof(Node) can be reduced later by: shrinking header, merging type&kind, let *children own its length, make name String* offset
public:
    // ⚠️ ORDER of fields matter when (de)serializing / casting from List …
//	static
    //	short _node_header_ = 0xDADA; // can be combined with byte kind => 2*short !
    int node_header = node_header_32;
//    Type64 type6;
    int length = 0;// #children
    Kind kind = unknown;// forced 32 bit,  improved from 'undefined' upon construction
    Node *children = nullptr;// GROW LATER! nullptr;// LIST, not link. block body content
    Value value = {.longy=0}; // value.node and next are NOT REDUNDANT  label(for:password):'Passwort' but children could be merged!?
//    32bit in wasm TODO pad with string in 64 bit
    // previous fields must be aligned to int64!
    String name = EMPTY;// nil_name;
    Node *type = 0;// variable/reference type or object class?
    Node *meta = 0;//  LINK, not list. attributes meta modifiers decorators annotations

    // todo rename and alias:
    // lets lads lats lates: lets because a=b;c=d …; lads children; lats laterals; lates delayed evaluation
    // the rest can be reconstructed / ignored
    Node *parent = nullptr;// strange order necessary for alignment of String struct!?!
    Node *next = 0; // in children list, redundant with children[i+1] => for debugging only
    char separator = 0;// " " ";" ","
//	char grouper = 0;// "()", "{}", "[]" via kind!  «…» via type Group("«…»")

    int capacity = NODE_DEFAULT_CAPACITY;
    int64 _hash = 0;// set by hash(); should copy! on *x=node / clone()
#ifdef DEBUG
// int code_position; // hash to external map
//	int lineNumber;
    String *line = 0;// debug! EXPENSIVE for non ast node_pointer!
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
#if WASM
        //        while ((long) current % 8)current++;// align
                return aligned_alloc(8,size);
#else
        return (Node *) (calloc(size, 1));// WOW THAT WORKS!!!
#endif
    }

    void operator delete(void *a) {
//        delete a;
        free(a);
        todo("delete Node");
    }
//	~Node()= default; // destructor
//	virtual ~Node() = default;


    Node() {
        kind = objects;
//		if(debug)name = "[]";
    }

    void init_children(int nr = -1) {
        if (nr < 0)nr = capacity;
        if (!children)children = (Node *) calloc(capacity, sizeof(Node));
    }


    explicit Node(String *args) : Node(*args) {
    }

//    explicit Node(String *args) BAD! {// initiator list C style {x,y,z,0} ZERO 0 ø TERMINATED!!
//        init_children();
//        while (args[length] and length < MAX_WASM_DATA_LENGTH) {
//            children[length] = Node(args[length]);
//            length++;
//        }
//        kind = groups;
//    }

    explicit Node(int buffer[]) {
        value.data = buffer;
        kind = buffers;
//		todo ("type of array");
//		if (debug)name = "int[]";
//			buffer.encoding = "a85";
    }

    explicit
    Node(int64 value, Type64 type64) {
        this->value.longy = value;// can also be double bytes:
        this->kind = (Kind) type64.value; // todo other types!
    }


    explicit
    Node(char c) {
        name = String(c);
        value.longy = c;
        kind = codepoint1;
    }

    explicit
    Node(char16_t c) {
        name = String(c);
        value.longy = c;
        kind = codepoint1;
    }

    explicit Node(double nr) {
        value.real = nr;
        kind = reals;
        if (debug) name = String(ftoa(nr)); // messes with setField contraction
//			name = String(itoa0(nr, 10)); // messes with setField contraction
    }

    explicit Node(int nr) : Node((int64) nr) {}

    explicit Node(Kind type) : Node() { kind = type; }

    explicit Node(float nr) : Node((double) nr) {}

// how to find how many no. of arguments actually passed to the function? YOU CAN'T! So …
// Pass the number of arguments as the first variable OR
// Require the last variable argument to be null, zero or whatever
// BOXED
    explicit Node(int a, int b, ...) {
        init_children();
        kind = objects;// groups list
        add(Node(a));
        va_list args;// WORK WITHOUT WASI!!
        va_start(args, b);
        int i = b;
        while (i) {
            add(Node(i));
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
    Node(chars a, chars b, ...) {
        kind = objects;// groups list
        add(Node(a).clone());
        va_list args;
        va_start(args, b);
        chars i = b;
        while (i) {
            Node *node = Node(i).clone();
            add(node);
            i = (char *) va_arg(args, char*);
        }
        va_end(args);
    }


    explicit
    Node(int64 nr) { // stupild
        value.longy = nr;
        kind = longs;
        if (debug)name = String(formatLong(nr)); // messes with setField contraction
    }


    explicit // wow without explicit everything breaks WHY? because Node(void*) goes here!!!
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
//            kind = unknown;
            kind = strings;
            value.string = new String(s.data, s.length, true);// todo COPY AGAIN!?
            if (name == EMPTY)name = s;
        }
    }

    explicit Node(Node **pNode) {
        children = pNode[0];
        kind = arrays;
        value.data = pNode[0];
    }

    explicit Node(codepoint c) {
        name = String(c);
        value.longy = c;
        kind = codepoint1;
//		value.string = &name;// todo uh, no, and danger! change name=>change value? hell no!
//		kind = strings;
    }


    explicit Node(smart_pointer_64 smart) {
        *this = *smartNode(smart);
    }

//    Node(wasm_node_index index) {
//        *this = *reconstructWasmNode(index);
//    }

// ⚠️ c polymorphism NOT TYPE SAFE: all unsigned int are treated as smart_pointer_32 !!
    explicit
    Node(smart_pointer_32 spo) {
        smartType4bit type4Bit = getSmartType(spo);
        int payload = spo << 4 >> 4; // delete 4 bit type header
//		if (type != int28 and type != float28 )payload = spo << 8 >> 8;
        switch (type4Bit) {
//            case 0:  duplicate case value: '0' and 'int28' both equal '0'
//                *this = *reconstructWasmNode(payload);
//                break;
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
                name = String(&((char *) memory)[payload]); // todo 0x10 ... 0x1F or length header
                kind = reference;
                break;
            case stringa:
                value.string = new String(&((char *) memory)[payload]);
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

    int64 hash() {
        static int _object_count = 1;
//		if (not _hash) _hash = random();//  expensive?
        if (not _hash)_hash = _object_count++;// //  (int64) (void *) this; could conflict on memory reuse
        return _hash;
    }

    Node *invoke(String function, Node *arguments) {
        print(function);
        print(arguments);
        todo("dynamic dispatch");
        // i32.const fun
        // call_indirect(type,table)
        return this;
    }

    Node &first() {
        if (length > 0 and children)
            return children[0];
//        if (children)
//            return children[0]; // hack for missing length!
        if (kind == assignment and value.node)
            return *value.node;// todo sure??, could be direct type!?
        if (kind == operators and next)
            return *next;// todo remove hack
        if (kind == key and value.node)
            return *value.node;
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

//	Node &operator^(Node& n)
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

    bool operator==(int64 other);

    bool operator==(float other);

    bool operator==(double other);

    bool operator==(chars other);

    bool operator==(String other);

    bool operator==(codepoint other);

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
        if (kind == codepoint1)
            return String((char32_t) value.longy);
        if (kind == strings)
            return *value.string;
        return name;
        error((char *) (String("WRONG TYPE ") + String((int64) kind)));
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
//	Node &add(Node node);

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
            if (children)
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


    int64 numbere() {
        return kind == longs or kind == bools ? value.longy : value.real;// danger
    }

    float floate() {
        return kind == longs ? value.longy : value.real;// danger
    }

    Node *has(String s, bool searchMeta = true, short searchDepth = 0) const;


//		https://github.com/pannous/angle/wiki/truthiness
//		if(name=="nil")return false;
//		if(name=="0")return false;
//    explicit operator bool() {// TRUTHINESS operator, implicit in if, while
//        return value.longy or length > 1 or (length == 1 and children and this != children and (bool) (children[0]));
//    }

    // type conversions
    explicit operator bool() const {
        return value.longy or length > 1 or (length == 1 and children and this != children and (bool) (children[0]));
    }


    // todo ⚠️ NEVER CAST Node* to smart_pointer_64, THIS IS NOT WHAT WE WANT!!
//    explicit operator smart_pointer_64 (){
//        if(smart_pointer_header_mask & (smart_pointer_64)this)
//            error("Node pointer out of reach > 2^48 ");
//        return (smart_pointer_64)this & smart_pointer_node_signature;
//    }

    smart_pointer_64 toSmartPointer() {
        if (kind == longs)
            return value.longy;
        if (smart_pointer_header_mask & (smart_pointer_64) this)
            error("Node pointer out of reach > 2^48 ");
        return (smart_pointer_64) this | smart_pointer_node_signature;
    }

    explicit operator int() const { return value.longy; }

    explicit operator int64() const { return value.longy; }

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

    chars serializeValue(bool deep = true) const;


    Node &setValue(Value v);

    Node &setValue(int64 v);


    Node &from(Node &node);// exclusive
    Node &from(String match);

    Node &from(int pos);

    Node &to(Node match);// exclusive
    Node &to(String match);

    Node &flat();

    Node &setName(chars name0);

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

    Node &setType(Kind kin, bool check = true);

//    Node &setType(const char *string) {// setClass
//        if(types.has)
////		type = &Node(string).setType(classe);
//        return *this;
//    }

    Node &setType(Node *_type) {
        type = _type;
        return *this;
    }

    List<String> &toList();

    bool empty();

    void clear();

    String *Line();

    void addMeta(Node *pNode);

    bool contains(const char *string);

    chars containsAny(List<chars> strings);

    int size();

    Node &childs();

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


struct smart_value {
    Type smartType;
    Value value;
};

smart_pointer_64 toSmartPointer(Node *n);

void debugNode(Node &n);
