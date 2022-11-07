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

int runtime_offset = 0; // imports + funcs
int import_count = 0;
short builtin_count = 0;// function_offset - import_count - runtime_offset;

//bytes data;// any data to be stored to wasm: values of variables, strings, nodes etc
char *data;// any data to be stored to wasm: values of variables, strings, nodes etc ( => memory in running app)
//int data_index_start = 0;
int data_index_end = 0;// position to write more data = end + length of data section
int last_data = 0;// last pointer outside stack
//Map<String *, long> referenceDataIndices; // wasm pointers to strings within wasm data WITHOUT runtime offset!
Map<String, long> referenceIndices; // wasm pointers to objects (currently: arrays?) within wasm data
Map<String, long> referenceDataIndices; // wasm pointers directly to object data, redundant ^^
Map<String, Node> referenceMap; // lookup types…

//Map<long,int> dataIndices; // wasm pointers to strings etc (key: hash!)  within wasm data



Module runtime;
String start = "main";

Valtype arg_type = voids;// autocast if not int
Valtype last_type = voids;// autocast if not int
Type last_typo = Kind::unknown;
Node *last_object = 0;

enum MemoryHandling {
	import_memory,
	export_memory,
	internal_memory,// declare but don't export
	no_memory,
};
MemoryHandling memoryHandling;// set later = export_memory; // import_memory not with mergeMemorySections!

//Map<String, Valtype> return_types;
//Map<int, List<String>> locals;
//Map<int, Map<int, String>> locals;
//List<String> declaredFunctions; only new functions that will get a Code block, no runtime/imports
Map<String, int> functionIndices;
Map<String, Code> functionCodes;
Map<String, int> typeMap;

Code createSection(Sections sectionType, Code data);

[[nodiscard]]
Code Call(char *symbol);//Node* args

//Code& unsignedLEB128(int);
//Code& flatten(byter);
//Code& flatten (Code& data);
//void todo1(char *message = "") {
//	printf("TODO %s\n", message);
//}

//typedef int number;
//typedef char byte;
//typedef char* Code;
//typedef char id;




//class Bytes {
//public:
//	int length;
//};


// https://pengowray.github.io/wasm-ops/
// values outside WASM ABI: 0=unknown/handled internally
// todo: norm operators before!
// * ∗ ⋅ ⋆ ✕ ×  …
byte opcodes(chars s, Valtype kind, Valtype previous = none) {
	//	previous is lhs in binops!

//	if(eq(s,"$1="))return set_local;
//	if (eq(s, "=$1"))return get_local;
//	if (eq(s, "=$1"))return tee_local;
	if (kind == unknown_type)
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
		if (eq(s, "&&"))return i32_and; // i32.and

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
		if (eq(s, "/"))return i64_di𝗏_s; // i64.di𝗏_s
		if (eq(s, "%"))return i64_rem_s; // i64.rem_s
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
		if (eq(s, "&&"))return i64_and; // i64.and

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

	breakpoint_helper
	trace("unknown or non-primitive operator %s\n"s % String(s));
	// can still be matched as function etc, e.g.  2^n => pow(2,n)   'a'+'b' is 'ab'
//		error("invalid operator");
	return 0;
}

// http://webassembly.github.io/spec/core/binary/modules.html#export-section
enum ExportType {
	func_export = (char) 0x00,
	table_export = 0x01,
	mem_export = 0x02,
	global_export = 0x03
};

// http://webassembly.github.io/spec/core/binary/types.html#function-types
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


enum NodTypes {
	numberLiteral,
	identifier,
	binaryExpression,
	printStatement,
	variableDeclaration,
	variableAssignment,
	functionDeclaration,
	whileStatement,
	ifStatement,
	callStatement,
	internalError,
};


bytes ieee754(float num) {
	char data[4];
	float *hack = ((float *) data);
	*hack = num;
	byte *flip = static_cast<byte *>(alloc(1, 5));
	short i = 4;
//	while (i--)flip[3 - i] = data[i];
	while (i--)flip[i] = data[i];// don't flip, just copy to malloc
	return flip;
}

bytes ieee754(double num) {
	char data[8];
	double *hack = ((double *) data);
	*hack = num;
	byte *flip = static_cast<byte *>(alloc(1, 9));
	short i = 8;
	while (i--)flip[i] = data[i];// don't flip, just copy to malloc
	return flip;
}
//Code emitExpression (Node* nodes);

// pure data ready to be emitted
bool isProperList(Node &node) {
	if (node.kind != groups and node.kind != objects) return false;
	if (node.length < 1) return false;
	for (Node child: node) {
		if (child.kind != longs and child.kind != strings)// todo … evaluate?
			return false;
		if (child.isSetter())
			return false;
	}
	return true;
}

void emitIntData(int i) {// append int to wasm data memory
	*(int *) (data + data_index_end) = i;
	data_index_end += 4;
//	while (((long)(data + data_index_end)%16))data_index_end++;// type 'long', which requires 4 byte alignment DO IT BEFORE!
}

[[nodiscard]]
Code emitPrimitiveArray(Node &node, String context) {
	let code = Code();
	if (!(node.kind == buffers)) todo("arrays of type "s + typeName(node.kind));
	emitIntData(array_header_32); // array header
	int length = *(int *) node.value.data;// very fragile :(
	emitIntData(length);
//	emitIntData(array_header_32 | int_array << 4 | node.length);
//	emitIntData(node.kind |  node.length); // save 4 bytes, rlly?
	//	if pure_array:
	int pointer = data_index_end; // return pure data
	for (int i = 0; i < length; i++) {
//			if (node.kind.type == int_array) {
		int v = ((int *) node.value.data)[i];
		emitIntData(v);
	}
#ifdef MULTI_VALUE
	// and RETURN
		code.addConst32(array_header_32 | node.length);// combined smart pointer? nah
	//	code.addConst32(node.length);
	//	code.addConst32(array_header_32);
#endif
// the last element of the stack is what is returned if no MULTI_VALUE
// the return statement makes drops superfluous and just takes as many elements from stack as needed (and ignores/drops the rest!)
	code.addConst(pointer);
	last_type = mapTypeToWasm(node);
	return code;
}

[[nodiscard]]
// todo emitPrimitiveArray vs just emitNode as it is (with child*)
Code emitArray(Node &node, String context) {
//	if (node.kind.type == int_array)
	if (node.kind == buffers)
		return emitPrimitiveArray(node, context);
	let code = Code();
//	code.addConst32(node.length);
	int pointer = data_index_end;

//	todo: sync with emitOffset
	emitIntData(array_header_32);
	emitIntData(node.kind);
	emitIntData(node.length);
//	assert_equals((long) data_index_end, (long) pointer + array_header_length);
	for (Node &child: node) {
// todo: smart pointers?
		code.add(emitData(child, context));// pointers in flat i32/i64 format!
	}
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
	last_data = pointer;
	last_type = array;
	return code;// pointer
//	return code.addConst(pointer);// once written to data section, we also want to use it immediately
}


bool isAssignable(Node node, Node *type = 0) {
	//todo
	//	if(node.metas().has("constant") or node.metas().has("immutable")) // …
	//		return false;
	//	if(type and type!=node.type)
	//		return false;
	return true;
}

int currentStackItemSize(Node array) {
	if (last_type == charp)return 1;// chars for now vs codepoint!
	if (last_type == stringp)return 1;// chars for now vs pointer!
	//	if (last_type == int16)return 2;
	if (last_type == int32)return 4;
	if (last_type == array)return 4;// pointer todo!
	if (last_type == int64)return 8;
	if (last_type == float32)return 4;
	if (last_type == float64)return 8;
	if (last_type == void_block)return 4;// int32 pointer hack todo!
	if (last_type == unknown_type)return 4;
	error("unknown size for stack item "s + typeName(last_type));
	return 1;
}

int headerOffset(Node array) {
	switch (array.kind) {
		case arrays:
			return array_header_length;
		case groups:
			return array_header_length;
		default:
			return 0;
	}
}

[[nodiscard]]
Code emitOffset(Node array, Node offset_pattern, bool sharp, String context, int size, int base) {
	Code code;
	if (offset_pattern.kind == reference) {
		const Code &offset = emitExpression(offset_pattern, context);
		code.add(offset);
		if (last_type != i32t)
			error("index must be of int type");
		if (sharp) {// todo remove if offset_pattern was static
			code.addConst(1);
			code.add(i32_sub);
		}
		if (size > 1) {
			code.addConst(size);
			code.add(i32_mul);
		}
		if (base > 0) { // after calculating offset!
			code.addConst(base);
			code.add(i32_add);
		}
	} else if (offset_pattern.kind == longs) {
		int offset = (long) offset_pattern.value.longy;
		if (offset < 1)error("operator # starts from 1, use [] for zero-indexing");
		if (sharp) offset--;

		if (base <= 0)// base may already be provided :(
			base += headerOffset(array);
		// todo: get string size, array length etc 1. at compiletime or 2. at runtime
		// todo: sanity checks 1. at compiletime or 2. at runtime
		//	if(offset_pattern>op[0].length)e
		//	rror("index out of bounds! %d > %d in %s (%s)"s % offset_pattern % );
		code.addConst(base + offset * size);
	} else
		error("index operator todo");
	return code;
	// calculated offset_pattern of 0 ususally points to ~6 bytes after Contents of section Data header 0100 4100 0b08
}

[[nodiscard]]
Code emitIndexWrite(Node array, int base, Node offset, Node value0, String context) {
	int size = currentStackItemSize(array);
	Valtype targetType = last_type;

	//		localTypes[context]
	Valtype valType = last_type;
	Code store;
	store = store + emitOffset(array, offset, true, context, size, base);

	if (value0.kind == strings)//todo stringcopy? currently just one char "abc"#2=B
	{
		char c = value0.value.string->charAt(0);
		store.addConst(c);
	} else
		store = store + emitValue(value0, context);
//	store.add(cast(last_type, valType));
//	store.add(cast(valType, targetType));
	store.add(cast(last_type, targetType));

	if (size == 1)store.add(i8_store);
	if (size == 2)store.add(i16_store);
	if (size == 4)store.add(i32_store);
	if (size == 8)store.add(i64_store);
	//	The static address offset is added to the dynamic address operand
	store.add(size > 2 ? 0x02 : 0);// alignment (?) "alignment must not be larger than natural" size > 2 ? 0x02 :
	store.add(0);// extra offset (why, wasm?)
//	store.add(base);// extra offset (why, wasm?)

	return store;
/*  000101: 41 94 08                   | i32.const 1044
	000104: 41 06                      | i32.const 6
    000106: 36 02 00                   | i32.store 2 0 */
}


// "hi"[0]="H"
[[nodiscard]]
Code emitIndexWrite(Node op, String context) {// todo offset - 1 when called via #!
	return emitIndexWrite(op["array"], 0, op["offset"], op["value"], context);
}

// "hi"#1="H"
[[nodiscard]]
Code emitPatternSetter(Node ref, Node offset, Node value, String context) {
	List<String> &current = locals[context];
	String &variable = ref.name;

	if (!current.has(variable)) {
		current.add(variable);
		error("!! Variable missed by analyzer: "_s + variable);
	}// else:
	int local_index = current.position(variable);
	last_type = localTypes[context][local_index];
	int base = 0;
	if (referenceIndices.has(variable))
		base=referenceDataIndices[variable];// todo?
//	last_type = mapTypeToWasm(value);
	Code code = emitIndexWrite(ref, base, offset, value, context);
	return code;
}


// assumes value is on top of stack
// todo merge with emitIndexRead !
[[nodiscard]]
Code emitIndexPattern(Node array, Node op, String context) {
	if (op.kind != patterns and op.kind != longs and op.kind != reference)error("op expected in emitIndexPattern");
	if (op.length != 1 and op.kind != longs)error("exactly one op expected in emitIndexPattern");
	int base = runtime.data_offset_end;// uh, todo?
	int size = currentStackItemSize(array);
	Node &pattern = op.first();
	Code load = emitOffset(array, pattern, op.name == "#", context, size, base);
	if (size == 1)load.add(i8_load);// i32.load8_u
	if (size == 2)load.add(i16_load);
	if (size == 4)load.add(i32_load);
	if (size == 8)load.add(i64_load);
	load.add(size > 2 ? 0x02 : 0);// alignment (?)
	load.add(0x00);// ?


	if (size == 1)
		last_typo = byte_char;// last_type = codepoint32;// todo and … bytes not exposed in ABI, so OK?
	else if (size <= 4)last_type = int32;
	else last_type = int64;
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
Code emitIndexRead(Node op, String context) {
	if (op.length < 2)
		error("index operator needs two arguments: node/array/reference and position");
	Node &array = op[0];// also String: byte array or codepoint array todo
	Node &pattern = op[1];
	int base = 0;//headerOffset(array); // runtime.data_offset_end;// uh, todo?
	int size = currentStackItemSize(array);
//	if(op[0].kind==strings) todo?
	last_type = arg_type;
	if (last_type == charp)size = 1;// chars for now vs codepoint!
	if (last_type == stringp)size = 1;// chars for now vs codepoint! todo: via runtime String[]!
	if (last_type == int32)size = 4;
	if (last_type == int64)size = 8;
	if (last_type == float32)size = 4;
	if (last_type == float64)size = 8;
	if (array.kind == reference or array.kind == key) {
		String &ref = array.name;
//		last_type=array.data_kind;
		if (referenceIndices.has(ref))// also to strings
			base += referenceDataIndices[ref];
//		else if (stringIndices.has(&ref))
//			base += referenceIndices[ref];
		else
			error("reference not declared as array type: "s + ref);
//		if(referenceMap.has(ref)) {
//			Node &reference = referenceMap[ref];
//			if(reference.type)
//			last_type = mapTypeToWasm(*reference.type);
////			last_typo.clazz
//		}
//		else error("reference should be mapped");

	} else if (array.kind == strings) {
		if (array.value.string)
		base += referenceDataIndices[*array.value.string];
	} else
		base += last_data;// todo: pray!

	Code load = emitOffset(array, pattern, op.name == "#", context, size, base);

	if (size == 1)load.add(i8_load);
	if (size == 2)load.add(i16_load);
	if (size == 4)load.add(i32_load);
	if (size == 8)load.add(i64_load);
	load.add(size > 2 ? 0x02 : 0);// alignment (?)
	load.add(0x00);// ?
//	if (size == 1)last_type = codepoint32;// todo only if accessing codepoints, not when pointing into UTF8 byte!!
	if (size == 1)
		last_typo = byte_char;
	else if (size <= 4)last_type = int32;
	else last_type = int64;
	return load;
	//	i32.const 1028
	//	i32.const 3
	//	i32.load
}

// write data to DATA SEGMENT (vs emitValue on stack)
// MAY return const(pointer)
[[nodiscard]]
Code emitData(Node node, String context) {
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
			if (node.value.longy > 0xF0000000) {
				error("true long big ints currently not supported");
				*(long *) (data + data_index_end) = node.value.longy;
				data_index_end += 8;
				last_type = i64t;
			}
			*(int *) (data + data_index_end) = node.value.longy;
			data_index_end += 4;
			last_type = int32;
			break;
		case reals:
//			bytes varInt = ieee754(node.value.real);
			*(double *) (data + data_index_end) = node.value.real;
			data_index_end += 8;
			last_type = float64;
			break;
		case reference:
			if (locals[current].has(name))
				error("locals dont belong in emitData!");
			else if (referenceIndices.has(name))
				todo("emitData reference makes no sense? "s + name);
			else
				error("can't save unknown reference pointer "s + name);
			break;
		case strings: {
			int stringIndex = data_index_end + runtime.data_offset_end;// uh, todo?
			if (not node.value.string)error("empty node.value.string");
			String string = *node.value.string;
			if (string and referenceDataIndices.has(string)) // todo: reuse same strings even if different pointer, aor make same pointer before
				stringIndex = referenceDataIndices[string];
			else {
				referenceDataIndices.insert_or_assign(string, data_index_end);
				//				Code lens(pString->length);// we follow the standard wasm abi to encode pString as LEB-lenght + data:
				//				strcpy2(data + data_index_end, (char*)lens.data, lens.length);
				//				data_index_end += lens.length;// unsignedLEB128 encoded length of pString
				strcpy2(data + data_index_end, string.data, string.length);
				data[data_index_end + string.length] = 0;
				// we add an extra 0, unlike normal wasm abi, because we have space in data section
				data_index_end += string.length + 1;
			}
			last_type = stringp;
			break;
		}
		case objects:
		case groups:
			// todo hold up: print("ok") should not emit an array of group(string(ok)) !?
			// keep last_type from last child, later if mixed types, last_type=ref or smarty
			return emitArray(node, context);
			break;
		case key:
		case patterns:
		default:
			error("emitData unknown type: "s + typeName(node.kind));
	}
	// todo: ambiguity emit("1;'a'") => result is pointer to [1,'a'] or 'a' ? should be 'a', but why?
	last_data = last_pointer;
	return code.addConst(last_pointer);
}

[[nodiscard]]
Code emitGetGlobal(Node node /* context : global ;) */) {
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
Code emitValue(Node node, String context) {
	Code code;
	String &name = node.name;
	last_typo = node.kind;// careful Kind::strings should map to Classes::c_string after emission!
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
			int local_index = locals[context].position(name);
			if (local_index < 0)error("UNKNOWN symbol "s + name + " in context " + context);
			if (node.value.node) {
				// todo HOLUP! x:41 is a reference? then *node.value.node makes no sense!!!
				code.add(emitSetter(node, *node.value.node, context));

			} else {
				code.addByte(get_local);// todo skip repeats
				code.addByte(local_index);
				if (node.length > 0) {
					return emitIndexPattern(Node(), node, context);
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
			int stringIndex = data_index_end + runtime.data_offset_end;// uh, todo?
			if (!node.value.string)error("missing node.value.string");
			String string = *node.value.string;
			if (referenceDataIndices.has(string)) // todo: reuse same strings even if different pointer, aor make same pointer before
				stringIndex = referenceDataIndices[string];
			else {
				referenceDataIndices.insert_or_assign(string, data_index_end);
//				Code lens(pString->length);// we follow the standard wasm abi to encode pString as LEB-lenght + data:
//				strcpy2(data + data_index_end, (char*)lens.data, lens.length);
//				data_index_end += lens.length;// unsignedLEB128 encoded length of pString
				strcpy2(data + data_index_end, string.data, string.length);
				data[data_index_end + string.length] = 0;
				if (referenceIndices.has(name)) {
					if (not isAssignable(node))
						error("can't reassign reference "s + name);
					else
						trace("reassigning reference "s + name + " to " + node.value.string);
				}
				if (node.parent and (node.parent->kind == reference or node.parent->kind == key)) {
					// todo move up! todo key bad criterion!!
					// todo: add header or copy WHOLE string object!
					referenceIndices.insert_or_assign(node.parent->name, data_index_end);// safe ref to string
					referenceDataIndices.insert_or_assign(node.parent->name, data_index_end);// safe ref to string
					referenceMap.insert_or_assign(node.parent->name, node); // lookup types, array length …
				}

				// we add an extra 0, unlike normal wasm abi, because we have space in data section
				data_index_end += string.length + 1;
			}
			last_type = charp;//
			code = Code(i32_const) + Code(stringIndex);// just a pointer
			if (node.length > 0) {
				if (node.length > 1)error("only 1 op allowed");
				Node &pattern = node.first();
				if (pattern.kind != patterns and pattern.kind != longs)
					error("only patterns allowed on string");
				code.add(emitIndexPattern(Node(), pattern, context));
			}
			return code;
//			return Code(stringIndex).addInt(pString->length);// pointer + length
		}
		case key:
			if (node.value.node)
				return emitValue(*node.value.node,
				                 context);// todo: make sure it is called from right context (after isSetter …)
		case patterns:
			return emitIndexPattern(Node(), node, context);// todo: make sure to have something indexable on stack!
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
		code.add(cast(node, *node.type));
	}
	// code.push(last_typo) only on return statement
	return code;
}


[[nodiscard]]
Code emitOperator(Node node, String context) {
	Code code;
	String &name = node.name;
//	name = normOperator(name);
	if (node.length == 0 and name == "=") return code;// BUG
	int index = functionIndices.position(name);
	if (name == "then")return emitIf(*node.parent, context);// pure if handled before
	if (name == ":=")return emitDeclaration(node, node.first());
	if (name == "=")return emitSetter(node, node.first(), context);
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
		Valtype lhs_type = last_type;
		arg_type = last_type;// needs to be visible to array index [1,2,3]#1 gets FUCKED up in rhs operations!!
		const Code &rhs_code = emitExpression(rhs, context);
		Valtype rhs_type = last_type;
		Valtype commonType = needsUpgrade(lhs_type, rhs_type, name);// 3.1 + 3 => 6.1 etc
		code.push(lhs_code);// might be empty ok
		code.add(cast(lhs_type, commonType));
		code.push(rhs_code);// might be empty ok
		code.add(cast(rhs_type, commonType));
		if (commonType != void_block)
			last_type = commonType;
		else last_type = rhs_type;

	} else if (node.length > 2) {// todo: n-ary? ∑? is just a function!
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
	 * */
	if (index >= 0) {// FUNCTION CALL
		print("OPERATOR / FUNCTION CALL: %s\n"s % name);
//				for (Node arg : node) {
//					emitExpression(arg,context);
//				};
		code.addByte(function);
		code.add(index);
		return code;
	}
	if (last_type == unknown_type)
		internal_error("unknown type should be inferred by now:\n"s + node.serialize());
	byte opcode = opcodes(name, last_type, arg_type);

	if (opcode >= 0x8b and opcode <= 0x98)
		code.add(cast(last_type, f32));// float ops
	if (opcode >= 0x99 and opcode <= 0xA6)
		code.add(cast(last_type, f64)); // double ops

	if (last_type == stringp)
		code.add(emitStringOp(node, String()));
	else if (opcode == f32_sqrt) {
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
//		increased.add(node.first());
		increased.add(new Node(1));
		code.add(emitSetter(node.first(), increased, context));
	} else if (name == "#") {// index operator
		if (node.parent and node.parent->name == "=")// setter!
			return emitIndexWrite(node[0], context);// todo
		else
			return emitIndexRead(node, context);
	} else if (isFunction(name) or isFunction(normOperator(name))) {
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
		code.add(tee_local);// neeeeds result local
		code.add(0);// careful, overwrites result OR SOMETHING ELSE!
		code.add(get_local);
		code.add(0);
		code.add(opcodes("*", last_type));
	} else if (name == "**" or name == "to the" or name == "^" or name == "^^") {
//		if(last_value==0)code.addConst(1);
//		if(last_value==1)return code;
		if (last_type == int32) code.add(emitCall(*new Node("powi"), context));
		else if (last_type == f32) code.add(emitCall(*new Node("powf"), context));
		else if (last_type == f64) code.add(emitCall(*new Node("pow"), context));
		else code.add(emitCall(*new Node("powi"), context));
	} else if (name.startsWith("-")) {
		code.add(i32_sub);
	} else if (name == "return") {
		// todo multi-value
		Valtype return_type = functionSignatures[context].return_types.last();
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
			code.add(get_local);
			int local_index = 0;// last result / last_index / locals[context].size() -1
			if (locals[context].has("n"))
				local_index = locals[context].position("n");
			code.addInt(local_index);
			code.add(cast(localTypes[context][local_index], float64));// todo all casts should be auto-cast now, right?
		}
		code.add(emitCall(*new Node("pow"), context));
//		else
//			code.add(emitCall(*new Node("powi"), context));

	} else {
		error("unknown opcode / call / symbol: "s + name + " : " + index);
	}

	if (opcode == get_local and node.length == 1) {// arg AFTER op (not as const!)
		long last_local = node.first().value.longy;
		code.push(last_local);
		last_type = localTypes[context][last_local];

	}
	if (opcode >= 0x45 and opcode <= 0x78)
		last_type = i32;// int ops (also f64.eqz …)
	return code;
}

Valtype needsUpgrade(Valtype lhs, Valtype rhs, String string) {
	if (lhs == float64 or rhs == float64)
		return float64;
	if (lhs == float32 or rhs == float32)return float32;
	return none;
}

[[nodiscard]]
Code emitStringOp(Node op, String context) {
//	Code stringOp;
//	op = normOperator(op.name);
	if (op == "+") {
		op = Node("concat");//demangled on readWasm, but careful, other signatures might overwrite desired one
		last_type = stringp;
		functionSignatures["concat"].returns(charp);// hack
//		op = Node("_Z6concatPKcS0_");//concat c++ mangled export:
//		op = Node("concat_char_const*__char_const*_");// wat name if not stripped in lib release build
		return emitCall(op, context);
//		stringOp.addByte();
	} else if (op == "=" or op == "==" or op == "is" or op == "equals") {
		op = Node("eq");//  careful : various signatures
		last_type = stringp;
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
	} else
		todo("string op not implemented: "s + op.name);
	return Code();
}

// starting with 0!
//inline haha you can't inline wasm
char getChar(chars string, int nr) {
	int len = strlen0(string);
	if (nr < 1)error("#index starts with 1, use [] if you want 0 indexing");
	if (nr > len)error("index out of bounds %i>%i "s % nr % len);
	return string[nr - 1 % len];
}

//	todo : ALWAYS MAKE RESULT VARIABLE FIRST IN BLOCK (index 0 after args, used for 'it'  after while(){} etc) !!!
int last_local = 0;


bool isVariableName(String name) {
	return name[0] >= 'A';// todo
}

// also init expressions of globals!
[[nodiscard]]
Code emitExpression(Node &node, String context/*="main"*/) { // expression, node or BODY (list)
//	if(nodes==NIL)return Code();// emit nothing unless NIL is explicit! todo
	Code code;
	String &name = node.name;
//	int index = functionIndices.position(name);
//	if (index >= 0 and not locals[context].has(name))
//		error("locals should be analyzed in parser");
//		locals[name] = List<String>();
//	locals[index]= Map<int, String>();
	if (node.kind == unknown and locals[context].has(node.name))
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
		last_type = localTypes[context][last_local];
//		}
		return code;
	}
	//	or node.kind == groups ??? NO!
	if ((node.kind == call or node.kind == reference or node.kind == operators) and functionIndices.has(name))
		return emitCall(node, context);

	Node &first = node.first();
	switch (node.kind) {
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
				for (Node child: node) {
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
			if (not isVariableName(name))
				todo("proper key emission");
			// else:
		case reference: {
			if (name.empty()) {
//				error("empty reference!");
				return Code();
			}
//			Map<int, String>
			List<String> &current_local_names = locals[context];
			int local_index = current_local_names.position(name);// defined in block header
			if (name.startsWith("$")) {// wasm style $0 = first arg
				local_index = atoi0(name.substring(1));
			}
			if (local_index < 0) { // collected before, so can't be setter here
				if (functionCodes.has(name) or functionSignatures.has(name))
					return emitCall(node, context);
				else if (globals.has(name)) return emitGetGlobal(node);
				else if (name == "ℯ")
					return emitValue(Node(2.7182818284590), current);
				else if (name == "τ" or name == "tau") // if not provided as global
					return emitValue(Node(6.283185307179586), current);
				else if (name == "π" or name == "pi") // if not provided as global
					return emitValue(Node(3.141592653589793), current);
				else if (!node.isSetter()) {
					print(locals[context]);
					if (not node.type)
						error("UNKNOWN local symbol ‘"s + name + "’ in context " + context);
				} else {
					error("local symbol ‘"s + name.trim() + "’ in " + context + " should be registered in analyze()!");
					current_local_names.add(name);// ad hoc x=42
					local_index = current_local_names.size() - 1;
				}
			}
			if (node.isSetter()) { //SET
				if (node.kind == key)
					code = code + emitExpression(*node.value.node, context); // done above!
				else
					code = code + emitValue(node, context); // done above!
				if (localTypes[context][local_index] == unknown_type)
					localTypes[context][local_index] = last_type;
				else
					code.add(cast(last_type, localTypes[context][local_index]));
//				todo: convert if wrong type
				code.addByte(tee_local);// set and get/keep
				code.addByte(local_index);
				// todo KF 2022-6-5 last_type set by emitExpression (?)
//				last_type = localTypes[context][local_index];
			} else {// GET
				code.addByte(get_local);// todo: skip repeats
				code.addByte(local_index);
				last_type = localTypes[context][local_index];
			}
		}
			break;
		case patterns: // x=[];x[1]=2;x[1]==>2 but not [1,2] lists
		{
			if (not node.parent)// todo: when is op not an operator? wrong: or node.parent->kind == groups)
				return emitArray(node, context);
			else if (node.parent->kind == declaration)
				return emitIndexWrite(*node.parent, context);
			else
				return emitIndexPattern(Node(), node, context);// make sure array is on stack!
		}
//		case groups: todo: true list vs list of expression
//		case objects:
		case arrays:
		case buffers:
//for (Node child : node) {
//	const Code &expression = emitExpression(child, context);
//	code.push(expression);
//};
			return emitArray(node, context);
			break;
		default:
			error("unhandled node type «"s + node.name + "» : " + typeName(node.kind));
	}
	return code;
}

[[nodiscard]]
[[nodiscard]]
Code emitWhile(Node &node, String context) {
	Code code;
	Node condition = node[0].values();
	Node then = node[1].values();
	code.addByte(loop);
	code.addByte(none);// type:void_block todo: I thought everything is an expression!?
//	code.addByte(int32);// everything is an expression!

	code = code + emitExpression(condition, context);// condition
	code.addByte(if_i);
	code.addByte(none);// type:void_block
//	code.addByte(int32);
	code = code + emitExpression(then, context);// BODY
	code.addByte(br);
	code.addByte(1);
	code.addByte(end_block);// end if condition then action
	code.addByte(end_block);// end while loop
	// type should fall through
//	int block_value= 0;// todo : ALWAYS MAKE RESULT VARIABLE FIRST IN FUNCTION!!!
//	code.addByte(block_value);
	return code;
}

// wasm loop…br_if is like do{}while(condition), so we have to rework while(condition){}
[[nodiscard]]
Code emitWhile2(Node &node, String context) {
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
Code emitExpression(Node *nodes, String context) {
	if (!nodes)return Code();
//	if(nodes==NIL)return Code();// emit nothing unless NIL is explicit! todo
	return emitExpression(*nodes, context);
}

[[nodiscard]]
Code emitCall(Node &fun, String context) {
	Code code;
	auto name = fun.name;
	if (not functionSignatures.has(name) or not functionIndices.has(name)) {
		auto normed = normOperator(name);
		if (not functionSignatures.has(normed) or not functionIndices.has(normed))
			error("unknown function "s + name + " (" + normed + ")");// checked before, remove
		else name = normed;
	}

	Signature &signature = functionSignatures[name];
	int index = functionIndices[name];
	if (index < 0)
		error("MISSING import/declaration for function %s\n"s % name);
	int i = 0;
	// args may have already been emitted, e.g. "A"+"B" concat
	for (Node arg: fun) {
		code.push(emitExpression(arg, context));
//		Valtype argType = mapTypeToWasm(arg); // todo ((+ 1 2)) needs deep analysis, or:
		Valtype argType = last_type;// evaluated expression smarter than node arg!
		Valtype &sigType = signature.types[i++];
		if (sigType != argType)
			code.push(cast(argType, sigType));
	};
	code.addByte(function);
	code.addInt(index);// as LEB!
	code.addByte(nop);// padding for potential relocation
	signature.is_used = true;
	signature.emit = true;
	// todo multi-value
	last_type = signature.return_types.last(none);
	last_typo.clazz = &signature.return_type;// todo dodgy!
	return code;
}

[[nodiscard]]
[[nodiscard]]
Code cast(Valtype from, Valtype to) {
	last_type = to;// danger: hides last_type in caller!
	Code nop;// if two arguments are the same, commontype is 'none' and we return empty code (not even a nop, technically)
	if (to == none or to == unknown_type)return nop;// no cast needed magic VERSUS wasm drop!!!
	if (from == to)return nop;// nop

	if (from == array and to == charp)return nop;// uh, careful? [1,2,3]#2 ≠ 0x0100000…#2
	if (from == i32t and to == charp)return nop;// assume i32 is a pointer here. todo?
	if (from == charp and to == i64t) return Code(i64_extend_i32_s);
	if (from == charp and to == i32t)return nop;// assume i32 is a pointer here. todo?
	if (from == 0 and to == i32t)return nop;// nil or false ok as int? otherwise add const 0!
	if (from == float32 and to == float64)return Code(f64_from_f32);
	if (from == float32 and to == i32t) return Code(f32_cast_to_i32_s);
	if (from == i32t and to == float32)return Code(f32_from_int32);
//	if (from == i32t and to == float64)return Code(i32_cast_to_f64_s);
	if (from == i64 and to == i32) return Code(i32_wrap_i64);
	if (from == f32 and to == i32) return Code(i32_trunc_f32_s);
//	if(from==f32u and to==i32)	return Code(i32_trunc_f32_𝗎);
	if (from == f64 and to == i32) return Code(i32_trunc_f64_s);
//	if(from==f64u and to==i32)	return Code(i32_trunc_𝖿𝟨𝟦_𝗎);
	if (from == i32 and to == i64) return Code(i64_extend_i32_s);
//	if(from==i32u and to==i64)	return Code(i64_extend_i32_𝗎);
	if (from == f32 and to == i64) return Code(i64_trunc_f32_s);
//	if(from==f32u and to==i64)	return Code(i64_trunc_f32_𝗎);
	if (from == f64 and to == i64) return Code(i64_trunc_f64_s);
//	if(from==f64u and to==i64)	return Code(i64_trunc_𝖿𝟨𝟦_𝗎);
	if (from == i32 and to == f32) return Code(f32_convert_i32_s);
//	if(from==i32u and to==f32)	return Code(f32_convert_i32_𝗎);
	if (from == i64 and to == f32)
		return Code(f32_convert_i64_s);
//	if(from==f64u and to==f32)	return Code(f32_convert_i64_𝗎);
	if (from == f64 and to == f32) return Code(f32_demote_f64);
	if (from == i32 and to == f64)
		return Code(f64_convert_i32_s);
//	if(from==i32u and to==f64)	return Code(f64_convert_i32_𝗎);
	if (from == i64 and to == f64)
		return Code(f64_convert_i64_s);
//	if(from==f64u and to==f64)	return Code(f64_convert_i64_𝗎);
	if (from == f32 and to == f64) return Code(f64_promote_f32);
//	if(from==f32 and to==i32)	return Code(i32_reinterpret_f32);
//	if(from==f64 and to==i64)	return Code(i64_reinterpret_𝖿𝟨𝟦);
//	if(from==i32 and to==f32)	return Code(f32_reinterpret_i32);
//	if(from==i64 and to==f64)	return Code(f64_reinterpret_i64);
	if (from == i64 and to == f32) return Code(f64_convert_i64_s).addByte(f32_from_f64);
	if (from == void_block)return nop;// todo: pray
	if (from == unknown_type)return nop;// todo: don't pray
	if (from == array and to == i32)return nop;// pray / assume i32 is a pointer here. todo!
	if (from == array and to == i64)return nop;// pray / assume i32 is a pointer here. todo!
	if (from == i32t and to == array)return nop;// pray / assume i32 is a pointer here. todo!
	if (from == f32 and to == array)return nop;// pray / assume f32 is a pointer here. LOL NO todo!
	if (from == i64 and to == array)return Code(i32_wrap_i64);;// pray / assume i32 is a pointer here. todo!
//	if (from == void_block and to == i32)
//		return Code().addConst(-666);// dummy return value todo: only if main(), else WARN/ERROR!
	error("incompatible types "s + typeName(from) + " => " + typeName(to));
	return nop;
}

// casting in our case also means construction! (x, y) as point == point(x,y)
[[nodiscard]]
Code cast(Node &from, Node &to) {
	if (to == Long)return cast(mapTypeToWasm(from), i64);
	if (to == Double)return cast(mapTypeToWasm(from), f64);
	Node calle("call");
	calle.add(from);
	calle.add(to);
	return emitCall(calle, "");// todo context?
}

[[nodiscard]]
Code emitDeclaration(Node fun, Node &body) {
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
	Signature &signature = functionSignatures[fun.name];
	signature.emit = true;// all are 'export' for now. also set in analyze!
	functionCodes[fun.name] = emitBlock(body, fun.name);
	last_type = none;// todo reference to new symbol x = (y:=z)
	return Code();// empty
}


[[nodiscard]]
Code emitSetter(Node node, Node &value, String context) {
	if (node.first().name == "#") {// x#y=z
		return emitPatternSetter(node.first().first(), node.first().last(), node.last(), context);
	}
	if (node.name == "=") {
		if (node.length != 2)error("assignment needs 2 arguments");
		return emitSetter(node[0], node[1], context);
	}
	List<String> &current = locals[context];
	String &variable = node.name;
	if (!current.has(variable)) {
		print(current);
		current.add(variable);
		error("variable %s in context %s missed by parser! "_s % variable % context);
	}
	int local_index = current.position(variable);
	auto valtype = localTypes[context][local_index];
	last_type = mapTypeToWasm(value);
	if (valtype == unknown_type) {
		valtype = last_type;// todo : could have been done in analysis!
		localTypes[context][local_index] = last_type;// NO! the type doesn't change: example: float x=7
	}
	if (last_type == array or valtype == charp) {
		referenceIndices.insert_or_assign(variable, data_index_end);// WILL be last_data !
		referenceMap[variable] = node; // lookup types, array length …
	}
	Code setter;
//	auto values = value.values();
	if (value.hash() == node.hash())
		value = node.values();
	Code value1 = emitExpression(value, context);
	setter.add(value1);
	setter.add(cast(last_type, valtype));
	setter.add(tee_local);
	setter.add(local_index);
	last_type = valtype;// still the type of the local, not of the value. example: float x=7
	return setter;
}

bool isPrimitive(Node &node);

Code zeroConst(Valtype valtype);

//[[nodiscard]];
Code emitIf(Node &node, String context) {
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
	code.addByte(returnType);// of then!
	code = code + then_block;// BODY
	bool else_done = false;
	if (node.length == 3) {
//		Node otherwise = node["else"];//->clone();
		Node otherwise = node[2].values();
		if (otherwise.length > 0 or isPrimitive(otherwise)) {
			code.addByte(elsa);
			code = code + emitExpression(otherwise, context);
			code.add(cast(last_type, returnType));
			else_done = true;
		}
	}
	if (not else_done and returnType != none) {
//		we NEED to return something in wasm! else: "type mismatch in if false branch"
		code.addByte(elsa);
		code.add(zeroConst(returnType));
	}
	code.addByte(end_block);
	return code;
}

Code zeroConst(Valtype returnType) {
	Code code;
	if (returnType == int32)
		code.addConst(0);
	if (returnType == f32) {
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
	code.addByte(function);
	int i = functionIndices.position(symbol);
	if (i < 0)error("UNKNOWN symbol "s + symbol);
//	code.opcode(unsignedLEB128(i),8);
	code.addByte(i);
	return Code();
}

[[nodiscard]]
Code encodeString(char *str) {
	size_t len = strlen0(str);
	Code code = Code(len, (bytes) str, len);
	return code;//.push(0);
};

//bytes
[[nodiscard]]
Code signedLEB128(int n) {
	Code result;
	while (true) {
		const char byt = n & 0x7f;
		n >>= 7;
		if (
				(n == 0 && (byt & 0x40) == 0) ||
				(n == -1 && (byt & 0x40) != 0)
				) {
			result.addByte(byt);
			return result;
		}
		result.addByte(byt | 0x80);
	}
//	return result.data;
}


[[nodiscard]]
Code emitBlock(Node &node, String context) {
//	todo : ALWAYS MAKE RESULT VARIABLE FIRST IN FUNCTION!!!
//	char code_data[] = {0/*locals_count*/,i32_const,42,call,0 /*logi*/,i32_auto,21,return_block,end_block};
// 0x00 == unreachable as block header !?
//	char code_data[] = {0/*locals_count*/,i32_auto,21,return_block,end_block};// 0x00 == unreachable as block header !?
//	Code(code_data,sizeof(code_data)); // todo : memcopy, else stack value is LOST
	Code block;
//	Map<int, String>
//	collect_locals(node, context);// DONE IN analyze
//	int locals_count = current_local_names.size();
//	locals[context] = current_local_names;

//	todo : ALWAYS MAKE RESULT VARIABLE FIRST IN BLOCK (index 0, used after while(){} etc) !!!
// todo: locals must ALWAYS be collected in analyze step, emitExpression is too late!
	last_local = 0;
	last_type = none;//int32;
	int locals_count = locals[context].size();
	trace("found %d locals for %s"s % locals_count % context);
//	print(locals[context]);
	int argument_count = functionSignatures[context].size();
	if (locals_count >= argument_count)
		locals_count = locals_count - argument_count;
	else
		warn("locals consumed by arguments"); // ok in  double := it * 2; => double(it){it*2}

	// todo block.addByte(i+1) // index seems to be wrong: i==NUMBER of locals of type xyz ??
//	013b74: 01 7f                      | local[0] type=i32
//	013b76: 02 7f                      | local[1..2] type=i32
//	013b78: 03 7f                      | local[3..5] type=i32

	Code inner_code_data = emitExpression(node, context);

	// locals can still be updated in emitExpression

	// 1. Emit block type header
	block.addByte(locals_count);
	for (int i = 0; i < locals_count; i++) {
		auto name = locals[context][i];// add later to custom section, here only for debugging
		Valtype valtype = localTypes[context][i];
//		block.addByte(i + 1);// index
		block.addByte(1);// count! todo: group by type nah
		if (valtype == unknown_type)
			valtype = int32;
// todo		internal_error("unknown type should be inferred by now for local "s + name);
		if (valtype == none or valtype == voids)
			valtype = int32;
		if (valtype == charp or valtype == array)
			valtype = int32; // int64? extend to smart pointer later!
		block.addByte(valtype);
	}

	// 2. emit code
	block.push(inner_code_data);

	// 3. emit return fixtures
#if MULTI_VALUE
	// todo multi-value
	// todo: we want the type to be on the stack IN FRONT of the value, so functions are backwards compatible and only depend on the func_type signature
	// todo: we need to conserve the last value in case of casting
//if(last_object)
//	block.addConst32(mapTypeToWasm(*last_object));
//else
	block.addConst32(last_typo.value);
#endif

	auto returnTypes = functionSignatures[context].return_types;
	//	for(Valtype return_type: returnTypes) uh, casting should have happened before for  multi-value
	Valtype return_type = returnTypes.last(voids);
	// switch back to return_types[context] for block?
	bool needs_cast = return_type != last_type;

	auto abi = wasp_smart_pointers;// functionSignatures[context].abi;

	if (abi == wasp_smart_pointers) {
//		if(last_type==charp)block.push(0xC0000000, false,true).addByte(i32_or);// string
//		if(last_type==charp)block.addConst(-1073741824).addByte(i32_or);// string
		if (last_typo.type == int_array or last_type == array) {
			block.addConst64(array_header_64).addByte(i64_or); // todo: other arrays
//			if (return_type==float64)
//			block.addByte(f64_reinterpret_i64);// hack smart pointers as main return: f64 has int range which is never hit
			last_type = int64;
		}
//		block.addConst32(array_header_32).addByte(i32_or); // todo: other arrays
		else if (last_typo.kind == strings or last_typo.type == c_string) { // last_type == charp
			block.addByte(i64_extend_i32_u);
			block.addConst64(string_header_64);
			block.addByte(i64_or);
			last_type = int64;
			needs_cast = return_type == i64;
		} else if (last_typo.kind == reference) {
//			if (last_type==charp)
//				block.addConst(string_header_64).addByte(i64_or);
//			todo("last_typo.type");
		} else if (last_type == float64 and context == start) {
//			block.addByte(i64_trunc_f64_s);
			block.addByte(i64_reinterpret_f64);// hack smart pointers as main return: f64 has int range which is never hit
			last_type = int64;
		}
//		if(last_type==charp)block.addConst32((unsigned int)0xC0000000).addByte(i32_or);// string
//		if(last_type==angle)block.addByte(i32_or).addInt(0xA000000);//
//		if(last_type==pointer)block.addByte(i32_or).addInt(0xF000000);//
	}
	if (last_type == none) last_type = voids;
	if (needs_cast) {
		if (return_type == Valtype::voids and last_type != Valtype::voids)
			block.addByte(drop);
		else if (return_type == Valtype::i32t and last_type == Valtype::voids)
			block.addConst(0);
		else if (return_type == Valtype::i64t and last_type == Valtype::voids)
			block.addConst64(0);
		else
			block.add(cast(last_type, return_type));
	}

//	if(context=="main" or (functionSignatures[context].abi==wasp and returnTypes.size()<2))
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
List<String> collect_locals(Node node, String context) {
	List<String> &current_locals = locals[context]; // no, because there might be gaps(unnamed locals!)
	for (Node n : node) {
		Valtype type = mapTypeToWasm(n);
		if (type != none and not current_locals.has(n.name)) {
			current_locals.add(n.name);
			localTypes[context].add(type);
		}
	}
	return current_locals;
}
*/


int last_index = -1;

[[nodiscard]]
Code typeSection() {
	// Function types are vectors of parameters and return types. Currently
	// TODO optimise - some of the procs might have the same type signature
	// the type section is a vector of function types
	int typeCount = 0;
	Code type_data;
//	print(functionIndices);
	for (String fun: functionSignatures) {
		if (!fun) {
//			print(functionIndices);
//			print(functionSignatures);
			breakpoint_helper
			warn("empty functionSignatures[ø] because context=start=''");
//			error("empty function creep functionSignatures[ø]");
			continue;
		}
		Signature &signature = functionSignatures[fun];
		if (not signature.emit /*export/declarations*/ and not signature.is_used /*imports*/) {
			trace("not signature.emit => skipping unused type for %s");
			trace(fun);
			continue;
		}
		if (signature.is_runtime)
			continue;
		if (signature.is_handled)
			continue;
//		if(signature.is_import) // types in import section!
//			continue;
		if (not functionIndices.has(fun))
			functionIndices[fun] = ++last_index;
//			error("function %s should be registered in functionIndices by now"s % fun);

		typeMap[fun] = runtime.type_count /* lib offset */ + typeCount++;
		signature.is_handled = true;
		int param_count = signature.size();
//		Code td = {0x60 /*const type form*/, param_count};
		Code td = Code(func) + Code(param_count);

		for (int i = 0; i < param_count; ++i) {
			td = td + Code(fixValtype(signature.types[i]));
		}
//		Valtype &ret = functionSignatures[fun].return_type;
		td.addByte(signature.return_types.size());
		for (Valtype ret: signature.return_types) {
			Valtype valtype = fixValtype(ret);
			td.addByte(valtype);
		}
		type_data = type_data + td;
	}
	return Code((char) type_section, encodeVector(Code(typeCount) + type_data)).clone();
}

Valtype fixValtype(Valtype &valtype) {
	if (valtype == charp) return int32;
	if (valtype > 0xC0)error("exposed internal Valtype");
	return valtype;
}

[[nodiscard]]
Code importSection() {
	if (runtime_offset) {
//		breakpoint_helper
//		printf("imports currently not supported\n");
		import_count = 0;
		return Code();
	}
	// the import section is a vector of imported functions
	Code imports;
	import_count = 0;
	for (String fun: functionSignatures) {
		Signature &signature = functionSignatures[fun];
		if (signature.is_import and signature.is_used and not signature.is_builtin) {
			++import_count;
			imports = imports + encodeString("env") + encodeString(fun).addByte(func_export).addType(typeMap[fun]);
		}
	}
	int extra_mem = 0;
	if (memoryHandling == import_memory) {
		extra_mem = 1;// add to import_section but not to functions:import_count
		int init_page_count = 1024; // 64kb each, 65336 pages max
		imports = imports + encodeString("env") + encodeString("memory") + (byte) mem_export/*type*/+ (byte) 0x00 +
		          Code(init_page_count);
	}
	if (imports.length == 0)return Code();
	auto importSection = createSection(import_section, Code(import_count + extra_mem) + imports);// + sqrt_ii
	return importSection.clone();
}

//int function_count;// misleading name: declared functions minus import_count  (together : functionIndices.size() )
//int new_count;
int function_block_count;

//int builtins_used=0;
[[nodiscard]]
Code codeSection(Node root) {
	// the code section contains vectors of functions
	// index needs to be known before emitting code, so call $i works
	int new_count;

	if (root.kind == objects)
		root.kind = expression;// todo why hack?

	new_count = declaredFunctions.size();
	for (auto declared: declaredFunctions) {
		print("declared function: "s + declared);
		if (!declared)error("empty function name (how?)");
		if (not functionIndices.has(declared))// used or not!
			functionIndices[declared] = ++last_index;// functionIndices.size();
	}
//	int index_size = functionIndices.size();
//	bool has_main = start and (declaredFunctions.has(start) or functionIndices.has(start));
//	if (import_count + builtin_count + has_main + new_count + runtime_offset != index_size) {
//		print(functionIndices);
//		String message = "inconsistent function_count %d import + %d builtin + %d new + %d runtime + %d main != %d"s;
// 		error(message % import_count % builtin_count % new_count % runtime_offset % has_main % index_size);
//	}
// https://pengowray.github.io/wasm-ops/
//	char code_data[] = {0x01,0x05,0x00,0x41,0x2A,0x0F,0x0B};// 0x41==i32_auto  0x2A==42 0x0F==return 0x0B=='end (function block)' opcode @+39
	byte code_data_fourty2[] = {0/*locals_count*/, i32_auto, 42, return_block, end_block};
	byte code_data_nop[] = {0/*locals_count*/, end_block};// NOP
	byte code_data_id[] = {1/*locals_count*/, 1/*one local has type: */, i32t, get_local, 0, return_block,
	                       end_block}; // NOP
	byte code_square_d[] = {1/*locals_count*/, 1/*one local has type: */, f64t, get_local, 0, get_local, 0, f64_mul, return_block, end_block};

	byte code_modulo_float[] = {1 /*locals declarations*/, 2 /*two of type*/, float32,
	                            0x20, 0x00, 0x20, 0x00, 0x20, 0x01, 0x95, 0x8f, 0x20, 0x01, 0x94, 0x93, 0x0b};
	byte code_modulo_double[] = {1 /*locals variables*/, 2 /*two of type*/, float64,
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
		if (functionSignatures["nop"].is_used)
			code_blocks = code_blocks + encodeVector(Code(code_data_nop, sizeof(code_data_nop)));
		if (functionSignatures["square_double"].is_used and functionSignatures["square_double"].is_builtin)// can also be linked via runtime/import!
			code_blocks = code_blocks + encodeVector(Code(code_square_d, sizeof(code_square_d)));
		if (functionSignatures["id"].is_used)
			code_blocks = code_blocks + encodeVector(Code(code_data_id, sizeof(code_data_id)));
		if (functionSignatures["modulo_float"].is_used)
			code_blocks = code_blocks + encodeVector(Code(code_modulo_float, sizeof(code_modulo_float)));
		if (functionSignatures["modulo_double"].is_used) {
			code_blocks = code_blocks + encodeVector(Code(code_modulo_double, sizeof(code_modulo_double)));
		}
	}

	Code main_block = emitBlock(root, start);// after imports and builtins

	if (start) {
		if (main_block.length == 0)
			functionSignatures[start].is_used = false;
		else
			code_blocks = code_blocks + encodeVector(main_block);
	} else {
		if (main_block.length > 5)
			error("no start function name given. null instead of 'main', can't assign block");
		else warn("no start block (ok)");
	}
	for (String fun: functionCodes) {// MAIN block extra ^^^
		Code &func = functionCodes[fun];
		code_blocks = code_blocks + encodeVector(func);
	}
	builtin_count = 0;
	for (auto name: functionSignatures) {
		Signature &signature = functionSignatures[name];
		if (signature.is_builtin and signature.is_used) builtin_count++;
	}

	bool has_main = start and functionIndices.has(start);
	int function_codes = functionCodes.size();
	function_block_count = has_main /*main*/ + builtin_count + function_codes;
	auto codeSection = createSection(code_section, Code(function_block_count) + code_blocks);
	return codeSection.clone();
}

[[nodiscard]]
[[nodiscard]]
Code exportSection() {
	short exports_count = 1;// main
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
	Code globalExports;
	for (int i = 0; i < globals.size(); i++) {
		String &name = globals.keys[i];
		Code globalExport = encodeString(name) + (byte) global_export + Code(i);
		globalExports.add(globalExport); // todo << NOW
		exports_count++;
	}

	Code exportsData = encodeVector(
			Code(exports_count) + encodeString(start) + (byte) func_export + Code(main_offset) + memoryExport +
			globalExports);

	auto exportSection = createSection(export_section, exportsData);
	return exportSection;
}

int global_import_count = 0;
int global_user_count = 0;

[[nodiscard]]
Code globalSection() {
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
		const Code &globalInit = emitExpression(global_node, "global");
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
Code dataSection() { // needs memory section too!
//	Contents of section Data:
//	0000042: 0100 4100 0b02 4869                      ..A...Hi
//https://webassembly.github.io/spec/core/syntax/modules.html#syntax-datamode
	Code datas;
	if (data_index_end == 0)return datas;//empty

	//Contents of section Data:
//0013b83: 0005 00 41 8108 0b fd1d 63 6f72 7275 7074  ...A.....corrupt
//	datas.addByte(00); WHY does module have extra 0x00 and 5 data sections???
//	datas.addByte(00); // it seems heading 0's get jumped over until #segments >0 :
	datas.addByte(01);// one memory initialization / data segment
	datas.addByte(00);// memory id always 0

	datas.addByte(0x41);// opcode for i32.const offset: followed by unsignedLEB128 value:
	datas.addInt(
			runtime.data_offset_end);// actual offset in memory todo: WHY cant it start at 0? wx  todo: module offset + module data length
	datas.addByte(0x0b);// mode: active?
	datas.addByte(data_index_end); // size of data
	const Code &actual_data = Code((bytes) data, data_index_end);
	datas.add(actual_data);// now comes the actual data  encodeVector()? nah manual here!
	return createSection(data_section, encodeVector(datas));// size added via actual_data
}

// Signatures
[[nodiscard]]
Code funcTypeSection() {// depends on codeSection, but must appear earlier in wasm
	// funcType_count = function_count EXCLUDING imports, they encode their type inline!
	// the function section is a vector of type indices that indicate the type of each function in the code section

	Code types_of_functions = Code(function_block_count);//  = Code(types_data, sizeof(types_data));
//	order matters in functionType section! must be same as in functionIndices
	for (int i = 0; i < function_block_count; ++i) {
		//	import section types separate WTF wasm
		int index = i + import_count + runtime_offset;
		String *fun = functionIndices.lookup(index);
		if (!fun) {
			print(functionIndices);
			error("missing typeMap for index "s + index);
		} else {
			int typeIndex = typeMap[*fun];
			if (typeIndex < 0) {
				if (runtime_offset == 0) // todo else ASSUME all handled correctly before
					error("missing typeMap for function %s index %d "s % fun % i);
			} else // just an implicit list funcId->typeIndex
				types_of_functions.push((byte) typeIndex);
		}
	}
	// @ WASM : WHY DIDN'T YOU JUST ADD THIS AS A FIELD IN THE FUNC STRUCT???
	Code funcSection = createSection(functypes_section, types_of_functions);
	return funcSection.clone();
}

[[nodiscard]]
Code functionSection() {
	return funcTypeSection();// (misnomer) vs codeSection() !
}

// todo : convert library referenceIndices to named imports!
[[nodiscard]]
Code nameSection() {
	Code nameMap;

	int total_func_count = last_index + 1;// functionIndices.size();// imports + function_count, all receive names
	int usedNames = 0;
	for (int index = runtime_offset; index < total_func_count; index++) {
		// danger: utf names are NOT translated to wat env.√=√ =>  (import "env" "\e2\88\9a" (func $___ (type 3)))
		String *name = functionIndices.lookup(index);
		if (not name)continue;// todo: no name bug (not enough mem?)
		if (functionSignatures[*name].is_import and runtime_offset > 0)continue;
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
	for (int index = runtime_offset; index <= last_index; index++) {
		String *key = functionIndices.lookup(index);
		if (!key or key->empty())continue;
		List<String> localNames = locals[*key];// including arguments
		int local_count = localNames.size();
		if (local_count == 0)continue;
		usedLocals++;
		localNameMap = localNameMap + Code(index) + Code(local_count); /*???*/
		for (int i = 0; i < localNames.size(); ++i) {
			String local_name = localNames[i];
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
//	Code exampleNames= Code((byte) 5) /*function index*/ + Code((byte) 1) /*count*/ + Code((byte) 0) /*l.nr*/ + Code("var1");
	// function 5 with one local : var1

//	localNameMap = localNameMap + exampleNames;

	auto moduleName = Code(module_name) + encodeVector(Code("wasp_module"));
	auto functionNames = Code(function_names) + encodeVector(Code(usedNames) + nameMap);
	auto localNames = Code(local_names) + encodeVector(Code(usedLocals) + localNameMap);
	auto globalNames = Code(global_names) + encodeVector(Code(usedGlobals) + globalNameMap);

//	The name section is a custom section whose name string is itself ‘𝚗𝚊𝚖𝚎’.
//	The name section should appear only once in a module, and only after the data section.
	const Code &nameSectionData = encodeVector(Code("name") + moduleName + functionNames + localNames + globalNames);
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
 *  There are currently two ways in which function indices are stored in the code section:
    Immediate argument of the call instruction (calling a function)
    Immediate argument of the i32.const instruction (taking the address of a function).
    The immediate argument of all such instructions are stored as padded LEB128 such that they can be rewritten
    without altering the size of the code section. !
    For each such instruction a R_WASM_FUNCTION_INDEX_LEB or R_WASM_TABLE_INDEX_SLEB reloc entry is generated
    pointing to the offset of the immediate within the code section.

    R_WASM_FUNCTION_INDEX_LEB relocations may fail to be processed, in which case linking fails.
    This occurs if there is a weakly-undefined function symbol, in which case there is no legal value that can be
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
	for (auto sig: functionSignatures) {// imports first
		Signature &signature = functionSignatures[sig];
		if (signature.is_import and signature.is_used) {
			functionIndices[sig] = ++last_index;// functionIndices.size();
			import_count++;
		}
	}
	for (auto sig: functionSignatures) {// now builtins
		if (functionSignatures[sig].is_builtin and functionSignatures[sig].is_used) {
			functionIndices[sig] = ++last_index;// functionIndices.size();
			builtin_count++;
		}
	}
}

[[nodiscard]]
Code memorySection() {
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
	functionCodes.clear();
	functionIndices.setDefault(-1);
	functionCodes.setDefault(Code());
	typeMap.setDefault(-1);
	typeMap.clear();
	referenceMap.setDefault(Node());
	data_index_end = 0;
	last_data = 0;
	data = (char *) malloc(MAX_DATA_LENGTH);
//	while (((long)data)%16)data++;// type 'long', which requires 4 byte alignment
}

[[nodiscard]]
Code &emit(Node &root_ast, Module *runtime0, String _start) {
	start = _start;
	clearEmitterContext();
	if (runtime0) {
		memoryHandling = no_memory;// done by runtime?
		runtime = *runtime0;// else filled with 0's
		runtime_offset = runtime.import_count + runtime.code_count;//  functionIndices.size();
		import_count = 0;
		builtin_count = 0;
		//		data_index_end = runtime.data_offset_end;// insert after module data!
		// todo: either write data_index_end DIRECTLY after module data and increase count of module data,
		// or increase memory offset for second data section! (AND increase index nontheless?)
		int newly_pre_registered = 0;//declaredFunctions.size();
		last_index = runtime_offset - 1;
	} else {
		memoryHandling = export_memory;
//#ifdef IMPORT_MEMORY
//		memoryHandling = import_memory; // works for micro-runtime
//#endif
//		memoryHandling = internal_memory; // works for wasm3
		last_index = -1;
		runtime = *new Module();// all zero
		runtime_offset = 0;
		typeMap.clear();
		functionIndices.clear();// ok preregistered functions are in functionSignatures
		add_builtins();
	}
	if (start) {// now AFTER imports and builtins
//		printf("start: %s\n", start.data);
//		functionSignatures[start] = Signature().returns(i32t);
		functionSignatures[start].emit = true;
		if (!functionIndices.has(start))
			functionIndices[start] = ++last_index;
//			functionIndices[start] =runtime_offset ? runtime_offset + declaredFunctions.size() :  ++last_index;// functionIndices.size();  // AFTER collecting imports!!
		else
			error("start already declared: "s + start + " with index " + functionIndices[start]);
	} else {
//		functionSignatures["_default_context_"] = Signature();
//		start = "_default_context_";//_default_context_
//		start = "";
	}

	const Code customSectionvector;
//	const Code &customSectionvector = encodeVector(Code("custom123") + Code("random custom section data"));
	// ^^^ currently causes malloc_error WHY??

	auto customSection = createSection(custom_section, customSectionvector);
	Code typeSection1 = typeSection();// types must be defined in analyze(), not in code declaration
	Code importSection1 = importSection();// needs type indices
	Code globalSection1 = globalSection();//
	Code codeSection1 = codeSection(root_ast); // needs functionSignatures and functionIndices prefilled!! :(
	Code funcTypeSection1 = funcTypeSection();// signatures depends on codeSection, but must come before it in wasm
	Code memorySection1 = memorySection();
	Code exportSection1 = exportSection();// depends on codeSection, but must come before it!!

	Code code = Code(magicModuleHeader, 4)
	            + Code(moduleVersion, 4)
	            + typeSection1
	            + importSection1
	            + funcTypeSection1 // signatures
	            + memorySection1 // Wasm MVP can only define one memory per module WHERE?
	            + globalSection1
	            + exportSection1
	            + codeSection1 // depends on importSection, yields data for funcTypeSection!
	            + dataSection()
	            //			+ linkingSection()
	            + nameSection()
//	 + dwarfSection() // https://yurydelendik.github.io/webassembly-dwarf/
//	 + customSection
	;
	code.debug();
#ifndef WEBAPP
//	free(data);// written to wasm code ok
#endif
	if (runtime0)functionSignatures.clear(); // cleanup after NAJA
	return code.clone();
}


// todo dedup runtime_emit!
//Node emit(String code, ParseOptions options) {
Code emit(String code) {// emit and run!
//	if (code.endsWith(".wasm")){
//		auto filename = findFile(code);
//		return Node(run_wasm(filename));
//	}
	Node data = parse(code);
#ifdef RUNTIME_ONLY
	warn("RUNTIME_ONLY cannot emit code")
	return Code();
#else
	data.print();
	clearAnalyzerContext();
	Node &charged = analyze(data);
	Code binary = emit(charged);// options & no_main ? 0 , 0
#ifndef INCLUDE_MERGER
	Code out = binary;
	if (merge_module_binaries.size() > 0)
		warn("wasp compiled without binary linking/merging. set(INCLUDE_MERGER 1) in CMakeList.txt");
//	return ERROR;
#else
	Code out = merge_binaries(merge_module_binaries);
	binary.save("raw.wasm");
	merge_module_binaries.add(binary);
	out.save();
#endif
	return out;
#endif
}
