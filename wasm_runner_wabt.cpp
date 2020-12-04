//#include "src/binary-reader.h"
#include <wast-lexer.h>
#include <error.h>
#include <shared-validator.h>
#include "src/interp/binary-reader-interp.h"
#include "src/interp/interp.h"

typedef unsigned char* bytes;

//using namespace wabt::interp;
using wabt::interp::Store;
using wabt::interp::ModuleDesc;
using wabt::interp::Module;
using wabt::interp::Ref;
using wabt::interp::RefVec;
using wabt::interp::RefPtr;
using wabt::interp::FuncType;
using wabt::interp::HostFunc;
using wabt::interp::Thread;
using wabt::interp::Value;
using wabt::interp::Values;
using wabt::interp::Trap;
using wabt::interp::Instance;

//using RefVec = std::vector<Ref>;

static bool validate_wasm = true;
static wabt::ReadBinaryOptions s_write_binary_options;
static wabt::Features wabt_features;

void BindImports(Module::Ptr& module, std::vector<Ref> &imports, Store store) {
//	auto* stream = s_stdout_stream.get();
	bool hostPrint = true;//false;



	for (auto&& import : module->desc().imports) {
		auto func_type = *wabt::cast<FuncType>(import.type.type.get());
		// convoluted shit, I don't like it
		auto host_square = HostFunc::New(store, func_type,
		                                 [=](Thread& thread, const Values& params,
		                                     Values& results, Trap::Ptr* trap) -> wabt::Result {
			                                 results.push_back(wabt::interp::Value::Make(42));
			                                 return wabt::Result::Ok;
		                                 });
		if (import.type.name == "square")
			imports.push_back(host_square.ref());
		else imports.push_back(Ref::Null);
		// By default, just push an null reference. This won't resolve, and instantiation will fail.
	}
}

// wabt has HORRIBLE api, but ok
int run_wasm(bytes buffer, int buf_size){
//	Module ModuleRead(buffer, buf_size); // binaryen
//	ReadBinaryInterp();
	Store store;

	ModuleDesc module_desc;
	const bool kReadDebugNames = true;
	const bool kStopOnFirstError = true;
	const bool kFailOnCustomSectionError = true;
	wabt::ReadBinaryOptions options(wabt_features, 0, kReadDebugNames,
	                                kStopOnFirstError, kFailOnCustomSectionError);
	wabt::Errors errors;
	CHECK_RESULT(ReadBinaryInterp(buffer, buf_size, options, &errors, &module_desc));
//	BindImports(const Module::Ptr& module, RefVec& imports);
//	wabt::interp::Module module=wabt::interp::Module(store, module_desc);
	wabt::interp::Module::Ptr module = wabt::interp::Module::New(store, module_desc);
	RefVec imports;
#if WASI
	uvwasi_t uvwasi;
    std::vector<const char*> argv; // ...
#endif
	BindImports(module, imports, store);

	Instance::Ptr instance;
	RefPtr<Trap> trap;
	Instance::Instantiate(store, module.ref(), imports, &trap);

	for (wabt::interp::ExportDesc export_ : module_desc.exports) {
		if (export_.type.type->kind != wabt::ExternalKind::Func) continue;
		if (export_.type.name != "main") continue;
		auto *func_type = wabt::cast<wabt::interp::FuncType>(export_.type.type.get());
		if (func_type->params.empty()) {
			auto func = store.UnsafeGet<wabt::interp::Func>(instance->funcs()[export_.index]);
			Values params;
			Values results;
			Trap::Ptr trap;
			wabt::Result ok = func->Call(store, params, results, &trap, 0  /*stream*/);
			int result0 = results.front().Get<int>();
			return result0;
//			WriteCall(s_stdout_stream.get(), export_.type.name, *func_type, params, results, trap);
		}
	}
	return -1;
}
