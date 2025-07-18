#pragma once

#define let auto
//#define var auto &// conflicts with const Var &var …

#define allow_untyped_nodes true  // IMPORTANT!  {a b c}#2"=="b" VALID or NOT ?!?
//#include "asserts.h" // 30.11. XXX
#include "wasm_helpers.h"

#if WASM
//typedef unsigned long size_t;
char* dropPath(char* str);
#define debug_line() printf("\n%s:%d\n",dropPath(__FILE__),__LINE__);
// #define debug_line() printf("\n%s:%d\n",__FILE__,__LINE__);
// #define debug_line() printf("\nfile://%s\n%s:%d\n",__FILE__,__FILE__,__LINE__);
// #define debug_line() printf("\nfile://%s:%d\n",__FILE__,__LINE__);
// cd / && python3 -m http.server 8000
// #define debug_line() printf("\nhttp://localhost:8000%s\n",__FILE__);
// #define debug_line() printf("\nhttp://localhost:8000/%s\n%s:%d\n",__FILE__,__FILE__,__LINE__);
// #define debug_line() printf("\nhttp://localhost:8000/%s:%d\n",__FILE__,__LINE__);
// #define debug_line() printf("\nvscode:%s?line=%d\n",__FILE__,__LINE__);
#else
#define debug_line() printf("\n%s:%d\n",__FILE__,__LINE__);
#endif
#define backtrace_line() {printf("\n%s:%d\n",__FILE__,__LINE__);}
#define backtrace_exit() {printf("\n%s:%d\n",__FILE__,__LINE__);proc_exit(-1);}


//typedef int64 i64;
typedef long long int64;
typedef unsigned long long uint64;
typedef unsigned long long u64;
//typedef uint64 bytes8;
typedef unsigned char byte;
typedef const char *chars;
typedef byte *bytes;

#define breakpoint_helper printf("\n%s:%d breakpoint_helper\n",__FILE__,__LINE__);

//#define min(a, b) (a < b ? a : b)
static int64 min(int64 a, int64 b) { return (a < b ? a : b); }

// saves one backtrace entry:
//#if WASM
//#define error(msg) {\
//put_chars("\nERROR\n"); \
//put_chars(__FILE__);\
//put_chars(":");\
//puti(__LINE__);\
//put_chars((char *) (msg));\
//put_chars("\n");   \
//proc_exit(-1); }
//#else
#define error(msg) error1(msg,__FILE__,__LINE__)
#define werror(msg) warn(msg)
//#endif


#define internal_error(msg) error1("internal error: " msg,__FILE__,__LINE__)

//typedef int64 int64;  = 0x7E, in enum

static bool I_know_what_I_am_doing = false;

#if RUNTIME_ONLY or RELEASE
#define todo(msg) ;
#define todow(msg) ;
#else
#define todo(msg) {breakpoint_helper;error1(String("TODO ") + String(msg),__FILE__,__LINE__);}
#define todow(msg) {warn(String("TODO ") + String(msg));}
//#define todow(msg) {breakpoint_helper;warn(str("TODO ") + msg);}
#endif

#if TRACE
//static bool tracing = true;
static bool tracing = false;  // todo
#else
#if MY_WASM and DEBUG
static bool tracing = false; // will further explode HEAP usage!!
// static bool tracing = true;
#elif DEBUG
// static bool tracing = true;
static bool tracing = false;
#else
static bool tracing = true;
// static bool tracing = false;
#endif
#endif

#if RELEASE
// SEND trace args to NIRVANA so they don't get emitted!
#define trace(x)
#define tracef(x, ...)
#else
#define trace(x) if(tracing)print(x);
#define tracef(x, ...) if(tracing)printf(x,__VA_ARGS__)
#endif

typedef unsigned char byte;
typedef const char *chars;
typedef byte *bytes;

// silent ++
#define check_silent(test, ...) if(!(test)){printf("\nNOT PASSING %s\n",#test);backtrace_line()}
//#define check_is(α, β) if((α)!=(β)){printf("%s != %s :\n",#α,#β);print(α);print(" != ");print(β);backtrace_line()}

#define check_is(α, β) if((α)==(β));else{printf("FAIL %s != %s :\n",#α,#β);print(α);print(" != ");print(β);backtrace_exit();}
// #define check_is(α, β) if((α)!=(β)){printf("FAIL %s != %s :\n",#α,#β);print(α);print(" != ");print(β);backtrace_exit();}
#define check_eq(α, β) if((α)!=(β)){printf("FAIL %s != %s :\n",#α,#β);backtrace_exit();}
#define check_eq_or(α, β, ɣ) if((α)!=(β)){printf("%s != %s : ",#α,#β);printf("%s",ɣ);backtrace_exit();}

//#define assert(test) if(!(test)){printf("\nNOT PASSING %s\n%s:%d\n",#test,__FILE__,__LINE__);proc_exit(0);}

#define check(test, ...) { \
  print("CHECKING "); \
  print(#test); \
  debug_line(); \
  if (test) { \
    print("OK check passes: "); \
    print(#test); \
  } else { \
    printf("\nNOT PASSING %s\n", #test); \
    __VA_OPT__(printf("Message: %s\n", __VA_ARGS__);) \
    backtrace_exit(); \
  } \
}

#define check_old(test, ...) {print("CHECKING ");print(#test);debug_line(); \
  if(test){print("OK check passes: ");print(#test);}else{printf("\nNOT PASSING %s\n%s\n",#test);backtrace_exit()}}

#include "String.h" // AFTER defines!
#include "smart_types.h"

#if not WEBAPP

[[noreturn]]
#endif
extern void error1(chars message, chars file = 0, int line = 0);

extern void info(chars);

extern void warn(chars);

extern void warning(chars);

// use host fetch if available, else c++ fetch (curl)
extern chars fetch(chars url);

#if not CURL // and not MY_WASM

inline extern chars fetch(chars url) { return "fetch/curl not available"; } // only 800 bytes via -lcurl !
#endif

int fileSize(char const *file);

chars concat(chars a, chars b);

chars concat(chars a, chars b, uint len_a, uint len_b);

template<class S>
// list HAS TO BE 0 terminated! Dangerous C!! ;)
bool contains(S list[], S match);

bool contains(chars list[], chars match);

unsigned int wordHash(const char *str, int max_chars = 20);

char *readFile(chars filename, int *size_out = 0); // 0 = err

//bool similar(float a, float b);
bool similar(double a, double b);

void lowerCase(char *string, int i);

double mod_d(double x, double y);
double powdi(double a, unsigned int b);

static double pi =
        3.141592653589793238462643383279502884197169399375105820974944592307816406286208998628034825342117067982148086513282306647093844609550582231725359408128481117450284102701938521105559644622948954930381964428810975665933446128475648233786783165271201909145648566923460348610454326648213393607260249141273724587006606315588174881520920962829254091715364367892590360011330530548820466521384146951941511609433057270365759591953092186117381932611793105118548074462379962749567351885752724891227938183011949129833673362440656643086021394946395224737190702179860943702770539217176293176752384674818467669405132000568127145263560827785771342757789609173637178721468440901224953430146549585371050792279689258923542019956112129021960864034418159813629774771309960518707211349999998372978049951059731732816096318595024459455346908302642522308253344685035261931188171010003137838752886587533208381420617177669147303598253490428755468731159562863882353787593751957781857780532171226806613001927876611195909216420198938095257201065485863278865936153381827968230301952035301852968995773622599413891249721775283479131515574857242454150695950829533116861727855889075098381754637464939319255060400927701671139009848824012858361603563707660104710181942955596198946767837449448255379774726847104047534646208046684259069491293313677028989152104752162056966024058038150193511253382430035587640247496473263914199272604269922796782354781636009341721641219924586315030286182974555706749838505494588586926995690927210797509302955321165344987;
static double e =
        2.718281828459045235360287471352662497757247093699959574966967627724076630353547594571382178525166427427466391932003059921817413596629043572900334295260595630738132328627943490763233829880753195251019011573834187930702154089149934884167509244761460668082264800168477411853742345442437107539077744992069551702761838606261331384583000752044933826560297606737113200709328709127443747047230696977209310141692836819025515108657463772111252389784425056953696770785449969967946864454905987931636889230098793127736178215424999229576351482208269895193668033182528869398496465105820939239829488793320362509443117301238197068416140397019837679320683282376464804295311802328782509819455815301756717361332069811250996181881593041690351598888519345807273866738589422879228499892086805825749279610484198444363463244968487560233624827041978623209002160990235304369941849146314093431738143640546253152096183690888707016768396424378140592714563549061303107208510383750510115747704171898610687396965521267154688957035035402123407849819334321068170121005627880235193033224745015853904730419957777093503660416997329725088687696640355570716226844716256079882651787134195124665201030592123667719432527867539855894489697096409754591856956380236370162112100130457037978681033709001941614020518609179729084169275588796225137863656439121201039712282082418729605673154981689957736225994138912497217752834791315155748572424541506959508295331168617278558890750983817546374649393192550604009277016711390098488240128583616035;

//    pi = 3.1415926535896689; // ⚠ todo ⚠️ "memory access out of bounds" WHY CAN'T WE SET A GLOBAL? mut?


bytes concat(bytes a, bytes b, int len_a, int len_b);

bytes concat(bytes a, byte b, int len);

bytes concat(char section, bytes a, int len_a);

chars concat(chars a, chars b);
// chars concat_chars(chars a, chars b);

//inline int abs_i(int x) noexcept;
//inline int64 abs_l(int64 x) noexcept;
//inline float abs_f(float x) noexcept;

//float ln(float y);
//float log(float y,float base=10);
//#undef log2
//#undef log10
//float log2(float y)noexcept;
//float log10(float y)noexcept;
//double sin(double x);
//double cos(double x);
//double pow(double x, double y);
//double floor(double x);

void warn(String warning);

bool fileExists(String filename);

String findFile(String filename, String current_dir); // empty() if not found
//char *findFile(char* filename);// 0 if not found
String load(String file);

bool isSmartPointer(int64 d);

//Node smartValue(int64 smartPointer);
extern "C" Node *smartNode(smart_pointer_64 smartPointer64);

int64 file_last_modified(char *file);

String extractFuncName(const String &fun);

String extractPath(String file);

bool isDir(const char *name);

#define maxi(a, b) a<b?b:a

//#define max(a, b) a<b?b:a too many conflicts with math.h common.h …
//inline just hides export name in wasm, no other optimization todo: maybe … somehow? use maxi! don't care!
static bool max(int64 a, int64 b) {
    return a < b ? b : a;
}

typedef String Path;

#define quit(α) printf("quit %s",#α);breakpoint_helper;proc_exit(42);

//#include <cstdlib> // exit
//static void quit(){
//    breakpoint_helper;
//    exit(42);
//}
template<class S>
class List;

[[maybe_unused]] List<String> arguments();

//int64 powi(int64 a, unsigned int b);
extern "C" int64 powi(int a, unsigned int b);

//extern long double powi(long double, long double);

char *base64_encode(const char *data);

bool contains(bytes list, bytes match, int len, int match_len);

// duplicate!
// char *demangle(String &fun); // implemented in wasm_reader.cpp
