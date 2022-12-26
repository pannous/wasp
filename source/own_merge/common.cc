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

#include "common.h"
#include "result.h"

#include <cassert>
#include <climits>
#include <cstdint>
#include <cstdio>
#include <cstring>

#include <sys/types.h>
#include <sys/stat.h>
#include <cerrno>
#include <cstdarg>


#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wformat-nonliteral"

String StringPrintf(const char *format, ...) {
    va_list args;
    va_list args_copy;
    va_start(args, format);
    va_copy(args_copy, args);
    size_t len = vsnprintf(nullptr, 0, format, args) + 1;  // For \0.
    List<char> buffer(len);
    va_end(args);

    vsnprintf(buffer.items, len, format, args_copy);
    va_end(args_copy);
    return String(buffer.items, (int) len - 1);
}

#pragma clang diagnostic pop

namespace wabt {

	Reloc::Reloc(RelocType type, Offset offset, Index index, int32_t addend)
			: type(type), offset(offset), index(index), addend(addend) {}

	const char *g_kind_name[] = {"func", "table", "memory", "global", "tag"};
//	WABT_STATIC_ASSERT(WABT_ARRAY_SIZE(g_kind_name) == kExternalKindCount);

	const char *g_reloc_type_name[] = {
			"R_WASM_FUNCTION_INDEX_LEB", "R_WASM_TABLE_INDEX_SLEB",
			"R_WASM_TABLE_INDEX_I32", "R_WASM_MEMORY_ADDR_LEB",
			"R_WASM_MEMORY_ADDR_SLEB", "R_WASM_MEMORY_ADDR_I32",
			"R_WASM_TYPE_INDEX_LEB", "R_WASM_GLOBAL_INDEX_LEB",
			"R_WASM_FUNCTION_OFFSET_I32", "R_WASM_SECTION_OFFSET_I32",
			"R_WASM_TAG_INDEX_LEB", "R_WASM_MEMORY_ADDR_REL_SLEB",
			"R_WASM_TABLE_INDEX_REL_SLEB", "R_WASM_GLOBAL_INDEX_I32",
			"R_WASM_MEMORY_ADDR_LEB64", "R_WASM_MEMORY_ADDR_SLEB64",
			"R_WASM_MEMORY_ADDR_I64", "R_WASM_MEMORY_ADDR_REL_SLEB64",
			"R_WASM_TABLE_INDEX_SLEB64", "R_WASM_TABLE_INDEX_I64",
			"R_WASM_TABLE_NUMBER_LEB", "R_WASM_MEMORY_ADDR_TLS_SLEB",
			"R_WASM_MEMORY_ADDR_TLS_I32",
	};
//	WABT_STATIC_ASSERT(WABT_ARRAY_SIZE(g_reloc_type_name) == kRelocTypeCount);

    static Result ReadStdin(List<uint8_t> *out_data) {
        out_data->resize(0);
        uint8_t buffer[4096];
        while (true) {
            size_t bytes_read = fread(buffer, 1, sizeof(buffer), stdin);
            if (bytes_read == 0) {
                if (ferror(stdin)) {
                    fprintf(stderr, "error reading from stdin: %s\n", strerror(errno));
                    return Result::Error;
                }
                return Result::Ok;
			}
			size_t old_size = out_data->size();
            out_data->resize(old_size + bytes_read);
            memcpy(out_data->items + old_size, buffer, bytes_read);
		}
	}

    Result ReadFile(string_view filename, List<uint8_t> *out_data) {
        String filename_str = filename.data;
        const char *filename_cstr = filename_str.data;

        if (filename == "-") {
            return ReadStdin(out_data);
        }

        struct stat statbuf;
        if (stat(filename_cstr, &statbuf) < 0) {
            fprintf(stderr, "%s: %s\n", filename_cstr, strerror(errno));
			return Result::Error;
		}

		if (!(statbuf.st_mode & S_IFREG)) {
			fprintf(stderr, "%s: not a regular file\n", filename_cstr);
			return Result::Error;
		}

		FILE *infile = fopen(filename_cstr, "rb");
		if (!infile) {
			fprintf(stderr, "%s: %s\n", filename_cstr, strerror(errno));
			return Result::Error;
		}

		if (fseek(infile, 0, SEEK_END) < 0) {
			perror("fseek to end failed");
			fclose(infile);
			return Result::Error;
		}

		long size = ftell(infile);
		if (size < 0) {
			perror("ftell failed");
			fclose(infile);
			return Result::Error;
		}

		if (fseek(infile, 0, SEEK_SET) < 0) {
			perror("fseek to beginning failed");
			fclose(infile);
			return Result::Error;
		}

		out_data->resize(size);
        if (size != 0 && fread(out_data->items, size, 1, infile) != 1) {
            fprintf(stderr, "%s: fread failed: %s\n", filename_cstr, strerror(errno));
            fclose(infile);
            return Result::Error;
        }

		fclose(infile);
		return Result::Ok;
	}


}  // namespace wabt
