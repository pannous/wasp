
#include "String.h"
#include "NodeTypes.h"
#include "Util.h"
#include "Map.h"
#include "Node.h"

#if not RUNTIME_ONLY

#include "Angle.h"


#endif
extern Map<String, Node *> types;// by name
#include "Code.h"

extern Map<String, Global> globals;

Type mapType(Node &arg) {
    if (arg.type)
        return mapType(arg.type);
    // 'UPGRADE' Kind to Type
    switch (arg.kind) {
        case referencex:
            return Primitive::wasm_externref;
        case reference:
//            if(arg.size()==1)
//                return mapType(arg[0]);
//            if(context and context->locals.has(arg.name))
//                return context->locals[arg.name].type;
//            if(context.locals)
            return node;
        case undefined:
        case unknown:
            return Primitive::unknown_type;
        case nils:
            return nils;
        case number:
            return Primitive::wasm_int64;
        case reals:
            return Primitive::wasm_float64;
        case realsF:
            return Primitive::wasm_f32;
        case longs:
            return Primitive::wasm_int64;
        case long32:
            return Primitive::wasm_int32;
        case objects:
        case groups:
        case patterns:
            if (use_wasm_arrays)
                return Primitive::wasmtype_array;// array
            else
                return Primitive::node_pointer;
        case strings:
            if (use_wasm_strings)
                return string_ref;
            else
                return Primitive::stringp;
        case codepoint1:
            return Primitive::codepoint32;
        case buffers:
            return Primitive::array;
        case bools:
            return Primitive::byte_i8;// too early?
        case errors: todow("external or internal error?")
            return Primitive::result_error;
        case clazz:
            return Primitive::type32;
        case arrays:
            return Primitive::array;
//        case wasmtype_array:
//            return Primitive::wasmtype_array;
//        case wasmtype_struct:
//            return Primitive::wasm_type;
        case enums:
            return Primitive::wasm_int32;
        case flags:
            return Primitive::wasm_int64;
        case call: {
            auto fun = arg.name;
//            findLibraryFunction(fun, true); too much here
#if not RUNTIME_ONLY
            if (functions.has(fun)) {
                auto function = functions[fun];
                return function.signature.return_types.last(voids);
            }
#endif
            todo("how to map "s + typeName(arg.kind) + " to (wasm) Type?");
        }
        case global:
            if (globals.has(arg.name))
                return globals[arg.name].type;
            else
                error("global not found "s + arg.name);
        case variants:
        case records:
        case constructor:
        case modul:
        case last_kind:
        case kind_padding:
        case structs:
        case generics:
        case linked_list:
        case key:
        case fields:
        case symbol:
        case operators:
        case functor:
        case expression:
        case declaration:
        case assignment: todo("how to map "s + typeName(arg.kind) + " to (wasm) Type?");
            break;
    }
    return arg.kind;
}

Type mapType(Node *arg) {
    if (not arg)return ignore;// todo?
    return mapTypeToPrimitive(*arg);
//    todo("mapType Node")
}

Type mapType(String arg, bool throws) {
    if (arg.startsWith("const "))
        arg = arg.substring(6);
//	if(arg=="const char*")return charp;
    if (arg.empty() or arg == "" or arg == " ") return voids;
    else if (arg == "void")return voids;
    else if (arg == "unsigned char*")return charp;// pointer with special semantics
    else if (arg == "char const*")return charp;// pointer with special semantics
    else if (arg == "char const*&")return charp;// todo ?
    else if (arg == "char*")return charp;
    else if (arg == "char32_t*")return codepoint1; // ≠ codepoint todo, not exactly: WITHOUT HEADER!
    else if (arg == "char const**")return pointer;
    else if (arg == "short")
        return int32;// vec_i16! careful c++ ABI overflow? should be fine since wasm doesnt have short
    else if (arg == "int")return int32;
    else if (arg == "IntegerType")return i32;
    else if (arg == "signed int")return i32s;
    else if (arg == "unsigned int")return i32;
    else if (arg == "unsigned char")return int32;
    else if (arg == "int*")return pointer;
    else if (arg == "void*")return pointer;
    else if (arg == "long")return i64;
    else if (arg == "long long")return i64;
    else if (arg == "unsigned long")return i64;// we don't care about unsigned
    else if (arg == "unsigned long long")return i64;
    else if (arg == "long&")return pointer;
    else if (arg == "unsigned short")return shorty;
    else if (arg == "int64")return i64;
    else if (arg == "uint64")return i64;
    else if (arg == "DoubleType")return float64;
    else if (arg == "long double")return float64;
    else if (arg == "double")return float64;
    else if (arg == "float")return float32;
    else if (arg == "bool")return int32;
    else if (arg == "const char")return byte_char;
    else if (arg == "char")return byte_char;// c++ char < angle codepoint ok
    else if (arg == "signed char")return byte_i8;
    else if (arg == "wchar_t")return (Valtype) codepoint32;// angle codepoint ok
    else if (arg == "char32_t")return codepoint32;// angle codepoint ok
    else if (arg == "char16_t")return codepoint32;// !? ⚠️ careful
    else if (arg == "char**")return pointer;// to chars
    else if (arg == "short*")return pointer;


        // Some INTERNAL TYPES are reflected upon / exposed as abi :
    else if (arg == "Type")return int32;// enum
    else if (arg == "Kind")return int32;// enum (short, ok)
    else if (arg == "Type")return type32;// enum
    else if (arg == "CodepointType")return charp;// enum

    else if (arg == "string")return stringp;
    else if (arg == "String*")return stringp;
    else if (arg == "String&")return stringp;// todo: how does c++ handle refs?
    else if (arg == "String")return string_struct;
    else if (arg == "const String")return string_struct;

    else if (arg == "Node")return node;// struct!

    else if (arg == "Node&")return node_pointer;// pointer? todo: how does c++ handle refs?
    else if (arg == "Node const&")return node_pointer;
    else if (arg == "Node const*")return node_pointer;
    else if (arg == "Node*")return node_pointer;

    else if (arg == "NumberType")return ignore;// todo
    else if (arg == "Number")return number;
    else if (arg == "BigInt")return number;// todo
    else if (arg == "Generics")return generics;
    else if (arg == "Type32")return type32;
    else if (arg == "Type")return type32;
    else if (arg == "Kind")return type32;
    else if (arg == "Primitive")return type32;// good thing wasm has no polymorphism
    else if (arg == "Valtype")return type32;// good enough!
    else if (arg == "Type64")return ignore; // for now   return smarti64;
    else if (arg == "Type64::Type64")return ignore; // for now   return smarti64;
    else if (arg == "Type&")error("Type should only be used as value");
    else if (arg == "List<String>")return list;
    else if (arg == "List<Type>")return list;
    else if (arg == "List<int>")return list;

    else if (arg == "Module")return modul;
    else if (arg == "Module const&")return modul;

    else if (arg == "SyntaxError")return errors;
//    else if (arg == "SyntaxError")return result_error;

    else if (arg == "std::is_arithmetic<int>::value")return todoe;// WAT?? PURE_WASM should work without std!!


        // IGNORE other INTERNAL TYPES:
    else if (arg == "Code")return ignore;
//    else if (arg == "Map<String")return ignore;
//    else if (arg == "int>")return ignore;// parse bug ^^
    else if (arg == "Function")return ignore;
    else if (arg == "Sections")return ignore;
    else if (arg == "Local")return ignore;
    else if (arg == "Code&")return ignore;
    else if (arg == "Code const&")return ignore;
    else if (arg == "Section")return ignore;
    else if (arg == "Global")return ignore;
    else if (arg == "ParserOptions")return ignore;
    else if (arg == "Value")return ignore;
    else if (arg == "Arg")return ignore; // truely internal, should not be exposed! e.g. Arg
    else if (arg == "Signature")return ignore;
    else if (arg == "Wasp")return ignore;
    else if (arg == "WitReader")return ignore;
    else if (arg == "if")
        return ignore; // bug!
    else if (arg == "__cxxabiv1")return ignore;
    else if (arg == "...")return ignore;// varargs, todo interesting!
    else if (arg.startsWith("Map")) return maps;
    else if (arg.startsWith("List")) return list;
    else if (arg.startsWith("std")) return todoe;// #234 ƒ244 std::initializer_list<String>::end() const ≈
    else if (arg.endsWith("&")) return pointer;
    else if (arg.endsWith("*")) return pointer;

    else {
//        breakpoint_helper
//        printf("unmapped c++ argument type %s\n", arg.data);
        if (not throws)
            return unknown_type;
        if (!arg.endsWith("*"))
            if (!arg.startsWith("Map<") and !arg.startsWith("List<"))
                error("unmapped c++ argument type %s\n"s % arg.data);
    }
    return i32t;
}


// four different types:
// 1. wasm Valtype
// 2. node.kind:Type
// 3. Any<Node and
// 4. some c++ types String List etc
// the last three can be added as special internal values to Valtype, outside the wasm spec
static Valtype mapArgToValtype(String &arg) {
    Type primitive = mapType(arg, true);
    return mapTypeToWasm(primitive);
}


extern "C" chars kindName(::Kind t) {
    return typeName(t, false);
}


//String
chars typeName(Kind t, bool throws) {
    switch (t) {
        case 0:
            return "ø"; // undefined or void
        case unknown:
            return "unknown";
        case objects:
            return "object";
        case groups:
            return "group";
        case patterns:
            return "pattern";
        case key:
            return "key";
//            return "node";
        case fields:
            return "field";
        case reference:
            return "reference";
        case referencex:
            return "$reference";
        case symbol:
            return "symbol";
        case operators:
            return "operator";
        case expression:
            return "expression";
        case strings:
            return "string";
        case linked_list:
            return "list";//"linked_list";
        case arrays:
            return "array";
        case buffers:
            return "buffer";
        case realsF:
            return "float";
        case reals:
            return "real";
        case number: // SmartNumber or Number* ?
            return "number";
        case longs:
            return "long";
        case long32:
            return "int";
            //		case ints:
            //			return "int";
        case bools:
            return "bool";
        case nils:
            return "nil";
        case call:
            return "call";// function
        case declaration:
            return "declaration";
        case assignment:
            return "assignment";
        case errors:
            return "error";
        case functor:
            return "functor";
        case codepoint1:
            return "codepoint";
        case enums:
            return "enum";
        case variants:
            return "variant";
        case records:
            return "record";
        case generics:
            return "generics";
        case clazz:
            return "class";
//        case wasmtype_struct:
        case structs:
            return "struct";
        case flags:
            return "flags";
        case constructor:
            return "constructor";
        case modul:
            return "module";
        case global: // todo global of what name, what type
            return "global";
        case last_kind:
        default:
            if ((short) t == stringref)
                return "stringref";// todo how here?
            if ((short) t == wasmtype_array)
                return "array";
            if ((short) t == ref)
                return "ref(…)";
            if (throws)
                error("MISSING Type Kind name mapping "s + (int) t);
            else return "";
    }
    return "";
}


chars typeName(Type t) {
    if (t.value < last_kind)return typeName(t.kind);
    if (t.value < 0x10000)return typeName((Primitive) t);
    if (isGeneric(t)) {
        auto kind = t.generics.kind;
        auto type = t.generics.value_type;
        return ""s + typeName((Kind) kind, false) + "<" + typeName((Primitive) type) + ">";
    }
    // todo : make sure to emit Nodes at > 0x10000 …
    warn("typeName %x %d "s % t.value % t.value);
#if !WASM
//    warn("Node pointers don't fit in 32 bit Type!")
    todo("Node pointers don't fit in 32 bit Type!");
#else
    if (t.value > 0x10000)return ((Node*)t.address)->name;
#endif
    error("MISSING Type name mapping "s + typeName(t));
    return "ƒ";
}

chars typeName(const Type *t) {
    if (not t)return "ø undefined";
    return typeName(*t);
}


// in final stage of emit, keep original types as int64 as possible
// todo: may depend on context?
Valtype mapTypeToWasm(Type t) {
    if (t.value < 0x80)
        return (Valtype) t.value;
    if (t.value < 0x10000) // todo
        return mapTypeToWasm((Primitive) t);
    if (isGeneric(t) and use_wasm_arrays) {
        warn("isGeneric Type");
        debug_line();
//	    warn(typeName(t));
        puti(t.value);
        // todo!
//	    print(typeName(kind));
//	    print(typeName(valType));
        if (t.generics.value_type < 0)
            error1("generics with invalid Valtype");
        auto kind = (ushort) mapTypeToWasm((Kind) t.generics.kind);
        auto valType = (ushort) mapTypeToWasm((Primitive) t.generics.value_type);
        puti(kind);
        puti(valType);
        return (Valtype) (kind * 0x100 + valType);
        error("generics needs more than Valtype");
    }
    if (isArrayType(t))
        return mapTypeToWasm(array); // wasm_pointer
#if WASM
    Node *type_node = (Node *) (long) t.value;
#else
    Node *type_node = (Node *) (long) t.value;
    error("Type32 can't hold pointer on 64bit systems");
#endif
    warn("Insecure mapTypeToWasm %x %d as Node*"s % t.value % t.value);
    if (type_node->node_header == node_header_32)
        return mapTypeToWasm(*type_node);// might crash
    todo("mapTypeToWasm %x %d %s"s % t.value % t.value % typeName(t));
    return Valtype::int32;
}

Primitive mapTypeToPrimitive(Node &n) {
    if (n == IntegerType)
        return Primitive::wasm_int32;
    if (n == ByteType)
        return Primitive::byte_i8;// careful in structs!
    if (n == ShortType)
        return Primitive::int16;
    if (n == LongType)
        return Primitive::wasm_int64;
    if (n == DoubleType)
        return Primitive::wasm_float64;
    if (n == CodepointType)
        return Primitive::codepoint32;
    if (n == StringType)
        return Primitive::stringp;
    else if (mapType(n.name, false) != unknown_type)
        return mapType(n.name, false).type;
    else todow("mapTypeToPrimitive "s + n.serialize());
    return Primitive::unknown_type;
}

Valtype mapTypeToWasm(Node &n) {
    if (n == IntegerType)
        return i32;
    if (n == ByteType)
        return i32;// careful in structs!
    if (n == LongType)
        return i64;
    if (n == DoubleType)
        return float64;
    if (n == CodepointType)
        return (Valtype) codepoint32;

    if (n.type and n.type != n)
        return mapTypeToWasm(*n.type);

    if (types.has(n.name)) {
        Node *&typ = types[n.name];
        if (typ != n) return mapTypeToWasm(typ); // ⚠️ beware circles!
    }


//    if (not n.name.empty() and functions.has(n.name)) {
//        List<Type> &returnTypes = functions[n.name].signature.return_types;
//        if (returnTypes.empty())return voids;
//        Valtype valtype = mapTypeToWasm(returnTypes.last());
//        return valtype;
//    }

    //	if(n.type)…

    // int is not a true angle type, just an alias for int64.
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
        todo("type of call")
//        List<Type> &returnTypes = functions[n.name].signature.return_types;
//        if (returnTypes.size() > 1) todo("multi-value");
//        Type &type = returnTypes.last();
//        return (Valtype) type.value;
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
    return none;
}

chars typeName(Primitive p) {
    switch (p) {
        case Primitive::wasmtype_array:
        case Primitive::array:
            return "array";
        case Primitive::charp: // char*
            return "chars";
//        case c_string: // same as char*
//            return "c_string";
        case Primitive::node:
            return "Node";
        case node_pointer:
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
        case Primitive::wasm_type:
        case Primitive::wasmtype_struct:
            return "struct";
        case string_struct:
            return "String";
        case stringp:
            return "string";
        case byte_i8:
            return "byte";
        case Primitive::shorty:
            return "short";
        case byte_char:
            return "byte_char"; // ascii char
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
        case wasm_externref:
            return "$ref";
        case smarti64:
            return "smarty";// wasp smart pointer int64 ABI vs multi value ABI (value, type)
        case pad_to32_bit:
            error("don't use");
            break;
            break;

    }
    return "?";
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
        case Valtype::string_ref:
            return "wasm_string_ref";
        case Valtype::externref:
            return "ref"; // todo :
        case Valtype::ref: // wasm struct / array
            return "wasm_struct";
//        case unknown_type:
//            return "unknown";// internal
        default: {
            chars s = typeName((Primitive) t);
            if (s)return s;
            s = typeName((Kind) t);
            if (s)return s;
            s = typeName((Valtype) t);
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
        case Primitive::wasmtype_struct:
        case Primitive::wasm_type:
            return wasm_struct;//  ⚠️ todo: plus struct type
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
            if (use_wasm_strings)
                return string_ref;
            return Valtype::wasm_pointer;
        case node_pointer: // ⚠️ Node* Node** or Node[] ?
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
        case wasmtype_array:
            if (use_wasm_arrays)
                return Valtype::wasm_array; //  ⚠️ todo plus a type!
            return Valtype::wasm_pointer;
        case charp:
            return Valtype::wasm_pointer;
        case codepoint32:
            return Valtype::int32;// easy ;)

        case node:
            if (allow_untyped_nodes)
                return Valtype::int32;// todo!
        case string_struct:
            //      a String struct is unrolled in the c/wasm-abi
            error("string struct in final stage");
        case list:
        case array_header:
        case maps:
//            return none;// todo:
//      a String struct is unrolled in the c/wasm-abi
            error("struct in final stage");
        case todoe:
            return Valtype::none; // none is too weak, it needs to fail on access, when reaching emit / emitCall etc!
            breakpoint_helper
            todo("some unmapped Type, debug earlier in mapType");
        case ignore:
            return Valtype::none;
        case byte_i8:
        case int16:
            // ⚠️ careful in arrays we may write byte_i8 as Byte !
            return Valtype::int32;
        case byte_char:
            // ⚠️ careful in arrays we may write byte_char as ByteCharType !
            return Valtype::int32;
//        case c_string:// charp
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
        case Primitive::smarti64:
            return Valtype::i64;
        default:
            error("missing type in mapTypeToWasm "s + typeName(p));
    }
}


bool isGroup(Kind type) {
    return type == groups or type == objects or type == patterns or type == expression;
//    or (Primitive)type == node_pointer or type== lists or type == maps;
}

// todo: use SINGLE BIT to denote a generic type, and the rest for the value type and other flags
uint generics_mask = 0xF0FF0000;// ⚠️ preserve 4 bits for not generics flags ( static mutable … )
Type valueType(Type type) {
    if (type.value & generics_mask)
        return type.generics.value_type;
    error("not a generic type "s + typeName(type));
    return none;
}

Type genericType(Type type, Type value_type) {
    warn("genericType "s + typeName(type) + " for " + typeName(value_type));
    if (type.value >= 0x10000 or value_type.value >= 0x10000)
        error("not a generic type holder "s + typeName(type) + " for " + typeName(value_type));
    return Type(Generics{.kind = (ushort) type.value, .value_type = (ushort) value_type.value});
}

bool isArrayType(Type type) {
    return type.isArray();
}

bool isGeneric(Type type) {
    return type.value & generics_mask;// >=0x10000;
}

void print(Kind k) {
    print(typeName(k));
}