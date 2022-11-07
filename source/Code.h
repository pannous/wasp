#pragma once
//
// Created by me on 08.12.20.
//

#include "String.h"
#include "Map.h"
#include "Node.h"
#include "List.h"
#ifndef PURE_WASM
#include <stdio.h>
#endif
//size_t strlen(const char *__s);

#ifndef WASP_CODE_H
#define WASP_CODE_H
typedef unsigned char byte;
typedef const char *chars;
typedef byte *bytes;

// in Util.h but we cant import
bytes concat(bytes a, bytes b, int len_a, int len_b);

bytes concat(bytes a, char b, int len);

bytes concat(char section, bytes a, int len_a);

class Code;

// Different environments offer different run_wasm implementations:
// wasm JIT runtimes: wasm3, wasm-micro-runtime, wabt, V8 via webview:
// wasmer via console, node/browser via import, webview  …
long run_wasm(bytes wasm_bytes, int len);

long run_wasm(chars wasm_path);

extern bytes magicModuleHeader;
extern bytes moduleVersion;

Code encodeVector(Code data);

Code &unsignedLEB128(long n);

Code &signedLEB128(long value);

class Code {
public:
	Code(byte *data0, int from, int pos) {
		data = data0;
		start = from;
//		pos=???
	}

	virtual ~Code() {
		data = 0;
	}

	int header = array_header_32;// todo: code_header_32 if extra fields are needed beyond standard
	int kind = byte_char;
	int length = 0;
	bytes data = 0;
	int start = 0;// internal reader pointer
	bool encoded = false;// first byte = size of vector

	Code() {}

	Code(bytes a, int len, bool needs_copy = true) {
		data = a;
		length = len;
		if (needs_copy) {
			data = static_cast<bytes>(malloc(length));
			memcpy(data, a, length);
		}
	}

	Code(char *datas, int size, bool needs_copy = true) : Code((bytes) datas, size, needs_copy) {}


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
		short len = strlen0(string);
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


	Code operator++() {
		if (length == 0)return Code();
//		start++;
		data++;
		length--;
		return *this;
	}

	Code operator++(int postfix) {
		if (postfix == 0)postfix = 1;
		if (length < postfix)return Code();
		data += postfix;
		length -= postfix;
		return *this;
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

	Code addByte(byte opcode) {
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
	Code &push(long long nr, bool sign = true, bool LEB = true) {
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

	Code &push(long nr, bool sign = true, bool LEB = true) {
		return push((long long) nr, sign, LEB);
	}

	Code &push(int nr, bool sign = true, bool LEB = true) {
		return push((long long) nr, sign, LEB);
	}

	Code &push(unsigned int nr, bool sign = true, bool LEB = true) {
		return push((long long) nr, sign, LEB);
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
		if(!String(file_name).endsWith(".wasm"))
			file_name =(char *) concat(file_name, ".wasm");
		FILE *file = fopen(file_name, "wb");
		fwrite(data, length, 1, file);
		fclose(file);
#endif
	}

	long run() {
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

	Code addConst(unsigned int i) {
		add(0x41 /*i32_const*/);
		if (i > 0x80000000)push(-(long) i, false, true);// stupid sign bit hack
		else push(i, true, true);
//		else push(i, false, true);
		return *this;
	}

	Code addConst64(long long i) {
//		if (i < 0x100000000 and i > -0x100000000)
//			add(0x41 /*i32_const*/);
//		else
		add(0x42 /* i64_const */);
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
	unknown_type = -1,

	voids = 0x00, // DANGER!=void_block  internal only for return type

	void_block = 0x40,
	none = 0x40, // NOT voids!!!

// extensions
	anyref = 0x6f,// was conceptually an namewise merged into externref
	externref = 0x6f, // -0x11
	funcref = 0x70, // -0x10
	func = 0x60,

	int32 = 0x7f,
	i32t = 0x7f,
	i32 = 0x7f,
	i32s = 0x7f,
//	i32u = 0x7f,// todo ignore!

	float32 = 0x7d,
	f32t = 0x7d,
	f32 = 0x7d,
	f32s = 0x7d,
//	f32u = 0x7d,// todo ignore!

	int64 = 0x7E, // signed or unsigned? we don't care
	i64t = 0x7E,
	i64 = 0x7E,
	i64s = 0x7E,

	float64 = 0x7C,
	f64t = 0x7C,
	f64 = 0x7C,


	// SPECIAL INTERNAL TYPES ONLY, not part of spec but they ARE represented through c++=>wasm types (int32?) :
	// enums with the same value can NOT be distinguished thereafter!!! :(
	// todo Signatures need a real Type, not a Valtype!
	//	https://github.com/pannous/angle/wiki/smart-pointer
	codepoint32 = int32,
	pointer = int32,// 0xF0, // internal
//	node = int32, // NEEDS to be handled smartly, CAN't be differentiated from int32 now!
//	node = 0xA0,
	angle = 0xA0,//  angle object pointer/offset versus smarti vs anyref
	any = 0xA1,// Wildcard for function signatures, like haskell add :: a->a
//	unknown = any,
	array = 0xAA,
	charp = 0xC0, // vs
	stringp = 0xC0,// use charp?  pointer? enough?? no!??
//	value = 0xA1,// wrapped node Value, used as parameter? extract and remove! / ignore
	todoe = 0xFE, // todo
//	error_ = 0xE0, why propagate?
//	pointer = 0xF0,
//	externalPointer = 0xFE,
	ignore = 0xAF, // truely internal, should not be exposed! e.g. Arg
//	smarti32 = 0xF3,// see smartType
//	smarti64 = 0xF6,
};

Valtype mapTypeToWasm(Node &n);

chars typeName(Valtype t, bool fail= true);


// https://webassembly.github.io/spec/core/binary/types.html#binary-blocktype
//enum Blocktype { // same as Valuetype
//	void_block = 0x40
//};

// https://pengowray.github.io/wasm-ops/ <<< table
// https://webassembly.github.io/spec/core/binary/instrunctions.html <<< list (chrome)
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

	// EXTENSIONS:
	try_ = 0x06, // since Chrome 95
	catch_ = 0x07,
	throw_ = 0x08,
	rethrow_ = 0x09,
	br_on_exn_ = 0x0A, // branch on exception
	delegate_ = 0x18,

	end_block = 0x0b, //11
	br = 0x0c,
	br_if = 0x0d,
	return_block = 0x0f,
	function = 0x10,
	call_ = 0x10,
	call_indirect = 0x11,

	// EXTENSIONS:
	return_call = 0x12,  // the tail-call version of call ≠ return_block
	return_call_indirect = 0x13, // the tail-call version of call_indirect
	call_ref = 0x14, // [ts1 (ref $t)] -> [ts2] iff $t = [ts1] -> [ts2]
	return_call_ref = 0x15,
	func_bind = 0x16,// (type $t) 	$t : u32
	let_local = 0x17, // 	let <bt> <locals> 	bt : blocktype, locals : (as in functions)

	drop = 0x1a, // pop stack
	select_if = 0x1B, // c?a:b ternary (condition 3rd arg!) todo: use!
	select_t = 0x1C, // extension … ?

	local_get = 0x20,
	get_local = 0x20,// get to stack

	local_set = 0x21,
	local_tee = 0x22,
	// aliases:
	set_local = 0x21,// set and pop
	tee_local = 0x22,// set and leave on stack

	global_get = 0x23,
	global_set = 0x24,

	//	 Anyref/externref≠funcref tables, Table.get and Table.set (for Anyref only).
	//	Support for making Anyrefs from Funcrefs is out of scope
	table_get = 0x25,
	table_set = 0x26,

	i8_load = 0x2d, //== 𝟶𝚡𝟸𝙳, i32.load8_u
	i16_load = 0x2f, //== 𝟶𝚡𝟸𝙳, i32.load8_u
	i32_load = 0x28,// load word from i32 address
	f32_load = 0x2A,
	i32_store = 0x36,// store word at i32 address
	f32_store = 0x38,
	// todo : peek 65536 as float directly via opcode
	i64_load = 0x29, // memory.peek memory.get memory.read
	i64_store = 0x37, // memory.poke memory.set memory.write
	i32_store_8 = 0x3A,
	i32_store_16 = 0x3B,
	i8_store = 0x3A,
	i16_store = 0x3B,

//	i32_store_byte = 0x3a,// store byte at i32 address
	i32_auto = (byte) 0x41,
	i32_const = 0x41,
	i64_auto = 0x42,
	i64_const = 0x42,
	f32_auto = 0x43,

	f32_const = 0x43,
	f64_const = 0x44,

	i32_eqz = 0x45, // use for not!
	f32_eqz = 0x45, // HACK: no such thing!

//	negate = 0x45,
//	not_truty = 0x45,
	i32_eq = 0x46,
	i32_ne = 0x47,
	i32_lt = 0x48,
	i32_gt = 0x4A,
	i32_le = 0x4C,
	i32_ge = 0x4E,

	i64_eqz = 0x50,
	f64_eqz = 0x50, // HACK: no such thing!

	i64_eq = 0x51,
	i64_ne = 0x52,
	i64_lt_s = 0x53,
	i64_lt_u = 0x54,
	i64_gt_s = 0x55,
	i64_gt_u = 0x56,
	i64_le_s = 0x57,
	i64_le_u = 0x58,
	i64_ge_s = 0x59,
	i64_ge_u = 0x5a,

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
	i32_rem_u = 0x70,

	i32_and = 0x71,
	i32_or = 0x72,
	i32_xor = 0x73,
	i32_shl = 0x74,
	i32_shr_s = 0x75,
	i32_shr_u = 0x76,
	i32_rotl = 0x77,
	i32_rotr = 0x78,

	//	⚠ warning: funny UTF characters ahead! todo: replace c => c etc?
	i64_clz = 0x79,
	i64_ctz = 0x7A,
	i64_popcnt = 0x7B,
	i64_add = 0x7C,
	i64_sub = 0x7D,
	i64_mul = 0x7E,
	i64_di𝗏_s = 0x7F,
	i64_di𝗏_u = 0x80,
	i64_rem_s = 0x81,
	i64_rem_u = 0x82,
	i64_and = 0x83,
	i64_or = 0x84,
	i64_xor = 0x85,
	i64_s𝗁l = 0x86,
	i64_s𝗁r_s = 0x87,
	i64_s𝗁r_u = 0x88,
	i64_rotl = 0x89,
	i64_rotr = 0x8A,

	// beginning of float opcodes
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
	f32_mul = 0x94,// f32.mul
	f32_div = 0x95,

	f64_abs = 0x99,
	f64_neg = 0x9a,
	f64_ceil = 0x9b,
	f64_floor = 0x9c,
	f64_trunc = 0x9d,
	f64_nearest = 0x9e,
	f64_sqrt = 0x9f,
	f64_add = 0xA0,
	f64_sub = 0xa1,
	f64_mul = 0xa2,
	f64_div = 0xa3,
	f64_min = 0xa4,
	f64_max = 0xa5,
	f64_copysign = 0xa6,


	i32_wrap_i64 = 0xA7,
	i32_trunc_f32_s = 0xA8, // cast/convert ( ≠ bitwise reinterpret )
	i32_trunc_f32_u = 0xA9, // always use sign!
	i32_trunc_f64_s = 0xAA,
	i32_trunc_f64_u = 0xAB,
	i64_extend_i32_s = 0xAC,
	i64_extend_i32_u = 0xAD,
	i64_trunc_f32_s = 0xAE,
	i64_trunc_f32_u = 0xAF,
	i64_trunc_f64_s = 0xB0, // cast convert ( ≠ bitwise reinterpret )
	i64_trunc_f64_u = 0xB1,
	f32_convert_i32_s = 0xB2,
	f32_convert_i32_u = 0xB3,
	f32_convert_i64_s = 0xB4,
	f32_convert_i64_u = 0xB5,
	f32_demote_f64 = 0xB6,
	f64_convert_i32_s = 0xB7,
	f64_convert_i32_u = 0xB8,
	f64_convert_i64_s = 0xB9,
	f64_convert_i64_u = 0xBA,
	f64_promote_f32 = 0xBB,
	i32_reinterpret_f32 = 0xBC, // f32->i32 bit wise reinterpret != cast/trunc/convert
	i64_reinterpret_f64 = 0xBD, // use to hack smart pointers as main return: f64 has int range which is never hit
	f32_reinterpret_i32 = 0xBE,// i32->f32
	f64_reinterpret_i64 = 0xBF,
	f32_from_f64 = f32_demote_f64,
	f64_from_f32 = f64_promote_f32,
	f32_from_int32 = 0xB2,
	f32_cast_to_i32_s = 0xa8,// truncation ≠ proper rounding (f32_round = 0x90)!


	//	signExtensions
	i32_extend8_s = 0xC0,
	i32_extend16_s = 0xC1,
	i64_extend8_s = 0xC2,
	i64_extend16_s = 0xC3,
	i64_extend32_s = 0xC4,
//	i64_extend_i32_s = 0xAC, WHAT IS THE DIFFERENCE?
// i64.extend_s/i32 sign-extends an i32 value to i64, whereas
// i64.extend32_s sign-extends an i64 value to i64

//referenceTypes
//https://github.com/WebAssembly/function-references/blob/master/proposals/function-references/Overview.md#local-bindings
	ref_null = 0xD0,
	ref_is_null = 0xD1,
	ref_func = 0xD2, // 0xd2 varuint32 0x0b Returns a funcref reference to function $funcidx
//	ref_null=-0x14,// 	(ref null ht) 	$t : heaptype  -0x10:func -0x11:extern i >= 0 :	i
//	ref_typed=-0x15,// 	(ref ht) 	$t : heaptype
	ref_as_non_null = 0xd3,// 	ref.as_non_null
	br_on_null = 0xd4, //	br_on_null $l 	$l : u32
	br_on_non_null = 0xd6,// 	br_on_non_null $l 	$l : u32

// saturated truncation  saturatedFloatToInt
//i32_trunc_sat_f32_s=0xFC00,
//i32_trunc_sat_f32_u=0xFC01,
//i32_trunc_sat_f64_s=0xFC02,
//i32_trunc_sat_f64_u=0xFC03,
//i64_trunc_sat_f32_s=0xFC04,
//i64_trunc_sat_f32_u=0xFC05,
//i64_trunc_sat_f64_s=0xFC06,
//i64_trunc_sat_f64_u=0xFC07,

// bulkMemory
	memory_init = 0xFC08,
	data_drop = 0xFC09,
	memory_copy = 0xFC0a,
	memory_fill = 0xFC0b,
	table_init = 0xFC0c,
	elem_drop = 0xFC0d,
	table_copy = 0xFC0e,
	table_grow = 0xFC0f,
	table_size = 0xFC10,
	table_fill = 0xFC11,

// SIMD
	simd____ = 0xFD,
};

// https://webassembly.github.io/spec/core/binary/modules.html#sections
enum Sections {
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
	data_section = 11,
	// extensions:
	datacount = 12,
	tag_section = 13
};

enum nameSubSectionTypes {
	module_name = 0,
	function_names = 1,
	local_names = 2,
	global_names = 7,
};

typedef enum constancy {
	defaulty,
	mutabley,
	constanty,
	finaly,
} Constancy;

//localContextTypes is messy
/*
class Variable {
//	String* name = "";// could be reused by multiple, but useful to debug
	short position = 0;// in context / in global
	Valtype kind;
	Node *type;
	Constancy constancy;
	bool global;
	Node *context;
//	List<Node*> modifiers; // public static const … should translate into fields but keep for extra etc?
	Node *descriptor;// ?
};
*/

enum ABI {
//	unknown,
	erased = 0, // unknown type info erased
	native = 0, // wasm/c
	cpp, // _Z5abs_ff demangle
	wasp,
	wasp_smart_pointers, // compatible with lacking multi-value in wasm engine
	meta, // types specified in custom meta section
	meta_names, // types specified via naming convention square__int_as_int, square__float_as_float
};

class Signature {
public:
	ABI abi = wasp_smart_pointers;//erased;
	String function = "";// could be reused by multiple, but useful to debug
// todo: add true Wasp Type Signature to wasm Valtype Signature
	Map<int, Valtype> types;
	List<Valtype> return_types;// should be 2 in standard Wasp ABI unless emitting pure primitive functions or arrays/structs?
	Node return_type{};
//	Valtype return_type = voids;
	bool is_import = false; // not serialized in functype section, but in import section wt
	bool is_runtime = false;
	bool is_builtin = false;// hard coded functions, tests only? todo remove
	bool is_handled = false; // already emitted (e.g. as runtime)
	bool is_used = false;// called
	bool emit = false;// only those types/functions that are declared (export) or used in call
#ifdef DEBUG
	String debug_name;
#endif


	// these explicit constructions are needed when using types return_types as reference!
	Signature() : return_types(*new List<Valtype>), types(*new Map<int, Valtype>) {}
//	Signature(const Signature& old) : return_types(old.return_types), types(old.types) {}
//	Signature(List<Valtype> &returnTypes, Map<int, Valtype> &types) : return_types(returnTypes), types(types) {}
//	Signature &operator=(const Signature old){
//		if (this == &old) return *this;
//		return_types=old.return_types;
//		types=old.types;
//		// really neccessary??
//		is_import  = old.is_import ;
//		is_runtime = old.is_runtime;
//		is_builtin = old.is_builtin;
//		is_handled = old.is_handled;
//		is_used  = old.is_used ;
//		emit  = old.emit ;
//		return *this;
//	}

//		Signature &operator=(const Signature &old){
//		if (this == &old) return *this;
//		return_types=old.return_types;
//		types=old.types;
//		// really neccessary??
//		is_import  = old.is_import ;
//		is_runtime = old.is_runtime;
//		is_builtin = old.is_builtin;
//		is_handled = old.is_handled;
//		is_used  = old.is_used ;
//		emit  = old.emit ;
//		return *this;
//	}

	int size() {
		return types.size();
	}

	Signature &handled() {
		is_handled = true;
		return *this;
	}

	Signature &import() {
		is_import = true;
		return *this;
	}

	Signature &builtin() {
		is_builtin = true;
		return *this;
	}

	Signature &runtime() {
		is_runtime = true;
		return *this;
	}

	Signature &add(Valtype t) {
#ifdef DEBUG
		debug_name += typeName(t);
		debug_name += " ";
#endif
		types.insert_or_assign(types.size(), t);
		return *this;
	}

	Signature &add(Node type) {
		Valtype t = mapTypeToWasm(type);
#ifdef DEBUG
		debug_name += typeName(t);
		debug_name += " ";
#endif
		types.insert_or_assign(types.size(), t);
		return *this;
	}


	Signature &returns(Node type) {
		return_type = type;
		if (type.kind != nils)// todo? type.kind!=undefined … ?
		{
			Valtype valtype = mapTypeToWasm(type);
			return_types.add(valtype);
		}// value, should map to int32 unless unboxing long, float
		return *this;
	}


	Signature &returns(Valtype valtype) {
		if (valtype != voids and valtype != none) {
#ifdef DEBUG
			debug_name += ": ";
			debug_name += typeName(valtype);
#endif
			return_types.add(valtype);
		}
		return *this;
	}

	String format() {
		String f;
//#if RELEASE
//#else
//		f += this->function;
//		f += "(";
//		for (auto type:this->types) {
//			f += typeName(this->types[type]);
//			f += ",";
//		}
//		f += ")";
//		f += typeName(this->return_type);
//#endif
		return f;
	}
};

#ifndef RUNTIME_ONLY

//String sectionName(::Section section);
String sectionName(Sections section);

Code createSection(Sections sectionType, Code data);

//Code createSection(::Section sectionType, Code data);

Code &unsignedLEB128(long n);

Code &signedLEB128(long value);

#endif

#endif //WASP_CODE_H
