#pragma once
#include "Code.h"
class Module{
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

	int type_count;
	int code_count;
	int import_count;
	int func_count;// code_count + import_count
	int export_count;
	// name section:
	Code local_names;
	Code function_names;
};
Module read_wasm(chars file);

