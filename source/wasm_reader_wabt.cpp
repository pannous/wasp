//
// Created by me on 27.11.20.
//

//#include <wasm3.h>
//#include <m3_env.h>
#include <wast-lexer.h>
#include <error.h>
#include <shared-validator.h>
#include <vector>
#include "wasm_reader_wabt.h"
#include "wasm_emitter.h"
#include "binary-reader.h"
#include "binary-reader-ir.h"
#include "binary-writer.h"
#include "common.h"
#include "error-formatter.h"
#include "feature.h"
#include "ir.h"
#include "result.h"
#include "option-parser.h"
#include "stream.h"
#include "validator.h"
#include "wast-parser.h"


typedef unsigned char *bytes;
#define pointer std::unique_ptr


static wabt::Features wabt_features;
static bool validate_wasm = true;
static bool dump_module = false;//debug


#define consume(len, bytes) if(!consume_x(code,&pos,len,bytes)){printf("\nNOT consuming %s:%d\n",__FILE__,__LINE__);exit(0);}

#define check(test) if(test){log("OK check passes: ");printf("%s",(#test));}else{printf("\nNOT PASSING %s\n%s:%d\n",#test,__FILE__,__LINE__);exit(0);}

#define pointerr std::unique_ptr

bool consume_x(byte *code, int *pos, int len, byte *bytes) {
//	if(bytes)
	int i = 0;
	while (i < len) {
		if (bytes and code[*pos] != bytes[i])
			return false;
		*pos = *pos + 1;
		i++;
	}
	return true;
}

int pos = 0;
byte *code;

byte typ() {
	return code[pos++];
}

int unsignedLEB128() {
	int n = 0;
	do {
		byte b = code[pos++];
		n = n << 7;
		n = n ^ (b & 0x7f);
		if (b & 0x80 == 0)break;
	} while (n != 0);
	return n;
}

int unsignedLEB128(Code code) {
	int n = 0;
	do {
		byte b = code[code.start++];
		n = n << 7;
		n = n ^ (b & 0x7f);
		if ((b & 0x80) == 0)break;
	} while (n != 0);
	return n;
}

int siz() {
	return unsignedLEB128();
}

Code vec() {
	int from = pos;
	int len = siz();
	consume(len, 0);
	return Code(code, from, pos);
}

Code consumeTypeSection() {
	int from = pos;
	byte type = typ();
	Code type_vector = vec();
	int typeCount = unsignedLEB128(type_vector);
	printf("typeCount %d\n", typeCount);
	Code type_data = type_vector.rest();
	return Code(code, from, pos);
//	return Code(type, encodeVector(Code(typeCount) + type_data));
}

Code read(byte *code0, int length) {
	pos = 0;
	int from = pos;
	code = code0;
	consume(4, reinterpret_cast<byte *>(magicModuleHeader));
	consume(4, reinterpret_cast<byte *>(moduleVersion));
	consumeTypeSection();
//	consumeImportSection();
	return Code(code, from, pos);
}

int fileSize(char const *file) {
	FILE *ptr;
	ptr = fopen(file, "rb");  // r for read, b for binary
	if (!ptr)error("File not found "s + file);
	fseek(ptr, 0L, SEEK_END);
	int sz = ftell(ptr);
	return sz;
}


using namespace wabt;


static pointerr<FileStream> s_log_stream;// = FileStream::CreateStdout();


static void DebugBuffer(const OutputBuffer &buffer) {
	pointerr<FileStream> stream = FileStream::CreateStdout();
	if (!buffer.data.empty()) {
		stream->WriteMemoryDump(buffer.data.data(), buffer.data.size());
	}
}


static void ParseOptions(int argc, char **argv); // wasm-link.cc

//void remove(std::vector str_vec,chars element){// WTF C++
//	str_vec.erase(std::remove(str_vec.begin(), str_vec.end(), element), str_vec.end());
//}

void remove_import(Module *module, chars fun) {
	std::vector<Import *> &imports = module->imports;
	for (wabt::Import *import : imports) {
		if (import->field_name == fun)
			imports.erase(std::remove(imports.begin(), imports.end(), import));// wth cpp!
	}
}

void remove_function(Module *module, chars fun) {
	std::vector<Func *> &funcs = module->funcs;
	for (wabt::Func *func : funcs) {
		if (func->name == fun) {
			funcs.erase(std::remove(funcs.begin(), funcs.end(), func));
		}// wth cpp!
	}
}


void write_module(Module *module, chars file = 0) {
//	string_view filename = module->loc.filename;
	std::string filename = module->loc.filename.to_string();
	if (fileempty(name)) filename = "out.wasm";
	if (file)filename = file;
//	FileStream *stream = new FileStream(filename);
	MemoryStream *stream = new MemoryStream();// MoveDataImpl not implemented for FileStream
	Features features;
	features.EnableAll();
	WriteBinaryOptions options;;
	options.write_debug_names = true;
	options.features = features;
	options.canonicalize_lebs = true;
	options.relocatable = true;
	WriteBinaryModule(stream, module, options);
	stream->WriteToFile(filename);
}

void save_wasm(Module *module, chars file) {
	write_module(module, file);
}

Module *refactor_wasm(Module *module, chars old_name, chars new_name) {
	if (eq(old_name, new_name)) {
		warn("refactor_wasm: old and new name are the same");
		return 0;
	}
	for (wabt::Func *f : module->funcs)
		if (f->name == new_name)
			error("new name already taken in wasm module");
	for (wabt::Func *f : module->funcs)
		if (f->name == old_name)
			f->name = new_name;
//	const string_view out = "refactored_" + module->loc.filename;
//	printf("%s",out.to_string().data());
	module->loc.filename = "out.wasm";
	write_module(module);
	return module;
}


Module *read_wasm(bytes data, int size, const char *filename = "<binary>") {
	Features features;
	features.EnableAll();
	ReadBinaryOptions options;
	options.features = features;
	options.read_debug_names = 1;
	Errors errors;
	Module *module = new Module();
	wabt::ReadBinaryIr(filename, data, size, options, &errors, module);
	for (wabt::Func *f : module->funcs) {
		printf("%s\n", f->name.data());
	}
	return module;
}

int readWat(const char *infile) {
	string_view s_infile = "t.wat";
	std::vector<uint8_t> file_data;
	Result result = ReadFile(s_infile, &file_data);
	pointerr<WastLexer> lexer = WastLexer::CreateBufferLexer(s_infile, file_data.data(), file_data.size());
	if (Failed(result)) WABT_FATAL("unable to read file: %s\n", s_infile);

	Errors errors;
	pointerr<wabt::Module> module;

	WastParseOptions parse_wast_options(wabt_features);
	result = ParseWatModule(lexer.get(), &module, &errors, &parse_wast_options);

	if (Succeeded(result) && validate_wasm) {
		ValidateOptions options(wabt_features);
		result = ValidateModule(module.get(), &errors, options);
	}
	if (Succeeded(result)) {
		MemoryStream stream;
		wabt::WriteBinaryOptions write_binary_options;
		write_binary_options.features = wabt_features;
		result = WriteBinaryModule(&stream, module.get(), write_binary_options);
		if (Succeeded(result)) {
			OutputBuffer &buffer = stream.output_buffer();
			buffer.WriteToFile(s_infile.substr(0, s_infile.find(".wat")));
			if (dump_module)DebugBuffer(buffer);
		}
	}
	auto line_finder = lexer->MakeLineFinder();
	FormatErrorsToFile(errors, Location::Type::Text, line_finder.get());
}


Module *readWasm(char const *file) {
//	readWat(file);
	printf("parsing: %s\n", file);
	int size = fileSize(file);
	if (size <= 0)error("File not found "s + file);
	unsigned char buffer[size];
	fread(buffer, sizeof(buffer), size, fopen(file, "rb"));
	Module *module = read_wasm(buffer, size, file);
	check(module->loc.filename == file);
	return module;
}

#undef pointerr