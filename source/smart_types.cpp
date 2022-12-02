#include "smart_types.h"

smartType4bit getSmartType(smart_pointer_32 spo) {
	short byt = spo >> 28;
	smartType4bit ok = (smartType4bit) byt;
	return ok;
}

smartType4bit getSmartType(smart_pointer_64 spo) {
    short byt = spo >> 60;
    smartType4bit ok = (smartType4bit) byt;
    return ok;
}
