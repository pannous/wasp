#include "smart_types.h"

#include "Node.h"

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

// smart pointers returned if ABI does not allow multi-return, as in int main(){}
Node smartNode32(int smartPointer32) {
    auto smart_pointer = smartPointer32 & 0x00FFFFFF; // data part
    if ((smartPointer32 & 0xF0000000) == array_header_32 /* and abi=wasp */) {
        // smart pointer to smart array
        int *index = ((int *) wasm_memory) + smart_pointer;
        int kind = *index++;
        if (kind == array_header_32)
            kind = *index++;
        int len = *index++; // todo: leb128 vector later
        Node arr = Node();
        //		arr.kind.value = kind;
        int pos = 0;
        while (len-- > 0) {
            auto val = index[pos++];
            arr.add(new Node(val));
        }
        arr.kind = objects;
        //			check_eq(arr.length,len);
        //			check(arr.type=…
        return arr;
    }
    if ((smartPointer32 & 0xFF000000) == string_header_32 /* and abi=wasp */) {
        // smart pointer for string
        return Node(((char *) wasm_memory) + smart_pointer);
    }
    error1("missing smart pointer type "s + typeName(Type(smartPointer32)));
    return Node();
}


