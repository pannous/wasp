#pragma once
//
// Created by me on 08.12.20.
//

#include "String.h"

#ifndef WASP_CODE_H
#define WASP_CODE_H
typedef unsigned char byte;
typedef const char* chars;
typedef byte* bytes;
class Code;
int run_wasm(bytes wasm_bytes, int len);
int run_wasm(chars wasm_path);


bytes concat(bytes a, bytes b, int len_a, int len_b);
bytes concat(bytes a, char b,int len);
bytes concat(char section, bytes a, int len_a);
Code encodeVector(Code data);
Code &unsignedLEB128(long n);
Code &signedLEB128(long value);
class Code{
public:

	bytes data=0;
	int length=0;
	int start=0;// internal reader pointer
	bool encoded= false;// first byte = size of vector

	Code(){}
	Code(bytes a, int len){
//		 todo : memcopy, else stack value is LOST
		data=a;
		length=len;
	}

	Code(chars string,bool size_header=true,bool null_terminated=false) {
		long len = (long) strlen0(string);
		if(null_terminated)len++;
		if(size_header){ push(len); }
		push((bytes)string, len);
		if(null_terminated)push((byte )0);
		// wasm strings start with their length and do NOT end with 0 !! :(
	}

//	Code(char byte){
//		data = static_cast<bytes>(alloc(sizeof(char),1));
//		data[0] = byte;
//		length = 1;
//	}

	Code(byte byte){
		data = static_cast<bytes>(alloc(sizeof(char),1));
		data[0] = byte;
		length = 1;
	}
	/*Code(bytes datas, int from, int to*//*exclusive*//*) {
		// AMBIGUOUS: from offset on DATA?
		if(from<0 or to<0 or from>=to)
			error("bad indices");
		data = datas+from;// just reuse, if created with new
		start=from;// NO! start=0 refers to OWN data!
		length = to-from;
//		data = static_cast<bytes>(alloc(sizeof(char), to - from));
//		memcpy0(data,datas+from,to-from);
	}*/

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

	Code& add(Code more){
		if(more.length>0)
			push(more);
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

	Code& push(long nr, bool sign= true) {
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
#endif
	}
	int run(){
		return run_wasm(data, length);
	}

//	Code& vector() {
//		if(encoded)return *this;
//		Code code = unsignedLEB128(length) + flatten(*this);
//		code.encoded = true;
//		return code;
//	}
	Code rest(int start0=-1) {
		if(start0<0)start0=start;
		return Code(data+start0, length-start0);
//		return Code(data, start, length);

	}
};



// https://webassembly.github.io/spec/core/binary/types.html
// https://webassembly.github.io/spec/core/binary/values.html
enum Valtype {
	i32t = 0x7f,
	f32t = 0x7d,
	i64t = 0x7E,
	f64t = 0x7C,
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
// USE wasm-objdump -d  to see function disassembled:
enum Opcodes {
//	start = 0x00,
	start_function = 0x00,
//	unreachable = 0x00,
	nop = 0x01, // useful for relocation padding call 1 -> call 10000000
	block = 0x02,
	loop = 0x03,
	if_i = 0x04,// precede by i32 result, follow by i32_type (7f)
	elsa = 0x05,
	br = 0x0c,
	br_if = 0x0d,
	end_block = 0x0b, //11
	return_block = 0x0f,
	function = 0x10,
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
	f32_ne = 0x5c, // !=
	f32_lt = 0x5d,
	f32_gt = 0x5e,
	f32_le = 0x5f,
	f32_ge = 0x60,

	f64_eq = 0x61,
	f64_ne = 0x62, // !=
	f64_lt = 0x63,
	f64_gt = 0x64,
	f64_le = 0x65,
	f64_ge = 0x66,

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

// https://webassembly.github.io/spec/core/binary/modules.html#sections
enum Section {
	// with the exception of custom, these Sections must appear in the following order:
	custom = 0,
	type = 1,
	import = 2,
	functypes = 3,
	table = 4,
	memory_section = 5,
	global = 6,
	exports = 7,
	start_section = 8,
	element = 9,
	code_section = 10, // 0x0a
	data_section = 11
};

class Signature {
public:
	Map<int, Valtype> types;
	Valtype return_type = voids;
	bool is_import = false; // not serialized in functype section, but in import section wt

	int size() {
		return types.size();
	}
	Signature import(){
		is_import = true;
		return *this;
	}
	Signature add(Valtype t) {
		types.insert_or_assign(types.size(), t);
		return *this;
	}

	Signature &returns(Valtype valtype) {
		return_type = valtype;
//		return_types[name] = valtype;
		return *this;
	}
};

String sectionName(Section section);
Code createSection(Section sectionType, Code data) ;
Code& unsignedLEB128(long n);
Code& signedLEB128(long value);

#endif //WASP_CODE_H