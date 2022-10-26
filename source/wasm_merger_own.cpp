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

Code createSection(Sections sectionType, Code data);

Code code(std::vector<uint8_t> vector1);

using namespace wabt;
using namespace wabt::link;
//using wabt::link::LinkerInputBinary;
//using wabt::link::Section;
//using wabt::Reloc;
//using wabt::RelocType;
//using wabt::WriteU32Leb128Raw;

typedef unsigned char byte;
typedef const char *chars;
typedef byte *bytes;


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


// DANGER: modifies the start reader position of code, but not it's data!
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
		: filename(filename),
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
			LOG_DEBUG("reloc for disabled import. new index = %d + %d\n", function_index, offset);
		} else {
			Index new_index = import->relocated_function_index;
			LOG_DEBUG("reloc for active import. old index = %d, new index = %d\n", function_index, new_index);
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
	wabt::ReadU32Leb128(section_data + r->offset, section_data + section_size, &cur_value);

	switch (r->type) {
		case RelocType::FuncIndexLEB: {
			new_value = binary->RelocateFuncIndex(cur_value);
			// if calls are padded
			short nop_offset = r->offset + 1;
			while (section_data[nop_offset] == 0x80) {
				*(section_data + nop_offset++) = 0x01;// NOP the rest!
			}
//			*(section_data + nop_offset) = 0x01;
			int leb_length = nop_offset;
			unsignedLEB128((bytes) section_data, section_size, leb_length);
			while (leb_length-- > 0) *(section_data + nop_offset) = 0x01;// MORE NOPs to delete the old value

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
			WriteFixedU32Leb128Raw(section_data + r->offset, section_data + section_size, new_value);
			break;
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
		case RelocType::GlobalIndexI32:
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

	std::vector<Reloc> PatchCodeSection(std::vector<byte> section_data, int length, size_t offset);

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
	WriteFixedU32Leb128At(&stream_, fixup.first, stream_.offset() - fixup.second,
	                      "fixup size");
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
		for (const Export &export_: binary->exports) {
			WriteStr(&stream_, s(export_.name), "export name");
			stream_.WriteU8Enum(export_.kind, "export kind");
			Index index = export_.index;
			switch (export_.kind) {
				case ExternalKind::Func:
					index = binary->RelocateFuncIndex(index);
					break;
				default:
//					WABT_FATAL("unsupport export type: %d %s\n", static_cast<int>(export_.kind), export_.name.data);
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

void Linker::WriteDataSection(const SectionPtrVector &sections,
                              Index total_count) {
	Fixup fixup = WriteUnknownSize();

	WriteU32Leb128(&stream_, total_count, "data segment count");
	for (const Section *sec: sections) {
		for (const DataSegment &segment: *sec->data.data_segments) {
			WriteDataSegment(segment,
			                 sec->binary->memory_page_offset * WABT_PAGE_SIZE);
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
		default: {
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

struct ExportInfo {
	ExportInfo(const Export *export_, LinkerInputBinary *binary) : export_(export_), binary(binary) {}

	const Export *export_;
	LinkerInputBinary *binary;
};

struct FuncInfo {
	FuncInfo(const Func *export_, LinkerInputBinary *binary) : func(export_), binary(binary) {}

	const Func *func;
	LinkerInputBinary *binary;
};

void Linker::ResolveSymbols() {
	// Create hashmap of all exported symbols from all inputs.
	BindingHash export_map;
	BindingHash func_map;

	std::vector<ExportInfo> export_list;
	std::vector<FuncInfo> func_list;

	for (const std::unique_ptr<LinkerInputBinary> &binary: inputs_) {
		for (const Export &export_: binary->exports) {
			export_list.emplace_back(&export_, binary.get());
			// TODO(sbc): Handle duplicate names.
			export_map.emplace(export_.name, Binding(export_list.size() - 1));
		}
		for (const Func &func: binary->functions) {
			if (not empty(func.name)) {
				func_map.emplace(func.name, func.index);
				func_list.emplace_back(&func, binary.get());
			}
		}
		for (int i = 0; i < binary->debug_names.size(); ++i) {
			String &name = binary->debug_names[i];
			func_map.emplace(name, i);
		}
	}

	// Iterate through all imported functions resolving them against exported
	// ones.
	for (std::unique_ptr<LinkerInputBinary> &binary: inputs_) {
		for (FunctionImport &import: binary->function_imports) {
			String name = import.name;
			Index export_index = export_map.FindIndex(import.name);
			if (export_index != kInvalidIndex) {

				// We found the symbol exported by another module.
				const ExportInfo &export_info = export_list[export_index];

				// TODO(sbc): verify the foreign function has the correct signature.
				import.active = false;
				import.foreign_binary = export_info.binary;
				import.foreign_index = export_info.export_->index;
				binary->active_function_imports--;
				printf("LINKED export to import: %s\n", import.name.data);
			} else {
				// link unexported functions, because clang -Wl,--relocatable,--export-all DOES NOT preserve EXPORT wth
				Index func_index = func_map.FindIndex(import.name);
				FuncInfo &funcInfo = func_list[func_index];
				import.active = false;
				import.foreign_binary = funcInfo.binary;
				import.foreign_index = funcInfo.func->index;
				binary->active_function_imports--;
				printf("LINKED unexported function to import: %s\n", import.name.data);
			}
//			if (export_index == kInvalidIndex) {
//				if (!s_relocatable)
//					WABT_FATAL("undefined symbol: %s\n", import.name.data);
//			}
		}
	}
}

void Linker::CalculateRelocOffsets() {
	Index memory_page_offset = 0;
	Index type_count = 0;
	Index global_count = 0;
	Index function_count = 0;
	Index table_elem_count = 0;
	Index total_function_imports = 0;
	Index total_global_imports = 0;

	for (std::unique_ptr<LinkerInputBinary> &binary: inputs_) {
		// The imported_function_index_offset is the sum of all the function
		// imports from objects that precede this one.  i.e. the current running
		// total.
		binary->imported_function_index_offset = total_function_imports;
		binary->imported_global_index_offset = total_global_imports;
		binary->memory_page_offset = memory_page_offset;

		size_t delta = 0;
		for (size_t i = 0; i < binary->function_imports.size(); i++) {
			if (!binary->function_imports[i].active) {
				delta++;
			} else {
				binary->function_imports[i].relocated_function_index =
						total_function_imports + i - delta;
			}
		}

		memory_page_offset += binary->memory_page_count;
		total_function_imports += binary->active_function_imports;
		total_global_imports += binary->global_imports.size();
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
					binary->global_index_offset = total_global_imports -
					                              sec->binary->global_imports.size() +
					                              global_count;
					global_count += sec->count;
					break;
				case BinarySection::Function:
					binary->function_index_offset = total_function_imports -
					                                sec->binary->function_imports.size() +
					                                function_count;
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
////			WriteRelocSection(static_cast<BinarySection>(i), sections[i]);
//		}
//	}
}

void Linker::DumpRelocOffsets() {
	for (const std::unique_ptr<LinkerInputBinary> &binary: inputs_) {
		LOG_DEBUG("Relocation info for: %s\n", binary->filename);
		LOG_DEBUG(" - type index offset       : %d\n", binary->type_index_offset);
		LOG_DEBUG(" - mem page offset         : %d\n",
		          binary->memory_page_offset);
		LOG_DEBUG(" - function index offset   : %d\n",
		          binary->function_index_offset);
		LOG_DEBUG(" - global index offset     : %d\n",
		          binary->global_index_offset);
		LOG_DEBUG(" - imported function offset: %d\n",
		          binary->imported_function_index_offset);
		LOG_DEBUG(" - imported global offset  : %d\n",
		          binary->imported_global_index_offset);
	}
}

void Linker::CreateRelocs() {
	for (std::unique_ptr<LinkerInputBinary> &binary: inputs_) {
		//	binary->sections;
		Section *section = getSection(binary, BinarySection::Code);
		if (!section)return;
		auto relocs = PatchCodeSection(binary->data, binary->data.size(), section->offset + 1);
		for (Reloc &reloc: relocs)
			section->relocations.push_back(reloc);
//		if(!section->data.data_segments)
//			continue;
//		for (const DataSegment &segment: *section->data.data_segments) {
//			auto data = (bytes) segment.data;
//			//		vector<Reloc> relocs=
//			PatchCodeSection(data, section->payload_size, 0);
//		}
		//	//if(section)
	}
}

OutputBuffer Linker::PerformLink() {
	CalculateRelocOffsets();
	ResolveSymbols();
	CalculateRelocOffsets();// again: might be negative if import is removed!
	CreateRelocs();
	DumpRelocOffsets();
	WriteBinary();
//	call to deleted constructor of 'wabt::MemoryStream'
//	 /Users/me/dev/apps/wasp/source/own_merge/stream.h:183:33: note: 'MemoryStream' has been explicitly marked deleted here
//	WABT_DISALLOW_COPY_AND_ASSIGN(MemoryStream);
// All of this will be completely unnecessary once C++17 comes with mandatory copy-elision comes around, so you can look forward to that.
	return stream_.output_buffer();
}

Section *Linker::getSection(std::unique_ptr<LinkerInputBinary> &binary, BinarySection section) {
	for (std::unique_ptr<Section> &sec: binary->sections) {
		if (sec->section_code == section)return sec.get();
	}
	return nullptr;
}

Code merge_files(std::vector<String> infiles) {
	Linker linker;
	Result ok = Result::Ok;
	for (const String &input_filename: infiles) {
		std::vector<uint8_t> file_data;
//		ok = ReadFile(input_filename.data, &file_data);
		if (!ok)continue;
		auto binary = new LinkerInputBinary(input_filename.data, file_data);
		linker.AppendBinary(binary);
		LinkOptions options = {NULL};
		ReadBinaryLinker(binary, &options);
	}
	return code(linker.PerformLink().data);
}

Code code(std::vector<uint8_t> vector1) {
	return Code(vector1.data(), vector1.size());
}


void merge_files(int argc, char **argv) {
	std::vector<String> infiles;
	while (argc-- > 0)
		infiles.push_back(argv[argc]);
	merge_files(infiles);
}


//#import "wasm_patcher.cpp"

//
// Created by me on 19.10.21.
//
#include <map>
#include "wasm_patcher.h"
//#include "wasm_reader.h"

#include "Map.h"
#include "Code.h"

std::map<short, int> new_indices = {{0, 1},
                                    {1, 2}};
//Map<short, int> new_indices = {{1, 2}};
//Map<short, int> opcode_args = {
// todo -1 everywhere can't be right
std::map<short, int> opcode_args = {
		{nop,                 -1}, // useful for relocation padding call 1 -> call 10000000
		{block,               -1},
		{loop,                -1},
		{if_i,                -1},// precede by i32 result}, follow by i32_type {7f}
		{elsa,                -1},

		// EXTENSIONS:
		{try_,                -1},
		{catch_,              -1},
		{throw_,              -1},
		{rethrow_,            -1},
		{br_on_exn_,          -1}, // branch on exception

		{end_block,           -1}, //11
		{br,                  -1},
		{br_if,               -1},
		{return_block,        -1},
		{function,            -1},

		// EXTENSIONS:
		{call_ref,            -1},
		{return_call_ref,     -1},
		{func_bind,           -1},// {type $t} {$t : u32
		{let_local,           -1}, // {let <bt> <locals> {bt : blocktype}, locals : {as in functions}

		{drop,                -1}, // pop stack
		{select_if,           -1}, // a?b:c ternary todo: use!
		{select_t,            -1}, // extension … ?

		{local_get,           -1},
		{local_set,           -1},
		{local_tee,           -1},
		{get_local,           -1},// get to stack
		{set_local,           -1},// set and pop
		{tee_local,           -1},// set and leave on stack

		{global_get,          -1},
		{global_set,          -1},

		//{ Anyref/externref≠funcref tables}, Table.get and Table.set {for Anyref only}.
		//{Support for making Anyrefs from Funcrefs is out of scope
		{table_get,           -1},
		{table_set,           -1},

		{i8_load,             -1}, //== 𝟶𝚡𝟸𝙳}, i32.load𝟪_u
		{i16_load,            -1}, //== 𝟶𝚡𝟸𝙳}, i32.load𝟪_u
		{i32_load,            -1},// load word from i32 address
		{f32_load,            -1},
		{i32_store,           -1},// store word at i32 address
		{f32_store,           -1},
		// todo : peek 65536 as float directly via opcode
		{i64_load,            -1}, // memory.peek memory.get memory.read
		{i64_store,           -1}, // memory.poke memory.set memory.write
		{i32_store_8,         -1},
		{i32_store_16,        -1},
		{i8_store,            -1},
		{i16_store,           -1},

		//{i32_store_byte, -1},// store byte at i32 address
		{i32_auto,            -1},
		{i32_const,           -1},
		{i64_auto,            -1},
		{i64_const,           -1},
		{f32_auto,            -1},

		{i32_eqz,             -1}, // use for not!
//		{negate,                              -1},
//		{not_truty,                           -1},
		{i32_eq,              -1},
		{i32_ne,              -1},
		{i32_lt,              -1},
		{i32_gt,              -1},
		{i32_le,              -1},
		{i32_ge,              -1},

		{i64_eqz,             -1},
		{f32_eqz,             -1}, // HACK: no such thing!


		{i64_eqz,             -1},
		{i64_eq,              -1},
		{i64_ne,              -1},
		{i64_lt_s,            -1},
		{i64_lt_u,            -1},
		{i64_gt_s,            -1},
		{i64_gt_u,            -1},
		{i64_le_s,            -1},
		{i64_le_u,            -1},
		{i64_ge_s,            -1},
		{i64_ge_u,            -1},

		{f32_eq,              -1},
		{f32_ne,              -1}, // !=
		{f32_lt,              -1},
		{f32_gt,              -1},
		{f32_le,              -1},
		{f32_ge,              -1},

		{f64_eq,              -1},
		{f64_ne,              -1}, // !=
		{f64_lt,              -1},
		{f64_gt,              -1},
		{f64_le,              -1},
		{f64_ge,              -1},

		{i32_add,             -1},
		{i32_sub,             -1},
		{i32_mul,             -1},
		{i32_div,             -1},
		{i32_rem,             -1}, // 5%4=1
		{i32_modulo,          -1},
		{i32_rem_u,           -1},

		{i32_and,             -1},
		{i32_or,              -1},
		{i32_xor,             -1},
		{i32_shl,             -1},
		{i32_shr_s,           -1},
		{i32_shr_u,           -1},
		{i32_rotl,            -1},
		{i32_rotr,            -1},

		//{⚠ warning: funny UTF characters ahead! todo: replace c => c etc?
		{i64_clz,             -1},
		{i64_ctz,             -1},
		{i64_popcnt,          -1},
		{i64_add,             -1},
		{i64_sub,             -1},
		{i64_mul,             -1},
//		{i64_div_s,                       -1},
//		{i64_div_u,                       -1},
		{i64_rem_s,           -1},
		{i64_rem_u,           -1},
		{i64_and,             -1},
		{i64_or,              -1},
		{i64_xor,             -1},
		{i64_s𝗁l,             -1},
		{i64_s𝗁r_s,           -1},
		{i64_s𝗁r_u,           -1},
		{i64_rotl,            -1},
		{i64_rotr,            -1},

		// beginning of float opcodes
		{f32_abs,             -1},
		{f32_neg,             -1},

		// todo : difference : ???
		{f32_ceil,            -1},
		{f32_floor,           -1},
		{f32_trunc,           -1},
		{f32_round,           -1},// truncation ≠ proper rounding!
		{f32_nearest,         -1},

		{f32_sqrt,            -1},
		{f32_add,             -1},
		{f32_sub,             -1},
		{f32_mul,             -1},// f32.mul
		{f32_div,             -1},

		{f64_abs,             -1},
		{f64_neg,             -1},
		{f64_ceil,            -1},
		{f64_floor,           -1},
		{f64_trunc,           -1},
		{f64_nearest,         -1},
		{f64_sqrt,            -1},
		{f64_add,             -1},
		{f64_sub,             -1},
		{f64_mul,             -1},
		{f64_div,             -1},
		{f64_min,             -1},
		{f64_max,             -1},
		{f64_copysign,        -1},

		{f32_cast_to_i32_s,   -1},// truncation ≠ proper rounding {f32_round, -1}!
		{i32_trunc_f32_s,     -1}, // cast/convert != reinterpret
		{f32_convert_i32_s,   -1},// convert FROM i32
//		{i32_cast_to_f32_s,                   -1},
		//{i32_cast_to_f64_s =

		{f32_from_int32,      -1},
		{f64_promote_f32,     -1},
		{f64_from_f32,        f64_promote_f32},
		{i32_reinterpret_f32, -1}, // f32->i32 bit wise reinterpret != cast/trunc/convert
		{f32_reinterpret_i32, -1}, // i32->f32

		{i32_wrap_i64,        -1},
		{i32_trunc_f32_s,     -1},
		{i32_trunc_f32_u,     -1},
		{i32_trunc_f64_s,     -1},
		{i32_trunc_f64_u,     -1},
		{i64_extend_i32_s,    -1},
		{i64_extend_i32_u,    -1},
		{i64_trunc_f32_s,     -1},
		{i64_trunc_f32_u,     -1},
		{i64_trunc_f64_s,     -1},
		{i64_trunc_f64_u,     -1},
		{f32_convert_i32_s,   -1},
		{f32_convert_i32_u,   -1},
		{f32_convert_i64_s,   -1},
		{f32_convert_i64_u,   -1},
		{f32_demote_f64,      -1},
		{f64_convert_i32_s,   -1},
		{f64_convert_i32_u,   -1},
		{f64_convert_i64_s,   -1},
		{f64_convert_i64_u,   -1},
		{f64_promote_f32,     -1},
		{i32_reinterpret_f32, -1},
		{i64_reinterpret_f64, -1},
		{f32_reinterpret_i32, -1},
		{f64_reinterpret_i64, -1},
		{f32_from_f64,        f32_demote_f64},

		//{signExtensions
		{i32_extend8_s,       -1},
		{i32_extend16_s,      -1},
		{i64_extend8_s,       -1},
		{i64_extend16_s,      -1},
		{i64_extend32_s,      -1},
		//{i64_extend_i32_s, -1}, WHAT IS THE DIFFERENCE?
		// i64.extend_s/i32 sign-extends an i32 value to i64}, whereas
		// i64.extend32_s sign-extends an i64 value to i64

		//referenceTypes
		//https://github.com/WebAssembly/function-references/blob/master/proposals/function-references/Overview.md#local-bindings
		{ref_null,            -1},
		{ref_is_null,         -1},
		{ref_func,            -1}, // -1 varuint32 -1 Returns a funcref reference to function $funcidx
		//{ref_null=--1},// {{ref null ht} {$t : heaptype  --1:func --1:extern i >= 0 :{i
		//{ref_typed=--1},// {{ref ht} {$t : heaptype
		{ref_as_non_null,     -1},// {ref.as_non_null
		{br_on_null,          -1}, //{br_on_null $l {$l : u32
		{br_on_non_null,      -1},// {br_on_non_null $l {$l : u32

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


typedef unsigned char *bytes;


std::vector<Reloc> Linker::PatchCodeSection(std::vector<byte> section_data, int length, size_t offset) {
	std::vector<Reloc> relocs;
	//	int codeCount = unsignedLEB128(codes_vector);
	//	Code code = vec(codes);
	int start = offset;
	int fun_length = unsignedLEB128(section_data, length, start);
	byte b = section_data[start++];
	assert(b == 0x00);// // beginning
	while ((b = section_data[start++]) != 0x0b and start < length and start - offset < fun_length) {
		Opcodes op = (Opcodes) b;
		if (op == call_) {
			short nop_offset = start;
			short index = unsignedLEB128(section_data, length, start);
			if (new_indices.contains(index)) {
				Reloc reloc(wabt::RelocType::FuncIndexLEB, nop_offset - offset + 1, new_indices[index]);
				relocs.push_back(reloc);
			}
		} else if (op == i32_auto) {
			int nr = unsignedLEB128(section_data, length, start);
			printf("i32.const %d", nr);
		} else {
			int arg_bytes = opcode_args[op];
			if (arg_bytes > 0)
				start += arg_bytes;
			if (arg_bytes < 0)
				printf("UNKNOWN OPCODE ARGS %d", op);
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
		if (new_indices.contains(index)) {
//			Reloc reloc(wabt::RelocType::FuncIndexLEB, nop_offset,new_indices[index] );
			while (section_data[nop_offset] == 0x80)
				*(section_data + nop_offset++) = 0x01;// NOP the rest!
			*(section_data + nop_offset) = 0x01;// ONE MORE NOP to delete the old value
			// else CHECK SIZE!
			WriteU32Leb128Raw(section_data + nop_offset, section_data + length, new_indices[index]);
		}
	} else {
		int arg_bytes = opcode_args[op];
		start += arg_bytes;
	}

}