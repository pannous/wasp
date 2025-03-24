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
extern Map<String, Global> globals;

//Map<String, Signature> functions;// todo Signature copy by value is broken
Code emitString(Node &node, Function &context);

Code emitString(const String &text, Function &context);

Code emitArray(Node &node, Function &context);

Map<int, uint> sourceMap; // line number to wasm offset

//int runtime_data_offset = 0;// 0x10000;
int runtime_data_offset = 0x10000; // prevent memory clash with runtime.
int runtime_function_offset = 0; // imports + funcs
int import_count = 0;
short builtin_count = 0; // function_offset - import_count - runtime_offset;

//bytes data;// any data to be stored to wasm: values of variables, strings, node_pointer etc
char *data; // any data to be stored to wasm: values of variables, strings, node_pointer etc ( => memory in running app)
//int data_index_start = 0;
int data_index_end = 0;
// position to write more data = end + length of data section when building ≠ heap_end in live app
int last_object_pointer = 0; // outside stack
//int last_data_pointer = 0;// last_data plus header , see referenceDataIndices
// todo use these for named data e.g. named strings and arrays
int named_data_segments = 0; // first is just implicit "wasp_data" zero page
List<int> data_segment_offsets; // todo redundant with referenceIndices
List<String> data_segment_names;

//Map<String *, int64> referenceDataIndices; // wasm pointers to strings within wasm data WITHOUT runtime offset!
// todo: put into Function.locals :
typedef int nodehash;
Map<int64, int> referenceNodeIndices; // wasm pointers to node_pointer
Map<String, int64> referenceIndices; // wasm pointers to objects (currently: arrays?) within wasm data
Map<String, int64> referenceDataIndices; // wasm pointers directly to object data, redundant ^^ TODO REMOVE
Map<String, Node> referenceMap; // lookup types… todo: Node pointer? or copy ok?
Map<String, int> typeMap; // wasm type index for funcTypeSection. todo keep in Function
Map<String, int> call_indices; // todo keep in Function
Map<String, Code> functionCodes; // EXCLUDING MAIN todo keep in Function
//Map<String, Signature> functions;// for funcs AND imports, serialized differently (inline for imports and extra functype section)

//Map<int64,int> dataIndices; // wasm pointers to strings etc (key: hash!)  within wasm data

//Map<String, Valtype> return_types;
//Map<int, List<String>> locals;
//Map<int, Map<int, String>> locals;
//List<String> declaredFunctions; only new functions that will get a Code block, no runtime/imports
//List<Function> imports;// from libraries. todo: these are inside functions<> for now!

String start = "wasp_main";

Type arg_type = voids; // autocast if not int
//Type last_type = voids;// autocast if not int
Type last_type = Kind::unknown;
Type last_value_type = Kind::unknown; // array<type>
Node *last_object = 0;

enum MemoryHandling {
    import_memory,
    export_memory,
    internal_memory, // declare but don't export
    no_memory,
};

MemoryHandling memoryHandling; // set later = export_memory; // import_memory not with mergeMemorySections!

// todo: gather all 'private' headers here, not in between and in wasm_emitter.h
// private headers:
Code emitConstruct(Node &node, Function &context);

Code emitGetter(Node &node, Node &field, Function &context);

void discard(Code code) {
}

void discard(Code &code) {
    // nop, to explicitly silence functions declared with 'nodiscard' attribute
}

[[nodiscard]]
Code Call(char *symbol); //Node* args


// https://pengowray.github.io/wasm-ops/
// values outside WASM ABI: 0=unknown/handled internally
// todo: norm operators before!
// * ∗ ⋅ ⋆ ✕ ×  …
unsigned short opcodes(chars s, Valtype kind, Valtype previous = none) {
    //	previous is lhs in binops!
    if (kind == string_ref or previous == string_ref) {
        if (eq(s, "+"))return string_concat;
        if (eq(s, " "))return string_concat;
        if (eq(s, "."))return string_concat;
        if (eq(s, "="))return string_eq;
        if (eq(s, "=="))return string_eq;
        if (eq(s, "is"))return string_eq;
        if (eq(s, "~"))return string_eq; // todo: regex
    }
    //	if(eq(s,"$1="))return set_local;
    //	if (eq(s, "=$1"))return get_local;
    //	if (eq(s, "=$1"))return tee_local;
    if (eq(s, "return"))return return_block;
    if (eq(s, "nop") or eq(s, "pass"))
        return nop_;
    if ((Type) kind == unknown_type)
        error("unknown type should be inferred by now");
    if (kind == voids or kind == void_block or kind == i32t) {
        // INT32
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
        if (eq(s, "^"))return 0; // POWER handled on higher level

        if (eq(s, "or"))return i32_or; // i32.or
        //        if (eq(s, "else"))return i32_or; // i32.or
        if (eq(s, "∨"))return i32_or; // i32.or // looks like 'v' but isn't
        if (eq(s, "⋁"))return i32_or; // i32.or
        if (eq(s, "||"))return i32_or; // i32.or  // ≠ norm ‖
        if (eq(s, "|"))return i32_or; // i32.or // todo: pipe is different!

        if (eq(s, "xor"))return i32_xor; // i32.xor
        if (eq(s, "^|"))return i32_xor; // i32.xor //always bitwise todo: truthy 0x0101 xor 0x1010 !?
        if (eq(s, "⊻"))return i32_xor; // i32.xor

        if (eq(s, "not"))return i32_eqz; // i32.eqz  // no such thing as i32_not, but the same if you think about it
        if (eq(s, "¬"))return i32_eqz; // i32.eqz
        if (eq(s, "!"))return i32_eqz; // i32.eqz
    } else if (kind == i64t) {
        // INT32
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
        if (eq(s, "^"))return 0; // POWER handled on higher level

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
        if (eq(s, "~"))return f64_eq; // f32.eq Todo
        if (eq(s, "≈"))return f64_eq; // f32.eq Todo
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

        if (eq(s, "⌊"))return f64_floor; // f64.floor
        if (eq(s, "floor"))return f64_floor; // f64.floor // conflicts with user keywords!
        if (eq(s, "⌋"))return f64_floor; // f64.floor // vs trunc towards 0?

        if (eq(s, "⌈"))return f64_ceil; // f64.ceil
        if (eq(s, "ceil"))return f64_ceil; // f64.ceil
        if (eq(s, "⌉"))return f64_ceil; // f64.ceil

        if (eq(s, "⌊"))return f64_nearest; // f64.nearest
        if (eq(s, "round"))return f64_nearest; // f64.nearest // conflicts with user keywords!
        if (eq(s, "⌋"))return f64_nearest; // f64.nearest
    } else if (kind == f32t) {
        if (eq(s, "not"))return f32_eqz; // f32.eqz  // f32.eqz  // HACK: no such thing!
        if (eq(s, "¬"))return f32_eqz; // f32.eqz  // HACK: no such thing!
        if (eq(s, "!"))return f32_eqz; // f32.eqz  // HACK: no such thing!
        if (eq(s, "+"))return f32_add; // f32.add
        if (eq(s, "-"))return f32_sub; // f32.sub
        if (eq(s, "*"))return f32_mul; // f32.mul
        if (eq(s, "/"))return f32_div; // f32.div
        if (eq(s, "="))return f32_eq; // f32.eq
        if (eq(s, "=="))return f32_eq; // f32.eq
        if (eq(s, "~"))return f32_eq; // f32.eq Todo
        if (eq(s, "≈"))return f32_eq; // f32.eq Todo
        if (eq(s, ">"))return f32_gt; // f32.gt
        if (eq(s, ">="))return f32_ge; // f32.ge
        if (eq(s, "<"))return f32_lt; // f32.lt
        if (eq(s, "<="))return f32_le; // f32.le
    }
    if (eq(s, "√"))return f64_sqrt; // else √42*√42=41.9999
    if (eq(s, "√"))return f32_sqrt; // f32.sqrt
    if (eq(s, "sqrt"))return f32_sqrt; // f32.sqrt
    if (eq(s, "root"))return f32_sqrt; // f32.sqrt // conflicts with user keywords!
    // string addition etc. handled elsewhere!
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

    if (eq(s, "%"))return 0; // handle later

    if (tracing)
        breakpoint_helper
    trace("unknown or non-primitive operator %s\n"s % String(s)); // OK! not (necessarily) a problem:
    // can still be matched as context etc, e.g.  2^n => pow(2,n)   'a'+'b' is 'ab'
    //		error("invalid operator");
    return 0;
}

byte opcodes(chars s, Type kind, Type previous = none) {
    return opcodes(s, mapTypeToWasm(kind), mapTypeToWasm(previous));
}


// http://webassembly.github.io/spec/core/binary/modules.html#export-section
enum ExportType {
    // todo == ExternalKind
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
Code encodeLocal(int64 count, Valtype type) {
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
    while (i--)flip[i] = float_data[i]; // don't flip, just copy to malloc
    return flip;
}

bytes ieee754(double num) {
    char dat[8];
    double *hack = ((double *) dat);
    *hack = num;
    byte *flip = static_cast<byte *>(alloc(1, 9));
    short i = 8;
    while (i--)flip[i] = dat[i]; // don't flip, just copy to malloc
    return flip;
}

//Code emitExpression (Node* node_pointer);

// pure data ready to be emitted
bool isProperList(Node &node) {
    if (node.kind != groups and node.kind != objects) return false;
    if (node.length < 1) return false;
    for (Node &child: node) {
        if (child.kind != longs and child.kind != strings) // todo … evaluate?
            return false;
        if (child.isSetter())
            return false;
    }
    return true;
}

Code emitSimilar(Node &node, Function &context) {
    Code code;
    // a ≈ b <> | a - b | < ε
    //    code.add(emitCall("similar", context));
    todow("emitSimilar a ≈ b <> | a - b | < ε    e.g. π≈3.14159");
    return code;
}

// append padding bytes to wasm data memory
void emitPadding(int num, byte val = 0) {
    while (num-- > 0)
        data[data_index_end++] = val;
}

void emitPaddingAlignment(short size) {
    // e.g. 8 for int64 padding BEFORE emitLongData() / emitData(int64)
    emitPadding((size - (data_index_end % size)) %
                size); // fill up to int64 padding ⚠️ the field-sizes before node.value MUST sum up to n*8!
}


// append byte to wasm data memory
void emitByteData(byte i) {
    *(byte *) (data + data_index_end++) = i;
    last_value_type = byte_i8;
}


// append short to wasm data memory
void emitShortData(short i, bool pad = false) {
    // ⚠️ DON'T PAD INSIDE STRUCTS!?
    if (pad)while (((int64) (data + data_index_end) % 2))data_index_end++; // type 'int' requires 4 byte alignment
    *(short *) (data + data_index_end) = i;
    data_index_end += 2;
    last_value_type = shorty;
}

// append int to wasm data memory
void emitIntData(int i, bool pad = true) {
    if (pad)while (((int64) (data + data_index_end) % 4))data_index_end++; // type 'int' requires 4 byte alignment
    if ((int64) (data + data_index_end) % 4)
        warn("emitIntData unaligned!");
    *(int *) (data + data_index_end) = i;
    data_index_end += 4;
}

// append int64 to wasm data memory
void emitLongData(int64 i, bool pad = false) {
    // ⚠️ DON'T PAD INSIDE STRUCTS! pad before!
    if (pad)while (((int64) (data + data_index_end) % 8))data_index_end++; // type 'int64' requires 8 byte alignment
    if ((int64) (data + data_index_end) % 8)
        warn("emitLongData unaligned!");
    *(int64 *) (data + data_index_end) = i;
    data_index_end += 8;
}

void emitSmartPointer(smart_pointer_64 p) {
    emitLongData(p, true);
}

Code emitWaspString(Node &node, Function &context) {
    // emit node as serialized wasp string
    const String &string = node.serialize();
    const Code &code = emitString(*new Node(string), context);
    context.track(node, code, 0);
    return code;
}


//typedef int64 wasm_node_index;
wasm_node_index emitNodeBinary(Node &node, Function &context) {
    //    if((int64)&node < 0x100000000L)
    //        return (int64)&node; // inside wasm stack context, we can just pass around the wasm_node_index. Todo, but not in compile context!
    int64 hash = node.hash();
    if (referenceNodeIndices.has(hash))
        return referenceNodeIndices[hash];
    else referenceNodeIndices[hash] = -1; // fixup marker for cyclic graphs todo …

    //    if (node.value.longy != 0x01010101) {
    //        node.meta = new Node("Hahaha");
    //        node.meta->setValue(0x01010101);
    //    }

    int wasm_type_pointer = node.type ? emitNodeBinary(*node.type, context) : 0; // just drop smart header
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
    emitLongData(node.value.longy, false); // too late to pad, otherwise
    //    check_is(sizeof(node.kind), 1) // todo
    //    emitByteData(node.kind); // breaks alignment
    emitIntData(node.kind);
    check_is((int) sizeof(node.kind), 4) // forced 32 bit for alignment!
    emitIntData(wasm_meta_pointer);
    emitString(node /*.name*/, context); // directly in place!
    //    emitIntData(wasm_meta_pointer);
    //    emitIntData(wasm_next_pointer);
    //    emitPadding(3*8);// pointers, hash, capacity, … extra fields
    //    emitPaddingAlignment(8);
    last_type = Primitive::node;
    if (node.type)
        last_type = node.type;
    last_object = &node;
    last_object_pointer = node_start;
    tracef("node_start %d data_index_end %d\n", node_start, data_index_end);
    // already stored in emitArray() : usually enough, unless we want extra node metadata?
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


// extern "C" ExternRef createHtml(ExternRef parent /*0*/,chars innerHTML); // html{bold{Hello}} => appendChild bold to body
[[nodiscard]]
Code emitHtml(Node &node, Function &function, ExternRef parent = 0) {
    Code code;
    if (node.name == "html") {
        for (auto &child: node)
            code.add(emitHtml(child, function, parent)); // html is not parent
        return code;
    }
    static ExternRef previous = (void *) -1;
    if (parent == previous); // use previous return as parent (on stack)
    else if (parent)code.add(emitData(*new Node(parent), function));
    else code.add(emitCall("getDocumentBody", function));
    //    else code.addConst32(0); // get document body in js
    code.add(emitString(node, function));
    if (node.kind == strings)
        code.add(emitCall("createHtml", function));
    else {
        code.add(emitString(*new Node("IDK_ID"), function));
        code.add(emitCall("createHtmlElement", function));
    }
    trace(node.name);
    for (auto &child: node) {
        code.add(emitHtml(child, function, previous)); // html is not parent
    }
    return code;
}

[[nodiscard]]
Code emitScript(Node &node, Function &function) {
    Code code;
    printf("emitScript … %s", node.serialize().data);
    if (node.name == "script") {
        if (node.value.data) {
            code.add(emitString(node.value.node->serialize(), function)); // todo : value can't be string RIGHT!?
        }
        for (auto &child: node) {
            const String &text = child.serialize();
            printf("emitScript %s", text.data);
            code.add(emitString(text, function));
        }
    } else {
        printf("emitScript %s", node.name.data);
        code.add(emitString(node, function));
    }
    code.add(emitCall("addScript", function));
    return code;
}


short arrayElementSize(Node &node);

Primitive addTypeFromSize(Node &array, short size);


bool eq(Type a, Type b) {
    return a.value == b.value;
}

extern Map<Type, int> arrayTypes;

[[nodiscard]]
uint arrayTypeIndex(Type value_type) {
    if (not arrayTypes.contains(value_type))
        error("array types must be analyzed before the code section! "s + typeName(value_type));
    return arrayTypes[value_type];
}


Code stringRefLength() {
    Code code;
    //    code.addOpcode(string_measure_utf8);
    code.addOpcode(string_measure_wtf8);
    code.add(1); // memory?
    code.add(1); // encoding 0:utf8 1:wtf8  2:utf16 3:utf32 // todo why is utf8 'unreachable'?
    last_type = i64;
    code.addOpcode((i64_extend_i32_s));
    return code;
}


Code castStringToRef() {
    Code code;
    return code;
}

Code castRefToChars() {
    Code code;
    // expects stringref on stack
    code.addConst32(data_index_end); // heap_end
    //  (string.encode_utf8 $memory $? $encoding (stack: $stref $heap_end)
    code.addOpcode(string_encode_wtf8);
    code.add(1/*wtf8*/); // encoding 0:wtf 1:utf8 2:utf16 3:utf32
    code.add(0); // memory
    code.addOpcode(drop); // string_encode_utf8 returns the number of bytes written, we don't need it
    code.addConst32(data_index_end); // heap_end
    data_index_end += 100; // lol
    last_type = charp;
    return code;
}


Code emitLength(Node &node, Function &context) {
    Code code;
    //    code.addConst32(42);
    //    return code;
    Type type = node.kind;
    if (context.locals.contains(node.name)) {
        Local local = context.locals[node.name];
        type = local.type;
        //        if(not on_stack)
        //        code.addOpcode(local_get);
        //        code.addInt(local.position);
    }
    if (type == stringp or type == strings) {
        if (use_wasm_strings and type == strings)
            return stringRefLength();
        code.addConst32(0x04); // offset to string length in our struct, from data pointer!
        code.addOpcode(i32_sub);
        code.addOpcode(i32_load);
        code.add(0x02); // alignment (?)
        code.add(0x00); //
        //        return emitCall("strlen", context);
    } else if (type.isArray()) {
        auto valueType1 = valueType(type);
        if (use_wasm_arrays) {
            code.addOpcode(arrayLen);
            code.addInt(arrayTypes[valueType1]);
        } else {
            code.addConst32(8); // offset to string length in our struct, from data pointer!
            code.addOpcode(i32_add);
            //			code.addOpcode(i8_load);
            code.addOpcode(i32_load);
            code.add(0);
            code.add(0);
        }
    }
    last_type = i32;
    return code;
}

Code emitWasmArrayGetter(Node &node, Function &context, Local local) {
    Code code;
    Type value_type = last_value_type;
    if (local.type.isGeneric())
        value_type = local.type.generics.value_type;
    uint type_index = arrayTypes[value_type];
    code.addConst32(node.value.longy - 1); // offset TODO use stack!
    code.addOpcode(arrayGet);
    code.addInt(type_index);
    last_type = value_type;
    return code;
}


[[nodiscard]]
Code emitWasmArray(Node &node, Function &context) {
    Code code;
    Type type = preEvaluateType(node,
                                context); // mapTypeToWasm(node.first()); // todo check all elements are of same type
    Type value_type = type.generics.value_type;
    uint type_index = arrayTypeIndex(value_type);
    for (auto &child: node) {
        code.addConst32(child.value.longy); // even for i8!
        // todo : float, string, struct, … :
        //        code.emitValue(child, context);
    }
    auto length = node.size();
    code.addOpcode(arrayInitStatic);
    code.addInt(type_index);
    code.addInt(node.size());

    last_type = Type(Generics{.kind = array, .value_type = (ushort) value_type.value});
    last_type = wasmtype_array;
    last_value_type = value_type;
    return code;
}

// just register the name for custom section here, one per each data-object like string, array, object, …
// only for debugging so add strip option
void addNamedDataSegment(int pointer, Node &node) {
    String name = "data"; // todo add type and counter, e.g. string-1, string-2, … array-1, …
    if (not node.name.empty())
        name = node.name;
    else if (node.parent and not node.parent->name.empty())
        name = node.parent->name;
    //    else todow("all data_segments should have names!");
    //    { // todo: end this segment even if next one not named.
    named_data_segments++;
    data_segment_offsets.add(pointer);
    data_segment_names.add(name); // only for debugging so add strip option
    // todo: un-redundant:
    //        referenceIndices.insert_or_assign(node.name, pointer);
    //        referenceDataIndices.insert_or_assign(node.name, pointer + array_header_length);
    //        referenceMap[node.name] = node;
}

// todo emitPrimitiveArray vs just emitNode as it is (with child*)
[[nodiscard]]
Code emitArray(Node &node, Function &context) {
    //	if (node.kind.type == int_array)

    if (node.kind == buffers)
        return emitPrimitiveArray(node, context);
    // ⚠️careful primitive array (buffer) ≠ array of primitives, which follows …

    if (use_wasm_arrays)
        return emitWasmArray(node, context);

    List<wasm_node_index> children;
    Kind value_kind = node.first().kind;
    for (Node &child: node) {
        if (not isPrimitive(child)) {
            warn("non primitive element forces node emission");
            return Code((int64) emitNodeBinary(node, context), false);
        }
        if (child.kind != value_kind) {
            // todo try coherence lifting e.g. [1 'a' 'ü'] => codepoint[]
            warn("non coherent element forces node emission");
            warn("collection kind %s versus element kind %s "s % typeName(value_kind) % typeName(child.kind));
            return Code((int64) emitNodeBinary(node, context), false);
        }
        children.add(child.value.longy);
    }

    //    Primitive smallest_common_type = ::byte_char;// todo bit for bitvectors?
    //    short itemSize=0;// bit vector
    short itemSize = arrayElementSize(node);
    wasm_node_index typ_index = 0;
    if (!node.type) /*node.type=*/addTypeFromSize(node, itemSize);
    if (node.type) // save meta information about (generic) type!
        typ_index = emitNodeBinary(*node.type, context); // danger! Byte now lives inside wasm!

    let code = Code();
    emitPaddingAlignment(8);
    int pointer = data_index_end;
    addNamedDataSegment(pointer, node);

    //	todo: sync with emitOffset
    emitIntData(array_header_32, false);
    //    todo ⚠️really lose information here? use emitNodeBinary if full representation required
    emitIntData(node.kind, false); // useless, always groups patterns or objects, who cares now?
    //    emitIntData(value_kind, false); // only works in homogenous arrays!
    emitIntData(node.length, false);
    //    emitIntData(stack_Item_Size, false);// reduntant via type
    if (node.type) emitIntData(typ_index, false); // or node_header_32
    else emitIntData(value_kind /*or kind_header_32*/, false); // todo make sure node.type > Kind AS PER Type enum

    bool continuous = true;
    if (!continuous) emitIntData(data_index_end + 4, false); // just emit immediately after

    //    for(wasm_node_index i:children){
    for (Node &child: node) {
        // ok we checked for coherence before
        int64 i = child.value.longy;
        if (itemSize == 1)emitByteData(i);
        else if (itemSize == 2)emitShortData(i);
        else if (itemSize == 4)emitIntData(i, false);
        else if (itemSize == 8)emitLongData(i, false); // ok can even be float64, UNINTERPRETED here
    }

    //    last_value_pointer = data_index_end;
    //	assert_equals((int64) data_index_end, (int64) pointer + array_header_length);
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
    code.addConst32(pointer); // base for future index getter/setter [0] #1
    return code; // pointer
    //	return code.addConst(pointer);// once written to data section, we also want to use it immediately
}

// todo better
Primitive addTypeFromSize(Node &array, short size) {
    Kind kind = array.first().kind;
    //    kind= smallestCommonType(node)
    if (size == 1 and kind == longs)array.type = &ByteType;
    else if (size == 1 and kind == Kind::codepoint1)array.type = &ByteCharType;
    else if (size == 1)array.type = &ByteType;
    else if (size == 2)array.type = &ShortType;
    else if (size == 4 and kind == longs)array.type = &IntegerType;
    else if (size == 4 and kind == Kind::codepoint1)array.type = &CodepointType;
    else if (size == 8 and kind == longs) array.type = &LongType;
    else if (size == 8 and kind == reals) array.type = &DoubleType;
    else
        error("can't infer type from size "s + size);
    return mapTypeToPrimitive(*array.type);
}


Type commonElementType(Node &array) {
    Type kind = array.first().kind;
    if (kind == longs) {
        auto element_size = arrayElementSize(array);
        return addTypeFromSize(array, element_size);
    }
    for (Node &child: array) {
        if (child.kind != kind) {
            warn("non coherent element forces node emission");
            return unknown_type;
        }
    }
    return kind;
}

// premature optimization BAD! but it's so easy;)
// adds type as BAD SIDE EFFECT
short arrayElementSize(Node &node) {
    if (node.type)
        return stackItemSize(*node.type);
    short smallestCommonitemSize = 1; // byte
    for (Node &child: node) {
        if (child.kind == reference)
            return 4; // 8; // or 4 for wasm_node_index  can't be bigger, also don't change type!
        if (!isPrimitive(child))
            return 4; // wasm pointer
        //            error("shouldn't come here:\n!isPrimitive(child)\n"s + child.serialize());
        if (child.kind == bools)continue; // can't be smaller
        if (child.kind == reals) {
            return 8; // can't be bigger, also don't change type!
        }
        if (child.kind == codepoint1) {
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
            uint64 val = std::abs(child.value.longy) * 2; // *2 for signed variants
            //        if(val<=1) bit vector
            //        if(val>1)itemSize = maxi(itemSize,1); // byte
            if (val >= 0x100)smallestCommonitemSize = maxi(smallestCommonitemSize, 2); // short
            else if (val >= 0x10000)smallestCommonitemSize = maxi(smallestCommonitemSize, 4); // ints
            else if (val >= 0x100000000) {
                smallestCommonitemSize = maxi(smallestCommonitemSize, 8); // longs
                break; // can't be bigger
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

int currentStackItemSize(Node &array, Function &context, Type element_type) {
    if (element_type != unknown)
        return stackItemSize(element_type);
    if (array.type)
        return stackItemSize(*array.type);
    else if (array.kind == groups or array.kind == objects or array.kind == patterns)
        return arrayElementSize(array); // todo why AGAIN? why is type lost after emitArray?
    if (array.kind == reference) {
        Local &local = context.locals[array.name];
        // todo: fix up local elsewhere!
        if (not local.ref)
            local.ref = &referenceMap[array.name];
        if (local.ref and not local.typeXX)
            local.typeXX = local.ref->type;
        if (local.typeXX)
            return stackItemSize(*local.typeXX); // or
        return stackItemSize(local.type);
    }
    if (array.kind == strings) return 1; // char for now todo: call String.codepointAt()
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
        base = 0; // headerOffset(array);// reuse for additional header!

    if (offset_pattern.kind == patterns) {
        if (sharp == true)
            error("sharp pattern?");
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
        if (sharp) {
            // todo remove if offset_pattern was static
            code.addConst32(1);
            code.add(i32_sub);
        }
        if (size > 1) {
            code.addConst32(size);
            code.add(i32_mul);
        }
        if (base > 0) {
            // after calculating offset!
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
    if (base_on_stack) // and base>0)
        code.add(i32_add);
    return code;
    // calculated offset_pattern of 0 ususally points to ~6 bytes after Contents of section Data header 0100 4100 0b08
}

[[nodiscard]]
Code emitIndexWrite(Node &array, int base, Node offset, Node value0, Function &context) {
    int size = currentStackItemSize(array, context, unknown);
    Type targetType = last_type;
    //    Valtype targetType = mapTypeToWasm(last_type);
    Code store = emitOffset(array, offset, true, context, size, base, false);

    if (value0.kind == strings) //todo stringcopy? currently just one char "abc"#2=B
    {
        char c = value0.value.string->charAt(0);
        store.addConst32(c);
    } else
        store = store + emitValue(value0, context);
    //	store.add(cast(last_type, valType));
    //	store.add(cast(valType, targetType));
    store.add(cast(last_type, targetType));
    store.addByte(nop_); // reloc padding
    store.addByte(nop_);
    store.addByte(nop_);
    store.addByte(nop_);
    if (size == 1)store.add(i8_store);
    if (size == 2)store.add(i16_store);
    if (size == 4)store.add(i32_store);
    if (size == 8)store.add(i64_store);
    //	The static address offset is added to the dynamic address operand
    store.add(size > 2 ? 0x02 : 0); // alignment (?) "alignment must not be larger than natural"
    store.add(0); // extra offset (why, wasm?)
    //	store.add(base);// extra offset (why, wasm?)
    store.addByte(nop_); // reloc padding
    store.addByte(nop_);
    store.addByte(nop_);
    store.addByte(nop_);

    return store;
    /*  000101: 41 94 08                   | i32.const 1044
        000104: 41 06                      | i32.const 6
        000106: 36 02 00                   | i32.store 2 0 */
}


// "hi"[0]="H"
[[nodiscard]]
Code emitIndexWrite(Node &op, Function &context) {
    // todo offset - 1 when called via #!
    return emitIndexWrite(op["array"], 0, op["offset"], op["value"], context);
}

// "hi"#1='H'
[[nodiscard]]
Code emitPatternSetter(Node &ref, Node offset, Node value, Function &context) {
    String &variable = ref.name;

    if (!context.locals.has(variable))
        error("!! Variable missed by analyzer: "_s + variable);

    Local &local = context.locals[variable];
    last_type = local.type;
    int base = 0;
    if (local.data_pointer)
        base = local.data_pointer;
    if (referenceIndices.has(variable)) {
        base = referenceDataIndices[variable]; // todo?
        ref = referenceMap[variable]; // ref to value!
    }
    if (referenceDataIndices.has(variable))
        base = referenceDataIndices[variable]; // todo?
    //	last_type = mapTypeToWasm(value);
    Code code = emitIndexWrite(ref, base, offset, value, context);
    return code;
}


// assumes value is on top of stack
// todo merge with emitIndexRead !
[[nodiscard]]
Code emitIndexPattern(Node &array, Node &op, Function &context, bool base_on_stack, Type element_type) {
    if (op.kind != patterns and op.kind != longs and op.kind != reference)
        error("op expected in emitIndexPattern");
    if (op.length == 0 and op.kind == reference)return emitGetter(array, op, context);
    if (op.length != 1 and op.kind != longs)
        error("exactly one op expected in emitIndexPattern");
    int base = base_on_stack ? 0 : last_object_pointer + headerOffset(array); // emitting directly without reference
    int size = currentStackItemSize(array, context, element_type);
    Node &pattern = op.first();
    Code load = emitOffset(array, pattern, op.name == "#", context, size, base, base_on_stack);
    load.addByte(nop_); // reloc padding
    load.addByte(nop_);
    load.addByte(nop_);
    load.addByte(nop_);
    if (size == 1)load.add(i8_load); // i32.load8_u
    if (size == 2)load.add(i16_load);
    if (size == 4)load.add(i32_load);
    if (size == 8)load.add(i64_load);
    // memarg offset u32 align u32 DOESNT FIT:!?!
    load.add(size > 2 ? 0x02 : 0); // alignment (?)
    load.add(0x00); // ?
    load.addByte(nop_);
    load.addByte(nop_);
    load.addByte(nop_);
    load.addByte(nop_);

    // careful could also be uint8!
    if (size == 1) {
        if (element_type == stringp or element_type == charp or element_type == byte_char)
            last_type = byte_char;
        else
            last_type = int32t; // even for bool!
    } else if (size <= 4)
        last_type = int32t;
        // last_type = codepoint32;// todo and … bytes not exposed in ABI, so OK?
    else last_type = i64;
    //	if(op.kind==reference){
    //		Node &reference = referenceMap[op.name];
    //		if(reference.element_type){
    //			last_type.clazz = reference.element_type;
    //		}else if(reference.kind==strings)
    //			last_type=
    //			else
    //			last_type=reference.kind;
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
    Node &array = op[0]; // also String: byte array or codepoint array todo
    Node &pattern = op[1];
    int size = currentStackItemSize(array, context, unknown);
    //	if(op[0].kind==strings) todo?
    last_type = arg_type;
    int base; // …
    if (array.kind == reference or array.kind == key) {
        String &ref = array.name;
        auto local = context.locals[ref];
        if (not local.type.isArray())
            error("reference not declared as array type: "s + ref);
        last_type = valueType(local.type);
        if (referenceIndices.has(ref)) // also to strings
            base = referenceDataIndices[ref];
        //		else if (stringIndices.has(&ref))
        //			base += referenceIndices[ref];
        if (referenceMap.has(ref)) {
            Node &reference = referenceMap[ref];
            array = reference;
            //			if(reference.type)
            //			last_type = mapTypeToWasm(*reference.type);
            ////			last_type.clazz
        }

        if (use_wasm_arrays and local.type.isArray()) {
            Code code;
            code.addOpcode(get_local);
            code.addByte(local.position); // base location stored in variable!
            return code + emitWasmArrayGetter(pattern, context, local);
        }
        //		else error("reference should be mapped");
    } else if (array.kind == strings) {
        String *string = array.value.string;
        if (string)
            base = referenceDataIndices[*string];
    } else {
        if (not base_on_stack) todo("reference array (again)");
        base = last_object_pointer; // + headerOffset(array);// todo: pray!
    }

    Code load;
    if (offset_on_stack and base_on_stack) {
        // offset never precalculated, RIGHT?
        if (op.name == "#") {
            // todo: this is stupid code duplication of emitOffset!
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
    load.addByte(nop_);
    load.addByte(nop_); // reloc padding
    load.addByte(nop_);
    load.addByte(nop_);
    if (size == 1)load.add(i8_load);
    if (size == 2)load.add(i16_load);
    if (size == 4)load.add(i32_load);
    if (size == 8)load.add(i64_load);
    load.add(size > 2 ? 0x02 : 0); // alignment (?)
    load.add(0x00); // ?
    load.addByte(nop_); // reloc padding
    load.addByte(nop_);
    load.addByte(nop_);
    load.addByte(nop_);

    //	if (size == 1)last_type = codepoint32;// todo only if accessing codepoint1, not when pointing into UTF8 byte!!
    if (size == 1) {
        last_type = byte_char;
        last_type = int32t; // ! even bool is represented as int in wasm!!!
    } //last_type = byte_char;
    else if (size <= 4)last_type = int32t;
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
    Code code; // POINTER to DATA SEGMENT
    // (data "\ff\33\01\00")
    // (data (;1;) (i32.const 10) "⚠️segment# and offset CAN OVERLAP! ⚠️ \00")
    int last_pointer = data_index_end;
    addNamedDataSegment(last_pointer, node);
    switch (node.kind) {
        case nils: // also 0, false
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
                error("true int64 big ints currently not supported");
                *(int64 *) (data + data_index_end) = node.value.longy;
                data_index_end += 8;
                last_type = i64t;
            } else {
                *(int *) (data + data_index_end) = node.value.longy;
                data_index_end += 4;
                last_type = int32t;
            }
            break;
        case reals:
            //			bytes varInt = ieee754(node.value.real);
            *(double *) (data + data_index_end) = node.value.real;
            data_index_end += 8;
            last_type = float64t;
            break;
        case reference:
            if (context.locals.has(name))
                return emitNode(node, context);
                //                error("locals dont belong in emitData!");
            else if (referenceIndices.has(name)) todo("emitData reference makes no sense? "s + name) else
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
        case referencex: {
            Node calle("getElementById"); // $
            calle.add(Node(name));
            return emitCall(calle, context);
        }
        case key:
        case patterns:
        default:
            error("emitData unknown type: "s + typeName(node.kind));
    }
    // todo: ambiguity emit("1;'a'") => result is pointer to [1,'a'] or 'a' ? should be 'a', but why?
    last_object_pointer = last_pointer;
    return code.addConst32(last_pointer); // redundant with returns ok
}

List<String> wasm_strings;

Code emitStringRef(Node &node, Function &context) {
    Code code;
    wasm_strings.add(*node.value.string);
    code.addOpcode(string_const);
    code.addInt(1); // memory index?
    code.addInt(wasm_strings.size() - 1); // string index
    last_type = string_ref;
    return code;
}

Code emitString(const String &text, Function &context) {
    return emitString((new Node(text))->setType(strings), context); // todo: ?
    //    return emitStringRef(*new Node(text), context);
}

Code emitString(Node &node, Function &context) {
    if (node.kind != strings)
        return emitString(node.name, context);
    if (not node.value.string)
        error("empty node.value.string");
    //    emitPadding(data_index_end % 4);// pad to int size, too late if in node struct!

    int last_pointer = data_index_end;
    String &string = *node.value.string;

    addNamedDataSegment(last_pointer, node.parent and not node.parent->name.empty() ? *node.parent : node);
    referenceMap[string] = node;
    if (string and referenceIndices.has(string)) {
        // todo: reuse same strings even if different pointer, aor make same pointer before
        last_object_pointer = referenceIndices[string];
        return Code().addConst32(last_object_pointer);
    }
    bool as_c_io_vector = true;
    if ((Primitive) node.kind == leb_string) {
        Code lens(string.length); // wasm abi to encode string as LEB-length + data:
        strcpy2(data + data_index_end, (char *) lens.data, lens.length);
        data_index_end += lens.length; // unsignedLEB128 encoded length of pString
        // strcpy2 the string later: …
    } else if (as_c_io_vector) {
        // wasp abi:
        emitIntData(data_index_end + 8, false); // char* for ciov, redundant but also acts as checksum
        emitIntData(string.length, false);
    } else {
        // ⚠️ WASP ABI is NOT the same as String, because pointer are 4/8 bit and we ignore other fields!!
        // ⚠️ ANY CHANGE HERE NEEDS TO BE REFLECTED in wasp.js and smartNode (and getField??)
        emitIntData(data_index_end + 20, false); // 32 bit 'pointer'
        emitIntData(string.length, false);
        //        emitIntData(string_header_32, false); WASP ABI is NOT the same as String ^^
        emitIntData(1, false); // iovs len? // todo: know what you are doing!
        //        emitIntData(string.codepoint_count, false);// type + child_pointer in node
        emitLongData(data_index_end + 8, false); // 64 bit POINTER to char[] AGAIN!? which just follows:
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

    if (use_wasm_strings) // prepend length via data instead of string_const
        return Code().addInt(string.length).addInt(chars_start).addOpcode(string_new_wtf8);

    return Code().addConst32(chars_start); // direct data!
}

[[nodiscard]]
Code emitGetGlobal(Node &node /* context : global ;) */) {
    Code code;
    String &name = node.first().name;
    auto global = globals[name];
    int i = globals.position(name);
    if (i < 0)
        error("cant find global with name "s + name);
    if (i > globals.size())
        error("invalid index for global with name "s + name);
    check_eq_or(i, global.index, "global index mismatch")
    check_eq_or(global.name, name, "global name mismatch")
    code.addByte(global_get);
    code.addByte(i);
    last_type = global.type;
    return code;
}

// print value on stack (vs emitData)
// todo: last_type not enough if operator left≠right, e.g. ['a']#1  2.3 == 4 ?
[[nodiscard]]
Code emitValue(Node &node, Function &context) {
    Code code;
    String &name = node.name;
    if (node.value.node)
        last_type = node.kind; // careful Kind::strings should map to Classes::c_string after emission!
    last_object = &node; // careful, could be on stack!
    // code.push(last_type) only on return statement
    switch (node.kind) {
        case nils: // also 0, false
            code.addByte((byte) i32_auto); // nil is pointer
            code.push((short) 0);
            break;
        case bools:
            //		case ints:
            code.addByte((byte) i32_auto);
            code.push(node.value.longy); // LEB encoded!
            last_type = i32t;
            break;
        case longs:
            // todo: ints vs longs!!!
            if (context.name == "global") {
                // hack to allow bigger values later because we can't cast in global init!
                code.addByte((byte) i64_const);
            } else if (node.value.longy <= 0x10000000 and node.value.longy > -0x100000000) {
                // room for smart pointers
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
            last_type = f64t; // auto cast return!
            code.addByte((byte) f64_const);
            code.push(ieee754(node.value.real), 8);
        //			code.push(ieee754(node.value.real), 8);
            break;
        //		case identifier:
        case global:
        //            return emitGetGlobal(node);
        case reference: {
            if (not knownSymbol(name, context))
                error("UNKNOWN symbol "s + name + " in context " + context);
            if (node.value.node) {
                if (name == node.value.string)
                    goto emit_getter_block;
                Node &value = *node.value.node;
                warn("HOLUP! x:42 is a reference? then *node.value.node makes no sense!!!"); // todo FIX!!
                code.add(emitSetter(node, value, context));
            } else {
            emit_getter_block:
                auto local = context.locals[name];
                code.addByte(get_local); // todo skip repeats
                code.addByte(local.position); // base location stored in variable!
                last_type = local.type;
                if (node.length > 0) {
                    if (use_wasm_arrays)
                        return emitWasmArrayGetter(node, context, local);
                    return emitIndexPattern(NUL, node, context, true, local.type); // todo?
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
            if (!node.value.string)
                error("missing node.value.string");
            last_object_pointer = data_index_end;
            String string = *node.value.string;
            if (referenceDataIndices.has(string))
            // todo: reuse same strings even if different pointer, aor make same pointer before
            //                last_object_pointer = referenceIndices[string];
                last_object_pointer = referenceDataIndices[string] - 8;
            else {
                if (use_wasm_strings)
                    return emitStringRef(node, context);
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
                    else
                        trace("reassigning reference "s + name + " to " + node.value.string);
                }
                if (node.parent and (node.parent->kind == reference or node.parent->kind == key)) {
                    // todo move up! todo key bad criterion!!
                    // todo: add header or copy WHOLE string object!
                    referenceIndices.insert_or_assign(node.parent->name, last_object_pointer); // safe ref to string
                    referenceDataIndices.insert_or_assign(node.parent->name,
                                                          last_object_pointer + 8); // safe ref to string
                    referenceMap.insert_or_assign(node.parent->name, node); // lookup types, array length …
                }

                // we add an extra 0, unlike normal wasm abi, because we have space in data section
            }
            last_type = charp;
            code = Code(i32_const) + Code(last_object_pointer + 8); // just a pointer to DATA
            if (node.length > 0) {
                if (node.length > 1)
                    error("only 1 op allowed");
                Node &pattern = node.first();
                if (pattern.kind != patterns and pattern.kind != longs)
                    error("only patterns allowed on string");
                code.add(emitIndexPattern(NUL, pattern, context, false, charp));
            }
            return code;
            //			return Code(stringIndex).addInt(pString->length);// pointer + length
        }
        case key:
            if (node.value.node) {
                Node &value = *node.value.node;
                return emitValue(value, context); // todo: make sure it is called from right context (after isSetter …)
            }
        case patterns:
            // todo: make sure to have something indexable on stack!
            return emitIndexPattern(NUL, node, context, false, last_type);
        case expression: {
            Node values = node.values(); // remove setter part
            return emitExpression(values, context);
        }
        //			error("expression should not be print on stack (yet) (maybe serialize later)")
        case call:
            return emitCall(node, context); // yep should give a value ok
        case groups: // (1 2 3)
        case objects: // { 1 2 3}
            return emitData(node, context); // todo: could be closure, node, … !!
        default:
            error("emitValue unknown type: "s + typeName(node.kind));
    }
    if (node.type) {
        if (node.type->kind != flags)
            code.add(emit_cast(node, *node.type, context));
    }
    // code.push(last_type) only on return statement
    return code;
}


[[nodiscard]]
Code emitAttributeSetter(Node &node, Function &context) {
    Node *value = node.value.node;
    if (!value)value = node.last().value.node;
    if (!value)
        error("attribute setter missing value");
    todo("emitAttributeSetter");
    return Code();
}


[[nodiscard]]
Code emitInvokeExternRef(Node &node, Node &field, Function &function) {
    auto op = Node("invokeExternRef");
    op.add(node);
    op.add(Node(field.name));
    //    op.add(field.childs());// params as Node
    if (field.length == 1)
        op.add(field.first());
    else
        op.add(field.childs()); // todo params type node vs chars .serialize()
    return emitCall(op, function);
}

// external reference ≠ wasm reference types (struct …) !
[[nodiscard]]
Code emitReferenceProperty(Node &node, Node &field, Function &function) {
    if (field.length > 0)return emitInvokeExternRef(node, field, function);
    auto op = Node("getExternRefPropertyValue");
    op.add(node);
    op.add(Node(field.name));
    return emitCall(op, function);
}


[[nodiscard]]
Code emitAttributeCall(Node &node, Node &field, Function &context) {
    if (node.kind == referencex)
        return emitInvokeExternRef(node, field, context);
    Node *value = node.value.node;
    if (!value)value = node.last().value.node;
    if (!value)
        error("attribute setter missing value");
    todo("emitAttributeCall");
    return Code();
}

Code emitGetter(Node &node, Node &field, Function &context) {
    Code code;
    // todo we could match the field to data at compile time in some situations, but let's implement the general case first:
    if (node.kind == referencex)
        return emitReferenceProperty(node, field, context);
    if (node.kind == reference and node.length > 0) // else loop!
        return emitIndexPattern(node, field, context, false, unknown);
    //        code.add(emitValue(node, context));
    else todo("get pointer of node on stack");
    if (field.kind == strings)
        code.add(emitString(field, context));
    else
        code.add(emitString(field.name, context));

    Function &getField = functions["getField"];
    code.addByte(function_call);
    code.addInt(getField.code_index);
    return code;
}

Code emitReferenceAttribute(Node &node, Node &field, Function &function) {
    todo("emitReferenceAttribute");
    return Code();
}


Code emitReferenceTypeConstructor(Node &node, Function &context);

// todo ⚠️ discern get / set / call attribute! x.y x.y=z x.y()
[[nodiscard]]
Code emitAttribute(Node &node, Function &context) {
    if (node.length < 2)
        error("need object and field");
    if (node.length > 3)
        error("extra attribute stuff");
    if (node.value.node or node.last().value.node)
        return emitAttributeSetter(node, context); // danger get->set ?
    Node field = node.last(); // NOT ref, we resolve it to index!
    Node &object = node.first();
    if (field.length) // e.g. $canvas.getContext('2d')
        return emitAttributeCall(object, field, context); // danger get->set ?

    if (object.kind == constructor and use_wasm_structs) {
        Code ref = emitReferenceTypeConstructor(object, context);
        return ref + emitReferenceAttribute(object, field, context);
    }

    Type element_type = unknown;
    if (!object.type and types.has(object.name))
        object.type = types[object.name];

    if (object.type) {
        Node type = *object.type;
        if (!type.has(field.name))
            error("field %s missing in type %s of %s "s % field.name % type.name % node.serialize());
        Node &member = type[field.name];
        int index = member.value.longy;
        if (member.kind == fields) {
            // todo directly
            index = member["position"].value.longy;
            element_type = mapType(member["type"] /*.value*/);
        }
        if (index < 0 or index > type.length)
            error("invalid field index %d of %s in element_type %s of %s "s % index % field.name % type.name %
            node.serialize());
        field = Node(index);
    } else
        emitGetter(object, field, context);

    Code code = emitData(object, context);
    // base pointer on stack
    // todo move => once done
    // a.b and a[b] are equivalent in angle
    return code + emitIndexPattern(object, field, context, true, element_type); // emitIndexRead
}

// wasm reference ≠ external reference !
Code emitReferenceAttribute(Node &object, Node field_name, Function &function) {
    Code code;
    //    return code; // todo

    //    // fb 03 00 00  struct.get $type(0) $field(0) (stack: instance-ref_type)
    //    uint type_index = 0x01;
    //    uint field_index = 0x01;
    //    uint type_index = object.value.longy;
    auto typ = *types[object.name];
    uint type_index = typ.value.longy;
    auto field = typ[field_name.name];
    uint field_index = field.value.longy;
    //    code.add(struct_prefix);
    code.addOpcode(struct_get);
    code.add(type_index);
    code.add(field_index);
    last_type = int32t; // todo get type from field
    return code;
}


[[nodiscard]]
Code emitOperator(Node &node, Function &context) {
    Code code;
    String &name = node.name;
    //	name = normOperator(name);
    if (node.name == "nop" or node.name == "pass")return code;
    if (node.length == 0 and (name == "=" or name == ":=")) return code; // BUG

    if (name == "then")return emitIf(*node.parent, context); // pure if handled before
    auto first = node.first();
    //    if (name == ":=")return emitDeclaration(node, first); todo!
    if (name == ":=")return emitSetter(node, first, context);
    if (name == "=")return emitSetter(node, first, context); // todo node.first dodgy
    if (name == ".") return emitAttribute(node, context);
    //    if (name=="#" and node.length==1)return emitLength(node, context);
    //    if (name=="#" and node.length==2 and use_wasm_arrays  return emitWasmArrayGetter(node, context);
    //	if (name=="#" and node.length==2)return emitIndexPattern(node[0], node[1], context, false, unknown); elsewhere
    if (name == "::=")return emitGetGlobal(node); // globals ASSIGNMENT already handled in analyze / globalSection()
    if (node.length < 1 and not node.value.node and not node.next) {
        if (name == "not")return emitValue(True, context);
        error("missing args for operator "s + name);
    } else if (node.length == 1) {
        Node arg = node.children[0];
        const Code &arg_code = emitExpression(arg, context); // should ALWAYS just be value, right?
        arg_type = last_type;
        code.push(arg_code); // might be empty ok
    } else if (node.length == 2) {
        Node lhs = node.children[0]; //["lhs"];
        Node rhs = node.children[1]; //["rhs"];
        const Code &lhs_code = emitExpression(lhs, context);
        Type lhs_type = last_type;
        arg_type = last_type; // needs to be visible to array index [1,2,3]#1 gets FUCKED up in rhs operations!!
        if (isGeneric(last_type))
            arg_type = last_type.generics.value_type;
        const Code &rhs_code = emitExpression(rhs, context);
        Type rhs_type = last_type;
        Type common_type = commonType(lhs_type, rhs_type); // 3.1 + 3 => 6.1 etc
        bool same_domain = common_type != none; // todo: only some operators * / + - only sometimes autocast!
        code.push(lhs_code); // might be empty ok
        if (same_domain)
            code.add(cast(lhs_type, common_type));
        code.push(rhs_code); // might be empty ok
        if (same_domain)
            code.add(cast(rhs_type, common_type));
        if (common_type != void_block)
            last_type = common_type;
        else last_type = rhs_type;
    } else if (node.length > 2) {
        // todo: n-ary? ∑? is just a context!
        error("Too many args for operator "s + name);
        //	} else if (node.next) { // todo really? handle ungrouped HERE? just hiding bugs?
        //		const Code &arg_code = emitExpression(*node.next, context);
        //		code.push(arg_code);// might be empty ok
    } else if (node.value.node and node.kind == key) {
        // todo: serialize?
        const Code &arg_code = emitExpression(*node.value.node, context);
        code.push(arg_code);
    }
    /*
     * PARAMETERS of operators (but not functions) are now on the STACK!!
     * */
    if (functions.has(name) and not(name == "‖")) {
        print("OPERATOR / FUNCTION CALL: %s\n"s % name);
        code.addByte(function_call);
        int index = functions[name].call_index; // could be import
        if (index < 0)
            error("function %s must have call_index by now"s % name);
        code.add(index);
        return code;
    }
    if (last_type == unknown_type) {
        if (allow_untyped_nodes)
            last_type = Primitive::node; // anything
        else
            internal_error("unknown type should be inferred by now:\n"s + node.serialize());
    }
    unsigned short opcode = opcodes(name, mapTypeToWasm(last_type), mapTypeToWasm(arg_type));

    if (opcode >= 0x8b and opcode <= 0x98)
        code.add(cast(last_type, float32t)); // float ops
    if (opcode >= 0x99 and opcode <= 0xA6)
        code.add(cast(last_type, float64t)); // double ops

    if (last_type == stringp or last_type == charp) {
        code.add(emitStringOp(node, context));
        return code;
    } else if (opcode == f32_sqrt) {
        code.addByte(f32_sqrt);
        last_type = f32t;
    } else if (opcode == f32_eqz) {
        // hack for missing f32_eqz
        if (last_type == float32t)
            code.addByte(i32_reinterpret_f32); // f32->i32  i32_trunc_f32_s would also work, but reinterpret is cheaper
        code.addByte(i32_eqz);
        last_type = i32t; // bool'ish
    } else if (opcode == f64_eqz) {
        // hack for missing f32_eqz
        if (last_type == float64t)
            code.addByte(i64_reinterpret_f64); // f32->i32  i32_trunc_f32_s would also work, but reinterpret is cheaper
        code.addByte(i64_eqz);
        last_type = i32t; // bool'ish
    } else if (name == "*" and isArrayType(last_type)) {
        code.add(emitCall("matrix_multiply", context)); // gpu / vector shim
    } else if (name == "++" or name == "--") {
        Node increased = Node(name[0]).setType(operators);
        increased.add(first); // if not first emitted
        increased.add(new Node(1)); // todo polymorph operator++ instead of +1 !
        code.add(emitSetter(first, increased, context));
    } else if (name == "#") {
        // index operator
        if (node.length == 1)
            return code + emitLength(first, context);
        if (node.parent and node.parent->name == "=") // setter!
            return code + emitIndexWrite(node[0], context); // todo
        else {
            return code + emitIndexRead(node, context, true, true);
        }
    } else if (isFunction(name, true) or isFunction(normOperator(name), true)) {
        emitCall(node, context);
    } else if (opcode > 0xC0 and opcode < 0xFB00) {
        error("internal opcode not handled"s + hex(opcode));
    } else if (opcode > 0) {
        code.addOpcode(opcode);
        if (opcode > 0xFB00)
            code.add(0x01); // todo: memory index argument!?
        if (last_type == none or last_type == voids)
            last_type = i32t;
        if ((opcode >= 0x45 and opcode <= 0x78) or opcode == string_eq)
            last_type = i32; // int ops (also f64.eqz …)
    } else if (name == "²") {
        //		error("this should be handled universally in analyse: x² => x*x no matter what!");
        // BUT non-lazy calling twice? x² => x * result
        Local &result = context.locals.last(); // ["result"];
        code.add(tee_local); // neeeeds result local
        code.add(result.position);
        code.add(get_local);
        code.add(result.position);
        code.add(opcodes("*", last_type));
    } else if (name == "+=") {
        //
        error(" += usually handled in analyze!");
    // } else if (name == "exp" or name == "ℇ") {
    } else if (name == "**" or name == "to the" or name == "^" or name == "^^" or name == "exp" or name == "ℇ") {
        //        code.add(cast(last_type, Primitive::wasm_float64));
        //        code.add(emitCall("pow", context));
        //		if(last_value==0)code.addConst(1);
        //		if(last_value==1)return code;
        //#if MY_WASM
        //        getWaspFunction("pow");
        //        code.add(emitCall("pow", context));
        //#else
        if (last_type == int32t) code.add(emitCall("powi", context));
        else if (last_type == float32t) code.add(emitCall("powf", context));
        else if (last_type == float64t) code.add(emitCall("pow", context));
        else if (last_type == int64s) code.add(emitCall("pow_long", context));
        else todo("^ power with type "s + typeName(last_type));
        // else code.add(emitCall("pow_long", context));
        //#endif
        //         'powi' is a builtin with type 'long double (long double, long double)'
    } else if (name.startsWith("-")) {
        code.add(i32_sub);
    } else if (name == "return") {
        // todo multi-value
        List<Type> &returnTypes = context.signature.return_types;
        Valtype return_type = mapTypeToWasm(returnTypes.last());
        code.add(cast(last_type, return_type));
        code.add(return_block);
    } else if (name == "as") {
        code.add(emitCall("emit_cast", context));
    } else if (name == "%") {
        // int cases handled above
        if (last_type == float32t)
            return code.add(emitCall(Node("modulo_float").setType(call), context)); // mod_f
        else
            return code.add(emitCall(Node("modulo_double").setType(call), context)); // mod_d
    } else if (name == "?") {
        return emitIf(node, context);
    } else if (name == "≈") {
        return emitSimilar(node, context);
    } else if (name == "ⁿ") {
        if (node.length == 1) {
            code.add(cast(last_type, float64t)); // todo all casts should be auto-cast (in emitCall) now, right?
        }
        if (node.length <= 1) {
            // use stack
            if (not context.locals.has("n"))
                error("unknown n");
            code.add(get_local);
            code.addInt(context.locals["n"].position);
            code.add(cast(context.locals["n"].type, float64t)); // todo all casts should be auto-cast now, right?
        }
        code.add(emitCall("pow", context));
        //		else
        //			code.add(emitCall("powi", context));
    } else {
        error("unknown opcode / call / symbol: "s + name);
    }

    if (opcode == get_local and node.length == 1) {
        // arg AFTER op (not as const!)
        int64 last_local = first.value.longy;
        code.push(last_local);
        last_type = context.locals.at(last_local).type;
    }
    if (opcode >= 0x45 and opcode <= 0x78)
        last_type = i32; // int ops (also f64.eqz …)
    return code;
}

Type commonType(Type lhs, Type rhs) {
    // todo: per function / operator!
    if (lhs == rhs)return lhs;
    if (lhs == i64 and rhs == int32t) return i64;
    if (lhs == int32t and rhs == i64) return i64;
    if (lhs == unknown_type) return rhs; // bad guess, could be any object * n !
    //    if (lhs == reference) return rhs;// bad guess, could be any object * n !
    //    if(rhs == reference) return lhs;// bad guess, could be any object * n !
    //    if(rhs == unknown_type) return lhs;// bad guess, could be any object * n !
    if (lhs == float64t or rhs == float64t)return float64t;
    if (lhs == float32t or rhs == float32t)return float32t;
    return lhs; // todo!
    //    return none;
}

Valtype needsUpgrade(Valtype lhs, Valtype rhs, String string) {
    if (lhs == float64t or rhs == float64t)
        return float64t;
    if (lhs == float32t or rhs == float32t)return float32t;
    return none;
}

[[nodiscard]]
Code emitStringOp(Node &op, Function &context) {
    //	Code stringOp;
    //	op = normOperator(op.name);
    if (op == "+") {
        op = Node("_Z6concatPKcS0_"); //demangled on readWasm, but careful, other signatures might overwrite desired one
        functions["_Z6concatPKcS0_"].signature.return_types[0] = charp;
        // can't infer from demangled export name nor wasm type!
        return emitCall(op, context);
        //		stringOp.addByte();
    } else if (op == "==" or op == "is" or op == "equals") {
        op = Node("_Z2eqPKcS0_i"); //  careful : various signatures
        //        op = Node("_Z2eqR6StringPKc");//  careful : various signatures
        last_type = charp; //stringp;
        return Code(i32_const) + Code(-1) + emitCall(op, context); // third param required!
    } else if (op == "#") {
        // todo: all different index / op matches
        if (op.length == 1)
            return emitLength(op.first(), context);
        op = Node("getChar"); //  careful : various signatures
        functions["getChar"].signature.return_types[0] = codepoint1; // can't infer from demangled export name nor wasm
        return emitCall(op, context);
    } else if (op == "not" or op == "¬") {
        // todo: all different index / op matches
        op = Node("empty"); //  careful : various signatures for falsey falsy truthy
        return emitCall(op, context).add(i32_eqz);
    } else if (op == "logs" or op == "prints" or op == "print") {
        // should be handled before, but if not print anyway
        op = Node("puts"); // todo: chars vs shared String& ?
        return emitCall(op, context);
    } else todo("string op not implemented: "s + op.name);
    return Code();
}

//	todo : ALWAYS MAKE RESULT VARIABLE FIRST IN BLOCK (index 0 after args, used for 'it'  after while(){} etc) !!!
int last_local = 0;


bool isVariableName(String name) {
    return name[0] >= 'A'; // todo
}

Code emitConstruct(Node &node, Function &context);

Primitive elementType(Type type32);

Code emitHtml(Node &node, Function &function, ExternRef parent);

// also value expressions of globals!
[[nodiscard]]
Code emitExpression(Node &node, Function &context/*="wasp_main"*/) {
    // expression, node or BODY (list)
    //	if(node_pointer==NIL)return Code();// emit nothing unless NIL is explicit! todo
    Code code;
    String &name = node.name;
    //	int index = functionIndices.position(name);
    //	if (index >= 0 and not context.locals.has(name))
    //		error("locals should be analyzed in parser");
    //		locals[name] = List<String>();
    //	locals[index]= Map<int, String>();
    if (node.kind == unknown and context.locals.has(node.name))
        node.kind = reference; // todo clean fallback
    //    if (node.name=="0") {
    //        code.addConst32(0);
    //        return code;
    //    }
    if (name == "html")
        return emitHtml(node, context, 0);
    if (name == "script" or name == "js" or name == "javascript")
        return emitScript(node, context);
    //	or name == "wasm" or name == "wasm32" or name == "wasm64"
    if (name == "if")
        return emitIf(node, context);
    if (name == "while")
        return emitWhile(node, context);
    if (name == "for")
        return emitFor(node, context);
    if (name == "it") {
        // todo when is it ok to just reuse the last value on stack!?
        //		if(last_type==none or last_type==voids){
        code.addByte(get_local);
        code.addByte(last_local);
        last_type = context.locals.at(last_local).type;
        //		}
        return code;
    }
    //	if (name=="#"){
    //		Code c=emitExpression(node[0], context);
    //		return c+emitIndexPattern(node[0], node[1], context);// todo redundant somewhere!
    //	}
    //	or node.kind == groups ??? NO!

    if ((node.kind == call or node.kind == reference or node.kind == operators) and call_indices.has(name)) {
        if (not isFunction(name, true)); //				todo("how?");
        else
            return emitCall(node, context);
    }

    Node &first = node.first();
    switch (node.kind) {
        case functor:
        case records:
        case clazz:
        case flags:
        case structs:
            //        case wasmtype_struct:
            // nothing to do since meta info is in module
            // node is in `types` / `functions` and all fields are in `globals`
            // TYPE info at compile time in types[] // todo: emit reflection data / wit !
            // for INSTANCES see emitConstruct(node, context);
            return code;
        case objects:
        case groups:
            // todo: all cases of true list vs list of expression
            if (node.length > 0 and isProperList(node)) {
                return emitData(node, context); // pointer in const format!
                //				return emitArray(node, context);
            } // else fallthough:
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
            else goto reference; // : fallthrough
        case referencex: {
            if (name.startsWith("$")) {
                name = name.substring(1);
            }
            if (name[0] >= '0' && name[0] <= '9') goto reference; // wasm style $0 = first arg
            //	In HTML and JavaScript ids can start with a letter an underscore (_), or a colon (:) so no conflict with $0
            Node calle("getElementById"); // $
            calle.add(Node(name));
            return emitCall(calle, context);
            break;
        }
        reference:
        case global:
        case reference: {
            if (name.empty()) {
                warn("empty reference!");
                return Code();
            }
            //			Map<int, String>
            int local_index = context.locals.position(name); // defined in block header
            if (name.startsWith("$")) {
                // wasm style $0 = first arg
                local_index = parseLong(name.substring(1));
            }
            if (local_index < 0) {
                // collected before, so can't be setter here
                if (functionCodes.has(name) or functions.has(name))
                    return emitCall(node, context);
                else if (globals.has(name)) {
                    if (context.name == "global")
                        return emitValue(node, context);
                    if (node.isSetter())
                        return emitSetter(node, node, context);
                    return emitGetGlobal(node);
                } else if (name == "exp"){
                    todo("rewrite as ℯ^x");
                    // node.name = "pow";
                    // node.add(Node(2.7182818284590));
                    // return emitCall(node, context);
                } else if (name == "ℯ" or name == "e" or name == "ℇ") // if not provided as global
                    return emitValue(*new Node(2.7182818284590), context);
                else if (name == "τ" or name == "tau") // if not provided as global
                    return emitValue(*new Node(2 * pi), context);
                else if (name == "π" or name == "pi") // if not provided as global
                    return emitValue(*new Node(pi), context);
                else if (!node.isSetter()) {
                    //                    print(context.locals)
                    if (not node.type)
                        error("UNKNOWN local symbol ‘"s + name + "’ in context " + context);
                } else {
                    error("local symbol ‘"s + name.trim() + "’ in " + context + " should be registered in analyze()!");
                }
            }
            auto local = context.locals.at(local_index);
            if (node.isSetter()) {
                //SET
                if (node.kind == key)
                    code = code + emitExpression(*node.value.node, context); // done above!
                else
                    code = code + emitValue(node, context); // done above!
                if (local.type == unknown_type)
                    local.type = last_type;
                else if (local.type == array or local.type == wasmtype_array) todow("array types") else
                    code.add(cast(last_type, elementType(local.type)));
                //				todo: convert if wrong type
                code.addByte(tee_local); // set and get/keep
                code.addByte(local_index);
                // todo last_type set by emitExpression (?)
                //				last_type = declaredFunctions[context.locals.at([local_index).valtype;
            } else {
                // GET
                code.addByte(get_local); // todo: skip repeats
                code.addByte(local_index);
                last_type = local.type;
            }
        }
        break;
        case patterns: // x=[];x[1]=2;x[1]==>2 but not [1,2] lists
        {
            if (not node.parent) // todo: when is op not an operator? wrong: or node.parent->kind == groups)
                return emitArray(node, context);
            else if (node.parent->kind == declaration)
                return emitIndexWrite(*node.parent, context);
            else {
                Node array1 = NUL;
                if (node.parent)array1 = node.parent->first();
                if (array1.kind == reference) // and …
                    return emitArray(node, context);
                return emitIndexPattern(array1, node, context, false, last_value_type);
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
        case urls:
            return emitString(node, context);
        case undefined:
        case unknown: // todo: proper NIL!
            warn("UNKNOWN EMIT CASE: "s + node.serialize());
            return code;

        case constructor:
            return emitConstruct(node, context);
        default:
            error("unhandled node type «"s + node.name + "» : " + typeName(node.kind));
    }
    return code;
}

//extern "C" ExternRef createElement(ExternRef parent /*0*/, chars tag);
//extern "C" ExternRef createElement2(ExternRef parent /*0*/,chars tag,chars id,chars className,chars innerHTML);
[[nodiscard]]
Code emitHtmlWasp(Node &node, Function &function, ExternRef parent = 0) {
    Code code;
    if (node.name == "html") {
        code.add(emitCall("getDocumentBody", function)); // document.body as parent
    } else {
        //		if(parent)code.add(emitData(*new Node(parent), function)); todo ?
        code.add(emitString(node, function));
        code.add(emitCall("createElement", function));
        //	addVariable(node.name, parent); // can't, must be in analyze!
    }
    for (Node &child: node) {
        //		getVariable( parent); // can't, must be in analyze or on top of stack!
        code.add(emitHtml(child, function, 0));
        code.add(drop); // we don't need this as parent, revert to original parent
    }
    return code;
}

Primitive elementType(Type type32) {
    if (type32 == stringp)return byte_char;
    if (type32 == longs)return Primitive::wasm_int64; // todo should not have reached here
    if (type32 == nils)return nulls;
    if (type32 == byte_i8)return Primitive::byte_i8;
    if (isGeneric(type32))return (Primitive) type32.generics.value_type;
    if (type32 == wasmtype_array)
        error("array should have a type!");;
    error("elementType not implemented for "s + typeName(type32));
    return Primitive::wasm_int32;
}

void discard(Code &code);

void discard(Code code);

// wasm reference type constructor
Code emitReferenceTypeConstructor(Node &node, Function &context) {
    Code code;
    int pointer = data_index_end;
    for (Node &field: node) {
        code += emitValue(field, context); // on stack
    }
    uint type_index = types[node.name]->value.longy;
    //    code.add(struct_prefix);
    code.addOpcode(struct_new);
    code.add(type_index);
    last_type = Valtype::wasm_struct; // HOLUP, we need to know the type of the struct! e.g. 6b00 => wasm_struct $0
    last_object = &node;
    last_object_pointer = pointer;
    return code;
}

Code emitConstruct(Node &node, Function &context) {
    if (use_wasm_structs)
        return emitReferenceTypeConstructor(node, context);
    Code code;
    int pointer = data_index_end;
    for (Node &field: node) {
        discard(emitData(field, context)); // just write the values to memory and lastly return start-pointer
    }
    last_object = &node;
    last_object_pointer = pointer;
    //    if(node.type)
    //    last_type = mapTypeToPrimitive(*node.type);
    last_type = pointer;
    code.addConst32(pointer); // base for future index getter/setter [0] #1
    return code;
}

[[nodiscard]]
Code emitForIterator(Node &node, Function &context) {
    if (node.length < 2)
        error("Invalid 'for' loop structure. Expected variable and list or iterable.");

    Code code;

    // Extract components
    Node &variable = node[0]; // Loop variable (e.g., 'i')
    Node &iterable = node[1]; // List or iterable (e.g., 'list')
    Node body = (node.length > 2) ? node[2] : Node(); // Loop body

    // Validate components
    if (!variable.value.data)
        error("Missing loop variable in 'for' statement.");
    if (!iterable.value.data)
        error("Missing list or iterable in 'for' statement.");

    // Initialize iterator
    Node iterator = Node("iterator");
    iterator.add(iterable);
    Code iteratorInitCode = emitExpression(iterator, context);
    code = code + iteratorInitCode;

    // Begin loop block
    code.addByte(loop);
    code.addByte(none); // No return type for the loop itself

    // Emit condition: Check if the iterator has more elements
    Node hasNext = Node("hasNext");
    hasNext.add(iterator);
    Code conditionCode = emitExpression(hasNext, context);
    code = code + conditionCode;
    code.addByte(if_i);
    code.addByte(none); // Void block for condition

    // Emit loop body
    // Assign the current element of the iterator to the variable
    Node getNext = Node("getNext");
    getNext.add(iterator);
    Node assign = Node("=");
    assign.add(variable);
    assign.add(getNext);
    Code assignCode = emitExpression(assign, context);
    code = code + assignCode;
    Code bodyCode = emitExpression(body, context);
    code = code + bodyCode;
    code.addByte(br_branch); //  back to the start of the loop
    code.addByte(0); // Branch to the start of this loop block
    code.addByte(end_block);
    code.addByte(end_block);
    return code;
}

[[nodiscard]]
Code emitForArray(Node &node, Function &context) {
    // builtin list in wasm
    if (node.length < 2)
        error("Invalid 'for' loop structure. Expected variable and list or iterable.");

    // Extract components
    Code code;
    Node &variable = node[0]; // Loop variable (e.g., 'i')
    Node &list = node[1]; // Built-in list (e.g., 'list')
    Node body = (node.length > 2) ? node[2] : Node(); // Loop body

    // Validate components
    if (!variable.value.data)
        error("Missing loop variable in 'for' statement.");
    if (!list.value.data)
        error("Missing list in 'for' statement.");

    //    if(!context.locals.has(variable.name))
    context.allocateLocal(variable.name); // error if there?

    // Emit list length retrieval (memory read from *pointer - 4)
    code.addByte(get_local); // Load the list's pointer
    int pointer = (int) (long) (list.value.data);
    code.addInt(pointer, false);
    code.addByte(i32_load); // WebAssembly load instruction
    code.addByte(4); // Alignment (4 bytes for 32-bit integer)
    code.addByte(-4); // Offset -4 to retrieve length

    // Save length to a local variable
    int lengthLocalIndex = context.allocateLocal("array_length"); // Allocate a local variable for list length
    code.addByte(set_local);
    code.addByte(lengthLocalIndex);

    // Initialize index to 0
    int indexLocal = context.allocateLocal("array_index"); // Allocate a local variable for the index
    code.addByte(i32_const);
    code.addByte(0);
    code.addByte(set_local);
    code.addByte(indexLocal);

    // Begin loop block
    code.addByte(loop);
    code.addByte(none); // No return type for the loop itself

    // Emit condition: Check if index < list length
    code.addByte(get_local);
    code.addByte(indexLocal);
    code.addByte(get_local);
    code.addByte(lengthLocalIndex);
    code.addByte(i32_lt); // Compare index < length
    code.addByte(if_i);
    code.addByte(none); // Void block for condition

    // Access list[index]
    code.addByte(get_local);
    code.addByte(pointer); // Load list pointer
    code.addByte(get_local);
    code.addByte(indexLocal); // Load index
    code.addByte(i32_add); // Compute list + index
    code.addByte(i32_load); // Load list[index]

    // Assign to the loop variable
    int variableLocal = context.locals[variable.name].position; // Allocate or retrieve a local for the variable
    code.addByte(set_local);
    code.addByte(variableLocal);

    // Emit loop body
    Code bodyCode = emitExpression(body, context);
    code = code + bodyCode;

    // Increment index
    code.addByte(get_local);
    code.addByte(indexLocal);
    code.addByte(i32_const);
    code.addByte(1); // Increment by 1
    code.addByte(i32_add);
    code.addByte(set_local);
    code.addByte(indexLocal);

    // Branch back to the start of the loop
    code.addByte(br_branch);
    code.addByte(0); // Branch to the start of this loop block

    code.addByte(end_block);
    code.addByte(end_block);
    return code;
}

[[nodiscard]]
Code emitFor(Node &node, Function &context) {
    // for i in 1 to 4 {}    i <= 4
    // for i in 1 upto 4 {}  i < 4
    if (node.length < 4)
        return emitForArray(node, context);
    //  todo: general case: for i in iterable {}
    //        error("Invalid 'for' loop structure. Expected variable, from to , and body.");

    Code code;

    // Extract components
    //    Node &variable = *node[0].value.node;       // Loop variable (e.g., 'i')
    //    Node &begin = node[1];          // Range (e.g., '1..10')
    //    Node &end = node[2];          // Range (e.g., '1..10')
    //    Node &body = node[3];           // Loop body

    Node &variable = node["variable"];
    Node &begin = node["begin"];
    Node &end = node["end"];
    Node &body = node["body"];
    Node &upto = node["upto"];

    let variables = context.locals;
    //        return emitForIterator(node, context);

    // Decompose the range into start, end, and step
    //    Node begin = range.children[0]; // Start of the range (e.g., '1')
    //    Node end = range.children[1];   // End of the range (e.g., '10')
    //    Node step = range.size() > 2 ? range.children[2] : Node("1"); // Default step = 1
    Node step = Node(1); // Default step = 1

    // Emit initialization (e.g., 'i = start')
    Node assignment = Node("=");
    assignment.setType(operators);
    assignment.add(variable);
    assignment.add(begin);
    code = code + emitExpression(assignment, context);

    // Begin loop block
    code.addByte(loop);
    code.addByte(none); // No return type for the loop itself

    // Emit condition (e.g., 'i < end')
    Node condition("<=");
    if (upto.value.longy) // 1 upto 4 = 1 2 3
        condition = Node("<");
    condition.setType(operators);
    condition.add(variable);
    condition.add(end);
    code = code + emitExpression(condition, context);
    code.addByte(if_i);
    code.addByte(none); // Void block for condition

    // Set the result variable to the current (=>last) loop variable
    code.addByte(get_local);
    code.addByte(variables[variable.name].position); // Set the result variable to the current (=>last) loop variable
    code.add(cast(i32, i64));
    code.addByte(set_local); // Set the result variable to the current (=>last) loop variable
    code.addByte(variables["result"].position);

    // Emit loop body
    Code bodyCode = emitExpression(body, context);
    code = code + bodyCode;

    // Emit increment (e.g., 'i += step')
    // Create the addition expression: i + step
    Node addition = Node("+");
    addition.setType(operators);
    addition.add(variable); // Left operand: i
    addition.add(step); // Right operand: step (e.g., 1)

    // Create the assignment: i = (i + step)
    Node reassignment = Node("=");
    reassignment.setType(operators);
    reassignment.add(variable); // Left-hand side: i
    reassignment.add(addition); // Right-hand side: result of (i + step)

    // Emit the assignment expression
    Code incrementCode = emitExpression(reassignment, context);
    code = code + incrementCode;

    // Branch back to the start of the loop
    code.addByte(br_branch);
    code.addByte(1); // Branch to the start of this loop block

    // End condition block
    code.addByte(end_block);

    // End loop block
    code.addByte(end_block);

    code.addByte(get_local);
    code.addByte(variables["result"].position); // Set the result variable to the current (=>last) loop variable
    last_type = i64;

    return code;
}

[[nodiscard]] // for i=0;i<10;i++ {}
Code emitForClassic(Node &node, Function &context) {
    Code code;

    Node initializer = node[0]; // i = 0
    Node condition = node[1]; // i < 1000
    Node increment = node[2]; // i++
    Node body = node[3]; // loop body

    Valtype loop_type = none; // Assume no return type for now

    // Emit initializer (i = 0)
    code = code + emitExpression(initializer, context);

    // Start the loop block
    code.addByte(loop);
    code.addByte(loop_type);

    // Emit condition (i < 1000)
    code = code + emitExpression(condition, context);
    code.addByte(if_i);
    code.addByte(none); // Type: void_block

    // Emit loop body
    code = code + emitExpression(body, context);

    // Emit increment (i++)
    code = code + emitExpression(increment, context);

    // Branch back to the start of the loop
    code.addByte(br_branch);
    code.addByte(0); // Loop to the start of the current loop

    // End the condition block
    code.addByte(end_block);

    // End the loop block
    code.addByte(end_block);

    last_type = loop_type;
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

    code = code + emitExpression(condition, context); // condition
    code.addByte(if_i);
    code.addByte(none); // type:void_block
    //	code.addByte(int32);
    code = code + emitExpression(then, context); // BODY

    code.addByte(br_branch);
    code.addByte(1);
    code.addByte(end_block); // end if block
    if (loop_type == none and last_type == i32)
        code.addByte(drop); //hack for nop todo
    // else fall through
    code.addByte(end_block); // end while loop
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
    code.addByte(none); // void_block
    // comprinte the while expression
    code.add(emitExpression(node[0], context)); // node.value.node or
    //	code.add(emitExpression(node["condition"], context));// node.value.node or
    code.addByte(i32_eqz);
    // br_if $label0
    code.addByte(br_if);
    code.addByte(1);
    //			code.push(signedLEB128(1));
    // the nested logic
    code.add(emitExpression(node[1], context)); // BODY
    //	code.add(emitExpression(node["then"], context));// BODY
    // br $label1
    //	code.addByte(br);
    //	code.addByte(0);
    //				code.push(signedLEB128(0));
    code.addByte(end_block); // end loop
    code.addByte(get_local);
    int block_value = 0; // todo : ALWAYS MAKE RESULT VARIABLE FIRST IN FUNCTION!!!
    code.addByte(block_value); // todo: skip if last value is result
    //	code.addByte(end_block); // end block
    return code;
}

[[nodiscard]]
Code emitExpression(Node *nodes, Function &context) {
    if (!nodes)return Code();
    //	if(node_pointer==NIL)return Code();// emit nothing unless NIL is explicit! todo
    return emitExpression(*nodes, context);
}

Code emitCall(String fun, Function &context) {
    return emitCall(*new Node(fun), context);
}

[[nodiscard]]
Code emitCall(Node &fun, Function &context) {
    Code code;
    auto name = fun.name;
    if (not functions.has(name)) {
        auto normed = normOperator(name);
        if (not functions.has(normed))
            error("unknown function "s + name + " (" + normed + ")");
        else name = normed;
    }
    print("emitCall");
    print(fun);

    Function &function = functions[name]; // NEW context! but don't write context ref!
    Signature &signature = function.signature;

    int index = function.call_index;
    if (call_indices.has(name)) {
        if (index >= 0 and index != call_indices[name]) todo("index!=functionIndices[name]");
        index = call_indices[name];
    } else {
        breakpoint_helper
        warn("relying on context.index OK?");
        //		functionIndices[name] = context.index;
    }
    if (index < 0)
        error(
        "Calling %s NO INDEX. TypeSection created before code Section. Indices must be known by now! Mark imports as used!"s
        %
        name);
    int i = 0;
    auto sig_size = signature.parameters.size();
    if (fun.size() > sig_size) {
        print(function);
        print(signature);
        error("too many arguments for function %s %d >= %d "s % name % fun.size() % sig_size);
    }
    for (Node &arg: fun) {
        code.push(emitExpression(arg, context));
        //		Valtype argType = mapTypeToWasm(arg); // todo ((+ 1 2)) needs deep analysis, or:
        Type argType = last_type; // TODO! evaluated expression smarter than node arg!
        if (i >= sig_size) {
            warn("args may have already been emitted");
            break; // todo cast earlier / multi-value cast of stack!!
        }
        Type sigType = signature.parameters[i++].type;
        if (sigType != argType)
            code.push(cast(argType, sigType));
    };
    code.addByte(function_call);
    code.addInt(index); // as LEB!
    code.addByte(nop_); // padding for potential relocation
    code.addByte(nop_);
    code.addByte(nop_);
    code.addByte(nop_);
    context.is_used = true;

    // todo multi-value
    Type return_type = signature.return_types.last(none);
    last_type = return_type;
    // if (signature.wasm_return_type)
    // 	check_is(mapTypeToWasm(last_type), signature.wasm_return_type);
    //	last_type.clazz = &signature.return_type;// todo dodgy!
    return code;
}

[[nodiscard]]
Code cast(Valtype from, Valtype to) {
    Code nop;
    // if two arguments are the same, commontype is 'none' and we return empty code (not even a nop, technically)
    if (from == to)return nop; // nop
    if (from == void_block)return nop; // todo: pray
    if ((Type) from == unknown_type)return nop; // todo: don't pray
    if ((Type) to == unknown_type)return nop; // todo: don't pray
    if (to == none or (Type) to == unknown_type or to == voids)return nop; // no cast needed magic VERSUS wasm drop!!!
    last_type = to; // danger: hides last_type in caller!
    if (from == 0 and to == i32t)return nop; // nil or false ok as int? otherwise add const 0!
    if (from == i32t and (Type) to == reference)return nop; // should be reference pointer, RIHGT??
    if (from == anyref and (Type) to == i64)return nop; // todo call $$ ref.value()
    if (from == i32 and to == externref)return nop; // ref ≈ void* ≈ i64 so ok?
    if (from == i64 and to == externref)return nop; // ref ≈ void* ≈ i64 so ok?
    if (from == externref and to == i32)return nop; // ref ≈ void* ≈ i32 so ok???
    //      expected type externref, found call of type i64   NOT ok ;)

    if ((Type) from == codepoint1 and to == i64)
        return Code(i64_extend_i32_s);
    if (from == float32t and to == float64t)return Code(f64_from_f32);
    if (from == float32t and to == i32t) return Code(f32_cast_to_i32_s);
    if (from == i32t and to == float32t)return Code(f32_from_int32);
    //	if (from == i32t and to == float64)return Code(i32_cast_to_f64_s);
    if (from == i64 and to == i32) return Code(i32_wrap_i64);
    if (from == float32t and to == i32) return Code(i32_trunc_f32_s);
    //	if(from==f32u and to==i32)	return Code(i32_trunc_f32_𝗎);
    if (from == float64t and to == i32) return Code(i32_trunc_f64_s);
    //	if(from==f64u and to==i32)	return Code(i32_trunc_𝖿𝟨𝟦_𝗎);
    if (from == i32 and to == i64) return Code(i64_extend_i32_s);
    //	if(from==i32u and to==i64)	return Code(i64_extend_i32_𝗎);
    if (from == float32t and to == i64) return Code(i64_trunc_f32_s);
    //	if(from==f32u and to==i64)	return Code(i64_trunc_f32_𝗎);
    if (from == float64t and to == i64) return Code(i64_trunc_f64_s);
    //	if(from==f64u and to==i64)	return Code(i64_trunc_𝖿𝟨𝟦_𝗎);
    if (from == i32 and to == float32t) return Code(f32_convert_i32_s);
    //	if(from==i32u and to==f32)	return Code(f32_convert_i32_𝗎);
    if (from == i64 and to == float32t)
        return Code(f32_convert_i64_s);
    //	if(from==f64u and to==f32)	return Code(f32_convert_i64_𝗎);
    if (from == float64t and to == float32t) return Code(f32_demote_f64);
    //	if (from == i32 and to == wasm_string_ref)
    //		return nop;// todo: string_ref is a pointer, so cast to i32 is ok?
    if (from == i32 and to == float64t)
        return Code(f64_convert_i32_s);
    //	if(from==i32u and to==f64)	return Code(f64_convert_i32_𝗎);
    if (from == i64 and to == float64t)
        return Code(f64_convert_i64_s);
    //	if(from==f64u and to==f64)	return Code(f64_convert_i64_𝗎);
    if (from == float32t and to == float64t) return Code(f64_promote_f32);
    //	if(from==f32 and to==i32)	return Code(i32_reinterpret_f32);
    //	if(from==f64 and to==i64)	return Code(i64_reinterpret_𝖿𝟨𝟦);
    //	if(from==i32 and to==f32)	return Code(f32_reinterpret_i32);
    //	if(from==i64 and to==f64)	return Code(f64_reinterpret_i64);
    if (from == i64 and to == float32t) return Code(f64_convert_i64_s).addByte(f32_from_f64);

    //    if (from == string_ref and to == i64)return stringRefLength();
    //    if (from == string_ref and to == i64)return castRefToChars();
    //	if (from == void_block and to == i32)
    //		return Code().addConst(-666);// dummy return value todo: only if main(), else WARN/ERROR!
    error("incompatible valtypes "s + (int) from + "->" + (int) to + " / " + typeName(from) + " => " + typeName(to));
    return nop;
}


[[nodiscard]]
Code cast(Type from, Type to) {
    Code nop;
    // if two arguments are the same, commontype is 'none' and we return empty code (not even a nop, technically)
    if (to == none or to == unknown_type or to == voids)return nop; // no cast needed magic VERSUS wasm drop!!!
    if (from == wasmtype_array and isArrayType(to))return nop; // uh, careful? [1,2,3]#2 ≠ 0x0100000…#2
    if (from == to)return nop; // nop
    last_type = to; // danger: hides last_type in caller!
    if (from == node and to == i64t)
        return Code(i64_extend_i32_s).addConst64(node_header_64) + Code(i64_or); // turn it into node_pointer_64 !
    if (from == array and to == charp)return nop; // uh, careful? [1,2,3]#2 ≠ 0x0100000…#2
    if (from == i32t and to == charp)return nop; // assume i32 is a pointer here. todo?
    if (from == charp and to == i64t) return Code(i64_extend_i32_s);
    if (from == charp and to == i32t)return nop; // assume i32 is a pointer here. todo?
    if (from == array and to == i32)return nop; // pray / assume i32 is a pointer here. todo!
    if (from == charp and to == strings)return nop;
    if (isGeneric(from) and isGeneric(to))return nop;
    if (from == codepoint1 and to == i64t)
        return Code(i64_extend_i32_s);
    if (from == array and to == i64)return Code(i64_extend_i32_u);; // pray / assume i32 is a pointer here. todo!
    if (from == i32t and to == array)return nop; // pray / assume i32 is a pointer here. todo!
    if (from == float32t and to == array)return nop; // pray / assume f32 is a pointer here. LOL NO todo!
    if (from == i64 and to == array)return Code(i32_wrap_i64);; // pray / assume i32 is a pointer here. todo!
    //    if(Valtype)
    return cast(mapTypeToWasm(from), mapTypeToWasm(to));
}

// casting in our case also means construction! (x, y) as point == point(x,y)
[[nodiscard]]
Code emit_cast(Node &from, Node &to, Function &context) {
    // todo: only cast directly if from is compatible, don't cast string(pointer) to int in "'123' as int"
    //	if( wasm_compatible(from, to))return cast(mapTypeToWasm(from), mapTypeToWasm(to));
    if (to == IntegerType)return cast(mapTypeToWasm(from), i32);
    if (to == LongType)return cast(mapTypeToWasm(from), i64);
    if (to == DoubleType)return cast(mapTypeToWasm(from), float64t);
    // todo: call cast(Node &from, Node &to) in library!
    Node calle("cast");
    calle.add(from);
    calle.add(to);
    return emitCall(calle, context); // todo context?
}

[[nodiscard]]
Code emitDeclaration(Node &fun, Node &body) {
    // todo: x := 7 vs x := y*y
    if (not call_indices.has(fun.name))
        error("Declaration %s need to be registered in the parser so they can be called!"s % fun.name);
    info("emitting function "s % fun.name);
    Function &declared = functions[fun.name];
    //	Signature &signature = context.signature;
    declared.is_declared = true; // all are 'export' for now. also set in analyze!
    functionCodes[fun.name] = emitBlock(body, declared);
    last_type = none; // todo reference to new symbol x = (y:=z)
    return Code(); // empty, no inner functions, not part of main
}


Code emitGlobalSetter(Node &node, Node &value, Function &context) {
    Code code;
    String &variable = node.name;
    if (!globals.has(variable))
        error("unknown global variable %s"_s % variable);
    Global &global = globals[variable];
    code = emitValue(value, context);
    code.add(cast(last_type, global.type));
    code.addByte(set_global);
    code.addByte(global.index);
    code.addByte(get_global); // we need to tee / return the value, so we can use it in expressions
    code.addByte(global.index);
    return code;
}


[[nodiscard]]
Code emitSetter(Node &node, Node &value, Function &context) {
    if (node.first().name == "#") {
        // x#y=z
        return emitPatternSetter(node.first().first(), node.first().last(), node.last(), context);
    }
    if (node.name == "=" or node.name == ":=") {
        if (node.length != 2)
            error("assignment needs 2 arguments");
        return emitSetter(node[0], node[1], context);
    }
    String &variable = node.name;
    if (globals.has(variable))
        return emitGlobalSetter(node, value, context);
    if (!context.locals.has(variable)) {
        //        error("variable %s in context %s missed by parser! "_s % variable % context.name);
        warn("variable %s in context %s emitted as node data:\n"_s % variable % context.name + node.serialize());
        Code code = emitNode(node, context);
        //        addLocal(context, variable, Primitive::node, false);
        //        code.add(tee_local);
        //        code.add(context.locals[variable].position);
        return code; // wasm_node_index 'pointer' (const.int)
    }

    Local &local = context.locals[variable];
    auto variable_type = local.type;
    //    Valtype value_type = mapTypeToWasm(value); // todo?
    if (variable_type == unknown_type or variable_type == voids) {
        variable_type = last_type; // todo : could have been done in analysis!
        local.type = last_type; // NO! the type doesn't change: example: float x).valtype7
    }
    if (last_type == array or variable_type == array or variable_type == charp) {
        referenceIndices.insert_or_assign(variable, data_index_end); // WILL be last_data !
        referenceDataIndices[variable] = data_index_end + headerOffset(value);
        referenceMap[variable] = value; // node; // lookup types, array length …
    }
    Code setter;
    //	auto values = value.values();
    if (value.hash() == node.hash())
        value = node.values();
    //    value.parent = &node;
    if (value.kind == arrays or value.kind == objects or value.kind == groups or value.kind == patterns)
        value.name = node.name; // HACK to store referenceIndices
    value.parent = &node; // might have been lost through operator shuffle. todo: fix in analyze
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
        last_type = variable_type; // still the type of the local, not of the value. example: float x=7
    context.track(node, setter, 0);
    return setter;
}

//bool isPrimitive(Node &node);

Code zeroConst(Type valtype);

//[[nodiscard]];
Code emitIf(Node &node, Function &context) {
    Code code;
    //	 gets rid of operator, we MAY want .flat() ?
    Node condition = node[0].values();
    if (condition.isEmpty() or condition.isNil())
        condition = Node(0);
    //	Node &condition = node["condition"];
    code = code + emitExpression(condition, context);
    code.add(cast(last_type, int32t));
    code.addByte(if_i);
    Node then = node[1].values();
    //	Node &then = node["then"];
    auto then_block = emitExpression(then, context);
    auto returnType = last_type;
    code.addByte(mapTypeToWasm(returnType)); // of then!
    code = code + then_block; // BODY
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
    context.track(node, code, 0);
    return code;
}

Code zeroConst(Type returnType) {
    Code code;
    if (returnType == int32t)
        code.addConst32(0);
    if (returnType == float32t) {
        code.add(f32_const);
        code.push((bytes) malloc(4), 4);
    }
    if (returnType == float64t) {
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
Code Call(char *symbol) {
    //},Node* args=0) {
    Code code;
    code.addByte(function_call);
    int i = call_indices.position(symbol);
    if (i < 0)
        error("UNKNOWN symbol "s + symbol);
    //	code.opcode(unsignedLEB128(i),8);
    code.addByte(i);
    return Code();
}

[[nodiscard]]
Code encodeString(chars str) {
    int len = (int) strlen(str);
    if (len >= 80) todo("leb len ");
    Code code;
    code.addInt(len);
    code.push((bytes) str, len); // abusing section as len field
    return code; //.push(0);
};

int last_code_byte = 0;

//Code cast FROM SmartPointer more difficult
Code castToSmartPointer(Type from, Type return_type, Function &context, bool &needs_cast) {
    Code block;
    if (from == return_type /*== smarti64*/) return block;
    if (from.type == int_array or from == array) {
        block.add(cast(from, i64));
        block.addConst64(array_header_64).addByte(i64_or); // todo: other arrays
        //			if (return_type==float64)
        //			block.addByte(f64_reinterpret_i64);// hack smart pointers as main return: f64 has int range which is never hit
        last_type = i64;
    }
    //		block.addConst32(array_header_32).addByte(i32_or); // todo: other arrays

    //		if(from==charp)block.push(0xC0000000, false,true).addByte(i32_or);// string
    //		if(from==charp)block.addConst(-1073741824).addByte(i32_or);// string
    else if (from.kind == strings or from.type == c_string or from == charp or
             from == string_ref) {
        //
        if (from == string_ref)block += castRefToChars();
        block.addByte(i64_extend_i32_u);
        block.addConst64(string_header_64);
        block.addByte(i64_or);
        last_type = i64;
        needs_cast = return_type == i64;
    } else if (from.kind == reference) {
        //			if (from==charp)
        //				block.addConst(string_header_64).addByte(i64_or);
        todo("from ref");
    } else if (from == float64t and context.name == start) {
        // hack smart pointers as main return: f64 has range which is never hit by int
        block.addByte(
            i64_reinterpret_f64); // todo: not for _start in wasmtime... or 'unwrap' / print smart pointers via builtin
        //            block.add(emitCall("print_smarty", context)); // _Z5printd putf putd
        //            block.add(emitCall("_Z5printd", context));
        last_type = i64;
    } else if (from == byte_char or from == codepoint1) {
        block.addByte(i64_extend_i32_u);
        block.addConst64(codepoint_header_64);
        block.addByte(i64_or);
        last_type = i64;
        needs_cast = return_type == i64;
    }
    //		if(from==charp)block.addConst32((unsigned int)0xC0000000).addByte(i32_or);// string
    //		if(from==angle)block.addByte(i32_or).addInt(0xA000000);//
    //		if(from==pointer)block.addByte(i32_or).addInt(0xF000000);//
    return block;
}


[[nodiscard]]
Code emitBlock(Node &node, Function &context) {
    //	todo : ALWAYS MAKE RESULT VARIABLE FIRST IN FUNCTION!!!
    //	char code_data[] = {0/*locals_count*/,i32_const,42,call,0 /*logi*/,i32_auto,21,return_block,end_block};
    // 0x00 == unreachable as block header !?
    //	char code_data[] = {0/*locals_count*/,i32_auto,21,return_block,end_block};// 0x00 == unreachable as block header !?
    //	Code(code_data,sizeof(code_data)); // todo : memcopy, else stack value is LOST
    Code block;
    //	block.position
    context.track(node, block, last_code_byte); // start sourceMap empty
    //	Map<int, String>
    //	collect_locals(node, context);// DONE IN analyze
    //	int locals_count = current_local_names.size();
    //	context.locals = current_local_names;
    //	todo : ALWAYS MAKE RESULT VARIABLE FIRST IN BLOCK (index 0, used after while(){} etc) !!!
    // todo: locals must ALWAYS be collected in analyze step, emitExpression is too late!
    last_local = 0;
    last_type = none; //int32;

    int locals_count = context.locals.size(); // incuding params
    context.locals.add("result", {
                           .is_param = false, .position = locals_count++, .name = "result", .type = Valtype::i64,
                       });
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
        if (local.is_param)continue; // part of context type!
        Type type = local.type;
        //		block.addByte(i + 1);// index
        block.addByte(1); // count! todo: group by type nah
        if (type == unknown_type)
            type = int32t;
        // todo		internal_error("unknown type should be inferred by now for local "s + name);
        if (type == none or type == voids)
            type = int32t;
        if (type == charp or type == array)
            type = int32t; // int64? extend to smart pointer later!
        if (use_wasm_arrays and isGeneric(local.type)) {
            auto generics = local.type.generics;
            if (isGroup((Kind) generics.kind)) {
                auto type_index = arrayTypeIndex(valueType(local.type));
                block.addByte(wasm_struct);
                block.addByte(type_index);
            } else
                error("unknown generic type "s + typeName(local.type));
        } else {
            block.addByte(mapTypeToWasm(type));
        }
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
    auto abi = wasp_smart_pointers; // context.abi;


    if (return_type == Valtype::voids and last_type != Valtype::voids)
        block.addByte(drop);
    else if (return_type == Valtype::i32t and last_type == Valtype::voids)
        block.addConst32(0);
    else if (return_type == Valtype::i64t and last_type == Valtype::voids)
        block.addConst64(0); // 		needs_cast = false;
    else if (return_type == Valtype::i64t and last_type == Valtype::externref)
        block.addConst64(0); // todo: can't use smart pointers for elusive externref
    else if (abi == wasp_smart_pointers) {
        block.add(castToSmartPointer(last_type, return_type, context, needs_cast));
    }
    // todo: merge cast with castSmartPointer
    if (needs_cast and last_type.value) {
        block.add(cast(last_type, return_type));
    }

#if MULTI_VALUE
    // 4. emit multi value result, type after result will unexpectedly yield array [result, type] in wasmtime, js, … ABI!
    block.addConst32(last_type.value);

    // 4. emit multi value result, type BEFORE value
    // SWAP as we want the type to be on the stack IN FRONT of the value, so functions are backwards compatible and only depend on the func_type signature
    // this will result in an array [type, result] though, so DON'T SWAP!
//    int result_local = context.locals["result"].position;
//    block.add(tee_local);
//    block.addInt(result_local);
//    block.addConst32(last_type.value);
//    block.addConst64(last_type.value);
//    block.add(get_local);
//    block.addInt(result_local);
#endif


    //	if(context=="wasp_main" or (context.abi==wasp and returnTypes.size()<2))
    //		block.addInt(last_type);// should have been added to signature before, except for main!
    // make sure block has correct wasm type signature!

    //	var [type, data]=result // wasm order PRESERVED! no stack inversion!

    //if not return_block
    // the return statement makes drops superfluous and just takes as many elements from stack as needed (ggf multiple and ignores/drops the rest!)
    // to check if all parts of wasp are working flawlessly we may drop the return_block
    block.addByte(return_block);
    block.addByte(end_block);
    last_code_byte = block.length;
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


//    01 14 03 5f 02 7c 01 7c
// 01: type section
// 14: length
// 03: type count
// 5f: reference type "struct"
// 02: field count
// 7c: f64 01: mutable …
Code emitTypeSectionStructs(int &typeCount) {
    Code type_data;
    for (auto &name: types) {
        Node &type = *types[name];
        // todo 1. ALL structs? 2. only sealed structs?
        if (type.kind != structs) // wasmtype_struct
            continue; // todo classes too?
        type.value.longy = typeCount++;
        print("struct "s + name + " " + type.value.longy);
        type_data.addByte(0x5f /*wasmtype_struct*/);
        type_data.addByte(type.size()); // field count
        for (auto field: type) {
            auto field_type = field.type;
            auto valtype = fixValtype(mapTypeToWasm(field_type));
            type_data.addByte(valtype);
            type_data.addByte(0x01 /*mutable*/);
        }
    }
    return type_data;
}

Map<Type, int> arrayTypes;

Code emitTypeSectionArrays(int &typeCount) {
    Code type_data;
    for (auto &name: types) {
        Node &type = *types[name];
        if (type.kind != arrays) // wasmtype_array
            continue;
        type.value.longy = typeCount++;
        auto array_type = mapType(type.type);
        print("array "s + name + " of type " + typeName(array_type));
        type_data.addByte(0x5e /*wasmtype_array*/);
        type_data.addByte(mapTypeToWasm(array_type));
        type_data.addByte(0x01 /*mutable*/);
        arrayTypes[array_type] = type.value.longy;
    }
    if (arrayTypes.size() > 0)
        return type_data;
    else return {};
}


int last_index = -1;


// typeSection created before code Section. All imports must be known in advance!
[[nodiscard]]
Code emitTypeSection() {
    int typeCount = 0;
    // Function types are vectors of parameters and return types. Currently
    // the type section is a vector of context types
    // TODO optimise - some of the procs might have the same type signature
    Code type_data;
    //	print(functionIndices);
    for (String fun: functions) {
        if (!fun) {
            //			print(functionIndices);
            //			print(functions);
            print(fun);
            breakpoint_helper
            if (start.empty())
                warn("empty functions[ø] because context=start=''");
            else
                error("empty context creep functions[ø]");
            continue;
        }
        if (operator_list.has(fun)) {
            todow("how did we get here?");
            continue;
        }
        if (is_operator(fun[0])) {
            todo("how did we get here?");
            continue;
        } // todo how did we get here?
        Function &function = functions[fun];
        Signature &signature = function.signature;
        if (not function.is_declared /*export/declarations*/ and not function.is_used /*imports*/) {
            //            trace("not context.emit => skipping unused type for "s + fun);
            continue;
        }
        if (not call_indices.has(fun)) {
            //            warn("call_index %d for %s too late?"s % (last_index + 1) % fun);
            function.call_index = ++last_index;
            call_indices[fun] = last_index;
        }

        if (function.is_runtime)
            continue;
        if (function.signature.is_handled)
            continue;
        //		if(context.is_import) // types in import section!
        //			continue;

        //			error("context %s should be registered in functionIndices by now"s % fun);

        typeMap[fun] = typeCount++;
        function.signature.type_index = typeMap[fun]; // todo check old index? todo shared signatures!?!
        function.signature.is_handled = true; // todo remove
        int param_count = signature.size();
        //		Code td = {0x60 /*const type form*/, param_count};
        Code td = Code(func) + Code(param_count);

        for (int i = 0; i < param_count; ++i) {
            td += Code(fixValtype(mapTypeToWasm(signature.parameters[i].type)));
        }
        td.addByte(signature.return_types.size());
        for (Type ret: signature.return_types) {
            Valtype valtype = fixValtype(mapTypeToWasm(ret));
            td.addByte(valtype);
        }
        type_data += td;
    }

    if (use_wasm_structs)
        type_data += emitTypeSectionStructs(typeCount);
    if (use_wasm_arrays)
        type_data += emitTypeSectionArrays(typeCount);

    return Code((char) type_section, encodeVector(Code(typeCount) + type_data)).clone();
}

Valtype fixValtype(Valtype valtype) {
    if (valtype == (Valtype) charp) return int32t;
    if ((int) valtype >= node)
        error("exposed internal Valtype");
    //    if (valtype > 0xC0)error("exposed internal Valtype");
    return valtype;
}

[[nodiscard]]
Code emitImportSection() {
    // the import section is a vector of imported functions
    Code import_code;
    import_count = 0;
    for (String fun: functions) {
        Function &function = functions[fun];
        if (fun.empty() && function.is_used) {
            //            error("empty function bug");
            warn("empty function bug");
            continue;
        }
        String import_module = "env";
        if (function.module and not function.module->name.empty())
            import_module = function.module->name;
        if (function.is_import and function.is_used and not function.is_builtin) {
            if (function.call_index >= 0)
                check_silent(function.call_index == import_count) // todo remove when tested
            function.call_index = import_count++;
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
        extra_mem = 1; // add to import_section but not to functions:import_count
        int init_page_count = 1024; // 64kb each, 65336 pages max
        import_code =
                import_code + encodeString("env") + encodeString("memory") + (byte) mem_export/*type*/ + (byte) 0x00 +
                Code(init_page_count);
    }
    if (import_code.length == 0)return Code();
    auto importSection = createSection(import_section, Code(import_count + extra_mem) + import_code); // + sqrt_ii
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
        root.kind = expression; // todo why hack?

    //	int new_count;
    //	new_count = declaredFunctions.size();
    //    check_is(last_index,import_count)

    // todo: this whole block can be removed as call_indices are assigned earlier
    for (auto declared: functions) {
        if (declared == "global")continue;
        Function &function = functions[declared]; // todo use more often;)
        if (not function.is_declared)continue;
        function.is_import = false;
        if (declared.empty())
            error("Bug: empty context name (how?)");
        if (declared != "wasp_main") println("declared context: "s + declared);
        if (not call_indices.has(declared)) {
            // used or not!
            if (function.call_index >= 0 or function.code_index >= 0)
                error("function %s #%d already has index %d ≠ %d"s %
                (String) function % function.code_index % function.call_index % last_index + 1);
            function.code_index = ++last_index;
            call_indices[declared] = last_index;
        }
    }

    int fd_write_import = call_indices.has("fd_write") ? call_indices["fd_write"] : 0;
    int main_offset = call_indices.has(start) ? call_indices[start] : 0;
    if (main_offset >= 0x80) todow("leb main_offset")

    int print_node_import = call_indices.has("printNode") ? call_indices["printNode"] : 0;

    // https://pengowray.github.io/wasm-ops/
    //	char code_data[] = {0x01,0x05,0x00,0x41,0x2A,0x0F,0x0B};// 0x41==i32_auto  0x2A==42 0x0F==return 0x0B=='end (context block)' opcode @+39
    //	byte code_fourty2[] = {0/*locals_count*/, i32_auto, 42, return_block, end_block};
    byte code_nop[] = {0/*locals_count*/, end_block}; // NOP
    byte code_start[] = {0/*locals_count*/, call_, (byte) main_offset, nop_, nop_, drop, end_block}; // needs own type
    //    if(print_node_import)
    // needs runtime or merge with print_node.wasm
    //    byte code_start[] = {0 , call_, (byte) main_offset, nop_, nop_,call_,(byte)print_node_import, end_block};
    byte code_id[] = {1/*locals_count*/, 1/*one of type: */, i32t, get_local, 0, return_block, end_block}; // NOP
    byte code_square_d[] = {
        1/*locals_count*/, 1/* of type: */, f64t, get_local, 0, get_local, 0, f64_mul,
        return_block, end_block
    };

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
    byte code_put_string[] = {
        1/*locals_count*/, 1 /*one of type*/, int32t /* string& */ ,
        i32_const, 1, // stdout
        local_get, 0, // string* or char** ⚠️ use put_chars for char*
        i32_const, 1, // #string
        i32_const, 8, // out chars written => &trash
        call_, (byte) fd_write_import, nop_, nop_,
        end_block
    };

    // char* in wasp abi always have header at -8
    byte code_puts[] = {
        1/*locals_count*/, 1 /*one of type*/, int32t /* string& */ ,
        i32_const, 1, // stdout
        local_get, 0, // string* or char** ⚠️ use put_chars for char*
        i32_const, 8, i32_sub, //  char* in wasp abi always have header at -8
        i32_const, 1, // #string
        i32_const, 8, // out chars written => &trash
        call_, (byte) fd_write_import, nop_, nop_,
        end_block
    };


    byte code_len[] = {
        1/*locals_count*/, 1 /*one of type*/, int32t /* wasm_pointer */ ,
        local_get, 0, // any structure in Wasp ABI
        i32_const, 4, // length is second field in ALL Wasp structs!
        i32_add, // offset = base + 4
        i32_load, 2, 0, end_block
    };

    // slightly confusing locals variable declaration count scheme:
    byte code_modulo_float[] = {
        1 /*locals declarations*/, 2 /*two of type*/, float32t,
        0x20, 0x00, 0x20, 0x00, 0x20, 0x01, 0x95, 0x8f, 0x20, 0x01, 0x94, 0x93, 0x0b
    };
    byte code_modulo_double[] = {
        1 /*locals variable declarations:*/, 2 /*two of type*/, float64t,
        0x20, 0x00, //                     | local.get 0
        0x20, 0x00, //                     | local.get 0
        0x20, 0x01, //                     | local.get 1
        0xa3, //                     | f64.div
        0x9d, //                     | f64.trunc
        0x20, 0x01, //                     | local.get 1
        0xa2, //                     | f64.mul
        0xa1, //                     | f64.sub
        0x0b //                     | end
    };
    Code code_blocks;

    // order matters, in functionType section!
    //        if (functions["nop"].is_used)// NOT a function
    //            code_blocks += encodeVector(Code(code_nop, sizeof(code_nop)));
    if (functions["square_double"].is_used and functions["square_double"].is_builtin)
        // simple test function x=>x*x can also be linked via runtime/import!
        code_blocks += encodeVector(Code(code_square_d, sizeof(code_square_d)));
    if (functions["id"].is_used)
        code_blocks += encodeVector(Code(code_id, sizeof(code_id)));
    if (functions["modulo_float"].is_used)
        code_blocks += encodeVector(Code(code_modulo_float, sizeof(code_modulo_float)));
    if (functions["modulo_double"].is_used)
        code_blocks += encodeVector(Code(code_modulo_double, sizeof(code_modulo_double)));
    if (functions["len"].is_used)
        code_blocks += encodeVector(Code(code_len, sizeof(code_len)));
    if (functions["puts"].is_used) // calls import fd_write, can be import itself
        code_blocks += encodeVector(Code(code_puts, sizeof(code_puts)));
    if (functions["put_string"].is_used) // calls import fd_write, can be import itself
        code_blocks += encodeVector(Code(code_put_string, sizeof(code_put_string)));
    if (functions["quit"].is_used)
        code_blocks += encodeVector(Code(code_quit, sizeof(code_quit)));

    Code main_block = emitBlock(root, functions["wasp_main"]); // after imports and builtins

    if (main_block.length == 0)
        functions[start].is_used = false;
    else
        code_blocks = code_blocks + encodeVector(main_block);


    for (String fun: functionCodes) {
        // MAIN block extra ^^^
        Code &func = functionCodes[fun];
        code_blocks = code_blocks + encodeVector(func);
    }

    if (functions["_start"].is_used and functions["_start"].is_builtin)
        code_blocks = code_blocks + encodeVector(Code(code_start, sizeof(code_start)));

    builtin_count = 0;
    for (auto name: functions) {
        Function &context = functions[name];
        if (context.is_builtin and context.is_used)
            builtin_count++;
    }

    bool has_main = start and call_indices.has(start);
    int function_codes = functionCodes.size();
    function_block_count = has_main /*main*/ + builtin_count + function_codes;
    // 0a:code_section 30:len 02:block_count ( 19:len-block-1  02:locals 01:one-of 7f 01:one-of  7e … body … 0b: end_block)
    auto codeSection = createSection(code_section, Code(function_block_count) + code_blocks);
    return codeSection.clone();
}

Code emitStringSection() {
    if (not use_wasm_strings or wasm_strings.size() == 0)
        return Code();
    Code strings(0); // ?
    strings.addInt(wasm_strings.size()); // count of strings
    for (auto &s: wasm_strings) {
        strings += encodeString(s);
    }
    return createSection(string_section, strings);
}

[[nodiscard]]
Code emitExportSection() {
    //    https://webassembly.github.io/spec/core/binary/modules.html#binary-exportsec
    int exports_count = 1; // just main … todo easy
    // the export section is a vector of exported functions etc
    if (!start) // todo : allow arbirtrary exports, or export all
        return createSection(export_section, Code(0));
    int main_offset = 0;
    if (call_indices.has(start))
        main_offset = call_indices[start];
    Code memoryExport; // empty by default
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
#if not MICRO // WAMR doesn't like _start with return as per spec OK
    if (use_wasi) {
        exports_count++;
        int start_offset = main_offset;
        if (call_indices["_start"])
            start_offset = call_indices["_start"];
        mainExport = mainExport + encodeString("_start") + (byte) func_export + Code(start_offset);
    }
#endif

    Code globalExports;
    for (int i = 0; i < globals.size(); i++) {
        String &name = globals.keys[i];
        if (globals[name].index != i)
            error("global index mismatch");
        Code globalExport = encodeString(name) + (byte) global_export + Code(i);
        globalExports.add(globalExport); // todo << NOW
        exports_count++;
    }
    Code exportsData = encodeVector(
        Code(exports_count) + memoryExport + mainExport + globalExports);

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
    if (global_user_count == 0)return Code();
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
        Global global = globals.values[i];
        Node *global_init_node = global.value;
        //        Type &type = globals[global_name].type;
        Type &type = global.type;
        //        Valtype valtype = mapTypeToWasm(*global_node);
        Valtype valtype = mapTypeToWasm(type);

        check_eq_or(global.index, i, ("global index mismatch "s + global_name + " " + global.index + " != " + i).data);
        check_eq_or(global.name, global_name, "global name mismatch")
        if (not global_init_node) {
            warn("missing value for global "s + global_name);
            global_init_node = new Node(); // dummy
        }
        if (global.is_import or global_init_node->has("import"))
            continue;
        if (global_init_node->kind == expression) {
            // 1+2 is ok 1+π not
            warn("only the most primitive expressions are allowed in global initializers => move to wasp_main!");
            global_init_node = new Node(42); // dummy init
            global_init_node->kind = longs;
        }
        //        check_is(type, valtype);
        //        check_is(type, mapType(*global_node));
        //        if (valtype == i64)valtype = int32;// dirty hack for global x = 1 + 2
        globalsList.addByte(valtype);
        globalsList.addByte(global.is_mutable); // 1:mutable todo: default? not π ;)
        // expression set in analyse->groupOperators  if(name=="::=")globals[prev.name]=&next;
        //        Function fun{.name = global_name};
        last_type = valtype;
        if (global_init_node->empty())
            error("empty global initializer for "s + global_name);
        Function context{.name = "global"};
        const Code &globalInit = emitExpression(global_init_node, context); // todo ⚠️ global is not a context!
        globalsList.add(globalInit); // todo names in global context!?
        //        }
        //        else if (global_node->kind == longs) { // todo: just use emitExpression but keep i64 not i32!
        //            Code globalInit;
        //            globalInit.addByte((byte) i64_const);
        //            globalInit.push(global_node->value.longy);
        //            globalsList.add(globalInit);// todo names in global context!?
        //        }
        //        globalsList.add(cast(last_type, valtype));// instruction not valid in initializer expression: i64.extend_i32_s
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
    };
    last_type = none; // don't leak!
    auto globalSection = createSection(global_section, globalsList);
    return globalSection;
}


//// todo use these for named data e.g. named strings and arrays
//int named_data_segments = 0; // first is just "wasp_data" zero page
//List<int> data_segment_offsets;
//List<String> data_segment_names;

[[nodiscard]]
Code emitDataSections() {
    // needs memory section too!
    // split for named data e.g. named strings and arrays
    //https://webassembly.github.io/spec/core/syntax/modules.html#syntax-datamode
    Code datas;
    if (data_index_end == 0 or data_index_end == runtime_data_offset)return datas; //empty
    // see clearEmitterContext() for NULL PAGE of data_index_end

    datas.addByte(named_data_segments + 1); // n memory data segment initialization

    for (int i = 0; i <= named_data_segments; i++) {
        datas.addByte(00); // memory id always 0 until multi-memory
        datas.addByte(0x41); // opcode for i32.const offset: followed by unsignedLEB128 value:
        int offset = runtime_data_offset; // or 0
        int end = data_index_end;
        if (i > 0)offset = data_segment_offsets[i - 1]; // runtime_data_offset builtin ?
        if (i < named_data_segments)end = data_segment_offsets[i];
        datas.addInt(offset); // actual offset in memory
        // todo: WHY cant it start at 0? wx  todo: module offset + module data length
        datas.addByte(0x0b); // mode: active?
        auto size_of_data = end - offset;
        check_silent(size_of_data >= 0, "data segment beyound end");
        datas.addInt(size_of_data);
        const Code &actual_data = Code((bytes) data + offset, size_of_data);
        datas.add(actual_data); // now comes the actual data  encodeVector()? nah manual here!
    }
    return createSection(data_section, encodeVector(datas)); // size added via actual_data
}

[[nodiscard]]
Code emitDataSection() {
    // needs memory section too!
    //https://webassembly.github.io/spec/core/syntax/modules.html#syntax-datamode
    Code datas;
    if (data_index_end == 0 or data_index_end == runtime_data_offset)return datas; //empty
    // see clearEmitterContext() for NULL PAGE of data_index_end
    datas.addByte(01); // one memory initialization / data segment

    datas.addByte(00); // memory id always 0 until multi-memory
    datas.addByte(0x41); // opcode for i32.const offset: followed by unsignedLEB128 value:
    datas.addInt(runtime_data_offset ? runtime_data_offset : 0); // actual offset in memory
    // todo: WHY cant it start at 0? wx  todo: module offset + module data length
    datas.addByte(0x0b); // mode: active?
    auto size_of_data = data_index_end - runtime_data_offset;
    datas.addInt(size_of_data);
    //    const Code &actual_data = Code((bytes) data, size_of_data);
    // todo: WASTEFUL but clean, add/substruct offsets everywhere would be unsafe
    const Code &actual_data = Code((bytes) data + runtime_data_offset, size_of_data);
    datas.add(actual_data); // now comes the actual data  encodeVector()? nah manual here!
    return createSection(data_section, encodeVector(datas)); // size added via actual_data
}

// Signatures
[[nodiscard]]
Code emitFuncTypeSection() {
    // depends on codeSection, but must appear earlier in wasm
    //    https://webassembly.github.io/spec/core/binary/modules.html#binary-funcsec

    // funcType_count = function_count EXCLUDING imports, they encode their type inline!
    // the context section is a vector of type indices that indicate the type of each context in the code section

    Code types_of_functions = Code(function_block_count); //  = Code(types_data, sizeof(types_data));
    //	order matters in functionType section! must be same as in functionIndices
    for (int code_index = 0; code_index < function_block_count; code_index++) {
        //	import section types separate WTF wasm
        int call_index = code_index + import_count + runtime_function_offset;
        String *fun = call_indices.lookup(call_index);
        if (!fun) {
            print(call_indices);
            error("missing typeMap for index "s + call_index);
        } else {
            int typeIndex = typeMap[*fun];
            if (typeIndex >= 0) // just an implicit list funcId->typeIndex
                types_of_functions.push((int) typeIndex, false);
            else if (runtime_function_offset == 0) // todo else ASSUME all handled correctly before
                error("missing typeMap for context %s index %d "s % fun % code_index);
        }
    }
    // @ WASM : WHY DIDN'T YOU JUST ADD THIS AS A FIELD IN THE FUNC STRUCT???
    Code funcSection = createSection(functypes_section, types_of_functions);
    return funcSection.clone();
}

[[nodiscard]]
Code functionSection() {
    return emitFuncTypeSection(); // (misnomer) vs codeSection() !
}

// todo : convert library referenceIndices to named imports!
[[nodiscard]]
Code emitNameSection() {
    Code nameMap;

    int total_func_count = last_index + 1; // imports + function_count, all receive names
    int usedNames = 0;
    // (import "env" "putf" (func $putf (type 1))) imports have name too
    for (int index = runtime_function_offset; index < total_func_count; index++) {
        // danger: utf names are NOT translated to wat env.√=√ =>  (import "env" "\e2\88\9a" (func $___ (type 3)))
        String *name = call_indices.lookup(index);
        if (not name) todo("no name for %d! bug (not enough mem?)"s % index);
        nameMap = nameMap + Code(index) + Code(*name);
        usedNames += 1;
    }

    Code localNameMap;
    int usedLocals = 0;
    for (int index = runtime_function_offset; index <= last_index; index++) {
        String *key = call_indices.lookup(index);
        if (!key or key->empty())continue;
        Function &context = functions[*key];
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
    int usedGlobals = globals.count(); // currently all
    for (int i = 0; i < globals.count(); i++) {
        String &globalName = globals.keys[i];
        globalNameMap = globalNameMap + Code(i) + Code(globalName);
    }

    Code typeNameMap;
    Code fieldNameMap;
    int usedTypes = 0;
    int usedFields = 0;
    for (auto &type_name: types) {
        auto typ = *types[type_name];
        if (typ.kind != structs) // wasmtype_struct
            continue;
        usedTypes++;
        int typ_index = typ.value.longy;
        typeNameMap = typeNameMap + Code(typ_index) + Code(typ.name);
        // collect field names
        int field_index = 0;
        fieldNameMap = Code(typ_index) + Code(typ.size());
        for (auto &field: typ) {
            fieldNameMap = fieldNameMap + Code(field_index++) + Code(field.name);
            usedFields++;
        }
    }
    //    fieldNameMap = Code(usedTypes) + fieldNameMap; // prefixed !


    //	localNameMap = localNameMap + Code((byte) 0) + Code((byte) 1) + Code((byte) 0) + Code((byte) 0);// 1 unnamed local
    //	localNameMap = localNameMap + Code((byte) 4) + Code((byte) 0);// no locals for function4
    //	Code exampleNames= Code((byte) 5) /*context index*/ + Code((byte) 1) /*count*/ + Code((byte) 0) /*l.nr*/ + Code("var1");
    // context 5 with one local : var1

    //	localNameMap = localNameMap + exampleNames;

    auto moduleName = Code(module_name) + encodeVector(Code("wasp_module"));
    auto functionNames = Code(function_names) + encodeVector(Code(usedNames) + nameMap);
    auto localNames = Code(local_names) + encodeVector(Code(usedLocals) + localNameMap);
    auto typeNames = Code(type_names) + encodeVector(Code(usedTypes) + typeNameMap);
    auto globalNames = Code(global_names) + encodeVector(Code(usedGlobals) + globalNameMap);

    // custom data section for data names split in emitDataSections
    //    auto dataNames = Code(data_names) + encodeVector(Code(1)/*count*/ + Code(0) /*index*/ + Code("wasp_data"));
    auto dataNames = Code();
    dataNames.addInt(named_data_segments + 1); // Total count of named data segments
    dataNames.addInt(0); // Index of the data segment
    dataNames.add(Code("wasp_data")); // Name of the data segment
    for (int i = 1; i <= named_data_segments; i++) {
        dataNames.addInt(i); // Index of the data segment
        String &name = data_segment_names[i - 1];
        dataNames.add(Code(name)); // Name of the data segment
    }
    dataNames = Code(data_names) + encodeVector(dataNames);

    auto fieldNames = Code(field_names) + encodeVector(Code(usedFields) + fieldNameMap); // usedTypes ??

    //	The name section is a custom section whose name string is itself ‘𝚗𝚊𝚖𝚎’.
    //	The name section should appear only once in a module, and only after the data section.
    const Code &nameSectionData = encodeVector(
        Code("name") + moduleName + functionNames + localNames + typeNames + globalNames + dataNames + fieldNames);
    // global names are part of global section, as should be
    auto nameSection = createSection(custom_section, nameSectionData); // auto encodeVector AGAIN!
    //    nameSection.debug();
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
    For each such instruction as R_WASM_FUNCTION_INDEX_LEB or R_WASM_TABLE_INDEX_SLEB reloc entry is generated
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
    short type = 5; // SEGMENT alignment & flags
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
    // see emitDwarfSections in DwarfEmitter.cpp
    return createSection(custom_section, encodeVector(Code("external_debug_info") + Code("main.dwarf")));
}

[[nodiscard]]
Code emitProducers() {
    Code producers;
    producers.addByte(1); // count
    producers += Code("wasp "s + wasp_version);
    return createSection(custom_section, encodeVector(Code("producers") + producers));
}

[[nodiscard]]
Code emitTargetFeatures() {
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
    //	Strings features={"simd128","nontrapping-fptoint","sign-ext","tail-call","mutable-global","bulk-memory","exception-handling","atomics","multivalue","gc","reference-types","memory64","memory-packing","simd","relaxed-simd","threads","multi-value","tail-call","reference-types","bulk-memory","nontrapping-fptoint","sign-ext","simd128","tail-call","exception-handling","atomics","mutable-globals","nontrapping-fptoint","sign-ext","simd128","tail-call"};
    Strings features = {"sign-ext", "mutable-globals"};
    Code code;
    code += (byte) (features.size());
    for (String feature: features) {
        code.addByte(0x2b); // "+"
        code += Code(feature);
    }
    return createSection(custom_section, encodeVector(Code("target_features") + code));
};

[[nodiscard]]
Code emitSourceMap(String program) {
    //	Code code=Code("data:text/plain;base64,"s + base64_encode("//# sourceMappingURL=main.wasm.map"));
    //	Code code=Code("data:text/json;base64,"s + base64_encode("//# sourceMappingURL=main.wasm.map"));
    //	Code code = Code("data:text/json;base64,"s + base64_encode(program));
    //	Code code = Code("http://localhost:8000/main.wasm.map");
    Code code = Code("main.wasm.map");
    return createSection(custom_section, encodeVector(Code("sourceMappingURL") + code));
}

// see preRegisterSignatures
void add_imports_and_builtins() {
    import_count = 0;
    builtin_count = 0;
    for (auto sig: functions) {
        // imports first
        Function &function = functions[sig];
        if (function.is_import and function.is_used) {
            if (function.code_index >= 0)
                error("imports have no code_index!");
            if (function.call_index >= 0 and function.call_index != last_index)
                error("context already has index %d ≠ %d"s % function.code_index % last_index);
            function.call_index = ++last_index;
            call_indices[sig] = last_index;
            info("using import "s + sig);
            trace(function.name + function.signature.serialize());
            import_count++;
        }
    }
    for (auto sig: functions) {
        // now builtins
        Function &function = functions[sig];
        if (function.is_builtin and function.is_used) {
            if (function.code_index >= 0 and function.code_index + 1 != last_index) {
                werror("function %s already has index %d ≠ %d"s % function.name % function.code_index % last_index);
                call_indices[sig] = function.code_index;
            } else {
                function.call_index = ++last_index;
                function.code_index = function.call_index - import_count;
                call_indices[sig] = last_index;
                builtin_count++;
            }
        }
    }
}

[[nodiscard]]
Code emitMemorySection() {
    if (memoryHandling == import_memory or memoryHandling == no_memory) return Code(); // handled elsewhere
    /* limits https://webassembly.github.io/spec/core/binary/types.html#limits - indicates a min memory size of one page */
    //	int pages = 1;//  traps while(i<65336/4)k#i=0
    int pages = 1024 * 10; // 64kb each, 65336 pages max. makes VM slower?
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
    call_indices.setDefault(-1);
    call_indices.clear(); // ok preregistered functions are in functions
    //	functionCodes.setDefault(Code());
    typeMap.setDefault(-1);
    typeMap.clear();
    initTypes();

    //	referenceMap.setDefault(Node());
    //    runtime_data_offset = 0x100000;
    named_data_segments = 0;
    data_segment_offsets.clear();
    data_segment_names.clear();
    data_index_end = runtime_data_offset; //0
    last_object_pointer = 0;
    if (!data) data = (char *) calloc(MAX_WASM_DATA_LENGTH, sizeof(char)); // todo grow
    else memset(data, 0, MAX_WASM_DATA_LENGTH);
    while ((int64) data % 8)data++; // pre-align!
    emitLongData(0, true); // NULL PAGE! no object shall ever read or write from address 0 (sanity measure)
    emitLongData(0, true); // TRASH sink, e.g. for writing fd_write(fd,iov*,len, &trash out)
    //    emitString(*new Node("__WASP_DATA__\0"), *new Function());
    //    while (((int64) data) % 8)data++;// type 'int64', which requires 8 byte alignment
}

Code emitDwarfSections(); // sorry, no DwarfEmitter.h

[[nodiscard]]
Code &emit(Node &root_ast, String program) {
    memoryHandling = export_memory;
    //        memoryHandling = import_memory; // works for micro-runtime
    //        memoryHandling = internal_memory; // works for wasm3
    last_index = -1;
    runtime_function_offset = 0;
    add_imports_and_builtins();
    start = "wasp_main"; // necessary, else docs fail
    functions[start].is_declared = true;
    //    const Code customSectionvector;
    Code const &customSectionvector = encodeVector(Code("custom123") + Code("random custom section data"));
    auto customSection = createSection(custom_section, customSectionvector);
    Code typeSection1 = emitTypeSection(); // types must be defined in analyze(), not in code declaration
    Code importSection1 = emitImportSection(); // needs type indices
    Code globalSection1 = emitGlobalSection(); //
    Code codeSection1 = emitCodeSection(root_ast); // needs functions and functionIndices prefilled!! :(
    Code funcTypeSection1 = emitFuncTypeSection(); // signatures depends on codeSection, but must come before it in wasm
    Code memorySection1 = emitMemorySection();
    Code exportSection1 = emitExportSection(); // depends on codeSection, but must come before it!!

    Code code = Code(magicModuleHeader, 4)
                + Code(moduleVersion, 4)
                + typeSection1
                + importSection1
                + funcTypeSection1 // signatures
                + memorySection1 // Wasm MVP can only define one memory per module WHERE?
                + emitStringSection() // wasm stringref table
                + globalSection1
                + exportSection1
                + codeSection1 // depends on importSection, yields data for funcTypeSection!
                + emitDataSections()
                //			+ linkingSection()
                + emitNameSection()
                //	              + emitDwarfSections()  // https://yurydelendik.github.io/webassembly-dwarf/
                //	              + emitProducers()
                //	              + emitTargetFeatures()
                + emitSourceMap(program)

            //	 + customSection
            ;
    return code.clone();
}

Code &compile(String code, bool clean) {
    if (clean) {
        clearEmitterContext();
        clearAnalyzerContext(); // needs to be outside analyze, because analyze is recursive
    }
    Node parsed = parse(code);
    //    print(parsed.serialize());
    Node &ast = analyze(parsed, functions["wasp_main"]);
    Code &binary = emit(ast, code);
    //    binary.debug();
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
#else
    if (libraries.size() > 0)
        warn("wasp compiled without binary linking/merging. set(INCLUDE_MERGER 1) in CMakeList.txt");
#endif
    return binary;
}


#pragma clang diagnostic pop
