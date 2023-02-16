//
// Created by me on 07.12.20.
//

#include "wasm_linker.h"

#include "own_merge/binary.h"
#include "own_merge/wasm-link.h"
#include "own_merge/leb128.h"
#include "own_merge/stream.h"
#include "own_merge/type.h"
#include "own_merge/binary-writer.h"
#include "own_merge/binary-reader-linker.h"
#include "own_merge/result.h" // todo remove
#include "own_merge/binding-hash.h" // todo remove ?

#include "Code.h"
#include "wasm_reader.h"

#include <map>
#include <utility>
#include "Map.h"
#include "wasm_patcher.h"
//#include "wasm_reader.h"
#include "Code.h"
#include "Angle.h"
#include "Util.h"

typedef unsigned char *bytes;


using namespace wabt;
using namespace wabt::link;
//using wabt::link::LinkerInputBinary;
//using wabt::link::Section;
//using wabt::Reloc;
//using wabt::RelocType;
//using wabt::WriteU32Leb128Raw;

template<class S, class T>
struct Pair {
    S first;
    S second;
};

//String s(String s) {
//	return String(s.data(),s.size());
//}
String &s(String &x) { return x; }

String s(String x) { return x; }

//
//String::String(const String str):String(str.data){
//	// defeats the purpose of using a lightweight unicode-aware String class
//}

#define FIRST_KNOWN_SECTION SectionType::Type
//#define LOG_DEBUG(fmt, ...) if (s_debug) s_log_stream->Writef(fmt, __VA_ARGS__);
#define LOG_DEBUG(fmt, ...) if (s_debug) printf(fmt, __VA_ARGS__);

int unknown_opcode_length_TODO = -1;
const int leb = -2;// special marker for varlength leb argument ( i32.const … )
int heaptype = -3;
int datax = -4;// two leb params?
int block_index = leb;
int u32_type = 4;
int i32_type = 4;

// https://github.com/WebAssembly/function-references/blob/master/proposals/function-references/Overview.md#local-bindings
std::map<short, int> opcode_args = { // BYTES used by wasm op AFTER the op code (not the stack values! e.g. 4 bytes for f32.const )
        {nop_,                0}, // useful for relocation padding call 1 -> call 10000000
        {block,               leb},
        {loop,                0},
        {if_i,                0},// precede by i32 result}, follow by i32_type {7f}
        {else_,               0},
        {return_block,        0},

        // EXTENSIONS:
        {try_,                block_index},
        {catch_,              block_index},
        {throw_,              block_index},
        {rethrow_,            block_index},
        {br_on_exn_,          block_index}, // branch on exception

        {end_block,           0}, //11
        {br_branch,           block_index},
        {br_if,               block_index},
        {br_table,            block_index},
        {return_block,        block_index},
        {function_call,       block_index},

        // EXTENSIONS:
        {call_ref,            u32_type},
        {return_call_ref,     u32_type},
        {func_bind,           u32_type},// {type $t} {$t : u32_type
        {let_local,           leb}, // {let <bt> <locals> {bt : blocktype}, locals : {as in functions}

        {drop,                0}, // pop stack
        {select_if,           0}, // a?b:c ternary todo: use!
        {select_t,            1}, // extension … ?
        {0x27,                1}, //bug?
        {local_get,           leb},
        {local_set,           leb},
        {local_tee,           leb},
        {get_local,           leb},// get to stack
        {set_local,           leb},// set and pop
        {tee_local,           leb},// set and leave on stack

        {global_get,          leb},
        {global_set,          leb},

        //{ Anyref/externref≠funcref tables}, Table.get and Table.set {for Anyref only}.
        //{Support for making Anyrefs from Funcrefs is out of scope
        {table_get,           leb},
        {table_set,           leb},

        {i8_load,             datax}, //== 𝟶𝚡𝟸𝙳}, i32.load𝟪_u
        {i16_load,            datax}, //== 𝟶𝚡𝟸𝙳}, i32.load𝟪_u
        {i32_load,            datax},// load word from i32 address
        {f32_load,            datax},
        {i32_store,           datax},// store word at i32 address
        {f32_store,           datax},
        // todo : peek 65536 as float directly via opcode
        {i64_load,            datax}, // memory.peek memory.get memory.read
        {i64_store,           datax}, // memory.poke memory.set memory.write
        {i32_store_8,         datax},
        {i32_store_16,        datax},
        {i8_store,            datax},
        {i16_store,           datax},

        //{i32_store_byte, -1},// store byte at i32 address
        {i32_auto,            leb},
        {i32_const,           leb},
        {i64_auto,            leb},
        {i64_const,           leb},
        {f32_auto,            4},
        {f64_const,           8},

        {i32_eqz,             0}, // use for not!
//		{negate,                              -1},
//		{not_truty,                           -1},
        {i32_eq,              0},
        {i32_ne,              0},
        {i32_lt,              0},
        {i32_gt,              0},
        {i32_le,              0},
        {i32_ge,              0},

        {i64_eqz,             0},
        {f32_eqz,             0}, // HACK: no such thing!


        {i64_eqz,             0},
        {i64_eq,              0},
        {i64_ne,              0},
        {i64_lt_s,            0},
        {i64_lt_u,            0},
        {i64_gt_s,            0},
        {i64_gt_u,            0},
        {i64_le_s,            0},
        {i64_le_u,            0},
        {i64_ge_s,            0},
        {i64_ge_u,            0},

        {f32_eq,              0},
        {f32_ne,              0}, // !=
        {f32_lt,              0},
        {f32_gt,              0},
        {f32_le,              0},
        {f32_ge,              0},
        {f64_eq,              0},
        {f64_ne,              0}, // !=
        {f64_lt,              0},
        {f64_gt,              0},
        {f64_le,              0},
        {f64_ge,              0},

        {i32_add,             0},
        {i32_sub,             0},
        {i32_mul,             0},
        {i32_div,             0},
        {i32_rem,             0}, // 5%4=1
        {i32_modulo,          0},
        {i32_rem_u,           0},
        {i32_and,             0},
        {i32_or,              0},
        {i32_xor,             0},
        {i32_shl,             0},
        {i32_shr_s,           0},
        {i32_shr_u,           0},
        {i32_rotl,            0},
        {i32_rotr,            0},

        //{⚠ warning: funny UTF characters ahead! todo: replace c => c etc?
        {i64_clz,             0},
        {i64_ctz,             0},
        {i64_popcnt,          0},
        {i64_add,             0},
        {i64_sub,             0},
        {i64_mul,             0},
//		{i64_div_s,           0},
//		{i64_div_u,           0},
        {i64_rem_s,           0},
        {i64_rem_u,           0},
        {i64_and,             0},
        {i64_or,              0},
        {i64_xor,             0},
        {i64_shl,             0},
        {i64_shr_s,           0},
        {i64_shr_u,           0},
        {i64_rotl,            0},
        {i64_rotr,            0},

        // beginning of float opcodes
        // todo : difference : ???
        {f32_abs,             0},
        {f32_neg,             0},
        {f32_ceil,            0},
        {f32_floor,           0},
        {f32_trunc,           0},
        {f32_round,           0},// truncation ≠ proper rounding!
        {f32_nearest,         0},

        {f32_sqrt,            0},
        {f32_add,             0},
        {f32_sub,             0},
        {f32_mul,             0},// f32.mul
        {f32_div,             0},

        {f64_abs,             0},
        {f64_neg,             0},
        {f64_ceil,            0},
        {f64_floor,           0},
        {f64_trunc,           0},
        {f64_nearest,         0},
        {f64_sqrt,            0},
        {f64_add,             0},
        {f64_sub,             0},
        {f64_mul,             0},
        {f64_div,             0},
        {f64_min,             0},
        {f64_max,             0},
        {f64_copysign,        0},

        {f32_cast_to_i32_s,   0},// truncation ≠ proper rounding {f32_round, -1}!
        {i32_trunc_f32_s,     0}, // cast/convert != reinterpret
        {f32_convert_i32_s,   0},// convert FROM i32
//		{i32_cast_to_f32_s,                   -1},
        //{i32_cast_to_f64_s =

        {f32_from_int32,      0},
        {f64_promote_f32,     0},
        {f64_from_f32,        f64_promote_f32},
        {i32_reinterpret_f32, 0}, // f32->i32 bit wise reinterpret != cast/trunc/convert
        {f32_reinterpret_i32, 0}, // i32->f32

        {i32_wrap_i64,        0},
        {i32_trunc_f32_s,     0},
        {i32_trunc_f32_u,     0},
        {i32_trunc_f64_s,     0},
        {i32_trunc_f64_u,     0},
        {i64_extend_i32_s,    0},
        {i64_extend_i32_u,    0},
        {i64_trunc_f32_s,     0},
        {i64_trunc_f32_u,     0},
        {i64_trunc_f64_s,     0},
        {i64_trunc_f64_u,     0},
        {f32_convert_i32_s,   0},
        {f32_convert_i32_u,   0},
        {f32_convert_i64_s,   0},
        {f32_convert_i64_u,   0},
        {f32_demote_f64,      0},
        {f64_convert_i32_s,   0},
        {f64_convert_i32_u,   0},
        {f64_convert_i64_s,   0},
        {f64_convert_i64_u,   0},
        {f64_promote_f32,     0},
        {i32_reinterpret_f32, 0},
        {i64_reinterpret_f64, 0},
        {f32_reinterpret_i32, 0},
        {f64_reinterpret_i64, 0},
        {f32_from_f64,        f32_demote_f64},

        //{signExtensions
        {i32_extend8_s,       0},
        {i32_extend16_s,      0},
        {i64_extend8_s,       0},
        {i64_extend16_s,      0},
        {i64_extend32_s,      0},
        //{i64_extend_i32_s, -1}, WHAT IS THE DIFFERENCE?
        // i64.extend_s/i32 sign-extends an i32 value to i64}, whereas
        // i64.extend32_s sign-extends an i64 value to i64

        //referenceTypes
        // https://github.com/WebAssembly/function-references/blob/master/proposals/function-references/Overview.md#local-bindings
        {ref_null,            0},
        {ref_is_null,         0},
        {ref_func,            leb}, // -1 varuint32 -1 Returns a funcref reference to function $funcidx
        //{ref_null=--1},// {{ref null ht} {$t : heaptype  --1:func --1:extern i >= 0 :{i
        //{ref_typed=--1},// {{ref ht} {$t : heaptype
        {ref_as_non_null,     -1},// {ref.as_non_null
        {br_on_null,          u32_type}, //{br_on_null $l {$l : u32_type
        {br_on_non_null,      u32_type},// {br_on_non_null $l {$l : u32_type

        // saturated truncation  saturatedFloatToInt
        //i32_trunc_sat_f32_s=-1},
        //i32_trunc_sat_f32_u=-1},
        //i32_trunc_sat_f64_s=-1},
        //i32_trunc_sat_f64_u=-1},
        //i64_trunc_sat_f32_s=-1},
        //i64_trunc_sat_f32_u=-1},
        //i64_trunc_sat_f64_s=-1},
        //i64_trunc_sat_f64_u=-1},

        // bulkMemory
        {memory_init,         -1},
        {data_drop,           -1},
        {memory_copy,         -1},
        {memory_fill,         -1},
        {table_init,          -1},
        {elem_drop,           -1},
        {table_copy,          -1},
        {table_grow,          -1},
        {table_size,          -1},
        {table_fill,          -1},
};


// DANGER: modifies the start reader position of code, but not it's data!
[[nodiscard]]
int64 unsignedLEB128(bytes section_data, int length, int &start, bool advance = true) {
    int old_start = start;
    int64 n = 0;
    short shift = 0;
    do {
        byte b = section_data[start++];
        n = n | (((int64) (b & 0x7f)) << shift);
        if ((b & 0x80) == 0)break;
        shift += 7;
    } while (start < length);
    if (!advance)start = old_start;// reset
    return n;
}


int64 unsignedLEB128(List<byte> &section_data, int length, int &start) {
    int64 n = 0;
    short shift = 0;
    do {
        byte b = section_data[start++];
        n = n | (((int64) (b & 0x7f)) << shift);
        if ((b & 0x80) == 0)break;
        shift += 7;
    } while (start < length);
    return n;
}

int64 unsignedLEB128(List<byte> &section_data, int max_length, int &start_reference, bool advance) {
    if (advance)return unsignedLEB128(section_data, max_length, start_reference);
    int start = start_reference;// value
    return unsignedLEB128(section_data, max_length, start);// keep start_reference untouched!
}


int64 unsignedLEB128(Code section_data, int length, int &start, bool advance) {
    int old_start = start;
    int64 n = 0;
    short shift = 0;
    do {
        byte b = section_data[start++];
        n = n | (((int64) (b & 0x7f)) << shift);
        if ((b & 0x80) == 0)break;
        shift += 7;
    } while (start < length);
    if (!advance)start = old_start;//reset
    return n;
}



static bool s_debug = true;

Section::Section()
        : binary(nullptr),
          section_code(SectionType::Invalid),
          size(0),
          offset(0),
          payload_size(0),
          payload_offset(0),
          count(0),
          output_payload_offset(0) {
    ZeroMemory(data);
}

Section::~Section() {
    if (section_code == SectionType::Data) {
        delete data.data_segments;
    }
}

LinkerInputBinary::LinkerInputBinary(const char *filename, List<uint8_t> &data)
        : name(filename),
          data(data.items, data.size_, false),
//          size(data.size_),
          active_function_imports(0),
          active_global_imports(0),
          type_index_offset(0),
          function_index_offset(0),
          imported_function_index_offset(0),
          table_index_offset(0),
          memory_page_count(0),
          memory_page_offset(0),
          table_elem_count(0) {
}


bool LinkerInputBinary::IsFunctionImport(Index index) const {
//	assert(IsValidFunctionIndex(index));
    return index < function_imports.size();
}

bool LinkerInputBinary::IsInactiveFunctionImport(Index index) {
    return IsFunctionImport(index) && !function_imports[index].active;
}

bool LinkerInputBinary::IsValidFunctionIndex(Index index) const {
    return index < function_imports.size() + function_count;
}

Index LinkerInputBinary::RelocateGlobalIndex(Index global_index) {
    Index offset;
    if (global_index >= global_imports.size()) {
        offset = global_index_offset;
    } else {
        GlobalImport &globalImport = global_imports[global_index];
        if (!globalImport.active) {
            int foreignIndex = globalImport.foreign_index;
            offset = globalImport.foreign_binary->global_index_offset;
            LOG_DEBUG("reloc for disabled global import %s : new index = %d + %d\n", globalImport.name.data,
                      foreignIndex, offset);
        } else {
            Index new_index = globalImport.relocated_global_index;
            if (global_index != new_index)
                LOG_DEBUG("reloc for active global import %s: old index = %d, new index = %d\n", globalImport.name.data,
                          global_index, new_index);
            return new_index;

        }

        offset = imported_global_index_offset;
    }
    return global_index + offset;
}

Index LinkerInputBinary::RelocateFuncIndex(Index function_index) {
    Index offset;
    if (!IsFunctionImport(function_index)) {
        // locally declared function call.
        offset = function_index_offset;
//		LOG_DEBUG("func reloc %d + %d\n", function_index, offset);
    } else {
        // imported function call.
        FunctionImport *import = &function_imports[function_index];
        if (!import->active) {
            function_index = import->foreign_index;
            if (!import->foreign_binary) {
                return function_index; // wat?
//                check_silent(import->foreign_binary);
            }
            offset = import->foreign_binary->function_index_offset;// todo function_index ADDED LATER to offset!
            LOG_DEBUG("reloc for resolved import %s : new index = %d + %d\n", import->name.data, function_index,
                      offset);
        } else {
            Index new_index = import->relocated_function_index;
            if (function_index != new_index)
                LOG_DEBUG("reloc for active import %s: old index = %d, new index = %d\n", import->name.data,
                          function_index, new_index);
            return new_index;
        }
    }
    // ok offset can be 0
    return function_index + offset;
}

Index LinkerInputBinary::RelocateTypeIndex(Index type_index) const {
    return type_index + type_index_offset;
}

Index LinkerInputBinary::RelocateMemoryIndex(Index memory_index) const {
    return memory_index + memory_page_offset * 65536;
}

Index LinkerInputBinary::RelocateTable(Index global_index) const {
    if (needs_relocate) todow("RelocateTable")
    return global_index;// shouldn't reach this anyways
}

class Linker {
public:
    WABT_DISALLOW_COPY_AND_ASSIGN(Linker);

    Linker() = default;

    void AppendBinary(LinkerInputBinary *binary) {
        inputs_.add(binary);
    }

    OutputBuffer PerformLink();

    void CreateRelocs();

    void ApplyRelocations(Section *section);

    void ApplyRelocation(Section *section, const wabt::Reloc *r);

private:
    typedef Pair<Offset, Offset> Fixup;

    Fixup WriteUnknownSize();

    void FixupSize(Fixup);

    void WriteSectionPayload(Section *sec);

    void WriteTableSection(const SectionPtrVector &sections);

    void WriteExportSection();

    void WriteElemSection(const SectionPtrVector &sections);

    void WriteMemorySection(const SectionPtrVector &sections);

    void WriteFunctionImport(const FunctionImport &import, Index offset);

    void WriteGlobalImport(const GlobalImport &import);

    void WriteImportSection();

    void WriteFunctionSection(const SectionPtrVector &sections,
                              Index total_count);

    void WriteDataSegment(const DataSegment &segment, Address offset);

    void WriteDataSection(const SectionPtrVector &sections, Index total_count);

    void WriteNamesSection();

    void WriteLinkingSection(uint32_t data_size, uint32_t data_alignment);

//	void WriteRelocSection(BinarySection section_code,
//	                       const SectionPtrVector &sections);

    bool WriteCombinedSection(SectionType section_code,
                              const SectionPtrVector &sections);

    void RemoveRuntimeMainExport();

    void RemoveAllExports();

    void ResolveSymbols();

    void CalculateRelocOffsets();

    void WriteBinary();

    void DumpRelocOffsets();

    MemoryStream stream_;
    List<LinkerInputBinary *> inputs_{};
    ssize_t current_payload_offset_ = 0;

    Section *getSection(LinkerInputBinary *&uniquePtr, SectionType section);

    List<Reloc> CalculateRelocs(LinkerInputBinary *&binary, Section *section);

    List<uint8_t> lebVector(Index value);

};

void Linker::WriteSectionPayload(Section *sec) {
    assert(current_payload_offset_ != -1);
    sec->output_payload_offset = stream_.offset() - current_payload_offset_;
    uint8_t *payload = &sec->binary->data[sec->payload_offset];
    stream_.WriteData(payload, sec->payload_size, "section content");
}

Linker::Fixup Linker::WriteUnknownSize() {
    Offset fixup_offset = stream_.offset();
    WriteFixedU32Leb128(&stream_, 0, "unknown size");
    current_payload_offset_ = stream_.offset();
    return {.first=fixup_offset, .second=(unsigned long) current_payload_offset_};
}

void Linker::FixupSize(Fixup fixup) {
    WriteFixedU32Leb128At(&stream_, fixup.first, stream_.offset() - fixup.second, "fixup size");
}

void Linker::WriteTableSection(const SectionPtrVector &sections) {
    // Total section size includes the element count leb128 which is always 1 in
    // the current spec.
    Index table_count = 1;
    uint32_t flags = WABT_BINARY_LIMITS_HAS_MAX_FLAG;
    Index elem_count = 0;

    for (Section *section: sections) {
        elem_count += section->binary->table_elem_count;
    }

    Fixup fixup = WriteUnknownSize();
    WriteU32Leb128(&stream_, table_count, "table count");
    WriteType(&stream_, wabt::Type::FuncRef);
    WriteU32Leb128(&stream_, flags, "table elem flags");
    WriteU32Leb128(&stream_, elem_count, "table initial length");
    WriteU32Leb128(&stream_, elem_count, "table max length");
    FixupSize(fixup);
}

void Linker::WriteExportSection() {
    Index total_exports = 0;
    for (auto binary: inputs_) {
        total_exports += binary->exports.size();
    }

    Fixup fixup = WriteUnknownSize();
    WriteU32Leb128(&stream_, total_exports, "export count");

    int memories = 0;
    for (auto binary: inputs_) {
        for (const Export &export_: binary->exports) {
            WriteStr(&stream_, export_.name, "export name");

            stream_.WriteU8Enum(export_.kind, "export kind");
            Index index = export_.index;
            switch (export_.kind) {
                case ExternalKind::Func:
                    index = binary->RelocateFuncIndex(index);
                    break;
                case ExternalKind::Global:
                    index = binary->RelocateGlobalIndex(index);
                    break;
                case ExternalKind::Table:
                    index = binary->RelocateTable(index);
                    break;
                case ExternalKind::Memory:
                    memories++;
                    if (memories > 1)
                        error("only one memory allowed");
                    break;
                default:
                    WABT_FATAL("unsupport export type: %d %s\n", static_cast<int>(export_.kind), export_.name.data);
                    break;
            }
            WriteU32Leb128(&stream_, index, "export index");
        }
    }

    FixupSize(fixup);
}

void Linker::WriteElemSection(const SectionPtrVector &sections) {
    Fixup fixup = WriteUnknownSize();

    WriteU32Leb128(&stream_, 1, "segment count");
    if (sections.size() > 1) {
//        todo("MERGE ELEM");
        Index total_elem_count = 0;
        for (Section *section: sections) {
            total_elem_count += section->binary->table_elem_count;
        }
        WriteU32Leb128(&stream_, 0, "table index");
        WriteOpcode(&stream_, Opcode::I32Const);
        WriteS32Leb128(&stream_, 0U, "elem value literal");
        WriteOpcode(&stream_, Opcode::End);
        WriteU32Leb128(&stream_, total_elem_count, "num elements");

        current_payload_offset_ = stream_.offset();
    }

    for (Section *section: sections) {
        ApplyRelocations(section);
        WriteSectionPayload(section);
    }

//    if(sections.size()>0)
    FixupSize(fixup);
}

void Linker::WriteMemorySection(const SectionPtrVector &sections) {
    Fixup fixup = WriteUnknownSize();

    WriteU32Leb128(&stream_, 1, "memory count");

    Limits limits;
    limits.has_max = true;
    for (Section *section: sections) {
        limits.initial += section->data.initial;
    }
    limits.max = limits.initial;
    WriteLimits(&stream_, &limits);

    FixupSize(fixup);
}

void Linker::WriteFunctionImport(const FunctionImport &import, Index offset) {
    WriteStr(&stream_, import.module_name, "import module name");
    WriteStr(&stream_, import.name, "import field name");
    stream_.WriteU8Enum(ExternalKind::Func, "import kind");
    WriteU32Leb128(&stream_, import.type_index + offset, "import signature index");
}

void Linker::WriteGlobalImport(const GlobalImport &import) {
    WriteStr(&stream_, import.module_name, "import module name");
    WriteStr(&stream_, import.name, "import field name");
    stream_.WriteU8Enum(ExternalKind::Global, "import kind");
    WriteType(&stream_, import.type);//, s("import type").data);
    stream_.WriteU8(import.mutable_, "global mutability");
}

void Linker::WriteImportSection() {
    Index num_imports = 0;
    for (auto binary: inputs_) {
        for (const FunctionImport &import: binary->function_imports)
            if (import.active) num_imports++;
        for (const GlobalImport &globalImport: binary->global_imports)
            if (globalImport.active) num_imports++;// function and global imports mixed!
    }

    Fixup fixup = WriteUnknownSize();
    WriteU32Leb128(&stream_, num_imports, "num imports");

    for (auto binary: inputs_) {
        for (const FunctionImport &function_import: binary->function_imports) {
            if (function_import.active) {
                WriteFunctionImport(function_import, binary->type_index_offset);
            }
        }

        for (const GlobalImport &global_import: binary->global_imports) {
            if (global_import.active)
                WriteGlobalImport(global_import);
        }
    }
    FixupSize(fixup);
}

void Linker::WriteFunctionSection(const SectionPtrVector &sections,
                                  Index total_count) {
    Fixup fixup = WriteUnknownSize();

    WriteU32Leb128(&stream_, total_count, "function count");

    for (Section *sec: sections) {
        Index count = sec->count;
        Offset input_offset = 0;
        Index sig_index = 0;
        const uint8_t *start = &sec->binary->data[sec->payload_offset];
        const uint8_t *end = &sec->binary->data[sec->payload_offset + sec->payload_size];
        while (count--) {
            input_offset += ReadU32Leb128(start + input_offset, end, &sig_index);
            WriteU32Leb128(&stream_, sec->binary->RelocateTypeIndex(sig_index), "sig");
        }
    }

    FixupSize(fixup);
}

void Linker::WriteDataSegment(const DataSegment &segment, Address offset) {
    assert(segment.memory_index == 0);
    WriteU32Leb128(&stream_, segment.memory_index, "memory index");
    WriteOpcode(&stream_, Opcode::I32Const);
    uint32_t data_offset = segment.offset + offset;
    check_silent(data_offset >= 0);
//    tracing = 1;
//    tracef("data_offset %u\n", data_offset);
//    WriteU32Leb128(&stream_, data_offset, "offset"); // fails for data_offset >= 8192
    WriteFixedU32Leb128(&stream_, data_offset, "offset");// kf 2022-12 'fixed' above ^^ (?)
    WriteOpcode(&stream_, Opcode::End);
    WriteU32Leb128(&stream_, segment.size, "segment size");
    stream_.WriteData(segment.data, segment.size, "segment data");
}

void Linker::WriteDataSection(const SectionPtrVector &sections, Index total_count) {
    Fixup fixup = WriteUnknownSize();
    WriteU32Leb128(&stream_, total_count, "data segment count");
    for (const Section *sec: sections) {
        if (sec->data.data_segments)
            for (const DataSegment &segment: *sec->data.data_segments) {
                WriteDataSegment(segment, sec->binary->memory_page_offset * WABT_PAGE_SIZE);
            }
    }
    FixupSize(fixup);
}

void Linker::WriteNamesSection() {
    Index total_count = 0;
    for (auto binary: inputs_) {
        for (size_t i = 0; i < binary->debug_names.size(); i++) {
            if (binary->debug_names[i].empty()) {
                continue;
            }
            if (binary->IsInactiveFunctionImport(i)) {
                continue;
            }
            total_count++;
        }
    }

    if (!total_count) {
        return;
    }

    stream_.WriteU8Enum(SectionType::Custom, "section code");
    Fixup fixup = WriteUnknownSize();
    WriteStr(&stream_, "name"s, "custom section name"s);

    stream_.WriteU8Enum(NameSectionSubsection::Function, "subsection code");
    Fixup fixup_subsection = WriteUnknownSize();
    WriteU32Leb128(&stream_, total_count, "element count");

    // Write import names.
    for (auto binary: inputs_) {
        for (size_t i = 0; i < binary->debug_names.size(); i++) {
            if (binary->debug_names[i].empty() || !binary->IsFunctionImport(i)) {
                continue;
            }
            if (binary->IsInactiveFunctionImport(i)) {
                continue;
            }
            WriteU32Leb128(&stream_, binary->RelocateFuncIndex(i), "function index");
            WriteStr(&stream_, binary->debug_names[i], "function name");
        }
    }

    // Write non-import names.
    for (auto binary: inputs_) {
        for (size_t i = 0; i < binary->debug_names.size(); i++) {
            if (binary->debug_names[i].empty() || binary->IsFunctionImport(i)) {
                continue;
            }
            WriteU32Leb128(&stream_, binary->RelocateFuncIndex(i), "function index");
            WriteStr(&stream_, binary->debug_names[i], "function name");
        }
    }

    FixupSize(fixup_subsection);
    FixupSize(fixup);
}

void Linker::WriteLinkingSection(uint32_t data_size, uint32_t data_alignment) {
    stream_.WriteU8Enum(SectionType::Custom, "section code");
    Fixup fixup = WriteUnknownSize();

    WriteStr(&stream_, "linking"s, "linking section name"s);

    if (data_size) {
//    WriteU32Leb128(&stream_, LinkingEntryType::DataSize, "subsection code");
        todo("WriteLinkingSection");
        Fixup fixup_subsection = WriteUnknownSize();
        WriteU32Leb128(&stream_, data_size, "data size");
        FixupSize(fixup_subsection);
    }

    FixupSize(fixup);
}


bool Linker::WriteCombinedSection(SectionType section_code, const SectionPtrVector &sections) {
    if (!sections.size()) {
        return false;
    }

    if (section_code == SectionType::Start && sections.size() > 1) {
        WABT_FATAL("Don't know how to combine sections of type: %s\n",
                   GetSectionName(section_code));
    }

    Index total_count = 0;
    Index total_size = 0;



    // Sum section size and element count.
    for (Section *sec: sections) {
        ApplyRelocations(sec);
        total_size += sec->payload_size;
        total_count += sec->count;
    }

    if (section_code == wabt::SectionType::Data)
        printf("");

    stream_.WriteU8Enum(section_code, "section code");

    current_payload_offset_ = -1;

    switch (section_code) {
        case SectionType::Import:
            WriteImportSection();
            break;
        case SectionType::FuncType:
            WriteFunctionSection(sections, total_count);
            break;
        case SectionType::Table:
            WriteTableSection(sections);
            break;
        case SectionType::Export:
            WriteExportSection();
            break;
        case SectionType::Elem:
            WriteElemSection(sections);
            break;
        case SectionType::Memory:
            WriteMemorySection(sections);
            break;
        case SectionType::Data:
            WriteDataSection(sections, total_count);
            break;
        default: { // just append

            // Total section size includes the element count leb128.
            total_size += U32Leb128Length(total_count);

            // Write section to stream.
            WriteU32Leb128(&stream_, total_size, "section size");
            WriteU32Leb128(&stream_, total_count, "element count");
            current_payload_offset_ = stream_.offset();
            for (Section *sec: sections)
                WriteSectionPayload(sec);
        }
    }
    return true;
}


struct FuncInfo {
    FuncInfo(const Func *export_, LinkerInputBinary *binary) : func(export_), binary(binary) {}

    const Func *func{};
    LinkerInputBinary *binary{};
};

template<typename Collection, typename Element>
void remove_from_efin_vec(Collection &c, const Element &e) {
    c.erase(std::remove(c.begin(), c.end(), e), c.end());
};


void Linker::RemoveRuntimeMainExport() {
    for (auto &bin: inputs_) {
        short pos = -1;
        auto is_runtime = contains(bin->name, "wasp");
        for (Export &ex: bin->exports) {
            pos++;
            if (is_runtime and ex.name == "wasp_main")
                bin->exports.remove(pos);// ex
            if (is_runtime and ex.name == "_start") // not
                bin->exports.remove(pos);// USE wasp _start to print the result to wasi
        }
    }
}

void Linker::RemoveAllExports() {// except _start for stupid wasmtime:
//  (export "nil_name" (global 1))
// 1: command export 'nil_name' is not a function
    for (auto &bin: inputs_) {
        short pos = -1;
        for (Export &ex: bin->exports) {
            pos++;
            if (ex.kind != ExternalKind::Func)continue;
            while (bin->exports.size_ > 0 and not(ex.name == "wasp_main" or ex.name == "_start" or ex.name == "main")) {
                if (ex.kind != ExternalKind::Func)break;
                if (!bin->exports.remove(pos))break;
            }
        }
    }
}

void Linker::ResolveSymbols() {
    // Create hashmap of all exported symbols from all inputs.

    // ⚠️ all indices in func_map go into the function CODE section (LATER offset by the import count!)
    BindingHash func_map;
    Map<String, int> name_map = {.capacity=10000};// from debug section to
    // ⚠️ all indices here map into into following LIST, not into wasm!! so TWO indirections!
    BindingHash export_map;// of all kinds
//	BindingHash private_map;
    Map<String, FuncInfo *> private_map = {.capacity=10000};
    List<ExportInfo> export_list;
    List<ExportInfo> globals_export_list;
    List<FuncInfo> func_list;// internal index identical to func_map index!!
//    List<FuncInfo> import_list;//
    Map<String, FunctionImport *> import_map = {.capacity=10000};// currently only used to find duplicates FuncInfo


// binary->functions not filled yet!

    int memories = 0;
    for (auto binary: inputs_) {
        printf("!!!!!!!!!!!   %s #%lu !!!!!!!!!!!\n", binary->name, binary->debug_names.size());
        uint64 nr_imports = binary->function_imports.size();

        // FIND DUPLICATE imports and exports (no other purpose for now!)
        int import_index = 0;
        for (auto &import: binary->function_imports) {
            if (tracing)
                printf("%s import %s index %d\n", binary->name, import.name.data, import.index);
            if (import_map.has(import.name)) {
                warn("DUPLICATE import "s + import.name);// todo: check signatures
                import.active = false;
                FunctionImport *&previous_import = import_map[import.name];
                import.foreign_binary = previous_import->binary;
                import.foreign_index = previous_import->index;
                printf("previous_import: %s %d\n", previous_import->binary->name, previous_import->index);
//                auto *hack = new ExportInfo(new Export{.index=previous_import->sig_index}, previous_import->binary);
//                import.linked_function= hack;
                binary->active_function_imports--;
            } else {
                if (import.foreign_binary)
                    error("my a");
                import.binary = binary; // todo earlier
                import.index = import_index++;// only increase if active / not duplicate
                import_map.add(import.name, &import);
            }
        }

        short pos = -1;
        for (Export &_export: binary->exports) {
            pos++;
            if (export_map.FindIndex(_export.name) != kInvalidIndex) {
                warn("duplicate export name "s + _export.name);// Ignoring
                binary->exports.remove(pos);// todo: careful, does iterator skip an element now??
            }
            if (_export.kind == ExternalKind::Memory) {
                memories++;
                if (memories > 1)
                    binary->exports.remove(pos);
            }
        }

        for (Export &_export: binary->exports) {// todo: why not store index directly?
            if (tracing)
                printf("%s export kind %d '%s' index %d\n", binary->name, _export.kind, _export.name.data,
                       _export.index);
            if (_export.kind == wabt::ExternalKind::Global) {
                globals_export_list.add(ExportInfo(&_export, binary));
                export_map.emplace(_export.name, Binding(globals_export_list.size() - 1));
            } else if (_export.kind == ExternalKind::Func) {
                Func &func = binary->functions[_export.index - nr_imports];
                uint64 position = export_list.size();
                export_list.add(ExportInfo(&_export, binary));
                if (not func.name.data)
                    func.name = _export.name;
                if (func.name.length > 0) {
                    export_map.emplace(func.name, Binding(position));
                    String demangled = extractFuncName(func.name);
                    if (func.name != demangled and not export_map.contains(demangled)) {
                        export_map.emplace(demangled, Binding(position));
                    }
                }
            } else {
                export_list.add(ExportInfo(&_export, binary));
                warn("ignore export of kind %d %s"s % (short) _export.kind % GetKindName(_export.kind));
            }
        }
        for (const Func &func: binary->functions) {// only those with code, not imports
            if (not empty(func.name)) {
                if (tracing)
                    printf("func.name %s, func.index %d\n", func.name.data, func.index);
//				check(func_list.size()==func.index);
                func_map.emplace(func.name, func.index);
                private_map[String(func.name)] = new FuncInfo{&func, binary};
            }
            func_list.add(FuncInfo(&func, binary));
        }
//		for (const Global &func: binary->globals) todo

        // wasp.wasm currently has no binary->debug_names (only exports!) so ignore for now
//		for (int i = 0; i < binary->debug_names.size(); ++i) {
//			String &name = binary->debug_names[i];
//			printf("DEBUG func.name %s, func.index %d\n", name.data, i);
//			int true_index = i - binary->function_index_offset;
//			if (i < 0) {
//				continue;// debug name of import doesn't matter here!
//
//			} else {
//				Func &func1 = binary->functions[true_index];
//				check(func1.index == i);
//				func_map.emplace(name, i);
//			}
//		}
    }
//	check(export_list[export_map.FindIndex("_Z5atoi0PKc")].export_->index == 18);// todo !!!

    // Iterate through all imported globals and functions resolving them against exported ones.
    for (LinkerInputBinary *&binary: inputs_) {
        if (not binary->needs_relocate)continue;
        for (GlobalImport &global_import: binary->global_imports) {
            String &name = global_import.name;
            Index export_number = export_map.FindIndex(name);
            if (export_number == kInvalidIndex) {
                printf("Ignoring unresolved global import %s\n", name.data);
                continue;
            }
            ExportInfo &export_info = globals_export_list[export_number];
            String export_name = export_info.export_->name;
            Index export_index = export_info.export_->index;
            printf("LINKING GLOBAL %s import #%d to export #%d %s \n", name.data, global_import.foreign_index,
                   export_index, export_name.data);
            global_import.active = false;
            global_import.foreign_binary = export_info.binary;
            global_import.foreign_index = export_index;
            binary->active_global_imports--;
        }


        for (FunctionImport &import: binary->function_imports) {
            String &name = import.name;
            Index export_number = export_map.FindIndex(name);
            if (export_number != kInvalidIndex) {
                // We found the symbol exported by another module.
                ExportInfo &export_info = export_list[export_number];
                // TODO ⚠️ verify the foreign function has the correct signature.
                Index export_index = export_info.export_->index;
                int nr_imports = export_info.binary->function_imports.size();
                Func &exported = export_info.binary->functions[export_index - nr_imports];
                Index old_index = import.type_index;
                import.active = false;
                import.foreign_binary = export_info.binary;
                import.linked_function = &export_info;
                import.foreign_index = export_index;
//				Index new_Index; // see RelocateFuncIndex(); ⚠️ we can only calculate the new index once ALL imports are collected!
//				import.relocated_function_index = export_number; see RelocateFuncIndex()
                binary->active_function_imports--;// never used!?
                char *import_name = import.name;
                char *export_name = exported.name;
                printf("LINKED %s:%s import #%d %s to export #%d %s relocated_function_index %d \n", binary->name,
                       name.data, old_index,
                       import_name, export_index, export_name, export_number);
            } else {
                // todo all this is done in RelocateFuncIndex !
                // link unexported functions, because clang -Wl,--relocatable,--export-all DOES NOT preserve EXPORT wth
                Index func_index = func_map.FindIndex(name);
                if (func_index == kInvalidIndex) {
                    if (not contains(wasi_function_list, name.data)) {
//                        warn("unresolved import: %s  ( keep in case it's used inside binary) "s % name);
                        warn("unresolved import: %s  ( setting inactive due to wasi ) "s % name);
                        import.active = false;
                        binary->active_function_imports--;// never used!?
                    }
                    continue;
                }
//				check(func_list[func_index].func);
                FuncInfo funcInfo = *private_map[name];
//				FuncInfo &funcInfo = func_list[func_index];
                check_eq(name, funcInfo.func->name.data);
                import.active = false;
                import.foreign_binary = funcInfo.binary;
                import.foreign_index = funcInfo.func->index;
                binary->active_function_imports--;
                printf("LINKED unexported function to import: %s\n", name.data);
            }
        }
    }
}

void Linker::CalculateRelocOffsets() {
    Index memory_page_offset = 0;
    Index type_count = 0;
    Index global_count = 0;
    Index function_count = 0;
    Index my_function_count = 0;
    Index table_elem_count = 0;
    Index total_function_imports = 0;
    Index total_global_imports = 0;

    for (LinkerInputBinary *&binary: inputs_)
        for (Section *&sec: binary->sections)
            if (sec->section_code == SectionType::Data) {
                binary->memory_data_start = sec->data.data_segments->first().offset;
                break;
            }


    for (LinkerInputBinary *&binary: inputs_) {
        binary->delta = my_function_count; // offset all functions if not mapped to import

        // The imported_function_index_offset is the sum of all the function
        // imports from objects that precede this one.  i.e. the current running
        // total.
        binary->imported_function_index_offset = total_function_imports;
        binary->imported_global_index_offset = total_global_imports;
        if (binary->memory_data_start < memory_page_offset) {
            // memory_page_count = page_limits->initial
            // WE MAY HAVE OVERLAPPING MEMORY! especially when one component assumes memory growth! todo later: shared HEAP_END ?
            binary->memory_page_offset = memory_page_offset;
        }

        size_t delta = 0;// number of functions removed for this binary ( imports linked )
        for (size_t i = 0; i < binary->function_imports.size(); i++) {
            if (!binary->function_imports[i].active) delta++;
            else binary->function_imports[i].relocated_function_index = total_function_imports + i - delta;
        }
        int global_delta = 0;
        for (size_t i = 0; i < binary->global_imports.size(); i++) {
            if (!binary->global_imports[i].active) global_delta++;
            else binary->global_imports[i].relocated_global_index = total_global_imports + i - global_delta;
        }

        memory_page_offset += binary->memory_page_count;
        total_function_imports += binary->active_function_imports;
        total_global_imports += binary->active_global_imports;
        my_function_count += binary->active_function_imports;
        my_function_count += binary->function_count;
    }

    for (LinkerInputBinary *&binary: inputs_) {
        binary->table_index_offset = table_elem_count;
        table_elem_count += binary->table_elem_count;
        for (Section *&sec: binary->sections) {
            switch (sec->section_code) {
                case SectionType::Type:
                    binary->type_index_offset = type_count;
                    type_count += sec->count;
                    break;
                case SectionType::Global: {
                    uint64 new_offset = total_global_imports - sec->binary->global_imports.size() + global_count;
                    binary->global_index_offset = new_offset;
                }
                    global_count += sec->count;
                    break;
                case SectionType::FuncType: {
                    int new_offset = total_function_imports - sec->binary->function_imports.size() + function_count;
                    binary->function_index_offset = new_offset;
                    function_count += sec->count;
                }
                    break;
                default:
                    break;
            }
        }
    }
}

void Linker::WriteBinary() {
    // Find all the sections of each type.
    SectionPtrVector sections[kBinarySectionCount];
    for (LinkerInputBinary *&binary: inputs_) {
        for (Section *&sec: binary->sections) {
            Section *section = sec;
            int sectionCode = (int) sec->section_code;
            SectionPtrVector &sec_list = sections[sectionCode];
            sec_list.add(section);
        }
    }

    // Write the final binary.
    stream_.WriteU32(WABT_BINARY_MAGIC, "WABT_BINARY_MAGIC");
    stream_.WriteU32(WABT_BINARY_VERSION, "WABT_BINARY_VERSION");
    // Write known sections first.
    for (size_t i = (int) FIRST_KNOWN_SECTION; i < kBinarySectionCount; i++) {
        WriteCombinedSection((SectionType) i, sections[i]);
    }
    WriteNamesSection();
    /* Generate a new set of reloction sections */
//	if (s_relocatable) {
//		WriteLinkingSection(0, 0);
//		for (size_t i = (int) FIRST_KNOWN_SECTION; i < kBinarySectionCount; i++) {
//			WriteRelocSection(static_cast<BinarySection>(i), sections[i]);
//		}
//	}
}

void Linker::DumpRelocOffsets() {
    for (auto binary: inputs_) {
        bool needs_relocate = false;
        needs_relocate = needs_relocate or binary->type_index_offset != 0;
        needs_relocate = needs_relocate or binary->memory_page_offset != 0;
        needs_relocate = needs_relocate or binary->function_index_offset != 0;
        needs_relocate = needs_relocate or binary->global_index_offset != 0;
        needs_relocate = needs_relocate or binary->imported_function_index_offset != 0;
        needs_relocate = needs_relocate or binary->imported_global_index_offset != 0;
        if (needs_relocate) {
            LOG_DEBUG("Relocation info for: %s\n", binary->name);
            LOG_DEBUG(" - type index offset       : %d\n", binary->type_index_offset);
            LOG_DEBUG(" - mem page offset         : %d\n", binary->memory_page_offset);
            LOG_DEBUG(" - function index offset   : %d\n", binary->function_index_offset);
            LOG_DEBUG(" - global index offset     : %d\n", binary->global_index_offset);
            LOG_DEBUG(" - imported function offset: %d\n", binary->imported_function_index_offset);
            LOG_DEBUG(" - imported global offset  : %d\n", binary->imported_global_index_offset);
            if (not binary->needs_relocate)
                error("Binary %s markes as needs_relocate=false, but context forces relocations (imports…)."s %
                      String(binary->name));
        } else {
            LOG_DEBUG("Relocation info for: %s … NONE! Keeping binary as is.\n", binary->name);
        }
    }
}

void Linker::CreateRelocs() {
    for (LinkerInputBinary *&binary: inputs_) {
        if (not binary->needs_relocate) {
            trace("Skipping CreateRelocs for library "s + binary->name);
            continue;
        }

        Section *section = getSection(binary, SectionType::Code);
        if (!section)return;
        List<Reloc> relocs = CalculateRelocs(binary, section);
        for (Reloc &reloc: relocs)
            section->relocations.add(reloc);
//		if(!section->data.data_segments)
//			continue;
//		for (const DataSegment &segment: *section->data.data_segments) {
//		}
    }
}

OutputBuffer Linker::PerformLink() {
    RemoveRuntimeMainExport();
    CalculateRelocOffsets();
    ResolveSymbols(); // LINK import to exports …  binary->active_function_imports--
    CalculateRelocOffsets();// again: might be negative if import is removed!
    CreateRelocs();
    DumpRelocOffsets();
//    if(final_product)
    RemoveAllExports();
    WriteBinary();
//	check(inputs_[1]->sections[4]->data.data_segments->at(0).data[0]=='*');
    return stream_.output_buffer();
}

Section *Linker::getSection(LinkerInputBinary *&binary, SectionType section) {
    for (Section *&sec: binary->sections) {
        if (sec->section_code == section)return sec;
    }
    return nullptr;
}

// relocs can either be provided as custom section, or inferred from the linker.
List<Reloc> Linker::CalculateRelocs(LinkerInputBinary *&binary, Section *section) {
    List<Reloc> relocs;
    List<uint8_t> &binary_data = binary->data;// LATER plus section_offset todo shared Code view
    int length = binary_data.size();
//    uint8_t *binary_data = binary->data;// LATER plus section_offset todo shared Code view
//    int length = binary->size;
    size_t section_offset = section->offset;// into binary data
    int current_offset = section_offset;
    // #code_index =
    int64 function_count = unsignedLEB128(binary_data, length, current_offset, true);
//	DataSegment section_data = ;// *pSection->data.data_segments->data();
//    Index binary_delta = binary->delta;
    size_t section_size = section->size;
//	Index import_border = binary->imported_function_index_offset;// todo for current binary or for ALL?
    uint64 function_imports_count = binary->function_imports.size();
    uint64 old_import_border = function_imports_count;
    bool begin_function = true;
    int code_index = 0;
    int call_index = binary->imported_function_index_offset + code_index;
    String current_name = "?";

    Opcodes last_opcode;// to debug
//    int fun_start = 0;
    int fun_end = length;
//    Reloc *patch_code_block_size;
    println("PARSING FUNCTION SECTION");
    while (code_index < function_count && current_offset < length and
           current_offset - section_offset < section_size) {// go over ALL functions! ignore 00
        int64 last_const = 0;// use stack value for i32.load index or offset?
        if (begin_function) {
//            if (binary_data[current_offset - 1] != 0x0b and binary_data[current_offset - 1] != 0x0c)
//                breakpoint_helper;
            if (code_index > 0 and last_opcode != end_block)
                breakpoint_helper;
            begin_function = false;
            auto func1 = binary->functions[code_index];
            current_name = func1.name;
            if (!current_name.data or current_name.empty()) {
                //  ƒ146 empty because NO EXPORT in wasp-runtime.wasm why? some inline shit?
//          (func (;146;) (type 1) (param i32) (result i32)
//    local.get 0
//    i32.const 10
//    local.get 0
//    i32.const 10
//    i32.lt_s
//    select)
//                ƒ467 _start empty because deleted?
                warn("current_name.empty ƒ"s + call_index + "!");
                current_name = "ERR";
            }
//            fun_start = current;// use to create fun_length patches iff block needs leb insert
            int fun_length = unsignedLEB128(binary_data, length, current_offset, true);
            // length of ONE function code block, but don't proceed yet:
            fun_end = current_offset + fun_length;
            int local_types = unsignedLEB128(binary_data, length, current_offset, true);// PARAMS ARE NOT local vars!
            if (local_types > 100) {// todo: just warn after thoroughly tested
                // it DOES HAPPEN, e.g. in pow.wasm
                warn("suspiciously many local_types. parser out of sync? %s index %d types: %d\n"s % current_name %
                     func1.index % local_types);
            }// each type comes with a count e.g. (i32, 3) == 3 locals of type i32
            if (current_name.data /*and tracing*/)// else what is this #2 test/merge/main2.wasm :: (null)
                tracef("code#%d -> ƒ%d %s :: %s (#%d) len %d\n", code_index, call_index, binary->name,
                       current_name.data, local_types, fun_length);
//            if (call_index == 144)
//                tracing = true;
//            else if (tracing)
//                printf("#%d -> #%d (#%d)\n", current_fun, call_index, local_types);
            current_offset += local_types * 2;// type + nr
        }
        byte b = binary_data[current_offset++];
        Opcodes op = (Opcodes) b;
//        if (b == 0)
//            breakpoint_helper
//        if (call_index == 332)// op == i32_store and
//            breakpoint_helper
        Opcode opcode = Opcode::FromCode(b);
        if (current_offset >= fun_end) {
            begin_function = true;
            code_index++;
            call_index++;
            if (b != end_block and b != 0) {
                error("unexpected opcode at function end %x "s % b + opcode.GetName());
                breakpoint_helper;
            } else last_opcode = end_block;
            call_index = function_imports_count + code_index;
//			trace("begin_function %d\n", current_fun);
            continue;
        }
        int arg_bytes = opcode_args[op];
        if (op == call_) {
            uint64 index = unsignedLEB128(binary_data, length, current_offset, true);
            Index neu = binary->RelocateFuncIndex(index);
            if (index != neu) {
                Reloc reloc(wabt::RelocType::FuncIndexLEB, current_offset - section_offset - lebByteSize(index), neu);
                relocs.add(reloc);
            }
#ifdef DEBUG
            String function_name;
            if (index < old_import_border) {
                FunctionImport &anImport = binary->function_imports[index];
                function_name = "IMPORT "s + anImport.name;
            } else {
                Func &callee = binary->functions[index - binary->function_imports.size()];// old index + offset!
                function_name = callee.name;
            }
            if (tracing)
                printf("CALL %s ƒ%d %s calls %s $%llu -> %d\n", binary->name, call_index, current_name.data,
                       function_name.data, index,
                       neu);
#endif
        } else if (op == global_get || op == global_set) {
            short index = unsignedLEB128(binary_data, length, current_offset, false);
            Index neu = binary->RelocateGlobalIndex(
                    index);// todo : get foreign_index! how/why does it work for functions!?!
            if (index != neu) {
                Reloc reloc(wabt::RelocType::GlobalIndexLEB, current_offset - section_offset, neu);
                relocs.add(reloc);
            }
            current_offset += lebByteSize((uint64) index);
        } else if (op >= i32_load and op <= i32_store_16) {
            short alignment = unsignedLEB128(binary_data, length, current_offset, true);
            int64 offset = unsignedLEB128(binary_data, length, current_offset, false);
            last_const = offset;
            Index neu = binary->RelocateMemoryIndex(offset);
            if (offset != neu) {
                // todo when is MemoryAddressI32 … used??
                Reloc reloc(wabt::RelocType::MemoryAddressLEB, current_offset - section_offset, neu);
                relocs.add(reloc);
//                short diff = 0;// only NOPs now. later: cleaner solution, NOT HERE!. lebByteSize(neu) - lebByteSize(offset);// nop all for now!
//                Reloc patch_code_block_size{RelocType::PatchCodeBlockSize, (Index) fun_start - section_offset, (Index) diff};// add original fun_length later
//                relocs.add(patch_code_block_size);
            }
            current_offset += lebByteSize((uint64) offset);
        } else {
            if (arg_bytes > 0)
                current_offset += arg_bytes;
            else if (arg_bytes == datax) {
                last_const = unsignedLEB128(binary_data, length, current_offset, true);// alignment
                last_const = unsignedLEB128(binary_data, length, current_offset, true);// offset
            } else if (arg_bytes == leb) {
                last_const = unsignedLEB128(binary_data, length, current_offset,
                                            true);// start passed as reference will be MODIFIED!!
            } // auto variable argument(s)
            else if (arg_bytes == -1) {
                printf("previous opcode 0x%x %d “%s” ?\n", last_opcode, last_opcode,
                       Opcode::FromCode(last_opcode).GetName());
                printf("UNKNOWN OPCODE ARGS 0x%x %d “%s” length: %d?\n", op, op, opcode.GetName(), arg_bytes);
                error("UNKNOWN OPCODE");
            }
        }
        if (tracing)
            printf("ƒ%d OPCODE 0x%x %d “%s” last_const=%lld  length: %d? \n", call_index, op, op, opcode.GetName(),
                   last_const,
                   arg_bytes);
        last_opcode = op;
    }
    return relocs;
}


Code &code(List<uint8_t> bin) {
    return *new Code(bin.data(), bin.size());
}

Code merge_files(List<String> infiles) {
    Linker linker;
    Result ok = Result::Ok;
    for (const String &input_filename: infiles) {
        List<uint8_t> file_data;
        ok = ReadFile(input_filename.data, &file_data);
        if (!ok)continue;
        auto binary = new LinkerInputBinary(input_filename.data, file_data);
        linker.AppendBinary(binary);
        ReadBinaryLinker(binary);
    }
    return code(linker.PerformLink().data);
}


Code &merge_binaries(List<Code *> binaries) {
//	opcode_args[global_get] = leb;
//	check(opcode_args[global_get] == leb) // todo what kind of dark bug is that???
    Linker linker;
    if (binaries.size() == 1)
        return *binaries.items[0];
    for (Code *codep: binaries) {
        Code &code = *codep;
        List<uint8_t> file_data(code.data, code.data + code.length);
        const char *source = "<code>";
        if (not code.name.empty()) source = code.name.data;
        LinkerInputBinary *binary = new LinkerInputBinary(source, file_data);
        binary->needs_relocate = code.needs_relocate;
        ReadBinaryLinker(binary);
//		if (binary->filename == "main.wasm"s) {
//			printf("currently no exports! they'd mess with library!\n");
//			binary->exports.clear();
//			check(binary->exports.size()==0);
//		}
        linker.AppendBinary(binary);
    }
    const OutputBuffer &out = linker.PerformLink();
    return code(out.data);// data already copied, no need to .clone();
}

Code &merge_binaries(Code &main, Code &lib) {
    List<Code *> binaries;
    binaries.add(&main);
    binaries.add(&lib);
    return merge_binaries(binaries);
}

void merge_files(int argc, char **argv) {
    List<String> infiles;
    while (argc-- > 0)
        infiles.add(argv[argc]);
    merge_files(infiles);
}


// https://github.com/hyperledger-labs/solang/commit/7bb623bc864106c70b209aa8ec4dfe15ac262b68
// section is 5 (Memory for all relocations, even func-call! :( )
// match meta_data_version {1 | 2 => (),
void Linker::ApplyRelocations(Section *section) {
    if (section->relocations.size() <= 0) {
        return;
    }
    LOG_DEBUG("ApplyRelocations: %s\n", GetSectionName(section->section_code));
    // Perform relocations in-place.
    for (const Reloc &reloc: section->relocations) {
        ApplyRelocation(section, &reloc);
    }
}


void Linker::ApplyRelocation(Section *section, const wabt::Reloc *r) {
    // whole section will be written to stream, so don't write to stream here!
    LinkerInputBinary *binary = section->binary;
    if (not binary->needs_relocate)
        error("binary->needs_relocate marked false, but got a reloc!");

    const List<uint8_t> &immutable_data = binary->data; // if you insert, other sections mess up!
//    const List<uint8_t> &immutable_data = List(binary->data, binary->size);// if you insert, other sections mess up!
    uint8_t *section_start = (uint8_t *) &immutable_data[section->offset];// changing int values (offsets) is ok
    uint8_t *section_end = section_start + section->size;// safety to not write outside bounds
// 🪩
    Index cur_value = 0, new_value = -1;
    // todo: what if value at reloc location is not LEB ? does this ever happen?
    int leb_bytes = wabt::ReadS32Leb128(section_start + r->offset, section_end, &cur_value);
    while (leb_bytes-- > 1) *(section_start + r->offset + leb_bytes) = 0x01; // NOPs to delete the old value, keep one
//    bool write_leb = false;
    switch (r->type) {
        // todo INSERT if leb > old value for all types!  we do have &binary->data as vector so it's easy! … or not ;) :
        // todo add old value to Reloc! BUT: we can't expect it!
        // todo add the func start to Reloc, to patch immediately! via:
        // todo general patchLEB(data,offset,old,new,func_start,section->payload_size) function
        // todo general patchLEB(data,reloc,new,section->payload_size) function
        case RelocType::PatchCodeBlockSize:
            new_value = r->index + cur_value;
            break;
        case RelocType::FuncIndexLEB:
            new_value = binary->RelocateFuncIndex(cur_value);
            break;
        case RelocType::TypeIndexLEB:
            new_value = binary->RelocateTypeIndex(cur_value);
            WriteFixedU32Leb128Raw(section_start + r->offset, section_end, new_value);
            new_value = -1;// WriteFixed vs Write… !
            break;
        case RelocType::TableIndexSLEB:
            new_value = cur_value + binary->table_index_offset;
            WriteFixedU32Leb128Raw(section_start + r->offset, section_end, new_value);
            new_value = -1;// WriteFixed vs Write… !
            break;
        case RelocType::MemoryAddressLEB: {
            new_value = binary->RelocateMemoryIndex(cur_value);
            break;
        }
        case RelocType::GlobalIndexLEB:
            new_value = binary->RelocateGlobalIndex(cur_value);
            break; // versus :
        case RelocType::GlobalIndexI32:
        case RelocType::TableIndexI32:
        case RelocType::MemoryAddressSLEB:
        case RelocType::MemoryAddressI32:
        case RelocType::FunctionOffsetI32:
        case RelocType::SectionOffsetI32:
//		case RelocType::FuncIndexLEB:
//				case RelocType::EventIndexLEB:
        case RelocType::MemoryAddressRelSLEB:
        case RelocType::TableIndexRelSLEB:
        case RelocType::MemoryAddressLEB64:
        case RelocType::MemoryAddressSLEB64:
        case RelocType::MemoryAddressI64:
        case RelocType::MemoryAddressRelSLEB64:
        case RelocType::TableIndexSLEB64:
        case RelocType::TableIndexI64:
        case RelocType::TableNumberLEB:
        case RelocType::MemoryAddressTLSSLEB:
        case RelocType::MemoryAddressTLSI32:
        case RelocType::TagIndexLEB:
            WABT_FATAL("unhandled relocation type: %d\n", r->type);// GetRelocTypeName(r->type));
            // uh much to do!
    }
    if (new_value >= 0) {
        // THIS Write only makes sense for LEB types!
        short current_size = lebByteSize((uint64) cur_value);
        short new_size = lebByteSize((uint64) new_value);
        if (new_size > current_size) {
            uint8_t noper = *(section_start + r->offset + current_size);
            if (noper != nop_) todow(
                    "grow big leb %d >> %d (%d bytes > %d leb bytes)"s % new_value % cur_value % new_size %
                    current_size);
        }// memory messed up by now
        WriteU32Leb128Raw(section_start + r->offset, section_end, new_value);
    }

    //			data.insert(data.begin() + section->offset + r->offset, new_value);// this messes up the DATA section somehow!
//			List<unsigned char> neu = lebVector(new_value);//  Code((int64)new_value);
//			section->size += neu.size();
//			section->payload_size += neu.size();
//			data.insert(data.begin() + section->offset + r->offset, neu.begin(), neu.end());
}

List<uint8_t> Linker::lebVector(Index value) {
    List<uint8_t> neu;
    Code &leb128 = unsignedLEB128((int64) value);
    for (auto b: leb128)
        neu.add((uint8_t) b);
    return neu;
}

