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

#define consume(len, match) if(!consume_x(code,&pos,len,match)){printf("\nNOT consuming %s:%d\n",__FILE__,__LINE__);exit(0);}

#define check(test) if(test){log("OK check passes: ");log(#test);}else{printf("\nNOT PASSING %s\n%s:%d\n",#test,__FILE__,__LINE__);exit(0);}


bool consume_x(byte *code, int *pos, int len, byte *bytes) {
	if(*pos+len>size)
		error("END OF FILE");
	if(not bytes){
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
	return (Section)code[pos++];
}

int unsignedLEB128() {
	int n = 0;
	do {
		byte b = code[pos++];
		n = n << 7;
		n = n ^ (b & 0x7f);
		if ( (b & 0x80) == 0)
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
	return Code(code+from, pos-from);

}

void consumeTypeSection() {
	Code type_vector = vec();
	int typeCount = unsignedLEB128(type_vector);
	module.type_count = typeCount;
	printf("types: %d\n", module.type_count);
	module.type_data = type_vector.rest();
}
void consumeStartSection(){
	int start_index = unsignedLEB128();
}
String name(Code wstring){
	int len = unsignedLEB128(wstring);
	return String((char*)wstring.data+wstring.start, len, true);
}
void consumeNameSection(Code data) {
	printf("names: …\n");
}

// https://github.com/WebAssembly/tool-conventions/blob/master/Linking.md#linking-metadata-section
void consumeLinkingSection(Code data) {
	printf("linking: …\n");
//	int version = unsignedLEB128(data);
}

void consumeCustomSection(){
	Code customSectionDatas=vec();
	String type = name(customSectionDatas);
	if(type=="names")consumeNameSection(customSectionDatas);
	else if(type=="linking")consumeLinkingSection(customSectionDatas);
	else error("consumeCustomSection not implementated for "s+type);
}

void consumeFuncTypeSection(){
	Code type_vector = vec();
	module.func_count = unsignedLEB128(type_vector);
	printf("signatures: %d\n", module.func_count);
	module.functype_data = type_vector.rest();
}
void consumeCodeSection(){
	Code codes_vector = vec();
	int codeCount = unsignedLEB128(codes_vector);
	printf("codes: %d\n", codeCount);
	if(module.func_count != codeCount)error("missing code/signatures");
	module.code_data = codes_vector.rest();
}

void consumeExportSection(){
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

void consumeSections(){
	while (pos<size){
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
		case custom:
			consumeCustomSection();
			break;
		default:
			error("not implemented: "s + sectionName(section));
	}
	}
}

Module read_wasm(char const *file) {
	pos = 0;
	printf("parsing: %s\n", file);
	size = fileSize(file);
	bytes buffer=(bytes)alloc(1, size);// do not free
	fread(buffer, sizeof(buffer), size, fopen(file, "rb"));
	code = buffer;
	consume(4, reinterpret_cast<byte *>(magicModuleHeader));
	consume(4, reinterpret_cast<byte *>(moduleVersion));
	consumeSections();
	return module;
}

Code mergeTypeSection(Module lib, Module main){
	return Code(type, encodeVector(Code(lib.type_count+main.type_count) + lib.type_data + main.type_data));
}
Code mergeImportSection(Module lib, Module main){
	return createSection(import, Code(lib.import_count+main.import_count)+  lib.import_data + main.import_data);
}

Code mergeFuncTypeSection(Module lib, Module main){
	return createSection(functypes, Code(lib.func_count + main.func_count) + lib.functype_data + main.functype_data);
}
Code mergeExportSection(Module lib, Module main){
	return createSection(exports, Code(lib.export_count + main.export_count) + lib.export_data + main.export_data);
}
Code relocate(Code blocks){
	return blocks;// todo, maybe
}

Code mergeCodeSection(Module lib, Module main) {
	return createSection(code_section, Code(lib.func_count + main.func_count) + lib.code_data + relocate(main.code_data));
}

Code mergeDataSection(Module lib, Module main) {
	return Code();// todo
}
Code mergeLinkingSection(Module lib, Module main) {
	return Code();// todo
}
Code mergeNameSection(Module lib, Module main) {
	return Code();// todo
}

Code merge_code(Module lib, Module main){
	Code code = Code(magicModuleHeader, 4) + Code(moduleVersion, 4)
			+ mergeTypeSection(lib,main)
			+ mergeImportSection(lib,main)
			+ mergeFuncTypeSection(lib,main)
			+ mergeExportSection(lib,main)
			+ mergeCodeSection(lib,main)
			+ mergeDataSection(lib,main)
			+ mergeLinkingSection(lib,main)
			+ mergeNameSection(lib,main);
//	+ mergeCustomeSection(lib,main);
	return code.clone();
}

#undef pointerr