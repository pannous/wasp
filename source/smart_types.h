#pragma once
//
// Created by me on 15.11.20.
//
typedef long long int64;
//typedef double float64;
//typedef int1_t bit / bool 0/1 (flag entry)
//typedef uint2_t bibi™ ;) 0…3
//typedef uint3_t boc / octal-char 0…7
//typedef uint4_t bat / hex-char 0…F
//typedef uint8_t byte / unsigned char
//typedef uint16_t bort / word / short
//typedef uint32_t pint / int
//typedef uint64_t poins / int64 / point64 / poinsis
//typedef uint128_t pegat

// 32 bit i32 as [[smart pointer]]s with first hex (4bit) as type
// smartType4bit
// as used in smartlong, smart_pointer_32 and EXTENDED TO upto 4 bytes in smart_pointer_64 !
// OBSOLETE because wasm runtimes now support int64 return from main AND multi-value!
// first bat of Primitive's < 0x100 !
typedef enum smartType4bit {
    int28 = 0x0, // or long60 ?
//	overflow=0x1,
    plong = 0x1, // int64 pointer
    float28 = 0x2,
    //	foverflow=0x3,
    symbola = 0x4, // ≈ stringa &memoryChars[payload]
    json5 = 0x5,
//	long60p = 0x6, // pointer to long60
    septet = 0x7, // 7 hexes à 4 bit OR 7 bytes in smart64! 7 is NUMERIC through 0x7C…0x7F float32 etc
    utf8char = 0x8, // UTF24 Unicode
    stringa = 0x9, // may start with 0x10 ... 0x1F
    anys = 0xA, // Node* / angle array / object pointer to header! => i64 pointer : 32bit type + 32bit pointer indirect or in linear memory!
    nodep = 0xA,
    byter = 0xB, // unsigned char* with length ... ?
    codes = 0xC,  // direct wasm code! (char*) interpreted inline OR:
//	code=0xC,  // angle code tree REDUNDANT wit 0xA / 0xD
    datas = 0xD, // angle node tree as STRING vs 0xA as array
    error_spo = 0xE,
    sint28 = 0xF,// negatives
} smartType;


typedef int SmartPointer32;
typedef unsigned int uint;
typedef unsigned short ushort;
typedef unsigned int smart_pointer_32;// smart_pointer_32
typedef unsigned long long SmartPointer64;
typedef unsigned long long smart_pointer_64;
//typedef uint64 smart_type_64;
typedef smart_pointer_64 smarty;

smartType4bit getSmartType(smart_pointer_32 spo);
//smartType4bit getSmartType(smart_pointer_64 spo);

static smart_pointer_64 smart_pointer_type_mask = 0xFFFFFFFF00000000L; // full : smartType4bit plus 12 specifier bit plus 16 bit type payload
static smart_pointer_64 smart_pointer_header_mask = 0xFFFF000000000000L; // smartType4bit plus 12 specifier  todo maybe less F?
static smart_pointer_64 smart_pointer_payload_mask = 0x0000FFFF00000000L; // todo maybe less F?
//todo stringa = 0x9 vs
#define string_header_64 0x0010000000000000L // todo : what happened to 0x9 smartType4bit ??
#define array_header_64  0x0040000000000000L // why 0x004? because first 2 bats indicate doubles/ints!

static smart_pointer_64 smart_pointer_node_signature = 0xA000000000000000L;// ((smart_pointer_64)nodep) << 63; // 0xA0…0

//    short smart_type_payload = (short)(smartPointer64 & 0x0000FFFF00000000L)>>16;// type payload including length (of array)
