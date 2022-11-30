#include "smart_types.h"

//enum
smartType4bit getSmartType(smart_pointer_32 spo) {
//	printf("%08x",spo);
//	char out[8];
//	sprintf((char*)&out,"%08x",spo);
	short byt = spo >> 28;
	smartType4bit ok = (smartType4bit) byt;
	return ok;
}


//enum smartType4bit getSmartType(int spointer){
//
//}
