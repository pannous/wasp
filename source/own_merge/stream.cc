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

#include "stream.h"

#include <cassert>
#include <cctype>
#include <cerrno>

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wformat-nonliteral"
#define DUMP_OCTETS_PER_LINE 16
#define DUMP_OCTETS_PER_GROUP 2


#define ERROR0(msg) printf("%s:%d: " msg, __FILE__, __LINE__)
#define ERROR(fmt, ...) printf("%s:%d: " fmt, __FILE__, __LINE__, __VA_ARGS__)

//#define ERROR0(msg) fprintf(stderr, "%s:%d: " msg, __FILE__, __LINE__)
//#define ERROR(fmt, ...) fprintf(stderr, "%s:%d: " fmt, __FILE__, __LINE__, __VA_ARGS__)
namespace wabt {

    Stream::Stream(Stream *log_stream) : offset_(0), result_(Result::Ok), log_stream_(log_stream) {}

    void Stream::AddOffset(ssize_t delta) {
        offset_ += delta;
    }

    void Stream::WriteDataAt(size_t at, const void *src, size_t size, const char *desc, PrintChars print_chars) {
        if (Failed(result_)) { // why check last result_ here at beginning?
            return;
        }
        if (log_stream_) {
            log_stream_->WriteMemoryDump(src, size, at, print_chars, nullptr, desc);
        }
        result_ = WriteDataImpl(at, src, size);
    }

    void Stream::WriteData(const void *src, size_t size, const char *desc, PrintChars print_chars) {
        WriteDataAt(offset_, src, size, desc, print_chars);
        offset_ += size;
    }

	void Stream::MoveData(size_t dst_offset, size_t src_offset, size_t size) {
		if (Failed(result_)) {
			return;
		}
		if (log_stream_) {
			log_stream_->Writef("; move data: [%zu, %zu) -> [%zu, %zu)\n",
			                    src_offset, src_offset + size, dst_offset, dst_offset + size);
		}
		result_ = MoveDataImpl(dst_offset, src_offset, size);
	}

	void Stream::Truncate(size_t size) {
		if (Failed(result_)) {
			return;
		}
		if (log_stream_) {
			log_stream_->Writef("; truncate to %" PRIzd " (0x%zu)\n", size,
			                    size);
		}
		result_ = TruncateImpl(size);
		if (Succeeded(result_) && offset_ > size) {
			offset_ = size;
		}
	}

	void Stream::Writef(const char *format, ...) {
		WABT_SNPRINTF_ALLOCA(buffer, length, format);
		WriteData(buffer, length);
	}

	void Stream::WriteMemoryDump(const void *start,
	                             size_t size,
	                             size_t offset,
	                             PrintChars print_chars,
	                             const char *prefix,
	                             const char *desc) {
		const uint8_t *p = static_cast<const uint8_t *>(start);
		const uint8_t *end = p + size;
		while (p < end) {
			const uint8_t *line = p;
			const uint8_t *line_end = p + DUMP_OCTETS_PER_LINE;
			if (prefix) {
				Writef("%s", prefix);
			}
			Writef("%07zu: ", reinterpret_cast<intptr_t>(p) -
			                  reinterpret_cast<intptr_t>(start) + offset);
			while (p < line_end) {
				for (int i = 0; i < DUMP_OCTETS_PER_GROUP; ++i, ++p) {
					if (p < end) {
						Writef("%02x", *p);
					} else {
						WriteChar(' ');
						WriteChar(' ');
					}
				}
				WriteChar(' ');
			}

			if (print_chars == PrintChars::Yes) {
				WriteChar(' ');
				p = line;
				for (int i = 0; i < DUMP_OCTETS_PER_LINE && p < end; ++i, ++p)
					WriteChar(isprint(*p) ? *p : '.');
			}

			/* if there are multiple lines, only print the desc on the last one */
			if (p >= end && desc) {
				Writef("  ; %s", desc);
			}
			WriteChar('\n');
		}
	}



	MemoryStream::MemoryStream(Stream *log_stream) : Stream(log_stream), buf_(new OutputBuffer()) {}

	OutputBuffer *MemoryStream::ReleaseOutputBuffer() {
        return std::move(buf_);
    }

	Result MemoryStream::WriteDataImpl(size_t dst_offset, const void *src, size_t size) {
		if (size == 0) {
			return Result::Ok;
		}
		size_t end = dst_offset + size;
		if (end > buf_->data.size()) {
			buf_->data.resize(end);
//            buf_->data.size_ += end - buf_->data.size();
		}
		uint8_t *dst = &buf_->data[dst_offset];
		memcpy(dst, src, size);
		return Result::Ok;
	}

	Result MemoryStream::MoveDataImpl(size_t dst_offset,
	                                  size_t src_offset,
	                                  size_t size) {
		if (size == 0) {
			return Result::Ok;
		}
		size_t src_end = src_offset + size;
		size_t dst_end = dst_offset + size;
		size_t end = src_end > dst_end ? src_end : dst_end;
		if (end > buf_->data.size()) {
			buf_->data.resize(end);
		}

		uint8_t *dst = &buf_->data[dst_offset];
		uint8_t *src = &buf_->data[src_offset];
		memmove(dst, src, size);
		return Result::Ok;
	}

	Result MemoryStream::TruncateImpl(size_t size) {
        if (size > buf_->data.size()) {
            return Result::Error;
        }
        buf_->data.resize(size);
        return Result::Ok;
    }


}  // namespace wabt

#pragma clang diagnostic pop
