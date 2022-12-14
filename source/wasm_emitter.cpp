#pragma clang diagnostic push
#pragma ide diagnostic ignored "misc-no-recursion"
//https://blog.sentry.io/2020/08/11/the-pain-of-debugging-webassembly
//https://developer.mozilla.org/en-US/docs/WebAssembly/Understanding_the_text_format
//https://github.com/mdn/webassembly-examples/tree/master/understanding-text-format
// https://github.com/ColinEberhardt/chasm/blob/master/src/emitter.ts
// https://github.com/ColinEberhardt/chasm/blob/master/src/encoding.ts
// https://pengowray.github.io/wasm-ops/
#include <cmath> // pow
#include "Wasp.h"
#include "String.h"
#include "Map.h"
#include "Code.h"
#include "wasm_emitter.h"
#include "wasm_helpers.h"
//#include "wasm_runner.h"
#include "wasm_reader.h"
#include "wasm_merger.h"
//#include "asserts.h"

//Map<String, Signature> functions;// todo Signature copy by value is broken
Code emitString(Node &node, Function &context);

Code emitArray(Node &node, Function &context);

//int runtime_data_offset = 0;// 0x10000;
int runtime_data_offset = 0x10000; // prevent memory clash with runtime.
int runtime_function_offset = 0; // imports + funcs
int import_count = 0;
short builtin_count = 0;// function_offset - import_count - runtime_offset;

//bytes data;// any data to be stored to wasm: values of variables, strings, nodes etc
char *data;// any data to be stored to wasm: values of variables, strings, nodes etc ( => memory in running app)
//int data_index_start = 0;
int data_index_end = 0;// position to write more data = end + length of data section
int last_object_pointer = 0;// outside stack
//int last_data_pointer = 0;// last_data plus header , see referenceDataIndices
//Map<String *, long> referenceDataIndices; // wasm pointers to strings within wasm data WITHOUT runtime offset!
// todo: put into Function.locals :
typedef int nodehash;
Map<long, int> referenceNodeIndices;// wasm pointers to nodes
Map<String, long> referenceIndices; // wasm pointers to objects (currently: arrays?) within wasm data
Map<String, long> referenceDataIndices; // wasm pointers directly to object data, redundant ^^ TODO REMOVE
Map<String, Node> referenceMap; // lookup types… todo: Node pointer? or copy ok?
Map<String, int> typeMap;// wasm type index for funcTypeSection. todo keep in Function
Map<String, int> functionIndices; // todo keep in Function
Map<String, Code> functionCodes; // todo keep in Function
//Map<String, Signature> functions;// for funcs AND imports, serialized differently (inline for imports and extra functype section)

//Map<long,int> dataIndices; // wasm pointers to strings etc (key: hash!)  within wasm data

//Map<String, Valtype> return_types;
//Map<int, List<String>> locals;
//Map<int, Map<int, String>> locals;
//List<String> declaredFunctions; only new functions that will get a Code block, no runtime/imports
//List<Function> imports;// from libraries. todo: these are inside functions<> for now!

Module runtime;
String start = "main";

Type arg_type = voids;// autocast if not int
//Type last_type = voids;// autocast if not int
Type last_type = Kind::unknown;
Node *last_object = 0;

enum MemoryHandling {
    import_memory,
    export_memory,
    internal_memory,// declare but don't export
    no_memory,
};
MemoryHandling memoryHandling;// set later = export_memory; // import_memory not with mergeMemorySections!

// todo: gather all 'private' headers here, not in between and in wasm_emitter.h
// private headers:
Code emitConstruct(Node &node, Function &context);

Code emitGetter(Node &node, Node &field, Function &context);

void discard(Code code) {}

void discard(Code &code) {
    // nop, to explicitly silence functions declared with 'nodiscard' attribute
}

[[nodiscard]]
Code Call(char *symbol);//Node* args


// https://pengowray.github.io/wasm-ops/
// values outside WASM ABI: 0=unknown/handled internally
// todo: norm operators before!
// * ∗ ⋅ ⋆ ✕ ×  …
byte opcodes(chars s, Valtype kind, Valtype previous = none) {
    //	previous is lhs in binops!

//	if(eq(s,"$1="))return set_local;
//	if (eq(s, "=$1"))return get_local;
//	if (eq(s, "=$1"))return tee_local;
    if (eq(s, "return"))return return_block;
    if (eq(s, "nop") or eq(s, "pass"))
        return nop;
    if ((Type) kind == unknown_type)
        error("unknown type should be inferred by now");
    if (kind == voids or kind == void_block or kind == i32t) { // INT32
        if (eq(s, "+"))return i32_add; // i32.add
        //		if (eq(s, "-") and previous==none)return sign_flip; *-1
        if (eq(s, "-"))return i32_sub; // i32.sub

        // todo: norm operators before!
        if (eq(s, "*"))return i32_mul; // i32.mul
        if (eq(s, "⋅"))return i32_mul; // i32.mul
        if (eq(s, "⋆"))return i32_mul; // i32.mul
        if (eq(s, "×"))return i32_mul; // i32.mul
        if (eq(s, "✕"))return i32_mul; // i32.mul
        if (eq(s, "∗"))return i32_mul; // i32.mul

        if (eq(s, "/"))return i32_div; // i32.div
        if (eq(s, "÷"))return i32_div; // i32.div
        if (eq(s, "%"))
            return i32_rem; // i32.rem
        if (eq(s, "=="))return i32_eq; // i32.eq
        if (eq(s, "eq"))return i32_eq; // i32.eq
        if (eq(s, "equals"))return i32_eq; // i32.eq
        if (eq(s, "is"))return i32_eq; // i32.eq // careful could be declaration := !
        if (eq(s, "!="))return i32_ne; // i32.ne
        if (eq(s, "≠"))return i32_ne; // i32.ne

        if (eq(s, ">"))return i32_gt; // i32.gt
        if (eq(s, "<"))return i32_lt; // i32.lt
        if (eq(s, ">="))return i32_ge; // i32.ge
        if (eq(s, "<="))return i32_le; // i32.le
        if (eq(s, "≥"))return i32_ge; // i32.ge
        if (eq(s, "≤"))return i32_le; // i32.le

        if (eq(s, "&"))return i32_and; // i32.and
        if (eq(s, "&"))return i32_and; // i32.and

        if (eq(s, "and"))return i32_and; // i32.and
        if (eq(s, "⋀"))return i32_and; // i32.and
        if (eq(s, "∧"))return i32_and; // i32.and // ∧≠^ potence looks like
        if (eq(s, "^"))return 0;// POWER handled on higher level

        if (eq(s, "or"))return i32_or; // i32.or
        if (eq(s, "∨"))return i32_or; // i32.or // looks like 'v' but isn't
        if (eq(s, "⋁"))return i32_or; // i32.or
        if (eq(s, "||"))return i32_or; // i32.or  // ≠ norm ‖
        if (eq(s, "|"))return i32_or; // i32.or // todo: pipe is different!

        if (eq(s, "xor"))return i32_xor; // i32.xor
        if (eq(s, "^|"))return i32_xor; // i32.xor //always bitwise todo: truthy 0x0101 xor 0x1010 !?
        if (eq(s, "⊻"))return i32_xor; // i32.xor

        if (eq(s, "not"))return i32_eqz; // i32.eqz  // no such thing as i32_not, but the same if you think about it
        if (eq(s, "¬"))return i32_eqz; // i32.eqz

    } else if (kind == i64t) { // INT32
        if (eq(s, "+"))return i64_add; // i64.add
        //		if (eq(s, "-") and previous==none)return sign_flip; *-1
        if (eq(s, "-"))return i64_sub; // i64.sub
        if (eq(s, "*"))return i64_mul; // i64.mul
        if (eq(s, "/"))return i64_div_s; // i64.di𝗏_s
        if (eq(s, "%"))
            return i64_rem_s; // i64.rem_s
        if (eq(s, "=="))return i64_eq; // i64.eq
        if (eq(s, "eq"))return i64_eq; // i64.eq
        if (eq(s, "equals"))return i64_eq; // i64.eq
        if (eq(s, "is"))return i64_eq; // i64.eq // careful could be declaration := !
        if (eq(s, "!="))return i64_ne; // i64.ne
        if (eq(s, "≠"))return i64_ne; // i64.ne
        if (eq(s, ">"))return i64_gt_s; // i64.gt_s
        if (eq(s, "<"))return i64_lt_s; // i64.lt_s
        if (eq(s, ">="))return i64_ge_s; // i64.ge_s
        if (eq(s, "<="))return i64_le_s; // i64.le_s
        if (eq(s, "≥"))return i64_ge_s; // i64.ge_s
        if (eq(s, "≤"))return i64_le_s; // i64.le_s

        if (eq(s, "&"))return i64_and; // i64.and
        if (eq(s, "&"))return i64_and; // i64.and

        if (eq(s, "and"))return i64_and; // i64.and
        if (eq(s, "⋀"))return i64_and; // i64.and
        if (eq(s, "∧"))return i64_and; // i64.and // ∧≠^ potence looks like
        if (eq(s, "^"))return 0;// POWER handled on higher level

        if (eq(s, "or"))return i64_or; // i64.or
        if (eq(s, "∨"))return i64_or; // i64.or // looks like 'v' but isn't
        if (eq(s, "⋁"))return i64_or; // i64.or
        if (eq(s, "||"))return i64_or; // i64.or  // ≠ norm ‖
        if (eq(s, "|"))return i64_or; // i64.or // todo: pipe is different!

        if (eq(s, "xor"))return i64_xor; // i64.xor
        if (eq(s, "^|"))return i64_xor; // i64.xor //always bitwise todo: truthy 0x0101 xor 0x1010 !?
        if (eq(s, "⊻"))return i64_xor; // i64.xor

        if (eq(s, "not"))return i64_eqz; // i64.eqz
        if (eq(s, "¬"))return i64_eqz; // i64.eqz
        if (eq(s, "!"))return i64_eqz; // i64.eqz
    } else if (kind == f64t) {
        if (eq(s, "not"))return f64_eqz; // f64.eqz  // HACK: no such thing!
        if (eq(s, "¬"))return f64_eqz; // f64.eqz  // HACK: no such thing!
        if (eq(s, "!"))return f64_eqz; // f64.eqz  // HACK: no such thing!
        if (eq(s, "+"))return f64_add; // f64.add
        if (eq(s, "-"))return f64_sub; // f64.sub
        if (eq(s, "*"))return f64_mul; // f64.mul
        if (eq(s, "/"))return f64_div; // f64.div
        if (eq(s, "=="))return f64_eq; // f64.eq
        if (eq(s, ">"))return f64_gt; // f64.gt
        if (eq(s, ">="))return f64_ge; // f64.ge
        if (eq(s, "<"))return f64_lt; // f64.lt
        if (eq(s, "<="))return f64_le; // f64.le
        if (eq(s, "√"))return f64_sqrt;
        if (eq(s, "sqrt"))return f64_sqrt;
        if (eq(s, "root"))return f64_sqrt;
        if (eq(s, "abs"))return f64_abs; // there is NO i32_abs
        if (eq(s, "‖"))return f64_abs; // ║  primitive norm operator ≠ || or
        if (eq(s, "║"))
            return f64_abs; // f32.abs // 10000000 comparisons for a char never encountered. Todo: 0 cost hash
    } else if (kind == f32t) {
        if (eq(s, "not"))return f32_eqz; // f32.eqz  // f32.eqz  // HACK: no such thing!
        if (eq(s, "¬"))return f32_eqz; // f32.eqz  // HACK: no such thing!
        if (eq(s, "!"))return f32_eqz; // f32.eqz  // HACK: no such thing!

        if (eq(s, "+"))return f32_add; // f32.add
        if (eq(s, "-"))return f32_sub; // f32.sub
        if (eq(s, "*"))return f32_mul; // f32.mul
        if (eq(s, "/"))return f32_div; // f32.div
        if (eq(s, "=="))return f32_eq; // f32.eq
        if (eq(s, ">"))return f32_gt; // f32.gt
        if (eq(s, ">="))return f32_ge; // f32.ge
        if (eq(s, "<"))return f32_lt; // f32.lt
        if (eq(s, "<="))return f32_le; // f32.le
    }
    if (eq(s, "√"))return f64_sqrt; // else √42*√42=41.9999
    if (eq(s, "√"))return f32_sqrt; // f32.sqrt
    if (eq(s, "sqrt"))return f32_sqrt; // f32.sqrt
    if (eq(s, "root"))return f32_sqrt; // f32.sqrt // conflicts with user keywords!
    // string addition etc handled elsewhere!
    if (eq(s, "-…"))return f32_neg; // f32.neg
    if (eq(s, "negate"))return f32_neg; // f32.neg
// the following functions force i32->f32
//	if (eq(s, "sqare root"))return f32_sqrt; // f32.sqrt

    if (eq(s, "abs"))return f32_abs; // f32.abs // there is NO i32_abs
    if (eq(s, "‖"))return f32_abs; // f32.abs // ║  primitive norm operator ≠ || or
    if (eq(s, "║"))return f32_abs; // f32.abs // 10000000 comparisons for a char never encountered. Todo: 0 cost hash


// rarely used and only clutters the namespace :(
// lol "⌊3.7⌋" is cursed and is transformed into \n\t or something in wasm and IDE!
    if (eq(s, "⌊"))return f32_floor; // f32.floor
    if (eq(s, "floor"))return f32_floor; // f32.floor // conflicts with user keywords!
    if (eq(s, "⌋"))return f32_floor; // f32.floor // vs trunc towards 0?
//if (eq(s, "round … down"))return f32_floor; // f32.floor

    if (eq(s, "⌈"))return f32_ceil; // f32.ceil
    if (eq(s, "ceil"))return f32_ceil; // f32.ceil
    if (eq(s, "⌉"))return f32_ceil; // f32.ceil

    if (eq(s, "⌊"))return f32_nearest; // f32.nearest
    if (eq(s, "round"))return f32_nearest; // f32.nearest // conflicts with user keywords!
    if (eq(s, "⌋"))return f32_nearest; // f32.nearest

// todo : peek 65536 as float directly via opcode
    if (eq(s, "peek"))return i64_load; // i64.load   // memory.peek memory.get memory.read
    if (eq(s, "poke"))return i64_store; // i64.store  // memory.poke memory.set memory.write

    // todo : set_local,  global_get ...
    if (eq(s, "$"))
        return get_local; // $0 $1 ...

    if (eq(s, "%"))return 0;// handle later

    if (tracing) breakpoint_helper
    trace("unknown or non-primitive operator %s\n"s % String(s));// OK! not (necessarily) a problem:
    // can still be matched as context etc, e.g.  2^n => pow(2,n)   'a'+'b' is 'ab'
//		error("invalid operator");
    return 0;
}

byte opcodes(chars s, Type kind, Type previous = none) {
    return opcodes(s, mapTypeToWasm(kind), mapTypeToWasm(previous));
}


// http://webassembly.github.io/spec/core/binary/modules.html#export-section
enum ExportType { // todo == ExternalKind
    func_export = (char) 0x00,
    table_export = 0x01,
    mem_export = 0x02,
    global_export = 0x03
};

// http://webassembly.github.io/spec/core/binary/types.html#context-types
char functionType = 0x60;

char emptyArray = 0x0;
typedef unsigned char byte;
// https://webassembly.github.io/spec/core/binary/modules.html#binary-module



// https://webassembly.github.io/spec/core/binary/modules.html#code-section
[[nodiscard]]
Code encodeLocal(long count, Valtype type) {
    return unsignedLEB128(count).addByte(type);
}

// https://webassembly.github.io/spec/core/binary/modules.html#sections
// sections are encoded by their type followed by their vector contents


Code emitNode(Node &node, Function &context);

bytes ieee754(float num) {
    char float_data[4];
    float *hack = ((float *) float_data);
    *hack = num;
    byte *flip = static_cast<byte *>(alloc(1, 5));
    short i = 4;
//	while (i--)flip[3 - i] = data[i];
    while (i--)flip[i] = float_data[i];// don't flip, just copy to malloc
    return flip;
}

bytes ieee754(double num) {
    char dat[8];
    double *hack = ((double *) dat);
    *hack = num;
    byte *flip = static_cast<byte *>(alloc(1, 9));
    short i = 8;
    while (i--)flip[i] = dat[i];// don't flip, just copy to malloc
    return flip;
}
//Code emitExpression (Node* nodes);

// pure data ready to be emitted
bool isProperList(Node &node) {
    if (node.kind != groups and node.kind != objects) return false;
    if (node.length < 1) return false;
    for (Node &child: node) {
        if (child.kind != longs and child.kind != strings)// todo … evaluate?
            return false;
        if (child.isSetter())
            return false;
    }
    return true;
}

// append padding bytes to wasm data memory
void emitPadding(int num, byte val = 0) {
    while (num-- > 0)
        data[data_index_end++] = val;
}

void emitPaddingAlignment(short size) { // e.g. 8 for long padding BEFORE emitLongData() / emitData(long)
    emitPadding((size - (data_index_end % size)) %
                size);// fill up to long padding ⚠️ the field-sizes before node.value MUST sum up to n*8!
}


// append byte to wasm data memory
void emitByteData(byte i) {
    *(byte *) (data + data_index_end++) = i;
}


// append short to wasm data memory
void emitShortData(short i, bool pad = false) {// ⚠️ DON'T PAD INSIDE STRUCTS!?
    if (pad)while (((long) (data + data_index_end) % 2))data_index_end++;// type 'int' requires 4 byte alignment
    *(short *) (data + data_index_end) = i;
    data_index_end += 2;
}

// append int to wasm data memory
void emitIntData(int i, bool pad = true) {
    if (pad)while (((long) (data + data_index_end) % 4))data_index_end++;// type 'int' requires 4 byte alignment
    *(int *) (data + data_index_end) = i;
    data_index_end += 4;
}

// append long to wasm data memory
void emitLongData(long i, bool pad = false) { // ⚠️ DON'T PAD INSIDE STRUCTS! pad before!
    if (pad)while (((long) (data + data_index_end) % 8))data_index_end++;// type 'long' requires 8 byte alignment
    *(long *) (data + data_index_end) = i;
    data_index_end += 8;
}

void emitSmartPointer(smart_pointer_64 p) {
    emitLongData(p, true);
}


Code emitWaspString(Node &node, Function &context) {
    // emit node as serialized wasp string
    const String &string = node.serialize();
    const Code &code = emitString(*new Node(string), context);
    return code;
}


//typedef long long wasm_node_index;
wasm_node_index emitNodeBinary(Node &node, Function &context) {
//    if((long)&node < 0x100000000L)
//        return (long)&node; // inside wasm stack context, we can just pass around the wasm_node_index. Todo, but not in compile context!
    long hash = node.hash();
    if (referenceNodeIndices.has(hash))
        return referenceNodeIndices[hash];
    else referenceNodeIndices[hash] = -1;// fixup marker for cyclic graphs todo …

//    if (node.value.longy != 0x01010101) {
//        node.meta = new Node("Hahaha");
//        node.meta->setValue(0x01010101);
//    }

    int wasm_type_pointer = node.type ? emitNodeBinary(*node.type, context) : 0;// just drop smart header
    int wasm_meta_pointer = node.meta ? emitNodeBinary(*node.meta, context) : 0;
//    int wasm_parent_pointer = 0; // reconstruct later
//    int wasm_next_pointer = 0;

    emitPaddingAlignment(8);
    List<wasm_node_index> children;
    for (auto child: node) {
        wasm_node_index child_index = emitNodeBinary(child, context);
        children.add(child_index);
    }

    emitPaddingAlignment(8);
    int node_children_pointer = -1; // ignore children (debug)
    if (node.length > 0) {
//        emitLongData(0x5741535044415441L, false);
        node_children_pointer = data_index_end;
//        emitLongData(0xAA00aa00aa00, false);
        for (auto child: children)
            emitIntData(child);
//        emitLongData(0xFFEEDDCCBBAA9988L, false);
//        emitLongData(0x5741535044415441L, false);
//    emitLongData(0xFFEEDDCCBBAA9988L, true); // chaos monkey! randomly insert to check sanity
        emitPaddingAlignment(8);
    }
//    emitPadding(1);// wrong padding DOES fuck up struct parsing even on the host side!
    int node_start = data_index_end;
    referenceNodeIndices[hash] = node_start;

    emitIntData(node_header_32, false);
    emitIntData(node.length, false);
    emitIntData(wasm_type_pointer, false);
    emitIntData(node_children_pointer, false); // todo: we COULD write them directly behind all other fields!
//    node.value.longy = 0xFFEEDDCCBBAA9988L;// debug
    emitLongData(node.value.longy, false);// too late to pad, otherwise
//    check_is(sizeof(node.kind), 1) // todo
//    emitByteData(node.kind); // breaks alignment
    emitIntData(node.kind);
    check_is(sizeof(node.kind), 4) // forced 32 bit for alignment!
    emitIntData(wasm_meta_pointer);
    emitString(node /*.name*/, context);// directly in place!
//    emitIntData(wasm_meta_pointer);
//    emitIntData(wasm_next_pointer);
//    emitPadding(3*8);// pointers, hash, capacity, … extra fields
//    emitPaddingAlignment(8);
    last_type = Primitive::node;
    if (node.type)
        last_type = node.type;
    last_object = &node;
    last_object_pointer = node_start;
    printf("node_start %d data_index_end %d\n", node_start, data_index_end);
// already stored in emitArray() : usually enough, unless we want extra node meta data?
//    referenceIndices.insert_or_assign(node.name, pointer);
//    referenceDataIndices.insert_or_assign(node.name, pointer + array_header_length);
//    referenceMap[node.name] = node;


    return node_start;
}


[[nodiscard]]
Code emitNode(Node &node, Function &context) {
    return Code().addConst32(emitNodeBinary(node, context));
}


[[nodiscard]]
Code emitPrimitiveArray(Node &node, Function &context) {
    // todo emit with some __DATA__ header and or base64
    let code = Code();
    if (!(node.kind == buffers)) todo("arrays of type "s + typeName(node.kind));
    emitIntData(buffer_header_32, false); // array header
    // todo LEB variant
//    int length = *(int *) node.value.data;// very fragile :(
//    emitIntData(length, false);
//	emitIntData(array_header_32 | int_array << 4 | node.length);
//	emitIntData(node.kind |  node.length); // save 4 bytes, rlly?
    //	if pure_array:
    int pointer = data_index_end; // return pure data
    if (!node.meta or (*node.meta)["length"].kind != longs)
        warn("buffer length should be stored in meta, not in node.length, for safety!");
    int length = node.length;
    memcpy(data + data_index_end, node.value.data, length);
#if MULTI_VALUE
    // and RETURN
//        code.addConst(array_header_32 | node.length);// combined smart pointer? nah
    code.addConst32(node.length);
    //	code.addConst32(array_header_32);
#endif
// the last element of the stack is what is returned if no MULTI_VALUE
// the return statement makes drops superfluous and just takes as many elements from stack as needed (and ignores/drops the rest!)
    code.addConst32(pointer);
    last_type = mapTypeToWasm(node);
    return code;
}

short arrayElementSize(Node &node);

void addTypeFromSize(Node &array, short size);

[[nodiscard]]
// todo emitPrimitiveArray vs just emitNode as it is (with child*)
Code emitArray(Node &node, Function &context) {
//	if (node.kind.type == int_array)


    if (node.kind == buffers)
        return emitPrimitiveArray(node, context);
    // ⚠️careful primitive array (buffer) ≠ array of primitives, which follows …

    List<wasm_node_index> children;
    Kind value_kind = node.first().kind;
    for (Node &child: node) {
        if (not isPrimitive(child)) {
            warn("non primitive element forces node emission");
            return Code((long) emitNodeBinary(node, context), false);
        }
        if (child.kind != value_kind) {
            // todo try coherence lifting e.g. [1 'a' 'ü'] => codepoint[]
            warn("non coherent element forces node emission");
            warn("collection kind %s versus element kind %s "s % typeName(value_kind) % typeName(child.kind));
            return Code((long) emitNodeBinary(node, context), false);
        }
        children.add(child.value.longy);
    }

//    Primitive smallest_common_type = ::byte_char;// todo bit for bitvectors?
//    short itemSize=0;// bit vector
    short itemSize = arrayElementSize(node);
    wasm_node_index typ = 0;
    if (!node.type) /*node.type=*/addTypeFromSize(node, itemSize);
    if (node.type) typ = emitNodeBinary(*node.type, context);// danger! Byte now lives inside wasm!

    let code = Code();
    emitPaddingAlignment(8);
    int pointer = data_index_end;
//	todo: sync with emitOffset
    emitIntData(array_header_32, false);
//    todo ⚠️really lose information here? use emitNodeBinary if full representation required
    emitIntData(node.kind, false);// useless, always groups patterns or objects, who cares now?
//    emitIntData(value_kind, false); // only works in homogenous arrays!
    emitIntData(node.length, false);
//    emitIntData(stack_Item_Size, false);// reduntant via type
    if (node.type) emitIntData(typ);// or node_header_32
    else emitIntData(value_kind /*or kind_header_32*/);// todo make sure node.type > Kind AS PER Type enum

    bool continuous = true;
    if (!continuous) emitIntData(data_index_end + 4); // just emit immediately after

//    for(wasm_node_index i:children){
    for (Node &child: node) {
        // ok we checked for coherence before
        int64_t i = child.value.longy;
        if (itemSize == 1)emitByteData(i);
        else if (itemSize == 2)emitShortData(i);
        else if (itemSize == 4)emitIntData(i);
        else if (itemSize == 8)emitLongData(i);// ok can even be float, UNINTERPRETED here
    }

//    last_value_pointer = data_index_end;
//	assert_equals((long) data_index_end, (long) pointer + array_header_length);
    Code ignore;

    String ref = node.name;
    if (node.name.empty() and node.parent) {
        ref = node.parent->name;
    }
    if (not node.name.empty()) {
        // todo: consistently point to data or to wrapper 'object'?
        referenceIndices.insert_or_assign(ref, pointer);
        referenceDataIndices.insert_or_assign(ref, pointer + array_header_length);
        referenceMap[ref] = node;
    }
//	code.add(emitData(Node(0), context));// terminate list with 0.
    last_object_pointer = pointer;
    last_object = &node;
    last_type = array;
    code.addConst32(pointer);// base for future index getter/setter [0] #1
    return code;// pointer
//	return code.addConst(pointer);// once written to data section, we also want to use it immediately
}

// todo better
void addTypeFromSize(Node &array, short size) {
    Kind kind = array.first().kind;
//    kind= smallestCommonType(node)
    if (size == 1 and kind == longs)array.type = &ByteType;
    else if (size == 1 and kind == Kind::codepoints)array.type = &ByteChar;
    else if (size == 2)array.type = &ShortType;
    else if (size == 4 and kind == longs)array.type = &Int;
    else if (size == 4 and kind == Kind::codepoints)array.type = &Charpoint;
    else if (size == 8 and kind == longs) array.type = &Long;
    else if (size == 8 and kind == reals) array.type = &Double;
    else warn("can't infer type from size "s + size);
}

Node *smallestCommonType(Node &array) {
    todo("smallestCommonType");
}

// premature optimization BAD! but its so easy;)
// adds type as BAD SIDE EFFECT
short arrayElementSize(Node &node) {
    if (node.type)
        return stackItemSize(*node.type);
    short smallestCommonitemSize = 1;// byte
    for (Node &child: node) {
        if (child.kind == reference)
            return 4;// 8; // or 4 for wasm_node_index  can't be bigger, also don't change type!
        if (!isPrimitive(child))
            return 4;// wasm pointer
//            error("shouldn't come here:\n!isPrimitive(child)\n"s + child.serialize());
        if (child.kind == bools)continue;// can't be smaller
        if (child.kind == reals) {
            return 8; // can't be bigger, also don't change type!
        }
        if (child.kind == codepoints) {
            if (child.value.longy >= 0x80)
                return 8; // can't be bigger, also don't change type!
            else
                continue; // compress ascii
        }
        if (child.type) {
            Valtype valtype = mapTypeToWasm(child.type);
            smallestCommonitemSize = maxi(smallestCommonitemSize, stackItemSize(valtype));
//            todo("child.type comparison");
        } else {
            unsigned long long val = abs(child.value.longy) * 2;// *2 for signed variants
//        if(val<=1) bit vector
//        if(val>1)itemSize = maxi(itemSize,1); // byte
            if (val >= 0x100)smallestCommonitemSize = maxi(smallestCommonitemSize, 2);// short
            else if (val >= 0x10000)smallestCommonitemSize = maxi(smallestCommonitemSize, 4);// ints
            else if (val >= 0x100000000) {
                smallestCommonitemSize = maxi(smallestCommonitemSize, 8);// longs
                break;// can't be bigger
            }
        }
    }
    if (node.type) return smallestCommonitemSize;
//    else set type!
    addTypeFromSize(node, smallestCommonitemSize);
    return smallestCommonitemSize;
}


bool isAssignable(Node &node, Node *type = 0) {
    //todo
    //	if(node.metas().has("constant") or node.metas().has("immutable")) // …
    //		return false;
    //	if(type and type!=node.type)
    //		return false;
    return true;
}

int currentStackItemSize(Node &array, Function &context) {
    if (array.type)
        return stackItemSize(*array.type);
    else if (array.kind == groups or array.kind == objects or array.kind == patterns)
        return arrayElementSize(array);// todo why AGAIN? why is type lost after emitArray?
    if (array.kind == reference) {
        Local &local = context.locals[array.name];
        // todo: fix up local elsewhere!
        if (not local.ref)
            local.ref = &referenceMap[array.name];
        if (local.ref and not local.type)
            local.type = local.ref->type;
        if (local.type)
            return stackItemSize(*local.type); // or
        return stackItemSize(local.typo);
    }
    if (array.kind == strings) return 1;// char for now todo: call String.codepointAt()
//    if (last_object) CAN BE PATTERN, not array!
//        return stackItemSize(*last_object);
    if (stackItemSize(last_type, false))
        return stackItemSize(last_type);
    error("unknown size for stack item "s + array.string());
    return 1;
}

int headerOffset(Node &array) {
    if (array == NUL)
        return array_header_length;
//        error("how am I supposed to know the headerOffset??");
    switch (array.kind) {
        case objects:
        case arrays:
        case groups:
            return array_header_length;
        default:
            return 0;
    }
}

[[nodiscard]]
Code
emitOffset(Node &array, Node offset_pattern, bool sharp, Function &context, int size, int base, bool base_on_stack) {
    Code code;
//    if(base_on_stack and base>0)
//        base=0;//error("double base");
    if (base_on_stack)
        base = 0;// headerOffset(array);// reuse for additional header!

    if (offset_pattern.kind == patterns) {
        if (sharp == true)error("sharp pattern?");
        if (offset_pattern.length == 1) {
            offset_pattern = offset_pattern.first();
        } else todo("complex range patterns");
    }
    if (offset_pattern.kind == reference) {
        const Code &offset = emitExpression(offset_pattern, context);
        code.add(offset);
        if (last_type == i64)
            code.add(cast(last_type, i32));
        if (last_type != i32t)
            error("index must be of int type");
        if (sharp) {// todo remove if offset_pattern was static
            code.addConst32(1);
            code.add(i32_sub);
        }
        if (size > 1) {
            code.addConst32(size);
            code.add(i32_mul);
        }
        if (base > 0) { // after calculating offset!
            code.addConst32(base);
            code.add(i32_add);
        }
    } else if (offset_pattern.kind == longs) {
        int offset = (int) offset_pattern.value.longy;
        if (offset < 1 and sharp)
            error("operator # starts from 1, use [] for zero-indexing");
        if (sharp) offset--;
//        if (base <= 0)// base may already be provided :(
//            base += headerOffset(array);
        // todo: get string size, array length etc 1. at compiletime or 2. at runtime
        // todo: sanity checks 1. at compiletime or 2. at runtime
        //	if(offset_pattern>op[0].length)e
        //	rror("index out of bounds! %d > %d in %s (%s)"s % offset_pattern % );
        code.addConst32(base + offset * size);
    } else todo("Internal reference declaration or index operator bug");
    if (base_on_stack)// and base>0)
        code.add(i32_add);
    return code;
    // calculated offset_pattern of 0 ususally points to ~6 bytes after Contents of section Data header 0100 4100 0b08
}

[[nodiscard]]
Code emitIndexWrite(Node &array, int base, Node offset, Node value0, Function &context) {
    int size = currentStackItemSize(array, context);
    Type targetType = last_type;
//    Valtype targetType = mapTypeToWasm(last_type);
    Code store = emitOffset(array, offset, true, context, size, base, false);

    if (value0.kind == strings)//todo stringcopy? currently just one char "abc"#2=B
    {
        char c = value0.value.string->charAt(0);
        store.addConst32(c);
    } else
        store = store + emitValue(value0, context);
//	store.add(cast(last_type, valType));
//	store.add(cast(valType, targetType));
    store.add(cast(last_type, targetType));
    store.addByte(nop);// reloc padding
    store.addByte(nop);
    store.addByte(nop);
    store.addByte(nop);
    if (size == 1)store.add(i8_store);
    if (size == 2)store.add(i16_store);
    if (size == 4)store.add(i32_store);
    if (size == 8)store.add(i64_store);
    //	The static address offset is added to the dynamic address operand
    store.add(size > 2 ? 0x02 : 0);// alignment (?) "alignment must not be larger than natural"
    store.add(0);// extra offset (why, wasm?)
//	store.add(base);// extra offset (why, wasm?)
    store.addByte(nop);// reloc padding
    store.addByte(nop);
    store.addByte(nop);
    store.addByte(nop);

    return store;
/*  000101: 41 94 08                   | i32.const 1044
	000104: 41 06                      | i32.const 6
    000106: 36 02 00                   | i32.store 2 0 */
}


// "hi"[0]="H"
[[nodiscard]]
Code emitIndexWrite(Node &op, Function &context) {// todo offset - 1 when called via #!
    return emitIndexWrite(op["array"], 0, op["offset"], op["value"], context);
}

// "hi"#1='H'
[[nodiscard]]
Code emitPatternSetter(Node &ref, Node offset, Node value, Function &context) {
    String &variable = ref.name;

    if (!context.locals.has(variable))
        error("!! Variable missed by analyzer: "_s + variable);

    Local &local = context.locals[variable];
    last_type = local.typo;
    int base = 0;
    if (local.data_pointer)
        base = local.data_pointer;
    if (referenceIndices.has(variable)) {
        base = referenceDataIndices[variable];// todo?
        ref = referenceMap[variable];// ref to value!
    }
    if (referenceDataIndices.has(variable))
        base = referenceDataIndices[variable];// todo?
//	last_type = mapTypeToWasm(value);
    Code code = emitIndexWrite(ref, base, offset, value, context);
    return code;
}


// assumes value is on top of stack
// todo merge with emitIndexRead !
[[nodiscard]]
Code emitIndexPattern(Node &array, Node &op, Function &context, bool base_on_stack) {
    if (op.kind != patterns and op.kind != longs and op.kind != reference)error("op expected in emitIndexPattern");
    if (op.length == 0 and op.kind == reference)return emitGetter(array, op, context);
    if (op.length != 1 and op.kind != longs)error("exactly one op expected in emitIndexPattern");
    int base = base_on_stack ? 0 : last_object_pointer + headerOffset(array);// emitting directly without reference
    int size = currentStackItemSize(array, context);
    Node &pattern = op.first();
    Code load = emitOffset(array, pattern, op.name == "#", context, size, base, base_on_stack);
    load.addByte(nop);// reloc padding
    load.addByte(nop);
    load.addByte(nop);
    load.addByte(nop);
    if (size == 1)load.add(i8_load);// i32.load8_u
    if (size == 2)load.add(i16_load);
    if (size == 4)load.add(i32_load);
    if (size == 8)load.add(i64_load);
    // memarg offset u32 align u32 DOESNT FIT:!?!
    load.add(size > 2 ? 0x02 : 0);// alignment (?)
    load.add(0x00);// ?
    load.addByte(nop);
    load.addByte(nop);
    load.addByte(nop);
    load.addByte(nop);

    // careful could also be uint8!
    if (size == 1) {
        last_type = byte_char;// last_type = codepoint32;// todo and … bytes not exposed in ABI, so OK?
        last_type = int32; // even for bool!
    } else if (size <= 4)last_type = int32;
    else last_type = i64;
//	if(op.kind==reference){
//		Node &reference = referenceMap[op.name];
//		if(reference.type){
//			last_typo.clazz = reference.type;
//		}else if(reference.kind==strings)
//			last_typo=
//			else
//			last_typo=reference.kind;
//	}

    return load;
}

// todo: merge
// emitIndexPattern assumes value is on top of stack
// emitIndexRead prints(??) value/ref  on top of stack
[[nodiscard]]
Code emitIndexRead(Node &op, Function &context, bool base_on_stack, bool offset_on_stack) {
    if (op.length < 2)
        error("index operator needs two arguments: node/array/reference and position");
    Node &array = op[0];// also String: byte array or codepoint array todo
    Node &pattern = op[1];
    int size = currentStackItemSize(array, context);
//	if(op[0].kind==strings) todo?
    last_type = arg_type;
    int base; // …
    if (array.kind == reference or array.kind == key) {
        String &ref = array.name;
//		last_type=array.data_kind;
        if (referenceIndices.has(ref))// also to strings
            base = referenceDataIndices[ref];
//		else if (stringIndices.has(&ref))
//			base += referenceIndices[ref];
        else
            error("reference not declared as array type: "s + ref);
        if (referenceMap.has(ref)) {
            Node &reference = referenceMap[ref];
            array = reference;
//			if(reference.type)
//			last_type = mapTypeToWasm(*reference.type);
////			last_typo.clazz
        }
//		else error("reference should be mapped");

    } else if (array.kind == strings) {
        String *string = array.value.string;
        if (string)
            base = referenceDataIndices[*string];
    } else {
        if (not base_on_stack) todo("reference array (again)");
        base = last_object_pointer;// + headerOffset(array);// todo: pray!
    }

    Code load;
    if (offset_on_stack and base_on_stack) {// offset never precalculated, RIGHT?
        if (op.name == "#") {// todo: this is stupid code duplication of emitOffset!
            if (pattern.kind == longs and pattern.value.longy < 1)
                error("operator # starts from 1, use [] for zero-indexing");
            load.addConst32(1);
            load.addByte(i32_sub);
        }
        if (size > 1) {
            load.addConst32(size);
            load.addByte(i32_mul);
        }
        load.addByte(i32_add);
        auto headerOffset1 = headerOffset(array);
        if (headerOffset1 > 0) {
            load.addConst32(headerOffset1);
            load.addByte(i32_add);
        }
    } else {
        error("why not on stack?");
        load = load + emitOffset(array, pattern, op.name == "#", context, size, base, base_on_stack);
    }
    load.addByte(nop);
    load.addByte(nop);// reloc padding
    load.addByte(nop);
    load.addByte(nop);
    if (size == 1)load.add(i8_load);
    if (size == 2)load.add(i16_load);
    if (size == 4)load.add(i32_load);
    if (size == 8)load.add(i64_load);
    load.add(size > 2 ? 0x02 : 0);// alignment (?)
    load.add(0x00);// ?
    load.addByte(nop);// reloc padding
    load.addByte(nop);
    load.addByte(nop);
    load.addByte(nop);

//	if (size == 1)last_type = codepoint32;// todo only if accessing codepoints, not when pointing into UTF8 byte!!
    if (size == 1) {
        last_type = byte_char;
        last_type = int32;// ! even bool is represented as int in wasm!!!
    }   //last_typo = byte_char;
    else if (size <= 4)last_type = int32;
    else last_type = i64;
    return load;
    //	i32.const 1028
    //	i32.const 3
    //	i32.load
}


// write data to DATA SEGMENT (vs emitValue on stack)
// MAY return const(pointer)
[[nodiscard]]
Code emitData(Node &node, Function &context) {
    String &name = node.name;
    Code code;// POINTER to DATA SEGMENT
    int last_pointer = data_index_end;
    switch (node.kind) {
        case nils:// also 0, false
        case bools:
//			error("reuse constants for nil/true/false");
        case longs:

            // todo: add header?
            // todo: wasteful? but compare to boxed values NOT wasteful!
            // todo: add smart-pointer header?
//			if(leb)
//			Code &leb128 = signedLEB128(node.value.longy);
//			data_index_end+=leb128.length
// todo: this dynamic emit is NOT COMPATIBLE with array emission with static element size!
//            if (abs(node.value.longy<0x80)
//                    *(byte *) (data + data_index_end) = node.value.longy;
//            else
            if (node.value.longy > 0xF0000000) {
                error("true long big ints currently not supported");
                *(long *) (data + data_index_end) = node.value.longy;
                data_index_end += 8;
                last_type = i64t;
            } else {
                *(int *) (data + data_index_end) = node.value.longy;
                data_index_end += 4;
                last_type = int32;
            }
            break;
        case reals:
//			bytes varInt = ieee754(node.value.real);
            *(double *) (data + data_index_end) = node.value.real;
            data_index_end += 8;
            last_type = float64;
            break;
        case reference:
            if (context.locals.has(name))
                return emitNode(node, context);
//                error("locals dont belong in emitData!");
            else if (referenceIndices.has(name)) todo("emitData reference makes no sense? "s + name)
            else
                error("can't save unknown reference pointer "s + name);
            break;
        case strings:
            return emitString(node, context);
        case objects:
        case groups:
            // todo hold up: print("ok") should not emit an array of group(string(ok)) !?
            // keep last_type from last child, later if mixed types, last_type=ref or smarty
            return emitArray(node, context);
            break;
        case constructor:
            return emitConstruct(node, context);
        case key:
        case patterns:
        default:
            error("emitData unknown type: "s + typeName(node.kind));
    }
    // todo: ambiguity emit("1;'a'") => result is pointer to [1,'a'] or 'a' ? should be 'a', but why?
    last_object_pointer = last_pointer;
    return code.addConst32(last_pointer);// redundant with returns ok
}

Code emitString(Node &node, Function &context) {
    if (node.kind != strings)
        return emitString((new Node(node.name))->setType(strings), context);
    if (not node.value.string)
        error("empty node.value.string");
//    emitPadding(data_index_end % 4);// pad to int size, too late if in node struct!
    int last_pointer = data_index_end + runtime.data_offset_end;
    String &string = *node.value.string;
    referenceMap[string] = node;
    if (string and referenceIndices.has(string)) {
        // todo: reuse same strings even if different pointer, aor make same pointer before
        last_object_pointer = referenceIndices[string];
        return Code().addConst32(last_object_pointer);
    }
    bool as_c_io_vector = true;
    if ((Primitive) node.kind == leb_string) {
        Code lens(string.length);// wasm abi to encode string as LEB-length + data:
        strcpy2(data + data_index_end, (char *) lens.data, lens.length);
        data_index_end += lens.length;// unsignedLEB128 encoded length of pString
        // strcpy2 the string later: …
    } else if (as_c_io_vector) { // wasp abi:
        emitIntData(data_index_end + 8, false);// char* for ciov, redundant but also acts as checksum
        emitIntData(string.length, false);
    } else { // wasp abi:

        emitIntData(string_header_32, false);
        emitIntData(data_index_end + 20, false);
        emitIntData(string.length, false);
        emitIntData(1, false);// iovs len?
//        emitIntData(string.codepoint_count, false);// type + child_pointer in node
        emitLongData(data_index_end + 8, false);// POINTER to char[] which just follows:
    }
    int chars_start = data_index_end;
    // the actual string content:
    strcpy2(data + data_index_end, string.data, string.length);
    data[data_index_end + string.length] = 0;
    // we add an extra 0, unlike normal wasm abi, because we have space in data section
    referenceDataIndices.insert_or_assign(string, data_index_end);
    // todo this is EVIL: we assign the STRING to data_index_end, not the REFERENCE!!!

    data_index_end += string.length + 1;
    last_type = stringp;
    last_object_pointer = last_pointer;
    return Code().addConst32(chars_start);// direct data!
}

[[nodiscard]]
Code emitGetGlobal(Node &node /* context : global ;) */) {
    Code code;
    String &name = node.first().name;
    int i = globals.position(name);
    if (i < 0)error("cant find global with name "s + name);
    if (i > globals.size())error("invalid index for global with name "s + name);
    code.addByte(global_get);
    code.addByte(i);
    last_type = globalTypes.values[i];
    return code;
}

// print value on stack (vs emitData)
// todo: last_type not enough if operator left≠right, e.g. ['a']#1  2.3 == 4 ?
[[nodiscard]]
Code emitValue(Node &node, Function &context) {
    Code code;
    String &name = node.name;
    if (node.value.node)
        last_type = node.kind;// careful Kind::strings should map to Classes::c_string after emission!
    last_object = &node;// careful, could be on stack!
    // code.push(last_typo) only on return statement
    switch (node.kind) {
        case nils:// also 0, false
            code.addByte((byte) i32_auto);// nil is pointer
            code.push((short) 0);
            break;
        case bools:
//		case ints:
            code.addByte((byte) i32_auto);
            code.push(node.value.longy);// LEB encoded!
            last_type = i32t;
            break;
        case longs:
            // todo: ints vs longs!!!
            if (node.value.longy <= 0x10000000 and node.value.longy > -0x100000000) { // room for smart pointers
                last_type = i32t;
                code.addByte((byte) i32_const);
            } else {
                last_type = i64t;
                code.addByte((byte) i64_const);
            }
            code.push(node.value.longy);
            break;
//		case floats:
//			last_type = f32t;// auto cast return!
//			code.addByte((byte) f32_const);
//			code.push(ieee754((float)node.value.real), 4);
        case reals:
            last_type = f64t;// auto cast return!
            code.addByte((byte) f64_const);
            code.push(ieee754(node.value.real), 8);
//			code.push(ieee754(node.value.real), 8);
            break;
//		case identifier:
        case reference: {
            if (!context.locals.has(name) and not globals.has(name))
                error("UNKNOWN symbol "s + name + " in context " + context);
            if (node.value.node) {
                Node &value = *node.value.node;
                warn("HOLUP! x:42 is a reference? then *node.value.node makes no sense!!!");// todo FIX!!
                code.add(emitSetter(node, value, context));

            } else {
                code.addByte(get_local);// todo skip repeats
                code.addByte(context.locals[name].position);// base location stored in variable!
                if (node.length > 0) {
                    return emitIndexPattern(NUL, node, context, true);// todo?
                }
            }
        }
            break;
//			case binaryExpression:
//				code.opcode(binaryOpcode[node.value]);
//				break;
        case operators:
            warn("operators should never be emitted as values");
            return emitExpression(node, context);
        case strings: {
            // append pString (as char*) to data section and access via stringIndex
            if (!node.value.string)error("missing node.value.string");
            last_object_pointer = data_index_end + runtime.data_offset_end;// uh, todo?
            String string = *node.value.string;
            if (referenceDataIndices.has(string))
                // todo: reuse same strings even if different pointer, aor make same pointer before
                last_object_pointer = referenceDataIndices[string];
            else {
                emitString(node, context);
//                referenceDataIndices.insert_or_assign(string, data_index_end);
//				Code lens(pString->length);// we follow the standard wasm abi to encode pString as LEB-lenght + data:
//				strcpy2(data + data_index_end, (char*)lens.data, lens.length);
//				data_index_end += lens.length;// unsignedLEB128 encoded length of pString
//                strcpy2(data + data_index_end, string.data, string.length);
//                data[data_index_end + string.length] = 0;
//                data_index_end += string.length + 1;

                if (referenceIndices.has(name)) {
                    if (not isAssignable(node))
                        error("can't reassign reference "s + name);
                    else trace("reassigning reference "s + name + " to " + node.value.string);
                }
                if (node.parent and (node.parent->kind == reference or node.parent->kind == key)) {
                    // todo move up! todo key bad criterion!!
                    // todo: add header or copy WHOLE string object!
                    referenceIndices.insert_or_assign(node.parent->name, last_object_pointer);// safe ref to string
                    referenceDataIndices.insert_or_assign(node.parent->name,
                                                          last_object_pointer + 8);// safe ref to string
                    referenceMap.insert_or_assign(node.parent->name, node); // lookup types, array length …
                }

                // we add an extra 0, unlike normal wasm abi, because we have space in data section
            }
            last_type = charp;
            code = Code(i32_const) + Code(last_object_pointer + 8);// just a pointer to DATA
            if (node.length > 0) {
                if (node.length > 1)error("only 1 op allowed");
                Node &pattern = node.first();
                if (pattern.kind != patterns and pattern.kind != longs)
                    error("only patterns allowed on string");
                code.add(emitIndexPattern(NUL, pattern, context, false));
            }
            return code;
//			return Code(stringIndex).addInt(pString->length);// pointer + length
        }
        case key:
            if (node.value.node) {
                Node &value = *node.value.node;
                return emitValue(value, context);// todo: make sure it is called from right context (after isSetter …)
            }
        case patterns:
            return emitIndexPattern(NUL, node, context, false);// todo: make sure to have something indexable on stack!
        case expression: {
            Node values = node.values();// remove setter part
            return emitExpression(values, context);
        }
//			error("expression should not be print on stack (yet) (maybe serialize later)")
        case call:
            return emitCall(node, context);// yep should give a value ok
        case groups: // (1 2 3)
        case objects: // { 1 2 3}
            return emitData(node, context);// todo: could be closure, node, … !!
        default:
            error("emitValue unknown type: "s + typeName(node.kind));
    }
    if (node.type) {
        if (node.type->kind != flags)
            code.add(cast(node, *node.type, context));
    }
    // code.push(last_typo) only on return statement
    return code;
}


[[nodiscard]]
Code emitAttributeSetter(Node &node, Function &context) {
    Node *value = node.value.node;
    if (!value)value = node.last().value.node;
    if (!value)error("attribute setter missing value");
    todo("emitAttributeSetter");
//    return Code();
}


Code emitGetter(Node &node, Node &field, Function &context) {
    Code code;
    // todo we could match the field to data at compile time in some situations, but let's implement the general case first:
    if (node.kind == reference and node.length > 0) // else loop!
        return emitIndexPattern(node, field, context, false);
//        code.add(emitValue(node, context));
    else todo("get pointer of node on stack");
    if (field.kind == strings)
        code.add(emitString(field, context));
    else
        code.add(emitString(*new Node(field.name), context));

    Function &getField = functions["getField"];
    code.addByte(function_call);
    code.addInt(getField.code_index);
    return code;
}

// todo ⚠️ discern get / set / call attribute! x.y x.y=z x.y()
[[nodiscard]]
Code emitAttribute(Node &node, Function &context) {
    if (node.length < 2)error("need object and field");
    if (node.length > 3)error("extra attribute stuff");
    if (node.value.node or node.last().value.node)
        return emitAttributeSetter(node, context);// danger get->set ?
    Node field = node.last();// NOT ref, we resolve it to index!
    Node &object = node.first();

    if (!object.type and types.has(object.name))
        object.type = types[object.name];

    if (object.type) {
        Node type = *object.type;
        if (!type.has(field.name))
            error("field %s missing in type %s of %s "s % field.name % type.name % node.serialize());
        Node &member = type[field.name];
        int index = member.value.longy;
        if (member.kind == fields)// todo directly
            index = member["position"].value.longy;
        if (index < 0 or index > type.length)
            error("invalid field index %d of %s in type %s of %s "s % index % field.name % type.name %
                  node.serialize());
        field = Node(index);
    } else
        emitGetter(object, field, context);

    Code code = emitData(object, context);
    // base pointer on stack
    // todo move => once done
// a.b and a[b] are equivalent in angle
    return code + emitIndexPattern(object, field, context, true);// emitIndexRead
}


[[nodiscard]]
Code emitOperator(Node &node, Function &context) {
    Code code;
    String &name = node.name;
//	name = normOperator(name);
    if (node.name == "nop" or node.name == "pass")return code;
    if (node.length == 0 and name == "=") return code;// BUG
    int index = functionIndices.position(name);
    if (context.code_index < 0)context.code_index = index;// tdoo remove cluch
    if (index < 0)index = context.code_index;
    check_eq(index, context.code_index);
    if (name == "‖")index = -1;// AHCK!
    if (name == "then")return emitIf(*node.parent, context);// pure if handled before
    auto first = node.first();
    if (name == ":=")return emitDeclaration(node, first);
    if (name == "=")return emitSetter(node, first, context);// todo node.first dodgy
    if (name == ".") return emitAttribute(node, context);
//	if (name=="#")XXX return emitIndexPattern(node[0], node[1], context); elsewhere (and emit(node)!
    if (name == "::=")return emitGetGlobal(node); // globals ASSIGNMENT already handled in analyze / globalSection()
    if (node.length < 1 and not node.value.node and not node.next) {
        if (name == "not")return emitValue(True, context);
        error("missing args for operator "s + name);
    } else if (node.length == 1) {
        Node arg = node.children[0];
        const Code &arg_code = emitExpression(arg, context);// should ALWAYS just be value, right?
        arg_type = last_type;
        code.push(arg_code);// might be empty ok
    } else if (node.length == 2) {
        Node lhs = node.children[0];//["lhs"];
        Node rhs = node.children[1];//["rhs"];
        const Code &lhs_code = emitExpression(lhs, context);
        Type lhs_type = last_type;
        arg_type = last_type;// needs to be visible to array index [1,2,3]#1 gets FUCKED up in rhs operations!!
        const Code &rhs_code = emitExpression(rhs, context);
        Type rhs_type = last_type;
        Type common_type = commonType(lhs_type, rhs_type);// 3.1 + 3 => 6.1 etc
        code.push(lhs_code);// might be empty ok
        code.add(cast(lhs_type, common_type));
        code.push(rhs_code);// might be empty ok
        code.add(cast(rhs_type, common_type));
        if (common_type != void_block)
            last_type = common_type;
        else last_type = rhs_type;

    } else if (node.length > 2) {// todo: n-ary? ∑? is just a context!
        error("Too many args for operator "s + name);
//	} else if (node.next) { // todo really? handle ungrouped HERE? just hiding bugs?
//		const Code &arg_code = emitExpression(*node.next, context);
//		code.push(arg_code);// might be empty ok
    } else if (node.value.node and node.kind == key) {// todo: serialize?
        const Code &arg_code = emitExpression(*node.value.node, context);
        code.push(arg_code);
    }
    /*
     * PARAMETERS of operators (but not functions) are now on the STACK!!
        no more need for (Node &arg : node) emitExpression(arg,context);;
     * */
    if (index >= 0) {// FUNCTION CALL
        print("OPERATOR / FUNCTION CALL: %s\n"s % name);
        code.addByte(function_call);
        code.add(index);
        return code;
    }
    if (last_type == unknown_type) {
        if (allow_untyped_nodes)
            last_type = Primitive::node;// anything
        else
            internal_error("unknown type should be inferred by now:\n"s + node.serialize());
    }
    byte opcode = opcodes(name, mapTypeToWasm(last_type), mapTypeToWasm(arg_type));

    if (opcode >= 0x8b and opcode <= 0x98)
        code.add(cast(last_type, float32));// float ops
    if (opcode >= 0x99 and opcode <= 0xA6)
        code.add(cast(last_type, f64)); // double ops

    if (last_type == stringp or last_type == charp) {
        code.add(emitStringOp(node, context));
        return code;
    } else if (opcode == f32_sqrt) {
        code.addByte(f32_sqrt);
        last_type = f32t;
    } else if (opcode == f32_eqz) { // hack for missing f32_eqz
        if (last_type == float32)
            code.addByte(i32_reinterpret_f32);// f32->i32  i32_trunc_f32_s would also work, but reinterpret is cheaper
        code.addByte(i32_eqz);
        last_type = i32t;// bool'ish
    } else if (opcode == f64_eqz) { // hack for missing f32_eqz
        if (last_type == float64)
            code.addByte(i64_reinterpret_f64);// f32->i32  i32_trunc_f32_s would also work, but reinterpret is cheaper
        code.addByte(i64_eqz);
        last_type = i32t;// bool'ish
    } else if (name == "++" or name == "--") {
        Node increased = Node(name[0]).setType(operators);
        increased.add(first); // if not first emitted
        increased.add(new Node(1));// todo polymorph operator++ instead of +1 !
        code.add(emitSetter(first, increased, context));
    } else if (name == "#") {// index operator
        if (node.parent and node.parent->name == "=")// setter!
            return code + emitIndexWrite(node[0], context);// todo
        else {
            return code + emitIndexRead(node, context, true, true);
        }
    } else if (isFunction(name, true) or isFunction(normOperator(name), true)) {
        emitCall(node, context);
    } else if (opcode > 0xC0) {
        error("internal opcode not handled"s + opcode);
    } else if (opcode > 0) {
        code.addByte(opcode);
        if (last_type == none or last_type == voids)
            last_type = i32t;
        if (opcode >= 0x45 and opcode <= 0x78)
            last_type = i32;// int ops (also f64.eqz …)
    } else if (name == "²") {
//		error("this should be handled universally in analyse: x² => x*x no matter what!");
        // BUT non-lazy calling twice? x² => x * result
        Local &result = context.locals.last();// ["result"];
        code.add(tee_local);// neeeeds result local
        code.add(result.position);
        code.add(get_local);
        code.add(result.position);
        code.add(opcodes("*", last_type));
    } else if (name == "**" or name == "to the" or name == "^" or name == "^^") {
//		if(last_value==0)code.addConst(1);
//		if(last_value==1)return code;
        if (last_type == int32) code.add(emitCall(*new Node("powi"), context));
        else if (last_type == float32) code.add(emitCall(*new Node("powf"), context));
        else if (last_type == f64) code.add(emitCall(*new Node("pow"), context));
        else code.add(emitCall(*new Node("powi"), context));
    } else if (name.startsWith("-")) {
        code.add(i32_sub);
    } else if (name == "return") {
        // todo multi-value
        List<Type> &returnTypes = context.signature.return_types;
        Valtype return_type = mapTypeToWasm(returnTypes.last());
        code.add(cast(last_type, return_type));
        code.add(return_block);
    } else if (name == "as") {
        code.add(emitCall(*new Node("cast"), context));

    } else if (name == "%") {// int cases handled above
        if (last_type == float32)
            return code.add(emitCall(Node("modulo_float").setType(call), context));// mod_f
        else
            return code.add(emitCall(Node("modulo_double").setType(call), context));// mod_d
    } else if (name == "?") {
        return emitIf(node, context);
    } else if (name == "ⁿ") {
        if (node.length == 1) {
            code.add(cast(last_type, float64));// todo all casts should be auto-cast (in emitCall) now, right?
        }
        if (node.length <= 1) {// use stack
            if (not context.locals.has("n"))error("unknown n");
            code.add(get_local);
            code.addInt(context.locals["n"].position);
            code.add(cast(context.locals["n"].typo, float64));// todo all casts should be auto-cast now, right?
        }
        code.add(emitCall(*new Node("pow"), context));
//		else
//			code.add(emitCall(*new Node("powi"), context));

    } else {
        error("unknown opcode / call / symbol: "s + name + " : " + index);
    }

    if (opcode == get_local and node.length == 1) {// arg AFTER op (not as const!)
        long last_local = first.value.longy;
        code.push(last_local);
        last_type = context.locals.at(last_local).typo;
    }
    if (opcode >= 0x45 and opcode <= 0x78)
        last_type = i32;// int ops (also f64.eqz …)
    return code;
}

Type commonType(Type lhs, Type rhs) {
    // todo: per function / operator!
    if (lhs == i64 and rhs == int32) return i64;
    if (lhs == int32 and rhs == i64) return i64;
    if (lhs == float64 or rhs == float64)return float64;
    if (lhs == float32 or rhs == float32)return float32;
    // todo?
    return lhs;
}

Valtype needsUpgrade(Valtype lhs, Valtype rhs, String string) {
    if (lhs == float64 or rhs == float64)
        return float64;
    if (lhs == float32 or rhs == float32)return float32;
    return none;
}

[[nodiscard]]
Code emitStringOp(Node &op, Function &context) {
//	Code stringOp;
//	op = normOperator(op.name);
    if (op == "+") {
        op = Node("_Z6concatPKcS0_");//demangled on readWasm, but careful, other signatures might overwrite desired one
        functions["_Z6concatPKcS0_"].signature.return_types[0] = charp;// can't infer from demangled export name nor wasm type!
        return emitCall(op, context);
//		stringOp.addByte();
    } else if (op == "==" or op == "is" or op == "equals") {
        op = Node("eq");//  careful : various signatures
        last_type = charp;//stringp;
        return Code(i32_const) + Code(-1) + emitCall(op, context);// third param required!
    } else if (op == "#") {// todo: all different index / op matches
        op = Node("getChar");//  careful : various signatures
        return emitCall(op, context);
    } else if (op == "not" or op == "¬") {// todo: all different index / op matches
        op = Node("empty");//  careful : various signatures for falsey falsy truthy
        return emitCall(op, context).add(i32_eqz);
    } else if (op == "logs" or op == "prints" or op == "print") {// should be handled before, but if not print anyways
        op = Node("puts");// todo: chars vs shared String& ?
        return emitCall(op, context);
    } else todo("string op not implemented: "s + op.name);
    return Code();
}

// starting with 0!
//inline haha you can't inline wasm
char getChar(chars string, int nr) {
    int len = strlen(string);
    if (nr < 1)error("#index starts with 1, use [] if you want 0 indexing");
    if (nr > len)error("index out of bounds %i>%i "s % nr % len);
    return string[nr - 1 % len];
}

//	todo : ALWAYS MAKE RESULT VARIABLE FIRST IN BLOCK (index 0 after args, used for 'it'  after while(){} etc) !!!
int last_local = 0;


bool isVariableName(String name) {
    return name[0] >= 'A';// todo
}

Code emitConstruct(Node &node, Function &context);

// also init expressions of globals!
[[nodiscard]]
Code emitExpression(Node &node, Function &context/*="main"*/) { // expression, node or BODY (list)
//	if(nodes==NIL)return Code();// emit nothing unless NIL is explicit! todo
    Code code;
    String &name = node.name;
//	int index = functionIndices.position(name);
//	if (index >= 0 and not context.locals.has(name))
//		error("locals should be analyzed in parser");
//		locals[name] = List<String>();
//	locals[index]= Map<int, String>();
    if (node.kind == unknown and context.locals.has(node.name))
        node.kind = reference;// todo clean fallback

    if (name == "if")
        return emitIf(node, context);
    if (name == "while")
        return emitWhile(node, context);
    if (name == "it") {
        // todo when is it ok to just reuse the last value on stack!?
//		if(last_type==none or last_type==voids){
        code.addByte(get_local);
        code.addByte(last_local);
        last_type = context.locals.at(last_local).typo;
//		}
        return code;
    }
//	if (name=="#"){
//		Code c=emitExpression(node[0], context);
//		return c+emitIndexPattern(node[0], node[1], context);// todo redundant somewhere!
//	}
    //	or node.kind == groups ??? NO!

    if ((node.kind == call or node.kind == reference or node.kind == operators) and functionIndices.has(name)) {
        if (not isFunction(name, true));//				todo("how?");
        else
            return emitCall(node, context);
    }

    Node &first = node.first();
    switch (node.kind) {
        case functor:
        case records:
        case clazz:
        case structs:
        case flags:
            // nothing to do since meta info is in module
            // node is in `types` / `functions` and all fields are in `globals`
            // TYPE info at compile time in types[] // todo: emit reflection data / wit !
            // for INSTANCES see emitConstruct(node, context);
            return code;
        case objects:
        case groups:
            // todo: all cases of true list vs list of expression
            if (node.length > 0 and isProperList(node)) {
                return emitData(node, context);// pointer in const format!
//				return emitArray(node, context);
            }// else fallthough:
        case expression:
            if (not name.empty())
                code.add(emitSetter(node, node, context));
            else if (node.length > 0)
                for (Node &child: node) {
                    Code expression = emitExpression(child, context);
                    code.push(expression);
                };
            break;
        case call:
            return emitCall(node, context);
            break;
        case operators:
            return emitOperator(node, context);
        case declaration:
            return emitDeclaration(node, first);
        case assignment:
            return emitSetter(node, first, context);
        case nils:
        case longs:
        case reals:
        case bools:
        case strings:
            if (not node.isSetter() || node.value.longy == 0) // todo 0  x="x" '123'="123" redundancy bites us here
                return emitValue(node, context);
//			else
//				todo("FALLTHROUGH to set x=\"123\"!");
        case key: // todo i=ø
            if (name.length > 0 and name.charAt(0) >= '0' and name.charAt(0) <= '9') // if 0:3 else 4 hack
                return emitValue(*new Node(parseLong(name)), context);
            if (not isVariableName(name))
                return emitNode(node, context);
            // else:
        case reference: {
            if (name.empty()) {
//				error("empty reference!");
                return Code();
            }
//			Map<int, String>
            int local_index = context.locals.position(name);// defined in block header
            if (name.startsWith("$")) {// wasm style $0 = first arg
                local_index = parseLong(name.substring(1));
            }
            if (local_index < 0) { // collected before, so can't be setter here
                if (functionCodes.has(name) or functions.has(name))
                    return emitCall(node, context);
                else if (globals.has(name)) {
                    if (context.name == "global")
                        return emitValue(node, context);
                    return emitGetGlobal(node);
                } else if (name == "ℯ")
                    return emitValue(*new Node(2.7182818284590), context);
                else if (name == "τ" or name == "tau") // if not provided as global
                    return emitValue(*new Node(6.283185307179586), context);
                else if (name == "π" or name == "pi") // if not provided as global
                    return emitValue(*new Node(3.141592653589793), context);
                else if (!node.isSetter()) {
//                    print(context.locals)
                    if (not node.type)
                        error("UNKNOWN local symbol ‘"s + name + "’ in context " + context);
                } else {
                    error("local symbol ‘"s + name.trim() + "’ in " + context + " should be registered in analyze()!");
                }
            }
            if (node.isSetter()) { //SET
                if (node.kind == key)
                    code = code + emitExpression(*node.value.node, context); // done above!
                else
                    code = code + emitValue(node, context); // done above!
                if (context.locals.at(local_index).typo == unknown_type)
                    context.locals.at(local_index).typo = last_type;
                else
                    code.add(cast(last_type, context.locals.at(local_index).typo));
//				todo: convert if wrong type
                code.addByte(tee_local);// set and get/keep
                code.addByte(local_index);
                // todo KF 2022-6-5 last_type set by emitExpression (?)
//				last_type = declaredFunctions[context.locals.at([local_index).valtype;
            } else {// GET
                code.addByte(get_local);// todo: skip repeats
                code.addByte(local_index);
                last_type = context.locals.at(local_index).typo;
            }
        }
            break;
        case patterns: // x=[];x[1]=2;x[1]==>2 but not [1,2] lists
        {
            if (not node.parent)// todo: when is op not an operator? wrong: or node.parent->kind == groups)
                return emitArray(node, context);
            else if (node.parent->kind == declaration)
                return emitIndexWrite(*node.parent, context);
            else {
                Node array1 = NUL;
                if (node.parent)array1 = node.parent->first();
                return emitIndexPattern(array1, node, context, false);
            }
            // make sure array is on stack OR last_object_pointer is set!
        }
//		case groups: todo: true list vs list of expression
//		case objects:
        case arrays:
        case buffers:
//for (Node &child : node) {
//	const Code &expression = emitExpression(child, context);
//	code.push(expression);
//};
            return emitArray(node, context);
            break;
        case undefined:
        case unknown:// todo: proper NIL!
            return code;
        case constructor:
            return emitConstruct(node, context);
        default:
            error("unhandled node type «"s + node.name + "» : " + typeName(node.kind));
    }
    return code;
}

void discard(Code &code);

void discard(Code code);

Code emitConstruct(Node &node, Function &context) {
    Code code;
    int pointer = data_index_end;
    for (Node &field: node) {
        discard(emitData(field, context));// just write the values to memory and lastly return start-pointer
    }
    last_object = &node;
    last_object_pointer = pointer;
//    if(node.type)
//    last_typo = mapTypeToPrimitive(*node.type);
    last_type = pointer;
    code.addConst32(pointer);// base for future index getter/setter [0] #1
    return code;
}

[[nodiscard]]
Code emitWhile(Node &node, Function &context) {
    Code code;
    Node condition = node[0].values();
    Node then = node[1].values();
    Valtype loop_type = none; // todo: I thought everything is an expression!?
//	Valtype loop_type = i64;// everything is an expression!

    code.addByte(loop);
    code.addByte(loop_type);

    code = code + emitExpression(condition, context);// condition
    code.addByte(if_i);
    code.addByte(none);// type:void_block
//	code.addByte(int32);
    code = code + emitExpression(then, context);// BODY
    code.addByte(br_branch);
    code.addByte(1);
    code.addByte(end_block);// end if condition then action
    if (loop_type == none) {
        code.addByte(drop);
        code.addByte(drop);
    }
    // else type should fall through
    code.addByte(end_block);// end while loop
    last_type = loop_type;
//	int block_value= 0;// todo : ALWAYS MAKE RESULT VARIABLE FIRST IN FUNCTION!!!
    return code;
}

// wasm loop…br_if is like do{}while(condition), so we have to rework while(condition){}
[[nodiscard]]
Code emitWhile2(Node &node, Function &context) {
    Code code;
    // outer block
//	code.addByte(block);
//	code.addByte(void_block);
    // inner loop
    code.addByte(loop);
    code.addByte(none);// void_block
    // comprinte the while expression
    code.add(emitExpression(node[0], context));// node.value.node or
//	code.add(emitExpression(node["condition"], context));// node.value.node or
    code.addByte(i32_eqz);
    // br_if $label0
    code.addByte(br_if);
    code.addByte(1);
//			code.push(signedLEB128(1));
    // the nested logic
    code.add(emitExpression(node[1], context));// BODY
//	code.add(emitExpression(node["then"], context));// BODY
    // br $label1
//	code.addByte(br);
//	code.addByte(0);
//				code.push(signedLEB128(0));
    code.addByte(end_block); // end loop
    code.addByte(get_local);
    int block_value = 0;// todo : ALWAYS MAKE RESULT VARIABLE FIRST IN FUNCTION!!!
    code.addByte(block_value);// todo: skip if last value is result
//	code.addByte(end_block); // end block
    return code;
}

[[nodiscard]]
[[nodiscard]]
Code emitExpression(Node *nodes, Function &context) {
    if (!nodes)return Code();
//	if(nodes==NIL)return Code();// emit nothing unless NIL is explicit! todo
    return emitExpression(*nodes, context);
}

[[nodiscard]]
Code emitCall(Node &fun, Function &context) {
    Code code;
    auto name = fun.name;
    if (not functions.has(name)) {
        auto normed = normOperator(name);
        if (not functions.has(normed))
            error("unknown context "s + name + " (" + normed + ")"); // checked before, remove
        else name = normed;
    }
    Function &function = functions[name];// NEW context! but don't write context ref!
    Signature &signature = function.signature;

    int index = function.code_index;
    if (functionIndices.has(name)) {
        if (index >= 0 and index != functionIndices[name]) todow("index!=functionIndices[name]");
        index = functionIndices[name];
    } else {
//		breakpoint_helper
//		warn("relying on context.index OK?");
//		functionIndices[name] = context.index;
    }
    if (index < 0)
        error("Calling %s NO INDEX. TypeSection created before code Section. Indices must be known by now! "s % name);
    int i = 0;
    // args may have already been emitted, e.g. "A"+"B" concat
    for (Node &arg: fun) {
        code.push(emitExpression(arg, context));
//		Valtype argType = mapTypeToWasm(arg); // todo ((+ 1 2)) needs deep analysis, or:
        Type argType = last_type;// evaluated expression smarter than node arg!
        Type sigType = signature.parameter_types[i++];
        if (sigType != argType)
            code.push(cast(argType, sigType));
    };
    code.addByte(function_call);
    code.addInt(index);// as LEB!
    code.addByte(nop);// padding for potential relocation
    context.is_used = true;

    // todo multi-value
    Type return_type = signature.return_types.last(none);
    last_type = return_type;
    if (signature.wasm_return_type)
        check_eq(mapTypeToWasm(last_type), signature.wasm_return_type);
//	last_typo.clazz = &signature.return_type;// todo dodgy!
    return code;
}

[[nodiscard]]
Code cast(Valtype from, Valtype to) {
    Code nop;// if two arguments are the same, commontype is 'none' and we return empty code (not even a nop, technically)
    if (from == to)return nop;// nop
    if (from == void_block)return nop;// todo: pray
    if ((Type) from == unknown_type)return nop;// todo: don't pray
    if (to == none or (Type) to == unknown_type or to == voids)return nop;// no cast needed magic VERSUS wasm drop!!!
    last_type = to;// danger: hides last_type in caller!
    if (from == 0 and to == i32t)return nop;// nil or false ok as int? otherwise add const 0!
    if (from == float32 and to == float64)return Code(f64_from_f32);
    if (from == float32 and to == i32t) return Code(f32_cast_to_i32_s);
    if (from == i32t and to == float32)return Code(f32_from_int32);
//	if (from == i32t and to == float64)return Code(i32_cast_to_f64_s);
    if (from == i64 and to == i32) return Code(i32_wrap_i64);
    if (from == float32 and to == i32) return Code(i32_trunc_f32_s);
//	if(from==f32u and to==i32)	return Code(i32_trunc_f32_𝗎);
    if (from == f64 and to == i32) return Code(i32_trunc_f64_s);
//	if(from==f64u and to==i32)	return Code(i32_trunc_𝖿𝟨𝟦_𝗎);
    if (from == i32 and to == i64) return Code(i64_extend_i32_s);
//	if(from==i32u and to==i64)	return Code(i64_extend_i32_𝗎);
    if (from == float32 and to == i64) return Code(i64_trunc_f32_s);
//	if(from==f32u and to==i64)	return Code(i64_trunc_f32_𝗎);
    if (from == f64 and to == i64) return Code(i64_trunc_f64_s);
//	if(from==f64u and to==i64)	return Code(i64_trunc_𝖿𝟨𝟦_𝗎);
    if (from == i32 and to == float32) return Code(f32_convert_i32_s);
//	if(from==i32u and to==f32)	return Code(f32_convert_i32_𝗎);
    if (from == i64 and to == float32)
        return Code(f32_convert_i64_s);
//	if(from==f64u and to==f32)	return Code(f32_convert_i64_𝗎);
    if (from == f64 and to == float32) return Code(f32_demote_f64);
    if (from == i32 and to == f64)
        return Code(f64_convert_i32_s);
//	if(from==i32u and to==f64)	return Code(f64_convert_i32_𝗎);
    if (from == i64 and to == f64)
        return Code(f64_convert_i64_s);
//	if(from==f64u and to==f64)	return Code(f64_convert_i64_𝗎);
    if (from == float32 and to == f64) return Code(f64_promote_f32);
//	if(from==f32 and to==i32)	return Code(i32_reinterpret_f32);
//	if(from==f64 and to==i64)	return Code(i64_reinterpret_𝖿𝟨𝟦);
//	if(from==i32 and to==f32)	return Code(f32_reinterpret_i32);
//	if(from==i64 and to==f64)	return Code(f64_reinterpret_i64);
    if (from == i64 and to == float32) return Code(f64_convert_i64_s).addByte(f32_from_f64);

//	if (from == void_block and to == i32)
//		return Code().addConst(-666);// dummy return value todo: only if main(), else WARN/ERROR!
    error("incompatible types "s + typeName(from) + " => " + typeName(to));
    return nop;
}


[[nodiscard]]
Code cast(Type from, Type to) {
    Code nop;// if two arguments are the same, commontype is 'none' and we return empty code (not even a nop, technically)
    if (to == none or to == unknown_type or to == voids)return nop;// no cast needed magic VERSUS wasm drop!!!
    if (from == to)return nop;// nop
    last_type = to;// danger: hides last_type in caller!
    if (from == node and to == i64t)
        return Code(i64_extend_i32_s).addConst64(node_header_64) + Code(i64_or);// turn it into node_pointer_64 !
    if (from == array and to == charp)return nop;// uh, careful? [1,2,3]#2 ≠ 0x0100000…#2
    if (from == i32t and to == charp)return nop;// assume i32 is a pointer here. todo?
    if (from == charp and to == i64t) return Code(i64_extend_i32_s);
    if (from == charp and to == i32t)return nop;// assume i32 is a pointer here. todo?
    if (from == array and to == i32)return nop;// pray / assume i32 is a pointer here. todo!
    if (from == array and to == i64)return Code(i64_extend_i32_u);;// pray / assume i32 is a pointer here. todo!
    if (from == i32t and to == array)return nop;// pray / assume i32 is a pointer here. todo!
    if (from == float32 and to == array)return nop;// pray / assume f32 is a pointer here. LOL NO todo!
    if (from == i64 and to == array)return Code(i32_wrap_i64);;// pray / assume i32 is a pointer here. todo!
//    if(Valtype)
    return cast(mapTypeToWasm(from), mapTypeToWasm(to));
}

// casting in our case also means construction! (x, y) as point == point(x,y)
[[nodiscard]]
Code cast(Node &from, Node &to, Function &context) {
    if (to == Long)return cast(mapTypeToWasm(from), i64);
    if (to == Double)return cast(mapTypeToWasm(from), f64);
    Node calle("cast");
    calle.add(from);
    calle.add(to);
    return emitCall(calle, context);// todo context?
}

[[nodiscard]]
Code emitDeclaration(Node &fun, Node &body) {
    // todo: x := 7 vs x := y*y
    //
    if (not functionIndices.has(fun.name)) {
        error("Declaration %s need to be registered before in the parser so they can be called from main code!"s %
              fun.name);
//		functionIndices[fun.name] = functionIndices.size();
    }
//	else {
//		error("redeclaration of symbol: "s + fun.name);
//	}
    Function &context = functions[fun.name];
//	Signature &signature = context.signature;
    context.emit = true;// all are 'export' for now. also set in analyze!
    functionCodes[fun.name] = emitBlock(body, context);
    last_type = none;// todo reference to new symbol x = (y:=z)
    return Code();// empty
}

[[nodiscard]]
Code emitSetter(Node &node, Node &value, Function &context) {
    if (node.first().name == "#") {// x#y=z
        return emitPatternSetter(node.first().first(), node.first().last(), node.last(), context);
    }
    if (node.name == "=") {
        if (node.length != 2)error("assignment needs 2 arguments");
        return emitSetter(node[0], node[1], context);
    }
    String &variable = node.name;
    if (!context.locals.has(variable)) {
//        error("variable %s in context %s missed by parser! "_s % variable % context.name);
        warn("variable %s in context %s emitted as node data:\n"_s % variable % context.name + node.serialize());
        Code code = emitNode(node, context);
//        addLocal(context, variable, Primitive::node, false);
//        code.add(tee_local);
//        code.add(context.locals[variable].position);
        return code;// wasm_node_index 'pointer' (const.int)
    }

    Local &local = context.locals[variable];
    auto variable_type = local.typo;
//    Valtype value_type = mapTypeToWasm(value); // todo?
    if (variable_type == unknown_type or variable_type == voids) {
        variable_type = last_type;// todo : could have been done in analysis!
        local.typo = last_type;// NO! the type doesn't change: example: float x).valtype7
    }
    if (last_type == array or variable_type == array or variable_type == charp) {
        referenceIndices.insert_or_assign(variable, data_index_end);// WILL be last_data !
        referenceDataIndices[variable] = data_index_end + headerOffset(value);
        referenceMap[variable] = value;// node; // lookup types, array length …
    }
    Code setter;
//	auto values = value.values();
    if (value.hash() == node.hash())
        value = node.values();
//    value.parent = &node;
    if (value.kind == arrays or value.kind == objects or value.kind == groups)
        value.name = node.name;// HACK to store referenceIndices
    value.parent = &node;// might have been lost through operator shuffle. todo: fix in analyze
    Code value1 = emitExpression(value, context);
    setter.add(value1);
    setter.add(cast(last_type, variable_type));
    setter.add(tee_local);
    setter.add(local.position);
//    if (value.kind == strings){
//        referenceIndices.insert_or_assign(local.name,last_object_pointer);
//        referenceDataIndices[local.name] = last_object_pointer;//-8;// todo HAKC and BUG!! this should be -8 !!
//    }
    if (variable_type != void_block)
        last_type = variable_type;// still the type of the local, not of the value. example: float x=7
    return setter;
}

//bool isPrimitive(Node &node);

Code zeroConst(Type valtype);

//[[nodiscard]];
Code emitIf(Node &node, Function &context) {
    Code code;
    //	 gets rid of operator, we MAY want .flat() ?
    Node condition = node[0].values();
//	Node &condition = node["condition"];
    code = code + emitExpression(condition, context);
    code.add(cast(last_type, int32));
    code.addByte(if_i);
    Node then = node[1].values();
//	Node &then = node["then"];
    auto then_block = emitExpression(then, context);
    auto returnType = last_type;
    code.addByte(mapTypeToWasm(returnType));// of then!
    code = code + then_block;// BODY
    bool else_done = false;
    if (node.length == 3) {
//		Node otherwise = node["else"];//->clone();
        Node otherwise = node[2].values();
        if (otherwise.length > 0 or isPrimitive(otherwise)) {
            code.addByte(else_);
            code = code + emitExpression(otherwise, context);
            code.add(cast(last_type, returnType));
            else_done = true;
        }
    }
    if (not else_done and returnType != none) {
//		we NEED to return something in wasm! else: "type mismatch in if false branch"
        code.addByte(else_);
        code.add(zeroConst(returnType));
    }
    code.addByte(end_block);
    return code;
}

Code zeroConst(Type returnType) {
    Code code;
    if (returnType == int32)
        code.addConst32(0);
    if (returnType == float32) {
        code.add(f32_const);
        code.push((bytes) malloc(4), 4);
    }
    if (returnType == f64) {
        code.add(f64_const);
        code.push((bytes) malloc(8), 8);
    }
    if (returnType == i64) {
        code.add(i64_const);
        code.push((bytes) malloc(8), 8);
    }
    return code;
}

/*
[[nodiscard]]
Code emitIf_OLD(Node &node) {
	Code code;
//	case ifStatement:
	// if block
	code.addByte(block);
	code.addByte(void_block);
	// comprinte the if expression
	Node *condition = node[0].value.node;
//	Node *condition = node["condition"];//.value.node->clone();
	emitExpression(condition,context);
	code.addByte(i32_eqz);
	// br_if $label0
	code.addByte(br_if);
	code.addByte(0);
//			code.opcode(signedLEB128(0));
	// the nested logic
	Node *then = node[1].value.node;
//	Node *then = node["then"].value.node->clone();
	emitExpression(then);// BODY
	// end block
	code.addByte(end_block);

	if (node.length == 3) {

		// else block
//			or if OR-IF YAY semantically beautiful if false {} or if 2>1 {}
//			// comprinte the if expression (elsif) elif orif
		code.addByte(block);
		code.addByte(void_block);
//
//			emitExpression(node.param);
		code.addByte(i32_auto);
////				code.opcode(signedLEB128(1));
		code.addByte(1);
		code.addByte(i32_eq);
//			// br_if $label0
		code.addByte(br_if);
		code.addByte(0);
////				code.opcode(signedLEB128(0));
//			// the nested logic
//		Node *otherwise = node["else"].value.node->clone();
		Node *otherwise = node[2].value.node;//->clone();
		emitExpression(otherwise);

//			// end block
		code.addByte(end_block);
	}
	return code;
}
*/

[[nodiscard]]
Code Call(char *symbol) {//},Node* args=0) {
    Code code;
    code.addByte(function_call);
    int i = functionIndices.position(symbol);
    if (i < 0)error("UNKNOWN symbol "s + symbol);
//	code.opcode(unsignedLEB128(i),8);
    code.addByte(i);
    return Code();
}

[[nodiscard]]
Code encodeString(char *str) {
    size_t len = strlen(str);
    Code code = Code(len, (bytes) str, len);
    return code;//.push(0);
};


[[nodiscard]]
Code emitBlock(Node &node, Function &context) {
//	todo : ALWAYS MAKE RESULT VARIABLE FIRST IN FUNCTION!!!
//	char code_data[] = {0/*locals_count*/,i32_const,42,call,0 /*logi*/,i32_auto,21,return_block,end_block};
// 0x00 == unreachable as block header !?
//	char code_data[] = {0/*locals_count*/,i32_auto,21,return_block,end_block};// 0x00 == unreachable as block header !?
//	Code(code_data,sizeof(code_data)); // todo : memcopy, else stack value is LOST
    Code block;
//	Map<int, String>
//	collect_locals(node, context);// DONE IN analyze
//	int locals_count = current_local_names.size();
//	context.locals = current_local_names;
//	todo : ALWAYS MAKE RESULT VARIABLE FIRST IN BLOCK (index 0, used after while(){} etc) !!!
// todo: locals must ALWAYS be collected in analyze step, emitExpression is too late!
    last_local = 0;
    last_type = none;//int32;

    int locals_count = context.locals.size();// incuding params
    context.locals.add("result", {.is_param=false, .position=locals_count++, .name="result", .typo=Valtype::i64,});
    int argument_count = context.signature.size();
    if (locals_count >= argument_count)
        locals_count = locals_count - argument_count;
    else
        warn("locals consumed by arguments"); // ok in  double := it * 2; => double(it){it*2}

    trace("found %d locals for %s"s % locals_count % context.name);

    // todo block.addByte(i+1) // index seems to be wrong: i==NUMBER of locals of type xyz ??
//	013b74: 01 7f                      | local[0] type=i32
//	013b76: 02 7f                      | local[1..2] type=i32
//	013b78: 03 7f                      | local[3..5] type=i32

    Code inner_code_data = emitExpression(node, context);

    // locals can still be updated in emitExpression

    /// 1. Emit block type header
    block.addByte(locals_count);
//    for (int i = 0; i < locals_count; i++) {
//        auto name = context.locals.at(i);// add later to custom section, here only for debugging
//        Valtype valtype = context[i];
    for (auto name: context.locals) {
        trace("local "s + name);
        Local &local = context.locals[name];
        if (local.is_param)continue;// part of context type!
        Type typo = local.typo;
//		block.addByte(i + 1);// index
        block.addByte(1);// count! todo: group by type nah
        if (typo == unknown_type)
            typo = int32;
// todo		internal_error("unknown type should be inferred by now for local "s + name);
        if (typo == none or typo == voids)
            typo = int32;
        if (typo == charp or typo == array)
            typo = int32; // int64? extend to smart pointer later!
        block.addByte(mapTypeToWasm(typo));
    }

    // 2. emit code
    block.push(inner_code_data);

    // 3. force / cast last type to return type
    auto returnTypes = context.signature.return_types;
    //	for(Valtype return_type: returnTypes) uh, casting should have happened before for  multi-value
    Valtype return_type = mapTypeToWasm(returnTypes[0]);
    // switch back to return_types[context] for block?
    if (last_type == none) last_type = voids;
    if (last_type == void_block) last_type = voids;
    bool needs_cast = return_type != last_type;
    auto abi = wasp_smart_pointers;// context.abi;
    if (return_type == Valtype::voids and last_type != Valtype::voids)
        block.addByte(drop);
    else if (return_type == Valtype::i32t and last_type == Valtype::voids)
        block.addConst32(0);
    else if (return_type == Valtype::i64t and last_type == Valtype::voids)
        block.addConst64(0);// 		needs_cast = false;
    else if (abi == wasp_smart_pointers) {
//		if(last_type==charp)block.push(0xC0000000, false,true).addByte(i32_or);// string
//		if(last_type==charp)block.addConst(-1073741824).addByte(i32_or);// string
        if (last_type.type == int_array or last_type == array) {
            block.add(cast(last_type, i64));
            block.addConst64(array_header_64).addByte(i64_or); // todo: other arrays
//			if (return_type==float64)
//			block.addByte(f64_reinterpret_i64);// hack smart pointers as main return: f64 has int range which is never hit
            last_type = i64;
        }
//		block.addConst32(array_header_32).addByte(i32_or); // todo: other arrays
        else if (last_type.kind == strings or last_type.type == c_string or last_type == charp) { //
            block.addByte(i64_extend_i32_u);
            block.addConst64(string_header_64);
            block.addByte(i64_or);
            last_type = i64;
            needs_cast = return_type == i64;
        } else if (last_type.kind == reference) {
//			if (last_type==charp)
//				block.addConst(string_header_64).addByte(i64_or);
            todo("last_type ref");
        } else if (last_type == float64 and context.name == start) {
            // hack smart pointers as main return: f64 has range which is never hit by int
            block.addByte(i64_reinterpret_f64);
            last_type = i64;
        }
//		if(last_type==charp)block.addConst32((unsigned int)0xC0000000).addByte(i32_or);// string
//		if(last_type==angle)block.addByte(i32_or).addInt(0xA000000);//
//		if(last_type==pointer)block.addByte(i32_or).addInt(0xF000000);//
    }

    if (needs_cast and last_type.value) {
        block.add(cast(last_type, return_type));
    }

#if MULTI_VALUE
    // 4. emit multi value result, type after result will unexpectedly yield array [result, type] in wasmtime, js, … ABI!
    block.addConst32(last_typo.value);

    // 4. emit multi value result, type BEFORE value
    // SWAP as we want the type to be on the stack IN FRONT of the value, so functions are backwards compatible and only depend on the func_type signature
    // this will result in an array [type, result] though, so DON'T SWAP!
//    int result_local = context.locals["result"].position;
//    block.add(tee_local);
//    block.addInt(result_local);
//    block.addConst32(last_typo.value);
//    block.addConst64(last_typo.value);
//    block.add(get_local);
//    block.addInt(result_local);
#endif


//	if(context=="main" or (context.abi==wasp and returnTypes.size()<2))
//		block.addInt(last_typo);// should have been added to signature before, except for main!
// make sure block has correct wasm type signature!

//	var [type, data]=result // wasm order PRESERVED! no stack inversion!

//if not return_block
// the return statement makes drops superfluous and just takes as many elements from stack as needed (ggf multiple and ignores/drops the rest!)
// to check if all parts of wasp are working flawlessly we may drop the return_block
    block.addByte(return_block);
    block.addByte(end_block);
    return block;
}

/*
//Map<int, String>
// todo: why can't they be all found in parser? 2021/9/4 : they can ;)
List<String> collect_locals(Node &node, Function& context) {
	List<String> &current_locals = context.locals; // no, because there might be gaps(unnamed locals!)
	for (Node &n : node) {
		Valtype type = mapTypeToWasm(n);
		if (type != none and not current_locals.has(n.name)) {
			current_locals.add(n.name);
			declaredFunctions[context].add(type);
		}
	}
	return current_locals;
}
*/


int last_index = -1;

// typeSection created before code Section. All imports must be known in advance!
[[nodiscard]]
Code emitTypeSection() {
    // Function types are vectors of parameters and return types. Currently
    // the type section is a vector of context types
    // TODO optimise - some of the procs might have the same type signature
    int typeCount = 0;
    Code type_data;
//	print(functionIndices);
    for (String fun: functions) {
        if (!fun) {
//			print(functionIndices);
//			print(functions);
            breakpoint_helper
            warn("empty functions[ø] because context=start=''");
//			error("empty context creep functions[ø]");
            continue;
        }
        if (operator_list.has(fun)) {
            continue;
            todo("how did we get here?");
        }
        if (is_operator(fun[0])) {
            todo("how did we get here?");
            continue;
        }// todo how did we get here?
        Function &context = functions[fun];
        Signature &signature = context.signature;
        if (not context.emit /*export/declarations*/ and not context.is_used /*imports*/) {
            trace("not context.emit => skipping unused type for "s + fun);
            continue;
        }
        if (context.is_runtime)
            continue;
        if (context.signature.is_handled)
            continue;
//		if(context.is_import) // types in import section!
//			continue;
        if (not functionIndices.has(fun))
            functionIndices[fun] = ++last_index;
//			error("context %s should be registered in functionIndices by now"s % fun);

        typeMap[fun] = runtime.type_count /* lib offset */ + typeCount++;
        context.signature.type_index = typeMap[fun];// todo check old index? todo shared signatures!?!
        context.signature.is_handled = true;// todo remove
        int param_count = signature.size();
//		Code td = {0x60 /*const type form*/, param_count};
        Code td = Code(func) + Code(param_count);

        for (int i = 0; i < param_count; ++i) {
            td = td + Code(fixValtype(mapTypeToWasm(signature.parameter_types[i])));
        }
        td.addByte(signature.return_types.size());
        for (Type ret: signature.return_types) {
            Valtype valtype = fixValtype(mapTypeToWasm(ret));
            td.addByte(valtype);
        }
        type_data = type_data + td;
    }
    return Code((char) type_section, encodeVector(Code(typeCount) + type_data)).clone();
}

Valtype fixValtype(Valtype valtype) {
    if (valtype == (Valtype) charp) return int32;
    if ((int) valtype >= node) error("exposed internal Valtype");
    if (valtype > 0xC0)error("exposed internal Valtype");
    return valtype;
}

[[nodiscard]]
Code emitImportSection() {
    // the import section is a vector of imported functions
    Code import_code;
    import_count = 0;
    for (String fun: functions) {
        String import_module = "env";
        Function &function = functions[fun];
        if (function.is_import and function.is_used and not function.is_builtin) {
            function.code_index = import_count++;
            if (function.module and not function.module->name.empty())
                import_module = function.module->name;
            auto type = typeMap[fun];
            import_code =
                    import_code + encodeString(import_module) + encodeString(fun).addByte(func_export).addInt(type);
        }
    }

    if (runtime_function_offset) {
        if (import_count > 0)
            error("runtime_offset & imports currently not supported\n");
//		breakpoint_helper
        import_count = 0;
        return {};
    }

    int extra_mem = 0;
    if (memoryHandling == import_memory) {
        extra_mem = 1;// add to import_section but not to functions:import_count
        int init_page_count = 1024; // 64kb each, 65336 pages max
        import_code =
                import_code + encodeString("env") + encodeString("memory") + (byte) mem_export/*type*/+ (byte) 0x00 +
                Code(init_page_count);
    }
    if (import_code.length == 0)return Code();
    auto importSection = createSection(import_section, Code(import_count + extra_mem) + import_code);// + sqrt_ii
    return importSection.clone();
}

//int function_count;// misleading name: declared functions minus import_count  (together : functionIndices.size() )
//int new_count;
int function_block_count;

//int builtins_used=0;
[[nodiscard]]
Code emitCodeSection(Node &root) {
    // the code section contains vectors of functions
    // index needs to be known before emitting code, so call $i works

    if (root.kind == objects)
        root.kind = expression;// todo why hack?

//	int new_count;
//	new_count = declaredFunctions.size();
    for (auto declared: functions) {
        if (declared == "global")continue;
        Function &context = functions[declared];// todo use more often;)
        if (not context.emit)continue;
        if (declared.empty())error("Bug: empty context name (how?)");
        if (declared != "main") print("declared context: "s + declared);
        if (not functionIndices.has(declared)) {// used or not!
            functionIndices[declared] = ++last_index;
//            context.index=last_index; todo what if it already had different index!?
        }
    }

    int fd_write_import = functionIndices.has("fd_write") ? functionIndices["fd_write"] : 0;
    int main_offset = functionIndices.has(start) ? functionIndices[start] : 0;
    if (main_offset >= 0x80) todow("leb main_offset")


// https://pengowray.github.io/wasm-ops/
//	char code_data[] = {0x01,0x05,0x00,0x41,0x2A,0x0F,0x0B};// 0x41==i32_auto  0x2A==42 0x0F==return 0x0B=='end (context block)' opcode @+39
//	byte code_fourty2[] = {0/*locals_count*/, i32_auto, 42, return_block, end_block};
    byte code_nop[] = {0/*locals_count*/, end_block};// NOP
    byte code_start[] = {0/*locals_count*/, call_, (byte) main_offset, nop, nop, drop, end_block};// needs own type etc
    byte code_id[] = {1/*locals_count*/, 1/*one of type: */, i32t, get_local, 0, return_block, end_block}; // NOP
    byte code_square_d[] = {1/*locals_count*/, 1/* of type: */, f64t, get_local, 0, get_local, 0, f64_mul,
                            return_block, end_block};

    byte code_quit[] = {0/*locals_count*/, call_, 0 /*proc_exit*/, end_block};

    // put_chars(char*) compatible with put_chars(chars,len)??
//    byte put_chars[] = {1/*locals_count*/, 1 /*one of type*/, int32 /* (char*) */,
//                        local_get, 0,// char* ⚠️ use puts for string& / char** / char* with header
//                        i32_const, 8, // $temp
//                        i32_store, 2 , 0,
//                        i32_const, 1,// stdout
//                        i32_const, 8, // $temp
//                        i32_const, 1,// #string
//                        i32_const, 8,// out chars written => &trash
//                        call_, (byte) fd_write_import, nop, nop,
//                        local_get, 0, return_block,// return string*  HAS to return something according to stdio
//                        end_block};

    // put_string(string&) / put_string(char**)
    byte code_put_string[] = {1/*locals_count*/, 1 /*one of type*/, int32 /* string& */ ,
                              i32_const, 1,// stdout
                              local_get, 0,// string* or char** ⚠️ use put_chars for char*
                              i32_const, 1,// #string
                              i32_const, 8,// out chars written => &trash
                              call_, (byte) fd_write_import, nop, nop,
                              end_block};

    // char* in wasp abi always have header at -8
    byte code_puts[] = {1/*locals_count*/, 1 /*one of type*/, int32 /* string& */ ,
                        i32_const, 1,// stdout
                        local_get, 0,// string* or char** ⚠️ use put_chars for char*
                        i32_const, 8, i32_sub,//  char* in wasp abi always have header at -8
                        i32_const, 1,// #string
                        i32_const, 8,// out chars written => &trash
                        call_, (byte) fd_write_import, nop, nop,
                        end_block};


    byte code_len[] = {1/*locals_count*/, 1 /*one of type*/, int32 /* wasm_pointer */ ,
                       local_get, 0,// any structure in Wasp ABI
                       i32_const, 4,// length is second field in ALL Wasp structs!
                       i32_add, // offset = base + 4
                       i32_load, 2, 0, end_block};

    // slightly confusing locals variable declaration count scheme:
    byte code_modulo_float[] = {1 /*locals declarations*/, 2 /*two of type*/, float32,
                                0x20, 0x00, 0x20, 0x00, 0x20, 0x01, 0x95, 0x8f, 0x20, 0x01, 0x94, 0x93, 0x0b};
    byte code_modulo_double[] = {1 /*locals variable declarations:*/, 2 /*two of type*/, float64,
                                 0x20, 0x00, //                     | local.get 0
                                 0x20, 0x00, //                     | local.get 0
                                 0x20, 0x01, //                     | local.get 1
                                 0xa3,       //                     | f64.div
                                 0x9d,       //                     | f64.trunc
                                 0x20, 0x01, //                     | local.get 1
                                 0xa2,       //                     | f64.mul
                                 0xa1,       //                     | f64.sub
                                 0x0b        //                     | end
    };
    Code code_blocks;

    if (runtime.code_count == 0) {
        // order matters, in functionType section!
//        if (functions["nop"].is_used)// NOT a function
//            code_blocks = code_blocks + encodeVector(Code(code_nop, sizeof(code_nop)));
        if (functions["square_double"].is_used and functions["square_double"].is_builtin)
            // simple test function x=>x*x can also be linked via runtime/import!
            code_blocks = code_blocks + encodeVector(Code(code_square_d, sizeof(code_square_d)));
        if (functions["id"].is_used)
            code_blocks = code_blocks + encodeVector(Code(code_id, sizeof(code_id)));
        if (functions["modulo_float"].is_used)
            code_blocks = code_blocks + encodeVector(Code(code_modulo_float, sizeof(code_modulo_float)));
        if (functions["modulo_double"].is_used)
            code_blocks = code_blocks + encodeVector(Code(code_modulo_double, sizeof(code_modulo_double)));
        if (functions["len"].is_used)
            code_blocks = code_blocks + encodeVector(Code(code_len, sizeof(code_len)));
        if (functions["puts"].is_used) // calls import fd_write, can be import itself
            code_blocks = code_blocks + encodeVector(Code(code_puts, sizeof(code_puts)));
        if (functions["put_string"].is_used) // calls import fd_write, can be import itself
            code_blocks = code_blocks + encodeVector(Code(code_put_string, sizeof(code_put_string)));
        if (functions["quit"].is_used)
            code_blocks = code_blocks + encodeVector(Code(code_quit, sizeof(code_quit)));
    }

    Code main_block = emitBlock(root, functions["main"]);// after imports and builtins

    if (start) {
        if (main_block.length == 0)
            functions[start].is_used = false;
        else {
            code_blocks = code_blocks + encodeVector(main_block);
            if (functions["_start"].is_used and functions["_start"].is_builtin)
                code_blocks = code_blocks + encodeVector(Code(code_start, sizeof(code_start)));
        }
    } else {
        if (main_block.length > 5)
            error("no start context name given. null instead of 'main', can't assign block");
        else warn("no start block (ok)");
    }
    for (String fun: functionCodes) {// MAIN block extra ^^^
        Code &func = functionCodes[fun];
        code_blocks = code_blocks + encodeVector(func);
    }
    builtin_count = 0;
    for (auto name: functions) {
        Function &context = functions[name];
        if (context.is_builtin and context.is_used) builtin_count++;
    }

    bool has_main = start and functionIndices.has(start);
    int function_codes = functionCodes.size();
    function_block_count = has_main /*main*/ + builtin_count + function_codes;
    auto codeSection = createSection(code_section, Code(function_block_count) + code_blocks);
    return codeSection.clone();
}

[[nodiscard]]
Code emitExportSection() {
//    https://webassembly.github.io/spec/core/binary/modules.html#binary-exportsec
    short exports_count = 1;// just main … todo easy
// the export section is a vector of exported functions etc
    if (!start)// todo : allow arbirtrary exports, or export all
        return createSection(export_section, Code(0));
    int main_offset = 0;
    if (functionIndices.has(start))
        main_offset = functionIndices[start];
    Code memoryExport;// empty by default
    if (memoryHandling == export_memory) {
        exports_count++;
        memoryExport = encodeString("memory") + (byte) mem_export + Code(0);
//code = code + createSection(export_section, encodeVector(Code(exports_count) + memoryExport));
    }
    auto use_wasi = true;
    if (use_wasi) {
//        exports_count++;
//        start = "_start";
    }
    Code mainExport = encodeString(start) + (byte) func_export + Code(main_offset);
    if (use_wasi) {
        exports_count++;
        int start_offset = main_offset;
        if (functionIndices["_start"])
            start_offset = functionIndices["_start"];
        mainExport = mainExport + encodeString("_start") + (byte) func_export + Code(start_offset);
    }


    Code globalExports;
    for (int i = 0; i < globals.size(); i++) {
        String &name = globals.keys[i];
        Code globalExport = encodeString(name) + (byte) global_export + Code(i);
        globalExports.add(globalExport); // todo << NOW
        exports_count++;
    }
    Code exportsData = encodeVector(
            Code(exports_count) + mainExport + memoryExport + globalExports);

    auto exportSection = createSection(export_section, exportsData);
    return exportSection;
}

int global_user_count = 0;

[[nodiscard]]
Code emitGlobalSection() {
    // global imports purely in IMPORT section
    // user global index += global_import_count !
    //referenced through global indices, starting with the smallest index not referencing a global import.
    global_user_count = globals.count();
    Code globalsList;
    globalsList.add(global_user_count);
    /* example:
    globalsList.addByte(0x01);// global_user_count
    globalsList.addByte(int32);// value type of expression
    globalsList.addByte(0x00);// mutable?
    globalsList.addConst(9); // complicated initialization blocks allowed!!
    globalsList.addByte(end_block);
    */
//	last_type = int32;
    for (int i = 0; i < global_user_count; i++) {
        String global_name = globals.keys[i];
        Node *global_node = globals.values[i];
        if (not global_node) {
            warn("missing init for global "s + global_name);
            global_node = new Node();// dummy
        }
        if ((*global_node)["import"])continue;
//		Type type = global_node->kind;
        Valtype valtype = mapTypeToWasm(*global_node);
        globalTypes.insert_or_assign(global_name, valtype);
        globalsList.addByte(valtype);
        globalsList.addByte(0);// 1:mutable todo: default? not π ;)
        // expression set in analyse->groupOperators  if(name=="::=")globals[prev.name]=&next;
        const Code &globalInit = emitExpression(global_node,
                                                *new Function{.name="global"});// todo ⚠️ global is not a context!
        globalsList.add(globalInit);// todo names in global context!?
        globalsList.addByte(end_block);
        /*
        switch (type) {
            case longs:
                globalsList.addConst(global_node->value.longy);
                break;
            case reals:
                globalsList.addConst(global_node->value.real);
                break;
            default:
                error("Missing globals export for type "s + typeName(type));
        }
                */
    }
    last_type = none; // don't leak!
    auto globalSection = createSection(global_section, globalsList);
    return globalSection;
}

[[nodiscard]]
Code emitDataSection() { // needs memory section too!
//https://webassembly.github.io/spec/core/syntax/modules.html#syntax-datamode
    Code datas;
    if (data_index_end == 0 or data_index_end == runtime_data_offset)return datas;//empty
// see clearEmitterContext() for NULL PAGE of data_index_end
    datas.addByte(01);// one memory initialization / data segment
    datas.addByte(00);// memory id always 0 until multi-memory

    datas.addByte(0x41);// opcode for i32.const offset: followed by unsignedLEB128 value:
    datas.addInt(runtime_data_offset ? runtime_data_offset : runtime.data_offset_end); // actual offset in memory
    // todo: WHY cant it start at 0? wx  todo: module offset + module data length
    datas.addByte(0x0b);// mode: active?
    auto size_of_data = data_index_end - runtime_data_offset;
    datas.addInt(size_of_data);
//    const Code &actual_data = Code((bytes) data, size_of_data);
// todo: WASTEFUL but clean, add/substruct offsets everywhere would be unsafe
    const Code &actual_data = Code((bytes) data + runtime_data_offset, size_of_data);
    datas.add(actual_data);// now comes the actual data  encodeVector()? nah manual here!
    return createSection(data_section, encodeVector(datas));// size added via actual_data
}

// Signatures
[[nodiscard]]
Code emitFuncTypeSection() {// depends on codeSection, but must appear earlier in wasm
//    https://webassembly.github.io/spec/core/binary/modules.html#binary-funcsec

    // funcType_count = function_count EXCLUDING imports, they encode their type inline!
    // the context section is a vector of type indices that indicate the type of each context in the code section

    Code types_of_functions = Code(function_block_count);//  = Code(types_data, sizeof(types_data));
//	order matters in functionType section! must be same as in functionIndices
    for (int i = 0; i < function_block_count; ++i) {
        //	import section types separate WTF wasm
        int index = i + import_count + runtime_function_offset;
        String *fun = functionIndices.lookup(index);
        if (!fun) {
            print(functionIndices);
            error("missing typeMap for index "s + index);
        } else {
            int typeIndex = typeMap[*fun];
            if (typeIndex >= 0) // just an implicit list funcId->typeIndex
                types_of_functions.push((int) typeIndex, false);
            else if (runtime_function_offset == 0) // todo else ASSUME all handled correctly before
                error("missing typeMap for context %s index %d "s % fun % i);
        }
    }
    // @ WASM : WHY DIDN'T YOU JUST ADD THIS AS A FIELD IN THE FUNC STRUCT???
    Code funcSection = createSection(functypes_section, types_of_functions);
    return funcSection.clone();
}

[[nodiscard]]
Code functionSection() {
    return emitFuncTypeSection();// (misnomer) vs codeSection() !
}

// todo : convert library referenceIndices to named imports!
[[nodiscard]]
Code emitNameSection() {
    Code nameMap;

    int total_func_count = last_index + 1;// imports + function_count, all receive names
    int usedNames = 0;
    for (int index = runtime_function_offset; index < total_func_count; index++) {
        // danger: utf names are NOT translated to wat env.√=√ =>  (import "env" "\e2\88\9a" (func $___ (type 3)))
        String *name = functionIndices.lookup(index);
        if (not name)continue;// todo: no name bug (not enough mem?)
        if (functions[*name].is_import and runtime_function_offset > 0)continue;
        nameMap = nameMap + Code(index) + Code(*name);
        usedNames += 1;
    }

//	auto functionNames = Code(function_names) + encodeVector(Code(1) + Code((byte) 0) + Code("logi"));
//	functions without parameters need  entry ( 00 01 00 00 )
//  functions 5 with local 'hello' :  05 01 00 05 68 65 6c 6c 6f
//  functions 5 with local 'hello' :  05 02 00 05 68 65 6c 6c 6f 01 00 AND unnamed (local i32t)
// localMapEntry = (index nrLocals 00? string )

    Code localNameMap;
    int usedLocals = 0;
    for (int index = runtime_function_offset; index <= last_index; index++) {
        String *key = functionIndices.lookup(index);
        if (!key or key->empty())continue;
        Function &context = functions[*key];
//        List<String> localNames = context.locals[*key];// including arguments
        int local_count = context.locals.size();
        if (local_count == 0)continue;
        usedLocals++;
        localNameMap = localNameMap + Code(index) + Code(local_count); /*???*/
        for (int i = 0; i < context.locals.size(); ++i) {
            String local_name = context.locals.at(i).name;
            localNameMap = localNameMap + Code(i) + Code(local_name);
        }
//		error: expected local name count (1) <= local count (0) FOR FUNCTION ...
    }

    Code globalNameMap;
    int usedGlobals = globals.count();// currently all
    for (int i = 0; i < globals.count(); i++) {
        String &globalName = globals.keys[i];
        globalNameMap = globalNameMap + Code(i) + Code(globalName);
    }



//	localNameMap = localNameMap + Code((byte) 0) + Code((byte) 1) + Code((byte) 0) + Code((byte) 0);// 1 unnamed local
//	localNameMap = localNameMap + Code((byte) 4) + Code((byte) 0);// no locals for function4
//	Code exampleNames= Code((byte) 5) /*context index*/ + Code((byte) 1) /*count*/ + Code((byte) 0) /*l.nr*/ + Code("var1");
    // context 5 with one local : var1

//	localNameMap = localNameMap + exampleNames;

    auto moduleName = Code(module_name) + encodeVector(Code("wasp_module"));
    auto functionNames = Code(function_names) + encodeVector(Code(usedNames) + nameMap);
    auto localNames = Code(local_names) + encodeVector(Code(usedLocals) + localNameMap);
    auto globalNames = Code(global_names) + encodeVector(Code(usedGlobals) + globalNameMap);
    auto dataNames = Code(data_names) + encodeVector(Code(1)/*count*/ + Code(0) /*index*/ + Code("wasp_data"));


//	The name section is a custom section whose name string is itself ‘𝚗𝚊𝚖𝚎’.
//	The name section should appear only once in a module, and only after the data section.
    const Code &nameSectionData = encodeVector(
            Code("name") + moduleName + functionNames + localNames + globalNames + dataNames);
    // global names are part of global section, as should be
    auto nameSection = createSection(custom_section, nameSectionData); // auto encodeVector AGAIN!
    nameSection.debug();
    return nameSection.clone();
}

//Code dataSection() {
//	return Code();
//}

[[nodiscard]]
Code eventSection() {
    return Code();
}

/*
 *  There are currently two ways in which context indices are stored in the code section:
    Immediate argument of the call instruction (calling a context)
    Immediate argument of the i32.const instruction (taking the address of a context).
    The immediate argument of all such instructions are stored as padded LEB128 such that they can be rewritten
    without altering the size of the code section. !
    For each such instruction a R_WASM_FUNCTION_INDEX_LEB or R_WASM_TABLE_INDEX_SLEB reloc entry is generated
    pointing to the offset of the immediate within the code section.

    R_WASM_FUNCTION_INDEX_LEB relocations may fail to be processed, in which case linking fails.
    This occurs if there is a weakly-undefined context symbol, in which case there is no legal value that can be
    written as the target of any call instruction. The frontend must generate calls to undefined weak symbols
    via a call_indirect instruction.
*/
[[nodiscard]]
Code linkingSection() {
//	https://github.com/WebAssembly/tool-conventions/blob/master/Linking.md#linking-metadata-section
    short version = 2;
    Code subsection;
    short type = 5;// SEGMENT alignment & flags
    short payload_len = 0;
    Code payload_data;
    subsection.add(type).add(payload_len).push(payload_data);
    Code subsections;
/*
	5 / WASM_SEGMENT_INFO - Extra metadata about the data segments.
	6 / WASM_INIT_FUNCS - Specifies a list of constructor functions to be called at startup. Called after memory has been initialized.
	7 / WASM_COMDAT_INFO - Specifies the COMDAT groups of associated linking objects, which are linked only once and all together.
	8 / WASM_SYMBOL_TABLE - Specifies extra information about the symbols present in the module.
// https://en.wikipedia.org/wiki/Relocatable_Object_Module_Format
// http://wiki.dwarfstd.org/index.php?title=COMDAT_Type_Sections
	COMDAT - (C2h/C3h) Initialized common data
	COMDEF - (B0h) Uninitialized common data
 Thread-agnostic objects can be safely linked with objects that do or do not use atomics, although not both at the same time.
*/
    return createSection(custom_section, encodeVector(Code("linking") + Code(version) + subsections));
}

[[nodiscard]]
Code dwarfSection() {
    return createSection(custom_section, encodeVector(Code("external_debug_info") + Code("main.dwarf")));
}

/*
 * The generally accepted features are:
    atomics
    bulk-memory
    exception-handling
    multivalue
    mutable-globals
    nontrapping-fptoint
    sign-ext
    simd128
    tail-call
 */

// see preRegisterSignatures
void add_builtins() {
    import_count = 0;
    builtin_count = 0;
    for (auto sig: functions) {// imports first
        Function &context = functions[sig];
        if (context.is_import and context.is_used) {
            functionIndices[sig] = ++last_index;
            if (context.code_index >= 0 and context.code_index != last_index)
                error("context already has index %d ≠ %d"s % context.code_index % last_index);
            context.code_index = last_index;
            import_count++;
        }
    }
    for (auto sig: functions) {// now builtins
        Function &function = functions[sig];
        if (function.is_builtin and function.is_used) {
            functionIndices[sig] = ++last_index;
            if (function.code_index >= 0 and function.code_index != last_index)
                werror("function %s already has index %d ≠ %d"s % function.name % function.code_index % last_index);
            function.code_index = last_index;
            builtin_count++;
        }
    }
}

[[nodiscard]]
Code emitMemorySection() {
    if (memoryHandling == import_memory or memoryHandling == no_memory) return Code();// handled elsewhere
    /* limits https://webassembly.github.io/spec/core/binary/types.html#limits - indicates a min memory size of one page */
//	int pages = 1;//  traps while(i<65336/4)k#i=0
    int pages = 1024 * 10;// 64kb each, 65336 pages max. makes VM slower?
    auto code = createSection(memory_section, encodeVector(Code(1) + Code(0x00) + Code(pages)));
    return code;
}

// todo: merge with
void clearEmitterContext() {
//	clearAnalyzerContext(); NO keep it at times!
    referenceMap.clear();
    referenceIndices.clear();
    referenceDataIndices.clear();
    referenceNodeIndices.clear();
    functionCodes.clear();
    functionIndices.setDefault(-1);
    functionIndices.clear();// ok preregistered functions are in functions
//	functionCodes.setDefault(Code());
    typeMap.setDefault(-1);
    typeMap.clear();
//	referenceMap.setDefault(Node());
//    runtime_data_offset = 0x100000;
    data_index_end = runtime_data_offset; //0
    last_object_pointer = 0;
    if (!data) data = (char *) malloc(MAX_WASM_DATA_LENGTH);// todo grow
    else memset(data, 0, MAX_WASM_DATA_LENGTH);
    emitLongData(0, true);// NULL PAGE! no object shall ever read or write from address 0 (sanity measure)
    emitLongData(0, true);// TRASH sink, e.g. for writing fd_write(fd,iov*,len, &trash out)
//    emitString(*new Node("__WASP_DATA__\0"), *new Function());
//    while (((long) data) % 8)data++;// type 'long', which requires 8 byte alignment
}

[[nodiscard]]
Code &emit(Node &root_ast, Module *runtime0, String _start) {
    start = _start;
    if (runtime0) {
//		memoryHandling = no_memory;// done by runtime?
//		memoryHandling = export_memory;// try to combine? duplicate export name `memory` already defined
//        memoryHandling = import_memory;// works
        memoryHandling = internal_memory;
        runtime = *runtime0;// else filled with 0's
        runtime_function_offset = runtime.import_count + runtime.code_count;//  functionIndices.size();
        import_count = 0;
        builtin_count = 0;
        //		data_index_end = runtime.data_offset_end;// insert after module data!
        // todo: either write data_index_end DIRECTLY after module data and increase count of module data,
        // or increase memory offset for second data section! (AND increase index nontheless?)
//		int newly_pre_registered = 0;//declaredFunctions.size();
        last_index = runtime_function_offset - 1;
    } else {
        memoryHandling = export_memory;
//#ifdef IMPORT_MEMORY
//        memoryHandling = import_memory; // works for micro-runtime
//#endif
//        memoryHandling = internal_memory; // works for wasm3
//        memoryHandling = no_memory;
        last_index = -1;
        runtime = *new Module();// all zero
        runtime_function_offset = 0;
        add_builtins();
    }
    if (start) {// now AFTER imports and builtins
//		printf("start: %s\n", start.data);
//		functions[start] = Signature().returns(i32t);
        functions[start].emit = true;
        if (!functionIndices.has(start))
            functionIndices[start] = ++last_index;
//			functionIndices[start] =runtime_offset ? runtime_offset + declaredFunctions.size() :  ++last_index;  // AFTER collecting imports!!
        else
            error("start already declared: "s + start + " with index " + functionIndices[start]);
        if (start != "_start" and not functions.has("_start")) {
            functions["_start"] = {.code_index=++last_index, .name="_start", .is_builtin=true, .is_used=true};
            functionIndices["_start"] = last_index;
        }
    } else {
//		functions["_default_context_"] = Signature();
//		start = "_default_context_";//_default_context_
//		start = "";
    }

    const Code customSectionvector;
//	const Code &customSectionvector = encodeVector(Code("custom123") + Code("random custom section data"));
    // ^^^ currently causes malloc_error WHY??

    auto customSection = createSection(custom_section, customSectionvector);
    Code typeSection1 = emitTypeSection();// types must be defined in analyze(), not in code declaration
    Code importSection1 = emitImportSection();// needs type indices
    Code globalSection1 = emitGlobalSection();//
    Code codeSection1 = emitCodeSection(root_ast); // needs functions and functionIndices prefilled!! :(
    Code funcTypeSection1 = emitFuncTypeSection();// signatures depends on codeSection, but must come before it in wasm
    Code memorySection1 = emitMemorySection();
    Code exportSection1 = emitExportSection();// depends on codeSection, but must come before it!!

    Code code = Code(magicModuleHeader, 4)
                + Code(moduleVersion, 4)
                + typeSection1
                + importSection1
                + funcTypeSection1 // signatures
                + memorySection1 // Wasm MVP can only define one memory per module WHERE?
                + globalSection1
                + exportSection1
                + codeSection1 // depends on importSection, yields data for funcTypeSection!
                + emitDataSection()
                //			+ linkingSection()
                + emitNameSection()
//	 + dwarfSection() // https://yurydelendik.github.io/webassembly-dwarf/
//	 + customSection
    ;
    code.debug();
#ifndef WEBAPP
//	free(data);// written to wasm code ok
#endif
//	if (runtime0)
//		functions.clear(); // cleanup after NAJA
    return code.clone();
}
//
//Code emit(String code){
//	error("DEPRECATED");
//	return 0;
//}


//extern "C"
Code &compile(String code, bool clean) {
    if (clean) {
        clearEmitterContext();
        clearAnalyzerContext();// needs to be outside analyze, because analyze is recursive
    }

    Node parsed = parse(code);

    Node &ast = analyze(parsed, functions["main"]);
    functions["fd_write"].signature.wasm_return_type = int32;
//	preRegisterSignatures();// todo remove after fixing Signature BUG!!
//	check(functions["log10"].is_import)
//	check(functions["log10"].is_used)
    Code &binary = emit(ast);
    binary.save("main.wasm");

#ifdef INCLUDE_MERGER
    if (libraries.size() > 0) {
        binary.needs_relocate = true;
        List<Code *> merge_module_binaries;
        for (Module *library: libraries)
            merge_module_binaries.add(&library->code);
        merge_module_binaries.add(&binary);
        Code &merged = merge_binaries(merge_module_binaries);
        merge_module_binaries.clear();
        return merged;
    }
//	if (merge_module_binaries.size() > 0) {
//		merge_module_binaries.add(binary);
//		Code &merged = merge_binaries(merge_module_binaries);
//		merge_module_binaries.clear();
//		return merged;
//	}
#else
    if (libraries.size() > 0)
        warn("wasp compiled without binary linking/merging. set(INCLUDE_MERGER 1) in CMakeList.txt");
#endif
    return binary;
}


#pragma clang diagnostic pop
