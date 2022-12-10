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
//	Code *buffer =(Code *) malloc(sizeof(Code));// new Code();
    Code *buffer = new Code();
    int more = 1;
//	bool negative = (value < 0);
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
Code encodeVector(const Code &data) {
//	return data.vector();
    if (data.encoded)return data;
    Code code = unsignedLEB128(data.length);
    code = code + data;
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
    }
}

chars typeName(Primitive p) {
    switch (p) {
        case Primitive::array:
            return "array";
        case Primitive::charp:
            return "chars";
        case Primitive::node:
            return "Node";
        case nodes:
            return "node";// Node*
        case Primitive::ignore:
            return "«ignore»";// or "" ;)
        case Primitive::todoe:
            return "«todo»";
        case unknown_type:
            return "unknown";// internal
        case codepoint32:
            return "codepoint";
//        default:
//            error("missing name for Primitive %x "s % p + p);
        case wasm_leb:
            return "leb";
        case wasm_float64:
            return "float64";
        case wasm_f32:
            return "float32";
        case wasm_int64:
            return "int64";
        case wasm_int32:
            return "int32";
        case type32:
            return "type";
        case any:
            return "any";
        case string_struct:
            return "String";
        case stringp:
            return "string";
        case byte_i8:
            return "byte";
        case byte_char:
            return "byte_char";
        case array_start:
            return "array_start";
        case list:
            return "list";
        case vector:
            return "vector";
        case array_header:
            return "array_header";
        case int_array:
            return "int_array";
        case long_array:
            return "long_array";
        case float_array:
            return "float_array";
        case real_array:
            return "real_array";
        case c_string:
            return "c_string";
        case leb_string:
            return "leb_string";
        case utf16_string:
            return "utf16_string";
        case utf32_string:
            return "utf32_string";
        case json5_string:
            return "json5_string";
        case json_string:
            return "json_string";
        case wasp_string:
        case wasp_data_string:
        case wasp_code_string:
            return "wasp";
        case result_error:
            return "result_error";
        case fointer:
        case fointer_of_int32: // ...
            return "pointer";
        case missing_type:
            return "missing"; // ≠ nul, undefined
        case maps:
            return "Map";
        case pad_to32_bit:
            error("don't use");
            break;
    }
    return 0;
}

chars typeName(Valtype t, bool fail) {
    switch (t) {
        case Valtype::i32t:
            return "i32";
        case Valtype::i64:
            return "i64";
        case Valtype::float32:
            return "f32";
        case Valtype::float64:
            return "f64";
//		case Primitive::node_pointer:
//			return "node";
        case Valtype::voids:
            return "void";
        case Valtype::none:
            return "void_block";
//        case unknown_type:
//            return "unknown";// internal
        default: {
            chars s = typeName((Primitive) t);
            if (s)return s;
//			if (t==30)return "BUG!";// hide bug lol
            if (fail)
                error("missing name for Valtype %x "s % t + t);
        }
    }
    return 0;
}

Valtype mapTypeToWasm(Primitive p) {
    switch (p) {
        case unknown_type: // undefined
        case missing_type: // well defined, but still:
            error("unknown_type in final stage");
        case wasm_leb:
            error("wasm_leb in wasm write stage");
        case wasm_float64:
            return Valtype::float64;
        case wasm_f32:
            return Valtype::float32;
        case wasm_int64:
            return Valtype::i64;
        case wasm_int32:
            return Valtype::int32;
        case type32:
            return Valtype::int32;
        case stringp:
            return Valtype::wasm_pointer;
        case nodes: // ⚠️ Node* Node** or Node[] ?
            return Valtype::wasm_pointer; // CAN ONLY MEAN POINTER HERE, NOT STRUCT!!
        case any:
            return Valtype::wasm_pointer;
        case array:
        case vector:
        case int_array:
        case long_array:
        case float_array:
        case real_array:
        case array_start:// todo What is the difference?
            return Valtype::wasm_pointer;
        case charp:
            return Valtype::wasm_pointer;
        case codepoint32:
            return Valtype::int32;// easy ;)

        case node:
        case list:
        case string_struct:
        case array_header:
        case maps:
//            return none;// todo:
//      a String struct is unrolled in the c/wasm-abi
            error("struct in final stage");
        case todoe:
            return Valtype::none; // none is too weak, it needs to fail on access, when reaching emit / emitCall etc!
            breakpoint_helper
            todo("some unmapped Type, debug earlier in mapArgToType");
        case ignore:
            return Valtype::none;
        case byte_i8:
            // ⚠️ careful in arrays we may write byte_i8 as Byte !
            return Valtype::int32;
        case byte_char:
            // ⚠️ careful in arrays we may write byte_char as ByteChar !
            return Valtype::int32;
        case c_string:
        case leb_string:
        case utf16_string:
        case utf32_string:
        case json5_string:
        case json_string:
        case wasp_string:
        case wasp_data_string:
        case wasp_code_string:
            return Valtype::wasm_pointer;
        case fointer: // unspecified pointer
        case fointer_of_int32: // specified pointer but we don't care any more
            return Valtype::wasm_pointer;
        case result_error:
            error("internal error or planned error as exception in wasm code?");
//            return Valtype::int64;
//            smart_pointer_64 ?
        case pad_to32_bit:
            error("don't use");
            return Valtype::none;
    }
}

// in final stage of emit, keep original types as long as possible
// todo: may depend on context?
Valtype mapTypeToWasm(Type t) {
    if (t.value < 0x80)
        return (Valtype) t.value;
    if (t.value < 0x1000000) // todo
        return mapTypeToWasm((Primitive) t);
    Node *type_node = (Node *) t.value;
    warn("Insecure mapTypeToWasm %x %d as Node*"s % t.value % t.value);
    if (type_node->node_header == node_header_32)
        return mapTypeToWasm(*type_node);// might crash
    todo("mapTypeToWasm %x %d %s"s % t.value % t.value % typeName(t));
    return Valtype::int32;
}

Primitive mapTypeToPrimitive(Node &n) {
    if (n == Int)
        return Primitive::wasm_int32;
    if (n == ByteType)
        return Primitive::byte_i8;// careful in structs!
    if (n == Long)
        return Primitive::wasm_int64;
    if (n == Double)
        return Primitive::wasm_float64;
    if (n == Charpoint)
        return Primitive::codepointus;
    else todo("mapTypeToPrimitive " + n.serialize());
    return Primitive::unknown_type;
}

Valtype mapTypeToWasm(Node &n) {
    if (n == Int)
        return i32;
    if (n == ByteType)
        return i32;// careful in structs!
    if (n == Long)
        return i64;
    if (n == Double)
        return float64;
    if (n == Charpoint)
        return (Valtype) codepoint32;

    if (n.type and n.type != n)
        return mapTypeToWasm(*n.type);

    if (types.has(n.name)) {
        Node *&typ = types[n.name];
        if (typ != n) return mapTypeToWasm(typ); // ⚠️ beware circles!
    }


    if (not n.name.empty() and functions.has(n.name)) {
        List<Type> &returnTypes = functions[n.name].signature.return_types;
        if (returnTypes.empty())return voids;
        Valtype valtype = mapTypeToWasm(returnTypes.last());
        return valtype;
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
    if (n.kind == reference)return (Valtype) pointer;// todo? //	if and not functionIndices.has(n.name)
    if (n.kind == strings)
        return (Valtype) stringp;// special internal Valtype, represented as i32 index to data / pointer!
    if (n.kind == objects)return (Valtype) array;// todo
//	if (n.kind.type == int_array)return array;// todo
    if (n.kind == call) {
        List<Type> &returnTypes = functions[n.name].signature.return_types;
        if (returnTypes.size() > 1) todo("multi-value");
        Type &type = returnTypes.last();
        return (Valtype) type.value;
    }
    if (n.kind == key and n.value.data) return mapTypeToWasm(*n.value.node);
    //	if (n.kind == key and not n.value.data)return array;
    if (n.kind == groups)return (Valtype) array;// uh todo?
    if (n.kind == flags) return i64;
    if (n.kind == enums) return i64;
    if (n.kind == unknown) return int32;// blasphemy!
    Node first = n.first();
    if (first == n)return int32;// array of sorts
    if (n.kind == generics)return mapTypeToWasm(first);// todo
    if (n.kind == assignment)return mapTypeToWasm(first);// todo
    if (n.kind == operators)return mapTypeToWasm(first);// todo
    if (n.kind == expression)return mapTypeToWasm(first);// todo analyze expression WHERE? remove HACK!
    n.print();
    error("Missing map for type %s in mapTypeToWasm"s % typeName(n.kind));
//    return none;
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
short lebByteSize(unsigned long neu) {
    short size = 0;
    do {
        size++;
        neu = neu >> 7;
    } while (neu > 0);
    return size;
}

short lebByteSize(unsigned int neu) {
    return lebByteSize((unsigned long) neu);
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
short lebByteSize(long long aleb) {
    int more = 1;
    short size = 0;
    long long val = aleb;
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
//    Code &leb128 = signedLEB128((long) leb);// todo later … optimize inline if…
//    return leb128.length;
}
