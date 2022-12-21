/*
 * Copyright 2016 WebAssembly Community Group participants
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "ir.h"

#include <cassert>
#include <cstddef>
#include <numeric>

#include "cast.h"

namespace {

	const char *ExprTypeName[] = {
			"AtomicFence",
			"AtomicLoad",
			"AtomicRmw",
			"AtomicRmwCmpxchg",
			"AtomicStore",
			"AtomicNotify",
			"AtomicWait",
			"Binary",
			"Block",
			"Br",
			"BrIf",
			"BrTable",
			"Call",
			"CallIndirect",
			"CallRef",
			"Compare",
			"Const",
			"Convert",
			"Drop",
			"GlobalGet",
			"GlobalSet",
			"If",
			"Load",
			"LocalGet",
			"LocalSet",
			"LocalTee",
			"Loop",
			"MemoryCopy",
			"DataDrop",
			"MemoryFill",
			"MemoryGrow",
			"MemoryInit",
			"MemorySize",
			"Nop",
			"RefIsNull",
			"RefFunc",
			"RefNull",
			"Rethrow",
			"Return",
			"ReturnCall",
			"ReturnCallIndirect",
			"Select",
			"SimdLaneOp",
			"SimdLoadLane",
			"SimdStoreLane",
			"SimdShuffleOp",
			"LoadSplat",
			"LoadZero",
			"Store",
			"TableCopy",
			"ElemDrop",
			"TableInit",
			"TableGet",
			"TableGrow",
			"TableSize",
			"TableSet",
			"TableFill",
			"Ternary",
			"Throw",
			"Try",
			"Unary",
			"Unreachable",
	};

}  // end of anonymous namespace

namespace wabt {

	const char *GetExprTypeName(ExprType type) {
		static_assert(WABT_ENUM_COUNT(ExprType) == WABT_ARRAY_SIZE(ExprTypeName),
		              "Malformed ExprTypeName array");
		return ExprTypeName[size_t(type)];
	}

	const char *GetExprTypeName(const Expr &expr) {
		return GetExprTypeName(expr.type());
	}

	bool FuncSignature::operator==(const FuncSignature &rhs) const {
		return param_types == rhs.param_types && result_types == rhs.result_types;
	}

	const Export *Module::GetExport(string_view name) const {
		Index index = export_bindings.FindIndex(name);
		if (index >= exports.size()) {
			return nullptr;
		}
		return exports[index];
	}

	Index Module::GetFuncIndex(const Var &var) const {
		return func_bindings.FindIndex(var);
	}

	Index Module::GetGlobalIndex(const Var &var) const {
		return global_bindings.FindIndex(var);
	}

	Index Module::GetTableIndex(const Var &var) const {
		return table_bindings.FindIndex(var);
	}

	Index Module::GetMemoryIndex(const Var &var) const {
		return memory_bindings.FindIndex(var);
	}

	Index Module::GetFuncTypeIndex(const Var &var) const {
		return type_bindings.FindIndex(var);
	}

	Index Module::GetTagIndex(const Var &var) const {
		return tag_bindings.FindIndex(var);
	}

	Index Module::GetDataSegmentIndex(const Var &var) const {
		return data_segment_bindings.FindIndex(var);
	}

	Index Module::GetElemSegmentIndex(const Var &var) const {
		return elem_segment_bindings.FindIndex(var);
	}

	bool Module::IsImport(ExternalKind kind, const Var &var) const {
		switch (kind) {
			case ExternalKind::Func:
				return GetFuncIndex(var) < num_func_imports;

			case ExternalKind::Global:
				return GetGlobalIndex(var) < num_global_imports;

			case ExternalKind::Memory:
				return GetMemoryIndex(var) < num_memory_imports;

			case ExternalKind::Table:
                return GetTableIndex(var) < num_table_imports;

            case ExternalKind::Tag:
                return GetTagIndex(var) < num_tag_imports;

            default:
                return false;
        }
    }

//	void LocalTypes::Set(const TypeVector &types) {
//		decls_.clear();
//		if (types.empty()) {
//			return;
//		}
//
//		Type type = types[0];
//		Index count = 1;
//		for (Index i = 1; i < types.size(); ++i) {
//			if (types[i] != type) {
//				decls_.add(type, count);
//                type = types[i];
//				count = 1;
//			} else {
//				++count;
//			}
//		}
//        decls_.add(type, count);
//	}



    Type Func::GetLocalType(Index index) { //const
        Index num_params = decl.GetNumParams();
        if (index < num_params) {
            return GetParamType(index);
        } else {
            index -= num_params;
            assert(index < local_types.size());
            return local_types[index];
//            return *local_types.has(index);
        }
    }

    Type Func::GetLocalType(const Var &var) { // const
        return GetLocalType(GetLocalIndex(var));
    }

	Index Func::GetLocalIndex(const Var &var) const {
		if (var.is_index()) {
			return var.index();
		}
		return bindings.FindIndex(var);
	}

	const Func *Module::GetFunc(const Var &var) const {
		return const_cast<Module *>(this)->GetFunc(var);
	}

	Func *Module::GetFunc(const Var &var) {
		Index index = func_bindings.FindIndex(var);
		if (index >= funcs.size()) {
			return nullptr;
		}
		return funcs[index];
	}

	const Global *Module::GetGlobal(const Var &var) const {
		return const_cast<Module *>(this)->GetGlobal(var);
	}

	Global *Module::GetGlobal(const Var &var) {
		Index index = global_bindings.FindIndex(var);
		if (index >= globals.size()) {
			return nullptr;
		}
		return globals[index];
	}

	const Table *Module::GetTable(const Var &var) const {
		return const_cast<Module *>(this)->GetTable(var);
	}

	Table *Module::GetTable(const Var &var) {
		Index index = table_bindings.FindIndex(var);
		if (index >= tables.size()) {
			return nullptr;
		}
		return tables[index];
	}

	const Memory *Module::GetMemory(const Var &var) const {
		return const_cast<Module *>(this)->GetMemory(var);
	}

	Memory *Module::GetMemory(const Var &var) {
		Index index = memory_bindings.FindIndex(var);
		if (index >= memories.size()) {
			return nullptr;
		}
		return memories[index];
	}

	Tag *Module::GetTag(const Var &var) const {
		Index index = GetTagIndex(var);
		if (index >= tags.size()) {
			return nullptr;
		}
		return tags[index];
	}

	const DataSegment *Module::GetDataSegment(const Var &var) const {
		return const_cast<Module *>(this)->GetDataSegment(var);
	}

	DataSegment *Module::GetDataSegment(const Var &var) {
		Index index = data_segment_bindings.FindIndex(var);
		if (index >= data_segments.size()) {
			return nullptr;
		}
		return data_segments[index];
	}

	const ElemSegment *Module::GetElemSegment(const Var &var) const {
		return const_cast<Module *>(this)->GetElemSegment(var);
	}

	ElemSegment *Module::GetElemSegment(const Var &var) {
		Index index = elem_segment_bindings.FindIndex(var);
		if (index >= elem_segments.size()) {
			return nullptr;
		}
		return elem_segments[index];
	}

	const FuncType *Module::GetFuncType(const Var &var) const {
		return const_cast<Module *>(this)->GetFuncType(var);
	}

	FuncType *Module::GetFuncType(const Var &var) {
		Index index = type_bindings.FindIndex(var);
		if (index >= types.size()) {
			return nullptr;
		}
		return dyn_cast<FuncType>(types[index]);
	}

	Index Module::GetFuncTypeIndex(const FuncSignature &sig) const {
		for (size_t i = 0; i < types.size(); ++i) {
			if (auto *func_type = dyn_cast<FuncType>(types[i])) {
				if (func_type->sig == sig) {
					return i;
				}
			}
		}
		return kInvalidIndex;
	}

	Index Module::GetFuncTypeIndex(const FuncDeclaration &decl) const {
		if (decl.has_func_type) {
			return GetFuncTypeIndex(decl.type_var);
		} else {
			return GetFuncTypeIndex(decl.sig);
		}
    }

    void Module::AppendField(DataSegmentModuleField *field) {
        DataSegment &data_segment = field->data_segment;
        if (!data_segment.name.empty()) {
            data_segment_bindings.emplace(data_segment.name, Binding(field->loc, data_segments.size()));
        }
        data_segments.add(&data_segment);
//        todo("fields.add((field));")
    }


    void Module::AppendField(DataSegmentModuleField &field) {
        DataSegment &data_segment = field.data_segment;
        if (!data_segment.name.empty()) {
            data_segment_bindings.emplace(data_segment.name, Binding(field.loc, data_segments.size()));
        }
        data_segments.add(&data_segment);
    }


    void Module::AppendField(ElemSegmentModuleField *field) {
        ElemSegment &elem_segment = field->elem_segment;
        if (!elem_segment.name.empty()) {
            elem_segment_bindings.emplace(elem_segment.name,
                                          Binding(field->loc, elem_segments.size()));
        }
        elem_segments.add(&elem_segment);
//        todo("fields.add((field));")
//      fields.add((field));
    }

    void Module::AppendField(TagModuleField *field) {
        Tag &tag = field->tag;
        if (!tag.name.empty()) {
            tag_bindings.emplace(tag.name, Binding(field->loc, tags.size()));
        }
        tags.add(&tag);
//        todo("fields.add((field));")
//      fields.add((field));
    }

    void Module::AppendField(ExportModuleField *field) {
        // Exported names are allowed to be empty.
        Export &export_ = field->export_;
        export_bindings.emplace(export_.name, Binding(field->loc, exports.size()));
        exports.add(&export_);
//        todo("fields.add((field));")
//      fields.add((field));
    }

    void Module::AppendField(FuncModuleField *field) {
        Func &func = field->func;
        if (!func.name.empty()) {
            func_bindings.emplace(func.name, Binding(field->loc, funcs.size()));
        }
        funcs.add(&func);
//        todo("fields.add((field));")
//      fields.add((field));
    }

    void Module::AppendField(TypeModuleField *field) {
        TypeEntry &type = *field->type;
        if (!type.name.empty()) {
            type_bindings.emplace(type.name, Binding(field->loc, types.size()));
        }
        types.add(&type);
//        todo("fields.add((field));")
//      fields.add((field));
    }

    void Module::AppendField(GlobalModuleField *field) {
        Global &global = field->global;
        if (!global.name.empty()) {
            global_bindings.emplace(global.name, Binding(field->loc, globals.size()));
        }
        globals.add(&global);
//        todo("fields.add((field));")
//      fields.add((field));
    }

    void Module::AppendField(ImportModuleField *field) {
        Import *import = field->import;
        const String *name = nullptr;
        BindingHash *bindings = nullptr;
        Index index = kInvalidIndex;

        switch (import->kind()) {
            case ExternalKind::Func: {
                Func func = cast<FuncImport>(import)->func;
                name = &func.name;
                bindings = &func_bindings;
                index = funcs.size();
                funcs.add(&func);
                ++num_func_imports;
				break;
			}

			case ExternalKind::Table: {
                Table table = cast<TableImport>(import)->table;
				name = &table.name;
				bindings = &table_bindings;
                index = tables.size();
                tables.add(&table);
                ++num_table_imports;
				break;
			}

			case ExternalKind::Memory: {
                Memory memory = cast<MemoryImport>(import)->memory;
				name = &memory.name;
				bindings = &memory_bindings;
                index = memories.size();
                memories.add(&memory);
                ++num_memory_imports;
				break;
			}

			case ExternalKind::Global: {
                Global global = cast<GlobalImport>(import)->global;
				name = &global.name;
				bindings = &global_bindings;
                index = globals.size();
                globals.add(&global);
                ++num_global_imports;
				break;
			}

			case ExternalKind::Tag: {
                Tag tag = cast<TagImport>(import)->tag;
				name = &tag.name;
				bindings = &tag_bindings;
                index = tags.size();
                tags.add(&tag);
                ++num_tag_imports;
                break;
            }
        }

        assert(name && bindings && index != kInvalidIndex);
        if (!name->empty()) {
            bindings->emplace(*name, Binding(field->loc, index));
        }
        imports.add(import);
//        todo("fields.add((field));")
//      fields.add((field));
    }

    void Module::AppendField(MemoryModuleField *field) {
        Memory &memory = field->memory;
        if (!memory.name.empty()) {
            memory_bindings.emplace(memory.name, Binding(field->loc, memories.size()));
        }
        memories.add(&memory);
//        todo("fields.add((field));")
//      fields.add((field));
    }

    void Module::AppendField(StartModuleField *field) {
        starts.add(&field->start);
//        todo("fields.add((field));")
//      fields.add((field));
    }

    void Module::AppendField(TableModuleField *field) {
        Table &table = field->table;
        if (!table.name.empty()) {
            table_bindings.emplace(table.name, Binding(field->loc, tables.size()));
        }
        tables.add(&table);
//        todo("fields.add((field));")
//      fields.add((field));
    }

    void Module::AppendField(ModuleField *field) {
        switch (field->type()) {
            case ModuleFieldType::Func: todo()
//"AppeField(cast<FuncModuleField>((field)));"//                AppeField(cast<FuncModuleField>((field)));
                break;

            case ModuleFieldType::Global: todo()
//"AppeField(cast<GlobalModuleField>((field)));"//                AppeField(cast<GlobalModuleField>((field)));
                break;

            case ModuleFieldType::Import:
//                todo("Appendeld(cast<ImportModuleField>((field)));")
//				Appendeld(cast<ImportModuleField>((field)));
                break;

            case ModuleFieldType::Export:
//                todo("Appendeld(cast<ExportModuleField>((field)));")
//				Appendeld(cast<ExportModuleField>((field)));
                break;

            case ModuleFieldType::Type:
//                todo("Appendeld(cast<TypeModuleField>((field)));")
//				Appendeld(cast<TypeModuleField>((field)));
                break;

            case ModuleFieldType::Table:
//                todo("Appendeld(cast<TableModuleField>((field)));")
//				Appendeld(cast<TableModuleField>((field)));
                break;

            case ModuleFieldType::ElemSegment:
//                todo("Appendeld(cast<ElemSegmentModuleField>((field)));")
//				Appendeld(cast<ElemSegmentModuleField>((field)));
                break;

            case ModuleFieldType::Memory:
//                todo("Appendeld(cast<MemoryModuleField>((field)));")
//				Appendeld(cast<MemoryModuleField>((field)));
                break;

            case ModuleFieldType::DataSegment:
//                todo("Appendeld(cast<DataSegmentModuleField>((field)));")
//				Appendeld(cast<DataSegmentModuleField>((field)));
                break;

            case ModuleFieldType::Start:
//                todo("Appendeld(cast<StartModuleField>((field)));")
//				Appendeld(cast<StartModuleField>((field)));
                break;

            case ModuleFieldType::Tag:
//                todo("Appendeld(cast<TagModuleField>((field)));")
//				Appendeld(cast<TagModuleField>((field)));
                break;
		}
	}

	void Module::AppendFields(ModuleFieldList *fields) {
		while (!fields->empty())
            AppendField(&fields->first());
	}

	const Module *Script::GetFirstModule() const {
		return const_cast<Script *>(this)->GetFirstModule();
	}

	Module *Script::GetFirstModule() {
        for (auto command: commands) {
            if (auto *module_command = dyn_cast<ModuleCommand>(command)) {
                return &module_command->module;
            }
        }
		return nullptr;
	}

	const Module *Script::GetModule(const Var &var) const {
		Index index = module_bindings.FindIndex(var);
		if (index >= commands.size()) {
			return nullptr;
		}
        auto *command = cast<ModuleCommand>(commands[index]);
		return &command->module;
	}

    void MakeTypeBindingReverseMapping(
            size_t num_types,
            const BindingHash &bindings,
            List<String> *out_reverse_mapping) {
        out_reverse_mapping->clear();
        out_reverse_mapping->resize(num_types);
        for (const auto &pair: bindings) {
            assert(static_cast<size_t>(pair.second.index) <
                   out_reverse_mapping->size());
            (*out_reverse_mapping)[pair.second.index] = pair.first;
        }
    }

	Var::Var(Index index, const Location &loc)
			: loc(loc), type_(VarType::Index), index_(index) {}

	Var::Var(string_view name, const Location &loc)
			: loc(loc), type_(VarType::Name), name_(name) {}

	Var::Var(Var &&rhs) : Var(kInvalidIndex) {
        todo()
////"rhs);	this = (rhs);
    }

	Var::Var(const Var &rhs) : Var(kInvalidIndex) {
		*this = rhs;
	}

	Var &Var::operator=(Var &&rhs) {
		loc = rhs.loc;
		if (rhs.is_index()) {
			set_index(rhs.index_);
		} else {
			set_name(rhs.name_);
		}
		return *this;
	}

	Var &Var::operator=(const Var &rhs) {
		loc = rhs.loc;
		if (rhs.is_index()) {
			set_index(rhs.index_);
		} else {
			set_name(rhs.name_);
		}
		return *this;
	}

	Var::~Var() {
		Destroy();
	}

	void Var::set_index(Index index) {
		Destroy();
		type_ = VarType::Index;
		index_ = index;
	}

	void Var::set_name(String &&name) {
        Destroy();// wat !?
        type_ = VarType::Name;
        this->name_ = name;
//        todo("Construct(name_, (name));")
//		Construct(name_, (name));
    }

//
	void Var::set_name(string_view name) {
		Destroy();// wat !?
		type_ = VarType::Name;
		this->name_ = name;
	}

	void Var::Destroy() {
		if (is_name()) {
			Destruct(name_);
		}
	}

	uint8_t ElemSegment::GetFlags(const Module *module) const {
		uint8_t flags = 0;

		bool all_ref_func = elem_type == Type::FuncRef;

		switch (kind) {
			case SegmentKind::Active: {
				Index table_index = module->GetTableIndex(table_var);
				if (table_index != 0) {
					flags |= SegExplicitIndex;
				}
				break;
			}

			case SegmentKind::Passive:
				flags |= SegPassive;
				break;

			case SegmentKind::Declared:
				flags |= SegDeclared;
				break;
		}

		all_ref_func = all_ref_func &&
		               std::all_of(elem_exprs.begin(), elem_exprs.end(),
		                           [](const ElemExpr &elem_expr) {
			                           return elem_expr.kind == ElemExprKind::RefFunc;
		                           });
		if (!all_ref_func) {
			flags |= SegUseElemExprs;
		}

		return flags;
	}

	uint8_t DataSegment::GetFlags(const Module *module) const {
		uint8_t flags = 0;

		if (kind == SegmentKind::Passive) {
			flags |= SegPassive;
		}

		Index memory_index = module->GetMemoryIndex(memory_var);
		if (memory_index != 0) {
			flags |= SegExplicitIndex;
		}

		return flags;
	}


}  // namespace wabt
