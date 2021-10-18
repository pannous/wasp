//
// Created by me on 07.10.21.
//

#include "String.h"
#include "Util.h"
#include "unistd.h"

//bool fileExists(char* filename) {

bool fileExists(String filename) {
	trace("checking fileExists "s + filename);
	if (filename.empty())return false;
	return access(filename.data, F_OK) == 0;
}

String findFile(String filename) {
	if (filename.empty())return "";
//	filename = filename.replace("~", getpwuid(getuid())->pw_dir /*homedir*/);
	if (fileExists(filename))return filename;
	// todo: check wasm date, recompile if older than wasp / wast
	if (fileExists(filename + ".wasp"))return filename + ".wasp"s;
	if (fileExists(filename + ".wast"s))return filename + ".wast"s;
	if (fileExists(filename + ".wasm"))return filename + ".wasm"s;
	//		if (not filename.contains("/"))filename = findFile(project + "/" + filename) || filename;
	if (not filename.contains("/"))filename = findFile("lib/"s + filename) || filename;
	if (not filename.contains("/"))filename = findFile("src/"s + filename) || filename;
	if (not filename.contains("/"))filename = findFile("source/"s + filename) || filename;
	if (not filename.contains("/"))filename = findFile("include/"s + filename) || filename;
	if (not filename.contains("/"))filename = findFile("samples/"s + filename) || filename;
	return fileExists(filename) ? filename : "";
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


char *readFile(chars filename, int* size_out) {
	if (!filename)error("no filename given");
	if (!filename)return 0;
#ifndef WASM
	FILE *f = fopen(filename, "rt");
	if (!f)error("FILE NOT FOUND "_s + filename);
	fseek(f, 0, SEEK_END);
	long fsize = ftell(f);
	if(size_out)*size_out = fsize;
	fseek(f, 0, SEEK_SET);  /* same as rewind(f); */
	char *s = (char *) (alloc(fsize, 2));
	fread(s, 1, fsize, f);
	fclose(f);
	return s;
#else
	return 0;
#endif
}


// there is NO i32_abs in wasm, only f32_abs
int abs_i(int x) {
	return x > 0 ? x : -x;
}

// native to wasm
inline float abs_f(float x)

noexcept {
return x > 0 ? x : -
x;
}

bool similar(float a, float b) {
	if (a == b)return true;
	float epsilon = abs_f(a + b) / 1000000.;// percentual ++
	bool ok = a == b or abs_f(a - b) <= epsilon;
	return ok;
}


double pi = 3.141592653589793;

double mod_d(double x, double y) {
	return x - trunc(x / y) * y;
}

double sin(double x) {
	double tau = 6.283185307179586;// 2*pi
	// double pi_fourth=0.7853981633974483;
	double x2, r, x4;
	double
			S1 = -1.66666666666666324348e-01, /* 0xBFC55555, 0x55555549 */
	S2 = 8.33333333332248946124e-03, /* 0x3F811111, 0x1110F8A6 */
	S3 = -1.98412698298579493134e-04, /* 0xBF2A01A0, 0x19C161D5 */
	S4 = 2.75573137070700676789e-06, /* 0x3EC71DE3, 0x57B1FE7D */
	S5 = -2.50507602534068634195e-08, /* 0xBE5AE5E6, 0x8A2B9CEB */
	S6 = 1.58969099521155010221e-10; /* 0x3DE5D93A, 0x5ACFD57C */
	x = mod_d(x, tau);
//	 if(x<0) return -sin(-x);
	if (x >= pi) return -sin(mod_d(x, pi));

	// if(x%tau > pi) return -sin(x%tau);
	x2 = x * x;
	x4 = x2 * x2;
	r = S2 + x2 * (S3 + x2 * S4) + x2 * x4 * (S5 + x2 * S6);
	return x + x2 * x * (S1 + x2 * r);
}

double cos(double x) {
	double pi_half = 1.5707963267948966;
	return sin(x + pi / 2);// todo: eval compiler!
}
