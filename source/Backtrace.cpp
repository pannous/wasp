#include <execinfo.h> // for backtrace
#include <dlfcn.h>    // for dladdr needs compile flag -ldl
#include <cxxabi.h>   // for __cxa_demangle

#define _Backtrace_ 1

int addr2line(const char *fname, void *pVoid);

#include <cstdio>
#include <cstdlib>
#include <string>
#include <sstream>
#include "String.h"

//#include <boost/stacktrace.hpp>

// ... somewhere inside the bar(int) function that is called recursively:

String Backtrace(int skip = 0, int skipEnd = 1) {
//	return boost::stacktrace::stacktrace();

// This function produces a stack backtrace with demangled function & method names.
	void *callstack[128];
	const int nMaxFrames = sizeof(callstack) / sizeof(callstack[0]);
	char buf[1024];
	int nFrames = backtrace(callstack, nMaxFrames);
	char **symbols = backtrace_symbols(callstack, nFrames);

	std::ostringstream trace_buf;
//	trace_buf << "Backtrace:\n";
	for (int i = skip; i < nFrames - skipEnd; i++) {
		// skip __libc_start_main _start
//		printf("%s\n", symbols[i]);
		Dl_info info;
		chars name;
		if (dladdr(callstack[i], &info) && info.dli_sname) {
            char *demangled = NULL;
            int status = -1;
            if (i == skip)
                printf("%s\n", info.dli_fname);
#if not WASM
            if (info.dli_sname[0] == '_')
                demangled = abi::__cxa_demangle(info.dli_sname, NULL, 0, &status);
#endif
            if (contains(demangled, "decltype"))break;
            name = (status == 0) ? demangled : info.dli_sname == 0 ? symbols[i] : info.dli_sname;
            uint64 offset = (char *) callstack[i] - (char *) info.dli_saddr;
//			int line_nr = addr2line(info.dli_fname, info.dli_saddr);
            snprintf(buf, sizeof(buf), "%-3d %s + %llu @ %p \n", i, name, offset,
                     info.dli_saddr);// or dli_fbase for function!
//			snprintf(buf, sizeof(buf), "%s:%d \n", info.dli_fname, line_nr); wasp:42  eXECutable makes no SenSe!
        } else {
            snprintf(buf, sizeof(buf), "%-3d %*p %s\n", i, int(2 + sizeof(void *) * 2), callstack[i], symbols[i]);
        }
        trace_buf << buf;
        if (eq(name, "testCurrent()"))
            break;
    }
    if (nFrames == nMaxFrames)
        trace_buf << "[truncated]\n";
    auto string = trace_buf.str();
    const char *cStr = string.c_str();
    printf("%s\n", cStr);
//	printf("/me/dev/script/wasm/wasp/tests.cpp:196:10 << TODO: correct line use assert_is()\n");
    return String(trace_buf.str().data());
}



// with line numbers only on linux /
/*
#include <sys/prctl.h>
void print_trace() {
	char pid_buf[30];
	sprintf(pid_buf, "%d", getpid());
	char name_buf[512];
	name_buf[readlink("/proc/self/exe", name_buf, 511)]=0;
	auto PR_SET_PTRACER=1;
	int PR_SET_PTRACER_ANY=1;
	prctl(PR_SET_PTRACER, PR_SET_PTRACER_ANY, 0, 0, 0);
	int child_pid = fork();
	if (!child_pid) {
		dup2(2,1); // redirect output to stderr - edit: unnecessary?
		execl("/usr/bin/gdb", "gdb", "--batch", "-n", "-ex", "thread", "-ex", "bt", name_buf, pid_buf, NULL);
		abort(); */
/* If gdb failed to start *//*

	} else {
		waitpid(child_pid,NULL,0);
	}
}
*/
