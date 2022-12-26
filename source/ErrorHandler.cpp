//
// Created by pannous on 17.11.19.
//

#define ErrorHandler

#include "ErrorHandler.h"

//#include <zconf.h>
//#include <execinfo.h>
//#include <bits/types/siginfo_t.h>
//#include <csignal>
//#include <cstdio>


#ifndef _GNU_SOURCE
//#define _GNU_SOURCE
#endif

/* Bug in gcc prevents from using CPP_DEMANGLE in pure "C" */
#if !defined(__cplusplus) && !defined(NO_CPP_DEMANGLE)
//#define NO_CPP_DEMANGLE
#endif

#ifndef WASM
//#warning NOT WASM


#include <memory.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <signal.h>
//The deprecated ucontext routines require _XOPEN_SOURCE to be defined
#define _XOPEN_SOURCE

#include <ucontext.h>
#include <dlfcn.h>

#ifndef NO_CPP_DEMANGLE
#include <cxxabi.h>
#include <unwind.h>
#ifdef __cplusplus
using __cxxabiv1::__cxa_demangle;
#endif
#endif

#ifdef HAS_ULSLIB
#include "uls/logger.h"
#define sigsegv_outp(x)         sigsegv_outp(,gx)
#else
#define sigsegv_outp(x, ...)    fprintf(stderr, x "\n", ##__VA_ARGS__)
#endif

#if defined(REG_RIP)
# define SIGSEGV_STACK_IA64
# define REGFORMAT "%016lx"
#elif defined(REG_EIP)
# define SIGSEGV_STACK_X86
# define REGFORMAT "%08x"
#else
# define SIGSEGV_STACK_GENERIC
# define REGFORMAT "%x"
#endif
bool showRegisters = false;

#define MAC
typedef char const *chars;

// NICE, BREAKPOINT WORKS!!!
static void signal_segv(int signum, siginfo_t *info, void *ptr) {
	static chars si_codes[4] = {"", "SEGV_MAPERR", "SEGV_ACCERR", "SIGABRT"};
	int i, f = 0;
	ucontext_t *ucontext = (ucontext_t *) ptr;
	Dl_info dlinfo;
	void **bp = 0;
	void *ip = 0;
#ifdef HAS_ULSLIB
	//#ifndef MAC
		sigsegv_outp("Segmentation Fault!");
		sigsegv_outp("info.si_signo = %d", signum);
		sigsegv_outp("info.si_errno = %d", info->si_errno);
		sigsegv_outp("info.si_code  = %d (%s)", info->si_code, si_codes[info->si_code]);
		sigsegv_outp("info.si_addr  = %p", info->si_addr);
		if(showRegisters)
		for(i = 0; i < NGREG; i++)
			sigsegv_outp("reg[%02d]       = 0x" REGFORMAT, i, ucontext->uc_mcontext.gregs[i]);

#ifndef SIGSEGV_NOSTACK
#if defined(SIGSEGV_STACK_IA64) || defined(SIGSEGV_STACK_X86)
#if defined(SIGSEGV_STACK_IA64)
		ip = (void*)ucontext->uc_mcontext.gregs[REG_RIP];
		bp = (void**)ucontext->uc_mcontext.gregs[REG_RBP];
#elif defined(SIGSEGV_STACK_X86)
		ip = (void*)ucontext->uc_mcontext.gregs[REG_EIP];
		bp = (void**)ucontext->uc_mcontext.gregs[REG_EBP];
#endif

		sigsegv_outp("Stack trace:");
		while(bp && ip) {
			if(!dladdr(ip, &dlinfo))
				break;
			chars symname = dlinfo.dli_sname;

#ifndef NO_CPP_DEMANGLE
			int status;
			char * tmp = __cxa_demangle(symname, NULL, 0, &status);
			if (status == 0 && tmp)
				symname = tmp;
#endif

	//		sigsegv_outp("% 2d: %p %s + %lu (%s)", // with pointer
						 sigsegv_outp("% 2d: %s + %lu (%s)",
						 ++f,
	//		             ip,
						 symname,
						 (unsigned number)ip - (unsigned number)dlinfo.dli_saddr,
						 dlinfo.dli_fname);


			if(dlinfo.dli_sname && !strcmp(dlinfo.dli_sname, "wasp_main"))
				break;

			ip = bp[1];
			bp = (void**)bp[0];
		}
#else
		sigsegv_outp("Stack trace (non-dedicated):");
		sz = backtrace(bt, 20);
		strings = backtrace_symbols(bt, sz);
		for(i = 0; i < sz; ++i)
			sigsegv_outp("%s", strings[i]);
#endif
		sigsegv_outp("End of stack trace.");
		sigsegv_outp("Debug to catch Segmentation Fault on the spot, set breakpoint on signal_segv()");

#else
		sigsegv_outp("Not printing stack strace.");
#endif
#endif
//	_exit(-1);
}
// Alternative: turn SIGSEGV into c++ exception https://stackoverflow.com/questions/2350489/how-to-catch-segmentation-fault-in-linux
// Alternative: check if pointer is 'good' in advance … use rarely!!

//istead of _Unwind_Backtrace(tracer, &state);

static void handler(int sig, siginfo_t *si, void *context) {
	printf("Got SIGSEGV at address: 0x%lx\n", (int64) si->si_addr);
    printf("Implements the handler only\n");
//	Backtrace();
}


static void __attribute__((constructor)) setup_sigsegv() {
	struct sigaction action;
	memset(&action, 0, sizeof(action));
	action.sa_sigaction = signal_segv;
	action.sa_flags = SA_SIGINFO;
	if (sigaction(SIGSEGV, &action, NULL) < 0)
		perror("sigaction");
}


void register_global_signal_exception_handler() {
	setup_sigsegv();
}

#endif
