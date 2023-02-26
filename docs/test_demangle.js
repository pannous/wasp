import {demangle} from "./demangle.js";

function assertEqual(a, b) {
    if (a !== b) {
        throw new Error(`Assertion failed: ${a} !== ${b}`);
    }
    console.log(`OK ${a} == ${b}`);
}

// assertEqual(demangle("_ZNK4NodeixEi"), "Node::operator[](int) const");
assertEqual(demangle("_ZNK6String5emptyEv"), "String::empty() const"); // ignore const
assertEqual(demangle("_ZN4ListI6StringEC2Em"), "List<String>::List(unsigned long)");
// assertEqual(demangle("_Zli1sPKcm") , "operator\"\" s(char const*, unsigned long)");
assertEqual(demangle("_ZN6StringplES_"), "String::operator+(String)");
// assertEqual(demangle("_ZNK6String5emptyEv") , "String::empty() const");
assertEqual(demangle("_ZN6StringnwEm"), "String::operator new(unsigned long)");

assertEqual(demangle("_ZN6StringC2Ev"), "String::String()");
// assertEqual(demangle("_Z6printfPKcS0_i") , "printf(char const*, char const*, int)");
assertEqual(demangle("_Z6printfPKcS0_i"), "printf(const char*, const char*, int)");

// assertEqual(demangle("_ZN4ListI6StringEC2Em") , "List<String>::List(unsigned long)()");
// assertEqual(demangle("_Zli2_sPKcm") , "operator\"\"_s(char const*, unsigned long)");
assertEqual(demangle("foo"), "foo");
assertEqual(demangle("_ZN6StringC2EPcib"), "String::String(char*, int, bool)");
