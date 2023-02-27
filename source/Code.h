#pragma once
//
// Created by me on 08.12.20.
//

#include "String.h"
#include "Map.h"
#include "Node.h"
#include "List.h"

//#ifndef PURE_WASM
#include <cstdio>
// header OK?
//#endif
//size_t strlen(const char *__s);

#ifndef WASP_CODE_H
#define WASP_CODE_H
typedef unsigned char byte;
typedef const char *chars;
typedef byte *bytes;


class Module;

// in Util.h but we cant import
bytes concat(bytes a, bytes b, int len_a, int len_b);

bytes concat(bytes a, byte b, int len);

bytes concat(char section, bytes a, int len_a);

class Code;

// Different environments offer different run_wasm implementations:
// wasm JIT runtimes: wasm3, wasm-micro-runtime, wabt, V8 via webview:
// wasmer via console, node/browser via import, webview  …
// extern "C" for webview / other host
//#include <future>
//std::future<int64> run_wasm(bytes wasm_bytes, int len); // <thread> is not supported on this single threaded system
#if MY_WASM
extern "C" int run_wasm(bytes wasm_bytes, int len);
#else
extern "C" int64 run_wasm(bytes wasm_bytes, int len);
#endif
extern "C" int64 run_wasm_file(chars wasm_path);

extern bytes magicModuleHeader;
extern bytes moduleVersion;

Code encodeVector(const Code &data);

Code &unsignedLEB128(int64 n);

Code &signedLEB128(int64 value);

class Code {
public:

    int header = array_header_32;// todo: code_header_32 if extra fields are needed beyond standard
    int kind = byte_i8;
    int length = 0;
    bytes data = 0;
    int64 start = 0;// internal reader pointer
    bool encoded = false;// first byte = size of vector
    bool shared = true;// can't free data until all views are destroyed OR because this is a view on other's data!!
    bool needs_relocate = true; // unless specified
    String name;// function or file
    Code() {}

    Code(bytes a, int len, bool needs_copy = true) {
        data = a;
        length = len;
        if (needs_copy) {
            shared = false; // free later
            data = static_cast<bytes>(calloc(length + 1, sizeof(bytes)));
            memcpy(data, a, length);
        }
    }

    virtual ~Code() {
//		if (!shared and data and length)// mechanism doesn't work
//			free(data);// pointer being freed was not allocated
        data = 0;
    }

    Code(char *datas, int size, bool needs_copy = true) : Code((bytes) datas, size, needs_copy) {}
//	Code(byte *data0, int from, int pos, bool copy=false) : Code(data0,from,copy) {}

    Code(byte byte) {
        data = (bytes) alloc(1, 1);
        data[0] = byte;
        length = 1;
    }

    Code(short opcode) {
        if (opcode < 0x100)
            add(opcode);
        else {
            data = (bytes) alloc(2, 1);
            data[0] = opcode % 0x100;
            data[1] = opcode / 0x100;
            length = 2;
        }
    }


    Code(int nr, bool LEB = true) {// ambiguous: byte (un)signedLEB128 or int32 !!
        if (LEB) {
//			push(nr, false, LEB);
            push(nr, nr < 0, LEB);
        } else {
            data = new byte[4];
            shared = false;
            *(int *) data = nr;
            *(int *) data = nr;
            length = 4;
        }
    }

    Code(int64 nr, bool LEB = true) {
        if (LEB) {
            push(nr, false, LEB);
        } else {
            data = new byte[8];
            shared = false;
            *(int64 *) data = nr;
            length = 8;
        }
    }

    Code(chars string, bool size_header = true, bool null_terminated = false) {
        short len = strlen(string);
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

    byte *begin() {
        return data;
    }

    byte *end() {
        return data + length;
    }

    Code &operator++() {
        if (length == 0)return *this;
//		start++;
        data++;
        length--;
        return *this;
    }

    Code &operator++(int postfix) {
        if (postfix == 0)postfix = 1;
        if (length < postfix)return *this;
        data += postfix;
        length -= postfix;
        return *this;
    }

    Code &operator+(Code more) { // todo use non-modifying version if …
        return this->push(more);
    }

    Code &operator+=(Code more) {
        return this->push(more);
    }

    Code &operator+(char more) {
        return this->push(more);
    }

    Code &operator+(byte more) {
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
        // todo: append to last byte if possible
        data = concat(data, opcode, length);
        length++;
        return *this;
    }

    Code &nop() {
        return add(0x01);
    }

    Code &addOpcode(unsigned short opcode) {
        // todo: append to last byte if possible
        if (opcode < 0x100)
            return add((byte) opcode);
        data = concat(data, (byte) (opcode / 0x100), length);
//        data = concat(data, 0xFB, length);
        data = concat(data, (byte) (opcode % 0x100), length + 1);
        length += 2;
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
    Code &push(int64 nr, bool sign = true, bool LEB = true) {
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
            *(int64 *) data = nr;
            length = 8;
        }
        return *this;
    }

    Code &push(int nr, bool sign = true, bool LEB = true) {
        return push((int64) nr, sign, LEB);
    }

    Code &push(unsigned int nr, bool sign = true, bool LEB = true) {
        return push((int64) nr, sign, LEB);
    }

    Code &push(bytes more, int len) {
        shared = false;
        data = concat(data, more, length, len);
        length = length + len;
        return *this;
    }

    Code &clone(bool deep = true) {
        Code *copy = new Code();
//		*copy = *this;// DOESNT!
        copy->data = data;
        copy->length = length;
        if (deep) {
            copy->shared = false;
            copy->data = static_cast<bytes>(malloc(length));
            memcpy(copy->data, data, length);
        } else shared = true;
        return *copy;
    }

    void debug() {
        String s;
        for (int i = length - 1000; i < length; i++) {
//            put_chars(hex(data[i]));
            s += hex(data[i], 1) + ", ";
        }
        print(s);
//        save();
    }

    Code &save(chars file_name = "test.wasm") {
        if (name.empty())name = file_name;
#ifndef WASM
        if (!String(file_name).endsWith(".wasm"))
            file_name = (char *) concat(file_name, ".wasm");
        FILE *file = fopen(file_name, "wb");
        fwrite(data, length, 1, file);
        fclose(file);
#endif
        return *this;
    }

    smart_pointer_64 run() {
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
        shared = true;// can't free until this is destroyed:
        return Code(data + start0, length - start0);
//		return Code(data, start, length);
    }

    Code addInt(int i, bool leb = true) {
//  ⚠️ as op PARAM/VALUE , use addConst32 for STACK VALUE (prefixed with i32_const)
//  ⚠️ leb = true in most cases in Code!
        push((int64) i, true, leb);
        return *this;
    }

    Code addConst32(unsigned int i) {
        add(0x41 /*i32_const*/);
        if (i > 0x80000000)push(-(int64) i, false, true);// stupid sign bit hack
        else push(i, true, true);
//		else push(i, false, true);
        return *this;
    }

    Code &addConst64(int64 i) {
//		if (i < 0x100000000 and i > -0x100000000)
//			add(0x41 /*i32_const*/);
//		else
        add(0x42 /* i64_const */);
        push(i);
        return *this;
    }

};


int stackItemSize(Valtype valtype, bool throws = true);

int stackItemSize(Node &clazz, bool throws = true);

int stackItemSize(Type type, bool throws = true);


// https://webassembly.github.io/spec/core/binary/types.html#binary-blocktype
//enum Blocktype { // same as Valuetype
//	void_block = 0x40
//};

// https://pengowray.github.io/wasm-ops/ <<< table
// https://webassembly.github.io/spec/core/binary/instructions.html <<< list
// USE wasm-objdump -d  to see function disassembled:
enum Opcodes {
//	start = 0x00,
    start_function = 0x00,
//	unreachable = 0x00,
    nop_ = 0x01, // useful for relocation padding call 1 -> call 10000000
    block = 0x02,
    loop = 0x03,
    if_i = 0x04,// precede by i32 result, follow by i32_type (7f)
    else_ = 0x05,

    // EXTENSIONS:
    try_ = 0x06, // since Chrome 95
    catch_ = 0x07,
    throw_ = 0x08,
    rethrow_ = 0x09,
    br_on_exn_ = 0x0A, // branch on exception
    delegate_ = 0x18,

    end_block = 0x0b, //11
    br_branch = 0x0c,// ususally called 'br' but it interfered with c++ break autocomplete
    br_if = 0x0d,
    br_table = 0x0e,
    return_block = 0x0f,
    function_call = 0x10,
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

    i32_store_8 = 0x3A, // i32.store8
    i8_store = 0x3A,

    i32_store_16 = 0x3B,
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
    i64_div_s = 0x7F,
    i64_div_u = 0x80,
    i64_rem_s = 0x81,
    i64_rem_u = 0x82,
    i64_and = 0x83,
    i64_or = 0x84,
    i64_xor = 0x85,
    i64_shl = 0x86,
    i64_shr_s = 0x87,
    i64_shr_u = 0x88,
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
    // proposed for vector pipelin?
    f32_min = 0x96,
    f32_max = 0x97,
    f32_copysign = 0x98,

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

// DON'T USE Prefixes directly, use the corresponding functions with 'short' opcodes e.g. 0xfb03
//    struct_prefix = 0xfb, // struct.get, struct.set, struct.new, struct.narrow, struct.widen see ~/dev/wasm/gc_structs/module.wat
//    string_prefix = 0xfb, // string_new, string_length, string_get, string_set, string_copy, string_fill, string_grow, string_size, string_drop

    math_prefix_s = 0xfc,

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

    float_rounding___ = 0xFC, // proposed prefix https://github.com/WebAssembly/design/issues/1456
// SIMD
    simd____ = 0xFD,
};


// struct_prefix 0xfb …
enum struct_opcodes { // on Valtype wasm_struct = 0x6b,
    struct_get = (short) 0xfb03, // fb 03 00 00  struct.get $type(0) $field(0) (stack: instance-ref)
    struct_set = 0xfb06, // fb 06 00 00  struct.set $type(0) $mut_field(0) (stack: instance-ref value)
    struct_new = (short) 0xfb07, // fb 07 00  struct.new $type(0)  (stack: params)

    structGet = 0xfb03, // struct.get
    structGetS = 0xfb04, // struct.get_s
    structGetU = 0xfb05, // struct.get_u
    structSet = 0xfb06, // struct.set
    structNew = 0xfb07, // struct.new
    structNewDefault = 0xfb08, // struct.new_default
    //    struct_narrow = 0x0?,
    //    struct_widen = 0x0?,
};

enum array_opcodes { // on Valtype wasm_array = 0x…,
    arrayGet = 0xfb13, // array.get
    arrayGetS = 0xfb14, // array.get_s
    arrayGetU = 0xfb15, // array.get_u
    arraySet = 0xfb16, // array.set
    arrayLenDeprecated = 0xfb17, // array.len
    arrayCopy = 0xfb18, // array.copy /* not standardized - V8 experimental */
    arrayLen = 0xfb19, // array.len
    arrayNewFixed = 0xfb1a, // array.new_fixed  array.new_static in V8!
    arrayInitStatic = arrayNewFixed, // ^^ array.init_static in V8!
    arrayNew = 0xfb1b, // array.new
    arrayNewDefault = 0xfb1c, // array.new_default
    arrayNewData = 0xfb1d, // array.new_data
    arrayNewElem = 0xfb1f, // array.new_elem
};

enum reference_opcodes { // on Valtype wasm_i31 = 0x…,
    i31New = 0xfb20, // i31.new
    i31GetS = 0xfb21, // i31.get_s
    i31GetU = 0xfb22, // i31.get_u
    refTest = 0xfb40, // ref.test
    refTestDeprecated = 0xfb44, // ref.test
    refCast = 0xfb45, // ref.cast
    brOnCast = 0xfb46, // br_on_cast
    brOnCastFail = 0xfb47, // br_on_cast_fail
    refCastNop = 0xfb4c, // ref.cast_nop
    refIsData = 0xfb51, // ref.is_data
    refIsI31 = 0xfb52, // ref.is_i31
    refIsArray = 0xfb53, // ref.is_array
    refAsData = 0xfb59, // ref.as_data  !!!
    refAsI31 = 0xfb5a, // ref.as_i31
    refAsArray = 0xfb5b, // ref.as_array !!!
    brOnData = 0xfb61, // br_on_data
    brOnI31 = 0xfb62, // br_on_i31
    brOnArray = 0xfb66, // br_on_array
    brOnNonData = 0xfb64, // br_on_non_data
    brOnNonI31 = 0xfb65, // br_on_non_i31
    brOnNonArray = 0xfb67, // br_on_non_array
    externInternalize = 0xfb70, // extern.internalize
    externExternalize = 0xfb71, // extern.externalize
};

// string_prefix 0xfb (251) …
// https://github.com/WebAssembly/stringref
// https://github.com/WebAssembly/stringref/blob/master/proposals/stringref/Overview.md
// https://github.com/WebAssembly/wabt/issues/2144 wat2wasm --enable-all --enable-stringref << IN PROGRESS!
// idx point to section stringrefs ::= section_14(0x00 vec(vec(u8)))
enum string_ops { // on Valtype stringref = 0x64
    string_new_utf8 = 0xfb80, // $mem:u32  BROKEN!? use string_new_wtf8
    string_new_wtf16 = 0xfb81, // $mem:u32   (Wobbly Transformation Format, 8-bit) unpaired surrogate halves (U+D800 through U+DFFF) are allowed.
    string_const = 0xfb82, // $idx:u32 (idx points to section stringrefs) <<<

    // length
    string_measure_utf8 = 0xfb83, //
    string_measure_wtf8 = 0xfb84, //
    string_measure_wtf16 = 0xfb85, //

    // string to data
    string_encode_utf8 = 0xfb86, // $mem:u32
    string_encode_wtf16 = 0xfb87, // $mem:u32

    string_concat = 0xfb88, //
    string_eq = 0xfb89, //
    string_is_usv_sequence = 0xfb8a, //

    // data to string
    string_new_lossy_utf8 = 0xfb8b, // $mem:u32
    string_new_wtf8 = 0xfb8c, // $mem:u32

    string_encode_lossy_utf8 = 0xfb8d, // $mem:u32
    string_encode_wtf8 = 0xfb8e, // $mem:u32
    string_as_wtf8 = 0xfb90, // ?

    stringview_wtf8_advance = 0xfb91, //
    stringview_wtf8_encode_utf8 = 0xfb92, // $mem:u32
    stringview_wtf8_slice = 0xfb93, //
    stringview_wtf8_encode_lossy_utf8 = 0xfb94, // $mem:u32
    stringview_wtf8_encode_wtf8 = 0xfb95, // $mem:u32
    string_as_wtf16 = 0xfb98, //
    stringview_wtf16_length = 0xfb99, //
    stringview_wtf16_get_codeunit = 0xfb9a, //
    stringview_wtf16_encode = 0xfb9b, // $mem:u32
    stringview_wtf16_slice = 0xfb9c, //
    string_as_iter = 0xfba0, //
    stringview_iter_next = 0xfba1, //
    stringview_iter_advance = 0xfba2, //
    stringview_iter_rewind = 0xfba3, //
    stringview_iter_slice = 0xfba4, //

    // chrome v8 :
    string_compare = 0xfba8, // (str1: string, str2: string) -> int32  -1, 0 or 1 if the compared strings are lessThan, equal or greaterThan
    string_from_code_point = 0xfba9, // (cp: int32) -> string
    string_hash = 0xfbaa, // (str: string) -> int32

    // array to stringref
    string_new_utf8_array = 0xfbb0, //           [gc]
    string_new_wtf16_array = 0xfbb1, //           [gc]
    string_encode_utf8_array = 0xfbb2, //           [gc]
    string_encode_wtf16_array = 0xfbb3, //           [gc]
    string_new_lossy_utf8_array = 0xfbb4, //           [gc]
    string_new_wtf8_array = 0xfbb5, //           [gc]
    string_encode_lossy_utf8_array = 0xfbb6, //           [gc]
    string_encode_wtf8_array = 0xfbb7, //           [gc]
};


//field must be mutable
//(type $array (array (mut i32)))

//	(func $create (param $x f64) (param $y f64) (result (ref $Point))
//		(struct.new $Point (local.get $x) (local.get $y))
//	)



// don't forget the PREFIX before each vector op:
// combined with vector prefix Valtypes, e.g. vec_i64 = 0x77 => 0x7700 == vec_i64.length
// https://github.com/WebAssembly/flexible-vectors/blob/main/proposals/flexible-vectors/BinaryFlexibleVectors.md
enum VectorOpcodes { // Immediate operands in comments:
    vector_length_op = 0x00,
    vector_splat_op = 0x10,
    extract_lane_imm_u = 0x11, // i:ImmLaneIdx16 for i8, ImmLaneIdx4 for i32 etc ( product must be 64!)
    extract_lane_imm_s = 0x12, // i:ImmLaneIdx… ^^
    replace_lane_imm = 0x13,   // i:ImmLaneIdx… ^^
    extract_lane_u = 0x14,
    extract_lane_s = 0x15,
    replace_lane = 0x16,
    extract_lane_mod_u = 0x17,
    extract_lane_mod_s = 0x18,
    replace_lane_mod = 0x19,

    vector_lshl = 0x20, // (lane) shift left    VERSUS bitwise  	0x50
    vector_lshr = 0x21, // (lane) shift right

    // Ints vec.i64.add  0x77.0x30 DIFFERENT to vec.f32.add 	0x76.0x94  GRRR!
    vector_add = 0x30, // ok overlap with i32.add … would be nonsensical
    vector_sub = 0x31,
    vector_mul = 0x32,
//    vector_div = 0x3x, "intentionally missing"  because not in simd128 either
    vector_neg = 0x33,
    vector_min_u = 0x34,
    vector_min_s = 0x35,
    vector_max_u = 0x36,
    vector_max_s = 0x37,
    vector_avgr_u = 0x38, // average <3 !
    vector_abs = 0x39,

    vector_shl = 0x50, //  bitwise!     VERSUS (lane) shift left    lshl = 0x20,
    vector_shr = 0x51, //  bitwise!     VERSUS (lane) shift right   lshr = 0x21,
    vector_shr_s = 0x52,

    vector_and = 0x53,
    vector_or = 0x54,
    vector_xor = 0x55,
    vector_not = 0x56,
    vector_andnot = 0x57,
    vector_bitselect = 0x58, // slice from to how?  Bits 3...7 of 01100101: 25 (_11001__)

    vector_any_true = 0x60,
    vector_all_true = 0x61,

    vector_eq = 0x70,
    vector_ne = 0x71,
    vector_lt_u = 0x72,
    vector_lt_s = 0x73,
    vector_lt = 0x74,
    vector_le_u = 0x75,
    vector_le_s = 0x76,
    vector_le = 0x77,

    vector_gt_u = 0x78,
    vector_gt_s = 0x79,
    vector_gt = 0x7a,
    vector_ge_u = 0x7b,
    vector_ge_s = 0x7c,
    vector_ge = 0x7d,

    vector_load = 0x80,
    // … space!
    vector_store = 0x87,

// Floating-point :
    vector_neg_f = 0x90,
    vector_abs_f = 0x91,
    vector_pmin = 0x92,
    vector_pmax = 0x93,
    vector_add_f = 0x94,
    vector_sub_f = 0x95,
    vector_div_f = 0x96,
    vector_mul_f = 0x97,
    vector_sqrt_f = 0x98,

    vector_convert_s = 0xA0,
    vector_narrow_s = 0xA1,
    vector_narrow_u = 0xA2,
    vector_widen_low_u = 0xA3,
    vector_widen_low_s = 0xA4,
    vector_widen_high_u = 0xA5,
    vector_widen_high_s = 0xA6,
};


typedef VectorOpcodes vecop;

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
    element_section = 9, // Element segments initialize a subrange of a table from a static vector of elements.
    code_section = 10, // 0x0a
    data_section = 11,
    // extensions:
    datacount = 12,
    tag_section = 13,
    string_section = 14, // wasm stringref table
};

enum nameSubSectionTypes {
    module_name = 0,
    function_names = 1,
    local_names = 2,
    type_names = 4, // GC struct
    global_names = 7,
    data_names = 9,
    field_names = 10, // GC struct.field
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
    native = 0, // wasm/c also wasi?
    cpp, // _Z5abs_ff demangle, ill defined type arguments
    wasp,// multi-value return tuples (value, type), node header tuples (head, type, meta, size, children) see ABI.h
    wasp_smart_pointers, // compatible with lacking multi-value in wasm engine
    canonical, // type string = (pointer,length) …
    wit = canonical, // lookup type and data schemes in wit 'header' files
//	meta, // types specified in custom meta section
//	meta_names, // function types specified via naming convention square__int_as_int, square__float_as_float
};

class Function;

// todo we have a problem:  is_handled applies to a specific function, not it's Signature potentially shared with OTHER functions!
// wasm function type signatures plus some…
class Signature {
public:
    int type_index = -1;// in type section ≠ function index!!
    ABI abi = wasp_smart_pointers;//erased;
// todo: add true Wasp Type Signature to wasm Valtype Signature
    List<Function *> functions;// using this Signature; debug only?

//    List<Argument> params; :
    List<String> parameter_names;// per function 😕
    List<Type> parameter_types;// implicit index

    List<Type> return_types;// should be 2 in standard Wasp ABI unless emitting pure primitive functions or arrays/structs?
//	Type return_type{};// use return_types.last(default)
    Valtype wasm_return_type = voids;// until checked debug only!
    bool is_handled = false;
    // these explicit constructions are needed when using types return_types as reference!
//	Signature() : return_types(*new List<Valtype>), types(*new Map<int, Valtype>) {}
//	Signature() : return_types(*new List<Type>), types(*new Map<int, Type>) {}

#if DEBUG and not WASM
    String debug_name;// todo can .lldbinit call format() !?!
#endif

    bool operator==(const Signature &other) const {
        for (int i = 0; i < parameter_types.size(); ++i) {
            if (parameter_types[i] != other.parameter_types[i])
                return false;
        }
        if (return_types != other.return_types)
            breakpoint_helper
//            return false;
        for (int i = 0; i < return_types.size(); ++i) {
            if (return_types[i] != other.return_types[i])
                return false;
        }
        return type_index == other.type_index and \
        wasm_return_type == other.wasm_return_type;
    }

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

    int size() const {
        return parameter_types.size();
    }


    Signature &add(Type t, String name = "") {
#if DEBUG and not WASM
        debug_name += typeName(t);
        debug_name += " ";
#endif
        parameter_types.add(t);
        parameter_names.add(name);
        return *this;
    }

    Signature &add(Valtype t, String name = "") {
        return add((Type) t, name);
    }

    Signature &add(Node type, String name = "") {
        return add((Type) mapTypeToPrimitive(type), name);
    }

//
//		Signature &returns(Node& type) {
//		return_type = Type(type);
//		wasm_return_type = mapTypeToWasm(type);
//		if (type.kind != nils)// todo? type.kind!=undefined … ?
//		{
//			Valtype valtype = mapTypeToWasm(type);
//			return_types.add(valtype);
//		}// value, should map to int32 unless unboxing int64, float
//		return *this;
//	}

    Signature &returns(Type type) {
//		return_type = type;
        if (type.kind != nils and type.kind != undefined and type.kind != unknown) {
            if (multi_value or return_types.size() == 0)
                return_types.add(type);
            else
                return_types[0] = type;
#if DEBUG and not WASM
            debug_name += ": ";
            debug_name += typeName(type);
#endif
        }
        wasm_return_type = mapTypeToWasm(type);
        return *this;
    }

    Signature &returns(Valtype valtype) {
        wasm_return_type = valtype;
        if (valtype != voids and valtype != none) {
            if (valtype == float64 or valtype == float32 or valtype == int32 or valtype == i64)
                return_types.add(valtype);
            else
                error("UNKNOWN Valtype mapping "s + typeName(valtype));
#if DEBUG and not WASM
            debug_name += ": ";
            debug_name += typeName(valtype);
#endif
        }
//		return_type = Type(valtype);// OVERLAP HAS TO BE OK!?!
        return *this;
    }

    static String format() {
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

    void merge(Signature &s) { // ok don't duplicate, just fill empties
        if (type_index < 0)type_index = s.type_index;
//		return_type = s.return_type;
        if (return_types.empty()) {
            wasm_return_type = s.wasm_return_type;
            return_types = s.return_types;// todo copy construktor OK??
            if ((wasm_return_type == void_block or wasm_return_type == voids) and return_types.size_ > 0)
                wasm_return_type = mapTypeToWasm(return_types.last());
        }
        if (parameter_types.empty())
            parameter_types = s.parameter_types;
        // todo: fix debug name and parameter_names!
    }

    bool has(String string) {
        return parameter_names.contains(string);
    }

    Signature clone() {
        return *this;
    }

    explicit operator String() {
        return serialize();
    }

    String serialize() {
        String s;
//        s+=functions
        s += "(";
        for (int i = 0; i < parameter_types.size(); ++i) {
            if (i > 0)s += ",";
            s += " "s + typeName(parameter_types[i]);
            s += " "s + parameter_names[i];
        }
        s += ")";
        if (not return_types.empty()) {
            s += " ⇨";
            for (Type type: return_types)
                s += " "s + typeName(type);
        }
        return s;
    }
};


struct Global {
    int index = -1;// always set!
    String name;
    Type type = unknown;
    Node *value; // expression
    bool is_mutable = true;// default!
    bool is_import = false;
    bool is_export = true;
};

struct Local { // todo: use
//    bool is_global;
    bool is_param; // function arguments and locals share same index space, but are emitted differently
    int position; // also implicit in Function{ List<Local> locals;}
    String name;
//    Valtype type = unknown_type;
//    Primitive grrr
    Type type;// = unknown_type;
    Node *typeXX; // todo: REMOVE!
    Node *ref;// why still needed?
    int data_pointer = 0;// compile time handling of reference data, e.g. after emitData()
};

class Function {
public:
    int code_index = -1;// todo: split into code_index and call_index (== code_index + import_count) ???
    int call_index = -1;// code_index + module.function_import.size()
    String name;
    String export_name;
    Signature signature;
    Module *module = 0;
    Node *body = 0;
    Code *code = 0;

    bool is_import = false; // not serialized in functype section, but in import section wt
//    bool is_declared; // has fresh Code body to emit!
    bool is_declared = false;// only those types/functions that are declared (export) or used in call
    bool is_runtime = false;// old special imports to wasm.wasm
    bool is_handled = false; // already emitted (e.g. as runtime)
    bool is_builtin = false;// hard coded functions, tests only? todo remove
    bool is_used = false;// called imports / buildins
    bool is_polymorph = false;// IF polymorph, this 'Function' acts as abstract only, all REAL Functions are in variants
    List<Function> variants;// = 20;//={.capacity=20};// multi dispatch!

    //    Code* code; // todo: use
    Map<String, Local> locals;  // todo: use, instead of global locals!

    Function &handled() {
        is_handled = true;
        return *this;
    }

    Function &import() {
        is_import = true;
        return *this;
    }

    Function &builtin() {
        is_builtin = true;
        return *this;
    }

    Function &runtime() {
        is_runtime = true;
        is_declared = false;
        return *this;
    }

//    explicit
    operator String() {
        return name;
    }

    bool operator==(const Function &other) const {
        return name == other.name and signature == other.signature;
    }

};


#ifndef RUNTIME_ONLY

//String sectionName(::Section section);
String sectionName(Sections section);

Code createSection(Sections sectionType, const Code &data);

//Code createSection(::Section sectionType, Code data);

Code &unsignedLEB128(int64 n);

Code &signedLEB128(int64 value);

#endif

#endif //WASP_CODE_H

/*
0 0 1
128 80 2
16384 4000 3
2097152 200000 4
268435456 10000000 5
34359738368 800000000 6
4398046511104 40000000000 7
562949953421312 2000000000000 8
36028797018963968 80000000000000 9
 */
short lebByteSize(
        uint64 neu);// unsigned variants have delayed size increase by factor 2! ( 0x80 needs 2 bytes vs 0x40 signed!!)
short lebByteSize(unsigned int neu);

/*
0 0 1
64 40 2
8192 2000 3
1048576 100000 4
134217728 8000000 5
17179869184 400000000 6
2199023255552 20000000000 7
281474976710656 1000000000000 8
36028797018963968 80000000000000 9
-65 ffffffffffffffbf 2
-8320 ffffffffffffdf80 3
-1064960 ffffffffffefc000 4
-136314880 fffffffff7e00000 5
-17448304640 fffffffbf0000000 6
-2233382993920 fffffdf800000000 7
-285873023221760 fffefc0000000000 8
-36591746972385280 ff7e000000000000 9
 */
short lebByteSize(int64 neu);

#if MY_WASM
extern "C" char* js_demangle(char* name);
Function getWaspFunction(String name);
#endif

void print(Signature &signature);
