#pragma once
//#include <climits>
//
// Created by pannous on 15.07.20.
//

#include "WasmHelpers.h"
#include "String.h"
#include "Backtrace.h"
//#define size_t int
//extern unsigned int *memory;


#ifdef WASM

void *malloc(size_t size){//}  __result_use_check __alloc_size(1){ // heap
	void* last = current;
	last = current;
	current += size * 2 + 1;
	return last;
}

void * memcpy ( char * destination, char * source, size_t num ){
	while(--num>=0)destination[num] = source[num];
}
void * memcpy ( void * destination, const void * source, size_t num ){
	memcpy((char *) destination, (char *) source, num);
}

void log(chars s) {
	logs(s);
}
void log(char *s) {
	logs(s);
//	while(*s)logc(*s++);
//	logc('\n');
}

void log(char c) {
	logc(c);
}
void log(int i) {
	logi(i);
}
void log(long c) {
	logi((int)c);
}
void println(String s){
	print(s);
	logc('\n');
}
void printf(const char *s) {
	logs(s);
//	while(*s)logc(*s++);
}
void print(String s){
		log(s.data);
//	logs(s.data,s.length)
}
void printf(const char *format, int i) {
	print(String(format).replace("%d", String(i)).replace("%i", String(i)).replace("%li", String(i)));
}
#ifndef WASM
void printf(const char *format, number i) {
	print(String(format).replace("%d", String(i)).replace("%i", String(i)).replace("%li", String(i)));
}
#endif
void printf(const char *format, chars value) {
	print(String(format).replace("%s", value));
}
void printf(const char *format, chars i, chars j){
	print(String(format).replace("%s", i).replace("%s", j));
}

void printf(const char *format, chars i, chars j, int l){
	print(String(format).replace("%s", i).replace("%s", j).replace("%d", String(l)));
}

void printf(const char *format, const char *val, int value) {
	print(String(format).format((char*)val).format(value));
}

void printf(const char *format, void* value) {
#ifndef WASM
	print(String(format).replace("%p", String((number)value)));
#else
//	memory-value
	print(String(format).replace("%p", 0));// TODO
#endif

}
int isalnum ( int c ){
	return c>20;// todo lol
}
#endif
//#define __cxa_allocate_exception 1
//#define __cxa_throw 1
//#include <typeinfo>       // operator typeid
#ifndef WASM
#ifndef WASI
void raise(chars error){
//#ifdef _Backtrace_
	Backtrace(3);
//#endif
	throw error;
}
#else
extern "C" void ___cxa_throw(
		void* thrown_exception,
		struct type_info *tinfo,
		void (*dest)(void*));
void raise(chars error){
//	throw error;//  typeinfo for char const* yadiya
//	_raise(error);
	printf("ERROR %s\n",error);
	throw String(error);// OMG works in WASI, kinda, via exceptions.cpp polyfill, BUT doesn't add anything lol
//	___cxa_throw(0,0,0);// absorbed in wasmer
	exit(0);
}
#endif
#endif
void* alloc(int size,int num) { return calloc((size+1),(num));}
void *calloc(int size, int num) {// clean ('0') alloc
	void *mem = alloc(size,num);
	while (num > 0) { ((char *) mem)[--num] = 0; }
	return mem;
}

typedef unsigned long size_t;


// WOW CALLED INTERNALLY FROM C!!
//extern "C"
void * memset ( void * ptr, int value, size_t num ){
	int* ptr0=(int*)ptr;
	for (int i = 0; i < num; i++)
		ptr0[i]=value;
	return ptr;
}
//void* operator new[](size_t size){ // stack
//	current=memory;
//	memory+=size;
////	logs("new[]");
////	logi((number)current);
//	return current;
//}
//void* operator new(size_t size){ // stack
//	current=memory;
//	memory+=size;
////	logs("new");
////	logi((number)current);
//	return current;
//}

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


void _cxa_allocate_exception(){
	log("_cxa_allocate_exception!");
}

void _cxa_throw(){
	log("_cxa_throw");
	error("OUT OF MEMORY");
}


#ifndef WASM
//int square(int n){
//	return n * n;
//}
void logi(int l){
	printf("%d\n", l);
}
void log_f32(float l){
	printf("%f\n", l);
}

//#import "Backtrace.cpp"
//#include "ErrorHandler.h"
#ifndef __APPLE__
//#include <alloc.h>
#endif

#include <cstdio>

//NEEDED, else terminate called without an active exception?
#endif

void err(chars error) {
	raise(error);
}

void error(chars error) {
	printf("ERROR\n");
	printf(error);
	printf("\n");
	raise(error);
//	err(error);
}
void newline(){
	printf("\n");
}
void warn(chars warning) {
	printf(warning);
	newline();
}

void warning(chars warning) {
	printf(warning);// for now
}

