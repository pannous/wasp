//
// Created by me on 27.11.20.
//
#include "Code.h"
#include "stdio.h"
#include "wasm_reader.h"

typedef unsigned char *bytes;
int pos = 0;
int size = 0;
byte *code;
Module module;

bytes magicModuleHeader = new byte[]{0x00, 0x61, 0x73, 0x6d};
bytes moduleVersion = new byte[]{0x01, 0x00, 0x00, 0x00};

void parseFunctionNames(Code& code);

#define consume(len, match) if(!consume_x(code,&pos,len,match)){printf("\nNOT consuming %s:%d\n",__FILE__,__LINE__);exit(0);}

#define check(test) if(test){log("OK check passes: ");log(#test);}else{printf("\nNOT PASSING %s\n%s:%d\n",#test,__FILE__,__LINE__);exit(0);}


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
	do {
		byte b = code[pos++];
		n = n << 7;
		n = n ^ (b & 0x7f);
		if ((b & 0x80) == 0)
			break;
	} while (n != 0);
	return n;
}

// DANGER: modifies the start reader position of code, but not it's data!
int unsignedLEB128(Code& byt) {
	int n = 0;
	do {
		byte b = byt[byt.start++];
		n = n << 7;
		n = n ^ (b & 0x7f);
		if ((b & 0x80) == 0)break;
	} while (n != 0);
	return n;
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

Code vec(Code& data, bool consume= true) {
	int len = unsignedLEB128(data);
	Code code1 = Code(data.data+data.start, len);
	if(consume) data.start += len;
	else data.start-=1;// undo len read
	return code1;
}

void consumeTypeSection() {
	Code type_vector = vec();
	int typeCount = unsignedLEB128(type_vector);
	module.type_count = typeCount;
	printf("types: %d\n", module.type_count);
	module.type_data = type_vector.rest();
}

void consumeStartSection() {
	int start_index = unsignedLEB128();
}
void consumeTableSection(){
	module.table_data = vec();
}

void consumeMemorySection(){
	module.memory_data=vec();// todo ?
}
void consumeGlobalSection(){
	module.globals_data=vec();// todo
}

String name(Code& wstring) {
	int len = unsignedLEB128(wstring);
	const String &string = String((char *) wstring.data + wstring.start, len, true);
	wstring.start += len;// advance internally
	return string;
}

void consumeNameSection(Code& data) {
	printf("names: …\n");
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

void parseFunctionNames(Code& payload) {
	int function_count = unsignedLEB128(payload);
	for (int i = 0; i < function_count; ++i) {
		int index = unsignedLEB128(payload);
		String name1 = name(payload);
	}
}

// https://github.com/WebAssembly/tool-conventions/blob/master/Linking.md#linking-metadata-section
void consumeLinkingSection(Code& data) {
	printf("linking: …\n");
	module.linking_section = data;
//	int version = unsignedLEB128(data);
}

void consumeRelocateSection(Code& data) {
	printf("relocate: …\n");
	module.relocate_section = data;
}

void consumeDataSection() {
	module.data_section = vec();
}

void consumeCustomSection() {
	Code customSectionDatas = vec();
	String type = name(customSectionDatas);
	Code payload = customSectionDatas.rest();
	if (type == "names")consumeNameSection(payload);
	else if (type == "linking")consumeLinkingSection(payload);
	else if (type == "relocate")consumeRelocateSection(payload);
	else {
		pos = size;// force finish
//		error("consumeCustomSection not implementated for "s + type);
	}
	module.custom_sections.add(customSectionDatas);// raw
}

// connect func/code indices to type indices
void consumeFuncTypeSection() {
	Code type_vector = vec();
	module.code_count = unsignedLEB128(type_vector);// import type indices are part of import struct!
	printf("signatures: %d\n", module.code_count);
	module.functype_data = type_vector.rest();
}

void consumeCodeSection() {
	Code codes_vector = vec();
	int codeCount = unsignedLEB128(codes_vector);
	printf("codes: %d\n", codeCount);
	if (module.code_count != codeCount)error("missing code/signatures");
	module.code_data = codes_vector.rest();
	printf("code length: %d\n", module.code_data.length);
}

void consumeExportSection() {
	Code exports_vector = vec();
	int exportCount = unsignedLEB128(exports_vector);
	printf("exports: %d\n", exportCount);
	module.export_count = exportCount;
	module.export_data = exports_vector.rest();
}

void consumeImportSection() {
	Code imports_vector = vec();
	int importCount = unsignedLEB128(imports_vector);
	printf("imports %d\n", importCount);
	module.import_count = importCount;
	module.import_data = imports_vector.rest();
}


int fileSize(char const *file) {
	FILE *ptr;
	ptr = fopen(file, "rb");  // r for read, b for binary
	if (!ptr)error("File not found "s + file);
	fseek(ptr, 0L, SEEK_END);
	int sz = ftell(ptr);
	return sz;
}

void consumeSections() {
	while (pos < size) {
		Section section = typ();
		switch (section) {
			case type:
				consumeTypeSection();
				break;
			case import:
				consumeImportSection();
				break;
			case exports:
				consumeExportSection();
				break;
			case code_section:
				consumeCodeSection();
				break;
			case functypes:
				consumeFuncTypeSection();
				break;
			case start_section:
				consumeStartSection();
				break;
			case table:
				consumeTableSection();
				break;
			case memory_section:
				consumeMemorySection();
				break;
			case global:
				consumeGlobalSection();
				break;
			case data_section:
				consumeDataSection();
				break;
			case custom:
				consumeCustomSection();
// => consumeNameSection()
				break;
			default:
				error("not implemented: "s + sectionName(section));
		}
	}
}

Module read_wasm(char const *file) {
	module = *new Module();
	pos = 0;
	printf("parsing: %s\n", file);
	size = fileSize(file);
	bytes buffer = (bytes) alloc(1, size);// do not free
	fread(buffer, sizeof(buffer), size, fopen(file, "rb"));
	code = buffer;
	consume(4, reinterpret_cast<byte *>(magicModuleHeader));
	consume(4, reinterpret_cast<byte *>(moduleVersion));
	consumeSections();
	module.func_count = module.import_count + module.code_count;
	return module;
}
#undef pointerr