#include "stdio.h"
#include "src/binary-reader.h"
#include <wast-lexer.h>
#include <error.h>
#include <shared-validator.h>
#include "src/interp/binary-reader-interp.h"
#include "src/interp/interp.h"

typedef unsigned char* bytes;
#define pointer std::unique_ptr

using namespace wabt::interp;

//using RefVec = std::vector<Ref>;

double root(double n){
	double lo = 0, hi = n, mid;
	for(int i = 0 ; i < 100 ; i++){
		mid = (lo+hi)/2;
		if(mid*mid == n) return mid;
		if(mid*mid > n) hi = mid;
		else lo = mid;
	}
	return mid;
}

static bool validate_wasm = true;
static wabt::ReadBinaryOptions s_write_binary_options;
static wabt::Features wabt_features;

wabt::Result do_square(Thread& thread, const Values& params, Values& results, Trap::Ptr* trap){
	int x = params.front().Get<int>();
	results.front() = wabt::interp::Value::Make(x * x);
	return wabt::Result::Ok;
};

wabt::Result do_logi(Thread& thread, const Values& params, Values& results, Trap::Ptr* trap){
	printf("%d\n",params.front().Get<int>());
};

wabt::Result do_logf(Thread& thread, const Values& params, Values& results, Trap::Ptr* trap){
	printf("%d\n",params.front().Get<float>());
};

wabt::Result do_sqrt(Thread& thread, const Values& params, Values& results, Trap::Ptr* trap){
	int x = params.front().Get<int>();
	results.front() = wabt::interp::Value::Make((int)root(x));
};



void BindImports(Module* module, std::vector<Ref> &imports, Store& store) {
//	auto* stream = s_stdout_stream.get();
	bool hostPrint = true;//false;
	// convoluted shit, I don't like it
	for (auto&& import : module->desc().imports) {
		auto func_type = *wabt::cast<FuncType>(import.type.type.get());
		if (import.type.name == "square")imports.push_back(HostFunc::New(store, func_type,do_square).ref());
		else if (import.type.name == "logi")imports.push_back(HostFunc::New(store, func_type,do_logi).ref());
		else if (import.type.name == "logf")imports.push_back(HostFunc::New(store, func_type,do_logf).ref());
		else if (import.type.name == "√")imports.push_back(HostFunc::New(store, func_type,do_sqrt).ref());
		else imports.push_back(Ref::Null);
		// By default, just push an null reference. This won't resolve, and instantiation will fail.
	}
}
// wabt has HORRIBLE api, but ok
int run_wasm(bytes buffer, int buf_size){
	Store store;
	ModuleDesc module_desc;
	const bool kReadDebugNames = true;
	const bool kStopOnFirstError = true;
	const bool kFailOnCustomSectionError = true;
	wabt::ReadBinaryOptions options(wabt_features, 0, kReadDebugNames,
	                                kStopOnFirstError, kFailOnCustomSectionError);
	wabt::Errors errors;
	CHECK_RESULT(ReadBinaryInterp(buffer, buf_size, options, &errors, &module_desc));
	auto module = wabt::interp::Module::New(store, module_desc);
	RefVec imports;
#if WASI
	uvwasi_t uvwasi;
    std::vector<const char*> argv; // ...
#endif
	BindImports(module.get(), imports, store);

	RefPtr<Trap> trap;
	Instance::Ptr instance = Instance::Instantiate(store, module.ref(), imports, &trap);
	if(trap){
		printf("\nERROR in module\n");
		printf("%s\n\n",trap.get()->message().data());
		return -1;
	}


	for (wabt::interp::ExportDesc export_ : module_desc.exports) {
		if (export_.type.type->kind != wabt::ExternalKind::Func) continue;
		if (export_.type.name != "main") continue;
		auto *func_type = wabt::cast<wabt::interp::FuncType>(export_.type.type.get());
		if (func_type->params.empty()) {
			RefVec funcs = instance->funcs();
			Ref &func0 = funcs[export_.index];
			auto func = store.UnsafeGet<wabt::interp::Func>(func0);
			Values params;
			Values results;
			Trap::Ptr trap;
			wabt::Result ok = func->Call(store, params, results, &trap, 0 );
			int result0 = results.front().Get<int>();
			return result0;
//			WriteCall(s_stdout_stream.get(), export_.type.name, *func_type, params, results, trap);
		}
	}
	return -1;
}
