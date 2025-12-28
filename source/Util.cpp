// #include <cmath>
#include <stdlib.h> // abs(int)
// #include "String.h"
#include "Util.h"
#include "Code.h"

#ifndef WASM
// ok as wasi?
#include "unistd.h"
#include "NodeTypes.h"
#include "List.h"
#include "Angle.h"

#endif

template<typename S, typename T>
struct FuncLet {
    typedef T (lambda)(S param);
};


template<class S, class T>
// reads a wasm file and returns a function with signature f(S)->T
typename FuncLet<S, T>::lambda wasmlet(Path file, String func = "") {
    if (func == "")
        func = file.substring(file.lastIndexOf("/")).substring(0, file.lastIndexOf(".wasm")); // or just main()!
    // todo: capture [func] ?
    auto &lamb = [](S s) -> T {
        //  Module m=parse_wasm_file(file);
        //  return m.run(s)
        todo("wasmlet");
        return new T();
    };
    return lamb;
}

//template<class S,class T> T run_wasmlet(Path file, S arg){
//    return S((smartNode(smarty)run_wasm_function(file,file,arg));
//}


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
    if (isDir(filename.data))return false; // only true files here!
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
    ptr = fopen(file, "rb"); // r for read, b for binary
    if (!ptr) //return -1;
        error("File not found "s + file);
    fseek(ptr, 0L, SEEK_END);
    int sz = ftell(ptr);
    return sz;
#endif
    return -1; // todo
}

#if WASM && !WASI
#else

#include <sys/stat.h>
#include <time.h>

int64 file_last_modified(char *file) {
    struct stat attr;
    stat(file, &attr);
    return attr.st_mtime;
}

float file_last_modified(String &file) {
    struct stat attr;
    stat(file.data, &attr);
    return attr.st_mtime;
}

#endif

template<class>
class List; //<String>;

#include <stdio.h>
#include <sys/types.h>
#include <dirent.h>
#include <errno.h>

bool isDir(const char *name) {
#ifdef WASM
    return false;
#else
    DIR *directory = opendir(name);
    // allow symlinks to directories
    if (directory == NULL && errno == ENOENT) {
        // Check if it's a symlink
        struct stat statbuf;
        if (lstat(name, &statbuf) == 0 && S_ISLNK(statbuf.st_mode)) {
            return true;
        }
    }
    if (directory != NULL) {
        closedir(directory);
        return true;
    }
    if (errno == ENOTDIR)
        return false;
    return false; // -1;
#endif
}
bool isDir(String name) {return isDir(name.data);}

//String findNewestFile(String filename) {
String findFile(String filename, String current_dir) {
#if MY_WASM
    return filename; // todo read from host?
#elif RUNTIME_ONLY or WASM
    return "";
#else
    if (fileExists(filename))return filename;
    if (fileExists(current_dir + "/" + filename))return current_dir + "/" + filename;
    List<String> extensions = {"", ".wit", ".wasm", ".wast", ".wasp", ".witx"};
    List<String> folders = {"", "lib", "src", "wasp", "source", "include", "sample", "samples", "test", "tests"};
    List<String> paths;
    if (filename.contains("."))extensions = {""}; // don't do double extensions!
    if (isDir(filename)) {
        folders = {filename};
        filename = "index"; // todo main / lib
    } else if (isDir(current_dir + "/" + filename)) {
        folders = {filename};
        filename = "index"; // todo or main or lib or $filename
    }
    if (filename.contains("/"))
        for (auto &extension: extensions) {
            String &path = filename + extension;
            if (fileExists(path))
                paths.add(path);
        }
    else {
        for (auto &folder: folders)
            for (auto &extension: extensions) {
                String &path = current_dir + "/" + folder + "/" + filename + extension;
                if (fileExists(path))
                    paths.add(path);
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

bool contains(bytes list, bytes match, int len, int match_len) {
    for (int i = 0; i < len; i++)
        if (list[i] == match[0]) {
            bool ok = true;
            for (int j = 1; j < match_len; j++)
                if (list[i + j] != match[j]) {
                    ok = false;
                    break;
                }
            if (ok)return true;
        }
    return false;
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

short normChar(char c) {
    // 0..36 damn ;)
    if (c == '\n')return 0;
    if (c >= '0' and c <= '9') return c - '0' + 26;
    if (c >= 'a' and c <= 'z') return c - 'a' + 1; // NOT 0!!!
    if (c >= 'A' and c <= 'Z') return c - 'A' + 1; // NOT 0!!!
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
            return c; // for asian etc!
    }
}

unsigned int wordHash(const char *str, int max_chars) {
    // unsigned
    if (!str) return 0;
    int maxNodes = 100000;
    char c;
    unsigned int hash = 5381, hash2 = 7; // int64
    while (max_chars-- > 0 and (c = *str++)) {
        hash2 = hash2 * 31 + (short) (c);
        int next = normChar(c); //a_b-c==AbC
        if (next == 0)continue;
        hash = hash * 33 + next; // ((hash << 5) + hash
        hash = hash % maxNodes;
    }
    if (hash == 0)return hash2;
    return hash;
}

char *readFile(chars filename, int *size_out) {
    if (!filename)
        error("no filename given");
    if (!filename)return 0;
#ifndef WASM
    FILE *f = fopen(filename, "rt");
    if (!f)
        error("FILE NOT FOUND "_s + filename);
    fseek(f, 0, SEEK_END);
    int64 fsize = ftell(f);
    if (size_out)*size_out = fsize;
    fseek(f, 0, SEEK_SET); /* same as rewind(f); */
    char *s = (char *) (alloc(fsize, 2));
    fread(s, 1, fsize, f);
    fclose(f);
    return s;
#else
    return 0;
#endif
}

bool similar(double a, double b) {
    if (a == b)return true;
    if (a == 0)return abs(b) < .0001;
    if (b == 0)return abs(a) < .0001;
    double epsilon = abs(a + b) / 10000.; // percentual ++ todo add 10^order parameter
    bool ok = abs(a - b) <= epsilon;
    return ok;
}

//void lowerCaseUTF(chars string, int length); // defined in lowerCaseUTF.c

void lowerCase(char *string, int length) {
    if (!string || !*string) return;
    if (length <= 0)
        length = strlen(string);
    while (length-- > 0) {
        char &ch = string[length];
        if (ch >= 'A' and ch <= 'Z')
            ch += 32;
        //        if(ch < 0 ){
        //            lowerCaseUTF(string, length+2);// todo: go LTR to avoid corruption while(length >++len)
        //            return;
        //        }
    }
}

int equals_ignore_case(chars s1, chars s2, size_t ztCount) {
    bytes pStr1Low = 0;
    bytes pStr2Low = 0;
    bytes p1 = 0;
    bytes p2 = 0;
    if (s1 && *s1 && s2 && *s2) {
        pStr1Low = (bytes) calloc(strlen(s1) + 1, sizeof(unsigned char));
        if (pStr1Low) {
            pStr2Low = (bytes) calloc(strlen(s2) + 1, sizeof(unsigned char));
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

// todo merge with chars?
bytes concat(bytes a, bytes b, int len_a, int len_b) {
    bytes c = new unsigned char[len_a + len_b + 4]; // why+4 ?? else heap-buffer-overflow
    memcpy1(c, a, len_a);
    memcpy1(c + len_a, b, len_b);
    //	c[len_a + len_b + 1] = 0;// hwhy?
    return c;
}

chars concat(chars a, chars b) {
    if (!a or a[0] == 0)return b;
    if (!b or b[0] == 0)return a;
    int la = (int) strlen(a);
    int lb = (int) strlen(b);
    return concat(a, b, la, lb);
}
chars concat(const String& a, chars b) {
    return concat(a.data, b);
}

// extern "C" byte *heap_end;
// extern "C" byte *testHeap() {
//     return heap_end;// TODO ⚠️ NOT influenced by js HEAP_END += length !!!
// }


chars concat(chars a, chars b, uint la, uint lb) {
    if (!a or a[0] == 0 or la == 0)return b;
    if (!b or b[0] == 0 or lb == 0)return a;
#if WASM
    if ((byte *) a + la == heap_end) {
        memcpy((void *) &a[la], b, lb + 1);
        ((char *) a)[la + lb] = 0;
        heap_end += lb + 1;
        return a;
    }
#endif
    char *c = (char *) malloc((la + lb + 1) * sizeof(char));
    strcpy2(c, a);
    strcpy2(&c[la], b);
    c[la + lb] = 0;
    return c;
}

bytes concat(bytes a, byte b, int len) {
#if WASM
    if (a + len == heap_end) {
        a[len] = b;
        heap_end++;
        return a;
    }
#endif
    bytes c = new byte[len + 1];
    memcpy1(c, a, len);
    c[len] = b;
    return c;
}

chars concat_chars(chars a, chars b) {
    return concat(a, b, (uint) strlen(a), (uint) strlen(b));
}

bytes concat(char a, bytes b, int len) {
    bytes c = new unsigned char[len + 1];
    c[0] = a;
    memcpy1(c + 1, b, len);
    return c;
}

// Numerically accurate, dependency-free ln using bit tricks and minimax polynomial
// log10(1000000000000000000.0) ≈ 18, breaks after
double ln(double y) {
    if (y <= 0) return -1 / 0.0; // -inf or nan

    union { double d; uint64_t i; } u = { y };
    long e = ((long)((u.i >> 52) & 0x7FFl)) - 1023;
    u.i = (u.i & ((1ULL << 52) - 1)) | (1023ULL << 52);
    double m = u.d;
    // double m = frexp(y, &e); // y = m * 2^e, m in [0.5,1)  get much better e&m via emscripten / wasi-libc.

    double z = (m - 1.0) / (m + 1.0);
    double z2 = z * z;

    // Degree-5 odd polynomial for ln(m) = 2z (1 + z^2 * (...))
    double p = z2 * (1.0/3 + z2 * (1.0/5 + z2 * (1.0/7 + z2 * (1.0/9))));
    double log_m = 2 * (z + z * p);

    return log_m + e * 0.6931471805599453;
}

double log(double y, double base) {
    return ln(y) / ln(base);
}

// up to 10^18 … ok'ish
double log10(double y) { //noexcept{
    return ln(y)/2.302585092994046; // ln(10)
}

double log2(double y) { //noexcept{
    return ln(y)/0.6931471805599453; // ln(2)
}

String load(String file) {
    int size;
    char *text = readFile(file.data, &size);
    return String(text, size);
}

static String zerox = "0x00";
static String ZERO = "0";

String &hex(int64 d0, bool include_0x, bool upper_case) {
    int64 d = abs(d0);
    if (d == 0)
        return include_0x ? zerox : ZERO;
    int size = 4 + 64 / 4;
    char s[size];
    //#ifdef WASM
    int i = 0;
    while (d and size) {
        if (d % 16 < 10)
            s[i++] = '0' + d % 16;
        else
            s[i++] = (upper_case ? 'A' : 'a') + d % 16 - 10;
        d /= 16;
    }
    if (include_0x) {
        if (abs(d0) < 16)
            s[i++] = '0'; // pad 0x01
        s[i++] = 'x';
        s[i++] = '0';
    }
    if (d0 < 0)s[i++] = '-';
    s[i] = 0;
    reverseInPlace(s, i);
    return *new String(s);
}

bool isSmartPointer(int64 d) {
    //	if((d&negative_mask_64)==negative_mask_64)return false;
    if ((d & negative_mask_64) == negative_mask_64)return false;
    if (d & double_mask_64)return true;
    return d & smart_mask_64 and not(d & negative_mask_64);
}

Node smartValue(int64 smartPointer);

#if not WASM
#include <cxxabi.h>
#endif

String demangle(const String &fun) {
    char *result = 0;
    int status = -1;
#if MY_WASM
    result = js_demangle(fun);
    status = 1;
    // print("demangled "s+fun+" to "s+result);
#elif not WASM
    result = abi::__cxa_demangle(fun.data, 0, 0, &status);
#endif
    if (status < 0 or result == 0)
        return fun; // not demangled (e.g. "memory")
    return result;
}

String extractFuncName(const String &fun) {
    if (fun.length == 0)return "";
    int status;
    auto real_name = demangle(fun);
    // print("demangled "s + fun + " to "s + real_name);
    String ok = real_name.substring(0, real_name.lastIndexOf("("));
    if (ok.contains(':'))
        ok = ok.substring(ok.lastIndexOf("::") + 2);
    if (ok.contains(' '))
        ok = ok.substring(ok.lastIndexOf(" ") + 1);
    // print("demangled "s + fun + " to "s + ok);
    return ok; // .clone(); unnecessary: return by value copy
}


// compressed arrays
int stackItemSize(Node &clazz, bool throws) {
    if (clazz == BoolType)return 1; //0;
    if (clazz == ByteType)return 1;
    if (clazz == ShortType)return 2;
    if (clazz == IntegerType)return 4;
    if (clazz == LongType)return 4;
    if (clazz.kind == unknown)return 4; // hack
    if (clazz.kind == structs)
        return 4; // todo: ignore and just get index from member (OR bad idea: sum up type sizes)
    //     typeName(clazz) +
    //    return 4;
    if (throws) todo("stackItemSize for "s + clazz.name + kindName(clazz.kind) + " " + clazz.serialize());
    return stackItemSize(mapTypeToWasm(clazz));
}

// size of the primitive value, not sizeof(Node)
int stackItemSize(Type type, bool throws) {
    if (isGeneric(type))return stackItemSize(type.generics.value_type, throws);
    if (type == byte_i8)return 1;
    if (type == byte_char)return 1;
    if (type == charp)return 1; // chars for now vs codepoint!
    if (type == stringp)return 1; // chars for now vs pointer!
    if (type == int16t)return 2;
    if (type == codepoint32)return 4;
    if (type == Valtype::int32t)return 4;
    if (type == array)return 4; // pointer todo!
    if (type == Valtype::i64)return 8;
    if (type == Valtype::float32t)return 4;
    if (type == Valtype::float64t)return 8;
    if (type == void_block)return 4; // int32 pointer hack todo!
    if (type == unknown_type)return 4;
    //    if (type == node_pointer)return 4;
    if (throws)
        error("unknown size for stack item "s + typeName(type));
    return 0;
}

int stackItemSize(Valtype type, bool throws) {
    if (type == Valtype::int32t)return 4;
    if (type == Valtype::i64)return 8;
    if (type == Valtype::float32t)return 4;
    if (type == Valtype::float64t)return 8;
    if (throws)
        todo("stackItemSize for"s+typeName(type));
    return 0;
}


extern "C" int64 powi(int a, unsigned int b) {
    int64 res = 1;
    while (b-- > 0)res = res * a;
    return res;
}

double powdi(double a, unsigned int b) {
    double res = 1;
    while (b-- > 0)res = res * a;
    return res;
}


char *dropPath(char *file0) {
    auto file = String(file0);
    return file.substring(file.lastIndexOf("/") + 1).data;
}

String extractPath(String file) {
    if (!file.contains("/"))return "/";
    return file.substring(0, file.lastIndexOf("/"));
}


// Base64 encoding table

// Function to encode a string to Base64
char *base64_encode(const char *data) {
    static const char base64_table[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    size_t data_len = strlen(data);
    size_t encoded_len = 4 * ((data_len + 2) / 3);
    char *encoded = (char *) malloc(encoded_len + 1);

    if (encoded == NULL) return NULL; // Memory allocation failed

    unsigned char b;
    for (size_t i = 0, j = 0; i < data_len;) {
        uint32_t octet_a = i < data_len ? (unsigned char) data[i++] : 0;
        uint32_t octet_b = i < data_len ? (unsigned char) data[i++] : 0;
        uint32_t octet_c = i < data_len ? (unsigned char) data[i++] : 0;

        uint32_t triple = (octet_a << 16) + (octet_b << 8) + octet_c;

        encoded[j++] = base64_table[(triple >> 18) & 0x3F];
        encoded[j++] = base64_table[(triple >> 12) & 0x3F];
        b = (triple >> 6) & 0x3F;
        encoded[j++] = (i > data_len + 1) ? '=' : base64_table[b];
        b = triple & 0x3F;
        encoded[j++] = (i > data_len) ? '=' : base64_table[b];
    }

    encoded[encoded_len] = '\0';
    return encoded;
}


// todo: test polymorphism / multi dispatch
int square(int a) {
    return a * a;
}

double square(double a) {
    return a * a;
}

// int64 squarel(int64 n); // test wasm, otherwise use x² => x*x in analyze!
// int64 square(int64 a) { //
//     return a * a;
// }


String compileWast(chars file) {
#ifndef WASM
        // chars wast_compiler = "/usr/local/bin/wat2wasm  --enable-all --debug-names ";
        chars wast_compiler = "/opt/homebrew/bin/wasm-as --enable-reference-types --enable-gc ";
        int status = system(concat(wast_compiler, file));
        if (status)
            error("FAILED compiling wast dependencty "s + file);
#endif
        return String(file).replace(".wast", ".wasm");
}

bool file_exists(chars filename) {
    FILE *file = fopen(filename, "r");
    if (file) {
        fclose(file);
        return true;
    }
    return false;
}
bool file_exists(String filename) {
    return file_exists(filename.data);
}
List<String> ls(String path) {
    List<String> files;
#ifndef WASM
    DIR *dir;
    struct dirent *ent;
    if ((dir = opendir(path.data)) != NULL) {
        // print("ls "s + path + "\n");
        while ((ent = readdir(dir)) != NULL) {
            String name = String(ent->d_name);
            if (name != "." and name != "..")
                files.add(name);
            // print(" found "s + name + "\n");
        }
        closedir(dir);
    } else {
        // perror(("could not open directory "s + path).data);
        warn(("could not open directory "s + path).data);
    }
#endif
    return files;
}