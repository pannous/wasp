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

#ifndef WABT_LINK_H_
#define WABT_LINK_H_

#include <memory>
#include <vector>

#include "binary.h"
#include "common.h"
#include "type.h"
#include "../List.h"

namespace wabt {
	namespace link {

		class LinkerInputBinary;

		struct Export {
			ExternalKind kind;
			String name;
			Index index;
		};


		struct ExportInfo {
			ExportInfo(const Export *export_, LinkerInputBinary *binary) : export_(export_), binary(binary) {}
			const Export *export_;
			LinkerInputBinary *binary;
		};

		struct FunctionImport {
            String module_name;
            String name;
            Index type_index;
            bool active; /* Is this import present in the linked binary */
            LinkerInputBinary *binary;// original, may be relinked to
            LinkerInputBinary *foreign_binary;
            Index relocated_function_index;
            Index index;// implicit in list, but needed to link duplicate imports
            Index foreign_index;// after link to foreign export
            ExportInfo *linked_function;
        };

		struct GlobalImport {
			String module_name;
			String name;
			wabt::Type type;
			bool mutable_;
			bool active = true;
			LinkerInputBinary *foreign_binary;
			int foreign_index;
            int relocated_global_index;
		};

		struct DataSegment {
			Index memory_index;
			Address64 offset;
			const uint8_t *data;
			size_t size;
		};


		struct Func {
//        Var type_var;
            Index index;// code index
            String name;// set later in name section
            Index type_index;// =>
//        wabt::FuncSignature sig;
        };

		struct Section {
            WABT_DISALLOW_COPY_AND_ASSIGN(Section);

            Section();

            ~Section();

            /* The binary to which this section belongs */
            LinkerInputBinary *binary;
            List<Reloc> relocations; /* The relocations for this section */

            SectionType section_code;
            size_t size;
            size_t offset;

            size_t payload_size;
            size_t payload_offset;
            size_t payload_increase;// after reloc LEB inserts

            /* For known sections, the count of the number of elements in the section */
			Index count;

			union {
                /* DATA section data */
                List<DataSegment> *data_segments;
                /* MEMORY section data */
                uint64_t initial;
			} data;

			/* The offset at which this section appears within the combined output section. */
			size_t output_payload_offset;
		};

        typedef List<Section *> SectionPtrVector;

		class LinkerInputBinary {
		public:
			WABT_DISALLOW_COPY_AND_ASSIGN(LinkerInputBinary);


            LinkerInputBinary(const char *filename, List<uint8_t> &data);

			Index RelocateFuncIndex(Index findex);

            Index RelocateTypeIndex(Index index) const;

            Index RelocateMemoryIndex(Index memory_index) const;

			Index RelocateGlobalIndex(Index index);

            Index RelocateTable(Index findex) const;

            bool IsValidFunctionIndex(Index index) const;

            bool IsFunctionImport(Index index) const;

            bool IsInactiveFunctionImport(Index index);

            const char *name;

            // ⚠️ # ALL IMPORTS of ALL modules plus all functions of all previous modules!
            int delta{};// previous function_count, offset all functions in this module if not mapped to specific import

            List<uint8_t> data;//=(size_t)100000;// todo: safe sharing via offsets, not pointers … if resize()
//            uint8_t *data;
//            size_t size;
            List<Section *> sections;
            List<Export> exports = 10000;// {.capacity=10000};
            List<Func> functions;// only those with code, not imports:
            List<FunctionImport> function_imports;
            Index active_function_imports;
            List<GlobalImport> global_imports;
            Index active_global_imports;

            Index type_index_offset;
            Index function_index_offset; // globally after merging, RIGHT??
            Index imported_function_index_offset;// for current binary or for ALL?
            Index global_index_offset = 0;
            Index imported_global_index_offset{};
            Index table_index_offset;
            Index memory_page_count;
            Index memory_page_offset;
            Index memory_data_start;

            Index table_elem_count = 0;
            Index function_count = 0;

            List<String> debug_names;
            bool needs_relocate{};// keep runtime untouched!
        };

	}  // namespace link
}  // namespace wabt

#endif /* WABT_LINK_H_ */
