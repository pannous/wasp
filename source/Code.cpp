//
// Created by me on 08.12.20.
//
#include "Code.h"
#include "Interpret.h"

bytes magicModuleHeader = new byte[]{0x00, 0x61, 0x73, 0x6d};
bytes moduleVersion = new byte[]{0x01, 0x00, 0x00, 0x00};

typedef unsigned char uint8_t;
typedef uint8_t byt;

//https://en.wikipedia.org/wiki/LEB128
// little endian 257 = 0x81 (001) + 0x02 (256)
Code &unsignedLEB128(long n) {
	Code *buffer = new Code(); // IF RETURNING Code&
	do {
		byt byte = n & 0x7f;
		n = n >> 7;
		if (n != 0) {
			byte |= 0x80;// continuation bit
		}
		buffer->add(byte);
	} while (n > 0);
	return *buffer;
}

Code &signedLEB128(long value) {
	Code *buffer = new Code();
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
		buffer->add(byte); //		emit byte;
	}
	return *buffer;
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
	Code code = unsignedLEB128(data.length);
	code = code + data;
//	Code code = Code((byte) data.length) + data;
	code.encoded = true;
	return code;
}


String sectionName(Section section) {
	switch (section) {
		case type_section:
			return "type";
		case table_section:
			return "table";
		case functypes_section:
			return "func";
		case import_section:
			return "import";
		case custom_section:
			return "custom";
		case global_section:
			return "global";
		case element_section:
			return "element";
		case export_section:
			return "export_section";
		case code_section:
			return "code";
		case data_section:
			return "data";
		case start_section:
			return "start";
		case memory_section:
			return "memory";
		default:
			error("INVALID SECTION #%d\nprevious section must have been corrupted\n"s % section);
	}
	return "ERROR";
}

chars typeName(Valtype t) {
	switch (t) {
		case Valtype::i32t:
			return "i32";
		case Valtype::i64:
			return "i64";
		case Valtype::float32:
			return "f32";
		case Valtype::float64:
			return "f64";
		case Valtype::array:
			return "array";
		case Valtype::charp:
			return "char*";
		case Valtype::voids:
			return "void";
		case Valtype::none:
			return "void_block";
		case Valtype::unknown_type:
			return "unknown";// internal
		default:
			error("missing name for Valtype "s + t);
	}
	return 0;
}


Valtype mapTypeToWasm(Node &n) {
	if (n == Double)
		return float64;
	if (n == Long)
		return i64;
	if (functionSignatures.has(n.name)) {
//		breakpoint_helper
//		todo(">>>");
		return functionSignatures[n.name].return_types.last();// no ??
	}

	//	if(n.type)…

	// int is not a true angle type, just an alias for long.
	// todo: but what about interactions with other APIs? add explicit i32 !
	// todo: in fact hide most of this under 'number' magic umbrella
	if (n.kind == bools)return int32;
	if (n.kind == nils)return voids;// mapped to int32 later: ø=0
//	if (n.kind == reals)return float32;// float64; todo why 32???
	if (n.kind == reals)return float64;
	if (n.kind == longs)return int32;// int64; todo!!
	if (n.kind == reference)return pointer;// todo? //	if and not functionIndices.has(n.name)
	if (n.kind == strings)return stringp;// special internal Valtype, represented as i32 index to data / pointer!
	if (n.kind == objects)return array;// todo
//	if (n.kind.type == int_array)return array;// todo
	if (n.kind == call) {
		// todo multi-value 2.  not a wasm type, maybe get signature?
		return functionSignatures[n.name].return_types.last();
	}
	if (n.kind == key and n.value.data) return mapTypeToWasm(*n.value.node);
	//	if (n.kind == key and not n.value.data)return array;
	if (n.kind == groups)return array;// uh todo?
	if (n.kind == unknown) return int32;// blasphemy!
	Node first = n.first();
	if (first == n)return int32;// array of sorts
	if (n.kind == assignment)return mapTypeToWasm(first);// todo
	if (n.kind == operators)return mapTypeToWasm(first);// todo
	if (n.kind == expression)return mapTypeToWasm(first);// todo analyze expression WHERE? remove HACK!
	n.print();
	error("Missing map for type %s in mapTypeToWasm"s % typeName(n.kind));
	return none;
}

Code createSection(Section sectionType, Code data) {
	return Code((char) sectionType, encodeVector(data));
}
