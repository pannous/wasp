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
	while(*s)logc(*s++);
	logc('\n');
}
void log(char *s) {
	while(*s)logc(*s++);
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
void printf(const char *s) {
	while(*s)logc(*s++);
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

#ifndef WASM
void* alloc(int size,int num) { return malloc(size*num);}
#endif

