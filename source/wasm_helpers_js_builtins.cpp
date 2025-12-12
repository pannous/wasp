//#pragma once
#include <cstdlib> // OK in WASM!

#include "wasm_helpers.h"
#include "String.h"

// "wasm:text-encoder" "encodeStringToUTF8Array"
extern "C" bytes encodeStringToUTF8Array(ExternRef str);
// "wasm:text-encoder" "encodeStringIntoUTF8Array"
extern "C" void encodeStringIntoUTF8Array(ExternRef str, /*mut*/ bytes buffer, uint start, uint end);

// "wasm:text-decoder" "decodeStringFromUTF8Array"
extern "C" ExternRef decodeStringFromUTF8Array(bytes str, uint start, uint end);

// "wasm:text-encoder" "measureStringAsUTF8"
extern "C" uint measureStringAsUTF8(ExternRef str); // efficient strlen in bytes

//(; bound to memory 0 through the JS-API instantiating the builtins ;)
//(import "wasm:js-string" "encodeStringToMemoryUTF16" (func ...))