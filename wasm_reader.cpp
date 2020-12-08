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

#define consume(len, bytes) if(!consume_x(code,&pos,len,bytes)){printf("\nNOT consuming %s:%d\n",__FILE__,__LINE__);exit(0);}

#define check(test) if(test){log("OK check passes: ");log(#test);}else{printf("\nNOT PASSING %s\n%s:%d\n",#test,__FILE__,__LINE__);exit(0);}

#define pointerr std::unique_ptr

bool consume_x(byte *code, int *pos, int len, byte *bytes) {
//	if(bytes)
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

int unsignedLEB128(Code code) {
	int n = 0;
	do {
		byte b = code[code.pos++];
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
	int from = pos;
	int len = siz();
	consume(len, 0);
	return Code(code, from, pos);
}

Code consumeTypeSection() {
	Code type_vector = vec();
	module.type_count = unsignedLEB128(type_vector);
	printf("typeCount %d\n", module.type_count);
	module.type_data = type_vector.rest();
	return type_vector;
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
	Section section = typ();
	switch (section) {
		case type:
			consumeTypeSection();
		case import:
		default:
			error("not implemented"s + sectionName(section));
	}

}

Module read_wasm(char const *file) {
	printf("parsing: %s\n", file);
	size = fileSize(file);
	unsigned char buffer[size];
	fread(buffer, sizeof(buffer), size, fopen(file, "rb"));
	code = buffer;
	consume(4, reinterpret_cast<byte *>(magicModuleHeader));
	consume(4, reinterpret_cast<byte *>(moduleVersion));
	consumeSections();
	return module;
}

#undef pointerr