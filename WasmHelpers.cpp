//#pragma once
//#include <climits>
//
// Created by pannous on 15.07.20.
//

#include "WasmHelpers.h"
#include "String.h"
#define size_t int
//extern unsigned int *memory;

extern void logs (const char *,int len=-1 /*auto*/);
extern void logc(char s);
extern void logi(int i);


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

void printf(const char *format, void* value) {
#ifndef WASM
	print(String(format).replace("%p", String((number)value)));
#else
//	memory-value
	print(String(format).replace("%p", 0));// TODO
#endif

}
#endif

void* alloc(int size) { return malloc(size);}
