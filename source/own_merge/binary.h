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

#ifndef WABT_BINARY_H_
#define WABT_BINARY_H_

#include "common.h"

#include <cassert>
#include <cstdint>
#include <vector>


class ValueType;

using Index = uint32_t;
using TypeVector = List<ValueType>;

class ValueType {
public:
	// Matches binary format, do not change.
	enum Enum : int32_t {
		I32 = -0x01,        // 0x7f
		I64 = -0x02,        // 0x7e
		F32 = -0x03,        // 0x7d
		F64 = -0x04,        // 0x7c
		V128 = -0x05,       // 0x7b
		I8 = -0x06,         // 0x7a  : packed-type only, used in gc and as v128 lane
		I16 = -0x07,        // 0x79  : packed-type only, used in gc and as v128 lane
		FuncRef = -0x10,    // 0x70
		ExternRef = -0x11,  // 0x6f
		Func = -0x20,       // 0x60
		Struct = -0x21,     // 0x5f
		Array = -0x22,      // 0x5e
		Void = -0x40,       // 0x40
		___ = Void,         // Convenient for the opcode table in opcode.h

		Any = 0,          // Not actually specified, but useful for type-checking
		I8U = 4,   // Not actually specified, but used internally with load/store
		I16U = 6,  // Not actually specified, but used internally with load/store
		I32U = 7,  // Not actually specified, but used internally with load/store
	};

	ValueType() = default;  // Provided so ValueType can be member of a union.
	ValueType(int32_t code) : enum_(static_cast<Enum>(code)) {}

	ValueType(Enum e) : enum_(e) {}

	operator Enum() const { return enum_; }

	bool IsRef() const {
		return enum_ == ValueType::ExternRef || enum_ == ValueType::FuncRef;
	}

	bool IsNullableRef() const {
		// Currently all reftypes are nullable
		return IsRef();
	}

	const char *GetName() const {
		switch (enum_) {
			case ValueType::I32:
				return "i32";
			case ValueType::I64:
				return "i64";
			case ValueType::F32:
				return "f32";
			case ValueType::F64:
				return "f64";
			case ValueType::V128:
				return "v128";
			case ValueType::I8:
				return "i8";
			case ValueType::I16:
				return "i16";
			case ValueType::FuncRef:
				return "funcref";
			case ValueType::Func:
				return "func";
			case ValueType::Void:
				return "void";
			case ValueType::Any:
				return "any";
			case ValueType::ExternRef:
				return "externref";
			default:
				return "<type_index>";
		}
	}

	const char *GetRefKindName() const {
		switch (enum_) {
			case ValueType::FuncRef:
				return "func";
			case ValueType::ExternRef:
				return "extern";
			case ValueType::Struct:
				return "struct";
			case ValueType::Array:
				return "array";
			default:
				return "<invalid>";
		}
	}

	// Functions for handling types that are an index into the type section.
	// These are always positive integers. They occur in the binary format in
	// block signatures, e.g.
	//
	//   (block (result i32 i64) ...)
	//
	// is encoded as
	//
	//   (type $T (func (result i32 i64)))
	//   ...
	//   (block (type $T) ...)
	//
	bool IsIndex() const { return static_cast<int32_t>(enum_) >= 0; }

	Index GetIndex() const {
		assert(IsIndex());
		return static_cast<Index>(enum_);
	}

	TypeVector GetInlineVector() const {
		assert(!IsIndex());
		switch (enum_) {
			case ValueType::Void:
				return TypeVector();

			case ValueType::I32:
			case ValueType::I64:
			case ValueType::F32:
			case ValueType::F64:
			case ValueType::V128:
            case ValueType::FuncRef:
            case ValueType::ExternRef:
//				return TypeVector(this, this + 1);
                return TypeVector(*this, this + 1);
            default:
                WABT_UNREACHABLE;
		}
	}

private:
	Enum enum_;
};


#define WABT_BINARY_MAGIC 0x6d736100
#define WABT_BINARY_VERSION 1
#define WABT_BINARY_LIMITS_HAS_MAX_FLAG 0x1
#define WABT_BINARY_LIMITS_IS_SHARED_FLAG 0x2
#define WABT_BINARY_LIMITS_IS_64_FLAG 0x4
#define WABT_BINARY_LIMITS_ALL_FLAGS                                     \
  (WABT_BINARY_LIMITS_HAS_MAX_FLAG | WABT_BINARY_LIMITS_IS_SHARED_FLAG | \
   WABT_BINARY_LIMITS_IS_64_FLAG)

#define WABT_BINARY_SECTION_NAME "name"
#define WABT_BINARY_SECTION_RELOC "reloc"
#define WABT_BINARY_SECTION_LINKING "linking"
#define WABT_BINARY_SECTION_DYLINK "dylink"
#define WABT_BINARY_SECTION_DYLINK0 "dylink.0"

#define WABT_FOREACH_BINARY_SECTION(V) \
  V(Custom, custom, 0)                 \
  V(Type, type, 1)                     \
  V(Import, import, 2)                 \
  V(FuncType, function, 3)             \
  V(Table, table, 4)                   \
  V(Memory, memory, 5)                 \
  V(Tag, tag, 13)                      \
  V(Global, global, 6)                 \
  V(Export, export, 7)                 \
  V(Start, start, 8)                   \
  V(Elem, elem, 9)                     \
  V(DataCount, data_count, 12)         \
  V(Code, code, 10)                    \
  V(Data, data, 11)

namespace wabt {

/* clang-format off */
    enum class SectionType {
        Invalid = ~0, // ~0 ??
        Custom = 0,
        Type = 1,
        Import = 2,
        FuncType = 3,
        Table = 4,
        Memory = 5,
        Global = 6,
        Export = 7,
        Start = 8,
        Elem = 9,
        Code = 10,
        Data = 11,
        DataCount = 12,
        Event = 13,
		Tag = 14,
#define V(Name, name, code) Name = code,
//  WABT_FOREACH_BINARY_SECTION(V)
#undef V

		First = Custom,
		Last = Tag
	};
/* clang-format on */
	static const int kBinarySectionCount = WABT_ENUM_COUNT(SectionType);

	enum class SectionTypeOrder {
#define V(Name, name, code) Name,
		WABT_FOREACH_BINARY_SECTION(V)
#undef V
	};

	SectionTypeOrder GetSectionOrder(SectionType);

	const char *GetSectionName(SectionType);

	enum class NameSectionSubsection {
		Module = 0,
		Function = 1,
		Local = 2,
		Label = 3,
		Type = 4,
		Table = 5,
		Memory = 6,
		Global = 7,
		ElemSegment = 8,
		DataSegment = 9,
		Last = DataSegment,
	};

	const char *GetNameSectionSubsectionName(NameSectionSubsection subsec);

}  // namespace wabt

#endif /* WABT_BINARY_H_ */
