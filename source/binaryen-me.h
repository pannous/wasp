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

//================
//  C API
//
// The first part of the API lets you create modules and their parts.
//
// The second part of the API lets you perform operations on modules.
//
// The third part of the API lets you provide a general control-flow
//   graph (CFG) as input.
//
// The final part of the API contains miscellaneous utilities like
//   debugging for the API itself.
//
// ---------------
//
// Thread safety: You can create Expressions in parallel, as they do not
//                er to global state. AddFunction is also
//                thread-safe, which means that you can create functions and
//                their contents in multiple threads. This is important since
//                functions are where the majority of the work is done.
//                Other methods - creating imports, export_section, etc. - are
//                not currently thread-safe (as there is typically no need
//                to parallelize them).
//
//================

#ifndef wasm__c_h
#define wasm__c_h

#include <stddef.h>
#include <stdint.h>

#ifdef __GNUC__
#define WASM_DEPRECATED __attribute__((deprecated))
#elif defined(_MSC_VER)
#define WASM_DEPRECATED __declspec(deprecated)
#else
#define WASM_DEPRECATED
#endif

#if defined(__EMSCRIPTEN__)
#include <emscripten.h>
#define _API EMSCRIPTEN_KEEPALIVE
#elif defined(_MSC_VER) && !defined(BUILD_STATIC_LIBRARY)
#define _API __declspec(dllexport)
#else
#define _API
#endif

#ifdef __cplusplus
#define _(NAME)                                                     \
  namespace wasm {                                                             \
  class NAME;                                                                  \
  };                                                                           \
  typedef class wasm::NAME* ##NAME##;
#else
#define _(NAME) typedef struct ##NAME* ##NAME##;
#endif

#ifdef __cplusplus
extern "C" {
#endif

//
// ========== Module Creation ==========
//

// Index
//
// Used for internal indexes and list sizes.

typedef uint32_t Index;

// Core types (call to get the value of each; you can cache them, they
// never change)

typedef uintptr_t Type;

_API Type TypeNone(void);

_API Type TypeInt32(void);

_API Type TypeInt64(void);

_API Type TypeFloat32(void);

_API Type TypeFloat64(void);

_API Type TypeVec128(void);

_API Type TypeFunc(void);

_API Type TypeExtern(void);

_API Type TypeExn(void);

_API Type TypeAny(void);

_API Type TypeEq(void);

_API Type TypeI31(void);

_API Type TypeUnreachable(void);
// Not a real type. Used as the last parameter to Block to let
// the API figure out the type instead of providing one.
_API Type TypeAuto(void);

_API Type TypeCreate(Type *valueTypes,
                     uint32_t numTypes);

_API uint32_t TypeArity(Type t);

_API void TypeExpand(Type t, Type *buf);

WASM_DEPRECATED Type None(void);

WASM_DEPRECATED Type Int32(void);

WASM_DEPRECATED Type Int64(void);

WASM_DEPRECATED Type Float32(void);

WASM_DEPRECATED Type Float64(void);

WASM_DEPRECATED Type Undefined(void);

// Expression ids (call to get the value of each; you can cache them)

typedef uint32_t ExpressionId;

_API ExpressionId InvalidId(void);

#define DELEGATE(CLASS_TO_VISIT)                                               \
  _API ExpressionId ##CLASS_TO_VISIT##Id(void);

#include "wasm-delegations.h"

// External kinds (call to get the value of each; you can cache them)

typedef uint32_t ExternalKind;

_API ExternalKind ExternalFunction(void);

_API ExternalKind ExternalTable(void);

_API ExternalKind ExternalMemory(void);

_API ExternalKind ExternalGlobal(void);

_API ExternalKind ExternalEvent(void);

// Features. Call to get the value of each; you can cache them. Use bitwise
// operators to combine and test particular features.

typedef uint32_t Features;

_API Features FeatureMVP(void);

_API Features FeatureAtomics(void);

_API Features FeatureBulkMemory(void);

_API Features FeatureMutableGlobals(void);

_API Features FeatureNontrappingFPToInt(void);

_API Features FeatureSignExt(void);

_API Features FeatureSIMD128(void);

_API Features FeatureExceptionHandling(void);

_API Features FeatureTailCall(void);

_API Features FeatureerenceTypes(void);

_API Features FeatureMultivalue(void);

_API Features FeatureGC(void);

_API Features FeatureMemory64(void);

_API Features FeatureAll(void);

// Modules
//
// Modules contain lists of functions, imports, export_section, function types. The
// Add* methods create them on a module. The module owns them and will free
// their memory when the module is disposed of.
//
// Expressions are also allocated inside modules, and freed with the module.
// They are not created by Add* methods, since they are not added directly on
// the module, instead, they are arguments to other expression (and then they
// are the children of that AST node), or to a function (and then they are the
// body of that function).
//
// A module can also contain a function table for indirect calls, a memory,
// and a start method.

_(Module);// creates a Ref

_API Module ModuleCreate(void);

_API void ModuleDispose(Module module);

// Literals. These are passed by value.

struct Literal {
	uintptr_t type;
	union {
		int32_t i32;
		int64_t i64;
		float f32;
		double f64;
		uint8_t v128[16];
		const char *func;
		// TODO: exn
	};
};

_API struct Literal LiteralInt32(int32_t x);

_API struct Literal LiteralInt64(int64_t x);

_API struct Literal LiteralFloat32(float x);

_API struct Literal LiteralFloat64(double x);

_API struct Literal LiteralVec128(const uint8_t x[16]);

_API struct Literal LiteralFloat32Bits(int32_t x);

_API struct Literal LiteralFloat64Bits(int64_t x);

// Expressions
//
// Some expression have a Op, which is the more
// specific operation/opcode.
//
// Some expression have optional parameters, like Return may not
// return a value. You can supply a NULL pointer in those cases.
//
// For more information, see wasm.h

typedef int32_t Op;

_API Op ClzInt32(void);

_API Op CtzInt32(void);

_API Op PopcntInt32(void);

_API Op NegFloat32(void);

_API Op AbsFloat32(void);

_API Op CeilFloat32(void);

_API Op FloorFloat32(void);

_API Op TruncFloat32(void);

_API Op NearestFloat32(void);

_API Op SqrtFloat32(void);

_API Op EqZInt32(void);

_API Op ClzInt64(void);

_API Op CtzInt64(void);

_API Op PopcntInt64(void);

_API Op NegFloat64(void);

_API Op AbsFloat64(void);

_API Op CeilFloat64(void);

_API Op FloorFloat64(void);

_API Op TruncFloat64(void);

_API Op NearestFloat64(void);

_API Op SqrtFloat64(void);

_API Op EqZInt64(void);

_API Op ExtendSInt32(void);

_API Op ExtendUInt32(void);

_API Op WrapInt64(void);

_API Op TruncSFloat32ToInt32(void);

_API Op TruncSFloat32ToInt64(void);

_API Op TruncUFloat32ToInt32(void);

_API Op TruncUFloat32ToInt64(void);

_API Op TruncSFloat64ToInt32(void);

_API Op TruncSFloat64ToInt64(void);

_API Op TruncUFloat64ToInt32(void);

_API Op TruncUFloat64ToInt64(void);

_API Op ReinterpretFloat32(void);

_API Op ReinterpretFloat64(void);

_API Op ConvertSInt32ToFloat32(void);

_API Op ConvertSInt32ToFloat64(void);

_API Op ConvertUInt32ToFloat32(void);

_API Op ConvertUInt32ToFloat64(void);

_API Op ConvertSInt64ToFloat32(void);

_API Op ConvertSInt64ToFloat64(void);

_API Op ConvertUInt64ToFloat32(void);

_API Op ConvertUInt64ToFloat64(void);

_API Op PromoteFloat32(void);

_API Op DemoteFloat64(void);

_API Op ReinterpretInt32(void);

_API Op ReinterpretInt64(void);

_API Op ExtendS8Int32(void);

_API Op ExtendS16Int32(void);

_API Op ExtendS8Int64(void);

_API Op ExtendS16Int64(void);

_API Op ExtendS32Int64(void);

_API Op AddInt32(void);

_API Op SubInt32(void);

_API Op MulInt32(void);

_API Op DivSInt32(void);

_API Op DivUInt32(void);

_API Op RemSInt32(void);

_API Op RemUInt32(void);

_API Op AndInt32(void);

_API Op OrInt32(void);

_API Op XorInt32(void);

_API Op ShlInt32(void);

_API Op ShrUInt32(void);

_API Op ShrSInt32(void);

_API Op RotLInt32(void);

_API Op RotRInt32(void);

_API Op EqInt32(void);

_API Op NeInt32(void);

_API Op LtSInt32(void);

_API Op LtUInt32(void);

_API Op LeSInt32(void);

_API Op LeUInt32(void);

_API Op GtSInt32(void);

_API Op GtUInt32(void);

_API Op GeSInt32(void);

_API Op GeUInt32(void);

_API Op AddInt64(void);

_API Op SubInt64(void);

_API Op MulInt64(void);

_API Op DivSInt64(void);

_API Op DivUInt64(void);

_API Op RemSInt64(void);

_API Op RemUInt64(void);

_API Op AndInt64(void);

_API Op OrInt64(void);

_API Op XorInt64(void);

_API Op ShlInt64(void);

_API Op ShrUInt64(void);

_API Op ShrSInt64(void);

_API Op RotLInt64(void);

_API Op RotRInt64(void);

_API Op EqInt64(void);

_API Op NeInt64(void);

_API Op LtSInt64(void);

_API Op LtUInt64(void);

_API Op LeSInt64(void);

_API Op LeUInt64(void);

_API Op GtSInt64(void);

_API Op GtUInt64(void);

_API Op GeSInt64(void);

_API Op GeUInt64(void);

_API Op AddFloat32(void);

_API Op SubFloat32(void);

_API Op MulFloat32(void);

_API Op DivFloat32(void);

_API Op CopySignFloat32(void);

_API Op MinFloat32(void);

_API Op MaxFloat32(void);

_API Op EqFloat32(void);

_API Op NeFloat32(void);

_API Op LtFloat32(void);

_API Op LeFloat32(void);

_API Op GtFloat32(void);

_API Op GeFloat32(void);

_API Op AddFloat64(void);

_API Op SubFloat64(void);

_API Op MulFloat64(void);

_API Op DivFloat64(void);

_API Op CopySignFloat64(void);

_API Op MinFloat64(void);

_API Op MaxFloat64(void);

_API Op EqFloat64(void);

_API Op NeFloat64(void);

_API Op LtFloat64(void);

_API Op LeFloat64(void);

_API Op GtFloat64(void);

_API Op GeFloat64(void);

_API Op AtomicRMWAdd(void);

_API Op AtomicRMWSub(void);

_API Op AtomicRMWAnd(void);

_API Op AtomicRMWOr(void);

_API Op AtomicRMWXor(void);

_API Op AtomicRMWXchg(void);

_API Op TruncSatSFloat32ToInt32(void);

_API Op TruncSatSFloat32ToInt64(void);

_API Op TruncSatUFloat32ToInt32(void);

_API Op TruncSatUFloat32ToInt64(void);

_API Op TruncSatSFloat64ToInt32(void);

_API Op TruncSatSFloat64ToInt64(void);

_API Op TruncSatUFloat64ToInt32(void);

_API Op TruncSatUFloat64ToInt64(void);

_API Op SplatVecI8x16(void);

_API Op ExtractLaneSVecI8x16(void);

_API Op ExtractLaneUVecI8x16(void);

_API Op ReplaceLaneVecI8x16(void);

_API Op SplatVecI16x8(void);

_API Op ExtractLaneSVecI16x8(void);

_API Op ExtractLaneUVecI16x8(void);

_API Op ReplaceLaneVecI16x8(void);

_API Op SplatVecI32x4(void);

_API Op ExtractLaneVecI32x4(void);

_API Op ReplaceLaneVecI32x4(void);

_API Op SplatVecI64x2(void);

_API Op ExtractLaneVecI64x2(void);

_API Op ReplaceLaneVecI64x2(void);

_API Op SplatVecF32x4(void);

_API Op ExtractLaneVecF32x4(void);

_API Op ReplaceLaneVecF32x4(void);

_API Op SplatVecF64x2(void);

_API Op ExtractLaneVecF64x2(void);

_API Op ReplaceLaneVecF64x2(void);

_API Op EqVecI8x16(void);

_API Op NeVecI8x16(void);

_API Op LtSVecI8x16(void);

_API Op LtUVecI8x16(void);

_API Op GtSVecI8x16(void);

_API Op GtUVecI8x16(void);

_API Op LeSVecI8x16(void);

_API Op LeUVecI8x16(void);

_API Op GeSVecI8x16(void);

_API Op GeUVecI8x16(void);

_API Op EqVecI16x8(void);

_API Op NeVecI16x8(void);

_API Op LtSVecI16x8(void);

_API Op LtUVecI16x8(void);

_API Op GtSVecI16x8(void);

_API Op GtUVecI16x8(void);

_API Op LeSVecI16x8(void);

_API Op LeUVecI16x8(void);

_API Op GeSVecI16x8(void);

_API Op GeUVecI16x8(void);

_API Op EqVecI32x4(void);

_API Op NeVecI32x4(void);

_API Op LtSVecI32x4(void);

_API Op LtUVecI32x4(void);

_API Op GtSVecI32x4(void);

_API Op GtUVecI32x4(void);

_API Op LeSVecI32x4(void);

_API Op LeUVecI32x4(void);

_API Op GeSVecI32x4(void);

_API Op GeUVecI32x4(void);

_API Op EqVecF32x4(void);

_API Op NeVecF32x4(void);

_API Op LtVecF32x4(void);

_API Op GtVecF32x4(void);

_API Op LeVecF32x4(void);

_API Op GeVecF32x4(void);

_API Op EqVecF64x2(void);

_API Op NeVecF64x2(void);

_API Op LtVecF64x2(void);

_API Op GtVecF64x2(void);

_API Op LeVecF64x2(void);

_API Op GeVecF64x2(void);

_API Op NotVec128(void);

_API Op AndVec128(void);

_API Op OrVec128(void);

_API Op XorVec128(void);

_API Op AndNotVec128(void);

_API Op BitselectVec128(void);
// TODO: Add i8x16.popcnt to C and JS APIs once merged to the proposal
_API Op AbsVecI8x16(void);

_API Op NegVecI8x16(void);

_API Op AnyTrueVecI8x16(void);

_API Op AllTrueVecI8x16(void);

_API Op BitmaskVecI8x16(void);

_API Op ShlVecI8x16(void);

_API Op ShrSVecI8x16(void);

_API Op ShrUVecI8x16(void);

_API Op AddVecI8x16(void);

_API Op AddSatSVecI8x16(void);

_API Op AddSatUVecI8x16(void);

_API Op SubVecI8x16(void);

_API Op SubSatSVecI8x16(void);

_API Op SubSatUVecI8x16(void);

_API Op MulVecI8x16(void);

_API Op MinSVecI8x16(void);

_API Op MinUVecI8x16(void);

_API Op MaxSVecI8x16(void);

_API Op MaxUVecI8x16(void);

_API Op AvgrUVecI8x16(void);

_API Op AbsVecI16x8(void);

_API Op NegVecI16x8(void);

_API Op AnyTrueVecI16x8(void);

_API Op AllTrueVecI16x8(void);

_API Op BitmaskVecI16x8(void);

_API Op ShlVecI16x8(void);

_API Op ShrSVecI16x8(void);

_API Op ShrUVecI16x8(void);

_API Op AddVecI16x8(void);

_API Op AddSatSVecI16x8(void);

_API Op AddSatUVecI16x8(void);

_API Op SubVecI16x8(void);

_API Op SubSatSVecI16x8(void);

_API Op SubSatUVecI16x8(void);

_API Op MulVecI16x8(void);

_API Op MinSVecI16x8(void);

_API Op MinUVecI16x8(void);

_API Op MaxSVecI16x8(void);

_API Op MaxUVecI16x8(void);

_API Op AvgrUVecI16x8(void);
// TODO: Add i16x8.q15mulr_sat_s to C and JS APIs once merged to the proposal
// TODO: Add extending multiplications to APIs once they are merged as well
_API Op AbsVecI32x4(void);

_API Op NegVecI32x4(void);

_API Op AnyTrueVecI32x4(void);

_API Op AllTrueVecI32x4(void);

_API Op BitmaskVecI32x4(void);

_API Op ShlVecI32x4(void);

_API Op ShrSVecI32x4(void);

_API Op ShrUVecI32x4(void);

_API Op AddVecI32x4(void);

_API Op SubVecI32x4(void);

_API Op MulVecI32x4(void);

_API Op MinSVecI32x4(void);

_API Op MinUVecI32x4(void);

_API Op MaxSVecI32x4(void);

_API Op MaxUVecI32x4(void);

_API Op DotSVecI16x8ToVecI32x4(void);

_API Op NegVecI64x2(void);

_API Op AnyTrueVecI64x2(void);

_API Op AllTrueVecI64x2(void);

_API Op ShlVecI64x2(void);

_API Op ShrSVecI64x2(void);

_API Op ShrUVecI64x2(void);

_API Op AddVecI64x2(void);

_API Op SubVecI64x2(void);

_API Op MulVecI64x2(void);

_API Op AbsVecF32x4(void);

_API Op NegVecF32x4(void);

_API Op SqrtVecF32x4(void);

_API Op QFMAVecF32x4(void);

_API Op QFMSVecF32x4(void);

_API Op AddVecF32x4(void);

_API Op SubVecF32x4(void);

_API Op MulVecF32x4(void);

_API Op DivVecF32x4(void);

_API Op MinVecF32x4(void);

_API Op MaxVecF32x4(void);

_API Op PMinVecF32x4(void);

_API Op PMaxVecF32x4(void);

_API Op CeilVecF32x4(void);

_API Op FloorVecF32x4(void);

_API Op TruncVecF32x4(void);

_API Op NearestVecF32x4(void);

_API Op AbsVecF64x2(void);

_API Op NegVecF64x2(void);

_API Op SqrtVecF64x2(void);

_API Op QFMAVecF64x2(void);

_API Op QFMSVecF64x2(void);

_API Op AddVecF64x2(void);

_API Op SubVecF64x2(void);

_API Op MulVecF64x2(void);

_API Op DivVecF64x2(void);

_API Op MinVecF64x2(void);

_API Op MaxVecF64x2(void);

_API Op PMinVecF64x2(void);

_API Op PMaxVecF64x2(void);

_API Op CeilVecF64x2(void);

_API Op FloorVecF64x2(void);

_API Op TruncVecF64x2(void);

_API Op NearestVecF64x2(void);

_API Op TruncSatSVecF32x4ToVecI32x4(void);

_API Op TruncSatUVecF32x4ToVecI32x4(void);

_API Op TruncSatSVecF64x2ToVecI64x2(void);

_API Op TruncSatUVecF64x2ToVecI64x2(void);

_API Op ConvertSVecI32x4ToVecF32x4(void);

_API Op ConvertUVecI32x4ToVecF32x4(void);

_API Op ConvertSVecI64x2ToVecF64x2(void);

_API Op ConvertUVecI64x2ToVecF64x2(void);

_API Op LoadSplatVec8x16(void);

_API Op LoadSplatVec16x8(void);

_API Op LoadSplatVec32x4(void);

_API Op LoadSplatVec64x2(void);

_API Op LoadExtSVec8x8ToVecI16x8(void);

_API Op LoadExtUVec8x8ToVecI16x8(void);

_API Op LoadExtSVec16x4ToVecI32x4(void);

_API Op LoadExtUVec16x4ToVecI32x4(void);

_API Op LoadExtSVec32x2ToVecI64x2(void);

_API Op LoadExtUVec32x2ToVecI64x2(void);
// TODO: Add Load{32,64}Zero to C and JS APIs once merged to proposal
_API Op NarrowSVecI16x8ToVecI8x16(void);

_API Op NarrowUVecI16x8ToVecI8x16(void);

_API Op NarrowSVecI32x4ToVecI16x8(void);

_API Op NarrowUVecI32x4ToVecI16x8(void);

_API Op WidenLowSVecI8x16ToVecI16x8(void);

_API Op WidenHighSVecI8x16ToVecI16x8(void);

_API Op WidenLowUVecI8x16ToVecI16x8(void);

_API Op WidenHighUVecI8x16ToVecI16x8(void);

_API Op WidenLowSVecI16x8ToVecI32x4(void);

_API Op WidenHighSVecI16x8ToVecI32x4(void);

_API Op WidenLowUVecI16x8ToVecI32x4(void);

_API Op WidenHighUVecI16x8ToVecI32x4(void);

_API Op SwizzleVec8x16(void);

_(Expression);

// Block: name can be NULL. Specifying Undefined() as the 'type'
//        parameter indicates that the block's type shall be figured out
//        automatically instead of explicitly providing it. This conforms
//        to the behavior before the 'type' parameter has been introduced.
_API Expression
Block(Module module,
      const char *name,
      Expression *children,
      Index numChildren,
      Type type);
// If: ifFalse can be NULL
_API Expression If(Module module,
                   Expression condition,
                   Expression ifTrue,
                   Expression ifFalse);

_API Expression Loop(Module module,
                     const char *in,
                     Expression body);
// Break: value and condition can be NULL
_API Expression
Break(Module module,
      const char *name,
      Expression condition,
      Expression value);
// Switch: value can be NULL
_API Expression
Switch(Module module,
       const char **names,
       Index numNames,
       const char *defaultName,
       Expression condition,
       Expression value);
// Call: Note the 'returnType' parameter. You must declare the
//       type returned by the function being called, as that
//       function might not have been created yet, so we don't
//       know what it is.
_API Expression Call(Module module,
                     const char *target,
                     Expression *operands,
                     Index numOperands,
                     Type returnType);

_API Expression
CallIndirect(Module module,
             Expression target,
             Expression *operands,
             Index numOperands,
             Type params,
             Type results);

_API Expression
ReturnCall(Module module,
           const char *target,
           Expression *operands,
           Index numOperands,
           Type returnType);

_API Expression
ReturnCallIndirect(Module module,
                   Expression target,
                   Expression *operands,
                   Index numOperands,
                   Type params,
                   Type results);

// LocalGet: Note the 'type' parameter. It might seem redundant, since the
//           local at that index must have a type. However, this API lets you
//           build code "top-down": create a node, then its parents, and so
//           on, and finally create the function at the end. (Note that in fact
//           you do not mention a function when creating Expressions, only
//           a module.) And since LocalGet is a leaf node, we need to be told
//           its type. (Other nodes detect their type either from their
//           type or their opcode, or failing that, their children. But
//           LocalGet has no children, it is where a "stream" of type info
//           begins.)
//           Note also that the index of a local can er to a param or
//           a var, that is, either a parameter to the function or a variable
//           declared when you call AddFunction. See AddFunction
//           for more details.
_API Expression LocalGet(Module module,
                         Index index,
                         Type type);

_API Expression LocalSet(
		Module module, Index index, Expression value);

_API Expression LocalTee(Module module,
                         Index index,
                         Expression value,
                         Type type);

_API Expression GlobalGet(Module module,
                          const char *name,
                          Type type);

_API Expression GlobalSet(
		Module module, const char *name, Expression value);
// Load: align can be 0, in which case it will be the natural alignment (equal
// to bytes)
_API Expression Load(Module module,
                     uint32_t bytes,
                     int8_t signed_,
                     uint32_t offset,
                     uint32_t align,
                     Type type,
                     Expression ptr);
// Store: align can be 0, in which case it will be the natural alignment (equal
// to bytes)
_API Expression Store(Module module,
                      uint32_t bytes,
                      uint32_t offset,
                      uint32_t align,
                      Expression ptr,
                      Expression value,
                      Type type);

_API Expression Const(Module module,
                      struct Literal value);

_API Expression Unary(Module module,
                      Op op,
                      Expression value);

_API Expression Binary(Module module,
                       Op op,
                       Expression left,
                       Expression right);

_API Expression
Select(Module module,
       Expression condition,
       Expression ifTrue,
       Expression ifFalse,
       Type type);

_API Expression Drop(Module module,
                     Expression value);
// Return: value can be NULL
_API Expression Return(Module module,
                       Expression value);

_API Expression MemorySize(Module module);

_API Expression
MemoryGrow(Module module, Expression delta);

_API Expression Nop(Module module);

_API Expression
Unreachable(Module module);

_API Expression
AtomicLoad(Module module,
           uint32_t bytes,
           uint32_t offset,
           Type type,
           Expression ptr);

_API Expression
AtomicStore(Module module,
            uint32_t bytes,
            uint32_t offset,
            Expression ptr,
            Expression value,
            Type type);

_API Expression
AtomicRMW(Module module,
          Op op,
          Index bytes,
          Index offset,
          Expression ptr,
          Expression value,
          Type type);

_API Expression
AtomicCmpxchg(Module module,
              Index bytes,
              Index offset,
              Expression ptr,
              Expression expected,
              Expression replacement,
              Type type);

_API Expression
AtomicWait(Module module,
           Expression ptr,
           Expression expected,
           Expression timeout,
           Type type);

_API Expression
AtomicNotify(Module module,
             Expression ptr,
             Expression notifyCount);

_API Expression
AtomicFence(Module module);

_API Expression
SIMDExtract(Module module,
            Op op,
            Expression vec,
            uint8_t index);

_API Expression
SIMDReplace(Module module,
            Op op,
            Expression vec,
            uint8_t index,
            Expression value);

_API Expression
SIMDShuffle(Module module,
            Expression left,
            Expression right,
            const uint8_t mask[16]);

_API Expression SIMDTernary(Module module,
                            Op op,
                            Expression a,
                            Expression b,
                            Expression c);

_API Expression
SIMDShift(Module module,
          Op op,
          Expression vec,
          Expression shift);

_API Expression SIMDLoad(Module module,
                         Op op,
                         uint32_t offset,
                         uint32_t align,
                         Expression ptr);

_API Expression
MemoryInit(Module module,
           uint32_t segment,
           Expression dest,
           Expression offset,
           Expression size);

_API Expression DataDrop(Module module,
                         uint32_t segment);

_API Expression
MemoryCopy(Module module,
           Expression dest,
           Expression source,
           Expression size);

_API Expression
MemoryFill(Module module,
           Expression dest,
           Expression value,
           Expression size);

_API Expression Null(Module module,
                     Type type);

_API Expression
IsNull(Module module, Expression value);

_API Expression Func(Module module,
                     const char *func,
                     Type type);

_API Expression Eq(Module module,
                   Expression left,
                   Expression right);

_API Expression Try(Module module,
                    Expression body,
                    Expression catchBody);

_API Expression
Throw(Module module,
      const char *event,
      Expression *operands,
      Index numOperands);

_API Expression
Rethrow(Module module, Expression exn);

_API Expression
BrOnExn(Module module,
        const char *name,
        const char *eventName,
        Expression exn);

_API Expression
TupleMake(Module module,
          Expression *operands,
          Index numOperands);

_API Expression TupleExtract(
		Module module, Expression tuple, Index index);

_API Expression Pop(Module module,
                    Type type);

_API Expression I31New(Module module,
                       Expression value);

_API Expression I31Get(Module module,
                       Expression i31,
                       int signed_);
// TODO (gc): .test
// TODO (gc): .cast
// TODO (gc): br_on_cast
// TODO (gc): rtt.canon
// TODO (gc): rtt.sub
// TODO (gc): struct.new
// TODO (gc): struct.get
// TODO (gc): struct.set
// TODO (gc): array.new
// TODO (gc): array.get
// TODO (gc): array.set
// TODO (gc): array.len

// Expression

// Gets the id (kind) of the given expression.
_API ExpressionId
ExpressionGetId(Expression expr);
// Gets the type of the given expression.
_API Type ExpressionGetType(Expression expr);
// Sets the type of the given expression.
_API void ExpressionSetType(Expression expr,
                            Type type);
// Prints text format of the given expression to stdout.
_API void ExpressionPrint(Expression expr);
// Re-finalizes an expression after it has been modified.
_API void ExpressionFinalize(Expression expr);
// Makes a deep copy of the given expression.
_API Expression
ExpressionCopy(Expression expr, Module module);

// Block

// Gets the name (label) of a `block` expression.
_API const char *BlockGetName(Expression expr);
// Sets the name (label) of a `block` expression.
_API void BlockSetName(Expression expr,
                       const char *name);
// Gets the number of child expression of a `block` expression.
_API Index
BlockGetNumChildren(Expression expr);
// Gets the child expression at the specified index of a `block` expression.
_API Expression
BlockGetChildAt(Expression expr, Index index);
// Sets (replaces) the child expression at the specified index of a `block`
// expression.
_API void BlockSetChildAt(Expression expr,
                          Index index,
                          Expression childExpr);
// Appends a child expression to a `block` expression, returning its insertion
// index.
_API Index BlockAppendChild(
		Expression expr, Expression childExpr);
// Inserts a child expression at the specified index of a `block` expression,
// moving existing children including the one previously at that index one index
// up.
_API void BlockInsertChildAt(Expression expr,
                             Index index,
                             Expression childExpr);
// Removes the child expression at the specified index of a `block` expression,
// moving all subsequent children one index down. Returns the child expression.
_API Expression
BlockRemoveChildAt(Expression expr, Index index);

// If

// Gets the condition expression of an `if` expression.
_API Expression
IfGetCondition(Expression expr);
// Sets the condition expression of an `if` expression.
_API void IfSetCondition(Expression expr,
                         Expression condExpr);
// Gets the ifTrue (then) expression of an `if` expression.
_API Expression
IfGetIfTrue(Expression expr);
// Sets the ifTrue (then) expression of an `if` expression.
_API void IfSetIfTrue(Expression expr,
                      Expression ifTrueExpr);
// Gets the ifFalse (else) expression, if any, of an `if` expression.
_API Expression
IfGetIfFalse(Expression expr);
// Sets the ifFalse (else) expression, if any, of an `if` expression.
_API void IfSetIfFalse(Expression expr,
                       Expression ifFalseExpr);

// Loop

// Gets the name (label) of a `loop` expression.
_API const char *LoopGetName(Expression expr);
// Sets the name (label) of a `loop` expression.
_API void LoopSetName(Expression expr,
                      const char *name);
// Gets the body expression of a `loop` expression.
_API Expression
LoopGetBody(Expression expr);
// Sets the body expression of a `loop` expression.
_API void LoopSetBody(Expression expr,
                      Expression bodyExpr);

// Break

// Gets the name (target label) of a `br` or `br_if` expression.
_API const char *BreakGetName(Expression expr);
// Sets the name (target label) of a `br` or `br_if` expression.
_API void BreakSetName(Expression expr,
                       const char *name);
// Gets the condition expression, if any, of a `br_if` expression. No condition
// indicates a `br` expression.
_API Expression
BreakGetCondition(Expression expr);
// Sets the condition expression, if any, of a `br_if` expression. No condition
// makes it a `br` expression.
_API void BreakSetCondition(Expression expr,
                            Expression condExpr);
// Gets the value expression, if any, of a `br` or `br_if` expression.
_API Expression
BreakGetValue(Expression expr);
// Sets the value expression, if any, of a `br` or `br_if` expression.
_API void BreakSetValue(Expression expr,
                        Expression valueExpr);

// Switch

// Gets the number of names (target labels) of a `br_table` expression.
_API Index
SwitchGetNumNames(Expression expr);
// Gets the name (target label) at the specified index of a `br_table`
// expression.
_API const char *SwitchGetNameAt(Expression expr,
                                 Index index);
// Sets the name (target label) at the specified index of a `br_table`
// expression.
_API void SwitchSetNameAt(Expression expr,
                          Index index,
                          const char *name);
// Appends a name to a `br_table` expression, returning its insertion index.
_API Index SwitchAppendName(Expression expr,
                            const char *name);
// Inserts a name at the specified index of a `br_table` expression, moving
// existing names including the one previously at that index one index up.
_API void SwitchInsertNameAt(Expression expr,
                             Index index,
                             const char *name);
// Removes the name at the specified index of a `br_table` expression, moving
// all subsequent names one index down. Returns the name.
_API const char *SwitchRemoveNameAt(Expression expr,
                                    Index index);
// Gets the default name (target label), if any, of a `br_table` expression.
_API const char *
SwitchGetDefaultName(Expression expr);
// Sets the default name (target label), if any, of a `br_table` expression.
_API void SwitchSetDefaultName(Expression expr,
                               const char *name);
// Gets the condition expression of a `br_table` expression.
_API Expression
SwitchGetCondition(Expression expr);
// Sets the condition expression of a `br_table` expression.
_API void SwitchSetCondition(Expression expr,
                             Expression condExpr);
// Gets the value expression, if any, of a `br_table` expression.
_API Expression
SwitchGetValue(Expression expr);
// Sets the value expression, if any, of a `br_table` expression.
_API void SwitchSetValue(Expression expr,
                         Expression valueExpr);

// Call

// Gets the target function name of a `call` expression.
_API const char *CallGetTarget(Expression expr);
// Sets the target function name of a `call` expression.
_API void CallSetTarget(Expression expr,
                        const char *target);
// Gets the number of operands of a `call` expression.
_API Index
CallGetNumOperands(Expression expr);
// Gets the operand expression at the specified index of a `call` expression.
_API Expression
CallGetOperandAt(Expression expr, Index index);
// Sets the operand expression at the specified index of a `call` expression.
_API void CallSetOperandAt(Expression expr,
                           Index index,
                           Expression operandExpr);
// Appends an operand expression to a `call` expression, returning its insertion
// index.
_API Index CallAppendOperand(
		Expression expr, Expression operandExpr);
// Inserts an operand expression at the specified index of a `call` expression,
// moving existing operands including the one previously at that index one index
// up.
_API void
CallInsertOperandAt(Expression expr,
                    Index index,
                    Expression operandExpr);
// Removes the operand expression at the specified index of a `call` expression,
// moving all subsequent operands one index down. Returns the operand
// expression.
_API Expression
CallRemoveOperandAt(Expression expr, Index index);
// Gets whether the specified `call` expression is a tail call.
_API int CallIsReturn(Expression expr);
// Sets whether the specified `call` expression is a tail call.
_API void CallSetReturn(Expression expr,
                        int isReturn);

// CallIndirect

// Gets the target expression of a `call_indirect` expression.
_API Expression
CallIndirectGetTarget(Expression expr);
// Sets the target expression of a `call_indirect` expression.
_API void
CallIndirectSetTarget(Expression expr,
                      Expression targetExpr);
// Gets the number of operands of a `call_indirect` expression.
_API Index
CallIndirectGetNumOperands(Expression expr);
// Gets the operand expression at the specified index of a `call_indirect`
// expression.
_API Expression CallIndirectGetOperandAt(
		Expression expr, Index index);
// Sets the operand expression at the specified index of a `call_indirect`
// expression.
_API void
CallIndirectSetOperandAt(Expression expr,
                         Index index,
                         Expression operandExpr);
// Appends an operand expression to a `call_indirect` expression, returning its
// insertion index.
_API Index CallIndirectAppendOperand(
		Expression expr, Expression operandExpr);
// Inserts an operand expression at the specified index of a `call_indirect`
// expression, moving existing operands including the one previously at that
// index one index up.
_API void
CallIndirectInsertOperandAt(Expression expr,
                            Index index,
                            Expression operandExpr);
// Removes the operand expression at the specified index of a `call_indirect`
// expression, moving all subsequent operands one index down. Returns the
// operand expression.
_API Expression CallIndirectRemoveOperandAt(
		Expression expr, Index index);
// Gets whether the specified `call_indirect` expression is a tail call.
_API int CallIndirectIsReturn(Expression expr);
// Sets whether the specified `call_indirect` expression is a tail call.
_API void CallIndirectSetReturn(Expression expr,
                                int isReturn);
// Gets the parameter types of the specified `call_indirect` expression.
_API Type
CallIndirectGetParams(Expression expr);
// Sets the parameter types of the specified `call_indirect` expression.
_API void CallIndirectSetParams(Expression expr,
                                Type params);
// Gets the result types of the specified `call_indirect` expression.
_API Type
CallIndirectGetResults(Expression expr);
// Sets the result types of the specified `call_indirect` expression.
_API void CallIndirectSetResults(Expression expr,
                                 Type params);

// LocalGet

// Gets the local index of a `local.get` expression.
_API Index LocalGetGetIndex(Expression expr);
// Sets the local index of a `local.get` expression.
_API void LocalGetSetIndex(Expression expr,
                           Index index);

// LocalSet

// Gets whether a `local.set` tees its value (is a `local.tee`). True if the
// expression has a type other than `none`.
_API int LocalSetIsTee(Expression expr);
// Gets the local index of a `local.set` or `local.tee` expression.
_API Index LocalSetGetIndex(Expression expr);
// Sets the local index of a `local.set` or `local.tee` expression.
_API void LocalSetSetIndex(Expression expr,
                           Index index);
// Gets the value expression of a `local.set` or `local.tee` expression.
_API Expression
LocalSetGetValue(Expression expr);
// Sets the value expression of a `local.set` or `local.tee` expression.
_API void LocalSetSetValue(Expression expr,
                           Expression valueExpr);

// GlobalGet

// Gets the name of the global being accessed by a `global.get` expression.
_API const char *GlobalGetGetName(Expression expr);
// Sets the name of the global being accessed by a `global.get` expression.
_API void GlobalGetSetName(Expression expr,
                           const char *name);

// GlobalSet

// Gets the name of the global being accessed by a `global.set` expression.
_API const char *GlobalSetGetName(Expression expr);
// Sets the name of the global being accessed by a `global.set` expression.
_API void GlobalSetSetName(Expression expr,
                           const char *name);
// Gets the value expression of a `global.set` expression.
_API Expression
GlobalSetGetValue(Expression expr);
// Sets the value expression of a `global.set` expression.
_API void GlobalSetSetValue(Expression expr,
                            Expression valueExpr);

// MemoryGrow

// Gets the delta of a `memory.grow` expression.
_API Expression
MemoryGrowGetDelta(Expression expr);
// Sets the delta of a `memory.grow` expression.
_API void MemoryGrowSetDelta(Expression expr,
                             Expression delta);

// Load

// Gets whether a `load` expression is atomic (is an `atomic.load`).
_API int LoadIsAtomic(Expression expr);
// Sets whether a `load` expression is atomic (is an `atomic.load`).
_API void LoadSetAtomic(Expression expr,
                        int isAtomic);
// Gets whether a `load` expression operates on a signed value (`_s`).
_API int LoadIsSigned(Expression expr);
// Sets whether a `load` expression operates on a signed value (`_s`).
_API void LoadSetSigned(Expression expr,
                        int isSigned);
// Gets the constant offset of a `load` expression.
_API uint32_t LoadGetOffset(Expression expr);
// Sets the constant offset of a `load` expression.
_API void LoadSetOffset(Expression expr,
                        uint32_t offset);
// Gets the number of bytes loaded by a `load` expression.
_API uint32_t LoadGetBytes(Expression expr);
// Sets the number of bytes loaded by a `load` expression.
_API void LoadSetBytes(Expression expr,
                       uint32_t bytes);
// Gets the byte alignment of a `load` expression.
_API uint32_t LoadGetAlign(Expression expr);
// Sets the byte alignment of a `load` expression.
_API void LoadSetAlign(Expression expr,
                       uint32_t align);
// Gets the pointer expression of a `load` expression.
_API Expression
LoadGetPtr(Expression expr);
// Sets the pointer expression of a `load` expression.
_API void LoadSetPtr(Expression expr,
                     Expression ptrExpr);

// Store

// Gets whether a `store` expression is atomic (is an `atomic.store`).
_API int StoreIsAtomic(Expression expr);
// Sets whether a `store` expression is atomic (is an `atomic.store`).
_API void StoreSetAtomic(Expression expr,
                         int isAtomic);
// Gets the number of bytes stored by a `store` expression.
_API uint32_t StoreGetBytes(Expression expr);
// Sets the number of bytes stored by a `store` expression.
_API void StoreSetBytes(Expression expr,
                        uint32_t bytes);
// Gets the constant offset of a `store` expression.
_API uint32_t StoreGetOffset(Expression expr);
// Sets the constant offset of a `store` expression.
_API void StoreSetOffset(Expression expr,
                         uint32_t offset);
// Gets the byte alignment of a `store` expression.
_API uint32_t StoreGetAlign(Expression expr);
// Sets the byte alignment of a `store` expression.
_API void StoreSetAlign(Expression expr,
                        uint32_t align);
// Gets the pointer expression of a `store` expression.
_API Expression
StoreGetPtr(Expression expr);
// Sets the pointer expression of a `store` expression.
_API void StoreSetPtr(Expression expr,
                      Expression ptrExpr);
// Gets the value expression of a `store` expression.
_API Expression
StoreGetValue(Expression expr);
// Sets the value expression of a `store` expression.
_API void StoreSetValue(Expression expr,
                        Expression valueExpr);
// Gets the value type of a `store` expression.
_API Type StoreGetValueType(Expression expr);
// Sets the value type of a `store` expression.
_API void StoreSetValueType(Expression expr,
                            Type valueType);

// Const

// Gets the 32-bit integer value of an `i32.const` expression.
_API int32_t ConstGetValueI32(Expression expr);
// Sets the 32-bit integer value of an `i32.const` expression.
_API void ConstSetValueI32(Expression expr,
                           int32_t value);
// Gets the 64-bit integer value of an `i64.const` expression.
_API int64_t ConstGetValueI64(Expression expr);
// Sets the 64-bit integer value of an `i64.const` expression.
_API void ConstSetValueI64(Expression expr,
                           int64_t value);
// Gets the low 32-bits of the 64-bit integer value of an `i64.const`
// expression.
_API int32_t ConstGetValueI64Low(Expression expr);
// Sets the low 32-bits of the 64-bit integer value of an `i64.const`
// expression.
_API void ConstSetValueI64Low(Expression expr,
                              int32_t valueLow);
// Gets the high 32-bits of the 64-bit integer value of an `i64.const`
// expression.
_API int32_t ConstGetValueI64High(Expression expr);
// Sets the high 32-bits of the 64-bit integer value of an `i64.const`
// expression.
_API void ConstSetValueI64High(Expression expr,
                               int32_t valueHigh);
// Gets the 32-bit float value of a `f32.const` expression.
_API float ConstGetValueF32(Expression expr);
// Sets the 32-bit float value of a `f32.const` expression.
_API void ConstSetValueF32(Expression expr,
                           float value);
// Gets the 64-bit float (double) value of a `f64.const` expression.
_API double ConstGetValueF64(Expression expr);
// Sets the 64-bit float (double) value of a `f64.const` expression.
_API void ConstSetValueF64(Expression expr,
                           double value);
// Reads the 128-bit vector value of a `v128.const` expression.
_API void ConstGetValueV128(Expression expr,
                            uint8_t *out);
// Sets the 128-bit vector value of a `v128.const` expression.
_API void ConstSetValueV128(Expression expr,
                            const uint8_t value[16]);

// Unary

// Gets the operation being performed by a unary expression.
_API Op UnaryGetOp(Expression expr);
// Sets the operation being performed by a unary expression.
_API void UnarySetOp(Expression expr, Op op);
// Gets the value expression of a unary expression.
_API Expression
UnaryGetValue(Expression expr);
// Sets the value expression of a unary expression.
_API void UnarySetValue(Expression expr,
                        Expression valueExpr);

// Binary

// Gets the operation being performed by a binary expression.
_API Op BinaryGetOp(Expression expr);
// Sets the operation being performed by a binary expression.
_API void BinarySetOp(Expression expr,
                      Op op);
// Gets the left expression of a binary expression.
_API Expression
BinaryGetLeft(Expression expr);
// Sets the left expression of a binary expression.
_API void BinarySetLeft(Expression expr,
                        Expression leftExpr);
// Gets the right expression of a binary expression.
_API Expression
BinaryGetRight(Expression expr);
// Sets the right expression of a binary expression.
_API void BinarySetRight(Expression expr,
                         Expression rightExpr);

// Select

// Gets the expression becoming selected by a `select` expression if the
// condition turns out true.
_API Expression
SelectGetIfTrue(Expression expr);
// Sets the expression becoming selected by a `select` expression if the
// condition turns out true.
_API void SelectSetIfTrue(Expression expr,
                          Expression ifTrueExpr);
// Gets the expression becoming selected by a `select` expression if the
// condition turns out false.
_API Expression
SelectGetIfFalse(Expression expr);
// Sets the expression becoming selected by a `select` expression if the
// condition turns out false.
_API void SelectSetIfFalse(Expression expr,
                           Expression ifFalseExpr);
// Gets the condition expression of a `select` expression.
_API Expression
SelectGetCondition(Expression expr);
// Sets the condition expression of a `select` expression.
_API void SelectSetCondition(Expression expr,
                             Expression condExpr);

// Drop

// Gets the value expression being dropped by a `drop` expression.
_API Expression
DropGetValue(Expression expr);
// Sets the value expression being dropped by a `drop` expression.
_API void DropSetValue(Expression expr,
                       Expression valueExpr);

// Return

// Gets the value expression, if any, being returned by a `return` expression.
_API Expression
ReturnGetValue(Expression expr);
// Sets the value expression, if any, being returned by a `return` expression.
_API void ReturnSetValue(Expression expr,
                         Expression valueExpr);

// AtomicRMW

// Gets the operation being performed by an atomic read-modify-write expression.
_API Op AtomicRMWGetOp(Expression expr);
// Sets the operation being performed by an atomic read-modify-write expression.
_API void AtomicRMWSetOp(Expression expr,
                         Op op);
// Gets the number of bytes affected by an atomic read-modify-write expression.
_API uint32_t AtomicRMWGetBytes(Expression expr);
// Sets the number of bytes affected by an atomic read-modify-write expression.
_API void AtomicRMWSetBytes(Expression expr,
                            uint32_t bytes);
// Gets the constant offset of an atomic read-modify-write expression.
_API uint32_t AtomicRMWGetOffset(Expression expr);
// Sets the constant offset of an atomic read-modify-write expression.
_API void AtomicRMWSetOffset(Expression expr,
                             uint32_t offset);
// Gets the pointer expression of an atomic read-modify-write expression.
_API Expression
AtomicRMWGetPtr(Expression expr);
// Sets the pointer expression of an atomic read-modify-write expression.
_API void AtomicRMWSetPtr(Expression expr,
                          Expression ptrExpr);
// Gets the value expression of an atomic read-modify-write expression.
_API Expression
AtomicRMWGetValue(Expression expr);
// Sets the value expression of an atomic read-modify-write expression.
_API void AtomicRMWSetValue(Expression expr,
                            Expression valueExpr);

// AtomicCmpxchg

// Gets the number of bytes affected by an atomic compare and exchange
// expression.
_API uint32_t AtomicCmpxchgGetBytes(Expression expr);
// Sets the number of bytes affected by an atomic compare and exchange
// expression.
_API void AtomicCmpxchgSetBytes(Expression expr,
                                uint32_t bytes);
// Gets the constant offset of an atomic compare and exchange expression.
_API uint32_t
AtomicCmpxchgGetOffset(Expression expr);
// Sets the constant offset of an atomic compare and exchange expression.
_API void AtomicCmpxchgSetOffset(Expression expr,
                                 uint32_t offset);
// Gets the pointer expression of an atomic compare and exchange expression.
_API Expression
AtomicCmpxchgGetPtr(Expression expr);
// Sets the pointer expression of an atomic compare and exchange expression.
_API void AtomicCmpxchgSetPtr(Expression expr,
                              Expression ptrExpr);
// Gets the expression representing the expected value of an atomic compare and
// exchange expression.
_API Expression
AtomicCmpxchgGetExpected(Expression expr);
// Sets the expression representing the expected value of an atomic compare and
// exchange expression.
_API void
AtomicCmpxchgSetExpected(Expression expr,
                         Expression expectedExpr);
// Gets the replacement expression of an atomic compare and exchange expression.
_API Expression
AtomicCmpxchgGetReplacement(Expression expr);
// Sets the replacement expression of an atomic compare and exchange expression.
_API void
AtomicCmpxchgSetReplacement(Expression expr,
                            Expression replacementExpr);

// AtomicWait

// Gets the pointer expression of an `memory.atomic.wait` expression.
_API Expression
AtomicWaitGetPtr(Expression expr);
// Sets the pointer expression of an `memory.atomic.wait` expression.
_API void AtomicWaitSetPtr(Expression expr,
                           Expression ptrExpr);
// Gets the expression representing the expected value of an
// `memory.atomic.wait` expression.
_API Expression
AtomicWaitGetExpected(Expression expr);
// Sets the expression representing the expected value of an
// `memory.atomic.wait` expression.
_API void
AtomicWaitSetExpected(Expression expr,
                      Expression expectedExpr);
// Gets the timeout expression of an `memory.atomic.wait` expression.
_API Expression
AtomicWaitGetTimeout(Expression expr);
// Sets the timeout expression of an `memory.atomic.wait` expression.
_API void
AtomicWaitSetTimeout(Expression expr,
                     Expression timeoutExpr);
// Gets the expected type of an `memory.atomic.wait` expression.
_API Type
AtomicWaitGetExpectedType(Expression expr);
// Sets the expected type of an `memory.atomic.wait` expression.
_API void AtomicWaitSetExpectedType(Expression expr,
                                    Type expectedType);

// AtomicNotify

// Gets the pointer expression of an `memory.atomic.notify` expression.
_API Expression
AtomicNotifyGetPtr(Expression expr);
// Sets the pointer expression of an `memory.atomic.notify` expression.
_API void AtomicNotifySetPtr(Expression expr,
                             Expression ptrExpr);
// Gets the notify count expression of an `memory.atomic.notify` expression.
_API Expression
AtomicNotifyGetNotifyCount(Expression expr);
// Sets the notify count expression of an `memory.atomic.notify` expression.
_API void
AtomicNotifySetNotifyCount(Expression expr,
                           Expression notifyCountExpr);

// AtomicFence

// Gets the order of an `atomic.fence` expression.
_API uint8_t AtomicFenceGetOrder(Expression expr);
// Sets the order of an `atomic.fence` expression.
_API void AtomicFenceSetOrder(Expression expr,
                              uint8_t order);

// SIMDExtract

// Gets the operation being performed by a SIMD extract expression.
_API Op SIMDExtractGetOp(Expression expr);
// Sets the operation being performed by a SIMD extract expression.
_API void SIMDExtractSetOp(Expression expr,
                           Op op);
// Gets the vector expression a SIMD extract expression extracts from.
_API Expression
SIMDExtractGetVec(Expression expr);
// Sets the vector expression a SIMD extract expression extracts from.
_API void SIMDExtractSetVec(Expression expr,
                            Expression vecExpr);
// Gets the index of the extracted lane of a SIMD extract expression.
_API uint8_t SIMDExtractGetIndex(Expression expr);
// Sets the index of the extracted lane of a SIMD extract expression.
_API void SIMDExtractSetIndex(Expression expr,
                              uint8_t index);

// SIMDReplace

// Gets the operation being performed by a SIMD replace expression.
_API Op SIMDReplaceGetOp(Expression expr);
// Sets the operation being performed by a SIMD replace expression.
_API void SIMDReplaceSetOp(Expression expr,
                           Op op);
// Gets the vector expression a SIMD replace expression replaces in.
_API Expression
SIMDReplaceGetVec(Expression expr);
// Sets the vector expression a SIMD replace expression replaces in.
_API void SIMDReplaceSetVec(Expression expr,
                            Expression vecExpr);
// Gets the index of the replaced lane of a SIMD replace expression.
_API uint8_t SIMDReplaceGetIndex(Expression expr);
// Sets the index of the replaced lane of a SIMD replace expression.
_API void SIMDReplaceSetIndex(Expression expr,
                              uint8_t index);
// Gets the value expression a SIMD replace expression replaces with.
_API Expression
SIMDReplaceGetValue(Expression expr);
// Sets the value expression a SIMD replace expression replaces with.
_API void SIMDReplaceSetValue(Expression expr,
                              Expression valueExpr);

// SIMDShuffle

// Gets the left expression of a SIMD shuffle expression.
_API Expression
SIMDShuffleGetLeft(Expression expr);
// Sets the left expression of a SIMD shuffle expression.
_API void SIMDShuffleSetLeft(Expression expr,
                             Expression leftExpr);
// Gets the right expression of a SIMD shuffle expression.
_API Expression
SIMDShuffleGetRight(Expression expr);
// Sets the right expression of a SIMD shuffle expression.
_API void SIMDShuffleSetRight(Expression expr,
                              Expression rightExpr);
// Gets the 128-bit mask of a SIMD shuffle expression.
_API void SIMDShuffleGetMask(Expression expr,
                             uint8_t *mask);
// Sets the 128-bit mask of a SIMD shuffle expression.
_API void SIMDShuffleSetMask(Expression expr,
                             const uint8_t mask[16]);

// SIMDTernary

// Gets the operation being performed by a SIMD ternary expression.
_API Op SIMDTernaryGetOp(Expression expr);
// Sets the operation being performed by a SIMD ternary expression.
_API void SIMDTernarySetOp(Expression expr,
                           Op op);
// Gets the first operand expression of a SIMD ternary expression.
_API Expression
SIMDTernaryGetA(Expression expr);
// Sets the first operand expression of a SIMD ternary expression.
_API void SIMDTernarySetA(Expression expr,
                          Expression aExpr);
// Gets the second operand expression of a SIMD ternary expression.
_API Expression
SIMDTernaryGetB(Expression expr);
// Sets the second operand expression of a SIMD ternary expression.
_API void SIMDTernarySetB(Expression expr,
                          Expression bExpr);
// Gets the third operand expression of a SIMD ternary expression.
_API Expression
SIMDTernaryGetC(Expression expr);
// Sets the third operand expression of a SIMD ternary expression.
_API void SIMDTernarySetC(Expression expr,
                          Expression cExpr);

// SIMDShift

// Gets the operation being performed by a SIMD shift expression.
_API Op SIMDShiftGetOp(Expression expr);
// Sets the operation being performed by a SIMD shift expression.
_API void SIMDShiftSetOp(Expression expr,
                         Op op);
// Gets the expression being shifted by a SIMD shift expression.
_API Expression
SIMDShiftGetVec(Expression expr);
// Sets the expression being shifted by a SIMD shift expression.
_API void SIMDShiftSetVec(Expression expr,
                          Expression vecExpr);
// Gets the expression representing the shift of a SIMD shift expression.
_API Expression
SIMDShiftGetShift(Expression expr);
// Sets the expression representing the shift of a SIMD shift expression.
_API void SIMDShiftSetShift(Expression expr,
                            Expression shiftExpr);

// SIMDLoad

// Gets the operation being performed by a SIMD load expression.
_API Op SIMDLoadGetOp(Expression expr);
// Sets the operation being performed by a SIMD load expression.
_API void SIMDLoadSetOp(Expression expr,
                        Op op);
// Gets the constant offset of a SIMD load expression.
_API uint32_t SIMDLoadGetOffset(Expression expr);
// Sets the constant offset of a SIMD load expression.
_API void SIMDLoadSetOffset(Expression expr,
                            uint32_t offset);
// Gets the byte alignment of a SIMD load expression.
_API uint32_t SIMDLoadGetAlign(Expression expr);
// Sets the byte alignment of a SIMD load expression.
_API void SIMDLoadSetAlign(Expression expr,
                           uint32_t align);
// Gets the pointer expression of a SIMD load expression.
_API Expression
SIMDLoadGetPtr(Expression expr);
// Sets the pointer expression of a SIMD load expression.
_API void SIMDLoadSetPtr(Expression expr,
                         Expression ptrExpr);

// MemoryInit

// Gets the index of the segment being initialized by a `memory.init`
// expression.
_API uint32_t MemoryInitGetSegment(Expression expr);
// Sets the index of the segment being initialized by a `memory.init`
// expression.
_API void MemoryInitSetSegment(Expression expr,
                               uint32_t segmentIndex);
// Gets the destination expression of a `memory.init` expression.
_API Expression
MemoryInitGetDest(Expression expr);
// Sets the destination expression of a `memory.init` expression.
_API void MemoryInitSetDest(Expression expr,
                            Expression destExpr);
// Gets the offset expression of a `memory.init` expression.
_API Expression
MemoryInitGetOffset(Expression expr);
// Sets the offset expression of a `memory.init` expression.
_API void MemoryInitSetOffset(Expression expr,
                              Expression offsetExpr);
// Gets the size expression of a `memory.init` expression.
_API Expression
MemoryInitGetSize(Expression expr);
// Sets the size expression of a `memory.init` expression.
_API void MemoryInitSetSize(Expression expr,
                            Expression sizeExpr);

// DataDrop

// Gets the index of the segment being dropped by a `memory.drop` expression.
_API uint32_t DataDropGetSegment(Expression expr);
// Sets the index of the segment being dropped by a `memory.drop` expression.
_API void DataDropSetSegment(Expression expr,
                             uint32_t segmentIndex);

// MemoryCopy

// Gets the destination expression of a `memory.copy` expression.
_API Expression
MemoryCopyGetDest(Expression expr);
// Sets the destination expression of a `memory.copy` expression.
_API void MemoryCopySetDest(Expression expr,
                            Expression destExpr);
// Gets the source expression of a `memory.copy` expression.
_API Expression
MemoryCopyGetSource(Expression expr);
// Sets the source expression of a `memory.copy` expression.
_API void MemoryCopySetSource(Expression expr,
                              Expression sourceExpr);
// Gets the size expression (number of bytes copied) of a `memory.copy`
// expression.
_API Expression
MemoryCopyGetSize(Expression expr);
// Sets the size expression (number of bytes copied) of a `memory.copy`
// expression.
_API void MemoryCopySetSize(Expression expr,
                            Expression sizeExpr);

// MemoryFill

// Gets the destination expression of a `memory.fill` expression.
_API Expression
MemoryFillGetDest(Expression expr);
// Sets the destination expression of a `memory.fill` expression.
_API void MemoryFillSetDest(Expression expr,
                            Expression destExpr);
// Gets the value expression of a `memory.fill` expression.
_API Expression
MemoryFillGetValue(Expression expr);
// Sets the value expression of a `memory.fill` expression.
_API void MemoryFillSetValue(Expression expr,
                             Expression valueExpr);
// Gets the size expression (number of bytes filled) of a `memory.fill`
// expression.
_API Expression
MemoryFillGetSize(Expression expr);
// Sets the size expression (number of bytes filled) of a `memory.fill`
// expression.
_API void MemoryFillSetSize(Expression expr,
                            Expression sizeExpr);

// IsNull

// Gets the value expression tested to be null of a `.is_null` expression.
_API Expression
IsNullGetValue(Expression expr);
// Sets the value expression tested to be null of a `.is_null` expression.
_API void IsNullSetValue(Expression expr,
                         Expression valueExpr);

// Func

// Gets the name of the function being wrapped by a `.func` expression.
_API const char *FuncGetFunc(Expression expr);
// Sets the name of the function being wrapped by a `.func` expression.
_API void FuncSetFunc(Expression expr,
                      const char *funcName);

// Eq

// Gets the left expression of a `.eq` expression.
_API Expression
EqGetLeft(Expression expr);
// Sets the left expression of a `.eq` expression.
_API void EqSetLeft(Expression expr,
                    Expression left);
// Gets the right expression of a `.eq` expression.
_API Expression
EqGetRight(Expression expr);
// Sets the right expression of a `.eq` expression.
_API void EqSetRight(Expression expr,
                     Expression right);

// Try

// Gets the body expression of a `try` expression.
_API Expression
TryGetBody(Expression expr);
// Sets the body expression of a `try` expression.
_API void TrySetBody(Expression expr,
                     Expression bodyExpr);
// Gets the catch body expression of a `try` expression.
_API Expression
TryGetCatchBody(Expression expr);
// Sets the catch body expression of a `try` expression.
_API void TrySetCatchBody(Expression expr,
                          Expression catchBodyExpr);

// Throw

// Gets the name of the event being thrown by a `throw` expression.
_API const char *ThrowGetEvent(Expression expr);
// Sets the name of the event being thrown by a `throw` expression.
_API void ThrowSetEvent(Expression expr,
                        const char *eventName);
// Gets the number of operands of a `throw` expression.
_API Index
ThrowGetNumOperands(Expression expr);
// Gets the operand at the specified index of a `throw` expression.
_API Expression
ThrowGetOperandAt(Expression expr, Index index);
// Sets the operand at the specified index of a `throw` expression.
_API void ThrowSetOperandAt(Expression expr,
                            Index index,
                            Expression operandExpr);
// Appends an operand expression to a `throw` expression, returning its
// insertion index.
_API Index ThrowAppendOperand(
		Expression expr, Expression operandExpr);
// Inserts an operand expression at the specified index of a `throw` expression,
// moving existing operands including the one previously at that index one index
// up.
_API void
ThrowInsertOperandAt(Expression expr,
                     Index index,
                     Expression operandExpr);
// Removes the operand expression at the specified index of a `throw`
// expression, moving all subsequent operands one index down. Returns the
// operand expression.
_API Expression
ThrowRemoveOperandAt(Expression expr, Index index);

// Rethrow

// Gets the exception erence expression of a `rethrow` expression.
_API Expression
RethrowGetExn(Expression expr);
// Sets the exception erence expression of a `rethrow` expression.
_API void RethrowSetExn(Expression expr,
                        Expression exnExpr);

// BrOnExn

// Gets the name of the event triggering a `br_on_exn` expression.
_API const char *BrOnExnGetEvent(Expression expr);
// Sets the name of the event triggering a `br_on_exn` expression.
_API void BrOnExnSetEvent(Expression expr,
                          const char *eventName);
// Gets the name (target label) of a `br_on_exn` expression.
_API const char *BrOnExnGetName(Expression expr);
// Sets the name (target label) of a `br_on_exn` expression.
_API void BrOnExnSetName(Expression expr,
                         const char *name);
// Gets the expression erence expression of a `br_on_exn` expression.
_API Expression
BrOnExnGetExn(Expression expr);
// Sets the expression erence expression of a `br_on_exn` expression.
_API void BrOnExnSetExn(Expression expr,
                        Expression exnExpr);

// TupleMake

// Gets the number of operands of a `tuple.make` expression.
_API Index
TupleMakeGetNumOperands(Expression expr);
// Gets the operand at the specified index of a `tuple.make` expression.
_API Expression
TupleMakeGetOperandAt(Expression expr, Index index);
// Sets the operand at the specified index of a `tuple.make` expression.
_API void
TupleMakeSetOperandAt(Expression expr,
                      Index index,
                      Expression operandExpr);
// Appends an operand expression to a `tuple.make` expression, returning its
// insertion index.
_API Index TupleMakeAppendOperand(
		Expression expr, Expression operandExpr);
// Inserts an operand expression at the specified index of a `tuple.make`
// expression, moving existing operands including the one previously at that
// index one index up.
_API void
TupleMakeInsertOperandAt(Expression expr,
                         Index index,
                         Expression operandExpr);
// Removes the operand expression at the specified index of a `tuple.make`
// expression, moving all subsequent operands one index down. Returns the
// operand expression.
_API Expression TupleMakeRemoveOperandAt(
		Expression expr, Index index);

// TupleExtract

// Gets the tuple extracted from of a `tuple.extract` expression.
_API Expression
TupleExtractGetTuple(Expression expr);
// Sets the tuple extracted from of a `tuple.extract` expression.
_API void TupleExtractSetTuple(Expression expr,
                               Expression tupleExpr);
// Gets the index extracted at of a `tuple.extract` expression.
_API Index
TupleExtractGetIndex(Expression expr);
// Sets the index extracted at of a `tuple.extract` expression.
_API void TupleExtractSetIndex(Expression expr,
                               Index index);

// I31New

// Gets the value expression of an `i31.new` expression.
_API Expression
I31NewGetValue(Expression expr);
// Sets the value expression of an `i31.new` expression.
_API void I31NewSetValue(Expression expr,
                         Expression valueExpr);

// I31Get

// Gets the i31 expression of an `i31.get` expression.
_API Expression
I31GetGetI31(Expression expr);
// Sets the i31 expression of an `i31.get` expression.
_API void I31GetSetI31(Expression expr,
                       Expression i31Expr);
// Gets whether an `i31.get` expression returns a signed value (`_s`).
_API int I31GetIsSigned(Expression expr);
// Sets whether an `i31.get` expression returns a signed value (`_s`).
_API void I31GetSetSigned(Expression expr,
                          int signed_);

// Functions

_(Function);

// Adds a function to the module. This is thread-safe.
// @varTypes: the types of variables. In WebAssembly, vars share
//            an index space with params. In other words, params come from
//            the function type, and vars are provided in this call, and
//            together they are all the locals. The order is first params
//            and then vars, so if you have one param it will be at index
//            0 (and written $0), and if you also have 2 vars they will be
//            at indexes 1 and 2, etc., that is, they share an index space.
_API Function
AddFunction(Module module,
            const char *name,
            Type params,
            Type results,
            Type *varTypes,
            Index numVarTypes,
            Expression body);
// Gets a function erence by name.
_API Function GetFunction(Module module,
                          const char *name);
// Removes a function by name.
_API void RemoveFunction(Module module,
                         const char *name);

// Gets the number of functions in the module.
_API uint32_t GetNumFunctions(Module module);
// Get function pointer from its index.
_API Function
GetFunctionByIndex(Module module, Index id);

// Imports

_API void AddFunctionImport(Module module,
                            const char *internalName,
                            const char *externalModuleName,
                            const char *externalBaseName,
                            Type params,
                            Type results);

_API void AddTableImport(Module module,
                         const char *internalName,
                         const char *externalModuleName,
                         const char *externalBaseName);

_API void AddMemoryImport(Module module,
                          const char *internalName,
                          const char *externalModuleName,
                          const char *externalBaseName,
                          uint8_t shared);

_API void AddGlobalImport(Module module,
                          const char *internalName,
                          const char *externalModuleName,
                          const char *externalBaseName,
                          Type globalType,
                          int mutable_);

_API void AddEventImport(Module module,
                         const char *internalName,
                         const char *externalModuleName,
                         const char *externalBaseName,
                         uint32_t attribute,
                         Type params,
                         Type results);

// Exports

_(Export);

WASM_DEPRECATED Export AddExport(Module module,
                                 const char *internalName,
                                 const char *externalName);

_API Export AddFunctionExport(
		Module module, const char *internalName, const char *externalName);

_API Export AddTableExport(Module module,
                           const char *internalName,
                           const char *externalName);

_API Export AddMemoryExport(
		Module module, const char *internalName, const char *externalName);

_API Export AddGlobalExport(
		Module module, const char *internalName, const char *externalName);

_API Export AddEventExport(Module module,
                           const char *internalName,
                           const char *externalName);

_API void RemoveExport(Module module,
                       const char *externalName);

// Globals

_(Global);

_API Global AddGlobal(Module module,
                      const char *name,
                      Type type,
                      int8_t mutable_,
                      Expression init);
// Gets a global erence by name.
_API Global GetGlobal(Module module,
                      const char *name);

_API void RemoveGlobal(Module module,
                       const char *name);

// Events

_(Event);

_API Event AddEvent(Module module,
                    const char *name,
                    uint32_t attribute,
                    Type params,
                    Type results);

_API Event GetEvent(Module module,
                    const char *name);

_API void RemoveEvent(Module module,
                      const char *name);

// Function table. One per module

// TODO: Add support for multiple segments in SetFunctionTable.
_API void SetFunctionTable(Module module,
                           Index initial,
                           Index maximum,
                           const char **funcNames,
                           Index numFuncNames,
                           Expression offset);

_API int IsFunctionTableImported(Module module);

_API Index
GetNumFunctionTableSegments(Module module);

_API Expression GetFunctionTableSegmentOffset(
		Module module, Index segmentId);

_API Index GetFunctionTableSegmentLength(
		Module module, Index segmentId);

_API const char *GetFunctionTableSegmentData(
		Module module, Index segmentId, Index dataId);

// Memory. One per module

// Each segment has data in segments, a start offset in segmentOffsets, and a
// size in segmentSizes. exportName can be NULL
_API void SetMemory(Module module,
                    Index initial,
                    Index maximum,
                    const char *exportName,
                    const char **segments,
                    int8_t *segmentPassive,
                    Expression *segmentOffsets,
                    Index *segmentSizes,
                    Index numSegments,
                    uint8_t shared);

// Memory segments. Query utilities.

_API uint32_t GetNumMemorySegments(Module module);

_API uint32_t
GetMemorySegmentByteOffset(Module module, Index id);

_API size_t GetMemorySegmentByteLength(Module module,
                                       Index id);

_API int GetMemorySegmentPassive(Module module,
                                 Index id);

_API void CopyMemorySegmentData(Module module,
                                Index id,
                                char *buffer);

// Start function. One per module

_API void SetStart(Module module,
                   Function start);

// Features

// These control what features are allowed when validation and in passes.
_API Features
ModuleGetFeatures(Module module);

_API void ModuleSetFeatures(Module module,
                            Features features);

//
// ========== Module Operations ==========
//

// Parse a module in s-expression text format
_API Module ModuleParse(const char *text);

// Print a module to stdout in s-expression text format. Useful for debugging.
_API void ModulePrint(Module module);

// Print a module to stdout in asm.js syntax.
_API void ModulePrintAsmjs(Module module);

// Validate a module, showing errors on problems.
//  @return 0 if an error occurred, 1 if validated succesfully
_API int ModuleValidate(Module module);

// Runs the standard optimization passes on the module. Uses the currently set
// global optimize and shrink level.
_API void ModuleOptimize(Module module);

// Gets the currently set optimize level. Applies to all modules, globally.
// 0, 1, 2 correspond to -O0, -O1, -O2 (default), etc.
_API int GetOptimizeLevel(void);

// Sets the optimization level to use. Applies to all modules, globally.
// 0, 1, 2 correspond to -O0, -O1, -O2 (default), etc.
_API void SetOptimizeLevel(int level);

// Gets the currently set shrink level. Applies to all modules, globally.
// 0, 1, 2 correspond to -O0, -Os (default), -Oz.
_API int GetShrinkLevel(void);

// Sets the shrink level to use. Applies to all modules, globally.
// 0, 1, 2 correspond to -O0, -Os (default), -Oz.
_API void SetShrinkLevel(int level);

// Gets whether generating debug information is currently enabled or not.
// Applies to all modules, globally.
_API int GetDebugInfo(void);

// Enables or disables debug information in emitted binaries.
// Applies to all modules, globally.
_API void SetDebugInfo(int on);

// Gets whether the low 1K of memory can be considered unused when optimizing.
// Applies to all modules, globally.
_API int GetLowMemoryUnused(void);

// Enables or disables whether the low 1K of memory can be considered unused
// when optimizing. Applies to all modules, globally.
_API void SetLowMemoryUnused(int on);

// Gets whether fast math optimizations are enabled, ignoring for example
// corner cases of floating-point math like NaN changes.
// Applies to all modules, globally.
_API int GetFastMath(void);

// Enables or disables fast math optimizations, ignoring for example
// corner cases of floating-point math like NaN changes.
// Applies to all modules, globally.
_API void SetFastMath(int value);

// Gets the value of the specified arbitrary pass argument.
// Applies to all modules, globally.
_API const char *GetPassArgument(const char *name);

// Sets the value of the specified arbitrary pass argument. Removes the
// respective argument if `value` is NULL. Applies to all modules, globally.
_API void SetPassArgument(const char *name, const char *value);

// Clears all arbitrary pass arguments.
// Applies to all modules, globally.
_API void ClearPassArguments();

// Gets the function size at which we always inline.
// Applies to all modules, globally.
_API Index GetAlwaysInlineMaxSize(void);

// Sets the function size at which we always inline.
// Applies to all modules, globally.
_API void SetAlwaysInlineMaxSize(Index size);

// Gets the function size which we inline when functions are lightweight.
// Applies to all modules, globally.
_API Index GetFlexibleInlineMaxSize(void);

// Sets the function size which we inline when functions are lightweight.
// Applies to all modules, globally.
_API void SetFlexibleInlineMaxSize(Index size);

// Gets the function size which we inline when there is only one caller.
// Applies to all modules, globally.
_API Index GetOneCallerInlineMaxSize(void);

// Sets the function size which we inline when there is only one caller.
// Applies to all modules, globally.
_API void SetOneCallerInlineMaxSize(Index size);

// Gets whether functions with loops are allowed to be inlined.
// Applies to all modules, globally.
_API int GetAllowInliningFunctionsWithLoops(void);

// Sets whether functions with loops are allowed to be inlined.
// Applies to all modules, globally.
_API void SetAllowInliningFunctionsWithLoops(int enabled);

// Runs the specified passes on the module. Uses the currently set global
// optimize and shrink level.
_API void ModuleRunPasses(Module module,
                          const char **passes,
                          Index numPasses);

// Auto-generate drop() operations where needed. This lets you generate code
// without worrying about where they are needed. (It is more efficient to do it
// yourself, but simpler to use autodrop).
_API void ModuleAutoDrop(Module module);

// Serialize a module into binary form. Uses the currently set global debugInfo
// option.
// @return how many bytes were written. This will be less than or equal to
//         outputSize
size_t _API ModuleWrite(Module module,
                        char *output,
                        size_t outputSize);

// Serialize a module in s-expression text format.
// @return how many bytes were written. This will be less than or equal to
//         outputSize
_API size_t ModuleWriteText(Module module,
                            char *output,
                            size_t outputSize);

typedef struct BufferSizes {
	size_t outputBytes;
	size_t sourceMapBytes;
} BufferSizes;

// Serialize a module into binary form including its source map. Uses the
// currently set global debugInfo option.
// @returns how many bytes were written. This will be less than or equal to
//          outputSize
_API BufferSizes
ModuleWriteWithSourceMap(Module module,
                         const char *url,
                         char *output,
                         size_t outputSize,
                         char *sourceMap,
                         size_t sourceMapSize);

// Result structure of ModuleAllocateAndWrite. Contained buffers have
// been allocated using malloc() and the user is expected to free() them
// manually once not needed anymore.
typedef struct ModuleAllocateAndWriteResult {
	void *binary;
	size_t binaryBytes;
	char *sourceMap;
} ModuleAllocateAndWriteResult;

// Serializes a module into binary form, optionally including its source map if
// sourceMapUrl has been specified. Uses the currently set global debugInfo
// option. Differs from ModuleWrite in that it implicitly allocates
// appropriate buffers using malloc(), and expects the user to free() them
// manually once not needed anymore.
_API ModuleAllocateAndWriteResult
ModuleAllocateAndWrite(Module module,
                       const char *sourceMapUrl);

// Serialize a module in s-expression form. Implicity allocates the returned
// char* with malloc(), and expects the user to free() them manually
// once not needed anymore.
_API char *ModuleAllocateAndWriteText(Module module);

// Deserialize a module from binary form.
_API Module ModuleRead(char *input, size_t inputSize);

// Execute a module in the  interpreter. This will create an instance of
// the module, run it in the interpreter - which means running the start method
// - and then destroying the instance.
_API void ModuleInterpret(Module module);

// Adds a debug info file name to the module and returns its index.
_API Index ModuleAddDebugInfoFileName(
		Module module, const char *filename);

// Gets the name of the debug info file at the specified index. Returns `NULL`
// if it does not exist.
_API const char *
ModuleGetDebugInfoFileName(Module module,
                           Index index);

//
// ========== Function Operations ==========
//

// Gets the name of the specified `Function`.
_API const char *FunctionGetName(Function func);
// Gets the type of the parameter at the specified index of the specified
// `Function`.
_API Type FunctionGetParams(Function func);
// Gets the result type of the specified `Function`.
_API Type FunctionGetResults(Function func);
// Gets the number of additional locals within the specified `Function`.
_API Index FunctionGetNumVars(Function func);
// Gets the type of the additional local at the specified index within the
// specified `Function`.
_API Type FunctionGetVar(Function func,
                         Index index);
// Gets the number of locals within the specified function. Includes parameters.
_API Index
FunctionGetNumLocals(Function func);
// Tests if the local at the specified index has a name.
_API int FunctionHasLocalName(Function func,
                              Index index);
// Gets the name of the local at the specified index.
_API const char *FunctionGetLocalName(Function func,
                                      Index index);
// Sets the name of the local at the specified index.
_API void FunctionSetLocalName(Function func,
                               Index index,
                               const char *name);
// Gets the body of the specified `Function`.
_API Expression
FunctionGetBody(Function func);
// Sets the body of the specified `Function`.
_API void FunctionSetBody(Function func,
                          Expression body);

// Runs the standard optimization passes on the function. Uses the currently set
// global optimize and shrink level.
_API void FunctionOptimize(Function func,
                           Module module);

// Runs the specified passes on the function. Uses the currently set global
// optimize and shrink level.
_API void FunctionRunPasses(Function func,
                            Module module,
                            const char **passes,
                            Index numPasses);

// Sets the debug location of the specified `Expression` within the specified
// `Function`.
_API void FunctionSetDebugLocation(Function func,
                                   Expression expr,
                                   Index fileIndex,
                                   Index lineNumber,
                                   Index columnNumber);

//
// ========== Global Operations ==========
//

// Gets the name of the specified `Global`.
_API const char *GlobalGetName(Global global);
// Gets the name of the `GlobalType` associated with the specified `Global`. May
// be `NULL` if the signature is implicit.
_API Type GlobalGetType(Global global);
// Returns true if the specified `Global` is mutable.
_API int GlobalIsMutable(Global global);
// Gets the initialization expression of the specified `Global`.
_API Expression
GlobalGetInitExpr(Global global);

//
// ========== Event Operations ==========
//

// Gets the name of the specified `Event`.
_API const char *EventGetName(Event event);
// Gets the attribute of the specified `Event`.
_API int EventGetAttribute(Event event);
// Gets the parameters type of the specified `Event`.
_API Type EventGetParams(Event event);
// Gets the results type of the specified `Event`.
_API Type EventGetResults(Event event);

//
// ========== Import Operations ==========
//

// Gets the external module name of the specified import.
_API const char *
FunctionImportGetModule(Function import);

_API const char *
GlobalImportGetModule(Global import);

_API const char *EventImportGetModule(Event import);
// Gets the external base name of the specified import.
_API const char *
FunctionImportGetBase(Function import);

_API const char *GlobalImportGetBase(Global import);

_API const char *EventImportGetBase(Event import);

//
// ========== Export Operations ==========
//

// Gets the external kind of the specified export.
_API ExternalKind
ExportGetKind(Export export_);
// Gets the external name of the specified export.
_API const char *ExportGetName(Export export_);
// Gets the internal name of the specified export.
_API const char *ExportGetValue(Export export_);
// Gets the number of export_section in the module.
_API uint32_t GetNumExports(Module module);
// Get export pointer from its index.
_API Export
GetExportByIndex(Module module, Index id);

//
// ========= Custom sections =========
//

_API void AddCustomSection(Module module,
                           const char *name,
                           const char *contents,
                           Index contentsSize);

//
// ========= Effect analyzer =========
//

typedef uint32_t SideEffects;

_API SideEffects SideEffectNone(void);

_API SideEffects SideEffectBranches(void);

_API SideEffects SideEffectCalls(void);

_API SideEffects SideEffectReadsLocal(void);

_API SideEffects SideEffectWritesLocal(void);

_API SideEffects SideEffectReadsGlobal(void);

_API SideEffects SideEffectWritesGlobal(void);

_API SideEffects SideEffectReadsMemory(void);

_API SideEffects SideEffectWritesMemory(void);

_API SideEffects SideEffectImplicitTrap(void);

_API SideEffects SideEffectIsAtomic(void);

_API SideEffects SideEffectThrows(void);

_API SideEffects SideEffectDanglingPop(void);

_API SideEffects SideEffectAny(void);

_API SideEffects ExpressionGetSideEffects(
		Expression expr, Features features);

//
// ========== CFG / Relooper ==========
//
// General usage is (1) create a relooper, (2) create blocks, (3) add
// branches between them, (4) render the output.
//
// For more details, see src/cfg/Relooper.h and
// https://github.com/WebAssembly//wiki/Compiling-to-WebAssembly-with-#cfg-api

#ifdef __cplusplus
namespace CFG {
struct Relooper;
struct Block;
} // namespace CFG
typedef struct CFG::Relooper* Relooper;
typedef struct CFG::Block* RelooperBlock;
#else
typedef struct Relooper *Relooper;
typedef struct RelooperBlock *RelooperBlock;
#endif

// Create a relooper instance
_API Relooper RelooperCreate(Module module);

// Create a basic block that ends with nothing, or with some simple branching
_API RelooperBlock RelooperAddBlock(Relooper relooper,
                                    Expression code);

// Create a branch to another basic block
// The branch can have code on it, that is executed as the branch happens. this
// is useful for phis. otherwise, code can be NULL
_API void RelooperAddBranch(RelooperBlock from,
                            RelooperBlock to,
                            Expression condition,
                            Expression code);

// Create a basic block that ends a switch on a condition
_API RelooperBlock
RelooperAddBlockWithSwitch(Relooper relooper,
                           Expression code,
                           Expression condition);

// Create a switch-style branch to another basic block. The block's switch table
// will have these indexes going to that target
_API void RelooperAddBranchForSwitch(RelooperBlock from,
                                     RelooperBlock to,
                                     Index *indexes,
                                     Index numIndexes,
                                     Expression code);

// Generate structed wasm control flow from the CFG of blocks and branches that
// were created on this relooper instance. This returns the rendered output, and
// also disposes of the relooper and its blocks and branches, as they are no
// longer needed.
// @param labelHelper To render irreducible control flow, we may need a helper
//        variable to guide us to the right target label. This value should be
//        an index of an i32 local variable that is free for us to use.
_API Expression RelooperRenderAndDispose(
		Relooper relooper, RelooperBlock entry, Index labelHelper);

//
// ========= ExpressionRunner ==========
//

#ifdef __cplusplus
namespace wasm {
class CExpressionRunner;
} // namespace wasm
typedef class wasm::CExpressionRunner* ExpressionRunner;
#else
typedef struct CExpressionRunner *ExpressionRunner;
#endif

typedef uint32_t ExpressionRunnerFlags;

// By default, just interpret the expression, i.e. all we want to know is whether
// it computes down to a concrete value, where it is not necessary to preserve
// side effects like those of a `local.tee`.
_API ExpressionRunnerFlags ExpressionRunnerFlagsDefault();

// Be very caul to preserve any side effects. For example, if we are
// intending to replace the expression with a constant afterwards, even if we
// can technically interpret down to a constant, we still cannot replace the
// expression if it also sets a local, which must be preserved in this scenario
// so subsequent code keeps functioning.
_API ExpressionRunnerFlags ExpressionRunnerFlagsPreserveSideeffects();

// Traverse through function calls, attempting to compute their concrete value.
// Must not be used in function-parallel scenarios, where the called function
// might be concurrently modified, leading to undefined behavior. Traversing
// another function reuses all of this runner's flags.
_API ExpressionRunnerFlags ExpressionRunnerFlagsTraverseCalls();

// Creates an ExpressionRunner instance
_API ExpressionRunner
ExpressionRunnerCreate(Module module,
                       ExpressionRunnerFlags flags,
                       Index maxDepth,
                       Index maxLoopIterations);

// Sets a known local value to use. Order matters if expression have side
// effects. For example, if the expression also sets a local, this side effect
// will also happen (not affected by any flags). Returns `true` if the
// expression actually evaluates to a constant.
_API int ExpressionRunnerSetLocalValue(ExpressionRunner runner,
                                       Index index,
                                       Expression value);

// Sets a known global value to use. Order matters if expression have side
// effects. For example, if the expression also sets a local, this side effect
// will also happen (not affected by any flags). Returns `true` if the
// expression actually evaluates to a constant.
_API int ExpressionRunnerSetGlobalValue(ExpressionRunner runner,
                                        const char *name,
                                        Expression value);

// Runs the expression and returns the constant value expression it evaluates
// to, if any. Otherwise returns `NULL`. Also disposes the runner.
_API Expression ExpressionRunnerRunAndDispose(
		ExpressionRunner runner, Expression expr);

//
// ========= Utilities =========
//

// Enable or disable coloring for the Wasm printer
_API void SetColorsEnabled(int enabled);

// Query whether color is enable for the Wasm printer
_API int AreColorsEnabled();

#ifdef __cplusplus
} // extern "C"
#endif

#endif // wasm__c_h
