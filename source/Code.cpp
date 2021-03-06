//
// Created by me on 08.12.20.
//
#include "Code.h"

bytes magicModuleHeader = new byte[]{0x00, 0x61, 0x73, 0x6d};
bytes moduleVersion = new byte[]{0x01, 0x00, 0x00, 0x00};

bytes concat(bytes a, bytes b, int len_a, int len_b) {
	bytes c = new unsigned char[len_a + len_b];// why+4 ?? else heap-buffer-overflow
	memcpy0(c, a, len_a);
	memcpy0(c + len_a, b, len_b);
//	c[len_a + len_b + 1] = 0;// hwhy?
	return c;
}


bytes concat(bytes a, char b, int len) {
	bytes c = new unsigned char[len + 1];
	memcpy0(c, a, len);
	c[len] = b;
	return c;
}

bytes concat(char a, bytes b, int len) {
	bytes c = new unsigned char[len + 1];
	c[0] = a;
	memcpy0(c + 1, b, len);
	return c;
}

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
}

Code createSection(Section sectionType, Code data) {
	return Code((char) sectionType, encodeVector(data));
}
