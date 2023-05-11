#include "Util.h"

static chars version = "0.1.0";

static bool multi_value = false; // todo gather in one place: config.h

static bool throwing = true;// otherwise fallover beautiful-soup style generous parsing
static bool panicking = false;// false for error tests, webview, etc
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


