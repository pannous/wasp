#include <stdio.h> // FILE

#ifndef WASM

#include <stdlib.h> // system

#endif

#include "String.h"

typedef unsigned char *bytes;

int exec(char *command) {
#ifndef WASI
	String result;
	FILE *fpipe;
	fpipe = (FILE *) popen(command, "r");
	char ch;
	if (fpipe) while ((ch = fgetc(fpipe)) != EOF)result += ch;
	pclose(fpipe);
//	result.split("\n");
	result = result.substring(result.indexOf(">>>") + 4);
	return parseLong(result);
#else
	print("no popen exec in Wasi since 6.0 https://github.com/WebAssembly/wasi-sdk/releases");
	return -1;
#endif
}

extern "C" int64 run_wasm_file(char *wasm_path = "test.wasm") {
    String command = "wasmx ";
    system(command + wasm_path);
    int64 ok = exec(command + wasm_path);
    return ok;
}

extern "C" int64 run_wasm(bytes data, int length) {
    char *file_name = "test.wasm";
    FILE *file = fopen(file_name, "w");
    fwrite(data, length, 1, file);
    fclose(file);
    return run_wasm_file(file_name);
}
