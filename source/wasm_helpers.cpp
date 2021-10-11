//#pragma once

#ifdef WASI
//#include <cstdio> // printf
//#include </usr/local/Cellar/llvm/11.0.0/include/c++/v1/stdio.h> // printf
#endif

//#include <climits>
//
// Created by pannous on 15.07.20.
//

#include "wasm_helpers.h"
#include "String.h"
#include "Backtrace.h"
#include "Code.h"
//extern unsigned int *memory;

//#ifdef WASI
//#include <stdio.h> // printf
//#endif

void *wasm_memory = 0;// c pointer of VM, NOT memory inside wasm module

#ifdef WASM

void free(void*){/*lol*/}


void *malloc(size_t size){//}  __result_use_check __alloc_size(1){ // heap
	void *last = current;
	current += size;
//	if(size>1000)
//		error("TOO BIG LOL");
	if (MEMORY_SIZE and (long) current >= MEMORY_SIZE) {
#ifndef WASI
		logi(sizeof(Node));// 64
		logi(sizeof(String));// 20
		logi(sizeof(Value));// 8 long
		logi((int) last);
		logi((int) memory);
		logi((int) current);
		logi((int) HEAP_OFFSET);
		logi(MEMORY_SIZE);
//		error("OUT OF MEMORY");// needs malloc :(
#endif
		panic();
		last = current = (char *) (4 * HEAP_OFFSET);// reset HACK todo!
	}
	return last;
}
void println(String s){
	print(s);
	logc('\n');
}
//void printf(chars s) {
//	logs(s);
////	while(*s)logc(*s++);
//}
void print(String s){
		log(s.data);
//	logs(s.data,s.length)
}
void printf(chars format, int i) {
	print(String(format).replace("%d", String(i)).replace("%i", String(i)).replace("%li", String(i)));
}
#ifndef WASM
void printf(chars format, number i) {
	print(String(format).replace("%d", String(i)).replace("%i", String(i)).replace("%li", String(i)));
}
#endif
void printf(chars format, chars value) {
	print(String(format).replace("%s", value));
}
void printf(chars format, chars i, chars j){
	print(String(format).replace("%s", i).replace("%s", j));
}

void printf(chars format, chars i, chars j, int l){
	print(String(format).replace("%s", i).replace("%s", j).replace("%d", String(l)));
}
void printf(chars format, chars i, chars j, chars l){
print(String(format).replace("%s", i).replace("%s", j).replace("%d", l));
}
void printf(chars format, long i, long j) {
	if(contains(format,"%ld"))
	print(String(format).replace("%ld", String(i)).replace("%ld", String(j)));
	else
	print(String(format).replace("%d", String(i)).replace("%d", String(j)));
}
void printf(chars format, int i, int j) {
	print(String(format).replace("%d", String(i)).replace("%d", String(j)));
}

void printf(chars format, double i, double j) {
	print(String(format).replace("%f", String(i)).replace("%f", String(j)));
}

void printf(chars format, chars val, int value) {
	print(String(format).format((char*)val).format(value));
}

void printf(chars format, void* value) {
#ifndef WASM
	print(String(format).replace("%p", String((number)value)));
#else
//	memory-value
	print(String(format).replace("%p", 0));// TODO
#endif

}
#endif
int isalnum0(int c) {
	return (c >= '0' and c <= '9') or (c >= 'a' and c <= 'z') or (c >= 'A' and c <= 'Z');// todo lol
}
//#define __cxa_allocate_exception 1
//#define __cxa_throw 1
//#include <typeinfo>       // operator typeid
#ifndef WASM
#ifndef WASI
extern "C" unsigned int *memory = 0;// dummies, remove!
extern "C" /*unsigned */ char *current = 0;// dummies, remove!


double powd(double x, double y) {// why this crutch? maybe conflicting pow's in all those xyz_math.h
	return pow(x, y);
}

int raise(chars error) {
//#ifdef WASM3
	Backtrace(3);
	print(error);
	if (panicking)
		exit(EXIT_FAILURE);
#ifdef WASM
#endif
//	if (!throwing)
//#endif
	if (throwing)
		throw error;
	return -1;
}

#else
extern "C" void ___cxa_throw(
		void* thrown_exception,
		struct type_info *tinfo,
		void (*dest)(void*));
void raise(chars error){
//	throw error;//  typeinfo for chars yadiya
//	_raise(error);
	printf("ERROR %s\n",error);
	throw String(error);// OMG works in WASI, kinda, via exceptions.cpp polyfill, BUT doesn't add anything lol
//	___cxa_throw(0,0,0);// absorbed in wasmer
	exit(0);
}
#endif
#endif
int MAX_MEM = 65536 * 1024;// todo lol
extern "C" double sqrt(double);

//unsigned long __stack_chk_guard = 0xBAAAAAAD;
//void __stack_chk_guard_setup(void) { __stack_chk_guard = 0xBAAAAAAD;/*provide some magic numbers*/ }
//void __stack_chk_fail(void) { /*log("__stack_chk_fail");*/} //  Error message will be called when guard variable is corrupted


void *alloc(int size, int num) {
	return calloc(size, num);
}


#ifdef WASM

//void *calloc(int size, int num) {// clean ('0') alloc
void *calloc(size_t num, size_t size) //__attribute__((__malloc__, __warn_unused_result__))
{
	char *mem =(char *) malloc(size * num);
//#ifndef WASM
	//fails in WASI, why??
	while (num<MAX_MEM and num > 0) { ((char *) mem)[--num] = 0; }
//#endif
	return mem;
}
#endif
// WOW CALLED INTERNALLY FROM C!!
//extern "C"

#ifdef WASM

// new operator for ALL objects
void *operator new[](size_t size) { // stack
	char *use = current;
	current += size;
	return use;
}

// new operator for ALL objects
void *operator new(size_t size) { // stack
	char *use = current;
	current += size;
	return use;
}
#endif

// WHY NOT WORKING WHEN IMPORTED? FUCKING MANGLING!
// bus error == access out of scope, e.g. logc((void*)-1000)
//void log(char* s) {
//#ifdef WASM
//	while(*s)logc(*s++);
//#else
//	printf("%s\n", s);
//#endif
//}

//void log(chars s) {
//#ifdef WASM
//	while(*s)logc(*s++);
//#else
//	printf("%s\n", s);
//#endif
//}


void _cxa_allocate_exception() {
	log("_cxa_allocate_exception!");
}

void _cxa_throw() {
	log("_cxa_throw");
	error("OUT OF MEMORY");
}


#ifndef WASM


//#import "Backtrace.cpp"
//#include "ErrorHandler.h"
#ifndef __APPLE__
//#include <alloc.h>
#endif

#include <cstdio>

//NEEDED, else terminate called without an active exception?
#endif


void error1(chars message, chars file, int line) {
#ifdef _Backtrace_
//	Backtrace(2);// later, in raise
#endif
	if (file)printf("\n%s:%d\n", file, line);\
    raise(message);
	if (panicking) panic();// not reached
}

void newline() {
	printf("\n");
}

void info(chars msg) {
	printf("%s", msg);
	newline();
}

void warn(chars warning) {
	printf("%s", warning);
	newline();
}

void warn(String warning) {
	printf("%s", warning.data);
	newline();
}

void warning(chars warning) {
	printf("%s", warning);// for now
}


int squari(int a) {
	return a * a;
}

// wasm has sqrt opcode, ignore √ in interpreter for now! cmath only causes problems, including 1000 on mac and log()
int sqrt1(int a) {
#ifndef WASM
//	return sqrt(a);
#endif
	todo("own sqrt");
	return -1;
}

void printf(int i) {
	printf("%d", i);
}


#ifdef WASI
String Backtrace(int skip, int skipEnd){
	return "Backtrace: TODO";
}
#endif

void *memmove0(char *dest, const char *source, size_t num) {
	while (num < MAX_MEM and --num >= 0)
		dest[num] = source[num];
	return dest;
// memmove will never return anything other than dest.  It's useful for chaining
}

#ifndef WASI

//defined in /opt/wasm/wasi-sdk/share/wasi-sysroot/lib/wasm32-wasi/libc.a
void *memmove(void *dest, const void *source, size_t num) {
	return memmove0((char *) dest, (char *) source, num);
}

#endif


void memcpy0(bytes dest, bytes source, int i) {
	while (i < MAX_MEM and --i >= 0)
		dest[i] = source[i];
}

void memcpy0(char *destination, char *source, size_t num) {
	if ((long) destination + num >= MEMORY_SIZE)return;
//		panic();
	if ((long) source + num >= MEMORY_SIZE)return;
//		panic();
	if (num < 0)return;
//		panic();
	while (--num < MAX_MEM and num >= 0) {
		destination[num] = source[num];
	}
}
//void * memcpy (void * destination, const void * source, size_t num ){
//	memcpy0((char *) destination, (char *) source, num);
//}

#ifdef WASM
#ifndef WASI
extern "C"
void *memcpy(void *destination, const void *source, size_t num) {
	memcpy0((char *) destination, (char *) source, num);
	return destination;// yes?
}
#endif
#endif

typedef struct wasi_buffer {
	const void *buf;
	size_t buf_len;
} wasi_buffer;


#ifdef WASI

#ifndef MY_WASM
// String.cpp
//extern "C" void logc(char c){
//	printf("%c" , c);
//}
//extern "C" void logi(int i) {
//	printf("%d", i);
//}
#endif
extern "C" int raise(chars error){
	printf("%s" , error);
	return -1;
}

//#include <stdlib.h>
void exit0(){
//	exit(0);
}

//wasm-ld: error: duplicate symbol: exit
//extern "C" void exit(int fd){
	// todo HOW??
// Error while importing "wasi_snapshot_preview1"."proc_exit": unknown import.
//}
#endif

//#ifdef RUNTIME_ONLY
#if defined(RUNTIME_ONLY) || defined(WASM)
//int read_wasm(chars wasm_path){return -1;};
//int run_wasm(chars wasm_path){return -1;};
//int run_wasm(bytes data,int size){return -1;}
#endif

#ifdef RUNTIME_ONLY_MOCK
// mock
Node analyze(Node data){return data;};// wasp -> code  // build ast via operators
Node eval(String code){return Node(code);};// wasp -> code -> data   // interpreter mode vs:
Node emit(String code){return Node(code);};//  wasp -> code -> wasm  // to debug currently same as:
//Node parse(String code) {};// wasp -> data  // this is the pure Wasp part
//Node run(String source){};// wasp -> code -> wasm() -> data
//Code &emit(Node root_ast, Module *runtime0, String _start){};
Node Node::evaluate(bool){ return *this; }
int read_wasm(chars wasm_path){};
int run_wasm(chars wasm_path){};
int run_wasm(bytes data,int size){}
#endif

#ifndef MY_WASM

void panic() {
#ifndef WASM
	raise("panic");
#else
	char* x=0;
	x[-1]=2;// Bus error: 10
#endif
}

#endif


#ifndef MY_WASM

//#pragma message "using wasm imports"
void logs(chars c) {
//	if(from wasm)result=c
	printf("%s", c);
}

void logi(int i) {
	printf("%d", i);
}

void logp(long char_pointer) {// workaround for m3, which can't link pointers:  od.link_optional<logs>("*", "logs")
	printf("%s", (char *) char_pointer);
}

void logc(char c) {
	printf("%c", c);
}

void logx(int i) {
	printf("%x", i);
}

//#undef logf  // doesn't help math
void logf32(float l) {
	printf("%f\n", l);
}

#endif

#ifdef RUNTIME_ONLY
#include "Interpret.h"
Module read_wasm(chars file){return *new Module();}
#endif

// todo: INLINE into wasm code how? just use wasp runtime and wasm-gc wasm-opt to tree shake ok
long powi(int a, int b) {
	int res = a;
	while (b-- > 1)
		res *= a;
	return res;
}
//number powl(number a, long b){// optimized for longs!
//	long c=a;
//	while (b-->0)c=c*a;
//	return c;
//}

//#ifndef SDL
//#ifndef WEBAPP
//long init_graphics();
//int paint(int wasm_offset){return -1;};
//#endif
//#endif