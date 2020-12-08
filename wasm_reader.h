#pragma once
#include "Code.h"
class Module{
public:
	Code type_data;
	Code import_data;
	Code export_data;
	Code code_data;
	Code functype_data;
	Code name_data;
	int type_count;
	int func_count;
	int import_count;
	int export_count;
};
Module read_wasm(char const *file);