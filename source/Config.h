#include "Util.h"

#if WASM
__attribute__((section(".metadata")))
const char metadata_payload[] = "name=Wasp-Runtime;author=Pannous;license=MIT;";
// wasm-opt input.wasm -o output.wasm --custom-section=metadata='{"author":"Pannous","license":"MIT"}'
#endif

static chars wasp_version = "0.1.912";

static bool multi_return_values = false;

#ifdef RUNTIME_ONLY
static bool debug = false;
#else
static bool debug = true;// clone sub-strings instead of sharing etc
#endif
//static bool verbose = false;
//static bool quiet = false;
//static bool trace = false;

// todo: wrap parser-options for serialize
static bool use_polish_notation = false;// f(a,b) => (f a b) also : lisp mode (a 1 2)==a(1)(2)==a{1 2}


