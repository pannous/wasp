#pragma once
//
// Created by pannous on 19.12.19.
//

#include "smart_types.h" // todo merge
//#include "Code.h"
//#include "String.h"
#include "Util.h"

//enum Valtype;// forbids forward references to 'enum' types
class Node;

typedef void *ExternRef;

typedef unsigned int wasm_node_index; // Node* pointer INSIDE wasm_memory


union Type;

const char *typeName(const Type *t);

// for Opcodes see Code.h

// https://webassembly.github.io/spec/core/binary/types.html
// https://webassembly.github.io/spec/core/binary/values.html
// Wasp/angle has four different types:
// 1. wasm Valtype
// 2. node.kind:Type
// 3. Any<Node and
// 4. some c++ types String List etc
// the last three can be added as special internal values to Valtype, outside the wasm spec
enum Valtype {
    //    unknown_type = -1,

    voids = 0x00, // DANGER!=void_block  ⚠️ INTERNAL ⚠ only for return type
    void_block = 0x40, // ⚠️
    none = 0x40, // NOT voids!!!

    // ≠ externref plus type id! vs 0xfb… for functions struct.new struct.get …  ≠ wasmtype_struct 0x5f
    wasm_struct = 0x6b, // ≠ wasmtype_struct = 0x5f in type section
    wasm_array = wasm_struct, // 0x6b ≠ wasmtype_array = 0x5e in type section
    /*
    ref                    // Abstract supertype of all references
    ├── funcref            // References to WebAssembly functions
    ├── externref          // References to host-managed objects (e.g., JS objects)
    └── eqref              // References to equality-comparable objects (e.g., GC structs, arrays)
        └── i31ref             // References to immutable 31-bit integers
        └── (ref null <type>)  // Typed nullable references (introduced by GC proposal) only if T is equality-comparable
    */
    ref = 0x6C, // followed by 0x00 for nullable, Followed by the index of the referenced type!
    eqref = 0x6D, // gc structs, arrays
    i31ref = 0x6E,
    anyref = 0x6f, // was conceptually and name-wise merged into externref
    externref = 0x6f, // -0x11 111 (js)object !  ≠ 'ref'=wasm_struct!!
    object = externref,
    js_object = externref,
    funcref = 0x70, // -0x10
    func = 0x60,
    string_ref = 0x64, // wasm stringref
    stringref = string_ref, //         ; SLEB128(-0x1c)
    wasm_string_ref = string_ref, //      ; SLEB128(-0x1c)
    stringview_wtf8 = 0x63, //    ; SLEB128(-0x1d)
    stringview_wtf16 = 0x62, //   ; SLEB128(-0x1e)
    stringview_iter = 0x61, //    ; SLEB128(-0x1f)

    // these are followed by special vector opcodes ≠ normal opcodes
    vec_i8 = 0x7A, // see VectorOpcodes @ Code.h
    vec_i16 = 0x79,
    vec_i32 = 0x78,
    vec_i64 = 0x77,
    vec_f32 = 0x76, // ⚠ missed opportunity! :(
    vec_f64 = 0x75, // could have been shared with vector PREFIX 0x777C => 0x77 'vector' of 0x7C float64 !

    vec_v128 = 0x7B, // small 4 element SIMD vector todo or also 'real' vectors: ?
    //  [Flexible Vectors](https://github.com/WebAssembly/flexible-vectors/blob/main/proposals/flexible-vectors/FlexibleVectors.md)

    // ⚠️ strange order!
    float64t = 0x7C,
    f64t = 0x7C,
    //    f64 = 0x7C, // typedef double f64; in m3

    float32t = 0x7d,
    f32t = 0x7d,
    //    f32s = 0x7d,
    //    f32 = 0x7d, typedef float f32 in wasm2c, we MAY encounter it more often…
    //	f32u = 0x7d,// todo ignore!

    i64 = 0x7E, // signed or unsigned? we don't care
    i64t = 0x7E,
    i64s = 0x7E,
    int64t = 0x7E,
    int64s = 0x7E, // symbol now used as

    i8 = 0x7A, // == vec_i8 ⚠️ONLY in GC types array, struct, etc
    i16 = 0x79, // == vec_i16 ⚠️

    //    int64 = 0x7E,  // symbol now used as
    //    typedef int64 int64

    // int32t = 0X7F,
    ints = 0X7F, // longy, but not long
    int32t = 0X7F,
    i32t = 0x7f,
    i32 = 0x7f,
    i32s = 0x7f,
    size32 = 0x7f,
    wasm_pointer = int32t,
};

// types
//extern const Node DoubleType;
//extern const Node LongType;
extern Node IntegerType; // maps to int32 in external abi! don't use internally much, only for struct
extern Node LongType;
extern Node DoubleType; // FloatType RealType
extern Node BoolType;
extern Node CodepointType; // CharType CharacterType
extern Node ByteType;
extern Node ByteCharType; // ugly by design // todo char ≠ char* !
extern Node ShortType;
extern Node StringType;
extern Node TemplateType; // `strings with $values`
// extern Node ArrayType; // auto map to Node{kind=arrays} or Node{kind=vectors} or Node{kind=lists} etc


//#include "Util.h" // for error() :(
//#include "Code.h"

// Todo: can Type itself become a Node, making the distinction between type and kind superfluous?
// todo dangerous overlap with Valtype in Type!? OK only accessible via mapTypeToWasm
// needs to be stable: Kind is returned in multivalue and thus needs to be parsed by js!
// todo change naming scheme: remove false plural 's' where inappropiate: 'strings' groups … … …
// compare / merge(?) with enum Primitive? nah, primitive is internal / wasm only
// e.g. Kind.longy is container for i32 i64 bool enums ...
enum Kind /* 32 bit*/ {
    // todo: merge Node.kind with Node.class(?)
    // todo smartType4bit first 16 values!!
    // plurals because of namespace clash
    // TODO add subtypes of Class:Node Variable:Node etc ^^^
    undefined = 0, // ≠ any?
    unknown = 1, //
    objects, // {…} block data with children closures
    strings = 0x9,
    call = 0x10,
    // todo: make groups,  patterns, objects into ONE and differentiate via meta
    groups, // (…) meta params parameter attributes lists
    tuples = groups, // todo: with restrictions!
    patterns, // […] selectors matches, annotations! [public export extern] function mul(x,y){x*y}
    generics, // node tag or list<node>   NOT value.kind==strings !
    tags = generics, // <html>

    key, // key with value
    fields, // key in struct / class / type / prototype / interface / record (wit) possibly WITHOUT VALUE
    // todo do we really need strict schema separation from normal 'schema' of node{kind=clazz} ?

    reference, // variable identifier name x
    global, // global variable

    symbol, // one / plus / Jesus
    operators, // TODO: semantic types don't belong here! they interfere with internal structural types key etc!!
    functor, // while(xyz){abc} takes 1?/2/3 blocks if {condition} {then} {else}
    expression, // one plus one
    declaration, // x:=1
    assignment, // x = 1 // really?? needs own TYPE?
    buffers, // int[] DANGER todo length stored in node.length? or better in meta["length"] !
    //	ints, // use longy field, but in wasm longs are pointers!
    bools,
    errors, // internal wasp error, NOT to be used in Angle!
    clazz,

    // each class can be made into a typed list / vector int => int[] :, // as Java class,  primitive int vs Node(kind==int) == boxed IntegerType.
    //	vectors     use kind=array type=Any
    //	vectors, // any[] vs Node[]  list of type class!  e.g.  numbers xs => xs.type=number(type=vectors)
    arrays, // Node[] vs any[]  untyped array of Nodes. Unlike vector the node is NOT a class! todo: see Classes
    linked_list, // via children or value field??
    meta = linked_list,
    enums,
    flags, // just a boolean enum with “bit-boolean” values 1,2,4,8,…
    variants,
    //    unions, // todo, also option
    records, // todo merge concepts with module wasp clazz?
    constructor, // special call?
    modul, // module, interface, resource, world, namespace, library, package ≈ class …
    nils = 0x40, // ≈ void_block for compatibility!?  ≠ undefined
    referencex = externref, // external reference as per wasm spec
    number = 0x70, // SmartNumber or Number* as SmartPointer? ITS THE SAME!
    //    wasmtype_struct = 0x6b /* ⚠️ PLUS stuct ID! */, // opcodes 0xFB…
    //    wasmtype_array = 0xfb1a,

    structs = 0x77, // TODO BEWARE OF OVERLAP with primitives! :
    reals = 0x7C, /*  ≠ float64 , just hides bugs, these concepts should not be mixed */
    doubles = 0x7C,
    realsF = 0x7D, /*  ≠ float32 , just hides bugs, these concepts should not be mixed */
    floats = 0x7D,
    longs = 0x7E, // the signature of parameters/variables is independent!
    long32 = 0x7F, /* int stored in long node field  ≠ int32 , just hides bugs, these concepts should not be mixed */

    flag_entry = longs, // special semantics at compile time for now
    enum_entry = longs, // special semantics at compile time for now
    last_kind = 0x80,
    codepoint1 = 0xC4, // in value.longy field, boxed as node vs codepoint32 primitive todo?
    urls, // special references/strings IN NAME!

    kind_padding = 0x80000000, // TODO do 32 bit padding differently! keep kind in 8 bits! TODO remove!
};

bool isGroup(Kind type); // Node of kind group


// Raw types as encountered in C/C++ ABI
// unboxed primitive raw data (list is compatible with List though!)
// Todo these should NOT appear as Node.kind except for Node{value.data=Address<Primitive>}
// todo use NodeTypes.h Type for smart-pointers :
//	https://github.com/pannous/angle/wiki/smart-pointer
// see header_4 / smartType4bit
// todo universal micro bits for 1. POINTER 2. ARRAY 2. STRUCT with HEADER
// INTERNAL TYPES mapped to wasm types
enum Primitive /*32*/ {
    //    THE 0x00 …0x0F …0xF0 … range is reserved for numbers
    //   redundant Valtype overlap

    //    Kind::undefined = 0 ==
    unknown_type = 0, // defaults to int64!
    missing_type = 0x40, // well defined
    nulls = 0x40, // ≠ undefined
    wasm_type = 0x6b, /* ⚠️ PLUS stuct type ID! */
    wasm_ref = 0x6b, /* ⚠️ PLUS ref type ID! ≠ */
    wasm_externref = 0x6f,
    wasmtype_struct = 0x5f, // as in type section
    wasmtype_array = 0x5e, // as in type section
    wasm_leb = 0x77,
    wasm_float64 = 0x7C, // float64
    float_type = wasm_float64,
    wasm_f32 = 0x7d,
    wasm_int64 = 0x7E, // AS OPPOSED TO longs signed or unsigned? we don't care
    wasm_int32 = 0x7f, // make sure to not confuse these with boxed Number node_pointer of kind longs, reals!
    pointer = wasm_int32, // 0xF0, // todo: lower later! internal todo: int64 on wasm-64
    self = pointer,
    //    node_pointer = wasm_int32,
    node_pointer = 0xD000, // address of a Node in linear memory

    //	node = int32, // NEEDS to be handled smartly, CAN't be differentiated from int32 now!
    type32 = 0x80, // todo see smart_pointer_64 etc OK?
    ////	smarti32 = 0xF3,// see smartType
    smarti64 = 0xF6,
    node = 0xA0, // Node struct versus Node* node_pointer = 0xD000  todo better scheme 0xAF F for fointer!
    //    angle = 0xA4,//  angle object pointer/offset versus node_pointer smarti vs anyref todo What is this?
    any = 0xA1, // Wildcard for function signatures, like haskell add :: a->a
    //	unknown = any,
    array = 0xAA, // compatible with List, Node, String (?)
    charp = 0xC0, // char* vs codepoint(*) vs byte_char
    stringp = 0xCF, // String* vs stringref …
    string_struct = 0xC8, // String
    //    codepointus = 0xC1,  // when indexing int32 array
    codepoint32 = 0xC4, // just ONE codepoint as int! todo via map

    //    byte_char = 0xC1, // 0xBC

    //	floats = 0x1010, // only useful for main(), otherwise we can return real floats or wrapped Node[reals]
    //	pointer = int32,// 0xF0, // internal
    ////	node = int32, // NEEDS to be handled smartly, CAN't be differentiated from int32 now!
    ////	node = 0xA0,
    //	angle = 0xA0,//  angle object pointer/offset versus smarti vs anyref
    //	any = 0xA1,// Wildcard for function signatures, like haskell add :: a->a
    ////	unknown = any,
    //	array = 0xAA,
    //	charp = 0xC0, // vs
    //	stringp = 0xC0,// use charp?  pointer? enough?? no!??
    ////	value = 0xA1,// wrapped node Value, used as parameter? extract and remove! / ignore
    //	todoe = 0xFE, // todo
    ////	error_ = 0xE0, why propagate?
    ////	pointer = 0xF0,
    ////	externalPointer = 0xFE,
    //	ignore = 0xAF, // truely internal, should not be exposed! e.g. Arg

    todoe = 0xE1, // todo
    ignore = 0xAF, // truely internal, should not be exposed! e.g. Arg

    // todo: different levels
    //  error_header_4 = 0xE check(type >> 28 == error_header_4 )  / type64 >> 60
    //	error8 = 0xE0, why propagate?
    //	error_mask_32   = 0xEF000000
    //	error_header_32 = 0xE0000000   check(type & error_mask_32 = error_header_32)
    //	error_header_64 = 0xE000000000000000
    //	error_mask_60   = 0xEF00000000000000 NO, we split in type32 BEFORE!

    //	pointer = 0xF0,
    //	externalPointer = 0xFE,

    byte_i8 = 0xB0, // when indexing uint8 byte array.
    byte_char = 0xBC, // when indexing ascii array. todo: maybe codepoint into UTF8!?
    shorty = 0xB16,
    int16t = 0xB16,
    //    THE 0xF0 … range is reserved for numbers
    // Type =>  must use 'enum' tag to refer to type 'Type' NAH!
    //	c_char = 0xB0, // when indexing byte array. todo: maybe codepoint into UTF8!?
    array_start = 0xA000, // careful there are different kinds of arrays/lists/List/Node[lists]
    list = 0xA100, // [len(int32), data*] compatible with List
    vector = 0xA200,
    // [len(leb), data…] as in wasm, should never be boxed, but converted to list, string, objects… upon parsing
    array_header = 0xA300,
    // [type, len, data*] or [type, len, data…] ? 0 termination for sanity (but data can be 0 too!)
    //	compatible with List after shift, compatible with array_start after two shifts
    // TODO: array 0xA000 combinatorial with primitive types
    int_array = 0xA07f, // primitive, unboxed, no length!?
    long_array = 0xA07e,
    float_array = 0xA07d,
    real_array = 0xA07c,
    //	longs_array= 0xA00E, todo: boxed Node{kind=longs}
    //	reals_array= 0xA00C, todo: boxed Node{kind=reals}
    //	byte_list = 0xB000, // not 0 terminated, length via 3rd return item??
    //	byte_vector = 0xB001, // LEB encoded length header
    //    block = 0xB000,
    maps = 0xA0B0, // todo generics Map<String; int> …
    c_string = 0xC0, // char* same as charp pointer to utf-8 bytes in wasm's linear memory, 0 terminated
    //    c_string = 0xC000, // pointer to utf-8 bytes in wasm's linear memory, 0 terminated
    leb_string = 0xC001, // LEB encoded length header -> wasm_string
    // todo universal micro bits for 1. POINTER 2. ARRAY 2. STRUCT with HEADER
    utf16_string = 0xC016, // pointer to utf-16 bytes in wasm's linear memory, 0.0 terminated
    utf32_string = 0xC032, // pointer to utf-16 bytes in wasm's linear memory, 0.0 terminated
    json5_string = 0xC005,
    json_string = 0xC010,
    wasp_string = 0xC0D0, // serialized Node ≠ Node address
    wasp_data_string = 0xC0DA, // serialized Node ≠ Node address
    wasp_code_string = 0xC0DE, // charged data, see https://github.com/pannous/wasp/wiki/charged
    //	maps, // our custom Map object todo: remove?

    // 🔋 main type of all non-primitive objects
    //	data = 0xDADA, // address of a Node in linear memory
    //	dada = 0xDADA, // address of a Node in linear memory

    //	three3D = 0xDA3D, // binary encoded three dimensional object
    result_error = 0xE000, // different from well typed Nodes of kind 'error' and exceptions
    fointer = 0xF000, // see Addresses starting from 0x10000
    //	⚠️ fointers are combinatorial:
    fointer_of_int32 = 0xF07F, // int* etc
    //	fointer_of_int32 = 0xFA7F, // int[]* etc
    //	ffointer = 0xFF00, // pointer pointer
    //	ffointer_of_int32 = 0xFF7F, // int** etc
    // for 32 bit smart pointers, as used in return of int main(){}
    pad_to32_bit = 0xF0000000,
};


// e.g. array<int> or array<stringref> or struct<id> array<struct_id>
// ⚠️ 32 bit wasp generics condense to 16+ bit wasm types!
struct Generics32 /*32 bit*/ {
    ushort kind; // kind is padded to 32 bit so cant use directly
    ushort value_type;
};

struct Generics64 {
    Kind kind; // padded to 32 bit ok
    Primitive value_type;
};

typedef Generics32 Generics;

typedef int Address;


/*
 * i32 paged union Type
 * 0x00 - 0xFF zero page : Valtype / Kind
 * 0x100 - 0x1000 low page : reserved / Kind
 * 0x1000 - 0x10000 Todo Classes / pointer page ?
 * 0x10000 - string_header_32 pointer page: indirect types via address of describing Node{kind=clazz}
 * 0xAAAABBBB generic page: generic types (kind, value_type)
 *  e.g. person?[10] versus combinatorial primitive int[10]
 * string_header_32 - 0xFFFFFFFF high page reserved (combinatorial types!) Todo
 * check(sizeof(Type)==8) // otherwise all header structs fall apart
 * */
// for (wasm) function type signatures see Signature class!

//enum Valtype;// c++ forbids forward references to enums
// todo generic types Array<Type> … via type & array_mask
// todo complicated types Array<struct{…}> … via mask and index into types map à la  >>> ref(type_index) <<<
// todo ⚠️ 64 bit due to pointer!
union Type {
    // 64 bit due to pointer! todo: Type32 i32 union, 4 bytes with special ranges:
    /* this union is partitioned in the int space:
     0x0000 - Ill defined
     0x0001 - 0x1000 : Kinds of Node MUST NOT CLASH with Valtype!?
     0x1000 - 0x10000: Classes (just the builtin ones) smarty32 " todo 150_000 classes should be enough for anyone ;)"
     // todo memory offset ok? (data 0x1000 …)
// ⚠️ type.value >= 0x10000 means Generics! OR Pointer ??
     0x10000 - … 2^60    : Addresses (including pointers to any Node including Node{type=clazz}
     //todo endianness?

     some Classes might be composing:
     0xA001 array of Type 1 ( objects )
    */

    // todo: this union is BAD because we can not READ it safely, instead we need mapType / extractors for all combinations!
    unsigned int value = 0;
    //    uint64 value = 0;
    // one of:
    //    SmartPointer32 smarty;// when separating Types from values we don't need smart pointers
    Kind kind; // Node of this type
    Generics generics;
    //    Generics64 generics;
    Primitive type; // c_string int_array long_array float_array etc, can also be type of value.data in boxed Node
    Address address; // pointer to Node
    //    Node *clazz;// same as Address // 64 bit on normal systems!!!!
    //	Valtype valtype; // doesn't make sense here but try to avoid(guarantee?) overlap with type enum for future compatibility?

    Type() {
    }


    Type(Type *o) {
        value = o->value;
    }

    Type(Node *o) {
        if (!o) {
            this->kind = Kind::nils;
            return;
        }
        if (o == &DoubleType)
            this->kind = reals;
        else if (o == &LongType)
            this->kind = longs;
        else if (o == &IntegerType)
            this->type = wasm_int32;
        else if (o == &StringType)
            this->kind = strings;
            //        else if (mapType())
        else
            error("Type32(const Node &o)");
    }

    //
    //    Type32(const Node &o) {
    //        if(!o.empty())
    ////        error("TODO    Type32(const Node &o)"s+o.serialize());
    //        error("TODO    Type32(const Node &o)");
    //    }

    Type(unsigned int value) {
        this->value = value;
    }

    Type(Generics generics) {
        this->generics = generics;
    }

    //    Type(Generics32 generics) {
    //        this->generics = {(Kind)generics.kind, (Primitive) generics.value_type};
    //    }

    Type(Generics64 generics) {
        this->generics = {(ushort) generics.kind, (ushort) generics.value_type};
    }

    Type(Primitive primitive) {
        type = primitive;
    }

    Type(String &name) {
        Type mapType(String &arg, bool throws = false);
        *this = mapType(name, true); // throws if not found
    }


    Type(Kind kind) {
        this->kind = kind;
        if ((int) kind > 0x1000)
            error("erroneous or unsafe Type construction");
    }

    // explicit Type(const String &string) {
    //     Type mapType(const String &arg, bool throws = false);
    //     *this = mapType(string, true); // throws if not found
    // }

    explicit
    operator int() const { return this->value; }

    explicit
    operator Kind() const { return this->kind; }

    //
    explicit
    operator Generics() const { return this->generics; }

    //    explicit
    //    operator Generics64() const { return this->generics; }

    explicit
    operator Primitive() const { return this->type; }

    //    explicit
    //    operator Valtype() const { return this->type; }
    // c++ forbids forward references to enums
    //	Type(Valtype valtype){
    //		value = valtype;// transparent subset equivalence
    //	}

    explicit
    operator Node &() const {
        if (this->value < 0x1000)
            error("TODO mapTypeToNode");
#if WASM
        error("Unsafe wasm mapping Type to Node");
        return *(Node *) (void *) (int64) this->address;
#else
        error("Unknown mapping Type to Node");
#endif
        return *(Node *) (void *) (int64) this->address;
    }

    explicit operator const char *() const { return typeName(this); }

    bool operator==(Type other) const {
        return value == other.value;
    }

    bool operator==(Primitive other) const {
        return type == other;
    }

    //    bool operator==(uint64 other) {
    //        return value == other;
    //    }

    //    bool operator==(unsigned int other) {
    //        return value == other;
    //    }

    //    bool operator==(unsigned short other) {
    //        return value == other;
    //    }
    bool isGeneric() const {
        return value >= 0x10000; // todo generic BIT!
        //        type.value & generics_mask;// >=0x10000;
    }

    bool isArray() const {
        return kind == arrays or isGroup(kind) or (isGeneric() and isGroup((Kind) generics.kind));
    }
};

//if (type.value >= 0x10000) means Generics!
//typedef Type32 Type;
//typedef Type64 Type;

Type valueType(Type type);

Type genericType(Type type, Type value_type);


// on 64bit systems pointers (to types)
union Type64 {
    //  i64 union, 8 bytes with special ranges:
    int64 value = 0; // one of:
    SmartPointer64 smarty;
    Node *clazz; // same 64 bit on normal systems!!!!
    //     0x10000000_00000000 - 2^64 : SmartPointer64 ≈ SmartPointer32 + 4 byte value
};


enum Modifiers {
    final,
    constant,
    static_,
    public_,
    private_,
    protected_,
    virtual_,
    override_,
    abstract_,
    sealed_,
    synchronized_,
    transient_,
    volatile_,
    native_,
    internal_, // used in C# and Kotlin for internal classes and methods
    external_,
    //	undefined,
    missing,
    unknowns,
};

//#endif //MARK_NODETYPES_H

extern "C" chars kindName(::Kind t);

chars typeName(::Kind t, bool throws = true);

chars typeName(::Type t);

chars typeName(Primitive p);


// for Angle analyze / emitter . minimally more efficient than node.name=="if" …
//enum NodTypes {
//    numberLiteral,
//    identifier,
//    binaryExpression,
//    printStatement,
//    variableDeclaration,
//    variableAssignment,
//    functionDeclaration,
//    whileStatement,
//    ifStatement,
//    callStatement,
//    internalError,
//};


// in final stage of emit, keep original types as int64 as possible
Valtype mapTypeToWasm(Type t);

Valtype mapTypeToWasm(Node &n);

Primitive mapTypeToPrimitive(Node &n);

Valtype mapTypeToWasm(Primitive p);

Type mapType(String &arg, bool throws = false);

Type mapType(Node &arg);

//Type mapType(Node *arg,bool throws=false);
Type mapType(Node *arg);

chars typeName(Valtype t, bool fail = true);


bool isGeneric(Type type);

bool isArrayType(Type type); // ^^ or generic (wasm) array

void print(Kind k);

#ifndef  _READLINE_H_

class Function;

Type preEvaluateType(Node &node, Function *context0);

#endif
