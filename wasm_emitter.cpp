//https://blog.sentry.io/2020/08/11/the-pain-of-debugging-webassembly
//https://github.com/mdn/webassembly-examples/tree/master/understanding-text-format
// BASED ON https://github.com/ColinEberhardt/chasm/blob/master/src/emitter.ts
// https://github.com/ColinEberhardt/chasm/blob/master/src/encoding.ts
// https://pengowray.github.io/wasm-ops/
#include "Wasp.h"
#include "String.h"
#include "Map.h"
#include "wasm_emitter.h"
#include "wasm_helpers.h"
#include "wasm_runner.h"

#define check(test) if(test){log("OK check passes: ");log(#test);}else{printf("\nNOT PASSING %s\n%s:%d\n",#test,__FILE__,__LINE__);exit(0);}

Valtype last_type = voids;// autocast if not int
Map<String, Valtype> return_types;
//Map<int, List<String>> locals;
Map<String /*function*/, List<String> /* implicit indices 0,1,2,… */> locals;
Map<String, Signature> functionSignatures;
//Map<int, Map<int, String>> locals;
Map<String, int> functionIndices;
Map<String, Code> functionCodes;
Map<String, int> typeMap;

enum nameSubSectionTypes {
	module_name = 0,
	function_names = 1,
	local_names = 2,
};

Code Call(char *symbol);//Node* args
Code &unsignedLEB128(long n);

Code &signedLEB128(long value);

//Code& unsignedLEB128(int);
//Code& flatten(byter);
//Code& flatten (Code& data);
void todo(char *message = "") {
	printf("TODO %s\n", message);
}

//typedef int number;
//typedef char byte;
//typedef char* Code;
//typedef char id;





bytes concat(bytes a, bytes b, int len_a, int len_b) {
	bytes c = new char[len_a + len_b + 4];
	memcpy0(c, a, len_a);
	memcpy0(c + len_a, b, len_b);
	c[len_a + len_b + 1] = 0;
	return c;
}


bytes concat(bytes a, char b, int len) {
	bytes c = new char[len + 1];
	memcpy0(c, a, len);
	c[len] = b;
	return c;
}

bytes concat(char a, bytes b, int len) {
	bytes c = new char[len + 1];
	c[0] = a;
	memcpy0(c + 1, b, len);
	return c;
}
//
//class Nod{
//public:
//	Nod(Block &alternate, ExpressionNod &args, Block &consequent, ExpressionNod &expression,
//	    ExpressionNod &initializer, String &name, Block &nodes, String &typeName, String &value)
//			: alternate(alternate), args(args), consequent(consequent), expression(expression),
//			  initializer(initializer), name(name), nodes(nodes), type_name(typeName), value(value) {
//
//	}
//
//	Nod(Block *alternate, ExpressionNod *args, Block *consequent, ExpressionNod *expression,
//	    ExpressionNod *initializer, String *name, Block *nodes, String *typeName, String *value)
//	: alternate(*alternate), args(*args), consequent(*consequent), expression(*expression),
//	initializer(*initializer), name(*name), nodes(*nodes), type_name(*typeName), value(*value) {
//
//	}
//	Nod():Nod(0,0,0,0,0,0,0,0,0){}
//
//	String type_name;
//	int type;
//
//	String& value;// variable name or operator "+" …
//	ExpressionNod& expression;
//	ExpressionNod& initializer;
//	String& name;
//	ExpressionNod& args;
//	Block& nodes;
//	Block& consequent;
//	Block& alternate;
//	int index;// for args
//	Block *begin() const{
//		todo();
//		return &nodes;
//	}
//	Nod *end() const{
//		todo();
//		return 0;
//	}
//
//	Nod& operator[](int i){
//		todo();
//		return *this;
//	}
//};
//class ExpressionNod : public Nod{
//public:
//	ExpressionNod() {}
//
//	ExpressionNod& operator[](int i){
//		todo();
//		return *this;
//	}
//};
//
//class Block : public Nod{
//public:
//	Block() {}
//	Block& operator[](int i){
//		todo();
//		return *this;
//	}
//};
//
//class ProcStatementNod : public Nod{
//public:
//	ProcStatementNod() {}
//	ProcStatementNod& operator[](int i){
//		todo();
//		return *this;
//	}
//};

//#include <iostream>

class Bytes {
public:
	int length;
//	int length(){
//		return 1;
//	}
};

// nonsense, we already have bytes!?
//uint8_t uint8Array( std::initializer_list<Code> list ){
//	int total = 0;
//	for(Code more:list){total+=more.length;}
//	uint8_t all[total];
//int current = 0;
//for(Code more:list){
//	for (int i=0;i<more.length;i++){
//all[current++]=more.data[i];
//	}
//	}
//}



//bytes
Code signedLEB128(int i);

Code encodeString(char *String);

//typedef Code any;
//typedef Bytes any;

// class Code;

// Code flatten (any arr[]) {
// [].concat.apply([], arr);



// https://pengowray.github.io/wasm-ops/
byte opcodes(chars s, byte kind = 0) {
//	if(eq(s,"$1="))return set_local;
//	if (eq(s, "=$1"))return get_local;
//	if (eq(s, "=$1"))return tee_local;

	if (kind == 0) { // INT32
		if (eq(s, "+"))return i32_add;
		if (eq(s, "-"))return i32_sub;
		if (eq(s, "*"))return i32_mul;
		if (eq(s, "/"))return i32_div;
		if (eq(s, "%"))return i32_rem;
		if (eq(s, "=="))return i32_eq;
		if (eq(s, "!="))return i32_ne;
		if (eq(s, ">"))return i32_gt;
		if (eq(s, "<"))return i32_lt;
		if (eq(s, ">="))return i32_ge;
		if (eq(s, "<="))return i32_le;
		if (eq(s, "≥"))return i32_ge;
		if (eq(s, "≤"))return i32_le;

		if (eq(s, "&"))return i32_and;
		if (eq(s, "&&"))return i32_and;
		if (eq(s, "and"))return i32_and;
		if (eq(s, "or"))return i32_or;
		if (eq(s, "xor"))return i32_xor;
		if (eq(s, "not"))return i32_eqz;
		if (eq(s, "||"))return i32_or;
		if (eq(s, "|"))return i32_or;
	} else {

		if (eq(s, "not"))return f32_eqz; // HACK: no such thing!
		if (eq(s, "+"))return f32_add;
		if (eq(s, "-"))return f32_sub;
		if (eq(s, "*"))return f32_mul;
		if (eq(s, "/"))return f32_div;
		if (eq(s, "=="))return f32_eq;
		if (eq(s, ">"))return f32_gt;
		if (eq(s, ">="))return f32_ge;
		if (eq(s, "<"))return f32_lt;
		if (eq(s, "<="))return f32_le;
	}
	printf("unknown operator %s", s);
//		error("invalid operator");
	return 0;
}

// http://webassembly.github.io/spec/core/binary/modules.html#export-section
enum ExportType {
	func_export = (char) 0x00,
	table_export = 0x01,
	mem_export = 0x02,
	global_export = 0x03
};

// http://webassembly.github.io/spec/core/binary/types.html#function-types
char functionType = 0x60;

char emptyArray = 0x0;

// https://webassembly.github.io/spec/core/binary/modules.html#binary-module
bytes magicModuleHeader = new char[]{0x00, 0x61, 0x73, 0x6d};
bytes moduleVersion = new char[]{0x01, 0x00, 0x00, 0x00};



typedef uint8_t byt;

//https://en.wikipedia.org/wiki/LEB128
// little endian 257 = 0x81 (001) + 0x02 (256)
Code &unsignedLEB128(long n) {
	Code buffer;
//	Code* buffer=new Code(); // IF RETURNING Code&
	do {
		byt byte = n & 0x7f;
		n = n >> 7;
		if (n != 0) {
			byte |= 0x80;// continuation bit
		}
		buffer.add(byte);
	} while (n != 0);
	return buffer;
}

Code &signedLEB128(long value) {
	Code buffer;
	int more = 1;
	bool negative = (value < 0);
	long val = value;
/* the size in bits of the variable value, e.g., 64 if value's type is int64_t */
//	size = no. of bits in signed integer;
//	int size = 64;
	while (more) {
		byt byte = val & 0x7f;
		val >>= 7;
		/* the following is only necessary if the implementation of >>= uses a
		   logical shift rather than an arithmetic shift for a signed left operand */
//		if (negative)
//			val |= (~0 << (size - 7)); /* sign extend */

		/* sign bit of byte is second high order bit (0x40) */
		bool clear = (byte & 0x40) == 0;  /*sign bit of byte is clear*/
		bool set = byte & 0x40; /*sign bit of byte is set*/
		if ((val == 0 && clear) || (val == -1 && set))
			more = 0;
		else {
			byte |= 0x80;// continuation bit:  set high order bit of byte;
		}
		buffer.add(byte); //		emit byte;
	}
	return buffer;
}
// https://webassembly.github.io/spec/core/binary/conventions.html#binary-vec
// Vectors are encoded with their length followed by their element sequence
//Code encodeVector (char data[]) {
//	return Code(unsignedLEB128(sizeof(data)), flatten(data));
//}

//Code& encodeVector (Code& data) {
//	if(data.encoded)return data;
////	return Code(unsignedLEB128(data.length), flat(data),data.length);
//	Code code = unsignedLEB128(data.length) + flatten(data);
//	code.encoded = true;
//	return code;
//}
//Code& encodeVector (Code& data) {
Code encodeVector(Code data) {
//	return data.vector();
	if (data.encoded)return data;
//	Code code = unsignedLEB128(data.length) + flatten(data);
	Code code = Code((byte) data.length) + data;
	code.encoded = true;
	return code;
}

// https://webassembly.github.io/spec/core/binary/modules.html#code-section
Code encodeLocal(long count, Valtype type) {
	return unsignedLEB128(count).addByte(type);
}

// https://webassembly.github.io/spec/core/binary/modules.html#sections
// sections are encoded by their type followed by their vector contents
Code createSection(Section sectionType, Code data) {
	return Code(sectionType, encodeVector(data));
}


enum NodTypes {
	numberLiteral,
	identifier,
	binaryExpression,
	printStatement,
	variableDeclaration,
	variableAssignment,
	whileStatement,
	ifStatement,
	callStatement,
	internalError,
};


bytes ieee754(float num) {
	char data[4];
	float *hack = ((float *) data);
	*hack = num;
	char *flip = static_cast<char *>(alloc(5));
	short i = 4;
//	while (i--)flip[3 - i] = data[i];
	while (i--)flip[i] = data[i];// don't flip, just copy to malloc
	return flip;
}
//Code emitExpression (Node* nodes);

Code emitBlock(Node node, String functionContext);

//Code emitExpression(Node *node)__attribute__((warn_unused_result));
//Code emitExpression(Node *node)__attribute__((error_unused_result));


//Map<int, String>
List<String> collect_locals(Node node, String string);


Code emitValue(Node node, String context) {
	Code code;
	switch (node.kind) {
		case nils:// also 0, false
//			code.opcode((byte)i64_auto);// nil is pointer
			code.addByte((byte) i32_auto);// nil is pointer
			code.push((long) 0);
			break;
		case bools:
//		case ints:
			code.addByte((byte) i32_auto);
			code.push(node.value.longy);
			last_type = i32t;
//				code.opcode(ieee754(node.value.longy),4);
			break;
		case longs:
			// todo: ints!!!
			last_type = i32t;
//			if(call_extern)
			code.addByte((byte) i32_const);
//			code.opcode((byte)i64_auto);
			code.push(node.value.longy);
//				code.opcode(ieee754(node.value.longy),4);
			break;
		case reals:
			last_type = f32t;// auto cast return!
			code.addByte((byte) f32_auto);
			code.push(ieee754(node.value.real), 4);
			break;
		case reference:
		case identifier: {
			int local_index = locals[context].position(node.name);
			if (local_index < 0)error("UNKNOWN symbol "s + node.name + " in context " + context);
			code.addByte(get_local);
			code.addByte(local_index);
		}
			break;
//			case binaryExpression:
//				code.opcode(binaryOpcode[node.value]);
//				break;
		default:
			error("emitValue unknown type: "s + typeName(node.kind));
	}
	return code;
}

Code emitWhile(Node &node, String context);

Code emitIf(Node &node, String context);

Code emitCall(Node &node, String context);

Code emitDeclaration(Node fun, Node &body);

Code emitExpression(Node *nodes, String context);

Code emitExpression(Node &node, String context/*="main"*/) { // expression, node or BODY (list)
//	if(nodes==NIL)return Code();// emit nothing unless NIL is explicit! todo
	Code code;
	String &name = node.name;
	int index = functionIndices.position(name);
	if (index >= 0 and not locals.has(name))
		locals[name] = List<String>();
//	locals[index]= Map<int, String>();

	if (name == "if")
		return emitIf(node, context);
	if (name == "while")
		return emitWhile(node, context);
	if (name == "it") {
		code.addByte(get_local);
		code.addByte(0);// todo: LAST local?
		return code;
	}
	if (node.kind == call or (node.kind == reference or node.kind == groups) and functionIndices.has(name))
		return emitCall(node, context);

	switch (node.kind) {
		case expressions:
		case groups:
		case objects:
			for (Node child : node) {
				const Code &expression = emitExpression(child, context);
				code.push(expression);
			};
			break;
		case call:
			return emitCall(node, context);
			break;
		case operators: {
			if (name == "then")return emitIf(*node.parent, context);// pure if handled before
			if (name == ":=")
				return emitDeclaration(node, node.first());
//			return emitDeclaration(node.children[0], node.children[1]);
			if (node.length < 1) {
				node.log();
				error("missing args for operator "s + name);
			} else if (node.length == 1) {
//				if(name.in(function_list)) SHOULDN'T HAPPEN!
//				error("unexpected unary operator: "s + name);
				Node arg = node.children[0];
				const Code &arg_code = emitExpression(arg, context);// should ALWAYS just be value, right?
				code.push(arg_code);// might be empty ok
			} else if (node.length == 2) {
				Node lhs = node.children[0];//["lhs"];
				Node rhs = node.children[1];//["rhs"];
				const Code &lhs_code = emitExpression(lhs, context);
				const Code &rhs_code = emitExpression(rhs, context);
				code.push(lhs_code);// might be empty ok
				code.push(rhs_code);// might be empty ok
			} else if (node.length > 2) {// todo: n-ary? ∑? is just a function!
				error("Too many args for operator "s + name);
			}
			if (index >= 0) {// FUNCTION CALL
				log("OPERATOR FUNCTION CALL: %s\n"s % name);
//				for (Node arg : node) {
//					emitExpression(arg,context);
//				};
				code.addByte(function);
				code.addByte((index));// ok till index>127?
				break;
			}
			byte opcode = opcodes(name, last_type == f32t);
			if (opcode == f32_eqz) { // hack for missing f32_eqz
//				0.0 + code.addByte(f32_eq);
				code.addByte(
						i32_reinterpret_f32);// f32->i32  i32_trunc_f32_s would also work, but reinterpret is cheaper
				code.addByte(i32_eqz);
				last_type = i32t;// bool'ish
				break;
			} else if (opcode > 0)
				code.addByte(opcode);
			else {
				error("unknown opcode / call / symbol: "s + name);
			}
			if (opcode == i32_add or opcode == i32_modulo or opcode == i32_sub or opcode == i32_div or
			    opcode == i32_mul)
				last_type = i32t;
			if (opcode == f32_eq or opcode == f32_gt or opcode == f32_lt)
				last_type = i32t;// bool'ish
		}
			break;
		case declaration:
//			return emitDeclaration(node.children[0], node.children[1]);
			return emitDeclaration(node, node.first());
			break;
		case nils:
		case longs:
		case reals:
		case bools:
		case strings:
			if (not node.isSetter() || node.value.longy == 0) // todo 0
				return emitValue(node, context);
//			else FALLTHROUGH!
		case reference: {
//			Map<int, String>
			List<String> &current_local_names = locals[context];
			int local_index = current_local_names.position(name);// defined in block header
			if (local_index < 0) { // collected before, so can't be setter here
				if (functionCodes.has(name))
					return emitCall(node, context);
				if (!node.isSetter())
					error("UNKNOWN local symbol "s + name + " in context " + context);
				else {
					current_local_names.add(name);// ad hoc x=42
					local_index = current_local_names.size() - 1;
				}
			}
			if (node.isSetter()) { //SET
				code = code + emitValue(node, context); // done above!
				code.addByte(set_local);
				code.addByte(local_index);
			} else {// GET
				code.addByte(get_local);
				code.addByte(local_index);
			}
		}
			break;
		default:
			error("unhandled node type: "s + typeName(node.kind));
	}
	return code;
}

Code emitWhile(Node &node, String context) {
	Code code;
	// outer block
	code.addByte(block);
	code.addByte(void_block);
	// inner loop
	code.addByte(loop);
	code.addByte(void_block);
	// compute the while expression
	emitExpression(node[0], context);// node.value.node or
	code.addByte(i32_eqz);
	// br_if $label0
	code.addByte(br_if);
	code.addByte(1);
//			code.push(signedLEB128(1));
	// the nested logic
	emitExpression(node[1], context);// BODY
	// br $label1
	code.addByte(br);
	code.addByte(0);
//				code.push(signedLEB128(0));
	code.addByte(end_block); // end loop
	code.addByte(end_block); // end block
	return code;
}


Code emitExpression(Node *nodes, String context) {
	if (!nodes)return Code();
//	if(nodes==NIL)return Code();// emit nothing unless NIL is explicit! todo
	return emitExpression(*nodes, context);
}

Code emitCall(Node &fun, String context) {
	Code code;
	int index = functionIndices[fun.name];
	if (index < 0) error("MISSING import/declaration for function %s\n"s % fun.name);
	for (Node arg : fun) {
		code.push(emitExpression(arg, context));
	};
	code.addByte(function);
	code.addByte(index);// ok till index>127, then use unsignedLEB128
	code.addByte(nop);// padding for potential relocation
	last_type = return_types[fun.name];// voids;// todo lookup return type
	return code;
}

Code emitDeclaration2(Node fun, Node &body) {
	// OLD SHIT
	if (fun.name.empty()) {
		fun = body[0].flat();
	}

	if (fun.name.empty())
		error("NO SYMBOL NAME FOR declaration");

	if (body.has(":="))
		body = body.from(":=");
//		error("parser error :=");
}

Code emitDeclaration(Node fun, Node &body) {
	if (not functionIndices.has(fun.name)) {
		functionIndices[fun.name] = functionIndices.size();
	} else {
		error("redeclaration of symbol: "s + fun.name);
	}
	Signature &signature = functionSignatures[fun.name];
	if (signature.size() == 0 and body.has("it", false, 100))
		signature.add(i32t);

//			body=*fun.begin()
	Valtype returns = int32;// todo
	return_types[fun.name] = returns;
	signature.returns(returns);
	functionCodes[fun.name] = emitBlock(body, fun.name);
	last_type = voids;// todo reference to new symbol x = (y:=z)
	return Code();// empty
}


Code emitIf(Node &node, String context) {
	Code code;
	Node *condition = node[0].value.node;
//	Node &condition = node["condition"];
	code = code + emitExpression(condition, context);
	if (last_type != int32) {
		last_type = int32;
		printf("todo\n");
	}
	code.addByte(if_i);
	code.addByte(int32);
	Node *then = node[1].value.node;
//	Node &then = node["then"];
	code = code + emitExpression(then, context);// BODY
	if (node.length == 3) {
		code.addByte(elsa);
		Node *otherwise = node[2].value.node;//->clone();
		code = code + emitExpression(otherwise, context);
	}
	code.addByte(end_block);
	return code;
}

/*
Code emitIf_OLD(Node &node) {
	Code code;
//	case ifStatement:
	// if block
	code.addByte(block);
	code.addByte(void_block);
	// compute the if expression
	Node *condition = node[0].value.node;
//	Node *condition = node["condition"];//.value.node->clone();
	emitExpression(condition,context);
	code.addByte(i32_eqz);
	// br_if $label0
	code.addByte(br_if);
	code.addByte(0);
//			code.opcode(signedLEB128(0));
	// the nested logic
	Node *then = node[1].value.node;
//	Node *then = node["then"].value.node->clone();
	emitExpression(then);// BODY
	// end block
	code.addByte(end_block);

	if (node.length == 3) {

		// else block
//			or if OR-IF YAY semantically beautiful if false {} or if 2>1 {}
//			// compute the if expression (elsif) elif orif
		code.addByte(block);
		code.addByte(void_block);
//
//			emitExpression(node.param);
		code.addByte(i32_auto);
////				code.opcode(signedLEB128(1));
		code.addByte(1);
		code.addByte(i32_eq);
//			// br_if $label0
		code.addByte(br_if);
		code.addByte(0);
////				code.opcode(signedLEB128(0));
//			// the nested logic
//		Node *otherwise = node["else"].value.node->clone();
		Node *otherwise = node[2].value.node;//->clone();
		emitExpression(otherwise);

//			// end block
		code.addByte(end_block);
	}
	return code;
}
*/

Code Call(char *symbol) {//},Node* args=0) {
	Code code;
	code.addByte(function);
	int i = functionIndices.position(symbol);
	if (i < 0)error("UNKNOWN symbol "s + symbol);
//	code.opcode(unsignedLEB128(i),8);
	code.addByte(i);
	return Code();
}

Code encodeString(char *str) {
	size_t len = strlen0(str);
	Code code = Code(len, str, len);
	return code;//.push(0);
};

//bytes
Code signedLEB128(int n) {
	Code result;
	while (true) {
		const char byt = n & 0x7f;
		n >>= 7;
		if (
				(n == 0 && (byt & 0x40) == 0) ||
				(n == -1 && (byt & 0x40) != 0)
				) {
			result.addByte(byt);
			return result;
		}
		result.addByte(byt | 0x80);
	}
//	return result.data;
}


Code emitBlock(Node node, String context) {
//	char code_data[] = {0/*locals_count*/,i32_const,42,call,0 /*logi*/,i32_auto,21,return_block,end_block};// 0x00 == unreachable as block header !?
//	char code_data[] = {0/*locals_count*/,i32_auto,21,return_block,end_block};// 0x00 == unreachable as block header !?
//	Code(code_data,sizeof(code_data)); // todo : memcopy, else stack value is LOST
	Code block;
//	Map<int, String>
	collect_locals(node, context);// DONE IN analyze
//	int locals_count = current_local_names.size();
//	locals[context] = current_local_names;
	int locals_count = locals[context].size();
	int argument_count = functionSignatures[context].size();
	if (locals_count >= argument_count)
		locals_count = locals_count - argument_count;
	else
		warn("locals consumed by arguments");
	block.addByte(locals_count);
	for (int i = 0; i < locals_count; ++i) {
		block.addByte(i + 1);// index
		block.addByte(i32t);// type todo
	}

	Code inner_code_data = emitExpression(node, context);
	Valtype x = last_type;
	block.push(inner_code_data);
	Valtype returns = return_types[context];
	if (returns != last_type) {
		if (returns == Valtype::voids and last_type != Valtype::voids)
			block.addByte(drop);
		if (returns == Valtype::i32t and last_type == Valtype::f32t)
			block.addByte(i32_trunc_f32_s);
		if (returns == Valtype::i32t and last_type == Valtype::voids)
			block.addByte(i32_const).addByte(0);// hack? return 0/false by default. ok? see python!
//		if(returns==Valtype::f32)…
	}

//if not return_block
	block.addByte(return_block);
	block.addByte(end_block);
	return block;
}

//Map<int, String>
List<String> collect_locals(Node node, String string) {
	List<String> &current_locals = locals[string]; // no, because there might be gaps(unnamed locals!)
	for (Node n : node) {
		bool add = false;
		if (n.kind == longs and atoi0(n.name) != n.value.longy)add = true;
		if (n.kind == longs and not n.name.empty() and not atoi0(n.name))add = true;
		if (n.kind == reference and not functionIndices.has(n.name))add = true;
		if (add and not current_locals.has(n.name))
			current_locals.add(n.name);
	}
	return current_locals;
}


Code typeSection() {
	// Function types are vectors of parameters and return types. Currently
	// optimise TODO - some of the procs might have the same type signature
	// the type section is a vector of function types
//	auto typeSection = createSection(type, encodeVector(printFunctionType).push(funcTypes));
//	char type0[]={0x01,0x60/*const type form*/,0x02/*param count*/,0x7F,0x7F,0x01/*return count*/,0x7F};
	char vi[] = {0x60/*const type form*/, 0x00/*param count*/, 0x01/*return count*/,
	             i32t};// our main function! todo : be flexible!
	char iv[] = {0x60/*const type form*/, 0x01/*param count*/, i32t, 0x00/*return count*/};
	char vv[] = {0x60/*const type form*/, 0x00/*param count*/, 0x00/*return count*/};
	char ii[] = {0x60/*const type form*/, 0x01/*param count*/, i32t, 0x01/*return count*/, i32t};
	char iii[] = {0x60/*const type form*/, 0x02/*param count*/, i32t, i32t, 0x01/*return count*/, i32t};
	char fv[] = {0x60/*const type form*/, 0x01/*param count*/, f32t, 0x00/*return count*/};

	int typeCount = 5;
	Code type_data = Code(vi, sizeof(vi)) + Code(iv, sizeof(iv)) + Code(vv, sizeof(vv)) + Code(ii, sizeof(ii)) +
	                 Code(fv, sizeof(fv)); //			+ Code(iii, sizeof(iii));

	for (String fun :functionSignatures) {
		typeMap[fun] = typeCount;
		typeCount++;
		Signature &signature = functionSignatures[fun];
		int param_count = signature.size();
		Code td = {0x60 /*const type form*/, param_count};
		for (int i = 0; i < param_count; ++i) {
			td = td + Code(signature.types[i]);
		}
		Valtype &ret = return_types[fun];
		if (ret == voids) {
			td.addByte(0);
		} else {
			td.addByte(1/*return count*/).addByte(ret);
		}
		type_data = type_data + td;

	}
//auto typeSection =
	return Code(type, encodeVector(Code(typeCount) + type_data));
}

int import_count = 4;

Code importSection() {
	// the import section is a vector of imported functions

//  bytes printFunctionType = new char[]{functionType,encodeVector(f32), emptyArray}
	Code printFunctionType = Code(functionType).push(encodeVector(Code(f32t))).push(emptyArray);
//	functionSignatures["logi"] = Signature().add(i32t);
//	functionSignatures["logf"] = Signature().add(f32);
//	functionSignatures["square"] = Signature().add(i32t).returns(i32t);
//	functionSignatures["sqrt"] = Signature().add(i32t).returns(i32t);

// todo: remove hardcoded!
	Code logi_iv = encodeString("env") + encodeString("logi").addByte(func_export).addType(1);
	Code logf_fv = encodeString("env") + encodeString("logf").addByte(func_export).addType(4);
	Code square_ii = encodeString("env") + encodeString("square").addByte(func_export).addType(3);
	Code sqrt_ii = encodeString("env") + encodeString("√").addByte(func_export).addType(3);

//
	functionIndices["logi"] = functionIndices.size();// import!
	functionIndices["logf"] = functionIndices.size();// import!
	functionIndices["square"] = functionIndices.size();
	functionIndices["√"] = functionIndices.size();
	return_types["square"] = int32;
	return_types["id"] = int32;
	return_types["√"] = int32;
//	return_types["logi", voids);

//	auto importSection = createSection(import, encodeVector(printFunctionImport));//+memoryImport

	auto importSection = createSection(import, Code(import_count) + logi_iv + logf_fv + square_ii + sqrt_ii);
	return importSection;
}

int function_count;// minus import_count  (together : functionIndices.size() )
Code codeSection(Node root) {
	// the code section contains vectors of functions
	if (functionIndices.size() != import_count)
		error("inconsistent function_count");
	short function_offset = import_count;
// https://pengowray.github.io/wasm-ops/
//	char code_data[] = createSection() 0x01,0x08,0x00,0x01,0x3f,0x0F,0x0B};// ok 0x01==nop
//  Code code_data=encodeVectors(encodeVector(1/*function_index/))
//	char code_data[] = {0x01,0x05,0x00,0x41,0x2A,0x0F,0x0B};// 0x41==i32_auto  0x2A==42 0x0F==return 0x0B=='end (function block)' opcode @+39

// index needs to be known before emitting code, so call $i works
	functionIndices["main"] = function_offset++;
	functionIndices["nop"] = function_offset++;
	functionIndices["id"] = function_offset++;
	return_types["id"] = i32t;
	return_types["main"] = i32t;
	return_types["nop"] = voids;
	functionSignatures["nop"] = Signature();
	functionSignatures["id"] = Signature().add(i32t).returns(i32t);
	functionSignatures["main"] = Signature().returns(i32t);
	short builtin_count = 2;


	char code_data_fourty2[] = {0/*locals_count*/, i32_auto, 42, return_block, end_block}; // 0x00 == unreachable as block header !?
	char code_data_nop[] = {0/*locals_count*/, end_block};// NOP
	char code_data_id[] = {1/*locals_count*/, 1/*WTF? first local has type: */, i32t, get_local, 0, return_block,
	                       end_block};// NOP
//	char code_data[] = {0/*locals_count*/,i32_const,48,function,0 /*logi*/,i32_auto,21,return_block,end_block};// 0x00 == unreachable as block header !?
//	char code_data[] = {0x00, 0x41, 0x2A, 0x0F, 0x0B,0x01, 0x05, 0x00, 0x41, 0x2A, 0x0F, 0x0B};

	Code main_block = emitBlock(root, "main");
	Code nop_block = Code(code_data_nop, sizeof(code_data_nop));
	Code id_block = Code(code_data_id, sizeof(code_data_id));
	Code code_blocks = encodeVector(main_block) + encodeVector(nop_block) + encodeVector(id_block);
	for (String fun : functionCodes) {
		Code &func = functionCodes[fun];
		code_blocks = code_blocks + encodeVector(func);
	}
	function_count = builtin_count + 1/*main*/ + functionCodes.size();// offset by imports!?
	int index_size = functionIndices.size();
	if (function_count != index_size - import_count) {
		functionCodes.log();
		functionIndices.log();
		error("inconsistent function_count %d != %d - %d"s % function_count % index_size % import_count);
	}
	auto codeSection = createSection(code_section, Code(function_count) + code_blocks);
	return codeSection;
}

short exports_count = 1;

Code exportSection() {
// the export section is a vector of exported functions etc
	int main_offset = functionIndices["main"];
	Code exportsData = encodeVector(Code(exports_count) + encodeString("main") + (byte) func_export + Code(main_offset));
	auto exportSection = createSection(exports, exportsData);
	return exportSection;
}


Code funcTypeSection() {
	// the function section is a vector of type indices that indicate the type of each function in the code section
	Code types_of_functions = Code(function_count);//  = Code(types_data, sizeof(types_data));
	for (int i = 0; i < function_count; ++i) {
		//	import section types separate WTF wasm
		String *fun = functionIndices.lookup(i + import_count);
		if (!fun)error("missing typeMap for index "s + i);
		else {
			int typeIndex = typeMap[*fun];
			if (typeIndex < 0)
				error("missing typeMap for function %s index %d "s % fun % i);
			types_of_functions.push((byte) typeIndex);
		}
	}
	// @ WASM : WHY DIDN'T YOU JUST ADD THIS AS A FIELD IN THE FUNC STRUCT???
	Code funcSection = createSection(func, types_of_functions);
	return funcSection;
}

Code functionSection() {
	return funcTypeSection();// (misnomer) vs codeSection() !
}

Code nameSection() {
	Code nameMap;
//	check(symbols["logi"]==0);
//	check(symbols["√"]==3);// "\e2\88\9a"
//	nameMap =  Code((byte) 0) + Code("logi");
	for (int index = 0; index < functionIndices.size(); index++) {
		// danger: utf names are NOT translated to wat env.√=√ =>  (import "env" "\e2\88\9a" (func $___ (type 3)))
		String &name = functionIndices.keys[index];
		nameMap = nameMap + Code(index) + Code(name);
	}

//	auto nameSubSectionFunctionNames = Code(function_names) + encodeVector(Code(1) + Code((byte) 0) + Code("logi"));
//	functions without parameters need  entry ( 00 01 00 00 )
//  functions 5 with local 'hello' :  05 01 00 05 68 65 6c 6c 6f
//  functions 5 with local 'hello' :  05 02 00 05 68 65 6c 6c 6f 01 00 AND unnamed (local i32t)
// localMapEntry = (index nrLocals 00? string )

	Code localNameMap;
	for (String key : functionIndices) {
//		if (key != "main")continue;
		int function_index = functionIndices[key];
		List<String> localNames = locals[key];// including arguments
		int local_count = localNames.size();
		localNameMap = localNameMap + Code(function_index) + Code(local_count); /*???*/
		for (int i = 0; i < localNames.size(); ++i) {
//		String local_name = "test_"s+key+"#"+local_count;
			String local_name = localNames[i];
			localNameMap = localNameMap + Code(i) + Code(local_name);
		}
//		error: expected local name count (1) <= local count (0) FOR FUNCTION ...
	}
//	localNameMap = localNameMap + Code((byte) 0) + Code((byte) 1) + Code((byte) 0) + Code((byte) 0);// 1 unnamed local
//	localNameMap = localNameMap + Code((byte) 4) + Code((byte) 0);// no locals for function4
//	Code exampleNames= Code((byte) 5) /*function index*/ + Code((byte) 1) /*count*/ + Code((byte) 0) /*l.nr*/ + Code("var1");
	// function 5 with one local : var1

//	localNameMap = localNameMap + exampleNames;

	auto nameSubSectionModuleName = Code(module_name) + encodeVector(Code("wasp_module"));
	auto nameSubSectionFunctionNames = Code(function_names) + encodeVector(Code(functionIndices.size()) + nameMap);
	auto nameSubSectionLocalNames = Code(local_names) + encodeVector(Code(functionIndices.size()) + localNameMap);

//	auto nameSubSectionFuncNames = Code(module_name) + encodeVector(Code("wasp_module"));
//	The name section is a custom section whose name string is itself ‘𝚗𝚊𝚖𝚎’. The name section should appear only once in a module, and only after the data section.
	const Code &nameSectionData = encodeVector(
			Code("name") + nameSubSectionModuleName);// + nameSubSectionFunctionNames + nameSubSectionLocalNames);
	auto nameSection = createSection(custom, nameSectionData);
	nameSection.debug();
	return nameSection;
}

Code dataSection(){
	return Code();
}
Code eventSection(){
}

/*
 *  There are currently two ways in which function indices are stored in the code section:
    Immediate argument of the call instruction (calling a function)
    Immediate argument of the i32.const instruction (taking the address of a function).
    The immediate argument of all such instructions are stored as padded LEB128 such that they can be rewritten without altering the size of the code section. !
    For each such instruction a R_WASM_FUNCTION_INDEX_LEB or R_WASM_TABLE_INDEX_SLEB reloc entry is generated pointing to the offset of the immediate within the code section.

    R_WASM_FUNCTION_INDEX_LEB relocations may fail to be processed, in which case linking fails. This occurs if there is a weakly-undefined function symbol, in which case there is no legal value that can be written as the target of any call instruction. The frontend must generate calls to undefined weak symbols via a call_indirect instruction.
*/
Code linkingSection(){
//	https://github.com/WebAssembly/tool-conventions/blob/master/Linking.md#linking-metadata-section
	short version=2;
	Code subsection;
	short type=5;// SEGMENT alignment & flags
	short payload_len=0;
	Code payload_data;
	subsection.add(type).add(payload_len).push(payload_data);
	Code subsections;
/*
	5 / WASM_SEGMENT_INFO - Extra metadata about the data segments.
	6 / WASM_INIT_FUNCS - Specifies a list of constructor functions to be called at startup. Called after memory has been initialized.
	7 / WASM_COMDAT_INFO - Specifies the COMDAT groups of associated linking objects, which are linked only once and all together.
	8 / WASM_SYMBOL_TABLE - Specifies extra information about the symbols present in the module.
// https://en.wikipedia.org/wiki/Relocatable_Object_Module_Format
// http://wiki.dwarfstd.org/index.php?title=COMDAT_Type_Sections
	COMDAT - (C2h/C3h) Initialized common data
	COMDEF - (B0h) Uninitialized common data
 Thread-agnostic objects can be safely linked with objects that do or do not use atomics, although not both at the same time.
*/
	return createSection(custom, encodeVector(Code("linking") + Code(version) + subsections));
}
/*
 * The generally accepted features are:
    atomics
    bulk-memory
    exception-handling
    multivalue
    mutable-globals
    nontrapping-fptoint
    sign-ext
    simd128
    tail-call
 */

Code &emit(Node root_ast) {
	functionIndices.clear();
	functionCodes.clear();
	typeMap.clear();
	functionIndices.setDefault(-1);
	functionSignatures.setDefault(Signature());
	functionCodes.setDefault(Code());
	typeMap.setDefault(-1);
	return_types.setDefault(voids);
	return_types.setDefault(voids);
//	Map<int, String>

	/* limits https://webassembly.github.io/spec/core/binary/types.html#limits - indicates a min memory size of one page */
	auto memorySection = createSection(memory_section, encodeVector(Code(2)));
	auto memoryImport = encodeString("env") + encodeString("memory") + (byte) mem_export/*type*/+ (byte) 0x00 + (byte) 0x01;

	auto customSection = createSection(custom, encodeVector(Code("custom123") + Code("random custom section data")));
	const Code &importSection1 = importSection();
	const Code &codeSection1 = codeSection(root_ast);// depends on importSection, yields data for funcTypeSection!
	Code code = Code(magicModuleHeader, 4) + Code(moduleVersion, 4) + typeSection() + importSection1 +
	            funcTypeSection() + exportSection() + codeSection1 + dataSection() + linkingSection() + nameSection();// + customSection;
// memorySection +
//	Code code = Code(magicModuleHeader, 4) + Code(moduleVersion, 4) + typeSection + funcSection + exportSection + codeSection;// + memorySection + ;
	code.debug();
	return code.clone();
}
