// BASED ON https://github.com/ColinEberhardt/chasm/blob/master/src/emitter.ts
// https://github.com/ColinEberhardt/chasm/blob/master/src/encoding.ts
// https://pengowray.github.io/wasm-ops/
#include "Wasp.h"
#include "String.h"
#include "Map.h"
#include "wasm-emitter.h"
#include "WasmHelpers.h"
#include "wasm_runner.h"

#define check(test) if(test){log("OK check passes: ");log(#test);}else{printf("\nNOT PASSING %s\n%s:%d\n",#test,__FILE__,__LINE__);exit(0);}

#ifdef WASM
#import  "WasmHelpers.cpp"
#endif
//#include "string.h" // memcpy

typedef char byter[];

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




void memcpy(bytes c, bytes a, int i);

bytes concat(bytes a, bytes b, int len_a, int len_b) {
	bytes c = new char[len_a + len_b + 1];
	memcpy(c, a, len_a);
	memcpy(c + len_a, b, len_b);
	return c;
}

void memcpy(bytes dest, bytes source, int i) {
	while (i--)dest[i] = source[i];
}

bytes concat(byter a, char b, int len) {
	bytes c = new char[len + 1];
	memcpy(c, a, len);
	c[len] = b;
	return c;
}

bytes concat(char a, byter b, int len) {
	bytes c = new char[len + 1];
	c[0] = a;
	memcpy(c + 1, b, len);
	return c;
}
//
//class Nod{
//public:
//	Nod(Block &alternate, ExpressionNod &args, Block &consequent, ExpressionNod &expression,
//	    ExpressionNod &initializer, String &name, Block &statements, String &typeName, String &value)
//			: alternate(alternate), args(args), consequent(consequent), expression(expression),
//			  initializer(initializer), name(name), statements(statements), type_name(typeName), value(value) {
//
//	}
//
//	Nod(Block *alternate, ExpressionNod *args, Block *consequent, ExpressionNod *expression,
//	    ExpressionNod *initializer, String *name, Block *statements, String *typeName, String *value)
//	: alternate(*alternate), args(*args), consequent(*consequent), expression(*expression),
//	initializer(*initializer), name(*name), statements(*statements), type_name(*typeName), value(*value) {
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
//	Block& statements;
//	Block& consequent;
//	Block& alternate;
//	int index;// for args
//	Block *begin() const{
//		todo();
//		return &statements;
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


bool eq(const char *op, const char *string);


byte opcodes(const char *s, byte kind = 0) {
	if (kind == 0) { // INT32
		if (eq(s, "+"))return i32_add;
		if (eq(s, "-"))return i32_sub;
		if (eq(s, "*"))return i32_mul;
		if (eq(s, "/"))return i32_div;
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
		if (eq(s, "+"))return f32_add;
		if (eq(s, "-"))return f32_sub;
		if (eq(s, "*"))return f32_mul;
		if (eq(s, "/"))return f32_div;
		if (eq(s, "=="))return f32_eq;
		if (eq(s, ">"))return f32_gt;
		if (eq(s, "<"))return f32_lt;
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


bytes flatten(byter data) {
	todo();
	return data;
}

Code &flatten(Code &data) {
	todo();
	return data;
}

#ifdef WASM
typedef char uint8_t;
#endif

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
////	return Code(unsignedLEB128(data.length), flatten(data),data.length);
//	Code code = unsignedLEB128(data.length) + flatten(data);
//	code.encoded = true;
//	return code;
//}
//Code& encodeVector (Code& data) {
Code encodeVector(Code data) {
//	return data.vector();
	if (data.encoded)return data;
//	Code code = unsignedLEB128(data.length) + flatten(data);
	Code code = Code((byte) data.length) + flatten(data);
	code.encoded = true;
	return code;
}

// https://webassembly.github.io/spec/core/binary/modules.html#code-section
Code encodeLocal(long count, Valtype type) {
	return unsignedLEB128(count).push((char) type);
}

// https://webassembly.github.io/spec/core/binary/modules.html#sections
// sections are encoded by their type followed by their vector contents
Code createSection(Section sectionType, Code data) {
	return Code(sectionType, encodeVector(data));
}

//std::map<String ,int> symbols;
Map<String, int> symbols;

int localIndexForSymbol(String &name) {
	if (!symbols.contains(name)) {
		symbols.insert_or_assign(name, symbols.size());
	}
	return symbols[name];
};;

int localIndexForSymbol(char *name) {
	if (!symbols.contains(name)) {
		symbols.insert_or_assign(name, symbols.size());
	}
	return symbols[name];
};

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

bytes ieee754(String &value) {
	todo();
	return value.data;
}

bytes ieee754(float num) {
	todo();
	byt data[4];
	float *hack = ((float *) data);
	*hack = num;
	return reinterpret_cast<bytes>(hack);
}
//Code emitExpression (Node* nodes);

Code emitBlock(Node node);

Code emitExpression(Node *node);

Code emitExpression(Node node) { // expression, statement or BODY (list)
//	if(nodes==NIL)return Code();// emit nothing unless NIL is explicit! todo
	NodTypes type = internalError;// todo: type/kind
	Code code;
	Node statement = node;

	switch (node.kind) {
		case reference:
		case operators: {
//			if (node.length == 2) {// binary operators, and others
				code.push(emitExpression(node["lhs"]));// might be empty ok
				code.push(emitExpression(node["rhs"]));// might be empty ok
//			}
			byte opcode = opcodes(node.name);
			if (opcode > 0)
				code.addByte(opcode);
			else{
				breakpoint_helper
				error("unknown opcode / call / symbol: "s + node.name);
			}

		}
			break;
		case nils:// also 0, false
//			code.opcode((byte)i64_auto);// nil is pointer
			code.addByte((byte) i32_auto);// nil is pointer
			code.push((long) 0);
			break;
		case bools:
//		case ints:
			code.addByte((byte) i32_auto);
			code.push(node.value.longy);
//				code.opcode(ieee754(node.value.longy),4);
			break;
		case longs:
//			if(call_extern)
			code.addByte((byte) i32_const);
//			code.opcode((byte)i64_auto);
			code.push(node.value.longy);
//				code.opcode(ieee754(node.value.longy),4);
			break;
		case floats:
			code.addByte((byte) f32_auto);
			code.push(ieee754(node.value.floaty), 4);
			break;
//			case identifier:
//				code.opcode(get_local);
//				code.opcode(unsignedLEB128(localIndexForSymbol(node.value)), 8);
//				break;
//			case binaryExpression:
//				code.opcode(binaryOpcode[node.value]);
//				break;
		case expression:
		case groups:
		case objects:
			for (Node child : node) {
				code.push(emitExpression(child));
			};
			break;
		case printStatement:
			emitExpression(statement.param);
			code.push(Call("print"));
			break;
//			case variableDeclaration:
//				emitExpression(statement.initializer);
//				code.opcode(set_local);
//				code.opcode(unsignedLEB128(localIndexForSymbol(statement.name)), 8);
//				break;
//			case variableAssignment:
//				todo();
////				emitExpression(statement.value);
//				code.opcode(set_local);
//				code.opcode(unsignedLEB128(localIndexForSymbol(statement.name)), 8);
//				break;
	}
	switch (type) {
		case whileStatement:
			// outer block
			code.addByte(block);
			code.addByte(void_block);
			// inner loop
			code.addByte(loop);
			code.addByte(void_block);
			// compute the while expression
			emitExpression(statement.param);
			code.addByte(i32_eqz);
			// br_if $label0
			code.addByte(br_if);
			code.addByte(1);
//			code.push(signedLEB128(1));
			// the nested logic
			emitExpression(statement.value.node);// BODY
			// br $label1
			code.addByte(br);
			code.addByte(0);

//				code.push(signedLEB128(0));
			// end loop
			code.addByte(end_block);
			// end block
			code.addByte(end_block);
			break;
		case ifStatement:
			// if block
			code.addByte(block);
			code.addByte(void_block);
			// compute the if expression
			emitExpression(statement.param);
			code.addByte(i32_eqz);
			// br_if $label0
			code.addByte(br_if);
			code.addByte(0);
//			code.opcode(signedLEB128(0));
			// the nested logic
			emitExpression(statement.value.node);// BODY
			// end block
			code.addByte(end_block);

			// else block
			code.addByte(block);
			code.addByte(void_block);
			// compute the if expression
			emitExpression(statement.param);
			code.addByte(i32_auto);
//				code.opcode(signedLEB128(1));
			code.addByte(1);
			code.addByte(i32_eq);
			// br_if $label0
			code.addByte(br_if);
			code.addByte(0);
//				code.opcode(signedLEB128(0));
			// the nested logic
			emitExpression(&statement["else"]);
			// end block
			code.addByte(end_block);
			break;
		case callStatement:
			if (statement.name == "setpixel") {
				// compute and cache the setpixel parameters
				emitExpression(statement.param[0]);
				code.addByte(set_local);
				code.addByte(localIndexForSymbol("x"));
//					code.opcode(unsignedLEB128(localIndexForSymbol("x")));

				// write
				code.addByte(i32_store_8);
				code.addByte((char) 0x00);
				code.addByte((char) 0x00); // align and offset
			} else {
//					for (Node arg : *statement.param) {
//						emitExpression(*((ExpressionNod *) &arg));
//					};
				todo();
				int index = localIndexForSymbol(statement.name);
				code.addByte(call);
				code.addByte((index + 1));
//					code.opcode(unsignedLEB128(index + 1));

			};
			break;
	};

	return code;
}

Code emitExpression(Node *nodes) {
	if (!nodes)return Code();
//	if(nodes==NIL)return Code();// emit nothing unless NIL is explicit! todo
	return emitExpression(*nodes);
}

Code Call(char *symbol) {//},Node* args=0) {
	Code code;
	code.addByte(call);
	int i = localIndexForSymbol(symbol);
//	code.opcode(unsignedLEB128(i),8);
	code.addByte(i);
	return Code();
}

class Program {
public:
	Node &ast;
	Node main;

	Program() : ast(NIL) {
	}

	Program(Node node) : ast(node) {
//		collectFunctions();
//		collectLambdas();// closures all over
//		collectImports();
//		collectExports();
//		collectGlobals();
//		collectStrings();
		collectMain();
	}

	void collectMain() {
//		if(!functions["main"])
		main = ast;// root statements form 'main'
	}


//	char *mapp(char* (lambda)(String, int)) {
//		for(Nod n:*this){
//			lambda(n.value, n.index);
//		}
//	}
//	int findIndex(bool (lambda)(Nod)) {
//		int i=0;
//		for(Nod n:*this){
//			if(lambda(n)){
//				return i;
//			}
//			i++;
//		}
//		return -1;
//	}
	Node functions;
};


//Code codeFromProc (ProcStatementNod node, Program program_node) {
//	for(Nod arg:node.args){
//		symbols.insert_or_assign(arg.value, arg.index);
//	}
//	Code code;// not global ;)
//	emitStatements(node.statements);
//
//	auto localCount = symbols.size();
//	bytes locals = localCount > 0 ? encodeLocal(localCount, f32).data : new char[]{};
//	todo();// check if ok: localCount == size of locals ???
////	return encodeVector([...encodeVector(locals), ...code, Opcodes.end]);
//	return encodeVector(Code(locals,localCount).push(code).push(end_block));
//};


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


Code &emit(Program ast) {


	// Function types are vectors of parameters and return types. Currently
	// WebAssembly only supports single return values
//  bytes printFunctionType = new char[]{functionType,encodeVector(f32), emptyArray}
	Code printFunctionType = Code(functionType).push(encodeVector(Code(f32))).push(emptyArray);


	// optimise TODO - some of the procs might have the same type signature
	Code funcTypes;
	for (Node &proc : ast.functions) {
		Code args;
		for (Node arg: *proc.param) { args.addByte(f32); }
		Code functionTypes = Code(functionType).push(encodeVector(args)).push(emptyArray);
	}
//  ast.map(proc {[functionType, encodeVector(proc.args.map(_ -> f32)), emptyArray]);



	// the type section is a vector of function types
//	auto typeSection = createSection(type, encodeVector(printFunctionType).push(funcTypes));
//	char type0[]={0x01,0x60/*const type form*/,0x02/*param count*/,0x7F,0x7F,0x01/*return count*/,0x7F};
	char ii[] = {0x60/*const type form*/, 0x01/*param count*/, 0x7F/*int*/, 0x01/*return count*/, 0x7F/*int*/};
	char vi[] = {0x60/*const type form*/, 0x00/*param count*/, 0x01/*return count*/,
	             0x7F/*int*/};// our main function! todo : be flexible!
	char iv[] = {0x60/*const type form*/, 0x01/*param count*/, 0x7F/*int*/, 0x00/*return count*/};
	char vv[] = {0x60/*const type form*/, 0x00/*param count*/, 0x00/*return count*/};
	int typeCount = 3;
	const Code &type_data = encodeVector(
			Code(typeCount) + Code(vi, sizeof(vi)) + Code(iv, sizeof(iv)) + Code(vv, sizeof(vv)));
	auto typeSection = Code(type, type_data);

	auto lambdo = [](String val, int index) { return index + 1; /* type index */};
//	char func_types[]={0x01,0x00};
//			encodeVector(ast.mapp(lambdo)) TODO

	/* limits https://webassembly.github.io/spec/core/binary/types.html#limits - indicates a min memory size of one page */
	auto memorySection = createSection(memory_section, encodeVector(Code(2)));
	auto memoryImport =
			encodeString("env") + encodeString("memory") + (byte) mem_export/*type*/+ (byte) 0x00 + (byte) 0x01;

	// the import section is a vector of imported functions
	byte type_index = 1;// iv "(i)" int->void
	Code printFunctionImport = encodeString("env") + encodeString("logi").push((char) func_export).push(type_index);

//	auto importSection = createSection(import, encodeVector(Code(0)));
	int import_count = 1;
	auto importSection = createSection(import, Code(import_count) + printFunctionImport);
//	auto importSection = createSection(import, encodeVector(printFunctionImport));//+memoryImport

	// the export section is a vector of exported functions
	int main_offset = import_count;// first func after import functions (which have an index too!)
	auto exportSection = createSection(exports,
	                                   encodeVector(Code(0x01) + encodeString("main") + (byte) func_export +
	                                                Code(main_offset)/*.push(0).push(0)*/ ));
//					             Code(ast.findIndex([](Nod a) { return a.name == "main"; }) + 1)

	// the code section contains vectors of functions

	auto lambde = [](String val, int index) { return index + 1; /* type index */};
//	a -> codeFromProc(a, ast)
// ast.mapp(lambde(ast)); TODO
// https://pengowray.github.io/wasm-ops/
//	char code_data[] = createSection() 0x01,0x08,0x00,0x01,0x3f,0x0F,0x0B};// ok 0x01==nop
//  Code code_data=encodeVectors(encodeVector(1/*function_index/))
//	char code_data[] = {0x01,0x05,0x00,0x41,0x2A,0x0F,0x0B};// 0x41==i32_auto  0x2A==42 0x0F==return 0x0B=='end (function block)' opcode @+39
//function body count


	// the function section is a vector of type indices that indicate the type of each function in the code section
//	char func_types[]={0x01,0x00};
	char types_of_functions[] = {0x02, 0x00, 0x01};// mapping/connecting function index to type index
	// @ WASM : WHY DIDN'T YOU JUST ADD THIS AS A FIELD IN THE FUNC STRUCT???
	Code funcSection = createSection(func, Code(types_of_functions, sizeof(types_of_functions)));

//	char code_data[] = {0x00, 0x41, 0x2A, 0x0F, 0x0B,0x01, 0x05, 0x00, 0x41, 0x2A, 0x0F, 0x0B};
//	char code_data[] = {0x00,0x41,0x2A,0x0F,0x0B};// 0x00 == unreachable as block header !?
//	0a 0e 02 09 00  41 2a 10 00 41 15 0f 0b 02 00
//	0a 0a 02 05 00              41 15 0f 0b 02 00
// fun #c #f        const 42 call oo

//	char code_data[] = {0/*locals_count*/,i32_const,48,call,0 /*logi*/,i32_auto,21,return_block,end_block};// 0x00 == unreachable as block header !?
	char code_data[] = {0/*locals_count*/, i32_auto, 42, return_block,
	                    end_block};// 0x00 == unreachable as block header !?
	char code_data1[] = {0/*locals_count*/, end_block};
//	char code_data[] = {0x00,0x0b,0x02,0x00,0x0b};// empty type:1 len:2


//	Code function1 = codeBlock(code_data);
//	Code da_code2=Code(code_data,sizeof(code_data));
	Code da_code = emitBlock(ast.main);
//	check(da_code2 == da_code);
	Code da_code1 = Code(code_data1, sizeof(code_data1));

	char function_count = 2;
	auto codeSection = createSection(code_section,
	                                 Code(function_count) + encodeVector(da_code) + encodeVector(da_code1));


	auto customSection = createSection(custom, encodeVector(Code(types_of_functions, 3)));
//
	Code code = Code(magicModuleHeader, 4) + Code(moduleVersion, 4) + typeSection + importSection + funcSection +
	            exportSection + codeSection + customSection;
//	Code code = Code(magicModuleHeader, 4) + Code(moduleVersion, 4) + typeSection + funcSection + exportSection + codeSection;// + memorySection + ;
	code.debug();
	return code.clone();
}

Code emitBlock(Node node) {
//	char code_data[] = {0/*locals_count*/,i32_const,42,call,0 /*logi*/,i32_auto,21,return_block,end_block};// 0x00 == unreachable as block header !?
//	char code_data[] = {0/*locals_count*/,i32_auto,21,return_block,end_block};// 0x00 == unreachable as block header !?
//	Code(code_data,sizeof(code_data)); // todo : memcopy, else stack value is LOST
	Code block;
	int locals_count = 0;
	block.add(locals_count);
	Code inner_code_data = emitExpression(node);
	block.push(inner_code_data);
//if not return_block
	block.addByte(return_block);
	block.addByte(end_block);
	return block;
}


Code &emit(Node code) {
	return emit(Program(code));
}