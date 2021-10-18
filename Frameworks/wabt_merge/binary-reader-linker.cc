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

#define RELOC_SIZE 5

namespace wabt {
    namespace link {

        namespace {

            void TODO(char *what = 0) {
              printf("TODO");
              if (what)printf("%s", what);
            }

            class BinaryReaderLinker : public BinaryReaderNop { // < BinaryReaderDelegate {
            public:
                explicit BinaryReaderLinker(LinkerInputBinary *binary);

                Result BeginSection(Index section_index, BinarySection section_type, Offset size) override;

                Result OnImportFunc(Index import_index,
                                    string_view module_name,
                                    string_view field_name,
                                    Index func_index,
                                    Index sig_index) override;

                Result OnImportGlobal(Index import_index,
                                      string_view module_name,
                                      string_view field_name,
                                      Index global_index,
                                      Type type,
                                      bool mutable_) override;

                Result OnImportMemory(Index import_index,
                                      string_view module_name,
                                      string_view field_name,
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
                                string_view name) override;

                Result OnElemSegmentFunctionIndexCount(Index index,
                                                       Index count);// override;

                Result BeginDataSegment(Index index, Index memory_index);// override;
                Result OnDataSegmentData(Index index,
                                         const void *data,
                                         Address size) override;

                Result BeginNamesSection(Offset size) override;

                Result OnFunctionName(Index function_index,
                                      string_view function_name) override;

                Result OnRelocCount(Index count, Index section_code) override;
//  Result OnRelocCount(Index count, BinarySection section_code, string_view section_name) override;

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
//                                        string_view section_name) {
              BinarySection section_code0 = (BinarySection) (int) section_code;
              if (section_code == (int) BinarySection::Custom) {
                WABT_FATAL("relocation for custom sections not yet supported\n");
              }

              for (const std::unique_ptr<Section> &section : binary_->sections) {
                if ((int) section->section_code != section_code) {
                  continue;
                }
                reloc_section_ = section.get();
                return Result::Ok;
              }

//  WABT_FATAL("section not found: %d\n", static_cast<int>(section_code));
              return Result::Error;
            }

            Result BinaryReaderLinker::OnReloc(RelocType type, Offset offset, Index index, uint32_t addend) {
              //  if (offset + RELOC_SIZE > reloc_section_->size) {
//    WABT_FATAL("invalid relocation offset: %#" PRIoffset "\n", offset);
//  }
              reloc_section_->relocations.emplace_back(type, offset, index, addend);
              return Result::Ok;
            }

            Result BinaryReaderLinker::OnImportFunc(Index import_index,
                                                    string_view module_name,
                                                    string_view field_name,
                                                    Index global_index,
                                                    Index sig_index) {
              binary_->function_imports.emplace_back();
              FunctionImport *import = &binary_->function_imports.back();
              import->module_name = module_name.to_string();
              import->name = field_name.to_string();
              import->sig_index = sig_index;
              import->active = true;
              binary_->active_function_imports++;
              return Result::Ok;
            }

            Result BinaryReaderLinker::OnImportGlobal(Index import_index,
                                                      string_view module_name,
                                                      string_view field_name,
                                                      Index global_index,
                                                      Type type,
                                                      bool mutable_) {
              binary_->global_imports.emplace_back();
              GlobalImport *import = &binary_->global_imports.back();
              import->module_name = module_name.to_string();
              import->name = field_name.to_string();
              import->type = type;
              import->mutable_ = mutable_;
              binary_->active_global_imports++;
              return Result::Ok;
            }

            Result BinaryReaderLinker::OnImportMemory(Index import_index,
                                                      string_view module_name,
                                                      string_view field_name,
                                                      Index memory_index,
                                                      const Limits *page_limits) {
              WABT_FATAL("Linker does not support imported memories");
              return Result::Error;
            }

            Result BinaryReaderLinker::OnFunctionCount(Index count) {
              binary_->function_count = count;
              return Result::Ok;
            }

// unnecessary: use debug_names
            Result BinaryReaderLinker::OnFunction(Index index, Index sig_index) {
//  binary_.
//  FuncType& func_type = module_.func_types[sig_index];
              Func fun;// = {index, sig_index};
              fun.index = index;// not starting with 0 if there are imports
              fun.name = "";// unresolved
              binary_->functions.push_back(fun);
              return Result::Ok;
//  module_.funcs.push_back(FuncDesc{func_type, {}, 0});
//  func_types_.push_back(func_type);
            }

            Result BinaryReaderLinker::BeginSection(Index section_index, BinarySection section_code,
                                                    Offset size) {
              Section *sec = new Section();
              binary_->sections.emplace_back(sec);
              current_section_ = sec;
              sec->section_code = section_code;
              sec->size = size;
              sec->offset = state->offset;
              sec->binary = binary_;

              if (sec->section_code != BinarySection::Custom &&
                  sec->section_code != BinarySection::Start) {
                const uint8_t *start = &binary_->data[sec->offset];
                // Must point to one-past-the-end, but we can't dereference end().
                const uint8_t *end = &binary_->data.back() + 1;
                size_t bytes_read = ReadU32Leb128(start, end, &sec->count);
                if (bytes_read == 0) {
                  WABT_FATAL("error reading section element count\n");
                }
                sec->payload_offset = sec->offset + bytes_read;
                sec->payload_size = sec->size - bytes_read;
              }
              return Result::Ok;
            }

            Result BinaryReaderLinker::OnTable(Index index,
                                               Type elem_type,
                                               const Limits *elem_limits) {
              if (elem_limits->has_max && (elem_limits->max != elem_limits->initial)) {
                WABT_FATAL("Tables with max != initial not supported by wabt-link\n");
              }

              binary_->table_elem_count = elem_limits->initial;
              return Result::Ok;
            }

            Result BinaryReaderLinker::OnElemSegmentFunctionIndexCount(Index index,
                                                                       Index count) {
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

            Result BinaryReaderLinker::BeginDataSegment(Index index, Index memory_index) {
              Section *sec = current_section_;
              if (!sec->data.data_segments) {
                sec->data.data_segments = new std::vector<DataSegment>();
              }
              sec->data.data_segments->emplace_back();
              DataSegment &segment = sec->data.data_segments->back();
              segment.memory_index = memory_index;
              return Result::Ok;
            }

            Result BinaryReaderLinker::OnInitExprI32ConstExpr(Index index, uint32_t value) {
              Section *sec = current_section_;
              if (sec->section_code != BinarySection::Data) {
                return Result::Ok;
              }
              DataSegment &segment = sec->data.data_segments->back();
              segment.offset = value;
              return Result::Ok;
            }

            Result BinaryReaderLinker::OnDataSegmentData(Index index,
                                                         const void *src_data,
                                                         Address size) {
              Section *sec = current_section_;
              DataSegment &segment = sec->data.data_segments->back();
              segment.data = static_cast<const uint8_t *>(src_data);
              segment.size = size;
              return Result::Ok;
            }

            Result BinaryReaderLinker::OnExport(Index index,
                                                ExternalKind kind,
                                                Index item_index,
                                                string_view name) {
              if (kind == ExternalKind::Memory) {
//    WABT_FATAL("Linker does not support exported memories");
                return Result::Ok;
              }
              binary_->exports.emplace_back();
              Export *export_ = &binary_->exports.back();
              export_->name = name.to_string();
              export_->kind = kind;
              export_->index = item_index;
              return Result::Ok;
            }

            Result BinaryReaderLinker::BeginNamesSection(Offset size) {
              binary_->debug_names.resize(binary_->function_count + binary_->function_imports.size());
              return Result::Ok;
            }

            Result BinaryReaderLinker::OnFunctionName(Index index, string_view name) {
              for (Func &f:binary_->functions) {
                if (f.index == index)
                  f.name = name.to_string();
              }
              binary_->debug_names[index] = name.to_string();
              return Result::Ok;
            }

        }  // end anonymous namespace

        Result ReadBinaryLinker(LinkerInputBinary *input_info, LinkOptions *options) {
          BinaryReaderLinker reader(input_info);

          ReadBinaryOptions read_options;
          read_options.read_debug_names = true;
          read_options.fail_on_custom_section_error = false;
          read_options.stop_on_first_error = false;
          read_options.log_stream = options->log_stream;
          return ReadBinary(input_info->data.data(), input_info->data.size(),
                            &reader, (const ReadBinaryOptions) read_options);
        }

    }  // namespace link
}  // namespace wabt
