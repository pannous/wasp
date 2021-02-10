#pragma once

#include "Code.h"

class Module {
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
	Code data_section;
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
	int data_count = 0;
	// name section:
	Code element_section;// todo?
	Code local_names;
	Code function_names;
	int start_index;
};

Module read_wasm(chars file);

