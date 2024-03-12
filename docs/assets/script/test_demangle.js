import {demangle} from "docs/script/demangle.js";

function assertEqual(a, b) {
    if (a !== b) {
        throw new Error(`Assertion failed: ${a} !== ${b}`);
    }
    console.log(`OK ${a} == ${b}`);
}

assertEqual(demangle("_ZN4ListI6StringE3addES0_"), "List<String>::add(String)");
assertEqual(demangle("_ZN6String7replaceEPKcS_"), "String::replace(const char*, String)");
// assertEqual(demangle("_ZN6String7replaceEPKcS_" ), "String::replace(char const*, String)");
assertEqual(demangle("_ZN6String7replaceEPKcS0"), "String::replace(const char*, const char*)");
// assertEqual(demangle("_ZN6String7replaceEPKcS0"), "String::replace(char const*, char const*)");
assertEqual(demangle("_ZNK6StringeqEPc"), "String::operator==(char*) const");
assertEqual(demangle("_ZNK4Node3hasE6Stringbs"), "Node::has(String, bool, short) const");
assertEqual(demangle("_ZN4NodegtES_"), "Node::operator>(Node)");
assertEqual(demangle("_ZNK6String5emptyEv"), "String::empty() const"); // ignore const
assertEqual(demangle("_Z8typeName4Kindb"), "typeName(Kind, bool)");
assertEqual(demangle("_ZN6String5beginEv"), "String::begin()");
assertEqual(demangle("_ZN4Node7setTypeE4Kindb"), "Node::setType(Kind, bool)");
// assertEqual(demangle("_ZNK4NodeixEi"), "Node::operator[](int) const");
assertEqual(demangle("_ZN4ListI6StringEC2Em"), "List<String>::List(unsigned long)");
// assertEqual(demangle("_Zli1sPKcm") , "operator\"\" s(char const*, unsigned long)");
assertEqual(demangle("_ZN6StringplES_"), "String::operator+(String)");
// assertEqual(demangle("_ZNK6String5emptyEv") , "String::empty() const");
assertEqual(demangle("_ZN6StringnwEm"), "String::operator new(unsigned long)");

assertEqual(demangle("_ZN6StringC2Ev"), "String::String()");
assertEqual(demangle("_ZN6StringC2EPh"), "String::String(unsigned char*)");
// assertEqual(demangle("_Z6printfPKcS0_i") , "printf(char const*, char const*, int)");
assertEqual(demangle("_Z6printfPKcS0_i"), "printf(const char*, const char*, int)");

// assertEqual(demangle("_ZN4ListI6StringEC2Em") , "List<String>::List(unsigned long)()");
// assertEqual(demangle("_Zli2_sPKcm") , "operator\"\"_s(char const*, unsigned long)");
assertEqual(demangle("foo"), "foo");
assertEqual(demangle("_ZN6StringC2EPcib"), "String::String(char*, int, bool)");
