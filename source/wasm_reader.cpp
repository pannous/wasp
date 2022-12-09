//
// Created by me on 27.11.20.
//
#include "Code.h"
#include <cstdio>
#include "wasm_reader.h"
#include "Util.h"
//#include "wasm_emitter.h"
// compare with wasm-objdump -h

#define POLYMORPH_function_index_marker -2
bool debug_reader = tracing;//true;// tracing;
typedef unsigned char *bytes;
int pos = 0;
int size = 0;
byte *code;

// https://webassembly.github.io/spec/core/binary/modules.html#sections

//Module& module=*new Module();
//extern Map<String, int> module->functionIndices;// todo: use function[String].index
//extern List<String> declaredFunctions; only new functions that will get a Code block, no runtime/imports


Valtype mapArgToValtype(String &arg);

#define consume(len, match) if(!consume_x(code,&pos,len,match)){if(debug_reader)printf("\nNOT consuming %s\n%s:%d\n",#match,__FILE__,__LINE__);exit(0);}

bool consume_x(byte *code, int *pos, int len, byte *bytes) {
    if (*pos + len > size)
        error("END OF FILE");
    if (not bytes) {
        *pos = *pos + len;
        return true;
    }

    int i = 0;
    while (i < len) {
        if (bytes and code[*pos] != bytes[i])
            return false;
        *pos = *pos + 1;
        i++;
    }
    return true;
}

Sections typ() {
    return (Sections) code[pos++];
}

int unsignedLEB128() {
    int n = 0;
    short shift = 0;
    do {
        byte b = code[pos++];
        n = n | (((int) (b & 0x7f)) << shift);
        if ((b & 0x80) == 0)
            break;
        shift += 7;
    } while (pos < size);
    return n;
}

// DANGER: modifies the start reader position of code, but not it's data!
long unsignedLEB128(Code &byt) {
    long n = 0;
    short shift = 0;
    do {
        byte b = byt.data[byt.start++];
        n = n | (((long) (b & 0x7f)) << shift);
        if ((b & 0x80) == 0)break;// no more continuation
        shift += 7;
    } while (byt.start < byt.length);
    return n;
}

int consumeByte(Code &byt) {
    return byt.data[byt.start++];
}

int siz() {
    return unsignedLEB128();
}

Code vec() {
    int len = siz();
    int from = pos;
    consume(len, 0);
//	return Code(code+from, from, pos);
    return Code(code + from, pos - from);
}

Code vec(Code &data, bool consume = true) {
    int len = unsignedLEB128(data);
    Code code1 = Code(data.data + data.start, len);
    if (consume) data.start += len;
    else data.start -= 1;// undo len read
    return code1;
}

String &name(Code &wstring) {// Shared string view, so don't worry about trailing extra chars
    long len = unsignedLEB128(wstring);
    auto nam = (char *) wstring.data + wstring.start;
//    while(nam[0]<=33)nam++;// WTH! hiding strange bug where there is a byte behind unsignedLEB128. NOT FULLY consumed
    // // BUG SINC 2022-12-09 ~16-17pm
    String *string = new String(nam, len, true);
    wstring.start += len;// advance internally
//	if (len > 40)put(string);
    return *string;
}

// todo: treat all functions (in library file) as exports if ExportSection is empty !?
// that is: add their signatures to module …
void parseFunctionNames(Code &payload) {
    module->functionIndices.setDefault(-1);
//	module->functions.setDefault(Function());
//	put(module->functionIndices);// what we got so far?
    int function_count = unsignedLEB128(payload);
    int index = -1;
    for (int i = 0; i < function_count and payload.start < payload.length; ++i) {
        index = unsignedLEB128(payload);
        if (index < 0 or index > 100000)
            error("broken index"s + index);
        String func = name(payload).clone();// needs to be 0-terminated now
        Function &function = module->functions[func];
        function.index = index;
        function.name = func;
        if (debug_reader)printf("ƒ%d %s\n", index, func.data);
        if (module->functionIndices[func] > 0 and module->functionIndices[func] < function_count /*hack!*/) {
            if (module->functionIndices[func] == index)
                continue; // identical match, lib parsed twice without cleaning module->functionIndices!?
            // export section ≠ name section (when overloading…)
            trace("already has index: "s + func + " " + module->functionIndices[func] + "≠" + index);
            continue;
            func = func + "_func_" + index;// hack ok to avoid duplicates
            func = func.clone();
        }
        if (func.length > 0)
//			module->functionIndices.insert_or_assign(func, index);
            module->functionIndices[func] = index;
        else {
            error("function without name at index "s + index);// happens with unicode π(x) etc
            //			warn
            module->functionIndices["func_"s + index] = index;
//			module->functionIndices.insert_or_assign("func_"s + index, index);
        }
    }
//	  (import "env" "log_chars" (func (;0;) $logs (type 0)))  export / import names != internal names
//	for (int i = function_count; i < module->total_func_count; i++)
//		module->functionIndices.insert_or_assign("unnamed_func_"s + i, i);

}

//Map<int, Signature> funcTypes;
// import type indices are part of import struct!
void parseFuncTypeSection(Code &payload) {
    // imports stupidly have their own type,
    // we don't know here if i32 is pointer … so we may have to refine later
    for (int i = 0; i < module->code_count and payload.start < payload.length; ++i) {
        auto code_index = (int) (i + module->import_count); // implicit !?
        int typ = unsignedLEB128(payload);
        Function &function = module->functions.values[i];
        Signature &s = module->funcTypes[typ];
        function.signature.merge(s);
    }
}

// not part of name section wtf
void parseImportNames(Code &payload) {// and TYPES!
    trace("Imports:");
    for (int i = 0; i < module->import_count and payload.start < payload.length; ++i) {
        String &mod = name(payload);// module
        String &name1 = name(payload);// shared is NOT 0 terminated, needs to be 0-terminated now?
        trace("import %s"s % mod + "." + name1);
        int kind = unsignedLEB128(payload);// func, global, …
        int type = unsignedLEB128(payload);// i32 … for globals
        if (kind == 3 /*global import*/ ) {
            bool is_mutable = unsignedLEB128(payload);
            Global global{i, name1, (Valtype) type, is_mutable, true, false};
            module->globals.add(global);
            continue;
        }
        Signature &signature = module->funcTypes[type];
        module->functionIndices[name1] = i;
        module->functions[name1].signature.merge(signature);
        Signature &sic = getSignature(name1);// global!
        sic.merge(signature);// todo : LATER!
        module->import_names.add(name1);
    }
//	module->signatures = functionSignatures; // todo merge into global functionSignatures, not the other way round!!
}

// Signatures to be consumed in export section
void parse_type_data(Code &payload) {
    for (int i = 0; i < module->type_count and payload.start < payload.length; ++i) {
        Signature sic;
        sic.type_index = i;
        int typ = unsignedLEB128(payload);// implicit?
        if (typ != func)continue;
        int param_count = unsignedLEB128(payload);
        for (int j = 0; j < param_count; j++) {
            Valtype argt = (Valtype) unsignedLEB128(payload);
            sic.add(argt);
        }
        int return_count = unsignedLEB128(payload);
        if (return_count) {// todo: multi-value
            Valtype rt = (Valtype) unsignedLEB128(payload);
            sic.returns(rt);
        } else
            sic.returns(none);
        module->funcTypes.add(sic);
    }
}

void consumeTypeSection() {
    Code type_vector = vec();
    int typeCount = unsignedLEB128(type_vector);
    module->type_count = typeCount;
    if (debug_reader)printf("types: %d\n", module->type_count);
    module->type_data = type_vector.rest();
// todo: we need to parse this for automatic import and signatures
    parse_type_data(module->type_data);
//    parseFuncTypeSection(); LATER connect implicit func index with type index (and names)
}

void consumeStartSection() {
    module->start_index = unsignedLEB128();
    if (debug_reader)printf("start: #%d \n", module->start_index);
}

void consumeTableSection() {
    module->table_data = vec();
    module->table_count = 1;// unsignedLEB128(module->table_data);
    if (debug_reader)printf("tables: %d \n", module->table_count);
}

void consumeMemorySection() {
    module->memory_data = vec();// todo ?
//	module->memory_count = unsignedLEB128(module->memory_data);//  always 1 in MVP
    if (debug_reader)printf("memory_data: %d\n", module->memory_data.length);
}

void consumeGlobalSection() {
    module->globals_data = vec();// todo
    module->global_count = unsignedLEB128(module->globals_data); // NO SUCH THING!?
    if (debug_reader)printf("globals: %d\n", module->global_count);
}

void consumeNameSection(Code &data) {
    if (debug_reader)print("names: …\n");
    module->name_data = data.clone();
    while (data.start < data.length) {
        int type = unsignedLEB128(data);
        Code payload = vec(data);// todo test!
        switch (type) {
            case module_name: {
                module->name = name(payload);// wrapped in vector why?
            }
                break;
            case function_names:
                module->function_names = payload;
                parseFunctionNames(payload.clone());
                break;
            case local_names:
                module->local_names = payload;
                break;
            case global_names:
                module->global_names = payload;
                break;
            case todo_idk: todow("NAME TYPE 9");
                break;
            default:
                error("INVALID NAME TYPE "s + type);
        }
    }
}


// https://github.com/WebAssembly/tool-conventions/blob/master/Linking.md#linking-metadata-section
void consumeLinkingSection(Code &data) {
    if (debug_reader)print("linking: …\n");
    module->linking_section = data;
//	int version = unsignedLEB128(data);
}

void consumeRelocateSection(Code &data) {
    if (debug_reader)print("relocate: …\n");
    module->relocate_section = data;
}

void consumeDataSection() {
    Code datas = vec();
//	module->data_section = datas.clone();
    module->data_segments_count = unsignedLEB128(datas);
    module->data_segments = datas.rest();// whereever the start may be now
//	short memory_id =
    unsignedLEB128(datas);
    unsignedLEB128(datas);// skip i32.const opcode
    int data_offset = unsignedLEB128(datas);
    unsignedLEB128(datas);// skip '0b' whatever that is
    module->data_offset_end = data_offset + module->data_segments.length + 100;//  datas.length;
    // Todo ILL-DEFINED BEHAVIOUR! silent corruption if data_offset_end too small (why 100?)

//	if (debug_reader)
//printf("data sections: %d from offsets %d to %d \n", module->data_segments_count, data_offset,module->data_offset_end);
//	if(debug_reader)printf("data section offset: %ld \n", offset);
}

void consumeElementSection() {
    module->element_section = vec();
//	if(debug_reader)printf("element section (!?)");
    if (debug_reader)printf("element sections: %d \n", module->element_section.length);
}

void consumeCustomSection() {
    Code customSectionDatas = vec();
    String type = name(customSectionDatas);
    Code payload = customSectionDatas.rest();
    if (type == "names" or type == "name") {
        consumeNameSection(payload);
    } else if (type == "target_features")
//	https://github.com/WebAssembly/tool-conventions/blob/main/Linking.md#target-features-section
// 	atomics bulk-memory exception-handling multivalue mutable-globals nontrapping-fpoint sign-ext simd128 tail-call
    todow("target_features detection not yet supported")
    else if (type == "linking")
        // see https://github.com/WebAssembly/tool-conventions/blob/main/Linking.md
        consumeLinkingSection(payload);
    else if (type == "dylink.0")
        // see https://github.com/WebAssembly/tool-conventions/blob/main/DynamicLinking.md
    todow("dynamic linking not yet supported")
    else if (type.startsWith("reloc."))
        consumeRelocateSection(payload);// e.g. "reloc.CODE"
        // everything after the period is ignored and the specific target section is encoded in the reloc section itself.
        // see https://github.com/WebAssembly/tool-conventions/blob/main/Linking.md
    else if (type == "relocate")
        consumeRelocateSection(payload);
    else {
//		pos = size;// force finish
        todow("consumeCustomSection not implementated for "s + type);
        customSectionDatas.start = 0;// reset

//		TODO REENABLE!! currently causes SIGSEGV
//		module->custom_sections.add(customSectionDatas);// raw
    }
}


// connect func/code indices to type indices
void consumeFuncTypeSection() {
    Code type_vector = vec();
    module->code_count = unsignedLEB128(type_vector);// import type indices are part of import struct!
    if (debug_reader)printf("signatures: %d\n", module->code_count);
    module->functype_data = type_vector.rest();
    Code &funcs_to_types = module->functype_data.clone();
    for (int i = 0; i < module->code_count; ++i) {
        int type = unsignedLEB128(funcs_to_types);
        module->funcToTypeMap[i] = type;
//        printf("func code_index %d => type %d\n", i, type);
    }
    // parsed AGAIN later in parseFuncTypeSection to connect names . todo unnecessary?
}

// todo: treat all functions (in library file) as exports if ExportSection is empty !?
void consumeCodeSection() {
    Code codes_vector = vec();
    int codeCount = unsignedLEB128(codes_vector);
    if (debug_reader)printf("codes: %d\n", codeCount);
    if (module->code_count != codeCount)error("missing code/signatures");
    module->code_data = codes_vector.rest();
    if (debug_reader)printf("code length: %d\n", module->code_data.length);
}


// todo ifdef CPP not WASM(?)
#include <cxxabi.h> // for abi::__cxa_demangle

void fixupGenerics(char *s, int len) {
    int bra = 0;
    for (int i = 0; i < len; ++i) {
        if (s[i] == '<')bra++;
        if (s[i] == '>')bra--;
        if (bra and s[i] == ',')s[i] = ';';
    }
}

// we can reconstruct arguments from demangled exports or retained wast names
// _Z2eqPKcS0_i =>  func $eq_char_const*__char_const*__int_ <= eq(char const*, char const*, int)
List<String> demangle_args(String &fun) {
    int status;
//	String *real_name = new
//"print(Map<String, int>)"
    char *string = abi::__cxa_demangle(fun.data, 0, 0, &status);
    if (status != 0 or string == 0)return 0;
    String real_name = String(string);
    fixupGenerics(real_name.data, real_name.length);
    if (!real_name or !real_name.contains("("))return 0;
    String brace = real_name.substring(real_name.indexOf('(') + 1, real_name.indexOf(')'));//.clone();
    if (brace.contains("("))
        return 0;// function pointers not supported yet "List<String>::sort(bool (*)(String&, String&))"
    return brace.split(", ");
}


// todo: treat all functions (in library file) as exports if ExportSection is empty !?
// https://webassembly.github.io/spec/core/binary/modules.html#binary-exportsec
void consumeExportSection() {
    Code exports_vector = vec();
    int exportCount = unsignedLEB128(exports_vector);
    if (debug_reader)printf("export_section: %d\n", exportCount);
    module->export_count = exportCount;
    module->export_data = exports_vector.rest();
    Code &payload = module->export_data;
    for (int i = 0; i < exportCount; i++) {
        String export_name = name(payload);
        int export_type = unsignedLEB128(payload); // don't confuse with function_type if export_type==0
        int index = unsignedLEB128(payload);// for all types!
        if (export_type == 3 /*global*/) continue; // todo !?
        if (export_type == 2 /*memory*/) continue;
        if (export_type == 1 /*table*/) continue;
        if (export_type != 0 /*function export*/) {
            error("UNKNOWN export_type "s + export_type + " for " + export_name + " at " + index);
            continue;
        }
        String &func0 = export_name;
        String func = extractFuncName(func0);
        if (index < 0 or index > 100000)error("corrupt index "s + index);
        int code_index = index + module->import_count;

        if (func0 == "_Z5main4iPPc")
            continue;// don't make libraries 'main' visible, use own
        int status = 0; // for debugging:
        String demangled = abi::__cxa_demangle(func0.data, 0, 0, &status);// function name and cpp args but not return

        Function &fun = module->functions[func];// demangled
        Function &fun0 = module->functions[func0];// mangled
        // ⚠️ CAN BE THE SAME REFERENCE IF func==func0 !!! ⚠️

        fun.name = func;
        fun0.name = func0;

        if (debug_reader) {
            printf("ƒ%d %s ≈\n", index, func0.data);
            printf("ƒ%d %s ≈\n", index, demangled.data);
        }
//        if(fun.name=="puts")
//            breakpoint_helper

        if (fun.signature.size()) {
            trace("function %s already has signature "s % func + fun.signature.serialize());
            trace("function %s old index %d new index %d"s % func % fun.index % index);
            Function &abstract = *new Function{.name=func, .module=module, .is_runtime=true, .is_polymorph=true};
            abstract.variants.add(fun);
            module->functions[func] = abstract;
            fun = abstract.variants.items[2];
            fun = *new Function{.name=func, .module=fun.module, .is_runtime=true};
            module->functionIndices[func] = POLYMORPH_function_index_marker;
        } else {
            module->functionIndices[func] = code_index;// demangled
        }
        module->functionIndices[func0] = code_index;// mangled unique
        fun0.index = code_index;
        fun.index = code_index;


//        if (code_index == 369 or index == 369)
//            breakpoint_helper // todo operator+

        int funcType = module->funcToTypeMap[index];
        // todo: demangling doesn't yield return type, is wasm_signature ok?
        fun.signature.type_index = funcType;
        fun0.signature.type_index = funcType;// todo: remove duplicate, try fun0.signature=fun.signature at end again
        if (not(0 <= funcType and funcType <= module->funcTypes._size))
            check_silent(0 <= funcType and funcType <= module->funcTypes._size)
        Signature &wasm_signature = module->funcTypes[funcType];
        Valtype returns = mapTypeToWasm(wasm_signature.return_types.last(Kind::undefined));
        if (wasm_signature.wasm_return_type == void_block) returns = void_block;
        fun.signature.returns(returns);
        if (&fun != &fun0)
            fun0.signature.returns(returns);

        // todo: use wasm_signature if demangling fails, see merge(signature) below

        if (demangled.contains("::")) {
            String typ = demangled.to("::");
            auto type = mapArgToType(typ);// Primitive::self
            fun.signature.add(type, "self");
        }
// e.g. List<String>::add (String) has one arg, but wasm signature is (i32,i32):i32  ["_ZN4ListI6StringE3addES0_"]
// todo: demangle further and put into multi-dispatch

        List<String> args = demangle_args(func0);
        for (String &arg: args) {
            if (arg.empty())continue;
            fun.signature.add(mapArgToType(arg));
            if (&fun != &fun0)
                fun0.signature.add(mapArgToType(arg));
        }
        // can't after free
//        if (&fun != &fun0)
//            fun0.signature = fun.signature;// ⚠️ SHARING deep list fields now. this causes problems later:
//        fun0.signature = fun.signature.clone();// todo copy by value ok? NO: heap-use-after-free on address
        if (debug_reader) {
            const String &argos = args.join(",");
//            printf("ƒ%d %s(%s) ≈\n", index, func0.data, string);
            char *sig = fun.signature.serialize().data;
            printf("ƒ%d %s(%s)\n", index, func.data, argos.data);
            printf("ƒ%d %s%s\n", index, func.data, sig);
        }

    }
}


Type mapArgToType(String arg) {
//	if(arg=="const char*")return charp;
    if (arg.empty() or arg == "" or arg == " ") return voids;
    else if (arg == "unsigned char*")return charp;// pointer with special semantics
    else if (arg == "char const*")return charp;// pointer with special semantics
    else if (arg == "char const*&")return charp;// todo ?
    else if (arg == "char*")return charp;
    else if (arg == "char32_t*")return codepoints; // ≠ codepoint todo, not exactly: WITHOUT HEADER!
    else if (arg == "char const**")return pointer;
    else if (arg == "short")
        return int32;// vec_i16! careful c++ ABI overflow? should be fine since wasm doesnt have short
    else if (arg == "int")return int32;
    else if (arg == "signed int")return i32s;
    else if (arg == "unsigned int")return i32;
    else if (arg == "unsigned char")return int32;
    else if (arg == "int*")return pointer;
    else if (arg == "void*")return pointer;
    else if (arg == "long")return i64;
    else if (arg == "long&")return pointer;
    else if (arg == "long long")return i64;
    else if (arg == "unsigned long long")return i64;
    else if (arg == "double")return float64;
    else if (arg == "unsigned long")return i64;
    else if (arg == "float")return float32;
    else if (arg == "bool")return int32;
    else if (arg == "char")return int32;// c++ char < angle codepoint ok
    else if (arg == "wchar_t")return (Valtype) codepoint32;// angle codepoint ok
    else if (arg == "char32_t")return codepoint32;// angle codepoint ok
    else if (arg == "char16_t")return codepoint32;// !? ⚠️ careful
    else if (arg == "char**")return pointer;// to chars
    else if (arg == "short*")return pointer;


        // Some INTERNAL TYPES are reflected upon / exposed as abi :
    else if (arg == "Type")return int32;// enum
    else if (arg == "Kind")return int32;// enum (short, ok)
    else if (arg == "Type")return type32;// enum

    else if (arg == "String*")return stringp;
    else if (arg == "String&")return stringp;// todo: how does c++ handle refs?
    else if (arg == "String")return string_struct;

    else if (arg == "Node")return node;// struct!

    else if (arg == "Node&")return nodes;// pointer? todo: how does c++ handle refs?
    else if (arg == "Node const&")return nodes;
    else if (arg == "Node const*")return nodes;
    else if (arg == "Node*")return nodes;

    else if (arg == "Type32")return type32;
    else if (arg == "Type")return type32;
    else if (arg == "Kind")return type32;
    else if (arg == "Primitive")return type32;// good thing wasm has no polymorphism
    else if (arg == "Valtype")return type32;// good enough!
    else if (arg == "Type64")return ignore; // for now   return smarti64;
    else if (arg == "Type64::Type64")return ignore; // for now   return smarti64;
    else if (arg == "Type&")error("Type should only be used as value");

    else if (arg == "List<String>")return list;
    else if (arg == "List<Type>")return list;
    else if (arg == "List<int>")return list;

    else if (arg == "Module")return modul;
    else if (arg == "Module const&")return modul;

    else if (arg == "SyntaxError")return errors;
//    else if (arg == "SyntaxError")return result_error;

    else if (arg == "std::is_arithmetic<int>::value")return todoe;// WAT?? PURE_WASM should work without std!!


        // IGNORE other INTERNAL TYPES:
    else if (arg == "Code")return ignore;
//    else if (arg == "Map<String")return ignore;
//    else if (arg == "int>")return ignore;// parse bug ^^
    else if (arg == "Function")return ignore;
    else if (arg == "Sections")return ignore;
    else if (arg == "Local")return ignore;
    else if (arg == "Code&")return ignore;
    else if (arg == "Code const&")return ignore;
    else if (arg == "Section")return ignore;
    else if (arg == "Global")return ignore;
    else if (arg == "ParserOptions")return ignore;
    else if (arg == "Value")return ignore;
    else if (arg == "Arg")return ignore; // truely internal, should not be exposed! e.g. Arg
    else if (arg == "Signature")return ignore;
    else if (arg == "Wasp")return ignore;
    else if (arg == "WitReader")return ignore;
    else if (arg == "__cxxabiv1")return ignore;
    else if (arg == "...")return ignore;// varargs, todo interesting!
    else if (arg.startsWith("Map")) return maps;
    else if (arg.startsWith("List")) return list;
    else if (arg.endsWith("&")) return pointer;
    else if (arg.endsWith("*")) return pointer;

    else {
//        breakpoint_helper
//        printf("unmapped c++ argument type %s\n", arg.data);
        if (!arg.endsWith("*"))
            if (!arg.startsWith("Map<") and !arg.startsWith("List<"))
                error("unmapped c++ argument type %s\n"s % arg.data);
    }
    return i32t;
}


// four different types:
// 1. wasm Valtype
// 2. node.kind:Type
// 3. Any<Node and
// 4. some c++ types String List etc
// the last three can be added as special internal values to Valtype, outside the wasm spec
Valtype mapArgToValtype(String &arg) {
    Type primitive = mapArgToType(arg);
    return mapTypeToWasm(primitive);
}

void consumeImportSection() {
    Code imports_vector = vec();
    int importCount = unsignedLEB128(imports_vector);
    module->import_count = importCount;
    module->import_data = imports_vector.rest();
    parseImportNames(imports_vector);
    if (debug_reader)printf("imports: %d\n", importCount);
}


void consumeSections() {
    while (pos < size) {
        Sections section = typ();
        trace("Consuming section "s + sectionName(section));
        switch (section) {
            case type_section:
                consumeTypeSection();
                break;
            case import_section:
                consumeImportSection();
                break;
            case export_section:
                consumeExportSection();
                break;
            case code_section:
                consumeCodeSection();
                break;
            case functypes_section:
                consumeFuncTypeSection();
                break;
            case start_section:
                consumeStartSection();
                break;
            case table_section:
                consumeTableSection();
                break;
            case memory_section:
                consumeMemorySection();
                break;
            case global_section:
                consumeGlobalSection();
                break;
            case data_section:
                consumeDataSection();
                break;
            case element_section:
                consumeElementSection();
                break;
            case custom_section:
                consumeCustomSection(); // => consumeNameSection()
                break;
            default:
                if (debug_reader)print("Invalid section %d at pos %d %x\n"s % section % pos % pos);
                error("not implemented: "s + sectionName(section));
        }
    }
}

#ifndef RUNTIME_ONLY


Module &read_wasm(bytes buffer, int size0) {
    module = new Module(); // todo: make pure, not global!
    module->code = *new Code(buffer, size0, false);
    pos = 0;
    code = buffer;
    size = size0;
    consume(4, (byte *) (magicModuleHeader));
    consume(4, (byte *) (moduleVersion));
    consumeSections();
    module->total_func_count = module->import_count + module->code_count;
    parseFuncTypeSection(module->functype_data);
    // todo: sanity checks?
//    check_eq(module->funcToTypeMap._size, module->code_count)
    return *module;
}

//static
Map<long, Module *> module_cache{.capacity=100};

Module &read_wasm(chars file) {
    if (!s(file).endsWith(".wasm"))
        file = concat(file, ".wasm");
#if WASM
    return *new Module();
#else
    String name = file;
    if (name.contains("~"))
        file = name.replace("~", "/Users/me"); // todo $HOME
    if (module_cache.has(name.hash()))
        return *module_cache[name.hash()];

    if (debug_reader)print("--------------------------\n");
//    if (debug_reader)
    printf("parsing: %s\n", file);
    size = fileSize(file);
    if (size <= 0)error("file not found: "s + file);
    bytes buffer = (bytes) alloc(1, size);// do not free
    FILE *stream = fopen(file, "rb");
    fread(buffer, sizeof(buffer), size, stream);
    Module &wasm = read_wasm(buffer, size);
    wasm.code.name = name;
    wasm.name = name;
    fclose(stream);
    module_cache.add(name.hash(), &wasm);
    return wasm;
#endif
}

#endif
#undef pointerr

Code &read_code(chars file) {
    int size;
    char *data = readFile(file, &size);
    Code &cod = *new Code(data, size, false);
    cod.name = String(file);
    return cod;
}


//#import "wasm_patcher.cpp"
