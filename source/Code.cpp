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
Code &unsignedLEB128(int64 n) {
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

Code &signedLEB128(int64 value) {
//	Code *buffer =(Code *) malloc(sizeof(Code));// new Code();
    Code *buffer = new Code();
    int more = 1;
//	bool negative = (value < 0);
    int64 val = value;
/* the size in bits of the variable value, e.g., 64 if value's type is int64 */
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
Code encodeVector(const Code &data) {
//	return data.vector();
    if (data.encoded)return data;
    Code code = unsignedLEB128(data.length);
    code.add(data);
//	Code code = Code((byte) data.length) + data;
    code.encoded = true;
    return code;
}


String sectionName(Sections section) {
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
            return "INVALID";
    }
}

Code createSection(Sections sectionType, const Code &data) {
    return {(char) sectionType, encodeVector(data)};
}


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
short lebByteSize(uint64 neu) {
    short size = 0;
    do {
        size++;
        neu = neu >> 7;
    } while (neu > 0);
    return size;
}

short lebByteSize(unsigned int neu) {
    return lebByteSize((uint64) neu);
}


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
-1 0 1
-65 ffffffffffffffbf 2
-8193 ffffffffffffdfff 3
-1048577 ffffffffffefffff 4
-134217729 fffffffff7ffffff 5
-17179869185 fffffffbffffffff 6
-2199023255553 fffffdffffffffff 7
-281474976710657 fffeffffffffffff 8
-36028797018963969 ff7fffffffffffff 9
 */
short lebByteSize(int64 aleb) {
    int more = 1;
    short size = 0;
    int64 val = aleb;
    while (more) {
        uint8_t b = val & 0x7f;
        /* sign bit of byte is second high order bit (0x40) */
        val >>= 7;
        bool clear = (b & 0x40) == 0;  /*sign bit of byte is clear*/
        bool set = b & 0x40; /*sign bit of byte is set*/
        if ((val == 0 && clear) || (val == -1 && set))
            more = 0;
        else {
            b |= 0x80;// todo NEVER USED!! continuation bit:  set high order bit of byte;
        }
        size++;
    }
    return size;
//    Code &leb128 = signedLEB128((int64) leb);// todo later … optimize inline if…
//    return leb128.length;
}
