////
//// Created by me on 30.09.19.
////
//
////#include "Node.h"
//
//
//class Node {
//public:
//
////	Node(const char*);
//
//	Value value;
//	Node *parent = 0;
//	Entry *children = nullptr;
//	Type type = unknown;
//	int length = 0;
//
//	Node() {
//		type = object;
//	}
//
//
//	explicit Node(int buffer[]) {
//		value.data = buffer;
//		type = buffers;
////			buffer.encoding = "a85";
//	}
//
//	explicit Node(long nr) {
//		value.longy = nr;
//		type = longs;
//	}
//
//
//	explicit Node(int i) {
//		value.longy = i;
//		type = longs;
//	}
//
//	explicit Node(String s) {
//		type = strings;
//		value.string = s;
//	}
//
//	explicit Node(Node **pNode) {
//		type = arrays;
//		value.data = pNode;
//	}
//
//	bool operator==(float other);
//	bool operator==(long other);
//	bool operator==(String other);
//	bool operator==(Node other);
//	bool operator!=(Node other);
//
//
////	Node(bool b) {
////		Value(b ? 0 : 1);
////	}
//
//	String string() {
//		if (type == strings)
//			return value.string;
//		throw "WRING TYPE";
//	}
//
//	// moved outside because circular dependency
//	Node& operator[](int i);
//	Node& operator[](char c);
//	Node& operator[](String s);
//
//	Node& operator=(int i);
//
//	Node& operator=(chars s);
//
//	Node& set(String string, Node node);
//};
//
////
////String operator "" s(const char* c, size_t){
////return String(c);// "bla"s
////}
//
////String operator ""_s(const char* c, size_t){
////	return String(c);// "bla"_s
////}
//
////new String();
////auto ws = {' ', '\t', '\r', '\n'};
//Node Infinity = Node("Infinity");
//String EMPTY = String('\0');
//Node NaN = Node("NaN");
////NIL=0;
////Node NIL;
//Node NIL = Node("NIL");
//Node True = Node("True");
//Node False = Node("False");
//
//struct Entry {
//	String key;
//	Node value;
//};
//
//Node& Node::operator=(int i){
//	this->value.longy = i;
//	this->type = longs;
//	return *this;
//}
//
//Node& Node::operator=(chars c){
//	this->value.string = String(c);
//	this->type = strings;
//	return *this;
//}
//
//Node& Node::operator[](int i) {
//	if(i>=length)throw String("out of range ") +  i + " > " + length;
//	return this->children[i].value;
//}
//
//Node& Node::operator[](String s) {
//	for (int i = 0; i < length; i++) {
//		Entry &entry = this->children[i];
//		if (s == entry.key)
//			return entry.value;
//	}
////		raise("NO SUCH KEY");
//	return set(s, Node());// for n["a"]=b // todo: return DANGLING/NIL
////	return NIL;
//}
//
//
//Node& Node::operator[](char c){
//	return (*this)[String(c)];
//}
//
//Node& Node::set(String string, Node node) {
//	if (!children)children = static_cast<Entry *>(malloc(1000));
//	Entry &entry = children[this->length];
//	entry.key = string;
//	entry.value = node;
//	node.parent = this;
//	this->length++;
//	return entry.value;
//}
//
////Node::Node(const char *string) {
////	this->value.string = String(string);
////	this->type = strings;
////}
//
//bool Node::operator==(String other) {
//	return this->type==strings and this->value.string == other;
//}
//
//bool Node::operator==(long other) {
//	return (this->type==longs and this->value.longy == other) or (this->type==floats and value.floaty == other);
//}
//
//bool Node::operator==(float other) {
//	return (this->type==floats and this->value.floaty == other) or (this->type==longs and this->value.longy == other) ;
//}
//
//bool Node::operator==(Node other) {
//	for (int i = 0; i < length; i++) {
//		Entry &field = this->children[i];
//		if(field.value == other[field.key])
//			return false;
//	}
//	if(type != other.type)
//		return false;
//	if(type==strings)
//		return value.string == other.value.string;
//	return value.floaty == other.value.floaty;
//}
//bool Node::operator!=(Node other) {
//	return not(*this == other);
//}
