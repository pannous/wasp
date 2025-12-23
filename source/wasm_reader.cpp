#include "Code.h"
#include <cstdio>
#include <stdlib.h>
#include <cxxabi.h> // for abi::__cxa_demangle
#include "wasm_reader.h"
#include "Util.h"
#include "ffi_signatures.h"
//#include "wasm_emitter.h"
// compare with wasm-objdump -h

#define POLYMORPH_function_index_marker -2
#if MY_WASM and not WEBAPP
//bool build_module = false; // link via js
bool build_module = true; // for funclets … and wasm runtime merge?
#else
bool build_module = true;
#endif

bool debug_reader = tracing;
// bool debug_reader = true;

typedef unsigned char *bytes;
int pos = 0;
int size = 0;
byte *code;

// https://webassembly.github.io/spec/core/binary/modules.html#sections

//Module& module=*new Module();
//extern Map<String, int> module->functionIndices;// todo: use function[String].index
//extern List<String> declaredFunctions; only new functions that will get a Code block, no runtime/imports


Valtype mapArgToValtype(String &arg);

#define consume(len, match) if(!consume_x(code,&pos,len,match)){if(debug_reader)printf("\nNOT consuming %s",#match);backtrace_line();}

bool consume_x(byte *code_bytes, int *posi, int len, byte *bytes) {
    if (*posi + len > size)
        error("END OF FILE");
    if (not bytes) {
        *posi = *posi + len;
        return true;
    }

    int i = 0;
    while (i < len) {
        if (bytes and code_bytes[*posi] != bytes[i])
            return false;
        *posi = *posi + 1;
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
int64 unsignedLEB128(Code &byt) {
    if (!byt.data || byt.length <= 0) return 0;
    int64 n = 0;
    short shift = 0;
    do {
        byte b = byt.data[byt.start++];
        n = n | (((int64) (b & 0x7f)) << shift);
        if ((b & 0x80) == 0)break; // no more continuation
        shift += 7;
    } while (byt.start < byt.length);
    return n;
}

int consumeByte(Code &byt) {
    return byt.data[byt.start++];
}
int consumeInt32() {
    int b1 = code[pos++];
    int b2 = code[pos++];
    int b3 = code[pos++];
    int b4 = code[pos++];
    return (b1 << 0) | (b2 << 8) | (b3 << 16) | (b4 << 24);
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
    else data.start -= 1; // undo len read
    return code1;
}

String &name(Code &wstring) {
    // MUST copy the string data, not share, because wstring buffer gets reused
    int64 len = unsignedLEB128(wstring);
    auto nam = (char *) wstring.data + wstring.start;
    String *string = new String(nam, len, false); // false = copy data, don't share
    //    String *string = new String(nam, len, true); // OLD BUG: shared pointer gets corrupted
    wstring.start += len; // advance internally
    //	if (len > 40)put(string);
    return *string;
}

// todo: treat all functions (in library file) as exports if ExportSection is empty !?
// that is: add their signatures to module …
void parseFunctionNames(Code &payload) {
    //	module->functions.setDefault(Function());
    //	put(module->functionIndices);// what we got so far?
    int function_count = unsignedLEB128(payload);
    int call_index = -1;
    if (debug_reader)printf("function count: %d\n", function_count);
    Map<String, Function> &funcs = module->functions;
    for (int i = 0; i < function_count and payload.start < payload.length; ++i) {
        call_index = unsignedLEB128(payload);
        int code_index = call_index - module->import_count;
        if (i != call_index)
            trace("i≠index => some functions not named (or other things named)");
        if (call_index < 0 or call_index > 100000)
            error("broken index"s + call_index);
        String &name1 = name(payload);
        String func = name1.clone(); // needs to be 0-terminated now
        // if (debug_reader)print("ƒ%d %s\n"s % call_index % func);
        if (func.contains("ltoa"))
            print("ƒ%d %s\n"s % call_index % func);
        bool old = funcs.contains(func);
        if (old) {
            trace("function already exists: "s + func);
            Function &function = funcs[func];
        } else {
            Function &function = *new Function();
            if (function.code_index >= 0 and function.code_index != code_index)
                trace("already has index: "s + func + " " + function.code_index + "≠" + code_index);
            function.code_index = code_index;
            function.name = func;
            funcs.add(func, function);
        }
    }
    //	  (import "env" "log_chars" (func (;0;) $logs (type 0)))  export / import names != internal names
    //	for (int i = function_count; i < module->total_func_count; i++)
    //		module->functionIndices.insert_or_assign("unnamed_func_"s + i, i);
}

//Map<int, Signature> funcTypes;
// import type indices are part of import struct!
// FOR ALL FUNCTIONS WITHOUT EXPORT!
void parseFuncTypeSection(Code &payload) {
    info("parseFuncTypeSection");
    // imports stupidly have their own type,
    // we don't know here if i32 is pointer … so we may have to refine later
    //        int offset = 0; O(1)
    for (int code_index = 0; code_index < module->code_count and payload.start < payload.length; ++code_index) {
        uint call_index = (int) (code_index + module->import_count); // implicit !?
        int typ = unsignedLEB128(payload);
        Function *function = 0;
        for (int i = 0; i < module->functions.size(); ++i) {
            Function &f = module->functions.values[i];
            if (f.code_index == code_index or f.call_index == call_index) {
                function = &f;
                break;
            }
        }
        if (!function) {
            // error("named function not found for call_index "s + call_index);
            trace("named function not found for call_index "s + call_index);
            // e.g. function operator+ already has signature ( String self, real )  todo OVERLOAD BUG!
            String unnamed = "func_"s + call_index; // dummy name
            function = new Function{
                .code_index = ((int) call_index - module->import_count),
                .call_index = (int) call_index,
                .name = unnamed,
                .module = module
            };
            module->functions.add(unnamed, *function);
            // error("function not found for call_index "s + call_index);
            // continue;
        }
        Signature &s = module->funcTypes[typ];
        function->signature.merge(s);
        if (debug_reader) {
            char *sig = function->signature.serialize().data;
            print("update #%d ƒ%d %s%s\n"s % code_index % call_index % function->name.data % sig);
        }
    }
}

// not part of name section wtf
void parseImportNames(Code &payload) {
    // and TYPES!
    trace("Imports:");
    for (int i = 0; (i < module->import_count) and (payload.start < payload.length); ++i) {
        String library_name = name(payload); // library (e.g., "c", "m", "raylib") - MUST be copy, not reference!
        String function_name = name(payload); // function (e.g., "strlen", "ceil") - MUST be copy, not reference!
        trace("import %s"s % library_name + "." + function_name);
        int kind = unsignedLEB128(payload); // func, global, …
        int type = unsignedLEB128(payload); // i32 … for globals
        if (kind == 3 /*global import*/) {
            bool is_mutable = unsignedLEB128(payload);
            Global global{i, function_name, (Valtype) type,/*todo value*/0, is_mutable, true, false};
            module->globals.add(global);
            continue;
        }
        Signature &signature = module->funcTypes[type];
        int coi = module->import_count + i;
        Function func = {.code_index = coi, .call_index = i, .name = function_name, .signature = signature};

        // Check if this is an FFI import from a native library module
        if (library_name != "env" && library_name != "lib" && not library_name.contains("wasi")) {
            func.is_ffi = true;
            func.ffi_library = library_name;

            // Re-detect signature from library/function name
            // (WASM loses type info like charp -> i32, so we need to restore it)

            // ⚠️  CRITICAL: Arguments MUST be (function_name, library_name) NOT (library, function)!
            // ⚠️  This has broken 3+ times. Test with strlen before committing!
            // ⚠️  See docs/ffi-strlen-bug-guide.md for details
            detect_ffi_signature(function_name, library_name, func.signature);
        }

        module->functions.add(function_name, func);
        module->import_names.add(function_name);
    }
    info("imports DONE");
}

// Signatures to be consumed in export section
void parse_type_data(Code &payload) {
    for (int i = 0; i < module->type_count and payload.start < payload.length; ++i) {
        Signature sic;
        sic.type_index = i;
        int typ = unsignedLEB128(payload); // implicit?
        if (typ != func)continue;
        int param_count = unsignedLEB128(payload);
        for (int j = 0; j < param_count; j++) {
            Valtype argt = (Valtype) unsignedLEB128(payload);
            sic.add(argt);
        }
        int return_count = unsignedLEB128(payload);
        if (return_count) {
            // todo: multi-value
            Valtype rt = (Valtype) unsignedLEB128(payload);
            sic.returns(rt);
        } else
            sic.returns(none);
        if (module->funcTypes.size() != i)
            error("module->funcTypes pre filled");
        module->funcTypes.add(sic);
    }
}

void consumeTypeSection() {
    Code type_vector = vec();
    if (not build_module)return;
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
    module->table_count = 1; // unsignedLEB128(module->table_data);
    if (debug_reader)printf("tables: %d \n", module->table_count);
}

void consumeMemorySection() {
    module->memory_data = vec(); // todo ?
    //	module->memory_count = unsignedLEB128(module->memory_data);//  always 1 in MVP
    if (debug_reader)printf("memory_data: %d\n", module->memory_data.length);
}

void consumeGlobalSection() {
    module->globals_data = vec(); // todo
    module->global_count = unsignedLEB128(module->globals_data); // NO SUCH THING!?
    if (debug_reader)printf("globals: %d\n", module->global_count);
}

void consumeNameSection(Code &data) {
    if (debug_reader)print("names: …\n");
    module->name_data = data; //.clone();
    //    module->name_data = data.clone();
    while (data.start < data.length) {
        int type = unsignedLEB128(data);
        Code payload = vec(data); // todo test!
        //        print("name type: %d\n"s % type);
        //        printf("length: %d\n", payload.length);
        switch (type) {
            case module_name: {
                // 0
                module->name = name(payload); // wrapped in vector why?
            }
            break;
            case function_names: // 1
                module->function_names = payload;
                parseFunctionNames(payload);
            //                parseFunctionNames(payload.clone());
                break;
            case local_names:
                module->local_names = payload;
                break;
            case global_names:
                module->global_names = payload;
                break;
            case data_names:
                module->data_names = payload;
                break;
            case type_names: // gc!
                module->type_names = payload;
                break;
            case field_names: //= 10, // GC struct.field
                module->field_names = payload;
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
    if (not build_module)return;
    //	module->data_section = datas.clone();
    module->data_segments_count = unsignedLEB128(datas);
    module->data_segments = datas.rest(); // whereever the start may be now
    //	short memory_id =
    unsignedLEB128(datas);
    unsignedLEB128(datas); // skip i32.const opcode
    int data_offset = unsignedLEB128(datas);
    unsignedLEB128(datas); // skip '0b' whatever that is
    module->data_offset_end = data_offset + module->data_segments.length + 100; //  datas.length;
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

void consumeDataCountSection() {
    Code data_count_vector = vec();
    module->data_count = unsignedLEB128(data_count_vector);
    if (debug_reader)printf("data count: %d \n", module->data_count);
}

void consumeFFIMetaSection(Code &payload) {
    int ffi_count = unsignedLEB128(payload);
    if (debug_reader) printf("FFI metadata: %d functions\n", ffi_count);

    for (int i = 0; i < ffi_count; i++) {
        // Read function name
        Code func_name_vec = vec(payload);
        String func_name((chars)func_name_vec.data, func_name_vec.length);

        // Read library name
        Code lib_name_vec = vec(payload);
        String lib_name((chars)lib_name_vec.data, lib_name_vec.length);

        // Find and update the function metadata
        if (module->functions.has(func_name)) {
            Function& func = module->functions[func_name];
            func.is_ffi = true;
            func.ffi_library = lib_name;
            if (debug_reader) printf("  FFI: %s from %s\n", func_name.data, lib_name.data);
        }
    }
}

void consumeCustomSection() {
    Code customSectionDatas = vec();
    String type = name(customSectionDatas);
    Code payload = customSectionDatas.rest();
    trace("custom section:");
    trace(type);
    //    trace("custom section length: %d\n", payload.length);
    if (type == "names" or type == "name") {
        consumeNameSection(payload);
    } else if (type == "wasp.ffi") {
        consumeFFIMetaSection(payload);
    } else if (type == "target_features")
    //	https://github.com/WebAssembly/tool-conventions/blob/main/Linking.md#target-features-section
    // 	atomics bulk-memory exception-handling multivalue mutable-globals nontrapping-fpoint sign-ext simd128 tail-call
        todow("target_features detection not yet supported") else if (type == "linking")
        // see https://github.com/WebAssembly/tool-conventions/blob/main/Linking.md
        consumeLinkingSection(payload);
    else if (type == "dylink.0")
    // see https://github.com/WebAssembly/tool-conventions/blob/main/DynamicLinking.md
        todow("dynamic linking not yet supported") else if (type.startsWith("reloc."))
        consumeRelocateSection(payload); // e.g. "reloc.CODE"
        // everything after the period is ignored and the specific target section is encoded in the reloc section itself.
        // see https://github.com/WebAssembly/tool-conventions/blob/main/Linking.md
    else if (type == "relocate")
        consumeRelocateSection(payload);
    else if (type == "producers"); // ignore
    else if (type == "sourceMappingURL"); // ignore todo ?
    else {
        //		pos = size;// force finish
        todow("consumeCustomSection not implementated for "s + type);
        customSectionDatas.start = 0; // reset

        //		TODO REENABLE!! currently causes SIGSEGV
        //		module->custom_sections.add(customSectionDatas);// raw
    }
}


// connect func/code indices to type indices
void consumeFuncTypeSection() {
    Code type_vector = vec();
    module->code_count = unsignedLEB128(type_vector); // import type indices are part of import struct!
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
    if (module->code_count != codeCount)
        error("missing code/signatures");
    if (not build_module)return;
    module->code_data = codes_vector.rest();
    if (debug_reader)printf("code length: %d\n", module->code_data.length);
}


void fixupGenerics(char *s, int len) {
    int bra = 0;
    for (int i = 0; i < len; ++i) {
        if (s[i] == '<')bra++;
        if (s[i] == '>')bra--;
        if (bra and s[i] == ',')s[i] = ';';
    }
}

// todo dedup demangle in Util
char *demangle(String &fun) {
    char *string = 0;
#if MY_WASM
    string = js_demangle(fun); // https://github.com/kripken/cxx_demangle/blob/master/demangle.js
    return string;
#else
    int status;
    string = abi::__cxa_demangle(fun.data, 0, 0, &status);
#endif
    // if (string == 0)return 0;
    return string;
}

// we can reconstruct arguments from demangled exports or retained wast names
// _Z2eqPKcS0_i =>  func $eq_char_const*__char_const*__int_ <= eq(char const*, char const*, int)
List<String> demangle_args(String &fun) {
    char *string = demangle(fun);
    if (string == 0)return 0;
    String real_name = String(string);
    fixupGenerics(real_name.data, real_name.length);
    if (!real_name or !real_name.contains("("))return (size_t) 0;
    String brace = real_name.substring(real_name.indexOf('(') + 1, real_name.indexOf(')')); //.clone();
    if (brace.contains("("))
        return (size_t) 0; // function pointers not supported yet "List<String>::sort(bool (*)(String&, String&))"
    return brace.split(", ");
}


// todo: treat all functions (in library file) as exports if ExportSection is empty !?
// https://webassembly.github.io/spec/core/binary/modules.html#binary-exportsec
void consumeExportSection() {
    Code exports_vector = vec();
    if (not build_module)return;
    int exportCount = unsignedLEB128(exports_vector);
    if (debug_reader)printf("export_section: %d\n", exportCount);
    module->export_count = exportCount;
    module->export_data = exports_vector.rest();
    Code &payload = module->export_data;
    for (int i = 0; i < exportCount; i++) {
        // print("FUNCTION EXPORT %d\n"s % i);
        // if (i == 6)return; // DANGER hack js bug!
        String &export_name = name(payload);
        // module->export_names.add(export_name);
        int export_type = unsignedLEB128(payload); // don't confuse with function_type if export_type==0
        int index = unsignedLEB128(payload); // for all types!
        if (export_type == 3 /*global*/) continue; // todo !?
        if (export_type == 2 /*memory*/) continue;
        if (export_type == 1 /*table*/) continue;
        if (export_type != 0 /*function export*/) {
            error("UNKNOWN export_type "s + export_type + " for " + export_name + " at " + index);
            continue;
        }
        // we have export_name, export_name_demangled and internal_name, demangled and with(out) signature!
        // all united by call_index
        String &func0 = export_name;
        String func = extractFuncName(func0); // demangled name
        String demangled = demangle(func0); // demangled signature
        // index here means call_index > code_index
        if (index < module->import_count or index > 100000)
            error("corrupt index "s + index);
        int call_index = index; // why does export section link to call_index, not to code_index?
        int lower_index = index - module->import_count; // the index when called
        int code_index = lower_index;

        //        if (func == "parseLong")
        //            breakpoint_helper;// don't make libraries 'main' visible, use own
        int status = 0; // for debugging:

        // print("demangled: %s\n"s % demangled);
        Function &fun0 = module->functions[func0]; // mangled
        fun0.module = module;
        fun0.name = func0;

        if (func != func0 and not module->functions.contains(func)) {
            //  always insert the demangled synonym
            module->functions.add(func, *new Function{}); // sure start blank??
        }
        // ⚠️ CAN BE THE SAME REFERENCE IF func==func0 !!! ⚠️
        Function *fun = &module->functions[func]; // demangled
        fun->module = module;
        fun->name = func;

        if (debug_reader) {
            print("#%d ƒ%d %s ≈\n"s % code_index % index % func0.data);
            print("#%d ƒ%d %s ≈\n"s % code_index % index % func.data);
            print("#%d ƒ%d %s ≈\n"s % code_index % index % demangled.data);
        }
        //        if(fun.name=="puts")
        //            breakpoint_helper

        if (fun->is_polymorphic) {
            trace("HYPER (>2) polymorphic function %s"s % func);
            Function *abstract = fun;
            fun = new Function{
                .code_index = lower_index, .name = func0, .module = abstract->module, .is_runtime = true
            };
            abstract->variants.add(fun);
        } else if (fun->signature.size()) {
            // already has signature => make polymorphic
            trace("function %s already has signature "s % func + fun->signature.serialize());
            trace("function %s old code_index %d new code_index %d"s % func % fun->code_index % lower_index);
            Function *old_fun = fun->clone(); // keep pointer to old
            old_fun->name = old_fun->fullname; // restore original mangled name for linking
            fun = new Function{.code_index = lower_index, .name = func0, .module = old_fun->module, .is_runtime = true};

            module->functions[func] = *new Function;
            Function &abstract = module->functions[func];
            abstract.name = func;
            abstract.module = module;
            abstract.is_polymorphic = true;
            abstract.is_runtime = true; // sure?

            abstract.variants.add(old_fun);
            abstract.variants.add(fun);
            fun = abstract.variants.last();
        } else {
            fun0.code_index = lower_index;
            fun->code_index = lower_index;
            fun->fullname = func0;
        }

        //        if (call_index == 389)//
        //            debug_reader = true;

        int wasmFuncType = module->funcToTypeMap[code_index]; // mainly used for return type here
        // todo: demangling doesn't yield return type, is wasm_signature ok?
        fun->signature.type_index = wasmFuncType;
        fun0.signature.type_index = wasmFuncType;
        // todo: remove duplicate, try fun0.signature=fun.signature at end again
        if (not(0 <= wasmFuncType and wasmFuncType <= module->funcTypes.size_))
            check_silent(0 <= wasmFuncType and wasmFuncType <= module->funcTypes.size_) // todo HOW?
        Signature &wasm_signature = module->funcTypes[wasmFuncType];
        Valtype returns = mapTypeToWasm(wasm_signature.return_types.last(Kind::undefined));
        if (wasm_signature.wasm_return_type == void_block) returns = void_block;
        fun->signature.returns(returns);
        if (fun != &fun0)
            fun0.signature.returns(returns);
        // todo: use wasm_signature if demangling fails, see merge(signature) below
        if (demangled.contains("::")) {
            String typ = demangled.to("::");
            auto type = mapType(typ); // Primitive::self
            fun->signature.add(type, "self");
        }
        // e.g. List<String>::add (String) has one arg, but wasm signature is (i32,i32):i32  ["_ZN4ListI6StringE3addES0_"]
        // todo: demangle further and put into multi-dispatch
        List<String> args = demangle_args(func0);
        for (String &arg: args) {
            if (arg.empty())continue;
            fun->signature.add(mapType(arg));
            if (fun != &fun0)
                fun0.signature.add(mapType(arg));
        }
        if (not(demangled.contains("("))) {
            // extern "C" pure function name
            fun->signature = wasm_signature;
            fun0.signature = wasm_signature;
        }
        if (not module->functions.has(func0)) // mangled name
            module->functions.add(func0, *fun);
        // can't after free
        //        if (&fun != &fun0)
        //            fun0.signature = fun.signature;// ⚠️ SHARING deep list fields now. this causes problems later:
        //        fun0.signature = fun.signature.clone();// todo copy by value ok? NO: heap-use-after-free on address
        if (debug_reader) {
            const String &argos = args.join(",");
            //            printf("ƒ%d %s(%s) ≈\n", index, func0.data, string);
            char *sig = fun->signature.serialize().data;
            print("#%d ƒ%d %s(%s)\n"s % code_index % index % func.data % argos.data);
            print("#%d ƒ%d %s%s\n"s % code_index % index % func.data % sig);
            //            check(module->functions["test42"].signature.size()==0)
        }
    }
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
            case datacount:
                consumeDataCountSection();
                break;
            default:
                if (debug_reader)print("Invalid section %d at pos %d %x\n"s % section % pos % pos);
                error("not implemented: "s + sectionName(section));
        }
    }
}


//static
Map<int64, Module *> module_cache{100};

Code &read_code(chars file) {
    int code_size;
    char *data = readFile(file, &code_size);
    Code &cod = *new Code(data, code_size, false);
    cod.name = String(file);
    return cod;
}


//#import "wasm_patcher.cpp"
void Module::file(const char *string) {
}

void Module::save(const char *file) {
#if not WASM
    //    wasm_emitter::save(*this, file);
    //    wasm_writer::save(*this, file);
    FILE *stream = fopen(file, "wb");
    fwrite(code.data, sizeof(byte), code.length, stream);
    fclose(stream);
#endif
}


Module &read_wasm(String file) {
    if (file.empty())
        error("read_wasm: empty file name");
    if (module_cache.has(file.hash()))
        return *module_cache[file.hash()];
    String name = file;
    bytes buffer;
#if MY_WASM
    size_t size = 0;
    buffer = getWasmFunclet(file,&size);// load from host
#elif WASM
    return *new Module();
#else
    if (file.contains("~"))
        file = file.replace("~", "/Users/me"); // todo $HOME
    if (file.endsWith(".wast") or file.endsWith(".wat")) {
        file = compileWast(file.data);
    }

    if (!file.endsWith(".wasm"))
        file = concat(file, ".wasm");

    if (module_cache.has(name.hash()))
        return *module_cache[name.hash()];
    if (debug_reader)print("--------------------------\n");
    //    if (debug_reader)
#if not RELEASE
    printf("parsing: %s\n", file.data);
#endif
    size = fileSize(file.data);
    if (size <= 0)
        error("file not found: "s + file);
    buffer = (bytes) malloc(size + 4096 * 16); // do not free
    FILE *stream = fopen(file.data, "rb");
    fread(buffer, 1, size, stream); // demands blocks of 4096!
    fclose(stream);
#endif
    Module &wasm_module = read_wasm(buffer, size);
    wasm_module.code.name = name;
    wasm_module.name = name;
    if (not module_cache.has(name.hash()))
        module_cache.add(name.hash(), &wasm_module);
    return wasm_module;
}

Module &read_wasm(bytes buffer, int size0) {
    module = new Module(); // todo: make pure, not global!
    module->code = *new Code(buffer, size0, false);
    pos = 0;
    code = buffer;
    size = size0;
    consume(4, (byte *) (magicModuleHeader));
    consume(4, (byte *) (moduleVersion));
    consumeSections();
    parseFuncTypeSection(module->functype_data);
    module->total_func_count = module->code_count + module->import_count;
    return *module;
}
