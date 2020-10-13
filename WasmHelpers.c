//#include <climits>
//
// Created by me on 15.07.20.
//

#include "WasmHelpers.h"
#define size_t int

extern unsigned int *memory;
//extern unsigned int *& __unused heap;

unsigned int *current;
extern void logs (const char *);
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
void* alloc(long size){
	current = memory;//heap;
	memory += size * 2 + 1;
	return current;
}
#endif


#ifdef WASM
void printf(const char *s) {
//	printf(s);
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