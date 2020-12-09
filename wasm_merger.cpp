//
// Created by me on 09.12.20.
//

#include "wasm_merger.h"

int total_functions = -1;

Code mergeTypeSection(Module lib, Module main) {
	return Code(type, encodeVector(Code(lib.type_count + main.type_count) + lib.type_data + main.type_data));
}

Code mergeImportSection(Module lib, Module main) {
	return createSection(import, Code(lib.import_count + main.import_count) + lib.import_data + main.import_data);
}

Code mergeFuncTypeSection(Module lib, Module main) { // signatures
	return createSection(functypes, Code(lib.code_count + main.code_count) + lib.functype_data + main.functype_data);
}

Code mergeExportSection(Module lib, Module main) {
	return createSection(exports, Code(lib.export_count + main.export_count) + lib.export_data + main.export_data);
}

Code relocate(Code &blocks) {
	return blocks;// todo, maybe
}

Code mergeCodeSection(Module lib, Module main) {
	int codes = lib.code_count + main.code_count;
	return createSection(code_section, Code(codes) + lib.code_data + main.code_data); // relocate(
}

Code mergeCustomSections(Module lib, Module main) {
	Code list=*new Code();
	for (Code& sec: lib.custom_sections) {
		Code more = encodeVector(sec);
		list.add(more);
	}
	for (Code sec: main.custom_sections) {
		Code more = encodeVector(sec);
		list.add(more);
	}
	return list;
}


Code mergeNameSection(Module lib, Module main) {
	return createSection(custom, lib.name_data + main.name_data);// blindly blend and append moduleName + functionNames …
	auto moduleName = Code(module_name) + encodeVector(Code("wasp_module"));
	auto functionNames = Code(function_names) + encodeVector(Code(total_functions) + lib.function_names + main.function_names);
	auto localNames = Code(local_names) + encodeVector(Code(total_functions) + lib.local_names + main.local_names);
	const Code &nameSectionData = encodeVector(Code("name") + moduleName + functionNames + localNames);
	auto nameSection = createSection(custom, nameSectionData); // auto encodeVector AGAIN!
	return nameSection.clone();;
}

Code mergeDataSection(Module lib, Module main) {
	return encodeVector(lib.data_section + main.data_section);
}

Code mergeLinkingSection(Module lib, Module main) {
	return encodeVector(lib.linking_section + main.linking_section);
}

Code merge_wasm(Module lib, Module main) {
	total_functions = lib.func_count + main.func_count;
	Code code = Code(magicModuleHeader, 4) + Code(moduleVersion, 4)
	            + mergeTypeSection(lib, main)
	            + mergeImportSection(lib, main) // needed for correct functypes
	            + mergeFuncTypeSection(lib, main)
	            + mergeExportSection(lib, main)
	            + mergeCodeSection(lib, main)
	            //	            + mergeDataSection(lib, main)
	            //	            + mergeLinkingSection(lib, main)
	            //	            + mergeNameSection(lib, main)
//	            + mergeCustomSections(lib, main)
	            ;
	return code.clone();
}
