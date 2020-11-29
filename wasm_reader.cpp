//
// Created by me on 27.11.20.
//

#include <wasm3.h>
#include <m3_env.h>
#include "wasm_reader.h"
#include "wasm_emitter.h"

#define consume(len,bytes) if(!consume_x(code,&pos,len,bytes)){printf("\nNOT consuming %s:%d\n",__FILE__,__LINE__);exit(0);}

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

int pos = 0;
byte *code;

byte typ(){
	return code[pos++];
}

int unsignedLEB128() {
	int n=0;
	do {
		byte b = code[pos++];
		n = n << 7;
		n = n ^ (b & 0x7f);
		if(b & 0x80 == 0)break;
	} while (n != 0);
	return n;
}
int unsignedLEB128(Code code) {
	int n=0;
	do {
		byte b = code[code.pos++];
		n = n << 7;
		n = n ^ (b & 0x7f);
		if(b & 0x80 == 0)break;
	} while (n != 0);
	return n;
}
int siz() {
	return unsignedLEB128();
}
Code vec(){
	int from=pos;
	int len=siz();
	consume(len, 0);
	return Code(code, from, pos);
}
Code consumeTypeSection(){
	int from=pos;
	byte type=typ();
	Code type_vector=vec();
	int typeCount = unsignedLEB128(type_vector);
	printf("typeCount %d\n", typeCount);
	Code type_data = type_vector.rest();
	return Code(code, from, pos);
//	return Code(type, encodeVector(Code(typeCount) + type_data));
}

Code read(byte *code0, int length) {
	pos = 0;
	code = code0;
	consume(4, reinterpret_cast<byte *>(magicModuleHeader));
	consume(4, reinterpret_cast<byte *>(moduleVersion));
	consumeTypeSection();
//	consumeImportSection();
}
int fileSize(char const *file) {
	FILE *ptr;
	ptr = fopen(file, "rb");  // r for read, b for binary
	if (!ptr)error("File not found "s + file);
	fseek(ptr, 0L, SEEK_END);
	int sz = ftell(ptr);
	return sz;
}
Code readWasm(char const *file) {
	int sz = fileSize(file);
	FILE *ptr;
	ptr = fopen(file, "rb");  // r for read, b for binary
	if (!ptr)error("File not found "s + file);
	unsigned char buffer[sz];
	fread(buffer, sizeof(buffer), sz, ptr);
	Code c(buffer, 0, sz);
	c.run();
	c.debug();
	IM3Environment environment;
	IM3Module module;
	M3Result result = m3_ParseModule(environment, &module, buffer, sz);
	printf("parsed: %s\n", result);
	printf("Module: %s\n", module->name);

//	read(buffer, sz);
}