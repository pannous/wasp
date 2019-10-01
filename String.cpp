//
// Created by pannous on 24.10.18.
//

//#ifndef NETBASE_STRING_CPP
//#define NETBASE_STRING_CPP
//#include "String.h" prefering one big WASM object

//void *malloc(int size);

//#include <malloc.h>
#define let auto
#define var auto
typedef void *any;
typedef unsigned char byte;
typedef const char *chars;
//typedef int size_t;

extern unsigned int *memory;
extern unsigned int *&heap;

extern "C" void logs(const char *);
//extern void *malloc (size_t __size);
//extern void *malloc (int __size);

#ifndef WASM

#include <cstdio>

void raise(chars error);

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
		if(n<0 or n>9)
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
	String() {
		length = 0;
	}

	String(const char string[]) {
		data = const_cast<char *>(string);
		length = len(data);
	}

	String(char c) {
		data = static_cast<char *>(malloc(2));
		data[0] = c;
		data[1]=0;
		length = 1;
	}

	explicit String(int c) {
		data = itoa(c);
		length = len(data);
	}

	explicit String(long c) {
		data = itoa(c);
		length = len(data);
	}


	char charAt(int i) {
		if (i >= length)
			raise((String("IndexOutOfBounds at ") + itoa(i) + " in " + data).data);
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

	String substring(int from, int to) {
		auto *neu = static_cast<char *>(malloc((sizeof(char))*( to - from)));
		strcpy(neu, &data[from], to - from);
		neu[to - from]=0;
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


	String *operator+=(char c) {
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
		return this;
	}

	String operator+(String c) {
		auto *neu = static_cast<char *>(malloc(length + c.length + 1));
		if (data)strcpy(neu, data, length);
		if (c.data)strcpy(neu + length, c.data, c.length);
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

	String operator+(char* c) {
		return this->operator+(String(c));
	}

	bool operator==(char c) {
		return !empty() && this[0] == c && this[1] == '\0';
	}

	bool operator==(String s) {
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

	operator chars() {
		return data;
	}

	char *data{};
	int length = -1;

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
	bool contains(chars string){
		return indexOf(string)>=0;
	}

	String replace(chars string, chars with) {
		int i = this->indexOf(string);
		if (i >= 0)
			return String(substring(0, i) + with + substring(i + len(string), length));
		else
			return *this;
	}
};

//class NodeMap{
//
//};

enum Type {
	object=0,
	strings,
	arrays,// same as:
	buffers,
	nodes,
	longs,
	bools,
	floats,
	ints,
	nils,
	unknown = 20 //7
};

class Node;

union Value {
	Node *node;
	String string;
	long longy = 0;
	float floaty;
	void *data;

	Value() {}// = default;
};

struct Entry;

class Node {
public:

//	Node(const char*);

	Value value;
	Node *parent = 0;
	Entry *children = nullptr;
	Type type = unknown;
	int length = 0;

	Node() {
		type = object;
	}


	explicit Node(int buffer[]) {
		value.data = buffer;
		type = buffers;
//			buffer.encoding = "a85";
	}

	explicit Node(long nr) {
		value.longy = nr;
		type = longs;
	}


	explicit Node(String s) {
		type = strings;
		value.string = s;
	}

	explicit Node(Node **pNode) {
		type = arrays;
		value.data = pNode;
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
		throw "WRING TYPE";
	}

	// moved outside because circular dependency
	Node& operator[](int i);
	Node& operator[](char c);
	Node& operator[](String s);

	Node& operator=(int i);

	Node& operator=(chars s);

	Node& set(String string, Node node);
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

struct Entry {
	String key;
	Node value;
};

Node& Node::operator=(int i){
	this->value.longy = i;
	this->type = longs;
	return *this;
}

Node& Node::operator=(chars c){
	this->value.string = String(c);
	this->type = strings;
	return *this;
}

Node& Node::operator[](int i) {
	if(i>=length)throw String("out of range ") +  i + " > " + length;
	return this->children[i].value;
}

Node& Node::operator[](String s) {
	for (int i = 0; i < length; i++) {
		Entry &entry = this->children[i];
		if (s == entry.key)
			return entry.value;
	}
//		raise("NO SUCH KEY");
	return set(s, Node());// for n["a"]=b // todo: return DANGLING/NIL
//	return NIL;
}


Node& Node::operator[](char c){
	return (*this)[String(c)];
}

Node& Node::set(String string, Node node) {
	if (!children)children = static_cast<Entry *>(malloc(1000));
	Entry &entry = children[this->length];
	entry.key = string;
	entry.value = node;
	node.parent = this;
	this->length++;
	return entry.value;
}

//Node::Node(const char *string) {
//	this->value.string = String(string);
//	this->type = strings;
//}

bool Node::operator==(String other) {
	return this->type==strings and this->value.string == other;
}
bool Node::operator==(int other) {
	return (this->type==longs and this->value.longy == other) or (this->type==floats and value.floaty == other);
}
bool Node::operator==(long other) {
	return (this->type==longs and this->value.longy == other) or (this->type==floats and value.floaty == other);
}
bool Node::operator==(double other) {
	return (this->type==floats and this->value.floaty == ((float )other)) or (this->type==longs and this->value.longy == other) ;
}
bool Node::operator==(float other) {
	return (this->type==floats and this->value.floaty == other) or (this->type==longs and this->value.longy == other) ;
}

bool Node::operator==(Node other) {
	for (int i = 0; i < length; i++) {
		Entry &field = this->children[i];
		Node &val = other[field.key];
		if(field.value != val )
			return false;
	}
	if(type != other.type)
		return false;
	if(type==strings)
		return value.string == other.value.string;
	return value.floaty == other.value.floaty;
}
bool Node::operator!=(Node other) {
	return not(*this == other);
}


void log(long i) {
	printf("%li", i);
}
void log(chars s) {
	printf("%s\n",s);
}
void log(Node n) {
//	if(!n0)
//		return;
//	Node n=*n0;
	printf("Node ");
	if (&n == &NIL || n.type == nils) {
		printf("NIL\n");
		return;
	}
	printf("length %i ", n.length);
	printf("type %i ", n.type);
	if (&n == &True)
		printf("TRUE");
	if (&n == &False)
		printf("FALSE");
	if (n.type == object and n.value.data)
		printf(" name %s", n.value.string.data);
	if (n.type == bools)
		printf(" value %s", n.value.longy ? "TRUE" : "FALSE");
	if (n.type == strings)
		printf(" value %s", n.value.string.data);
	if (n.type == longs)
		printf(" value %li", n.value.longy);
	if (n.type == floats)
		printf(" value %f", n.value.floaty);
	printf("\n");
}

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


String operator "" s(const char* c, size_t){
	return String(c);// "bla"s  literal operator suffixes not preceded by ‘_’ are reserved for future standardization
}

String operator "" _s(const char* c, size_t){
	return String(c);
}
String operator "" _(const char* c, unsigned long t){
	return String(c);
}
