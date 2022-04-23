//
// Created by me on 07.10.21.
//

#include <cmath>
#include "String.h"
#include "Util.h"

#ifndef WASM
// ok as wasi?
#include "unistd.h"

#endif

//bool fileExists(char* filename) {

bool fileExists(String filename) {
#if RUNTIME_ONLY
	return false;
#else
	trace("checking fileExists "s + filename);
	if (filename.empty())return false;
	return access(filename.data, F_OK) == 0;
#endif
}


int fileSize(char const *file) {
#ifndef WASM
	FILE *ptr;
	ptr = fopen(file, "rb");  // r for read, b for binary
	if (!ptr)error("File not found "s + file);
	fseek(ptr, 0L, SEEK_END);
	int sz = ftell(ptr);
	return sz;
#endif
	return -1;// todo
}


String findFile(String filename) {
#if RUNTIME_ONLY
	return "";
#endif
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
	float epsilon = abs_f(a + b) / 10000.;// percentual ++
	if (a == 0 or b == 0)epsilon = 1 / 1000.;
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

void lowerCase(char *string, int length) {
	if (!string || !*string) return;
	if (length <= 0)
		length = strlen0(string);
	while (length-- > 0) {
		if (string[length] >= 'A' and string[length] <= 'Z')
			string[length] += 32;
	}
}

// todo: code licence?
// IN PLACE replacement!
// 11592 bytes lowerCase.wasm, UTF8 is HEAVY!
/*void lowerCase(chars string, int length) {
	if (!string || !*string) return;
	unsigned char *aChar = 0;
	unsigned char *p = (unsigned char *) string;
	if (length <= 0)length = strlen0(string);
	while (*p and length-- > 0) {
		if ((*p >= 0x41) && (*p <= 0x5a)) *//* US ASCII *//*
			(*p) += 0x20;
		else if (*p > 0xc0) {
			aChar = p;
			p++;
			switch (*aChar) {
				case 0xc3: *//* Latin 1 *//*
					if ((*p >= 0x80)
					    && (*p <= 0x9e)
					    && (*p != 0x97))
						(*p) += 0x20; *//* US ASCII shift *//*
					break;
				case 0xc4: *//* Latin ext *//*
					if (((*p >= 0x80)
					     && (*p <= 0xb7)
					     && (*p != 0xb0))
					    && (!(*p % 2))) *//* Even *//*
						(*p)++; *//* Next char is lwr *//*
					else if ((*p >= 0xb9)
					         && (*p <= 0xbe)
					         && (*p % 2)) *//* Odd *//*
						(*p)++; *//* Next char is lwr *//*
					else if (*p == 0xbf) {
						*aChar = 0xc5;
						(*p) = 0x80;
					}
					break;
				case 0xc5: *//* Latin ext *//*
					if ((*p >= 0x81)
					    && (*p <= 0x88)
					    && (*p % 2)) *//* Odd *//*
						(*p)++; *//* Next char is lwr *//*
					else if ((*p >= 0x8a)
					         && (*p <= 0xb7)
					         && (!(*p % 2))) *//* Even *//*
						(*p)++; *//* Next char is lwr *//*
					else if (*p == 0xb8) {
						*aChar = 0xc3;
						(*p) = 0xbf;
					} else if ((*p >= 0xb9)
					           && (*p <= 0xbe)
					           && (*p % 2)) *//* Odd *//*
						(*p)++; *//* Next char is lwr *//*
					break;
				case 0xc6: *//* Latin ext *//*
					switch (*p) {
						case 0x81:
							*aChar = 0xc9;
							(*p) = 0x93;
							break;
						case 0x86:
							*aChar = 0xc9;
							(*p) = 0x94;
							break;
						case 0x89:
							*aChar = 0xc9;
							(*p) = 0x96;
							break;
						case 0x8a:
							*aChar = 0xc9;
							(*p) = 0x97;
							break;
						case 0x8e:
							*aChar = 0xc9;
							(*p) = 0x98;
							break;
						case 0x8f:
							*aChar = 0xc9;
							(*p) = 0x99;
							break;
						case 0x90:
							*aChar = 0xc9;
							(*p) = 0x9b;
							break;
						case 0x93:
							*aChar = 0xc9;
							(*p) = 0xa0;
							break;
						case 0x94:
							*aChar = 0xc9;
							(*p) = 0xa3;
							break;
						case 0x96:
							*aChar = 0xc9;
							(*p) = 0xa9;
							break;
						case 0x97:
							*aChar = 0xc9;
							(*p) = 0xa8;
							break;
						case 0x9c:
							*aChar = 0xc9;
							(*p) = 0xaf;
							break;
						case 0x9d:
							*aChar = 0xc9;
							(*p) = 0xb2;
							break;
						case 0x9f:
							*aChar = 0xc9;
							(*p) = 0xb5;
							break;
						case 0xa9:
							*aChar = 0xca;
							(*p) = 0x83;
							break;
						case 0xae:
							*aChar = 0xca;
							(*p) = 0x88;
							break;
						case 0xb1:
							*aChar = 0xca;
							(*p) = 0x8a;
							break;
						case 0xb2:
							*aChar = 0xca;
							(*p) = 0x8b;
							break;
						case 0xb7:
							*aChar = 0xca;
							(*p) = 0x92;
							break;
						case 0x82:
						case 0x84:
						case 0x87:
						case 0x8b:
						case 0x91:
						case 0x98:
						case 0xa0:
						case 0xa2:
						case 0xa4:
						case 0xa7:
						case 0xac:
						case 0xaf:
						case 0xb3:
						case 0xb5:
						case 0xb8:
						case 0xbc:
							(*p)++; *//* Next char is lwr *//*
							break;
						default:
							break;
					}
					break;
				case 0xc7: *//* Latin ext *//*
					if (*p == 0x84)
						(*p) = 0x86;
					else if (*p == 0x85)
						(*p)++; *//* Next char is lwr *//*
					else if (*p == 0x87)
						(*p) = 0x89;
					else if (*p == 0x88)
						(*p)++; *//* Next char is lwr *//*
					else if (*p == 0x8a)
						(*p) = 0x8c;
					else if (*p == 0x8b)
						(*p)++; *//* Next char is lwr *//*
					else if ((*p >= 0x8d)
					         && (*p <= 0x9c)
					         && (*p % 2)) *//* Odd *//*
						(*p)++; *//* Next char is lwr *//*
					else if ((*p >= 0x9e)
					         && (*p <= 0xaf)
					         && (!(*p % 2))) *//* Even *//*
						(*p)++; *//* Next char is lwr *//*
					else if (*p == 0xb1)
						(*p) = 0xb3;
					else if (*p == 0xb2)
						(*p)++; *//* Next char is lwr *//*
					else if (*p == 0xb4)
						(*p)++; *//* Next char is lwr *//*
					else if (*p == 0xb6) {
						*aChar = 0xc6;
						(*p) = 0x95;
					} else if (*p == 0xb7) {
						*aChar = 0xc6;
						(*p) = 0xbf;
					} else if ((*p >= 0xb8)
					           && (*p <= 0xbf)
					           && (!(*p % 2))) *//* Even *//*
						(*p)++; *//* Next char is lwr *//*
					break;
				case 0xc8: *//* Latin ext *//*
					if ((*p >= 0x80)
					    && (*p <= 0x9f)
					    && (!(*p % 2))) *//* Even *//*
						(*p)++; *//* Next char is lwr *//*
					else if (*p == 0xa0) {
						*aChar = 0xc6;
						(*p) = 0x9e;
					} else if ((*p >= 0xa2)
					           && (*p <= 0xb3)
					           && (!(*p % 2))) *//* Even *//*
						(*p)++; *//* Next char is lwr *//*
					else if (*p == 0xbb)
						(*p)++; *//* Next char is lwr *//*
					else if (*p == 0xbd) {
						*aChar = 0xc6;
						(*p) = 0x9a;
					}
					*//* 0xba three byte small 0xe2 0xb1 0xa5 *//*
					*//* 0xbe three byte small 0xe2 0xb1 0xa6 *//*
					break;
				case 0xc9: *//* Latin ext *//*
					if (*p == 0x81)
						(*p)++; *//* Next char is lwr *//*
					else if (*p == 0x83) {
						*aChar = 0xc6;
						(*p) = 0x80;
					} else if (*p == 0x84) {
						*aChar = 0xca;
						(*p) = 0x89;
					} else if (*p == 0x85) {
						*aChar = 0xca;
						(*p) = 0x8c;
					} else if ((*p >= 0x86)
					           && (*p <= 0x8f)
					           && (!(*p % 2))) *//* Even *//*
						(*p)++; *//* Next char is lwr *//*
					break;
				case 0xcd: *//* Greek & Coptic *//*
					switch (*p) {
						case 0xb0:
						case 0xb2:
						case 0xb6:
							(*p)++; *//* Next char is lwr *//*
							break;
						case 0xbf:
							*aChar = 0xcf;
							(*p) = 0xb3;
							break;
						default:
							break;
					}
					break;
				case 0xce: *//* Greek & Coptic *//*
					if (*p == 0x86)
						(*p) = 0xac;
					else if (*p == 0x88)
						(*p) = 0xad;
					else if (*p == 0x89)
						(*p) = 0xae;
					else if (*p == 0x8a)
						(*p) = 0xaf;
					else if (*p == 0x8c) {
						*aChar = 0xcf;
						(*p) = 0x8c;
					} else if (*p == 0x8e) {
						*aChar = 0xcf;
						(*p) = 0x8d;
					} else if (*p == 0x8f) {
						*aChar = 0xcf;
						(*p) = 0x8e;
					} else if ((*p >= 0x91)
					           && (*p <= 0x9f))
						(*p) += 0x20; *//* US ASCII shift *//*
					else if ((*p >= 0xa0)
					         && (*p <= 0xab)
					         && (*p != 0xa2)) {
						*aChar = 0xcf;
						(*p) -= 0x20;
					}
					break;
				case 0xcf: *//* Greek & Coptic *//*
					if (*p == 0x8f)
						(*p) = 0x97;
					else if ((*p >= 0x98)
					         && (*p <= 0xaf)
					         && (!(*p % 2))) *//* Even *//*
						(*p)++; *//* Next char is lwr *//*
					else if (*p == 0xb4) {
						(*p) = 0x91;
					} else if (*p == 0xb7)
						(*p)++; *//* Next char is lwr *//*
					else if (*p == 0xb9)
						(*p) = 0xb2;
					else if (*p == 0xba)
						(*p)++; *//* Next char is lwr *//*
					else if (*p == 0xbd) {
						*aChar = 0xcd;
						(*p) = 0xbb;
					} else if (*p == 0xbe) {
						*aChar = 0xcd;
						(*p) = 0xbc;
					} else if (*p == 0xbf) {
						*aChar = 0xcd;
						(*p) = 0xbd;
					}
					break;
				case 0xd0: *//* Cyrillic *//*
					if ((*p >= 0x80)
					    && (*p <= 0x8f)) {
						*aChar = 0xd1;
						(*p) += 0x10;
					} else if ((*p >= 0x90)
					           && (*p <= 0x9f))
						(*p) += 0x20; *//* US ASCII shift *//*
					else if ((*p >= 0xa0)
					         && (*p <= 0xaf)) {
						*aChar = 0xd1;
						(*p) -= 0x20;
					}
					break;
				case 0xd1: *//* Cyrillic supplement *//*
					if ((*p >= 0xa0)
					    && (*p <= 0xbf)
					    && (!(*p % 2))) *//* Even *//*
						(*p)++; *//* Next char is lwr *//*
					break;
				case 0xd2: *//* Cyrillic supplement *//*
					if (*p == 0x80)
						(*p)++; *//* Next char is lwr *//*
					else if ((*p >= 0x8a)
					         && (*p <= 0xbf)
					         && (!(*p % 2))) *//* Even *//*
						(*p)++; *//* Next char is lwr *//*
					break;
				case 0xd3: *//* Cyrillic supplement *//*
					if (*p == 0x80)
						(*p) = 0x8f;
					else if ((*p >= 0x81)
					         && (*p <= 0x8e)
					         && (*p % 2)) *//* Odd *//*
						(*p)++; *//* Next char is lwr *//*
					else if ((*p >= 0x90)
					         && (*p <= 0xbf)
					         && (!(*p % 2))) *//* Even *//*
						(*p)++; *//* Next char is lwr *//*
					break;
				case 0xd4: *//* Cyrillic supplement & Armenian *//*
					if ((*p >= 0x80)
					    && (*p <= 0xaf)
					    && (!(*p % 2))) *//* Even *//*
						(*p)++; *//* Next char is lwr *//*
					else if ((*p >= 0xb1)
					         && (*p <= 0xbf)) {
						*aChar = 0xd5;
						(*p) -= 0x10;
					}
					break;
				case 0xd5: *//* Armenian *//*
					if ((*p >= 0x80)
					    && (*p <= 0x8f)) {
						(*p) += 0x30;
					} else if ((*p >= 0x90)
					           && (*p <= 0x96)) {
						*aChar = 0xd6;
						(*p) -= 0x10;
					}
					break;
				case 0xe1: *//* Three byte code *//*
					aChar = p;
					p++;
					switch (*aChar) {
						case 0x82: *//* Georgian asomtavruli *//*
							if ((*p >= 0xa0)
							    && (*p <= 0xbf)) {
								*aChar = 0x83;
								(*p) -= 0x10;
							}
							break;
						case 0x83: *//* Georgian asomtavruli *//*
							if (((*p >= 0x80)
							     && (*p <= 0x85))
							    || (*p == 0x87)
							    || (*p == 0x8d))
								(*p) += 0x30;
							break;
						case 0x8e: *//* Cherokee *//*
							if ((*p >= 0xa0)
							    && (*p <= 0xaf)) {
								*(p - 2) = 0xea;
								*aChar = 0xad;
								(*p) += 0x10;
							} else if ((*p >= 0xb0)
							           && (*p <= 0xbf)) {
								*(p - 2) = 0xea;
								*aChar = 0xae;
								(*p) -= 0x30;
							}
							break;
						case 0x8f: *//* Cherokee *//*
							if ((*p >= 0x80)
							    && (*p <= 0xaf)) {
								*(p - 2) = 0xea;
								*aChar = 0xae;
								(*p) += 0x10;
							} else if ((*p >= 0xb0)
							           && (*p <= 0xb5)) {
								(*p) += 0x08;
							}
							*//* 0xbe three byte small 0xe2 0xb1 0xa6 *//*
							break;
						case 0xb2: *//* Georgian mtavruli *//*
							if (((*p >= 0x90)
							     && (*p <= 0xba))
							    || (*p == 0xbd)
							    || (*p == 0xbe)
							    || (*p == 0xbf))
								*aChar = 0x83;
							break;
						case 0xb8: *//* Latin ext *//*
							if ((*p >= 0x80)
							    && (*p <= 0xbf)
							    && (!(*p % 2))) *//* Even *//*
								(*p)++; *//* Next char is lwr *//*
							break;
						case 0xb9: *//* Latin ext *//*
							if ((*p >= 0x80)
							    && (*p <= 0xbf)
							    && (!(*p % 2))) *//* Even *//*
								(*p)++; *//* Next char is lwr *//*
							break;
						case 0xba: *//* Latin ext *//*
							if ((*p >= 0x80)
							    && (*p <= 0x94)
							    && (!(*p % 2))) *//* Even *//*
								(*p)++; *//* Next char is lwr *//*
							else if ((*p >= 0xa0)
							         && (*p <= 0xbf)
							         && (!(*p % 2))) *//* Even *//*
								(*p)++; *//* Next char is lwr *//*
							*//* 0x9e Two byte small 0xc3 0x9f *//*
							break;
						case 0xbb: *//* Latin ext *//*
							if ((*p >= 0x80)
							    && (*p <= 0xbf)
							    && (!(*p % 2))) *//* Even *//*
								(*p)++; *//* Next char is lwr *//*
							break;
						case 0xbc: *//* Greek ex *//*
							if ((*p >= 0x88)
							    && (*p <= 0x8f))
								(*p) -= 0x08;
							else if ((*p >= 0x98)
							         && (*p <= 0x9d))
								(*p) -= 0x08;
							else if ((*p >= 0xa8)
							         && (*p <= 0xaf))
								(*p) -= 0x08;
							else if ((*p >= 0xb8)
							         && (*p <= 0xbf))
								(*p) -= 0x08;
							break;
						case 0xbd: *//* Greek ex *//*
							if ((*p >= 0x88)
							    && (*p <= 0x8d))
								(*p) -= 0x08;
							else if ((*p == 0x99)
							         || (*p == 0x9b)
							         || (*p == 0x9d)
							         || (*p == 0x9f))
								(*p) -= 0x08;
							else if ((*p >= 0xa8)
							         && (*p <= 0xaf))
								(*p) -= 0x08;
							break;
						case 0xbe: *//* Greek ex *//*
							if ((*p >= 0x88)
							    && (*p <= 0x8f))
								(*p) -= 0x08;
							else if ((*p >= 0x98)
							         && (*p <= 0x9f))
								(*p) -= 0x08;
							else if ((*p >= 0xa8)
							         && (*p <= 0xaf))
								(*p) -= 0x08;
							else if ((*p >= 0xb8)
							         && (*p <= 0xb9))
								(*p) -= 0x08;
							else if ((*p >= 0xba)
							         && (*p <= 0xbb)) {
								*(p - 1) = 0xbd;
								(*p) -= 0x0a;
							} else if (*p == 0xbc)
								(*p) -= 0x09;
							break;
						case 0xbf: *//* Greek ex *//*
							if ((*p >= 0x88)
							    && (*p <= 0x8b)) {
								*(p - 1) = 0xbd;
								(*p) += 0x2a;
							} else if (*p == 0x8c)
								(*p) -= 0x09;
							else if ((*p >= 0x98)
							         && (*p <= 0x99))
								(*p) -= 0x08;
							else if ((*p >= 0x9a)
							         && (*p <= 0x9b)) {
								*(p - 1) = 0xbd;
								(*p) += 0x1c;
							} else if ((*p >= 0xa8)
							           && (*p <= 0xa9))
								(*p) -= 0x08;
							else if ((*p >= 0xaa)
							         && (*p <= 0xab)) {
								*(p - 1) = 0xbd;
								(*p) += 0x10;
							} else if (*p == 0xac)
								(*p) -= 0x07;
							else if ((*p >= 0xb8)
							         && (*p <= 0xb9)) {
								*(p - 1) = 0xbd;
							} else if ((*p >= 0xba)
							           && (*p <= 0xbb)) {
								*(p - 1) = 0xbd;
								(*p) += 0x02;
							} else if (*p == 0xbc)
								(*p) -= 0x09;
							break;
						default:
							break;
					}
					break;
				case 0xe2: *//* Three byte code *//*
					aChar = p;
					p++;
					switch (*aChar) {
						case 0xb0: *//* Glagolitic *//*
							if ((*p >= 0x80)
							    && (*p <= 0x8f)) {
								(*p) += 0x30;
							} else if ((*p >= 0x90)
							           && (*p <= 0xae)) {
								*aChar = 0xb1;
								(*p) -= 0x10;
							}
							break;
						case 0xb1: *//* Latin ext *//*
							switch (*p) {
								case 0xa0:
								case 0xa7:
								case 0xa9:
								case 0xab:
								case 0xb2:
								case 0xb5:
									(*p)++; *//* Next char is lwr *//*
									break;
								case 0xa2: *//* Two byte small 0xc9 0xab *//*
								case 0xa4: *//* Two byte small 0xc9 0xbd *//*
								case 0xad: *//* Two byte small 0xc9 0x91 *//*
								case 0xae: *//* Two byte small 0xc9 0xb1 *//*
								case 0xaf: *//* Two byte small 0xc9 0x90 *//*
								case 0xb0: *//* Two byte small 0xc9 0x92 *//*
								case 0xbe: *//* Two byte small 0xc8 0xbf *//*
								case 0xbf: *//* Two byte small 0xc9 0x80 *//*
									break;
								case 0xa3:
									*(p - 2) = 0xe1;
									*(p - 1) = 0xb5;
									*(p) = 0xbd;
									break;
								default:
									break;
							}
							break;
						case 0xb2: *//* Coptic *//*
							if ((*p >= 0x80)
							    && (*p <= 0xbf)
							    && (!(*p % 2))) *//* Even *//*
								(*p)++; *//* Next char is lwr *//*
							break;
						case 0xb3: *//* Coptic *//*
							if (((*p >= 0x80)
							     && (*p <= 0xa3)
							     && (!(*p % 2))) *//* Even *//*
							    || (*p == 0xab)
							    || (*p == 0xad)
							    || (*p == 0xb2))
								(*p)++; *//* Next char is lwr *//*
							break;
						case 0xb4: *//* Georgian nuskhuri *//*
							if (((*p >= 0x80)
							     && (*p <= 0xa5))
							    || (*p == 0xa7)
							    || (*p == 0xad)) {
								*(p - 2) = 0xe1;
								*(p - 1) = 0x83;
								(*p) += 0x10;
							}
							break;
						default:
							break;
					}
					break;
				case 0xea: *//* Three byte code *//*
					aChar = p;
					p++;
					switch (*aChar) {
						case 0x99: *//* Cyrillic *//*
							if ((*p >= 0x80)
							    && (*p <= 0xad)
							    && (!(*p % 2))) *//* Even *//*
								(*p)++; *//* Next char is lwr *//*
							break;
						case 0x9a: *//* Cyrillic *//*
							if ((*p >= 0x80)
							    && (*p <= 0x9b)
							    && (!(*p % 2))) *//* Even *//*
								(*p)++; *//* Next char is lwr *//*
							break;
						case 0x9c: *//* Latin ext *//*
							if ((((*p >= 0xa2)
							      && (*p <= 0xaf))
							     || ((*p >= 0xb2)
							         && (*p <= 0xbf)))
							    && (!(*p % 2))) *//* Even *//*
								(*p)++; *//* Next char is lwr *//*
							break;
						case 0x9d: *//* Latin ext *//*
							if ((((*p >= 0x80)
							      && (*p <= 0xaf))
							     && (!(*p % 2))) *//* Even *//*
							    || (*p == 0xb9)
							    || (*p == 0xbb)
							    || (*p == 0xbe))
								(*p)++; *//* Next char is lwr *//*
							else if (*p == 0xbd) {
								*(p - 2) = 0xe1;
								*(p - 1) = 0xb5;
								*(p) = 0xb9;
							}
							break;
						case 0x9e: *//* Latin ext *//*
							if (((((*p >= 0x80)
							       && (*p <= 0x87))
							      || ((*p >= 0x96)
							          && (*p <= 0xa9))
							      || ((*p >= 0xb4)
							          && (*p <= 0xbf)))
							     && (!(*p % 2))) *//* Even *//*
							    || (*p == 0x8b)
							    || (*p == 0x90)
							    || (*p == 0x92))
								(*p)++; *//* Next char is lwr *//*
							else if (*p == 0xb3) {
								*(p - 2) = 0xea;
								*(p - 1) = 0xad;
								*(p) = 0x93;
							}
							*//* case 0x8d: // Two byte small 0xc9 0xa5 *//*
							*//* case 0xaa: // Two byte small 0xc9 0xa6 *//*
							*//* case 0xab: // Two byte small 0xc9 0x9c *//*
							*//* case 0xac: // Two byte small 0xc9 0xa1 *//*
							*//* case 0xad: // Two byte small 0xc9 0xac *//*
							*//* case 0xae: // Two byte small 0xc9 0xaa *//*
							*//* case 0xb0: // Two byte small 0xca 0x9e *//*
							*//* case 0xb1: // Two byte small 0xca 0x87 *//*
							*//* case 0xb2: // Two byte small 0xca 0x9d *//*
							break;
						case 0x9f: *//* Latin ext *//*
							if ((*p == 0x82)
							    || (*p == 0x87)
							    || (*p == 0x89)
							    || (*p == 0xb5))
								(*p)++; *//* Next char is lwr *//*
							else if (*p == 0x84) {
								*(p - 2) = 0xea;
								*(p - 1) = 0x9e;
								*(p) = 0x94;
							} else if (*p == 0x86) {
								*(p - 2) = 0xe1;
								*(p - 1) = 0xb6;
								*(p) = 0x8e;
							}
							*//* case 0x85: // Two byte small 0xca 0x82 *//*
							break;
						default:
							break;
					}
					break;
				case 0xef: *//* Three byte code *//*
					aChar = p;
					p++;
					switch (*aChar) {
						case 0xbc: *//* Latin fullwidth *//*
							if ((*p >= 0xa1)
							    && (*p <= 0xba)) {
								*aChar = 0xbd;
								(*p) -= 0x20;
							}
							break;
						default:
							break;
					}
					break;
				case 0xf0: *//* Four byte code *//*
					aChar = p;
					p++;
					switch (*aChar) {
						case 0x90:
							aChar = p;
							p++;
							switch (*aChar) {
								case 0x90: *//* Deseret *//*
									if ((*p >= 0x80)
									    && (*p <= 0x97)) {
										(*p) += 0x28;
									} else if ((*p >= 0x98)
									           && (*p <= 0xa7)) {
										*aChar = 0x91;
										(*p) -= 0x18;
									}
									break;
								case 0x92: *//* Osage  *//*
									if ((*p >= 0xb0)
									    && (*p <= 0xbf)) {
										*aChar = 0x93;
										(*p) -= 0x18;
									}
									break;
								case 0x93: *//* Osage  *//*
									if ((*p >= 0x80)
									    && (*p <= 0x93))
										(*p) += 0x28;
									break;
								case 0xb2: *//* Old hungarian *//*
									if ((*p >= 0x80)
									    && (*p <= 0xb2))
										*aChar = 0xb3;
									break;
								default:
									break;
							}
							break;
						case 0x91:
							aChar = p;
							p++;
							switch (*aChar) {
								case 0xa2: *//* Warang citi *//*
									if ((*p >= 0xa0)
									    && (*p <= 0xbf)) {
										*aChar = 0xa3;
										(*p) -= 0x20;
									}
									break;
								default:
									break;
							}
							break;
						case 0x96:
							aChar = p;
							p++;
							switch (*aChar) {
								case 0xb9: *//* Medefaidrin *//*
									if ((*p >= 0x80)
									    && (*p <= 0x9f)) {
										(*p) += 0x20;
									}
									break;
								default:
									break;
							}
							break;
						case 0x9E:
							aChar = p;
							p++;
							switch (*aChar) {
								case 0xA4: *//* Adlam *//*
									if ((*p >= 0x80)
									    && (*p <= 0x9d))
										(*p) += 0x22;
									else if ((*p >= 0x9e)
									         && (*p <= 0xa1)) {
										*(aChar) = 0xa5;
										(*p) -= 0x1e;
									}
									break;
								default:
									break;
							}
							break;
						default:
							break;
					}
					break;
				default:
					break;
			}
			aChar = 0;
		}
		p++;
	}
}*/

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
	memcpy0(c, a, len_a);
	memcpy0(c + len_a, b, len_b);
	//	c[len_a + len_b + 1] = 0;// hwhy?
	return c;
}


bytes concat(bytes a, char b, int len) {
	bytes c = new unsigned char[len + 1];
	memcpy0(c, a, len);
	c[len] = b;
	return c;
}

bytes concat(char a, bytes b, int len) {
	bytes c = new unsigned char[len + 1];
	c[0] = a;
	memcpy0(c + 1, b, len);
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