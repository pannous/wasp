#pragma once
//
// Created by pannous on 15.07.20.
//
typedef const char *chars;
extern unsigned int *memory;
extern unsigned int *current;
//extern unsigned int *memory;
//extern unsigned int *& __unused heap;

#ifndef WASM
#include <cstdlib>
void* alloc(int size,int num);
#endif

#ifdef WASM
#define number int
#define size_t unsigned number
//#define size_t int

//TypeError: wasm function signature contains illegal type:
// ONLY INT and FLOAT in wasm functions!!!
extern "C" void exit(int code=0);
extern "C" void logc(char s);
extern "C" void logi(int i);
// helpers calling

void err(char const*);
void warn(char const*);
void log(char*);
void log(char c);
void log(chars s);

//void    *alloc(size_t __size) __result_use_check __alloc_size(1);
//char* alloc(number l);
void usleep(number l);
//extern "C" void print (const char *);// no \n newline
//extern "C" void logs (const char *);// can't work!
class Node;
class String;
void print(String);
void log(String *s);
void printf(Node&);
void printf(Node&){
	log("void printf(Node&);");
}
int isalnum ( int c );

void printf(const char *s);  //stdio
void print(const char *format, int i);
void printf(char const*, char const*);
void printf(char const *format, int i);
void printf(const char *format, chars i);
void printf(const char *format, chars i, int line);
void printf(const char *format, chars i, chars j);
void printf(const char *format, chars i, chars j, int l);
void printf(const char *format, chars i, chars j, chars k, int l);
//extern void* malloc(size_t __size);// __result_use_check __alloc_size(1);
//extern
//void* alloc(size_t __size);
//extern
void* alloc(int i, int num);
//void* calloc(int i);


void *malloc(size_t size){//}  __result_use_check __alloc_size(1){ // heap
	void* last = current;
	last = current;
	current += size * 2 + 1;
	return last;
}
void* alloc(int size, int num){
	void* last = current;
	last = current;
	current += size * 2 + 1;
	return last;
}


void *calloc(int size, int num) {// clean ('0') alloc
	void *mem = alloc(size,num);
	while (num > 0) { ((char *) mem)[--num] = 0; }
	return mem;
}
#endif