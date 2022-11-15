//
// Created by me on 07.12.20.
//

#include "wasm_merger_own.h"

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
#include "Map.h"
#include "wasm_patcher.h"
//#include "wasm_reader.h"
#include "Code.h"

typedef unsigned char *bytes;


Code createSection(Sections sectionType, Code data);

Code &code(std::vector<uint8_t> vector1);

using namespace wabt;
using namespace wabt::link;
//using wabt::link::LinkerInputBinary;
//using wabt::link::Section;
//using wabt::Reloc;
//using wabt::RelocType;
//using wabt::WriteU32Leb128Raw;


//String s(String s) {
//	return String(s.data(),s.size());
//}
String &s(String &x) { return x; }

String s(String x) { return x; }

//
//String::String(const String str):String(str.data){
//	// defeats the purpose of using a lightweight unicode-aware String class
//}

#define FIRST_KNOWN_SECTION BinarySection::Type
//#define LOG_DEBUG(fmt, ...) if (s_debug) s_log_stream->Writef(fmt, __VA_ARGS__);
#define LOG_DEBUG(fmt, ...) if (s_debug) printf(fmt, __VA_ARGS__);

int unknown_opcode_length_TODO = -1;
const int leb = -2;// special marker for varlength leb argument ( i32.const … )
int heaptype = -3;
int block_index = leb;
int u32_type = 4;
int i32_type = 4;
// https://github.com/WebAssembly/function-references/blob/master/proposals/function-references/Overview.md#local-bindings
std::map<short, int> opcode_args = { // BYTES used by wasm op AFTER the op code (not the stack values! e.g. 4 bytes for f32.const )
		{nop,                 0}, // useful for relocation padding call 1 -> call 10000000
		{block,               leb},
		{loop,                0},
		{if_i,                0},// precede by i32 result}, follow by i32_type {7f}
		{elsa,                0},
		{return_block,        0},

		// EXTENSIONS:
		{try_,                block_index},
		{catch_,              block_index},
		{throw_,              block_index},
		{rethrow_,            block_index},
		{br_on_exn_,          block_index}, // branch on exception

		{end_block,           0}, //11
		{br,                  block_index},
		{br_if,               block_index},
		{br_table,            block_index},
		{return_block,        block_index},
		{function_call,       block_index},

		// EXTENSIONS:
		{call_ref,            u32_type},
		{return_call_ref,     u32_type},
		{func_bind,           u32_type},// {type $t} {$t : u32_type
		{let_local,           -1}, // {let <bt> <locals> {bt : blocktype}, locals : {as in functions}

		{drop,                0}, // pop stack
		{select_if,           2}, // a?b:c ternary todo: use!
		{select_t,            1}, // extension … ?

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
		{table_get,           -1},
		{table_set,           -1},

		{i8_load,             0}, //== 𝟶𝚡𝟸𝙳}, i32.load𝟪_u
		{i16_load,            0}, //== 𝟶𝚡𝟸𝙳}, i32.load𝟪_u
		{i32_load,            0},// load word from i32 address
		{f32_load,            0},
		{i32_store,           0},// store word at i32 address
		{f32_store,           0},
		// todo : peek 65536 as float directly via opcode
		{i64_load,            0}, // memory.peek memory.get memory.read
		{i64_store,           0}, // memory.poke memory.set memory.write
		{i32_store_8,         0},
		{i32_store_16,        0},
		{i8_store,            0},
		{i16_store,           0},

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
		{i64_s𝗁l,             0},
		{i64_s𝗁r_s,           0},
		{i64_s𝗁r_u,           0},
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
int unsignedLEB128(bytes section_data, int length, int &start) {
	int n = 0;
	short shift = 0;
	do {
		byte b = section_data[start++];
		n = n | (((long) (b & 0x7f)) << shift);
		if ((b & 0x80) == 0)break;
		shift += 7;
	} while (start < length);
	return n;
}


int unsignedLEB128(std::vector<byte> section_data, int length, int &start) {
	int n = 0;
	short shift = 0;
	do {
		byte b = section_data[start++];
		n = n | (((long) (b & 0x7f)) << shift);
		if ((b & 0x80) == 0)break;
		shift += 7;
	} while (start < length);
	return n;
}

int unsignedLEB128(std::vector<byte> section_data, int length, int &start_reference, bool advance) {
	if (advance)return unsignedLEB128(section_data, length, start_reference);
	int start = start_reference;// value
	return unsignedLEB128(section_data, length, start);// keep start_reference untouched!
}

int unsignedLEB128(Code section_data, int length, int &start) {
	int n = 0;
	short shift = 0;
	do {
		byte b = section_data[start++];
		n = n | (((long) (b & 0x7f)) << shift);
		if ((b & 0x80) == 0)break;
		shift += 7;
	} while (start < length);
	return n;
}

static bool s_debug = true;

Section::Section()
		: binary(nullptr),
		  section_code(BinarySection::Invalid),
		  size(0),
		  offset(0),
		  payload_size(0),
		  payload_offset(0),
		  count(0),
		  output_payload_offset(0) {
	ZeroMemory(data);
}

Section::~Section() {
	if (section_code == BinarySection::Data) {
		delete data.data_segments;
	}
}

LinkerInputBinary::LinkerInputBinary(const char *filename, const std::vector<uint8_t> &data)
		: name(filename),
		  data(data),
		  active_function_imports(0),
		  active_global_imports(0),
		  type_index_offset(0),
		  function_index_offset(0),
		  imported_function_index_offset(0),
		  table_index_offset(0),
		  memory_page_count(0),
		  memory_page_offset(0),
		  table_elem_count(0) {}


bool LinkerInputBinary::IsFunctionImport(Index index) {
//	assert(IsValidFunctionIndex(index));
	return index < function_imports.size();
}

bool LinkerInputBinary::IsInactiveFunctionImport(Index index) {
	return IsFunctionImport(index) && !function_imports[index].active;
}

bool LinkerInputBinary::IsValidFunctionIndex(Index index) {
	return index < function_imports.size() + function_count;
}

Index LinkerInputBinary::RelocateFuncIndex(Index function_index) {
	Index offset;
	if (!IsFunctionImport(function_index)) {
		// locally declared function call.
		offset = function_index_offset;
		LOG_DEBUG("func reloc %d + %d\n", function_index, offset);
	} else {
		// imported function call.
		FunctionImport *import = &function_imports[function_index];
		if (!import->active) {
			function_index = import->foreign_index;
			offset = import->foreign_binary->function_index_offset;
			LOG_DEBUG("reloc for disabled import %s : new index = %d + %d\n", import->name.data, function_index, offset);
		} else {
			Index new_index = import->relocated_function_index;
			LOG_DEBUG("reloc for active import %s: old index = %d, new index = %d\n", import->name.data, function_index, new_index);
			return new_index;
		}
	}
	return function_index + offset;
}

Index LinkerInputBinary::RelocateTypeIndex(Index type_index) {
	return type_index + type_index_offset;
}

Index LinkerInputBinary::RelocateGlobalIndex(Index global_index) {
	Index offset;
	if (global_index >= global_imports.size()) {
		offset = global_index_offset;
	} else {
		offset = imported_global_index_offset;
	}
	return global_index + offset;
}

static void ApplyRelocation(const Section *section, const wabt::Reloc *r) {
	LinkerInputBinary *binary = section->binary;
	uint8_t *section_data = &binary->data[section->offset];
	size_t section_size = section->size;

	Index cur_value = 0, new_value = 0;
	// todo: what if value at reloc location is not LEB ? does this ever happen?
	int leb_bytes = wabt::ReadU32Leb128(section_data + r->offset, section_data + section_size, &cur_value);
	while (leb_bytes-- > 0) *(section_data + r->offset + leb_bytes) = 0x01; // NOPs to delete the old value

	switch (r->type) {
		// todo INSERT if leb > old value for all types!  we do have &binary->data as vector so it's easy!
		case RelocType::FuncIndexLEB: {
			new_value = binary->RelocateFuncIndex(cur_value);
			WriteU32Leb128Raw(section_data + r->offset, section_data + section_size, new_value);
		}
			break;
		case RelocType::TypeIndexLEB:
			new_value = binary->RelocateTypeIndex(cur_value);
			WriteFixedU32Leb128Raw(section_data + r->offset, section_data + section_size, new_value);
			break;
		case RelocType::TableIndexSLEB:
			new_value = cur_value + binary->table_index_offset;
			WriteFixedU32Leb128Raw(section_data + r->offset, section_data + section_size, new_value);
			break;
		case RelocType::GlobalIndexLEB:
			new_value = binary->RelocateGlobalIndex(cur_value);
//			WriteFixedU32Leb128Raw(section_data + r->offset, section_data + section_size, new_value);
			WriteU32Leb128Raw(section_data + r->offset, section_data + section_size, new_value);
			break; // versus :
		case RelocType::GlobalIndexI32:
//			new_value = binary->RelocateGlobalIndex(cur_value);
		case RelocType::TableIndexI32:
		case RelocType::MemoryAddressLEB:
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
			WABT_FATAL("unhandled relocation type: %s\n", GetRelocTypeName(r->type));
			// uh much to do!
	}
// THIS Write only makes sense for LEB types!
}

// https://github.com/hyperledger-labs/solang/commit/7bb623bc864106c70b209aa8ec4dfe15ac262b68
// section is 5 (Memory for all relocations, even func-call! :( )
// match meta_data_version {1 | 2 => (),
static void ApplyRelocations(const Section *section) {
	if (section->relocations.size() <= 0) {
		return;
	}
	LOG_DEBUG("ApplyRelocations: %s\n", GetSectionName(section->section_code));
	// Perform relocations in-place.
	for (const Reloc &reloc: section->relocations) {
		ApplyRelocation(section, &reloc);
	}
}

class Linker {
public:
	WABT_DISALLOW_COPY_AND_ASSIGN(Linker);

	Linker() = default;

	void AppendBinary(LinkerInputBinary *binary) { inputs_.emplace_back(binary); }

	OutputBuffer PerformLink();

	void CreateRelocs();

	std::vector<Reloc> PatchCodeSection(std::vector<byte> section_data, size_t offset, size_t size, size_t indexOffset, short import_boarder);

	void PatchCodeSection(bytes section_data, int length, size_t i);

private:
	typedef std::pair<Offset, Offset> Fixup;

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

	bool WriteCombinedSection(BinarySection section_code,
	                          const SectionPtrVector &sections);

	void ResolveSymbols();

	void CalculateRelocOffsets();

	void WriteBinary();

	void DumpRelocOffsets();

	MemoryStream stream_;
	std::vector<std::unique_ptr<LinkerInputBinary>> inputs_;
	ssize_t current_payload_offset_ = 0;

	Section *getSection(std::unique_ptr<LinkerInputBinary> &uniquePtr, BinarySection section);

	List<Reloc> CalculateRelocs(std::unique_ptr<LinkerInputBinary> &binary, Section *section);
};

void Linker::WriteSectionPayload(Section *sec) {
	assert(current_payload_offset_ != -1);

	sec->output_payload_offset =
			stream_.offset() - current_payload_offset_;

	uint8_t *payload = &sec->binary->data[sec->payload_offset];
	stream_.WriteData(payload, sec->payload_size, "section content");
}

Linker::Fixup Linker::WriteUnknownSize() {
	Offset fixup_offset = stream_.offset();
	WriteFixedU32Leb128(&stream_, 0, "unknown size");
	current_payload_offset_ = stream_.offset();
	return std::make_pair(fixup_offset, current_payload_offset_);
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
	for (const std::unique_ptr<LinkerInputBinary> &binary: inputs_) {
		total_exports += binary->exports.size();
	}

	Fixup fixup = WriteUnknownSize();
	WriteU32Leb128(&stream_, total_exports, "export count");

	for (const std::unique_ptr<LinkerInputBinary> &binary: inputs_) {
		if (not binary->needs_relocate) {
			warn("binary "s + binary->name + " marked as needs_relocate = false");
			continue;
		}
		for (const Export &export_: binary->exports) {
			WriteStr(&stream_, s(export_.name), "export name");
			stream_.WriteU8Enum(export_.kind, "export kind");
			Index index = export_.index;
			switch (export_.kind) {
				case ExternalKind::Func:
					index = binary->RelocateFuncIndex(index);
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

	Index total_elem_count = 0;
	for (Section *section: sections) {
		total_elem_count += section->binary->table_elem_count;
	}

	WriteU32Leb128(&stream_, 1, "segment count");
	WriteU32Leb128(&stream_, 0, "table index");
	WriteOpcode(&stream_, Opcode::I32Const);
	WriteS32Leb128(&stream_, 0U, "elem init literal");
	WriteOpcode(&stream_, Opcode::End);
	WriteU32Leb128(&stream_, total_elem_count, "num elements");

	current_payload_offset_ = stream_.offset();

	for (Section *section: sections) {
		ApplyRelocations(section);
		WriteSectionPayload(section);
	}

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
	WriteU32Leb128(&stream_, import.sig_index + offset, "import signature index");
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
	for (const std::unique_ptr<LinkerInputBinary> &binary: inputs_) {
		for (const FunctionImport &import: binary->function_imports) {
			if (import.active) {
				num_imports++;
			}
		}
		num_imports += binary->global_imports.size();
	}

	Fixup fixup = WriteUnknownSize();
	WriteU32Leb128(&stream_, num_imports, "num imports");

	for (const std::unique_ptr<LinkerInputBinary> &binary: inputs_) {
		for (const FunctionImport &function_import: binary->function_imports) {
			if (function_import.active) {
				WriteFunctionImport(function_import, binary->type_index_offset);
			}
		}

		for (const GlobalImport &global_import: binary->global_imports) {
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
		const uint8_t *end =
				&sec->binary->data[sec->payload_offset + sec->payload_size];
		while (count--) {
			input_offset += ReadU32Leb128(start + input_offset, end, &sig_index);
			WriteU32Leb128(&stream_, sec->binary->RelocateTypeIndex(sig_index),
			               "sig");
		}
	}

	FixupSize(fixup);
}

void Linker::WriteDataSegment(const DataSegment &segment, Address offset) {
	assert(segment.memory_index == 0);
	WriteU32Leb128(&stream_, segment.memory_index, "memory index");
	WriteOpcode(&stream_, Opcode::I32Const);
	WriteU32Leb128(&stream_, segment.offset + offset, "offset");
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
	for (const std::unique_ptr<LinkerInputBinary> &binary: inputs_) {
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

	stream_.WriteU8Enum(BinarySection::Custom, "section code");
	Fixup fixup = WriteUnknownSize();
	WriteStr(&stream_, "name"s, "custom section name"s);

	stream_.WriteU8Enum(NameSectionSubsection::Function, "subsection code");
	Fixup fixup_subsection = WriteUnknownSize();
	WriteU32Leb128(&stream_, total_count, "element count");

	// Write import names.
	for (const std::unique_ptr<LinkerInputBinary> &binary: inputs_) {
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
	for (const std::unique_ptr<LinkerInputBinary> &binary: inputs_) {
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
	stream_.WriteU8Enum(BinarySection::Custom, "section code");
	Fixup fixup = WriteUnknownSize();

	WriteStr(&stream_, "linking"s, "linking section name"s);

	if (data_size) {
//    WriteU32Leb128(&stream_, LinkingEntryType::DataSize, "subsection code");
//		TODO("WriteLinkingSection");
		Fixup fixup_subsection = WriteUnknownSize();
		WriteU32Leb128(&stream_, data_size, "data size");
		FixupSize(fixup_subsection);
	}

	FixupSize(fixup);
}


bool Linker::WriteCombinedSection(BinarySection section_code, const SectionPtrVector &sections) {
	if (!sections.size()) {
		return false;
	}

	if (section_code == BinarySection::Start && sections.size() > 1) {
		WABT_FATAL("Don't know how to combine sections of type: %s\n",
		           GetSectionName(section_code));
	}

	Index total_count = 0;
	Index total_size = 0;

	// Sum section size and element count.
	for (Section *sec: sections) {
		total_size += sec->payload_size;
		total_count += sec->count;
	}

	stream_.WriteU8Enum(section_code, "section code");

	current_payload_offset_ = -1;

	switch (section_code) {
		case BinarySection::Import:
			WriteImportSection();
			break;
		case BinarySection::Function:
			WriteFunctionSection(sections, total_count);
			break;
		case BinarySection::Table:
			WriteTableSection(sections);
			break;
		case BinarySection::Export:
			WriteExportSection();
			break;
		case BinarySection::Elem:
			WriteElemSection(sections);
			break;
		case BinarySection::Memory:
			WriteMemorySection(sections);
			break;
		case BinarySection::Data:
			WriteDataSection(sections, total_count);
			break;
		default: { // just append
			// Total section size includes the element count leb128.
			total_size += U32Leb128Length(total_count);

			// Write section to stream.
			WriteU32Leb128(&stream_, total_size, "section size");
			WriteU32Leb128(&stream_, total_count, "element count");
			current_payload_offset_ = stream_.offset();
			for (Section *sec: sections) {
				ApplyRelocations(sec);
				WriteSectionPayload(sec);
			}
		}
	}
	return true;
}


struct FuncInfo {
	FuncInfo(const Func *export_, LinkerInputBinary *binary) : func(export_), binary(binary) {}

	const Func *func;
	LinkerInputBinary *binary;
};

void Linker::ResolveSymbols() {
	// Create hashmap of all exported symbols from all inputs.

	// ⚠️ all indices in func_map go into the function CODE section (LATER offset by the import count!)
	BindingHash func_map;
	Map<String, int> name_map;// from debug section to
	// ⚠️ all indices here map into into following LIST, not into wasm!! so TWO indirections!
	BindingHash export_map;
	std::vector<ExportInfo> export_list;
	std::vector<FuncInfo> func_list;// internal index identical to func_map index!!
	std::vector<FuncInfo> import_list;//

// binary->functions not filled yet!
	for (const std::unique_ptr<LinkerInputBinary> &binary: inputs_) {
		printf("!!!!!!!!!!!   %s #%lu !!!!!!!!!!!\n", binary->name, binary->debug_names.size());

		int pos = 0;
		for (const Export &export_: binary->exports) {// todo: why not store index directly?
			pos++;
			printf("%s export %s index %d\n", binary->name, export_.name.data, export_.index);
			export_list.emplace_back(&export_, binary.get());
			if (export_map.FindIndex(export_.name) != kInvalidIndex) {
				warn("Ignoring duplicate export name "s + export_.name);
				binary->exports.erase(binary->exports.begin() + pos);
			} else
				export_map.emplace(export_.name, Binding(export_list.size() - 1));
			// TODO Handle duplicate names.
		}
		for (const Func &func: binary->functions) {// only those with code, not imports
			if (not empty(func.name)) {
				printf("func.name %s, func.index %d\n", func.name.data, func.index);
//				check(func_list.size()==func.index);
				func_map.emplace(func.name, func.index);
			}
			func_list.emplace_back(&func, binary.get());
		}
//		for (const Func &func: binary->function_imports
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

	// Iterate through all imported functions resolving them against exported ones.
	for (std::unique_ptr<LinkerInputBinary> &binary: inputs_) {
		if (not binary->needs_relocate)continue;
		for (FunctionImport &import: binary->function_imports) {
			String &name = import.name;
			Index export_number = export_map.FindIndex(name);
			if (export_number != kInvalidIndex) {
				// We found the symbol exported by another module.
				ExportInfo &export_info = export_list[export_number];
				// TODO verify the foreign function has the correct signature.
				Index export_index = export_info.export_->index;
				Func &exported = export_info.binary->functions[export_index];
//				Index new_Index; // see RelocateFuncIndex(); ⚠️ WE CAN ONLY calculate the new index once ALL imports are collected!
				Index old_index = import.sig_index;
				import.active = false;
				import.foreign_binary = export_info.binary;
				import.foreign_index = export_index;
				import.linked_function = &export_info;
//				import.relocated_function_index = export_number; see RelocateFuncIndex()
				binary->active_function_imports--;
				char *import_name = import.name;
				char *export_name = exported.name;
				printf("LINKED %s import #%d %s to export #%d %s relocated_function_index %d \n", name.data, old_index, import_name, export_index, export_name,
				       export_number);
			} else {
				// todo all this is done in RelocateFuncIndex !
				// link unexported functions, because clang -Wl,--relocatable,--export-all DOES NOT preserve EXPORT wth
				Index func_index = func_map.FindIndex(name);
				if (func_index == kInvalidIndex) {
					warn("unresolved import: "s + name);
//					import.active = false;// don'true
//					binary->active_function_imports--;
//					todo("")
					warn("keep unresolved import in case it's used inside binary: "s + name);
//					import.foreign_index =  binary.import_delta + import.sig_index;
//					import.relocated_function_index =  binary.import_delta + import.sig_index;
					// no need for complicated calculations, just count the active imports so far
					continue;
					warn("delete unresolved import: "s + name);
					continue;
					error("can't find undefined symbol: "s + name);
				}
//				check(func_list[func_index].func);
				FuncInfo &funcInfo = func_list[func_index];
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

	for (std::unique_ptr<LinkerInputBinary> &binary: inputs_) {
		binary->delta = my_function_count; // offset all functions if not mapped to import

		// The imported_function_index_offset is the sum of all the function
		// imports from objects that precede this one.  i.e. the current running
		// total.
		binary->imported_function_index_offset = total_function_imports;
		binary->imported_global_index_offset = total_global_imports;
		binary->memory_page_offset = memory_page_offset;

		size_t delta = 0;// number of functions removed for this binary ( imports linked )
		for (size_t i = 0; i < binary->function_imports.size(); i++) {
			if (!binary->function_imports[i].active) {
				delta++;
			} else {
				binary->function_imports[i].relocated_function_index = total_function_imports + i - delta;
			}
		}

		memory_page_offset += binary->memory_page_count;
		total_function_imports += binary->active_function_imports;
		total_global_imports += binary->global_imports.size();
		my_function_count += binary->active_function_imports;
		my_function_count += binary->function_count;
	}

	for (std::unique_ptr<LinkerInputBinary> &binary: inputs_) {
		binary->table_index_offset = table_elem_count;
		table_elem_count += binary->table_elem_count;
		for (std::unique_ptr<Section> &sec: binary->sections) {
			switch (sec->section_code) {
				case BinarySection::Type:
					binary->type_index_offset = type_count;
					type_count += sec->count;
					break;
				case BinarySection::Global:
					binary->global_index_offset = total_global_imports - sec->binary->global_imports.size() + global_count;
					global_count += sec->count;
					break;
				case BinarySection::Function: {
					Index new_offset = total_function_imports - sec->binary->function_imports.size() + function_count;
					binary->function_index_offset = new_offset;
				}
					function_count += sec->count;
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

	for (std::unique_ptr<LinkerInputBinary> &binary: inputs_) {
		for (std::unique_ptr<Section> &sec: binary->sections) {
			Section *section = sec.get();
			int sectionCode = (int) sec->section_code;
			SectionPtrVector &sec_list = sections[sectionCode];
			sec_list.push_back(section);
		}
	}

	// Write the final binary.
	stream_.WriteU32(WABT_BINARY_MAGIC, "WABT_BINARY_MAGIC");
	stream_.WriteU32(WABT_BINARY_VERSION, "WABT_BINARY_VERSION");
	// Write known sections first.
	for (size_t i = (int) FIRST_KNOWN_SECTION; i < kBinarySectionCount; i++) {
		WriteCombinedSection((BinarySection) i, sections[i]);
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
	for (const std::unique_ptr<LinkerInputBinary> &binary: inputs_) {
		LOG_DEBUG("Relocation info for: %s\n", binary->name);
		LOG_DEBUG(" - type index offset       : %d\n", binary->type_index_offset);
		LOG_DEBUG(" - mem page offset         : %d\n", binary->memory_page_offset);
		LOG_DEBUG(" - function index offset   : %d\n", binary->function_index_offset);
		LOG_DEBUG(" - global index offset     : %d\n", binary->global_index_offset);
		LOG_DEBUG(" - imported function offset: %d\n", binary->imported_function_index_offset);
		LOG_DEBUG(" - imported global offset  : %d\n", binary->imported_global_index_offset);
	}
}

void Linker::CreateRelocs() {
	for (std::unique_ptr<LinkerInputBinary> &binary: inputs_) {
		if (not binary->needs_relocate) {
			trace("Skipping CreateRelocs for library "s + binary->name);
			continue;
		}
		Section *section = getSection(binary, BinarySection::Code);
		if (!section)return;
		List<Reloc> relocs = CalculateRelocs(binary, section);
		for (Reloc &reloc: relocs)
			section->relocations.push_back(reloc);
//		if(!section->data.data_segments)
//			continue;
//		for (const DataSegment &segment: *section->data.data_segments) {
//		}
	}
}

OutputBuffer Linker::PerformLink() {
	CalculateRelocOffsets();
	ResolveSymbols(); // LINK import to exports …  binary->active_function_imports--
	CalculateRelocOffsets();// again: might be negative if import is removed!
	CreateRelocs();
	DumpRelocOffsets();
	WriteBinary();
	return stream_.output_buffer();
}

Section *Linker::getSection(std::unique_ptr<LinkerInputBinary> &binary, BinarySection section) {
	for (std::unique_ptr<Section> &sec: binary->sections) {
		if (sec->section_code == section)return sec.get();
	}
	return nullptr;
}

// relocs can either be provided as custom section, or inferred from the linker. todo: don't infer if provided
List<Reloc> Linker::CalculateRelocs(std::unique_ptr<LinkerInputBinary> &binary, Section *section) {
	List<Reloc> relocs;
	std::vector<uint8_t> &binary_data = binary->data;// LATER plus section_offset todo shared Code view
	size_t section_offset = section->offset + 1;// into binary data
//	DataSegment section_data = ;// *pSection->data.data_segments->data();
	Index binary_delta = binary->delta;
	size_t section_size = section->size;
//	Index import_border = binary->imported_function_index_offset;// todo for current binary or for ALL?
	unsigned long old_import_border = binary->function_imports.size();
	int length = binary_data.size();
	int current = section_offset;
	bool begin_function = true;
	int current_fun = 0;
	String current_name = "?";
	int fun_end = length;
	while (current < length and current - section_offset < section_size) {// go over ALL functions! ignore 00
		if (begin_function) {
			begin_function = false;
			current_name = binary->functions[current_fun].name;
			int fun_length = unsignedLEB128(binary_data, length, current, true);// length of ONE function
			fun_end = current + fun_length;
			int locals = unsignedLEB128(binary_data, length, current, true);
			if (current_name.data)// else what is this #2 test/merge/main2.wasm :: (null)
				printf("#%d %s :: %s (#%d)\n", current_fun, binary->name, current_name.data, locals);
			current += locals * 2;// nr+type
		}
		byte b = binary_data[current++];
		if (current >= fun_end) {
			begin_function = true;
			current_fun++;
//			trace("begin_function %d\n", current_fun);
			continue;
		}
		Opcodes op = (Opcodes) b;
		Opcode opcode = Opcode::FromCode(b);
		if (op == call_) {
			short index = unsignedLEB128(binary_data, length, current, false);
			Index neu = binary->RelocateFuncIndex(index);
			if (index != neu) {
				Reloc reloc(wabt::RelocType::FuncIndexLEB, current - section_offset + 1, neu);
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
			printf("CALL %s %s calls %s $%d -> %d\n", binary->name, current_name.data, function_name.data, index, neu);
#endif
		} else if (op == global_get || op == global_set) {
			short index = unsignedLEB128(binary_data, length, current, false);
			Index neu = index + binary->global_index_offset;
//			Reloc reloc(wabt::RelocType::GlobalIndexI32, current - section_offset + 1, neu);
//			relocs.add(reloc);
			Reloc reloc2(wabt::RelocType::GlobalIndexLEB, current - section_offset + 1, neu);
			relocs.add(reloc2);
//			todo("reloc global_get");
//		} else if (op == global_set) {
//			todo("reloc global_get");
		} else if (op >= i32_load and op <= i32_store_16) {
			todo("reloc LOAD/STORE");
		} else {
			int arg_bytes = opcode_args[op];
			if (arg_bytes > 0)
				current += arg_bytes;
			else if (arg_bytes == leb) {
				unsignedLEB128(binary_data, length, current, true);// start passed as reference will be MODIFIED!!
			} // auto variable argument(s)
			else if (arg_bytes == -1) {
				printf("UNKNOWN OPCODE ARGS 0x%x %d “%s” length: %d?\n", op, op, opcode.GetName(), arg_bytes);
				error("UNKNOWN OPCODE");
			}
//			if (tracing)
//				printf("OPCODE 0x%x %d “%s” length: %d?\n", op, op, opcode.GetName(), arg_bytes);
		}
	}
	return relocs;
}


void Linker::PatchCodeSection(bytes section_data, int length, size_t offset) {
	//	int codeCount = unsignedLEB128(codes_vector);
	//	Code code = vec(codes);
	int start = offset;
	int fun_length = unsignedLEB128(section_data, length, start);
	Opcodes op = (Opcodes) section_data[start++];
	if (op == call_) {
		short nop_offset = start;
		short index = unsignedLEB128(section_data, length, start);
//		if (new_indices.contains(index)) {
////			Reloc reloc(wabt::RelocType::FuncIndexLEB, nop_offset,new_indices[index] );
//			while (section_data[nop_offset] == 0x80)
//				*(section_data + nop_offset++) = 0x01;// NOP the rest!
//			*(section_data + nop_offset) = 0x01;// ONE MORE NOP to delete the old value
//			// else CHECK SIZE!
//			WriteU32Leb128Raw(section_data + nop_offset, section_data + length, new_indices[index]);
//		}
	} else {
		int arg_bytes = opcode_args[op];
		start += arg_bytes;
	}
}


Code &code(std::vector<uint8_t> vector1) {
	return *new Code(vector1.data(), vector1.size());
}

Code merge_files(List<String> infiles) {
	Linker linker;
	Result ok = Result::Ok;
	for (const String &input_filename: infiles) {
		std::vector<uint8_t> file_data;
		ok = ReadFile(input_filename.data, &file_data);
		if (!ok)continue;
		auto binary = new LinkerInputBinary(input_filename.data, file_data);
		linker.AppendBinary(binary);
		LinkOptions options = {NULL};
		ReadBinaryLinker(binary, &options);
	}
	return code(linker.PerformLink().data);
}


Code &merge_binaries(List<Code> binaries) {
//	opcode_args[global_get] = leb;
//	check(opcode_args[global_get] == leb) // todo what kind of dark bug is that???
	Linker linker;
	if (binaries.size() == 1)
		return binaries.items[0].clone();
	for (const Code &code: binaries) {
		std::vector<uint8_t> file_data(code.data, code.data + code.length);
		const char *source = "<code>";
		if (not code.name.empty()) source = code.name.data;
		LinkerInputBinary *binary = new LinkerInputBinary(source, file_data);
		binary->needs_relocate = true;// code.needs_relocate;
		LinkOptions options = {NULL, code.needs_relocate};
		ReadBinaryLinker(binary, &options);
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

Code merge_binaries(Code main, Code lib) {
	List<Code> binaries;
	binaries.add(main);
	binaries.add(lib);
	return merge_binaries(binaries);
}

void merge_files(int argc, char **argv) {
	List<String> infiles;
	while (argc-- > 0)
		infiles.add(argv[argc]);
	merge_files(infiles);
}

