#pragma once
//
// Created by me on 15.11.20.
//


// 32 bit i32 as [[smart pointer]]s with first hex (4bit) as type
// smartType4bit
// as used in smartlong, smart_pointer_32 and EXTENDED TO upto 4 bytes in smart_pointer_64 !
// OBSOLETE because wasm runtimes now support long return from main AND multi-value!
typedef enum smartType4bit {
	int28 = 0x0, // or long60 ?
//	overflow=0x1,
	plong = 0x1, // long pointer
	float28 = 0x2,
	//	foverflow=0x3,
	symbola = 0x4, // ≈ stringa &memoryChars[payload]
	json5 = 0x5,
//	long60p = 0x6, // pointer to long60
	septet = 0x7, // 7 hexes à 4 bit OR 7 bytes in smart64!
	utf8char = 0x8, // UTF24 Unicode
	stringa = 0x9, // may start with 0x10 ... 0x1F
	anys = 0xA, // angle array / object pointer to header! => i64 pointer : 32bit type + 32bit pointer indirect or in linear memory!
	byter = 0xB, // unsigned char* with length ... ?
	codes = 0xC,  // direct wasm code! interpreted inline OR:
//	code=0xC,  // angle code tree REDUNDANT wit 0xA / 0xD
	datas = 0xD, // angle node tree as STRING vs 0xA as array
	error_spo = 0xE,
	sint28 = 0xF,// negatives
} smartType;


typedef unsigned int smart_pointer_32;// smart_pointer_32
typedef unsigned long long smart_pointer_64;
typedef unsigned long long smart_type_64;
typedef smart_pointer_64 smarty;
smartType4bit getSmartType(smart_pointer_32 spo);
//smartType4bit getSmartType(smart_pointer_64 spo);
