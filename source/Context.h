#pragma once
// AST Analysis Context (in Angle.cpp after Wasp.cpp parser, before wasm_emitter.cpp)
#include "String.h"
#include "Code.h"

extern Map<int64, bool> analyzed;

class Module {
    //public:
    //	virtual ~Module() {
    //		type_data = Code();
    ////		free(type_data);
    ////		free(type_data.data);
    //	}

public:
    //    Module(){}
    //    Module(String name) : name(name) {}
    String name;

    // unparsed (just the vector data and its length)
    Code code; // the whole code unparsed, used in merge
    Code type_data;
    Code import_data;
    Code export_data;
    Code functype_data;
    Code code_data;
    Code globals_data;
    Code memory_data;
    Code table_data;

    Code name_data;
    //	Code data_section;// data_segments with header: data_segments_count + memory_id + i32.const + offset + 0x0b?
    Code data_segments;
    // data_section without header ^^ (actual data) todo BUT memory_id + i32.const + offset + 0x0b is HEADER PER SEGMENT!
    Code linking_section;
    Code relocate_section;
    int funcToTypeMap[10000];
    //    List<int> funcToTypeMap;// implicit function code id to wasm type (excluding imports which have their own type! wtf wasm spec)
    List<Code> custom_sections; // all remaining

    int type_count = 0;
    int import_count = 0;
    int total_func_count = 0; // code_count + import_count
    int table_count = 0; // no such thing?
    int memory_count = 0;
    int export_count = 0;
    int global_count = 0;
    int code_count = 0;
    int data_segments_count = 0; // ≠ data_section.length   ALL MUST BE IN the one Data section!
    int data_count = 0; // extra section? what's that?
    // name section:
    Code element_section; // todo?
    Code local_names;
    Code global_names;
    Code data_names;
    Code function_names;
    Code type_names; // GC!
    Code field_names; // GC struct.field

    int start_index;
    bool is_native_library = false; // True for FFI native libraries (libc, libm, etc.)
    List<Global> globals;
    List<String> import_names;
    List<String> export_names;
    List<Signature> funcTypes{1000}; // c++ types from export name convention, implicit index
    //	List<Signature> wasmFuncTypes;// wasm types from funcTypes section implicit index
    // Map<String, Signature> signatures{2000};; // also implicit index->Signature 0,1,2… ! before merging!
    Map<String, Function> functions{1000}; // contains imports, so index via call_index, not code_index
    //	int data_offset=0;// todo: read from data section! why not 0 ?
    int data_offset_end = 0;

    void file(const char *string);

    void save(const char *file);

    bool uses_native_library() {
        for (int i = 0; i < functions.size(); i++)
            if (functions.values[i].is_ffi) // && !func.ffi_library.empty()) {
                return true;
        return false;
    }
};


void addLibrary(Module *modul);

void useFunction(String name);

bool addGlobal(Function &context, String name, Type type, bool is_param, Node *value);

bool addLocal(Function &context, String name, Type Type, bool is_param); // must NOT be accessible from Emitter!
void updateLocal(Function &context, String name, Type type);

bool isGlobal(Node &node, Function &function);

Signature &groupFunctionArgs(Function &function, Node &params); // todo move elsewhere!
void clearAnalyzerContext();

Function *use_required_import(Function *function);

void addLibraryFunctionAsImport(Function &func);

Function getWaspFunction(String name);

void use_runtime(const char *function);

void preRegisterFunctions();

bool isFunction(Node &op);

bool isFunction(String op, bool deep_search = true);


extern "C" int64 run_wasm_file(chars file);

bool isPrefixOperation(Node &node, Node &lhs, Node &rhs);

String &checkCanonicalName(String &name);

void refineSignatures(Map<String, Function> &map);

Module &loadModule(String name);

Function *findLibraryFunction(String name, bool searchAliases);

static bool eq(Module *x, Module *y) { return x->name == y->name; } // for List: libraries.has(library)

void addLibrary(Module *modul);

void useFunction(String name);

