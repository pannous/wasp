#pragma once
//#include <climits>
//
// Created by pannous on 15.07.20.
//

#include "WasmHelpers.h"
#include "String.h"
#define size_t int

extern unsigned int *memory;
//extern unsigned int *& __unused heap;

unsigned int *current;
extern void logs (const char *,int len=-1 /*auto*/);
extern void logc(char s);
extern void logi(int i);
#ifndef _MALLOC_UNDERSCORE_MALLOC_H_ // ;)
void *malloc(size_t size){//}  __result_use_check __alloc_size(1){ // heap
//	logs("malloc");
//	logi((long)current);
	current = memory;//heap;
	memory += size * 2 + 1;
	return current;
}
void* alloc(int size){
	current = memory;//heap;
	memory += size * 2 + 1;
	return current;
}

void *calloc(int i) {// clean ('0') alloc
	void *mem = alloc(i);
	while (i > 0) { ((char *) mem)[--i] = 0; }
	return mem;
}
#endif


#ifdef WASM
void log(chars s) {
	while(s++)logc(s[0]);
}
void log(char *s) {
	while(s++)logc(s[0]);
}
void printf(const char *s) {
	while(s++)logc(s[0]);
}
void print(String s){
		logs(s.data);
//	logs(s.data,s.length)
}
void printf(const char *format, int i) {
	print(String(format).replace("%d", String(i)).replace("%i", String(i)).replace("%li", String(i)));
}
void printf(const char *format, long i) {
	print(String(format).replace("%d", String(i)).replace("%i", String(i)).replace("%li", String(i)));
}
void printf(const char *format, chars value) {
	print(String(format).replace("%s", value));
}

void printf(const char *format, void* value) {
	print(String(format).replace("%p", String((long)value)));
}
#endif
