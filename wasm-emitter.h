#pragma once

#include "wasm_runner.h"

typedef const char *wasm_string;// wasm strings start with their length and do NOT end with 0 !! :(
typedef char* bytes;
bytes concat(bytes a, bytes b, int len_a, int len_b);
bytes concat(bytes a, char b,int len);
bytes concat(char section, bytes a, int len_a);
class Code;
Code& unsignedLEB128(long n);
Code& signedLEB128(long value);


class String;
class Nod;
class ExpressionNod;
//class Block;
//class Block : Node{
//
//	Block *begin() const {
//		return children;  CANT specialize/INHERIT:  obviously
//	}
//
//	Block *end() const{
//		return children + length;
//	}
//};
class ProcStatementNod;
class Code{
public:

	bytes data;
	int length=0;
	bool encoded= false;// first byte = size of vector

	Code(){}
	Code(bytes a, int len){
//		 todo : memcopy, else stack value is LOST
		data=a;
		length=len;
	}

	Code(const char *string,bool size_header=true,bool null_terminated=false) {
		long len = (long) strlen0(string);
		if(null_terminated)len++;
		if(size_header){ push(len); }
		push((bytes)string, len);
		if(null_terminated)push((byte )0);
		// wasm strings start with their length and do NOT end with 0 !! :(
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
	Code operator +(byte more){
		return this->push(more);
	}
	bool operator==(Code& other){
		if(length!=other.length)
			return false;
		if(data==other.data)return true;// same pointer shortcut
		for (int i = 0; i < length; ++i) {
			if(data[i]!=other.data[i])
				return false;
		}
		return true;
	}

	operator bytes(){return data;}// implicit cast yay
	Code& push(Code more) {
		data = concat(data, more.data,length,more.length);
		length = length + more.length;
		return *this;
	}

	Code& addType(short type) {
		data = concat(data, type,length);
		length++;
		return *this;
	}

	Code& addByte(byte opcode) {
		data = concat(data, opcode,length);
		length++;
		return *this;
	}

	Code& add(byte opcode) {
		data = concat(data, opcode,length);
		length++;
		return *this;
	}

	Code& push(char opcode) {
		data = concat(data, opcode,length);
		length++;
		return *this;
	}
	Code& push(unsigned char opcode) {
		data = concat(data, opcode,length);
		length++;
		return *this;
	}
	Code& push(short opcode) {
		data = concat(data, opcode,length);
		length++;
		return *this;
	}

	Code& push(long nr, bool sign= false) {
		Code val;
		if(sign)
			val = signedLEB128(nr);
		else
		   val = unsignedLEB128(nr);
		int l = val.length;
		data = concat(data, val.data, length, l);
		length+= l;
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
//		FILE *result=popen(command, "r");
//		char buf[100000];
//		while(fgets(buf, sizeof(buf), result) != NULL) {
//		printf("%s",buf);
//		}
#endif
	}
	int run(){
//		char *command = "wasmx test.wasm";
//		int ok=system(command);
		return run_wasm(data, length);
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
	data_section = 11
};


// https://webassembly.github.io/spec/core/binary/types.html
// https://webassembly.github.io/spec/core/binary/values.html
enum Valtype {
	i32 = 0x7f,
	f32 = 0x7d,
	i64 = 0x7E,
	f64 = 0x7C,
	int32 = 0x7f,
	float32 = 0x7d,
	int64 = 0x7E,
	float64 = 0x7C,
	none = 0x40,
	voids = 0x00, // internal only for return type
};

// https://webassembly.github.io/spec/core/binary/types.html#binary-blocktype
enum Blocktype {
	void_block = 0x40
};

// https://webassembly.github.io/spec/core/binary/instructions.html
// https://pengowray.github.io/wasm-ops/
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
	drop = 0x1a, // pop stack
	get_local = 0x20,// get to stack
	set_local = 0x21,// set and pop
	tee_local = 0x22,// set and leave on stack

	i32_store_8 = 0x3a,
	i32_auto = (byte)0x41,
	i32_const = 0x41,
	i64_auto = 0x42,
	i64_const = 0x42,
	f32_auto = 0x43,

	i32_eqz = 0x45, // use for not!
	i32_eq = 0x46,
	i32_ne = 0x47,
	i32_lt = 0x48,
	i32_gt = 0x4A,
	i32_le = 0x4C,
	i32_ge = 0x4E,

	i64_eqz = 0x50,
	f32_eqz = 0x50, // HACK: no such thing!

	f32_eq = 0x5b,
	f32_ne = 0x5c,
	f32_lt = 0x5d,
	f32_gt = 0x5e,

	i32_add = 0x6A,
	i32_sub = 0x6B,
	i32_mul = 0x6C,
	i32_div = 0x6D,
	i32_rem = 0x6F, // 5%4=1
	i32_modulo = 0x6F,

	i32_and = 0x71,
	i32_or = 0x72,
	i32_xor = 0x73,

	f32_add = 0x92,
	f32_sub = 0x93,
	f32_mul = 0x94,
	f32_div = 0x95,
	i32_trunc_f32_s = 0xa8, // cast/convert != reinterpret
	i32_reinterpret_f32 = 0xbc // bit wise reinterpret != cast/trunc/convert

};
//char start_function=0x00;//unreachable strange convention
extern char unreachable;//=0x00;//unreachable strange convention
Code &emit(Node code);

