#pragma once

#include "Code.h"

class Module {
public:
	virtual ~Module() {
		type_data = Code();
//		free(type_data);
//		free(type_data.data);
	}

public:
	String name;

	// unparsed (just the vector data and its length)
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
	Code data_segments;// data_section without header ^^ (actual data) todo BUT memory_id + i32.const + offset + 0x0b is HEADER PER SEGMENT!
	Code linking_section;
	Code relocate_section;
	List<Code> custom_sections;// all remaining

	int type_count = 0;
	int import_count = 0;
	int total_func_count = 0;// code_count + import_count
	int table_count = 0; // no such thing?
	int memory_count = 0;
	int export_count = 0;
	int global_count = 0;
	int code_count = 0;
	int data_segments_count = 0; // ≠ data_section.length   ALL MUST BE IN the one Data section!
	// name section:
	Code element_section;// todo?
	Code local_names;
	Code function_names;
	int start_index;
	List<String> import_names;
	Map<String, Signature> signatures;
//	int data_offset=0;// todo: read from data section! why not 0 ?
	int data_offset_end;
};

Module read_wasm(bytes buffer, int size);

Module read_wasm(chars file);

