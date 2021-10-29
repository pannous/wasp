//
// Created by me on 09.12.20.
//

#include "wasm_merger.h"

// https://webassembly.github.io/spec/core/binary/modules.html#sections



int total_functions = -1;

Code mergeTypeSection(Module lib, Module main) {
	return createSection(type_section,
	                     encodeVector(Code(lib.type_count + main.type_count) + lib.type_data + main.type_data));
}

Code mergeImportSection(Module lib, Module main) {
	return createSection(import_section,
	                     Code(lib.import_count + main.import_count) + lib.import_data + main.import_data);
}

Code mergeFuncTypeSection(Module lib, Module main) { // signatures
	return createSection(functypes_section, Code(lib.code_count + main.code_count) + lib.functype_data + main.functype_data);
}

Code mergeTableSection(Module lib, Module main) {
	if (lib.table_count == 0 and main.table_count == 0)return Code();
//	Code(lib.table_count + main.table_count) +
	return createSection(table_section, lib.table_data + main.table_data);
}

Code mergeMemorySection(Module lib, Module main) {
//	if(import_section.has(memory)) return Code();
//	𝚕𝚒𝚖𝚒𝚝𝚜::=|𝟶𝚡𝟶𝟶  𝑛:𝚞𝟹𝟸 𝟶𝚡𝟶𝟷  𝑛:𝚞𝟹𝟸  𝑚:𝚞𝟹𝟸⇒⇒{𝗆𝗂𝗇 𝑛,𝗆𝖺𝗑 𝜖}{𝗆𝗂𝗇 𝑛,𝗆𝖺𝗑 𝑚}
	byte min_only = 0x00;
//	byte min_and_max=0x01;
	unsigned int pages = (unsigned int) (WASM_MEMORY_SIZE / 65536);
	if (pages > 65536 / 4)pages = 65536 / 4;// /2 to be safe with unsigned
	const Code &memory_data = Code(1) + Code(min_only) + Code((int) pages);
	return createSection(memory_section, memory_data);// ignore lib/main

	if (lib.memory_data.length > 0) // Code(1) +
		return createSection(memory_section, lib.memory_data);// + main.memory_data)); ONLY 1 memory section right now!
	if (main.memory_data.length > 0)
		return createSection(memory_section, Code(1) + main.memory_data);// + main.memory_data)); ONLY 1 memory section right now!
	else
		return Code();
}

Code mergeGlobalSection(Module lib, Module main) {
	if (lib.global_count == 0 and main.global_count == 0)return Code();
	Code mergedGlobals = Code(lib.global_count + main.global_count) + lib.globals_data++ + main.globals_data++;
	return createSection(global_section, mergedGlobals);
}

Code mergeExportSection(Module lib, Module main) {
	return createSection(export_section, Code(lib.export_count + main.export_count) + lib.export_data + main.export_data);
}

Code mergeDataSection(Module lib, Module main) {
//	if (lib.data_section.length == 0 and main.data_section.length == 0)return Code();
	if (lib.data_segments_count == 0 and main.data_segments_count == 0)return Code();
//	Code(lib.data_segments_count + main.data_segments_count) + no such thing
//	return createSection(data_section, lib.data_section + main.data_section);
//	return Code(lib.data_section)+ Code(main.data_section);// 000004c: error: multiple Data sections
//	return createSection(data_section, Code(lib.data_segments_count + main.data_segments_count)+ lib.data_section+ main.data_section);// mergeExportSection style
	Code datas;
	int new_length = lib.data_segments_count + main.data_segments_count;
	datas.addByte(new_length);// one memory initialization active todo: increase when merging!
	datas.add(lib.data_segments);// now comes the actual data
	datas.add(main.data_segments);
	return createSection(data_section, encodeVector(datas));
}


Code relocate(Code &blocks) {
	return blocks;// todo, maybe
}

Code mergeCodeSection(Module lib, Module main) {
	int codes = lib.code_count + main.code_count;
	return createSection(code_section, Code(codes) + lib.code_data + main.code_data); // relocate(
}

Code mergeCustomSections(Module lib, Module main) {
	Code list;// = *new Code((byte)custom);
	Code header = Code((byte) custom_section);
	for (Code &sec: lib.custom_sections) {
		Code more = encodeVector(sec);
		list.add(header);
		list.add(more);
	}
	for (Code sec: main.custom_sections) {
		Code more = encodeVector(sec);
		list.add(header);
		list.add(more);
	}
	return list;
}


Code mergeNameSection(Module lib, Module main) {
	// blindly blend and append moduleName + functionNames …
	return createSection(custom_section, Code("name") + lib.name_data);
//	return createSection(custom_section, Code("name") + lib.name_data + main.name_data);

	auto moduleName = Code(module_name) + encodeVector(Code("merged_wasp"));
	auto functionNames = Code(function_names) + encodeVector(Code(lib.total_func_count) + lib.function_names);
	auto localNames = Code(local_names) + encodeVector(Code(lib.total_func_count) + lib.local_names);
//	auto functionNames = Code(function_names) + encodeVector(Code(main.total_func_count) + main.function_names);
//	auto localNames = Code(local_names) + encodeVector(Code(main.total_func_count) +  main.local_names);
//	auto functionNames = Code(function_names) + encodeVector(Code(total_functions) + lib.function_names + main.function_names);
//	auto localNames = Code(local_names) + encodeVector(Code(total_functions) + lib.local_names + main.local_names);
	const Code &nameSectionData = encodeVector(Code("name") + moduleName + functionNames + localNames);
	auto nameSection = createSection(custom_section, nameSectionData); // auto encodeVector AGAIN!
	return nameSection.clone();;
}

Code mergeLinkingSection(Module lib, Module main) {
	return encodeVector(lib.linking_section + main.linking_section);
}

Code merge_wasm(Module lib, Module main) {
	total_functions = lib.total_func_count + main.total_func_count;
	Code code = Code(magicModuleHeader, 4) + Code(moduleVersion, 4)
	            + mergeTypeSection(lib, main)
	            + mergeImportSection(lib, main) // needed for correct functypes_section
	            + mergeFuncTypeSection(lib, main)
	            + mergeTableSection(lib, main)
	            + mergeMemorySection(lib, main)
	            + mergeGlobalSection(lib, main)
	            + mergeExportSection(lib, main)
	            + mergeCodeSection(lib, main)
	            + mergeDataSection(lib, main)
	            //	            + mergeLinkingSection(lib, main)
	            + mergeNameSection(lib, main)
	            + mergeCustomSections(lib, main);
	return code.clone();
}
