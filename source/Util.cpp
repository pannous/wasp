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


char *readFile(chars filename) {
	if (!filename)error("no filename given");
	if (!filename)return 0;
#ifndef WASM
	FILE *f = fopen(filename, "rt");
	if (!f)error("FILE NOT FOUND "_s + filename);
	fseek(f, 0, SEEK_END);
	long fsize = ftell(f);
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
