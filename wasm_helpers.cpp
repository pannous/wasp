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
//#define size_t int
//extern unsigned int *memory;

//#ifdef WASI
//#include <stdio.h> // printf
//#endif


#ifdef WASM

void free(void*){/*lol*/}


void *malloc(size_t size){//}  __result_use_check __alloc_size(1){ // heap
	void* last = current;
	last = current;
	current += size * 2 + 1;
	return last;
}

void log(chars s) {
	logs(s);
}
void log(char *s) {
	logs(s);
//	while(*s)logc(*s++);
	logc('\n');
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
void printf(chars format,long i,long j){
	print(String(format).replace("%d", String(i)).replace("%d", String(j)));
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
int isalnum ( int c ){
	return c>20;// todo lol
}
#endif
//#define __cxa_allocate_exception 1
//#define __cxa_throw 1
//#include <typeinfo>       // operator typeid
#ifndef WASM
#ifndef WASI
extern bool throwing;// false for error tests etc
void raise(chars error) {
	if (throwing) throw error;
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

void *alloc(int size, int num) { return calloc((size + 1), (num)); }

void *calloc(int size, int num) {// clean ('0') alloc
	void *mem = malloc(size * num);
	while (num > 0) { ((char *) mem)[--num] = 0; }
	return mem;
}

// WOW CALLED INTERNALLY FROM C!!
//extern "C"
void *memset(void *ptr, int value, size_t num) {
	int *ptr0 = (int *) ptr;
	for (int i = 0; i < num; i++)
		ptr0[i] = value;
	return ptr;
}

// new operator for ALL objects
void *operator new[](size_t size) { // stack
	current = memory;
	memory += size;
	return current;
}

// new operator for ALL objects
void *operator new(size_t size) { // stack
	current = memory;
	memory += size;
	return current;
}

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

//int square(int n){
//	return n * n;
//}
void logi(int l) {
	printf("%d\n", l);
}

void log_f32(float l) {
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


void error1(chars message, chars file, int line) {
	//#ifdef _Backtrace_
	Backtrace(2);
//#endif
	if (file)
		printf("%s:%d\n", file, line);
	raise(message);
//	err(error);
}

void newline() {
	printf("\n");
}

void info(chars msg) {
	printf("%s",msg);
	newline();
}

void warn(chars warning) {
	printf("%s",warning);
	newline();
}

void warn(String warning) {
	printf("%s",warning.data);
	newline();
}

void warning(chars warning) {
	printf("%s",warning);// for now
}


int square(int a) {
	return a * a;
}

int sqrt1(int a) {
#ifndef WASM
	return sqrt(a);
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


void memcpy0(bytes dest, bytes source, int i) {
	while (i--)dest[i] = source[i];
}

void memcpy0(char *destination, char *source, size_t num) {
	while (--num >= 0)destination[num] = source[num];
}
//void * memcpy (void * destination, const void * source, size_t num ){
//	memcpy0((char *) destination, (char *) source, num);
//}

extern "C"
void *memcpy(void *destination, const void *source, size_t num) {
	memcpy0((char *) destination, (char *) source, num);
	return destination;// yes?
}

typedef struct wasi_buffer {
	const void *buf;
	size_t buf_len;
} wasi_buffer;

void logs(chars s) { // works in wasmer and wasmtime!
#ifndef WASM
	return; // this should ONLY be called in wasm context!
#endif
//#ifdef WASI
//	printf(s);
//#else // fake WASI … OR use wasmx import logs
	size_t len = strlen0(s);
	wasi_buffer buf = {s, len};
	size_t out;
	fd_write(1, &buf, len, &out);
//#endif
}


#ifdef WASI



extern "C" void logc(char c){
	printf("%c" , c);
}

extern "C" void raise(chars error){
	printf("%s" , error);
}


//extern "C" void exit(int fd){
	// todo HOW??
// Error while importing "wasi_snapshot_preview1"."proc_exit": unknown import.
//}
#endif