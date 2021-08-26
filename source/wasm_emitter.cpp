//https://blog.sentry.io/2020/08/11/the-pain-of-debugging-webassembly
//https://github.com/mdn/webassembly-examples/tree/master/understanding-text-format
// https://github.com/ColinEberhardt/chasm/blob/master/src/emitter.ts
// https://github.com/ColinEberhardt/chasm/blob/master/src/encoding.ts
// https://pengowray.github.io/wasm-ops/
#include "Wasp.h"
#include "String.h"
#include "Map.h"
#include "Code.h"
#include "wasm_emitter.h"
#include "wasm_helpers.h"
//#include "wasm_runner.h"
#include "wasm_reader.h"

#define check(test) if(test){log("OK check passes: ");log(#test);}else{printf("\nNOT PASSING %s\n%s:%d\n",#test,__FILE__,__LINE__);exit(0);}

int runtime_offset = 0; // imports + funcs
int import_count = 0;
short builtin_count = 0;// function_offset - import_count - runtime_offset;

Module runtime;
String start = "main";

Valtype last_type = voids;// autocast if not int
//Map<String, Valtype> return_types;
//Map<int, List<String>> locals;

//Map<int, Map<int, String>> locals;
//List<String> declaredFunctions; only new functions that will get a Code block, no runtime/imports
Map<String, int> functionIndices;
Map<String, Code> functionCodes;
Map<String, int> typeMap;


Code Call(char *symbol);//Node* args

//Code& unsignedLEB128(int);
//Code& flatten(byter);
//Code& flatten (Code& data);
void todo(char *message = "") {
	printf("TODO %s\n", message);
}

//typedef int number;
//typedef char byte;
//typedef char* Code;
//typedef char id;




//class Bytes {
//public:
//	int length;
//};

//bytes
Code signedLEB128(int i);

Code encodeString(char *String);

//typedef Code any;
//typedef Bytes any;

// class Code;

// Code flatten (any arr[]) {
// [].concat.apply([], arr);



// https://pengowray.github.io/wasm-ops/
byte opcodes(chars s, byte kind = 0) {
//	if(eq(s,"$1="))return set_local;
//	if (eq(s, "=$1"))return get_local;
//	if (eq(s, "=$1"))return tee_local;

	if (kind == 0 or kind == i32t) { // INT32
		if (eq(s, "+"))return i32_add;
		if (eq(s, "-"))return i32_sub;
		if (eq(s, "*"))return i32_mul;
		if (eq(s, "/"))return i32_div;
		if (eq(s, "%"))return i32_rem;
		if (eq(s, "=="))return i32_eq;
		if (eq(s, "!="))return i32_ne;
		if (eq(s, ">"))return i32_gt;
		if (eq(s, "<"))return i32_lt;
		if (eq(s, ">="))return i32_ge;
		if (eq(s, "<="))return i32_le;
		if (eq(s, "≥"))return i32_ge;
		if (eq(s, "≤"))return i32_le;

		if (eq(s, "&"))return i32_and;
		if (eq(s, "&&"))return i32_and;
		if (eq(s, "and"))return i32_and;
		if (eq(s, "or"))return i32_or;
		if (eq(s, "xor"))return i32_xor;
		if (eq(s, "not"))return i32_eqz;
		if (eq(s, "||"))return i32_or;
		if (eq(s, "|"))return i32_or;
	} else {

		if (eq(s, "not"))return f32_eqz; // HACK: no such thing!
		if (eq(s, "+"))return f32_add;
		if (eq(s, "-"))return f32_sub;
		if (eq(s, "*"))return f32_mul;
		if (eq(s, "/"))return f32_div;
		if (eq(s, "=="))return f32_eq;
		if (eq(s, ">"))return f32_gt;
		if (eq(s, ">="))return f32_ge;
		if (eq(s, "<"))return f32_lt;
		if (eq(s, "<="))return f32_le;
	}
	if (eq(s, "√"))return f32_sqrt; // forces i32->f32

	// todo : peek 65536 as float directly via opcode
	if (eq(s, "peek"))return i64_load;  // memory.peek memory.get memory.read
	if (eq(s, "poke"))return i64_store; // memory.poke memory.set memory.write

	// todo : set_local,  global_get ...
	if (eq(s, "$"))return get_local; // $0 $1 ...

	printf("unknown operator %s\n", s);
	breakpoint_helper
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
//Code emitExpression (Node* nodes);

Code emitBlock(Node node, String functionContext);

//Code emitExpression(Node *node)__attribute__((warn_unused_result));
//Code emitExpression(Node *node)__attribute__((error_unused_result));


//Map<int, String>
List<String> collect_locals(Node node, String context);


Code emitValue(Node node, String context) {
	Code code;
	switch (node.kind) {
		case nils:// also 0, false
//			code.opcode((byte)i64_auto);// nil is pointer
			code.addByte((byte) i32_auto);// nil is pointer
			code.push((long) 0);
			break;
		case bools:
//		case ints:
			code.addByte((byte) i32_auto);
			code.push(node.value.longy);// LEB encoded!
			last_type = i32t;
//				code.opcode(ieee754(node.value.longy),4);
			break;
		case longs:
			// todo: ints vs longs!!!
			last_type = i32t;
//			if(call_extern)
			code.addByte((byte) i32_const);
//			code.opcode((byte)i64_auto);
			code.push(node.value.longy);
//				code.opcode(ieee754(node.value.longy),4);
			break;
		case reals:
			last_type = f32t;// auto cast return!
			code.addByte((byte) f32_auto);
			code.push(ieee754(node.value.real), 4);
			break;
//		case identifier:
		case reference: {
			int local_index = locals[context].position(node.name);
			if (local_index < 0)error("UNKNOWN symbol "s + node.name + " in context " + context);
			code.addByte(get_local);
			code.addByte(local_index);
		}
			break;
//			case binaryExpression:
//				code.opcode(binaryOpcode[node.value]);
//				break;
		case operators:
			warn("operators should never be emitted as values");
			return emitExpression(node, context);
			break;
		default:
			error("emitValue unknown type: "s + typeName(node.kind));
	}
	return code;
}

Code emitOperator(Node node, String context) {
	Code code;
	String &name = node.name;
	int index = functionIndices.position(name);
	if (name == "then")return emitIf(*node.parent, context);// pure if handled before
	if (name == ":=") // todo or ... !
		return emitDeclaration(node, node.first());
	if (name == "=") // todo or ... !
		return emitSetter(node, node.first(), context);
	if (node.length < 1 and not node.value.node and not node.next) {
		node.log();
		error("missing args for operator "s + name);
	} else if (node.length == 1) {
//				if(name.in(function_list)) SHOULDN'T HAPPEN!
//				error("unexpected unary operator: "s + name);
		Node arg = node.children[0];
		const Code &arg_code = emitExpression(arg, context);// should ALWAYS just be value, right?
		code.push(arg_code);// might be empty ok
	} else if (node.length == 2) {
		Node lhs = node.children[0];//["lhs"];
		Node rhs = node.children[1];//["rhs"];
		const Code &lhs_code = emitExpression(lhs, context);
		const Code &rhs_code = emitExpression(rhs, context);
		code.push(lhs_code);// might be empty ok
		code.push(rhs_code);// might be empty ok
	} else if (node.length > 2) {// todo: n-ary? ∑? is just a function!
		error("Too many args for operator "s + name);
	} else if (node.next) { // todo really? handle ungrouped HERE? just hiding bugs?
		const Code &arg_code = emitExpression(*node.next, context);
		code.push(arg_code);// might be empty ok
	} else if (node.value.node) {
		const Code &arg_code = emitExpression(*node.value.node, context);
		code.push(arg_code);
	}
	if (index >= 0) {// FUNCTION CALL
		log("OPERATOR FUNCTION CALL: %s\n"s % name);
//				for (Node arg : node) {
//					emitExpression(arg,context);
//				};
		code.addByte(function);
		code.addByte((index));// ok till index>127?
		return code;
	}
	byte opcode = opcodes(name, last_type);
	if (opcode == f32_sqrt and last_type == i32t) {
		code.addByte(f32_convert_i32_s);// i32->f32
		code.addByte(f32_sqrt);
		code.addByte(i32_trunc_f32_s);// f32->i32  i32_trunc_f32_s would also work, but reinterpret is cheaper
//				last_type = f32t; todo: try upgrade type 2 + √2 -> float
	} else if (opcode == f32_eqz) { // hack for missing f32_eqz
//				0.0 + code.addByte(f32_eq);
		code.addByte(i32_reinterpret_f32);// f32->i32  i32_trunc_f32_s would also work, but reinterpret is cheaper
		code.addByte(i32_eqz);
		last_type = i32t;// bool'ish
	} else if (name == "++") {
		Node increased = Node("+").setType(operators);
		increased.add(node.first());
		increased.add(new Node(1));
//		emitExpression(increased,context);
		code.add(emitSetter(node.first(), increased, context));
	} else if (opcode > 0)
		code.addByte(opcode);
	else {
		error("unknown opcode / call / symbol: "s + name);
	}
	if (opcode == i32_add or opcode == i32_modulo or opcode == i32_sub or opcode == i32_div or
	    opcode == i32_mul)
		last_type = i32t;
	if (opcode == f32_eq or opcode == f32_gt or opcode == f32_lt)
		last_type = i32t;// bool'ish
	return code;
}

Code emitExpression(Node &node, String context/*="main"*/) { // expression, node or BODY (list)
//	if(nodes==NIL)return Code();// emit nothing unless NIL is explicit! todo
	Code code;
	String &name = node.name;
	int index = functionIndices.position(name);
//	if (index >= 0 and not locals[context].has(name))
//		error("locals should be analyzed in parser");
//		locals[name] = List<String>();
//	locals[index]= Map<int, String>();

	if (name == "if")
		return emitIf(node, context);
	if (name == "while")
		return emitWhile(node, context);
	if (name == "it") {
		code.addByte(get_local);
		code.addByte(0);// todo: LAST local?
		return code;
	}
	//	or node.kind == groups ??? NO!
	if ((node.kind == call or node.kind == reference) and functionIndices.has(name))
		return emitCall(node, context);

	switch (node.kind) {
		case expressions:
		case groups:
		case objects:
			for (Node child : node) {
				const Code &expression = emitExpression(child, context);
				code.push(expression);
			};
			break;
		case call:
			return emitCall(node, context);
			break;
		case operators:
			return emitOperator(node, context);
		case declaration:
			return emitDeclaration(node, node.first());
		case assignment:
			return emitSetter(node, node.first(), context);
		case nils:
		case longs:
		case reals:
		case bools:
		case strings:
			if (not node.isSetter() || node.value.longy == 0) // todo 0
				return emitValue(node, context);
//			else FALLTHROUGH!
		case reference: {
//			Map<int, String>
			List<String> &current_local_names = locals[context];
			int local_index = current_local_names.position(name);// defined in block header
			if (name.startsWith("$")) {
				local_index = atoi0(name.substring(1));
			}
			if (local_index < 0) { // collected before, so can't be setter here
				if (functionCodes.has(name))
					return emitCall(node, context);
				if (!node.isSetter())
					error("UNKNOWN local symbol "s + name + " in context " + context);
				else {
					current_local_names.add(name);// ad hoc x=42
					local_index = current_local_names.size() - 1;
				}
			}
			if (node.isSetter()) { //SET
				code = code + emitValue(node, context); // done above!
				code.addByte(set_local);
				code.addByte(local_index);
			} else {// GET
				code.addByte(get_local);
				code.addByte(local_index);
			}
		}
			break;
		default:
			error("unhandled node type: "s + typeName(node.kind));
	}
	return code;
}


Code emitWhile(Node &node, String context) {
	Code code;
	Node condition = node[0];
	Node then = node[1];
	code.addByte(loop);
	code.addByte(none);// type:void_block
//	code.addByte(int32);// type OR typeidx!?
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
Code emitWhile2(Node &node, String context) {
	Code code;
	// outer block
//	code.addByte(block);
//	code.addByte(void_block);
	// inner loop
	code.addByte(loop);
	code.addByte(none);// void_block
	// compute the while expression
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
	code.addByte(block_value);
//	code.addByte(end_block); // end block
	return code;
}


Code emitExpression(Node *nodes, String context) {
	if (!nodes)return Code();
//	if(nodes==NIL)return Code();// emit nothing unless NIL is explicit! todo
	return emitExpression(*nodes, context);
}

Code emitCall(Node &fun, String context) {
	Code code;
	int index = functionIndices[fun.name];
	if (index < 0)
		error("MISSING import/declaration for function %s\n"s % fun.name);
	for (Node arg : fun) {
		code.push(emitExpression(arg, context));
	};
	code.addByte(function);
	code.addInt(index);
	code.addByte(nop);// padding for potential relocation
	Signature &signature = functionSignatures[fun.name];
	signature.is_used = true;
	signature.emit = true;
	last_type = signature.return_type;
	return code;
}

Code emitDeclaration(Node fun, Node &body) {
	// todo: x := 7 vs x := y*y
	//
	if (not functionIndices.has(fun.name)) {
		error("Declarations need to be registered before in the parser so they can be called from main code!");
//		functionIndices[fun.name] = functionIndices.size();
	}
//	else {
//		error("redeclaration of symbol: "s + fun.name);
//	}
	Signature &signature = functionSignatures[fun.name];
	signature.emit = true;// all are 'export' for now. also set in analyze!
	functionCodes[fun.name] = emitBlock(body, fun.name);
//	last_type = voids;// todo reference to new symbol x = (y:=z)
	return Code();// empty
}


Code emitSetter(Node node, Node &value, String context) {
	List<String> &current = locals[context];
	String &variable = node.name;
	if (!current.has(variable)) {
		current.add(variable);
		error("variable missed by parser! "_s + variable);
	}
	int local_index = current.position(variable);
	Code setter;
	Code value1 = emitValue(value, context);
	last_type = mapTypeToWasm(value);
	localTypes[context][local_index] = last_type;
//	variableTypes
	setter.add(value1);
	setter.add(set_local);
	setter.add(local_index);
	setter.add(get_local);// make value available
	setter.add(local_index);
	return setter;
}


Code emitIf(Node &node, String context) {
	Code code;
	Node *condition = node[0].value.node;
//	Node &condition = node["condition"];
	code = code + emitExpression(condition, context);

	code.addByte(if_i);
	code.addByte(int32);
	Node *then = node[1].value.node;
//	Node &then = node["then"];
	code = code + emitExpression(then, context);// BODY
	if (node.length == 3) {
		code.addByte(elsa);
		Node *otherwise = node[2].value.node;//->clone();
		code = code + emitExpression(otherwise, context);
	}
	code.addByte(end_block);
	if (last_type != int32) {
		last_type = int32;
		printf("todo\n");
	}
	return code;
}

/*
Code emitIf_OLD(Node &node) {
	Code code;
//	case ifStatement:
	// if block
	code.addByte(block);
	code.addByte(void_block);
	// compute the if expression
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
//			// compute the if expression (elsif) elif orif
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

Code Call(char *symbol) {//},Node* args=0) {
	Code code;
	code.addByte(function);
	int i = functionIndices.position(symbol);
	if (i < 0)error("UNKNOWN symbol "s + symbol);
//	code.opcode(unsignedLEB128(i),8);
	code.addByte(i);
	return Code();
}

Code encodeString(char *str) {
	size_t len = strlen0(str);
	Code code = Code(len, (bytes) str, len);
	return code;//.push(0);
};

//bytes
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


Code emitBlock(Node node, String context) {
//	todo : ALWAYS MAKE RESULT VARIABLE FIRST IN FUNCTION!!!
//	char code_data[] = {0/*locals_count*/,i32_const,42,call,0 /*logi*/,i32_auto,21,return_block,end_block};// 0x00 == unreachable as block header !?
//	char code_data[] = {0/*locals_count*/,i32_auto,21,return_block,end_block};// 0x00 == unreachable as block header !?
//	Code(code_data,sizeof(code_data)); // todo : memcopy, else stack value is LOST
	Code block;
//	Map<int, String>
//	collect_locals(node, context);// DONE IN analyze
//	int locals_count = current_local_names.size();
//	locals[context] = current_local_names;

//	todo : ALWAYS MAKE RESULT VARIABLE FIRST IN BLOCK (index 0, used after while(){} etc) !!!
	int locals_count = locals[context].size();
	int argument_count = functionSignatures[context].size();
	if (locals_count >= argument_count)
		locals_count = locals_count - argument_count;
	else
		warn("locals consumed by arguments"); // ok in  double := it * 2; => double(it){it*2}
	block.addByte(locals_count);
	for (int i = 0; i < locals_count; ++i) {
		Valtype valtype = localTypes[context][i];
		block.addByte(i + 1);// index
		if (valtype == none or valtype == voids)
			valtype = int32;
		block.addByte(valtype);
	}

	Code inner_code_data = emitExpression(node, context);
	Valtype x = last_type;
	block.push(inner_code_data);
	Valtype returns = functionSignatures[context].return_type;// switch back to return_types[context] for block?
	if (returns != last_type) {
		if (returns == Valtype::voids and last_type != Valtype::voids)
			block.addByte(drop);
		if (returns == Valtype::i32t and last_type == Valtype::f32t)
			block.addByte(i32_trunc_f32_s);
		if (returns == Valtype::i32t and last_type == Valtype::voids)
			block.addByte(i32_const).addInt(-999);// hack? return 0/false by default. ok? see python!
//		if(returns==Valtype::f32)…
	}

//if not return_block
	block.addByte(return_block);
	block.addByte(end_block);
	return block;
}

//Map<int, String>
// todo: why can't they be all found in parser?
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


Code typeSection() {
	// Function types are vectors of parameters and return types. Currently
	// TODO optimise - some of the procs might have the same type signature
	// the type section is a vector of function types
	int typeCount = 0;
	Code type_data;

	log(functionIndices);

	for (String fun : functionSignatures) {
		if (!fun) {
//			log(functionIndices);
//			log(functionSignatures);
			breakpoint_helper
			warn("empty functionSignatures[ø] because context=start=''");
//			error("empty function creep functionSignatures[ø]");
			continue;
		}
		Signature &signature = functionSignatures[fun];
		if (not signature.emit /*export/declarations*/ and not signature.is_used /*imports*/) {
			trace("not signature.emit => skipping unused type for "s + fun);
			continue;
		}

		if (signature.is_handled)
			continue;
		if (not functionIndices.has(fun))
			functionIndices[fun] = functionIndices.size();// declared (imports and builtins before)
//			error("function %s should be registered in functionIndices by now"s % fun);

		typeMap[fun] = runtime.type_count /* lib offset */ + typeCount++;
		signature.is_handled = true;
		int param_count = signature.size();
//		Code td = {0x60 /*const type form*/, param_count};
		Code td = Code(0x60) + Code(param_count);

		for (int i = 0; i < param_count; ++i) {
			td = td + Code(signature.types[i]);
		}
		Valtype &ret = functionSignatures[fun].return_type;
		if (ret == voids) {
			td.addByte(0);
		} else {
			td.addByte(1/*return count*/).addByte(ret);
		}
		type_data = type_data + td;
	}
	return Code((char) type_section, encodeVector(Code(typeCount) + type_data)).clone();
}

Code importSection() {

	if (runtime_offset) {
		breakpoint_helper
		printf("imports currently not supported\n");
		import_count = 0;
		return Code();
	}
	// the import section is a vector of imported functions
	Code imports;
	import_count = 0;
	if (functionSignatures["logi"].is_used and ++import_count)
		imports = imports + encodeString("env") + encodeString("logi").addByte(func_export).addType(typeMap["logi"]);
	if (functionSignatures["logf"].is_used and ++import_count)
		imports = imports + encodeString("env") + encodeString("logf").addByte(func_export).addType(typeMap["logf"]);
	if (functionSignatures["square"].is_used and ++import_count)
		imports = imports + encodeString("env") + encodeString("square").addByte(func_export).addType(typeMap["square"]);
	if (imports.length == 0)return Code();
	auto importSection = createSection(import_section, Code(import_count) + imports);// + sqrt_ii
	return importSection.clone();
}

//int function_count;// misleading name: declared functions minus import_count  (together : functionIndices.size() )
//int new_count;
int function_block_count;

//int builtins_used=0;
Code codeSection(Node root) {
	// the code section contains vectors of functions
	// index needs to be known before emitting code, so call $i works
	int new_count;

	new_count = declaredFunctions.size();
	for (auto declared : declaredFunctions) {
		print("declared function: "s + declared);
		if (!declared)error("empty function name (how?)");
		if (not functionIndices.has(declared))// used or not!
			functionIndices[declared] = functionIndices.size();
	}
//	int index_size = functionIndices.size();
//	bool has_main = start and (declaredFunctions.has(start) or functionIndices.has(start));
//	if (import_count + builtin_count + has_main + new_count + runtime_offset != index_size) {
//		log(functionIndices);
//		String message = "inconsistent function_count %d import + %d builtin + %d new + %d runtime + %d main != %d"s;
// 		error(message % import_count % builtin_count % new_count % runtime_offset % has_main % index_size);
//	}
// https://pengowray.github.io/wasm-ops/
//	char code_data[] = {0x01,0x05,0x00,0x41,0x2A,0x0F,0x0B};// 0x41==i32_auto  0x2A==42 0x0F==return 0x0B=='end (function block)' opcode @+39
	byte code_data_fourty2[] = {0/*locals_count*/, i32_auto, 42, return_block, end_block};
	byte code_data_nop[] = {0/*locals_count*/, end_block};// NOP
	byte code_data_id[] = {1/*locals_count*/, 1/*WTF? first local has type: */, i32t, get_local, 0, return_block, end_block};// NOP
//	byte code_data_logi_21[] = {0/*locals_count*/,i32_const,48,function,0 /*logi*/,i32_auto,21,return_block,end_block};
//	byte code_data[] = {0x00, 0x41, 0x2A, 0x0F, 0x0B,0x01, 0x05, 0x00, 0x41, 0x2A, 0x0F, 0x0B};
	Code code_blocks;
	if (runtime.code_count == 0) {
		// order matters, in functionType section!
		if (functionSignatures["nop"].is_used)
			code_blocks = code_blocks + encodeVector(Code(code_data_nop, sizeof(code_data_nop)));
		if (functionSignatures["id"].is_used)
			code_blocks = code_blocks + encodeVector(Code(code_data_id, sizeof(code_data_id)));
	}

	Code main_block = emitBlock(root, start);// after imports and builtins

	if (start) {
		code_blocks = code_blocks + encodeVector(main_block);
	} else {
		if (main_block.length > 3)
			error("no start function name given. null instead of 'main', can't assign block");
		else warn("no start block (ok)");
	}
	for (String fun : functionCodes) {// MAIN block extra ^^^
		Code &func = functionCodes[fun];
		code_blocks = code_blocks + encodeVector(func);
	}
	builtin_count = 0;
	if (functionSignatures["nop"].is_used) builtin_count++;// used
	if (functionSignatures["id"].is_used) builtin_count++;// used

	check(new_count == functionCodes.size());

	bool has_main = start and functionIndices.has(start);
	function_block_count = has_main /*main*/ + builtin_count + functionCodes.size();

	auto codeSection = createSection(code_section, Code(function_block_count) + code_blocks);
	return codeSection.clone();
}

short exports_count = 1;

Code exportSection() {
// the export section is a vector of exported functions etc
	if (!start)
		return createSection(export_section, Code(0));
	int main_offset = 0;
	if (functionIndices.has(start))
		main_offset = functionIndices[start];
	Code exportsData = encodeVector(Code(exports_count) + encodeString(start) + (byte) func_export + Code(main_offset));

	auto exportSection = createSection(export_section, exportsData);
	return exportSection;
}


// Signatures
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
			log(functionIndices);
			error("missing typeMap for index "s + index);
		} else {
			int typeIndex = typeMap[*fun];
			if (typeIndex < 0) {
				if (runtime_offset == 0) // todo else ASSUME all handled correctly before
					error("missing typeMap for function %s index %d "s % fun % i);
			} else
				types_of_functions.push((byte) typeIndex);
		}
	}
	// @ WASM : WHY DIDN'T YOU JUST ADD THIS AS A FIELD IN THE FUNC STRUCT???
	Code funcSection = createSection(functypes_section, types_of_functions);
	return funcSection.clone();
}

Code functionSection() {
	return funcTypeSection();// (misnomer) vs codeSection() !
}

// todo : convert library references to named imports!
Code nameSection() {
	Code nameMap;
//	check(symbols["logi"]==0);
//	check(symbols["√"]==3);// "\e2\88\9a"
//	nameMap =  Code((byte) 0) + Code("logi");
	int total_func_count = functionIndices.size();// imports + function_count, all receive names
	for (int index = runtime_offset; index < total_func_count; index++) {
		// danger: utf names are NOT translated to wat env.√=√ =>  (import "env" "\e2\88\9a" (func $___ (type 3)))
		String &name = functionIndices.keys[index];
		nameMap = nameMap + Code(index) + Code(name);
	}

//	auto functionNames = Code(function_names) + encodeVector(Code(1) + Code((byte) 0) + Code("logi"));
//	functions without parameters need  entry ( 00 01 00 00 )
//  functions 5 with local 'hello' :  05 01 00 05 68 65 6c 6c 6f
//  functions 5 with local 'hello' :  05 02 00 05 68 65 6c 6c 6f 01 00 AND unnamed (local i32t)
// localMapEntry = (index nrLocals 00? string )

	Code localNameMap;
	for (String key : functionIndices) {
//		if (key != start)continue;
		int function_index = functionIndices[key];
		if (function_index < runtime_offset)continue;
		List<String> localNames = locals[key];// including arguments
		int local_count = localNames.size();
		localNameMap = localNameMap + Code(function_index) + Code(local_count); /*???*/
		for (int i = 0; i < localNames.size(); ++i) {
//		String local_name = "test_"s+key+"#"+local_count;
			String local_name = localNames[i];
			localNameMap = localNameMap + Code(i) + Code(local_name);
		}
//		error: expected local name count (1) <= local count (0) FOR FUNCTION ...
	}
//	localNameMap = localNameMap + Code((byte) 0) + Code((byte) 1) + Code((byte) 0) + Code((byte) 0);// 1 unnamed local
//	localNameMap = localNameMap + Code((byte) 4) + Code((byte) 0);// no locals for function4
//	Code exampleNames= Code((byte) 5) /*function index*/ + Code((byte) 1) /*count*/ + Code((byte) 0) /*l.nr*/ + Code("var1");
	// function 5 with one local : var1

//	localNameMap = localNameMap + exampleNames;

	auto moduleName = Code(module_name) + encodeVector(Code("wasp_module"));
	auto functionNames = Code(function_names) + encodeVector(Code(total_func_count) + nameMap);
	auto localNames = Code(local_names) + encodeVector(Code(total_func_count) + localNameMap);

//	The name section is a custom section whose name string is itself ‘𝚗𝚊𝚖𝚎’.
//	The name section should appear only once in a module, and only after the data section.
	const Code &nameSectionData = encodeVector(Code("name") + moduleName + functionNames + localNames);
	auto nameSection = createSection(custom_section, nameSectionData); // auto encodeVector AGAIN!
	nameSection.debug();
	return nameSection.clone();
}

//Code dataSection() {
//	return Code();
//}

Code eventSection() {
	return Code();
}

/*
 *  There are currently two ways in which function indices are stored in the code section:
    Immediate argument of the call instruction (calling a function)
    Immediate argument of the i32.const instruction (taking the address of a function).
    The immediate argument of all such instructions are stored as padded LEB128 such that they can be rewritten without altering the size of the code section. !
    For each such instruction a R_WASM_FUNCTION_INDEX_LEB or R_WASM_TABLE_INDEX_SLEB reloc entry is generated pointing to the offset of the immediate within the code section.

    R_WASM_FUNCTION_INDEX_LEB relocations may fail to be processed, in which case linking fails. This occurs if there is a weakly-undefined function symbol, in which case there is no legal value that can be written as the target of any call instruction. The frontend must generate calls to undefined weak symbols via a call_indirect instruction.
*/
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
	for (auto sig : functionSignatures) {// imports first
		if (functionSignatures[sig].is_import and functionSignatures[sig].is_used) {
			functionIndices[sig] = functionIndices.size();
			import_count++;
		}
	}
	for (auto sig : functionSignatures) {// now builtins
		if (functionSignatures[sig].is_builtin and functionSignatures[sig].is_used) {
			functionIndices[sig] = functionIndices.size();
			builtin_count++;
		}
	}
}

Code &emit(Node root_ast, Module *runtime0, String _start) {
	start = _start;
	typeMap.setDefault(-1);
	typeMap.clear();

	functionIndices.setDefault(-1);
	functionCodes.setDefault(Code());
	functionSignatures.setDefault(Signature());

	if (runtime0) {
		runtime = *runtime0;// else filled with 0's
		runtime_offset = runtime.import_count + runtime.code_count;//  functionIndices.size();
		import_count = 0;
		builtin_count = 0;
		int newly_pre_registered = 0;//declaredFunctions.size();

		if (runtime.total_func_count + newly_pre_registered != functionIndices.size()) {
//			log(functionSignatures.size());
			log(functionIndices);
//			error("runtime.total_func_count !=  functionIndices.size(): "s + runtime.total_func_count +" !="+ functionIndices.size());
			warn("runtime.total_func_count !=  functionIndices.size()");
		}
	} else {
		runtime = *new Module();// all zero
		runtime_offset = 0;
		typeMap.clear();
		functionIndices.clear();// ok preregistered functions are in functionSignatures
//		functionSignatures.clear(); BEFORE analyze(), not after!
		add_builtins();
	}
	if (start) {// now AFTER imports and builtins
		printf("start: %s\n", start.data);
		functionSignatures[start] = Signature().returns(i32t);
		functionSignatures[start].emit = true;
		if (!functionIndices.has(start))
			functionIndices[start] = runtime_offset ? runtime_offset + declaredFunctions.size()
			                                        : functionIndices.size();// AFTER collecting imports!!
		else
			error("start already declared: "s + start + " with index " + functionIndices[start]);
	} else {
//		functionSignatures["_default_context_"] = Signature();
//		start = "_default_context_";//_default_context_
//		start = "";
	}

	functionCodes.clear();
	locals.setDefault(List<String>());

	/* limits https://webassembly.github.io/spec/core/binary/types.html#limits - indicates a min memory size of one page */
	auto memorySection = createSection(memory_section, encodeVector(Code(2)));
	auto memoryImport = encodeString("env") + encodeString("memory") + (byte) mem_export/*type*/+ (byte) 0x00 + (byte) 0x01;
	auto customSection = createSection(custom_section, encodeVector(Code("custom123") + Code("random custom section data")));
	Code typeSection1 = typeSection();// types can be defined in analyze(), not in code declaration
	Code importSection1 = importSection();// needs type indices
	Code codeSection1 = codeSection(root_ast);
	Code funcTypeSection1 = funcTypeSection();// signatures depends on codeSection, but must come before it in wasm
	Code exportSection1 = exportSection();// depends on codeSection, but must come before it!!
	Code code = Code(magicModuleHeader, 4)
	            + Code(moduleVersion, 4)
	            + typeSection1
	            + importSection1
	            + funcTypeSection1 // signatures
	            + exportSection1
	            + codeSection1 // depends on importSection, yields data for funcTypeSection!
	            //			+ dataSection()
	            //			+ linkingSection()
	            + nameSection()
//	 + dwarfSection() // https://yurydelendik.github.io/webassembly-dwarf/
//	 + customSection
	;
	code.debug();
	if (runtime0)functionSignatures.clear(); // cleanup after NAJA
	return code.clone();
}
