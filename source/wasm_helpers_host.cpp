//#pragma once
//
// Created by pannous on 15.07.20.
//

#include <cstdlib> // OK in WASM!

#include "wasm_helpers.h"
#include "String.h"
#include "Backtrace.h"
#include "Code.h"
#include "Util.h"
//extern unsigned int *memory;

#include <math.h> // links to math.so todo: can it be inlined in wasm? otherwise needs extern "C" double pow

extern "C" unsigned int *memory = 0;
void *wasm_memory = 0;// c pointer of VM, NOT memory inside wasm module

// these wrappers are helpful because different hosts have different signatures, so wasm_helpers.h would need many #if's
void memcpy1(bytes dest, bytes source, int i) {
    memcpy(dest, source, i);
}


// NOT part of c! nice bifork
void *alloc(int num, int size) {
    return calloc(num, size);
//    return malloc(num * size);// good to find bugs
}

void panic() {
#ifndef WASM
    raise("panic");
#else
    char* x=0;
    x[-1]=2;// Bus error: 10
//    throw;
#endif
}

#ifdef RUNTIME_ONLY // No Angle.cpp!
#endif


//#ifndef WASM
//void printf(chars format, number i) {
//	print(String(format).replace("%d", String(i)).replace("%i", String(i)).replace("%li", String(i)));
//}
//#endif



// replaces own puts()
// poor man's WASI, only needed INTERNALLY if compiled without modern wasm runtime, link in diverse wasm runners as
// {"fd_write", (void *) fd_write_host, "(iiii)i", NULL, false}, …
void fd_write_host(int FD, char **strp, int *len, int *nwritten) {
//#if
    printf("%s", *strp);
    error("fd_write_host should ONLY be called via wasm runtime without wasi");
}
//__wasi_fd_write
/*
 * fd_write: wrap((fd, iovs, iovsLen, nwritten) => {
        const stats = CHECK_FD(fd, WASI_RIGHT_FD_WRITE);
        let written = 0;
        getiovs(iovs, iovsLen)
          .forEach((iov) => {
            let w = 0;
            while (w < iov.byteLength) {
              w += fs.writeSync(stats.real, iov, w, iov.byteLength - w);
            }
            written += w;
          });
 * */


float putf(float f) {
    printf("%f\n", f);
    return f;
}

void *putp(void *f) {
    printf("%p\n", f);
    return f;
}


int puti(int i) {
    printf("%d", i);
    return i;
}

int64 putl(int64 l) {
    printf("%lld", l);
    return l;
}

[[maybe_unused]] int64 putx(int64 l) {
    printf("%llx", l);
    return l;
}

void putp(int64 char_pointer) {// workaround for m3, which can't link pointers:  od.link_optional<puts>("*", "puts")
    printf("%llx", char_pointer);
}

codepoint put_char(codepoint c) {
    printf("%c", c);
    return c;
}

double powd(double x, double y) {
    return pow(x, y);
}

extern "C" void put_chars(chars c, size_t len) {
    printf("%s", c);
}

#if not WASM
[[noreturn]]
void proc_exit(int x) {
    exit(x);
}

#endif
