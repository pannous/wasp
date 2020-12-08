#include <stdio.h> // FILE
#include <stdlib.h> // system
#include "String.h"
typedef unsigned char* bytes;

int exec(char *command) {
	String result;
	FILE *fpipe;
	fpipe = (FILE *)popen(command, "r");
	char ch;
	if (fpipe) while ((ch = fgetc(fpipe)) != EOF)result+=ch;
	pclose(fpipe);
//	result.split("\n");
	result=result.substring(result.indexOf(">>>")+4);
	return atoi(result);
}

int run_wasm(char* wasm_path = "test.wasm"){
	char *command = "wasmx test.wasm";
	system(command);
	int ok=exec(command);
	return ok;
}

int run_wasm(bytes data, int length){
	char* file_name = "test.wasm";
	FILE* file=fopen(file_name,"w");
	fwrite(data, length, 1, file);
	fclose(file);
	return run_wasm(file_name);
}