#include "smart_types.h"

//enum
smartType getSmartType(spointer spo) {
//	printf("%08x",spo);
//	char out[8];
//	sprintf((char*)&out,"%08x",spo);
	short byt = spo >> 28;
	smartType ok = (smartType) byt;
	return ok;
}


//enum smartType4bit getSmartType(int spointer){
//
//}