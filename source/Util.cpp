//
// Created by me on 07.10.21.
//
// #include <cmath>
#include <stdlib.h> // abs(int)
// #include "String.h"
#include "Util.h"

#ifndef WASM
// ok as wasi?
#include "unistd.h"
#include "NodeTypes.h"
#include "List.h"

#endif


//bool tracing = false;
//bool tracing = true;
//#define trace // IGNORE AT COMPILE TIME (to not emit all those debug strings)
//void trace0(chars x) {
//    if (tracing)
//        warn(x);
//}

//bool fileExists(char* filename) {
bool fileExists(String filename) {
#if RUNTIME_ONLY
    return false;
#else
    trace("checking fileExists "s + filename);
    if (filename.empty())return false;
    if (isDir(filename))return false;// only true files here!
#if WASM && !WASI
    return false; // todo
#else
    return access(filename.data, 0 /*F_OK*/) == 0;
#endif
#endif
}

int fileSize(char const *file) {
#ifndef WASM
	FILE *ptr;
	ptr = fopen(file, "rb");  // r for read, b for binary
	if (!ptr)
		error("File not found "s + file);
	fseek(ptr, 0L, SEEK_END);
	int sz = ftell(ptr);
	return sz;
#endif
	return -1;// todo
}

#if WASM && !WASI
#else

#include <sys/stat.h>
#include <time.h>

long file_last_modified(char *file) {
	struct stat attr;
	stat(file, &attr);
    return attr.st_mtime;
}

float file_last_modified(String &file) {
    struct stat attr;
    stat(file, &attr);
    return attr.st_mtime;
}

#endif

template<class>
class List;//<String>;

#include <stdio.h>
#include <sys/types.h>
#include <dirent.h>
#include <errno.h>

bool isDir(const char *name) {
#ifdef WASM
    return false;
#else
    DIR *directory = opendir(name);
    if (directory != NULL) {
        closedir(directory);
        return true;
    }
    if (errno == ENOTDIR)
        return false;
    return false;// -1;
#endif
}

//String findNewestFile(String filename) {
String findFile(String filename, String current_dir) {
    if (fileExists(filename))return filename;
    if (fileExists(current_dir + "/" + filename))return current_dir + "/" + filename;
#if RUNTIME_ONLY or WASM
    return "";
#else
    List<String> extensions = {"", ".wit", ".wasm", ".wast", ".wasp", ".witx"};
    List<String> folders = {"", "lib", "src", "wasp", "source", "include", "sample", "samples", "test", "tests"};
    List<String> paths;
    if (filename.contains("."))extensions = {""}; // don't do double extensions!
    if (isDir(filename)) {
        folders = {filename};
        filename = "index";// todo main / lib
    } else if (isDir(current_dir + "/" + filename)) {
        folders = {filename};
        filename = "index";// todo or main or lib or $filename
    }
    if (filename.contains("/"))
        for (auto extension: extensions) {
            String &path = filename + extension;
            if (fileExists(path)) paths.add(path);
        }
    else {
        for (auto folder: folders)
            for (auto extension: extensions) {
                String &path = current_dir + "/" + folder + "/" + filename + extension;
                if (fileExists(path)) paths.add(path);
			}
	}
	if (paths.empty())return "";
	paths.sort(&file_last_modified);
	String &best = paths.last();
	return best;
#endif
}

template<class S>
bool contains(List<S> list, S match) {
	return list.has(match);
}

template<class S>
// list HAS TO BE 0 terminated! Dangerous C!! ;)
bool contains(S list[], S match) {
	S *elem = list;
	do {
		if (match == *elem)
			return true;
	} while (*elem++);
	return false;
}

short normChar(char c) {// 0..36 damn ;)
	if (c == '\n')return 0;
	if (c >= '0' and c <= '9') return c - '0' + 26;
	if (c >= 'a' and c <= 'z') return c - 'a' + 1;// NOT 0!!!
	if (c >= 'A' and c <= 'Z') return c - 'A' + 1;// NOT 0!!!
	switch (c) {
		case '"':
		case '\'':
		case '!':
		case '(':
		case '#':
		case '$':
        case '+':
        case ' ':
        case '_':
        case '-':
            return 0;
        default:
            return c;// for asian etc!
    }
}

// useless
int ord(codepoint p) {
    return p;
}

codepoint Char(int p) {
    return p;
}

unsigned int wordHash(const char *str, int max_chars) { // unsigned
    if (!str) return 0;
    int maxNodes = 100000;
    char c;
    unsigned int hash = 5381, hash2 = 7; // long
    while (max_chars-- > 0 and (c = *str++)) {
        hash2 = hash2 * 31 + (short) (c);
        int next = normChar(c);//a_b-c==AbC
        if (next == 0)continue;
		hash = hash * 33 + next;// ((hash << 5) + hash
		hash = hash % maxNodes;
	}
	if (hash == 0)return hash2;
	return hash;
}

char *readFile(chars filename, int *size_out) {
	if (!filename)error("no filename given");
	if (!filename)return 0;
#ifndef WASM
	FILE *f = fopen(filename, "rt");
	if (!f)error("FILE NOT FOUND "_s + filename);
	fseek(f, 0, SEEK_END);
	long fsize = ftell(f);
	if (size_out)*size_out = fsize;
	fseek(f, 0, SEEK_SET);  /* same as rewind(f); */
	char *s = (char *) (alloc(fsize, 2));
	fread(s, 1, fsize, f);
	fclose(f);
	return s;
#else
	return 0;
#endif
}

// abs, floor from math.h should work with wasm, but isn't found in pure toolchain or sometimes breaks
// there is NO i32_abs in wasm, only f32_abs
// but compiler should make use of /opt/wasm/wasi-sdk/share/wasi-sysroot/include/c++/v1/math.h !!
//SET(CMAKE_SYSROOT /opt/wasm/wasi-sdk/share/wasi-sysroot/) # also for WASM include!
//#include <stdlib.h> // pulls in declaration of malloc, free
//#include <math.h> // pow
// c++ compiler emits abs as:
//local.get 0
//i32.const 31
//i32.shr_s
//local.tee 0
//i32.add
//local.get 0
//i32.xor
// inline int abs_i(int x) noexcept{
//	return x > 0 ? x : -x;
//}
//inline long abs_l(long x) noexcept{
//	return x > 0 ? x : -x;
//}
//// native to wasm
//inline float abs_f(float x) noexcept {
//	return x > 0 ? x : -x;
//}
//// native to wasm
//inline float floor(float x) noexcept {
//	x-(long)x;// todo!
//}
bool similar(double a, double b) {
	if (a == b)return true;
	if (a == 0)return abs(b) < .0001;
	if (b == 0)return abs(a) < .0001;
	double epsilon = abs(a + b) / 10000.;// percentual ++ todo add 10^order parameter
	bool ok = abs(a - b) <= epsilon;
	return ok;
}
//
//double pi = 3.141592653589793;
//double mod_d(double x, double y) {
//	return x - trunc(x / y) * y;
//}

void lowerCase(char *string, int length) {
	if (!string || !*string) return;
	if (length <= 0)
		length = strlen0(string);
	while (length-- > 0) {
		if (string[length] >= 'A' and string[length] <= 'Z')
			string[length] += 32;
	}
}

void lowerCase(chars string, int length); // defined in lowerCase.c

int equals_ignore_case(chars s1, chars s2, size_t ztCount) {
	bytes pStr1Low = 0;
	bytes pStr2Low = 0;
	bytes p1 = 0;
	bytes p2 = 0;
	if (s1 && *s1 && s2 && *s2) {
		pStr1Low = (bytes) calloc(strlen0(s1) + 1, sizeof(unsigned char));
		if (pStr1Low) {
			pStr2Low = (bytes) calloc(strlen0(s2) + 1, sizeof(unsigned char));
			if (pStr2Low) {
				p1 = pStr1Low;
				p2 = pStr2Low;
				strcpy2((char *) pStr1Low, s1);
				strcpy2((char *) pStr2Low, s2);
				lowerCase((char *) pStr1Low, 0);
				lowerCase((char *) pStr2Low, 0);
				for (; ztCount--; p1++, p2++) {
					int iDiff = *p1 - *p2;
					if (iDiff != 0 || !*p1 || !*p2) {
//						free(pStr1Low);
//						free(pStr2Low);
						return iDiff;
					}
				}
				free(pStr1Low);
				free(pStr2Low);
				return 0;
			}
			free(pStr1Low);
			return (-1);
		}
		return (-1);
	}
	return (-1);
}

bytes concat(bytes a, bytes b, int len_a, int len_b) {
    bytes c = new unsigned char[len_a + len_b + 4];// why+4 ?? else heap-buffer-overflow
    memcpy1(c, a, len_a);
    memcpy1(c + len_a, b, len_b);
    //	c[len_a + len_b + 1] = 0;// hwhy?
    return c;
}

chars concat(chars a, chars b) {
//const char *concat(const char *a, const char *b) {
	if (!b or b[0] == 0)return a;
	int la = (int) strlen0(a);
	int lb = (int) strlen0(b);
	char *c = (char *) malloc((la + lb + 1) * sizeof(char));
	strcpy2(c, a);
	strcpy2(&c[la], b);
	c[la + lb] = 0;
	return c;
}

bytes concat(bytes a, char b, int len) {
	bytes c = new unsigned char[len + 1];
    memcpy1(c, a, len);
    c[len] = b;
	return c;
}

bytes concat(char a, bytes b, int len) {
	bytes c = new unsigned char[len + 1];
    c[0] = a;
    memcpy1(c + 1, b, len);
	return c;
}

float ln(float y) {// crappy!
//	if(y==1)return 0;
	float divisor, x, result;
	int log2 = 0;
	unsigned int v = y;
	while (v >>= 1) {
		log2++;
	}
//	log2 = msb((int)y); // See: https://stackoverflow.com/a/4970859/6630230
	divisor = (float) (1 << log2);
	if (divisor == 0)return -1 / 0.000000000001;// todo;) noexcept
	x = y / divisor;    // normalized value between [1.0, 2.0]
	result = -1.7417939 + (2.8212026 + (-1.4699568 + (0.44717955 - 0.056570851 * x) * x) * x) * x;
	result += ((float) log2) * 0.69314718; // ln(2) = 0.69314718
	return result;
}

float log(float y, float base) {
	return ln(y) * ln(base);
}
//float log10(float y) noexcept{
//	return ln(y)*2.302585092994046; // ln(10)
//}
//
//
//float log2(float y) noexcept{
//	return ln(y)*0.69314718;
//}

String load(String file) {
#if WASM
	return "";
#else
	FILE *ptr;
	ptr = fopen(file, "rb");  // r for read, b for binary
	if (!ptr)error("File not found "s + file);
	fseek(ptr, 0L, SEEK_END);
	int size = ftell(ptr);
	unsigned char *buffer = (unsigned char *) malloc(size);
	fseek(ptr, 0L, SEEK_SET);
	int ok = fread(buffer, sizeof(buffer), size, ptr);
	if (!ok)error("Empty file or error reading "s + file);
	String *binary = new String((char *) buffer, size, false);
//	assert_equals(binary->length, size);
	return *binary;
#endif
}

String &hex(long d) {
#ifdef WASM
	return * new String(itoa0(d));
#else
//	char* s= (char*) malloc(1+64/4);// 0x ?
	int size = 3 + 64 / 4;
	char s[size];
	snprintf(s, size, "0x%lx", d);
	return *new String(s);// todo mark data as to-free
#endif
}

bool isSmartPointer(long long d) {
//	if((d&negative_mask_64)==negative_mask_64)return false;
    if ((d & negative_mask_64) == negative_mask_64)return false;
    if (d & double_mask_64)return true;
    return d & smart_mask_64 and not(d & negative_mask_64);
}

Node smartValue(long smartPointer);


#include <cxxabi.h>

String demangle(String &fun) {
    if (fun.length == 0)return "";
    int status;
    char *string = abi::__cxa_demangle(fun.data, 0, 0, &status);
    if (status < 0 or string == 0)
        return fun;// not demangled (e.g. "memory")
    String real_name = String(string); // temp
    String ok = real_name.substring(0, real_name.indexOf('('));
    return ok;// .clone(); unnecessary: return by value copy
}


String extractPath(String file) {
    if (!file.contains("/"))return "/";
    return file.substring(0, file.lastIndexOf("/"));
}
