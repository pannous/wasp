#include <execinfo.h> // for backtrace
#include <dlfcn.h>    // for dladdr needs compile flag -ldl
#include <cxxabi.h>   // for __cxa_demangle

#include <cstdio>
#include <cstdlib>
#include <string>
#include <sstream>

// This function produces a stack backtrace with demangled function & method names.
std::string Backtrace(int skip = 1, int skipEnd = 2) {
#ifndef WASM
	void *callstack[128];
	const int nMaxFrames = sizeof(callstack) / sizeof(callstack[0]);
	char buf[1024];
	int nFrames = backtrace(callstack, nMaxFrames);
	char **symbols = backtrace_symbols(callstack, nFrames);

	std::ostringstream trace_buf;
	for (int i = skip; i < nFrames - skipEnd; i++) {
		// skip __libc_start_main _start
//		printf("%s\n", symbols[i]);
		Dl_info info;
		if (dladdr(callstack[i], &info) && info.dli_sname) {
			char *demangled = NULL;
			int status = -1;
			if (i == skip)
				printf("%s\n", info.dli_fname);
			if (info.dli_sname[0] == '_')
				demangled = abi::__cxa_demangle(info.dli_sname, NULL, 0, &status);
			auto name = (status == 0) ? demangled : info.dli_sname == 0 ? symbols[i] : info.dli_sname;
			unsigned long offset = (char *) callstack[i] - (char *) info.dli_saddr;
			snprintf(buf, sizeof(buf), "%-3d %s + %zd\n",
			         i,
                     name,
                     offset);
			free(demangled);
		} else {
			snprintf(buf, sizeof(buf), "%-3d %*p %s\n",
			         i, int(2 + sizeof(void *) * 2), callstack[i], symbols[i]);
		}
		trace_buf << buf;
	}
	free(symbols);
	if (nFrames == nMaxFrames)
		trace_buf << "[truncated]\n";
	printf("%s\n", trace_buf.str().c_str());
	return trace_buf.str();
#endif
}