#pragma once

#include "wasm_runner.h"

typedef char* bytes;
bytes concat(bytes a, bytes b, int len_a, int len_b);
bytes concat(bytes a, char b,int len);
bytes concat(char section, bytes a, int len_a);

class String;
class Nod;
class ExpressionNod;
class StatementNod;
class ProcStatementNod;
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
		for (int i = 0; i < length; i++)printf("%s%02x",i%4==0?" 0x":"", data[i]);
		printf("\n");
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
	void run(){
		run_wasm(data, length);
	}

//	Code& vector() {
//		if(encoded)return *this;
//		Code code = unsignedLEB128(length) + flatten(*this);
//		code.encoded = true;
//		return code;
//	}
};

// https://webassembly.github.io/spec/core/binary/modules.html#sections
enum Section {
	custom = 0,
	type = 1,
	import = 2,
	func = 3,
	table = 4,
	memory_section = 5,
	global = 6,
	exports = 7,
	start_section = 8,
	element = 9,
	code_section = 10, // 0x0a
	data = 11
};

// https://webassembly.github.io/spec/core/binary/types.html
enum Valtype {
	i32 = 0x7f,
	f32 = 0x7d
};

// https://webassembly.github.io/spec/core/binary/types.html#binary-blocktype
enum Blocktype {
	void_block = 0x40
};

// https://webassembly.github.io/spec/core/binary/instructions.html
enum Opcodes {
//	start = 0x00,
	start_function = 0x00,
//	unreachable = 0x00,
	block = 0x02,
	loop = 0x03,
	br = 0x0c,
	br_if = 0x0d,
	end_block = 0x0b,
	return_block = 0x0f,
	call = 0x10,
	get_local = 0x20,
	set_local = 0x21,
	i32_store_8 = 0x3a,
	i32_auto = 0x41,
	f32_auto = 0x43,
	i32_eqz = 0x45,
	i32_eq = 0x46,
	f32_eq = 0x5b,
	f32_lt = 0x5d,
	f32_gt = 0x5e,
	i32_and = 0x71,
	f32_add = 0x92,
	f32_sub = 0x93,
	f32_mul = 0x94,
	f32_div = 0x95,
	i32_trunc_f32_s = 0xa8
};
//char start_function=0x00;//unreachable strange convention
extern char unreachable;//=0x00;//unreachable strange convention
class TransformedProgram;
Code& emitter(TransformedProgram* ast);
