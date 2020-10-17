// BASED ON https://github.com/ColinEberhardt/chasm/blob/master/src/emitter.ts
//https://github.com/ColinEberhardt/chasm/blob/master/src/encoding.ts
#include "String.h"
#include "Map.h"
#include "wasm-emitter.h"
//#include "string.h" // memcpy
void * memcpy ( void * destination, const void * source, size_t num );
class Code;
typedef char* bytes;
typedef char byter[];

//Code& unsignedLEB128(int);
//Code& flatten(byter);
//Code& flatten (Code& data);
void todo() {
	printf("TODO ");
}

//typedef int number;
//typedef char byte;
//typedef char* Code;
//typedef char id;




bytes concat(bytes a, bytes b,int len_a, int len_b) {
	bytes c=new char[len_a+len_b+1];
	memcpy(c, a, len_a);
	memcpy(c+len_a, b, len_b);
	return c;
}
bytes concat(byter a, char b,int len) {
	bytes c=new char[len+1];
	memcpy(c, a, len);
	c[len]=b;
	return c;
}
bytes concat(char a, byter  b,int len) {
	bytes c=new char[len+1];
	c[0]=a;
	memcpy(c+1, b, len);
	return c;
}

class String;
class Nod;
class ExpressionNod;
class StatementNod;
class ProcStatementNod;
class Nod{
public:
	Nod(StatementNod &alternate, ExpressionNod &args, StatementNod &consequent, ExpressionNod &expression,
	    ExpressionNod &initializer, String &name, StatementNod &statements, String &typeName, String &value)
			: alternate(alternate), args(args), consequent(consequent), expression(expression),
			  initializer(initializer), name(name), statements(statements), type_name(typeName), value(value) {

	}

	Nod(StatementNod *alternate, ExpressionNod *args, StatementNod *consequent, ExpressionNod *expression,
			ExpressionNod *initializer, String *name, StatementNod *statements, String *typeName, String *value)
	: alternate(*alternate), args(*args), consequent(*consequent), expression(*expression),
	initializer(*initializer), name(*name), statements(*statements), type_name(*typeName), value(*value) {

	}
	Nod():Nod(0,0,0,0,0,0,0,0,0){}

	String& type_name;
	int type;

	String& value;// variable name or operator "+" …
	ExpressionNod& expression;
	ExpressionNod& initializer;
	String& name;
	ExpressionNod& args;
	StatementNod& statements;
	StatementNod& consequent;
	StatementNod& alternate;
	int index;// for args
	StatementNod *begin() const{
		todo();
		return &statements;
	}
	Nod *end() const{
		todo();
		return 0;
	}

	Nod& operator[](int i){
		todo();
		return *this;
	}
};
class ExpressionNod : public Nod{
public:
	ExpressionNod() {}

	ExpressionNod& operator[](int i){
		todo();
		return *this;
	}
};

class StatementNod : public Nod{
public:
	StatementNod() {}
	StatementNod& operator[](int i){
		todo();
		return *this;
	}
};

class ProcStatementNod : public Nod{
public:
	ProcStatementNod() {}
	ProcStatementNod& operator[](int i){
		todo();
		return *this;
	}
};

class Code{
public:
	bytes data;
	int length=0;
	bool encoded= false;// first byte = size of vector

	Code(){}
	Code(bytes a, int len){
		data=a;
		length=len;
	}

	Code(char byte){
		data = static_cast<bytes>(alloc(sizeof(char),1));
		data[0] = byte;
		length = 1;
	}
	Code(char section, Code code) {
		data = concat(section, code.data,code.length);
		length = code.length+1;
	}

	Code(char section, bytes dat, int len) {
		data = concat(section, dat, len);
		length=len+1;
	}

	Code operator +(Code more){
		return this->push(more);
	}
	Code operator +(char more){
		return this->push(more);
	}

	operator bytes(){return data;}// implicit cast yay
	Code& push(Code more) {
		data = concat(data, more.data,length,more.length);
		length = length + more.length;
		return *this;

	}

	Code& push(char opcode) {
		data = concat(data, opcode,length);
		length++;
		return *this;

	}
	Code& push(bytes more,int len) {
		data = concat(data, more,length,len);
		length = length + len;
		return *this;
	}

	Code &clone() {
		return *this;
	}

	void debug() {
		for (int i = 0; i < length; i++)printf("%02x", data[i]);
		save();
	}
	void save(char* file_name="test.wasm"){
#ifndef WASM
		FILE* file=fopen(file_name,"w");
		fwrite(data, length, 1, file);
		fclose(file);
		char *command = "wasmx test.wasm";
		int ok=system(command);
//		FILE *result=popen(command, "r");
//		char buf[100000];
//		while(fgets(buf, sizeof(buf), result) != NULL) {
//		printf("%s",buf);
//		}
#endif
	}

//	Code& vector() {
//		if(encoded)return *this;
//		Code code = unsignedLEB128(length) + flatten(*this);
//		code.encoded = true;
//		return code;
//	}
};
//#include <iostream>

class Bytes{
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
class BinaryOpcode{
	int operator [](const char *s){
		if(eq(s, "+"))return f32_add;
		if(eq(s, "-"))return f32_sub;
		if(eq(s, "*"))return f32_mul;
		if(eq(s, "/"))return f32_div;
		if(eq(s, "=="))return f32_eq;
		if(eq(s, ">"))return f32_gt;
		if(eq(s, "<"))return f32_lt;
		if(eq(s, "&&"))return i32_and;
		error("invalid operator");
		return -1;
	}

public:
	int operator [](String& s){
		if(eq(s, "+"))return f32_add;
		if(eq(s, "-"))return f32_sub;
		if(eq(s, "*"))return f32_mul;
		if(eq(s, "/"))return f32_div;
		if(eq(s, "=="))return f32_eq;
		if(eq(s, ">"))return f32_gt;
		if(eq(s, "<"))return f32_lt;
		if(eq(s, "&&"))return i32_and;
		error("UNKNOWN OP");
		return -1;
	}
};
BinaryOpcode binaryOpcode;
//std::map<std::string, int> binaryOpcode  = {
//		{"+", f32_add},
//		{"-", f32_sub},
//		{"*", f32_mul},
//		{"/", f32_div},
//		{"==", f32_eq},
//		{">", f32_gt},
//		{"<", f32_lt},
//		{"&&", i32_and}
//};;

// http://webassembly.github.io/spec/core/binary/modules.html#export-section
enum ExportType {
	func_export = 0x00,
	table_export = 0x01,
	mem_export = 0x02,
	global_export = 0x03
};

// http://webassembly.github.io/spec/core/binary/types.html#function-types
char functionType = 0x60;

char emptyArray = 0x0;

// https://webassembly.github.io/spec/core/binary/modules.html#binary-module
bytes magicModuleHeader = new char[] { 0x00, 0x61, 0x73, 0x6d };
bytes moduleVersion = new char[] { 0x01, 0x00, 0x00, 0x00 };



bytes flatten(byter data) {
	todo();
	return data;
}

Code& flatten(Code& data) {
	todo();
	return data;
}
#ifdef WASM
typedef char uint8_t;
#endif

typedef uint8_t byt;
Code unsignedLEB128(int n) {
	Code buffer;
//	Code* buffer=new Code(); // IF RETURNING Code&
	do {
		byt byte = n & 0x7f;
		n = n >> 7;
		if (n != 0) {
			byte |= 0x80;
		}
		buffer.push(byte);
	} while (n != 0);
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
Code encodeVector (Code data) {
//	return data.vector();
	if(data.encoded)return data;
	Code code = unsignedLEB128(data.length) + flatten(data);
	code.encoded = true;
	return code;
}

// https://webassembly.github.io/spec/core/binary/modules.html#code-section
Code encodeLocal (long count, Valtype type) {
	return unsignedLEB128(count).push(type);
}

// https://webassembly.github.io/spec/core/binary/modules.html#sections
// sections are encoded by their type followed by their vector contents
Code createSection (Section sectionType, Code data) {
	return Code(sectionType, encodeVector(data));
}
//std::map<String ,int> symbols;
Map<String ,int> symbols;

int localIndexForSymbol (String& name) {
	if (!symbols.contains(name)) {
		symbols.insert_or_assign(name, symbols.size());
	}
	return symbols[name];
};;
int localIndexForSymbol (char* name) {
	todo();
	if (!symbols.contains(name)) {
		symbols.insert_or_assign(name, symbols.size());
	}
	return symbols[name];
};;

enum NodTypes{
	numberLiteral,
	identifier,
	binaryExpression,
	printStatement,
	variableDeclaration,
	variableAssignment,
	whileStatement,
	ifStatement,
	callStatement,
};

bytes ieee754(String& value) {
	todo();
	return value.data;
}

bytes ieee754(float num) {
	todo();
	byt data[4];
	float* hack=((float*)data);
	*hack = num;
	return reinterpret_cast<bytes>(hack);
}

Code emitExpression (ExpressionNod nodes) {
//Code emitExpression (Nod nodes) {
	Code code;
	for (Nod node : nodes) {
		switch (node.type) {
			case numberLiteral:
				code.push(f32_auto);
				code.push(ieee754(node.value),4);
				break;
			case identifier:
				code.push(get_local);
				code.push(unsignedLEB128(localIndexForSymbol(node.value)), 8);
				break;
			case binaryExpression:
				code.push(binaryOpcode[node.value]);
				break;
		};
	}
	return code;
}


Code emitStatements (StatementNod statements) {
	Code code;// global object for now
	for (Nod statement : statements) {
		switch (statement.type) {
			case printStatement:
				emitExpression(statement.expression);
				code.push(call);
				code.push(unsignedLEB128(0),8);
				break;
			case variableDeclaration:
				emitExpression(statement.initializer);
				code.push(set_local);
				code.push(unsignedLEB128(localIndexForSymbol(statement.name)), 8);
				break;
			case variableAssignment:
				todo();
//				emitExpression(statement.value);
				code.push(set_local);
				code.push(unsignedLEB128(localIndexForSymbol(statement.name)), 8);
				break;
			case whileStatement:
				// outer block
				code.push(block);
				code.push(void_block);
				// inner loop
				code.push(loop);
				code.push(void_block);
				// compute the while expression
				emitExpression(statement.expression);
				code.push(i32_eqz);
				// br_if $label0
				code.push(br_if);
				code.push(signedLEB128(1));
				// the nested logic
				emitStatements(statement.statements);
				// br $label1
				code.push(br);
				code.push(signedLEB128(0));
				// end loop
				code.push(end_block);
				// end block
				code.push(end_block);
				break;
			case ifStatement:
				// if block
				code.push(block);
				code.push(void_block);
				// compute the if expression
				emitExpression(statement.expression);
				code.push(i32_eqz);
				// br_if $label0
				code.push(br_if);
				code.push(signedLEB128(0));
				// the nested logic
				emitStatements(statement.consequent);
				// end block
				code.push(end_block);

				// else block
				code.push(block);
				code.push(void_block);
				// compute the if expression
				emitExpression(statement.expression);
				code.push(i32_auto);
				code.push(signedLEB128(1));
				code.push(i32_eq);
				// br_if $label0
				code.push(br_if);
				code.push(signedLEB128(0));
				// the nested logic
				emitStatements(statement.alternate);
				// end block
				code.push(end_block);
				break;
			case callStatement:
				if (statement.name == "setpixel") {
					// compute and cache the setpixel parameters
					emitExpression(statement.args[0]);
					code.push(set_local);
					code.push(unsignedLEB128(localIndexForSymbol("x")));

					emitExpression(statement.args[1]);
					code.push(set_local);
					code.push(unsignedLEB128(localIndexForSymbol("y")));

					emitExpression(statement.args[2]);
					code.push(set_local);
					code.push(unsignedLEB128(localIndexForSymbol("color")));

					// compute the offset (x * 100) + y
					code.push(get_local);
					code.push(unsignedLEB128(localIndexForSymbol("y")));
					code.push(f32_auto);
					code.push(ieee754(100),4);
					code.push(f32_mul);

					code.push(get_local);
					code.push(unsignedLEB128(localIndexForSymbol("x")));
					code.push(f32_add);

					// convert to an integer
					code.push(i32_trunc_f32_s);

					// fetch the color
					code.push(get_local);
					code.push(unsignedLEB128(localIndexForSymbol("color")));
					code.push(i32_trunc_f32_s);

					// write
					code.push(i32_store_8);
					code.push((char) 0x00);
					code.push((char) 0x00); // align and offset
				} else {
					for (Nod arg : statement.args) {
						emitExpression(*((ExpressionNod *) &arg));
					};
					int index = 0;
					todo();
//            findIndex(f {f.name == statement.name);
					code.push(call);
					code.push(unsignedLEB128(index + 1));
				};
				break;
		};
	}
	return code;
}
class TransformedProgram:public Nod{
public:
	TransformedProgram() {}

//	char *mapp(char* (lambda)(String, int)) {
//		for(Nod n:*this){
//			lambda(n.value, n.index);
//		}
//	}
	int findIndex(bool (lambda)(Nod)) {
		int i=0;
		for(Nod n:*this){
			if(lambda(n)){
				return i;
			}
			i++;
		}
		return -1;
	}
};


Code codeFromProc (ProcStatementNod node, TransformedProgram program_node) {
	for(Nod arg:node.args){
		symbols.insert_or_assign(arg.value, arg.index);
	}
	Code code;// not global ;)
	emitStatements(node.statements);

	auto localCount = symbols.size();
	bytes locals = localCount > 0 ? encodeLocal(localCount, f32).data : new char[]{};
	todo();// check if ok: localCount == size of locals ???
//	return encodeVector([...encodeVector(locals), ...code, Opcodes.end]);
	return encodeVector(Code(locals,localCount).push(code).push(end_block));
};


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
			result.push(byt);
			return result;
		}
		result.push(byt | 0x80);
	}
//	return result.data;
}


Code& emitter(TransformedProgram* ast0) {
	TransformedProgram ast;

	// Function types are vectors of parameters and return types. Currently
	// WebAssembly only supports single return values
//  bytes printFunctionType = new char[]{functionType,encodeVector(f32), emptyArray}
	Code printFunctionType = Code(functionType).push(encodeVector(Code(f32))).push(emptyArray);

	ProcStatementNod proc;

	// optimise TODO - some of the procs might have the same type signature
	Code funcTypes;
	for (Nod &node : ast) {
		Code args;
		for (Nod arg: proc.args) { args.push(f32); }
		Code functionTypes = Code(functionType).push(encodeVector(args)).push(emptyArray);
	}
//  ast.map(proc {[functionType, encodeVector(proc.args.map(_ -> f32)), emptyArray]);



	// the type section is a vector of function types
//	auto typeSection = createSection(type, encodeVector(printFunctionType).push(funcTypes));
//	char type0[]={0x01,0x60/*const type form*/,0x02/*param count*/,0x7F,0x7F,0x01/*return count*/,0x7F};
	char type0[]={0x60/*const type form*/,0x01/*param count*/,0x7F/*int*/,0x01/*return count*/,0x7F/*int*/};
	char type1[]={0x60/*const type form*/,0x00/*param count*/,0x00/*return count*/};
	int typeCount=2;
	const Code &type_data = encodeVector(Code(typeCount) + Code(type0, sizeof(type0))+ Code(type1, sizeof(type1)));
	auto typeSection = Code(type, type_data);

	auto lambdo = [](String val, int index) { return index + 1; /* type index */};
//	char func_types[]={0x01,0x00};
//			encodeVector(ast.mapp(lambdo)) TODO

	/* limits https://webassembly.github.io/spec/core/binary/types.html#limits - indicates a min memory size of one page */
	auto memorySection = createSection(memory_section, encodeVector(Code(2)));
	auto memoryImport = encodeString("env")+ encodeString("memory")+mem_export/*type*/+ 0x00+ 0x01;

	// the import section is a vector of imported functions
	Code printFunctionImport = encodeString("env") + encodeString("logi").push(func_export).push(0x00)/* type index*/ ;

//	auto importSection = createSection(import, encodeVector(Code(0)));
	int import_count = 1;
	auto importSection = createSection(import, Code(import_count)+printFunctionImport);
//	auto importSection = createSection(import, encodeVector(printFunctionImport));//+memoryImport

	// the export section is a vector of exported functions
	int main_offset=import_count;// first func after import functions (which have an index too!)
	auto exportSection = createSection(exports,
			encodeVector(Code(0x01)+ encodeString("main")+ func_export + Code(main_offset)/*.push(0).push(0)*/ ));
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
	char func_types[]={0x02,0x00,0x01};
	Code funcSection = createSection(func, Code(func_types,sizeof(func_types)));

//	char code_data[] = {0x00, 0x41, 0x2A, 0x0F, 0x0B,0x01, 0x05, 0x00, 0x41, 0x2A, 0x0F, 0x0B};
//	char code_data[] = {0x00,0x41,0x2A,0x0F,0x0B};// 0x00 == unreachable as block header !?
	char code_data[] = {0/*locals_count*/,i32_auto,42,return_block,end_block};// 0x00 == unreachable as block header !?
	char code_data1[] = {0/*locals_count*/,end_block};
//	char code_data[] = {0x00,0x0b,0x02,0x00,0x0b};// empty type:1 len:2


//	Code function1 = codeBlock(code_data);
	Code da_code=Code(code_data,sizeof(code_data));
	Code da_code1=Code(code_data1,sizeof(code_data1));
	char function_count = 2;
	auto codeSection = createSection(code_section, Code(function_count)+encodeVector(da_code)+encodeVector(da_code1));

	auto customSection = createSection(custom, encodeVector(Code(func_types,3)));
//
	Code code = Code(magicModuleHeader, 4) + Code(moduleVersion, 4) + typeSection + importSection + funcSection + exportSection + codeSection + customSection ;
//	Code code = Code(magicModuleHeader, 4) + Code(moduleVersion, 4) + typeSection + funcSection + exportSection + codeSection;// + memorySection + ;
	code.debug();
	return code.clone();
}

