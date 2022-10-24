//
// Created by me on 27.11.20.
//
#include "Code.h"

#ifndef WASM
#include "stdio.h"
#endif

#include "wasm_reader.h"
#include "wasm_emitter.h"
#include "Util.h"

// https://webassembly.github.io/spec/core/binary/modules.html#sections
String sectionName(Section section);
// compare with wasm-objdump -h
bool debug_reader = false;
typedef unsigned char *bytes;
int pos = 0;
int size = 0;
byte *code;
Module module;
extern Map<String, int> functionIndices;
//extern List<String> declaredFunctions; only new functions that will get a Code block, no runtime/imports


Valtype mapArgToValtype(String arg);

#define consume(len, match) if(!consume_x(code,&pos,len,match)){if(debug_reader)printf("\nNOT consuming %s:%d\n",__FILE__,__LINE__);exit(0);}

bool consume_x(byte *code, int *pos, int len, byte *bytes) {
	if (*pos + len > size)
		error("END OF FILE");
	if (not bytes) {
		*pos = *pos + len;
		return true;
	}

	int i = 0;
	while (i < len) {
		if (bytes and code[*pos] != bytes[i])
			return false;
		*pos = *pos + 1;
		i++;
	}
	return true;
}

Section typ() {
	return (Section) code[pos++];
}

int unsignedLEB128() {
	int n = 0;
	short shift = 0;
	do {
		byte b = code[pos++];
		n = n | (((int) (b & 0x7f)) << shift);
		if ((b & 0x80) == 0)
			break;
		shift += 7;
	} while (pos < size);
	return n;
}

// DANGER: modifies the start reader position of code, but not it's data!
int unsignedLEB128(Code &byt) {
	int n = 0;
	short shift = 0;
	do {
		byte b = byt.data[byt.start++];
		n = n | (((long) (b & 0x7f)) << shift);
		if ((b & 0x80) == 0)break;
		shift += 7;
	} while (byt.start < byt.length);
	return n;
}

int consumeByte(Code &byt) {
	return byt.data[byt.start++];
}

int siz() {
	return unsignedLEB128();
}

Code vec() {
	int len = siz();
	int from = pos;
	consume(len, 0);
//	return Code(code+from, from, pos);
	return Code(code + from, pos - from);
}

Code vec(Code &data, bool consume = true) {
	int len = unsignedLEB128(data);
	Code code1 = Code(data.data + data.start, len);
	if (consume) data.start += len;
	else data.start -= 1;// undo len read
	return code1;
}

String &name(Code &wstring) {
	int len = unsignedLEB128(wstring);
	String *string = new String((char *) wstring.data + wstring.start, len, true);
	wstring.start += len;// advance internally
//	if (len > 40)put(string);
	return *string;
}

// todo: treat all functions (in library file) as exports if ExportSection is empty !?
// that is: add their signatures to module …
void parseFunctionNames(Code &payload) {
	functionIndices.setDefault(-1);
//	put(functionIndices);// what we got so far?
	int function_count = unsignedLEB128(payload);
	int index = -1;
	for (int i = 0; i < function_count and payload.start < payload.length; ++i) {
		index = unsignedLEB128(payload);
		if (index < 0 or index > 100000)
			error("broken index"s + index);
		if (i != index)// in partial main.wasm
			warn("index out of order "s + i + " <> " + index);// doesn't happen
		String func = name(payload).clone();// needs to be 0-terminated now
		if (functionIndices[func] > 0 and functionIndices[func] < function_count /*hack!*/) {
			if (functionIndices[func] == index)
				continue; // identical match, lib parsed twice without cleaning functionIndices!?
			// export section ≠ name section (when overloading…)
			trace("already has index: "s + func + " " + functionIndices[func] + "≠" + index);
			continue;
			func = func + "_func_" + index;// hack ok to avoid duplicates
			func = func.clone();
		}
		if (func.length > 0)
//			functionIndices.insert_or_assign(func, index);
			functionIndices[func] = index;
		else {
			error("function without name at index "s + index);// happens with unicode π(x) etc
			//			warn
			functionIndices["func_"s + index] = index;
//			functionIndices.insert_or_assign("func_"s + index, index);
		}
	}
//	  (import "env" "log_chars" (func (;0;) $logs (type 0)))  export / import names != internal names
//	for (int i = function_count; i < module.total_func_count; i++)
//		functionIndices.insert_or_assign("unnamed_func_"s + i, i);

}

//Map<int, Signature> funcTypes;
List<Signature> funcTypes;// implicit index
void parseFuncTypeSection(Code &payload) {
	// we don't know here if i32 is pointer … so we may have to refine later
	for (int i = 0; i < module.code_count and payload.start < payload.length; ++i) {
		int typ = unsignedLEB128(payload);// implicit?
		String *fun = functionIndices.lookup(i + module.import_count);
		if (!fun)continue;
//			error("no name for function "s+i);
		Signature &s = funcTypes[typ];
		Signature &sic = functionSignatures[*fun];
		functionSignatures[*fun] = s;
	}
}

// not part of name section wtf
void parseImportNames(Code &payload) {
	functionIndices.setDefault(-1);
	functionSignatures.setDefault(Signature());
	trace("Imports:");
	for (int i = 0; i < module.import_count and payload.start < payload.length; ++i) {
		String mod = name(payload);// module
		String name1 = name(payload).clone();// needs to be 0-terminated now
		int huh = unsignedLEB128(payload);
		int type = unsignedLEB128(payload);
		trace(name1);
		Signature &signature = funcTypes[type];
		functionIndices[name1] = i;
		functionSignatures[name1] = signature;// overwrites any preregistered signatures OK cause correct!?
		functionSignatures[name1].import().runtime().handled();//.functionType(huh);//.runtime()
		module.import_names.add(name1);
	}
	module.signatures = functionSignatures; // todo merge into global functionSignatures, not the other way round!!
}

// Signatures to be consumed in export section
void parse_type_data(Code &payload) {
	for (int i = 0; i < module.type_count and payload.start < payload.length; ++i) {
		Signature sic;
		int typ = unsignedLEB128(payload);// implicit?
		if (typ != func)continue;
		int param_count = unsignedLEB128(payload);
		for (int j = 0; j < param_count; j++) {
			Valtype argt = (Valtype) unsignedLEB128(payload);
			sic.add(argt);
		}
		int return_count = unsignedLEB128(payload);
		if (return_count) {// todo: multi-value
			Valtype rt = (Valtype) unsignedLEB128(payload);
			sic.returns(rt);
		} else
			sic.returns(none);
		funcTypes.add(sic);
	}
}

void consumeTypeSection() {
	Code type_vector = vec();
	int typeCount = unsignedLEB128(type_vector);
	module.type_count = typeCount;
	if (debug_reader)printf("types: %d\n", module.type_count);
	module.type_data = type_vector.rest();
// todo: we need to parse this for automatic import and signatures
	parse_type_data(module.type_data);
}

void consumeStartSection() {
	module.start_index = unsignedLEB128();
	if (debug_reader)printf("start: #%d \n", module.start_index);
}

void consumeTableSection() {
	module.table_data = vec();
	module.table_count = 1;// unsignedLEB128(module.table_data);
	if (debug_reader)printf("tables: %d \n", module.table_count);
}

void consumeMemorySection() {
	module.memory_data = vec();// todo ?
//	module.memory_count = unsignedLEB128(module.memory_data);//  always 1 in MVP
	if (debug_reader)printf("memory_data: %d\n", module.memory_data.length);
}

void consumeGlobalSection() {
	module.globals_data = vec();// todo
	module.global_count = unsignedLEB128(module.globals_data); // NO SUCH THING!?
	if (debug_reader)printf("globals: %d\n", module.global_count);
}

void consumeNameSection(Code &data) {
	if (debug_reader)printf("names: …\n");
	module.name_data = data.clone();
	while (data.start < data.length) {
		int type = unsignedLEB128(data);
		Code payload = vec(data);// todo test!
		switch (type) {
			case module_name: {
				module.name = name(payload);// wrapped in vector why?
			}
				break;
			case function_names:
				module.function_names = payload;
				parseFunctionNames(payload.clone());
				break;
			case local_names:
				module.local_names = payload;
				break;
			default:
				error("INVALID NAME TYPE");
		}
	}
}


// https://github.com/WebAssembly/tool-conventions/blob/master/Linking.md#linking-metadata-section
void consumeLinkingSection(Code &data) {
	if (debug_reader)printf("linking: …\n");
	module.linking_section = data;
//	int version = unsignedLEB128(data);
}

void consumeRelocateSection(Code &data) {
	if (debug_reader)printf("relocate: …\n");
	module.relocate_section = data;
}

void consumeDataSection() {
	Code datas = vec();
//	module.data_section = datas.clone();
	module.data_segments_count = unsignedLEB128(datas);
	module.data_segments = datas.rest();// whereever the start may be now
	short memory_id = unsignedLEB128(datas);
	unsignedLEB128(datas);// skip i32.const opcode
	int data_offset = unsignedLEB128(datas);
	unsignedLEB128(datas);// skip '0b' whatever that is
	module.data_offset_end = data_offset + module.data_segments.length +100;//  datas.length;
	// Todo ILL-DEFINED BEHAVIOUR! silent corruption if data_offset_end too small (why 100?)

//	if (debug_reader)
//printf("data sections: %d from offsets %d to %d \n", module.data_segments_count, data_offset,module.data_offset_end);
//	if(debug_reader)printf("data section offset: %ld \n", offset);
}

void consumeElementSection() {
module.element_section = vec();
//	if(debug_reader)printf("element section (!?)");
if (debug_reader)printf("element sections: %d \n", module.element_section.length);
}

void consumeCustomSection() {
	Code customSectionDatas = vec();
	String type = name(customSectionDatas);
	Code payload = customSectionDatas.rest();
	if (type == "names") consumeNameSection(payload);
	else if (type == "target_features") todo("target_features detection not yet supported");
//	https://github.com/WebAssembly/tool-conventions/blob/main/Linking.md#target-features-section
// 	atomics bulk-memory exception-handling multivalue mutable-globals nontrapping-fpoint sign-ext simd128 tail-call
	else if (type == "linking") consumeLinkingSection(payload);
		// see https://github.com/WebAssembly/tool-conventions/blob/main/Linking.md
	else if (type == "dylink.0") todo("dynamic linking not yet supported");
		// see https://github.com/WebAssembly/tool-conventions/blob/main/DynamicLinking.md
	else if (type.startsWith("reloc.")) consumeRelocateSection(payload);// e.g. "reloc.CODE"
		// everything after the period is ignored and the specific target section is encoded in the reloc section itself.
		// see https://github.com/WebAssembly/tool-conventions/blob/main/Linking.md
	else if (type == "relocate")consumeRelocateSection(payload);
	else {
//		pos = size;// force finish
//		error("consumeCustomSection not implementated for "s + type);
		customSectionDatas.start = 0;// reset
		module.custom_sections.add(customSectionDatas);// raw
	}
}


// connect func/code indices to type indices
void consumeFuncTypeSection() {
	Code type_vector = vec();
	module.code_count = unsignedLEB128(type_vector);// import type indices are part of import struct!
	if (debug_reader)printf("signatures: %d\n", module.code_count);
	module.functype_data = type_vector.rest();
}

// todo: treat all functions (in library file) as exports if ExportSection is empty !?
void consumeCodeSection() {
	Code codes_vector = vec();
	int codeCount = unsignedLEB128(codes_vector);
	if (debug_reader)printf("codes: %d\n", codeCount);
	if (module.code_count != codeCount)error("missing code/signatures");
	module.code_data = codes_vector.rest();
	if (debug_reader)printf("code length: %d\n", module.code_data.length);
}


#include <cxxabi.h> // for abi::__cxa_demangle

// we can reconstruct arguments from demangled exports or retained wast names
// _Z2eqPKcS0_i =>  func $eq_char_const*__char_const*__int_ <= eq(char const*, char const*, int)
List<String> demangle_args(String &fun) {
	List<String> args;
	int status;
	String *real_name = new String(abi::__cxa_demangle(fun.data, 0, 0, &status));
	if (status != 0)return args;
	if (!real_name or !real_name->contains("("))return args;
	String brace = real_name->substring(real_name->indexOf('(') + 1, real_name->indexOf(')'));//.clone();
	if (brace.contains("("))
		return args;// function pointers not supported yet "List<String>::sort(bool (*)(String&, String&))"
	args = brace.split(", ");
	return args;
}

String demangle(String &fun) {
	int status;
	String *real_name = new String(abi::__cxa_demangle(fun.data, 0, 0, &status));
	if (status != 0)return fun;// not demangled (e.g. "memory")
//	String ok = *real_name;
	String ok = real_name->substring(0, real_name->indexOf(
			'(')).clone();// todo: use type string somehow? info(char const*) …
	return ok;
}

// todo: treat all functions (in library file) as exports if ExportSection is empty !?
void consumeExportSection() {
	Code exports_vector = vec();
	int exportCount = unsignedLEB128(exports_vector);
	if (debug_reader)printf("export_section: %d\n", exportCount);
	module.export_count = exportCount;
	module.export_data = exports_vector.rest();
	Code &payload = module.export_data;
	for (int i = 0; i < exportCount; i++) {
		String func0 = name(payload).clone();
		if (func0 == "_Z5main4iPPc")continue;// don't make libraries 'main' visible, use own
		if (func0 == "_Z6concatPKcS0_")
			debug = 1;
		List<String> args = demangle_args(func0);
		String func = demangle(func0);// todo: use wasm_signature if demangling fails
		module.export_names.add(func);
		int type = unsignedLEB128(payload);
		int index = unsignedLEB128(payload);
		if (index < 0 or index > 100000)error("corrupt index "s + index);
		if (type == 0/*func*/ and not functionIndices.has(func)) {
			functionIndices[func0] = index;// mangled
			functionIndices[func] = index;// demangled

			Signature &wasm_signature = funcTypes[type];
			Valtype returns = int32;
			returns = mapTypeToWasm(wasm_signature.return_type);

			Signature &signature = Signature().runtime().returns(returns);
			// todo: use wasm_signature if demangling fails
			for (String arg: args) {
				if (arg.empty())continue;
				signature.add(mapArgToValtype(arg));
			}
			if (!functionSignatures.has(func))
				functionSignatures[func] = signature; // … library functions currently hardcoded
			if (!functionSignatures.has(func0))
				functionSignatures[func0] = signature;
		}
	}
}

// four different types:
// 1. wasm Valtype
// 2. node.kind:Type
// 3. Any<Node and
// 4. some c++ types String List etc
// the last three can be added as special internal values to Valtype, outside the wasm spec
Valtype mapArgToValtype(String arg) {
//	if(arg=="const char*")return Valtype::charp;
	if (arg.empty() or arg == "" or arg == " ") return Valtype::voids;
	else if (arg == "unsigned char*")return Valtype::charp;// pointer with special semantics
	else if (arg == "char const*")return Valtype::charp;// pointer with special semantics
	else if (arg == "char const*&")return Valtype::charp;// todo ?
	else if (arg == "char*")return Valtype::charp;
	else if (arg == "char32_t*")return Valtype::charp;// huh? why now?
	else if (arg == "char const**")return Valtype::pointer;
	else if (arg == "short")
		return Valtype::int32;// careful c++ ABI overflow? should be fine since wasm doesnt have short
	else if (arg == "int")return Valtype::int32;
	else if (arg == "unsigned char")return Valtype::int32;
	else if (arg == "int*")return Valtype::pointer;
	else if (arg == "void*")return Valtype::pointer;
	else if (arg == "long")return Valtype::int64;
	else if (arg == "double")return Valtype::float64;
	else if (arg == "unsigned long")return Valtype::int64;
	else if (arg == "float")return Valtype::float32;
	else if (arg == "bool")return Valtype::int32;
	else if (arg == "short*")return Valtype::int32;
	else if (arg == "char")return Valtype::int32;// c++ char < angle codepoint ok
	else if (arg == "wchar_t")return Valtype::codepoint32;// angle codepoint ok
	else if (arg == "char32_t")return Valtype::codepoint32;// angle codepoint ok
	else if (arg == "Type")return Valtype::int32;// enum
	else if (arg == "Kind")return Valtype::int32;// enum (short, ok)
	else if (arg == "Code")return Valtype::ignore;
	else if (arg == "Valtype")return Valtype::int32;// enum
	else if (arg == "String*")return Valtype::stringp;
	else if (arg == "String")return Valtype::stringp;// todo !DIFFERENT
	else if (arg == "String&")return Valtype::stringp;// todo: how does c++ handle refs?
	else if (arg == "Node*")return Valtype::pointer;
	else if (arg == "char**")return Valtype::pointer;// to chars
	else if (arg == "Node")return Valtype::pointer;
	else if (arg == "Node&")return Valtype::pointer;// pointer? todo: how does c++ handle refs?
	else if (arg == "Node const&")return Valtype::pointer;
		// todo:
	else if (arg == "List<String>")return Valtype::todoe;
	else if (arg == "List<Valtype>")return Valtype::todoe;
	else if (arg == "std::is_arithmetic<int>::value")return Valtype::todoe;// WAT?? PURE_WASM should work without std!!
//	else if (arg == "List< …

		// IGNORE INTERNAL TYPES:
	else if (arg == "Map<String")return Valtype::ignore;
	else if (arg == "int>")return Valtype::ignore;// parse bug ^^
	else if (arg == "Code")return Valtype::ignore;
	else if (arg == "Code&")return Valtype::ignore;
	else if (arg == "Module")return Valtype::ignore;
	else if (arg == "Section")return Valtype::ignore;
	else if (arg == "Valtype&")return Valtype::ignore;
	else if (arg == "Module const&")return Valtype::ignore;
	else if (arg == "Value")return Valtype::ignore;
	else if (arg == "Arg")return Valtype::ignore; // truely internal, should not be exposed! e.g. Arg
	else if (arg == "Signature")return Valtype::ignore;
	else if (arg == "Code const&")return Valtype::ignore;
	else
		printf("unmapped c++ argument type %s\n", arg.data);
	return i32t;
}

void consumeImportSection() {
	Code imports_vector = vec();
	int importCount = unsignedLEB128(imports_vector);
	module.import_count = importCount;
	module.import_data = imports_vector.rest();
	parseImportNames(imports_vector);
	if (debug_reader)printf("imports: %d\n", importCount);
}


void consumeSections() {
	while (pos < size) {
		Section section = typ();
		switch (section) {
			case type_section:
				consumeTypeSection();
				break;
			case import_section:
				consumeImportSection();
				break;
			case export_section:
				consumeExportSection();
				break;
			case code_section:
				consumeCodeSection();
				break;
			case functypes_section:
				consumeFuncTypeSection();
				break;
			case start_section:
				consumeStartSection();
				break;
			case table_section:
				consumeTableSection();
				break;
			case memory_section:
				consumeMemorySection();
				break;
			case global_section:
				consumeGlobalSection();
				break;
			case data_section:
				consumeDataSection();
				break;
			case element_section:
				consumeElementSection();
				break;
			case custom_section:
				consumeCustomSection(); // => consumeNameSection()
				break;
			default:
				if (debug_reader)printf("Invalid section %d at pos %d %x\n", section, pos, pos);
				error("not implemented: "s + sectionName(section));
		}
	}
}

#ifndef RUNTIME_ONLY


Module read_wasm(bytes buffer, int size0) {
	module = *new Module(); // todo: make pure, not global!
	pos = 0;
	code = buffer;
	size = size0;
	funcTypes.clear();
	consume(4, reinterpret_cast<byte *>(magicModuleHeader));
	consume(4, reinterpret_cast<byte *>(moduleVersion));
	consumeSections();
	module.total_func_count = module.import_count + module.code_count;
	parseFuncTypeSection(module.functype_data);// only after we have the name, so we can connect functionSignatures!
	return module;
}

	Module read_wasm(chars file) {
#if    WASM
	return Module();
#else
	if (debug_reader)printf("--------------------------\n");
		if (debug_reader)printf("parsing: %s\n", file);
		size = fileSize(file);
		if (size <= 0)error("file not found: "s + file);
		bytes buffer = (bytes) alloc(1, size);// do not free
		fread(buffer, sizeof(buffer), size, fopen(file, "rb"));
		return read_wasm(buffer, size);
#endif
		}

#endif
#undef pointerr

//#import "wasm_patcher.cpp"