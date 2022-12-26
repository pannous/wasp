/*
 * Copyright 2017 WebAssembly Community Group participants
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

#include "binary-reader-linker.h"

#include <vector>

#include "binary-reader-nop.h"
#include "wasm-link.h"
#include "leb128.h"

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-parameter"
#define RELOC_SIZE 5

namespace wabt {
    namespace link {

        namespace {

            class BinaryReaderLinker : public BinaryReaderNop { // < BinaryReaderDelegate {
            public:
                explicit BinaryReaderLinker(LinkerInputBinary *binary);

				Result BeginSection(Index section_index, SectionType section_type, Offset size) override;

				Result OnImportFunc(Index import_index,
				                    String module_name,
				                    String field_name,
				                    Index func_index,
				                    Index sig_index) override;

				Result OnImportGlobal(Index import_index,
				                      String module_name,
				                      String field_name,
				                      Index global_index,
				                      Type type,
				                      bool mutable_) override;

				Result OnImportMemory(Index import_index,
				                      String module_name,
				                      String field_name,
				                      Index memory_index,
				                      const Limits *page_limits) override;

				Result OnFunctionCount(Index count) override;

				Result OnFunction(Index index, Index sig_index) override;

				Result OnTable(Index index,
				               Type elem_type,
				               const Limits *elem_limits) override;

				Result OnMemory(Index index, const Limits *limits) override;

				Result OnExport(Index index,
				                ExternalKind kind,
				                Index item_index,
				                String name) override;

				Result OnElemSegmentFunctionIndexCount(Index index, Index count) override;

				Result BeginDataSegment(Index index, Index memory_index, uint8_t flags) override;

				Result OnDataSegmentData(Index index,
				                         const void *data,
				                         Address64 size) override;

				Result BeginNamesSection(Offset size) override;

				Result OnFunctionName(Index function_index,
				                      String function_name) override;

				Result OnRelocCount(Index count, Index section_code) override;
//  Result OnRelocCount(Index count, BinarySection section_code, String section_name) override;

				Result OnReloc(RelocType type, Offset offset, Index index, uint32_t addend) override;

				Result OnInitExprI32ConstExpr(Index index, uint32_t value) override;

				Section *reloc_section_ = nullptr;

			private:
				LinkerInputBinary *binary_;
				Section *current_section_ = nullptr;
			};

			BinaryReaderLinker::BinaryReaderLinker(LinkerInputBinary *binary)
					: binary_(binary) {}

			Result BinaryReaderLinker::OnRelocCount(Index count, Index section_code) {
//                                        BinarySection section_code,
//                                        String section_name) {
				SectionType section_code0 = (SectionType) (int) section_code;
				if (section_code0 == SectionType::Custom) {
                    WABT_FATAL("relocation for custom sections not yet supported\n");
                }

				for (auto section: binary_->sections) {
                    if ((int) section->section_code != (int) section_code) {
                        continue;
                    }
                    reloc_section_ = section;
                    return Result::Ok;
                }

				WABT_FATAL("section not found: %d\n", static_cast<int>(section_code));
				return Result::Error;
			}

			Result BinaryReaderLinker::OnReloc(RelocType type, Offset offset, Index index, uint32_t addend) {
                if (offset + RELOC_SIZE > reloc_section_->size) {
                    WABT_FATAL("invalid relocation offset: %zu\n", offset);
                }
                reloc_section_->relocations.add(Reloc(type, offset, index, addend));
                return Result::Ok;
            }

			Result BinaryReaderLinker::OnImportFunc(Index import_index,
			                                        String module_name,
			                                        String field_name,
			                                        Index global_index,
			                                        Index sig_index) {
                binary_->function_imports.add();
				FunctionImport *import = &binary_->function_imports.back();
				import->module_name = module_name.data;
                import->name = field_name.data;
                import->type_index = sig_index;
                import->active = true;
				binary_->active_function_imports++;
				return Result::Ok;
			}

			Result BinaryReaderLinker::OnImportGlobal(Index import_index,
			                                          String module_name,
			                                          String field_name,
			                                          Index global_index,
			                                          Type type,
			                                          bool mutable_) {
                binary_->global_imports.add();
				GlobalImport *import = &binary_->global_imports.back();
				import->module_name = module_name.data;
				import->name = field_name.data;
				import->type = type;
				import->mutable_ = mutable_;
				binary_->active_global_imports++;
				return Result::Ok;
			}

			Result BinaryReaderLinker::OnImportMemory(Index import_index,
			                                          String module_name,
			                                          String field_name,
			                                          Index memory_index,
			                                          const Limits *page_limits) {
                    WABT_FATAL("Linker does not support imported memories");
				return Result::Error;
            }

			Result BinaryReaderLinker::OnFunctionCount(Index count) {
				binary_->function_count = count;
				return Result::Ok;
			}

			Result BinaryReaderLinker::OnFunction(Index index, Index sig_index) {
                Func fun;// = {index, sig_index};
                fun.index = index;// not starting with 0 if there are imports
                fun.name = "";// unresolved
                // todo 2022-12 why was signature (type) index ignored here?
                fun.type_index = sig_index;
                binary_->functions.add(fun);
                return Result::Ok;
//              FuncType& func_type = binary_.func_types[sig_index];
//              func_types_.add(func_type);
            }

            Result BinaryReaderLinker::BeginSection(Index section_index, SectionType section_code, Offset size) {
                Section *sec = new Section();
                binary_->sections.add(sec);
                current_section_ = sec;
                sec->section_code = section_code;
                sec->size = size;
                sec->offset = state->offset;
                sec->binary = binary_;

                if (sec->section_code != SectionType::Custom &&
                    sec->section_code != SectionType::Start) {
                    const uint8_t *start = &binary_->data[sec->offset];
                    const uint8_t *end = start + binary_->data.size_;
                    // Must point to one-past-the-end, but we can't dereference end().
//                    const uint8_t *end = start + binary_->size;
                    size_t leb_bytes = ReadU32Leb128(start, end, &sec->count);
                    if (leb_bytes == 0) {
                        WABT_FATAL("error reading section element count\n");
                    }
                    sec->payload_offset = sec->offset + leb_bytes;
                    sec->payload_size = sec->size - leb_bytes;
                }
				return Result::Ok;
			}

            Result BinaryReaderLinker::OnTable(Index index, Type elem_type, const Limits *elem_limits) {
                if (elem_limits->has_max && (elem_limits->max != elem_limits->initial)) {
                    WABT_FATAL("Tables with max != initial not supported by wabt-link\n");
                }
                binary_->table_elem_count = elem_limits->initial;
                return Result::Ok;
            }

			Result BinaryReaderLinker::OnElemSegmentFunctionIndexCount(Index index, Index count) {
				Section *sec = current_section_;
				/* Modify the payload to include only the actual function indexes */
				size_t delta = state->offset - sec->payload_offset;
				sec->payload_offset += delta;
				sec->payload_size -= delta;
				return Result::Ok;
			}

			Result BinaryReaderLinker::OnMemory(Index index, const Limits *page_limits) {
				Section *sec = current_section_;
				sec->data.initial = page_limits->initial;
				binary_->memory_page_count = page_limits->initial;
				return Result::Ok;
			}

			Result BinaryReaderLinker::BeginDataSegment(Index index, Index memory_index, uint8_t options) {
				Section *sec = current_section_;
				if (!sec->data.data_segments) {
                    sec->data.data_segments = new List<DataSegment>();
				}
                DataSegment &segment = sec->data.data_segments->add();
				segment.memory_index = memory_index;
				return Result::Ok;
			}

			Result BinaryReaderLinker::OnInitExprI32ConstExpr(Index index, uint32_t value) {
                Section *sec = current_section_;
                // kf  or not sec->data.data_segments or sec->data.data_segments->size() == 0 ±
                if (sec->section_code != SectionType::Data) {
                    return Result::Ok;
                }
                DataSegment &segment = sec->data.data_segments->back(); // created in BeginDataSegment?
                segment.offset = value;
                return Result::Ok;
            }

            Result BinaryReaderLinker::OnDataSegmentData(Index index, const void *src_data, Address64 size) {
                Section *sec = current_section_;
                if (not sec->data.data_segments)error("sec->data.data_segments not initialized");
                DataSegment &segment = sec->data.data_segments->back(); // created in BeginDataSegment
                segment.data = static_cast<const uint8_t *>(src_data);
                segment.size = size;
                return Result::Ok;
            }

            Result BinaryReaderLinker::OnExport(Index index, ExternalKind kind, Index item_index, String name) {
                binary_->exports.add();
                Export *export_ = &binary_->exports.back();
                export_->name = name.data;
                export_->kind = kind;
                export_->index = item_index;
                return Result::Ok;
            }

			Result BinaryReaderLinker::BeginNamesSection(Offset size) {
				binary_->debug_names.resize(binary_->function_count + binary_->function_imports.size());
				return Result::Ok;
			}

			Result BinaryReaderLinker::OnFunctionName(Index index, String name) {
				for (Func &f: binary_->functions) {
					if (f.index == index)
						f.name = name.data;
				}
				binary_->debug_names[index] = name.data;
				return Result::Ok;
			}

		}  // end anonymous namespace

        Result ReadBinaryLinker(LinkerInputBinary *input_info) {
            BinaryReaderLinker reader(input_info);
            ReadBinaryOptions read_options;
            read_options.read_debug_names = true;
            read_options.fail_on_custom_section_error = false;
            read_options.stop_on_first_error = false;
            return ReadBinary(input_info->data.items, input_info->data.size_, &reader,
                              (const ReadBinaryOptions) read_options);

//            return ReadBinary(input_info->data.items, input_info->data.size(), &reader, (const ReadBinaryOptions) read_options);
        }

    }  // namespace link
}  // namespace wabt

#pragma clang diagnostic pop
