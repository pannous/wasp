#pragma once
//
// Created by me on 08.12.20.
//

#include "String.h"

#ifndef WASP_CODE_H
#define WASP_CODE_H
typedef unsigned char byte;
typedef const char *chars;
typedef byte *bytes;

class Code;

int run_wasm(bytes wasm_bytes, int len);

int run_wasm(chars wasm_path);

extern bytes magicModuleHeader;
extern bytes moduleVersion;

bytes concat(bytes a, bytes b, int len_a, int len_b);

bytes concat(bytes a, char b, int len);

bytes concat(char section, bytes a, int len_a);

Code encodeVector(Code data);

Code &unsignedLEB128(long n);

Code &signedLEB128(long value);

class Code {
public:

	bytes data = 0;
	int length = 0;
	int start = 0;// internal reader pointer
	bool encoded = false;// first byte = size of vector

	Code() {}

	Code(bytes a, int len) {
//		 todo : memcopy, else stack value is LOST
		data = a;
		length = len;
	}

	Code(byte byte) {
		data = (bytes) alloc(1, 1);
		data[0] = byte;
		length = 1;
	}

	Code(int nr, bool LEB = true) {// ambiguous: byte (un)signedLEB128 or int32 !!
		if (LEB) {
//			push(nr, false, LEB);
			push(nr, nr < 0, LEB);
		} else {
			data = new byte[4];
			*(int *) data = nr;
			length = 4;
		}
	}

	Code(long nr, bool LEB = true) {
		if (LEB) {
			push(nr, false, LEB);
		} else {
			data = new byte[8];
			*(long *) data = nr;
			length = 8;
		}
	}

	Code(chars string, bool size_header = true, bool null_terminated = false) {
		long len = (long) strlen0(string);
		if (null_terminated)len++;
		if (size_header) { push(len); }
		push((bytes) string, len);
		if (null_terminated)push((byte) 0);
		// wasm strings start with their length and do NOT end with 0 !! :(
	}

//	Code(char byte){
//		data = static_cast<bytes>(alloc(sizeof(char),1));
//		data[0] = byte;
//		length = 1;
//	}

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
		data = concat(section, code.data, code.length);
		length = code.length + 1;
	}

	Code(char section, bytes dat, int len) {
		data = concat(section, dat, len);
		length = len + 1;
	}

	Code operator+(Code more) {
		return this->push(more);
	}

	Code operator+(char more) {
		return this->push(more);
	}

	Code operator+(byte more) {
		return this->push(more);
	}

	bool operator==(Code &other) {
		if (length != other.length)
			return false;
		if (data == other.data)return true;// same pointer shortcut
		for (int i = 0; i < length; ++i) {
			if (data[i] != other.data[i])
				return false;
		}
		return true;
	}

	operator bytes() { return data; }// implicit cast yay
	Code &push(Code more) {
		data = concat(data, more.data, length, more.length);
		length = length + more.length;
		return *this;
	}

	Code &addType(short type) {
		data = concat(data, type, length);
		length++;
		return *this;
	}

	Code &addByte(byte opcode) {
		data = concat(data, opcode, length);
		length++;
		return *this;
	}

//	Code &add(Code &more) {
//		if (more.length > 0)
//			push(more);
//		return *this;
//	}
	Code &add(Code more) {
		if (more.length > 0)
			push(more);
		return *this;
	}

	Code &add(byte opcode) {
		data = concat(data, opcode, length);
		length++;
		return *this;
	}

	Code &push(char opcode) {
		data = concat(data, opcode, length);
		length++;
		return *this;
	}

	Code &push(unsigned char opcode) {
		data = concat(data, opcode, length);
		length++;
		return *this;
	}

	Code &push(short opcode) {
		data = concat(data, opcode, length);
		length++;
		return *this;
	}

//	All integers are encoded using the LEB128 variable-length integer encoding!  LEB=false should ONLY occur in custom data section!
	Code &push(long nr, bool sign = true, bool LEB = true) {
		Code val;
		if (LEB) {
			if (sign)
				val = signedLEB128(nr);
			else
				val = unsignedLEB128(nr);
			int l = val.length;
			data = concat(data, val.data, length, l);
			length += l;
		} else {
			data = new byte[8];
			*(long *) data = nr;
			length = 8;
		}
		return *this;
	}

	Code &push(bytes more, int len) {
		data = concat(data, more, length, len);
		length = length + len;
		return *this;
	}

	Code &clone() {
		Code *copy = new Code();
		*copy = *this;
		return *copy;
	}

	void debug() {
//		for (int i = 0; i < length; i++)printf("%s%02x", i % 4 == 0 ? " 0x" : "", data[i]);
//		printf("\n");
		save();
	}

	void save(char *file_name = "test.wasm") {
#ifndef WASM
		FILE *file = fopen(file_name, "wb");
		fwrite(data, length, 1, file);
		fclose(file);
#endif
	}

	int run() {
		return run_wasm(data, length);
	}

//	Code& vector() {
//		if(encoded)return *this;
//		Code code = unsignedLEB128(length) + flatten(*this);
//		code.encoded = true;
//		return code;
//	}
	Code rest(int start0 = -1) {
		if (start0 < 0)start0 = start;
		return Code(data + start0, length - start0);
//		return Code(data, start, length);

	}

	// as LEB!
	Code addInt(int i, bool leb = true) {
		push((long) i, true, leb);
		return *this;
	}

	Code addConst(long i) {
		add(0x41 /*i32_const*/);
		push(i);
		return *this;
	}

};


// https://webassembly.github.io/spec/core/binary/types.html
// https://webassembly.github.io/spec/core/binary/values.html
// Wasp/angle has four different types:
// 1. wasm Valtype
// 2. node.kind:Type
// 3. Any<Node and
// 4. some c++ types String List etc
// the last three can be added as special internal values to Valtype, outside the wasm spec
enum Valtype {
	int32 = 0x7f,
	i32t = 0x7f,

	float32 = 0x7d,
	f32t = 0x7d,

	int64 = 0x7E, // signed or unsigned? we don't care
	i64t = 0x7E,

	float64 = 0x7C,
	f64t = 0x7C,

	none = 0x40, // ===
	void_block = 0x40, // VERSUS:
	voids = 0x00, // DANGER!=void_block  internal only for return type

	// SPECIAL INTERNAL TYPES ONLY, not part of spec but they ARE represented through c++=>wasm types (int32?) :
	// enums with the same value can NOT be distinguished thereafter!!! :(
	// todo Signatures need a real Type, not a Valtype!
	//	https://github.com/pannous/angle/wiki/smart-pointer
	pointer = int32,// 0xF0, // internal
	codepoint32 = int32,
	node = pointer,
	array = 13,
	charp = 0xC0, // vs
	string = 0xC0,// use charp?  pointer, // enough no!??
//	value = 0xA1,// wrapped node Value, used as parameter? extract and remove! / ignore
	todoe = 0xF0, // todo
	ignore = 0xAA, // truely internal, should not be exposed! e.g. Arg
};


// https://webassembly.github.io/spec/core/binary/types.html#binary-blocktype
//enum Blocktype { // same as Valuetype
//	void_block = 0x40
//};

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

	i8_load = 0x2d, //== 𝟶𝚡𝟸𝙳, 𝗂𝟥𝟤.𝗅𝗈𝖺𝖽𝟪_u
	i16_load = 0x2f, //== 𝟶𝚡𝟸𝙳, 𝗂𝟥𝟤.𝗅𝗈𝖺𝖽𝟪_u
	i32_load = 0x28,// load word from i32 address
	i32_store = 0x36,// store word at i32 address
//	i32_store_int32= 0x36,
//	i32_store_word= 0x36,
//	i32_store_8 = 0x3a,
//	i32_store_byte = 0x3a,// store byte at i32 address
	i32_auto = (byte) 0x41,
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

	f32_abs = 0x8B,
	f32_neg = 0x8C,

	// todo : difference : ???
	f32_ceil = 0x8D,
	f32_floor = 0x8E,
	f32_trunc = 0x8F,
	f32_round = 0x90,// truncation ≠ proper rounding!
	f32_nearest = 0x90,

	f32_sqrt = 0x91,
	f32_add = 0x92,
	f32_sub = 0x93,
	f32_mul = 0x94,
	f32_div = 0x95,

	local_get = 0x20,
	local_set = 0x21,
	local_tee = 0x22,
	global_get = 0x23,
	global_set = 0x24,

// todo : peek 65536 as float directly via opcode
	i64_load = 0x29, // memory.peek memory.get memory.read
	i64_store = 0x37, // memory.poke memory.set memory.write

	f32_cast_to_i32_s = 0xa8,// truncation ≠ proper rounding (f32_round = 0x90)!
	i32_trunc_f32_s = 0xa8, // cast/convert != reinterpret
	f32_convert_i32_s = 0xB2,// convert FROM i32
	i32_cast_to_f32_s = 0xB2,

	i32_reinterpret_f32 = 0xbc, // f32->i32 bit wise reinterpret != cast/trunc/convert
	f32_reinterpret_i32 = 0xBE, // i32->f32

};

// https://webassembly.github.io/spec/core/binary/modules.html#sections
enum Section {
	// with the exception of custom, these Sections must appear in the following order:
	custom_section = (byte) 0,
	type_section = 1,
	import_section = 2,
	functypes_section = 3,
	table_section = 4,
	memory_section = 5,
	global_section = 6,
	export_section = 7,
	start_section = 8,
	element_section = 9,
	code_section = 10, // 0x0a
	data_section = 11
};

enum nameSubSectionTypes {
	module_name = 0,
	function_names = 1,
	local_names = 2,
};

class Signature {
public:
	String function = "";// could be reused by multiple, but useful to debug
	Map<int, Valtype> types;
	Valtype return_type = voids;
	bool is_import = false; // not serialized in functype section, but in import section wt
	bool is_runtime = false;
	bool is_builtin = false;// hard coded functions, tests only? todo remove
	bool is_handled = false; // already emitted (e.g. as runtime)
	bool is_used = false;// called
	bool emit = false;// only those types/functions that are declared (export) or used in call

	int size() {
		return types.size();
	}

	Signature handled() {
		is_handled = true;
		return *this;
	}

	Signature import() {
		is_import = true;
		return *this;
	}

	Signature builtin() {
		is_builtin = true;
		return *this;
	}

	Signature runtime() {
		is_runtime = true;
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

Code createSection(Section sectionType, Code data);

Code &unsignedLEB128(long n);

Code &signedLEB128(long value);

#endif //WASP_CODE_H
