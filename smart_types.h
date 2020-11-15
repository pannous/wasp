//
// Created by me on 15.11.20.
//

#pragma once

#ifndef WASP_SMART_TYPES_H
#define WASP_SMART_TYPES_H

#endif //WASP_SMART_TYPES_H

// 32 bit i32 as [[smart pointer]]s with first hex as type
// smartType4bit
typedef enum smartType{
	int28=0x0,
//	overflow=0x1,
    plong=0x1, // long pointer
	float28=0x2,
	//	foverflow=0x3,
	symbola=0x4,
	json5 = 0x5,
	int60p = 0x6, // pointer to
	septet=0x7,
	utf8char=0x8, // UTF24 Unicode
	stringa = 0x9, // may start with 0x10 ... 0x1F
	any=0xA, // angle pointer/object => i64 pointer : 32bit type + 32bit pointer indirect or in linear memory!
	byter=0xB, // unsigned char* with length ... ?
	code=0xC,  // direct wasm code! interpreted inline OR:
//	code=0xC,  // angle code tree
	data=0xD, // angle node tree
	error_spo=0xE,
	sint28=0xF,// negatives
}smartType;
//enum smartType8bit{
//	int24=0x0,
////	overflow=0x1,
//	plong=0x1, // long pointer
//	float28=0x2,
//	//	foverflow=0x3,
//	json5 = 0x5,
//	int60p = 0x6, // pointer to
//	septet=0x7,
//	utf8char=0x8, // UTF24 Unicode
//	string = 0x9, // may start with 0x10 ... 0x1F
//	anys=0xA, // any angle pointer => i64 pointer : 32bit type + 32bit pointer
//	byter=0xB, // unsigned char*
//	code=0xC,
//	data=0xD,
//	error_spo=0xE,
//	sint28=0xF,
//};

typedef unsigned int spointer;
smartType getSmartType(spointer spo);