#undef Function

#ifdef USE_READLINE

#include <readline/history.h> // libreadline-dev
#include <readline/readline.h>
#include <cstdlib>
#include <cstring>

#endif

#include "String.h"
#include "Node.h"
#include "tests.h"

//using namespace std;
// static struct termios stored_settings;

#define elif else if

void showHelpMessage() {
//	print("⚠️  The wasp console currently works by re-executing after each new entry.");
//	print("⚠️  Be careful with destructive or external effects!");
//	print("");
    print("Detailed documentation can be found at https://github.com/pannous/wasp/wiki ");
    print("AVAILABLE COMMANDS:");
    print("help :h or ? ; clean :c");
}

bool file_read_done = false;

void getline(char *buf) {
    if (buf == 0) return; // end
#ifdef USE_READLINE
    int MAXLENGTH = 10000;
    const char *PROMPT = "wasp> ";
    if (!file_read_done) file_read_done = 1 + read_history(".wasp_history");
    char *tmp = readline(PROMPT);
    if (tmp == 0 or strlen(tmp) == 0) {
        return;
    }
//	tmp=fixQuotesAndTrim(tmp);// LATER!
    if (strncmp(tmp, buf, MAXLENGTH) and strlen(tmp) > 0)
        add_history(tmp); // only add new content
    strncpy(buf, tmp, MAXLENGTH);
    buf[MAXLENGTH - 1] = '\0';
    write_history(".wasp_history");
#else
    print("wasp compiled without console");
#endif
}

//bool parse(string* data) {
#ifdef CONSOLE
//static char *lastCommand;
//static char *commandCode;
#endif

// Todo: web version?
[[noreturn]] void console() {
	printf("\nWasp version %s\n", version);
	showHelpMessage();
	char *data = (char *) malloc(10000);
#ifdef signal
	setjmp(try_context); //recovery point
#endif
	debug = false;
	String code;
	while (true) {
		getline(data);
        if (eq(data, "help") or eq(data, ":help") or eq(data, ":h") or eq(data, "?")) {
            showHelpMessage();
        } elif (eq(data, "clear") or eq(data, ":clear") or eq(data, ":c") or eq(data, "\\L")) {
            code = "";
        } elif (eq(data, "clean") or eq(data, ":clean") or eq(data, ":reset") or eq(data, "reset")) {
            code = "";
        } else if (eq(data, "test") or eq(data, ":test") or eq(data, "tests") or eq(data, ":tests") or eq(data, ":t")) {
#ifndef NO_TESTS
            testCurrent();
#endif
        } else {
            code += data;
            code += ";\n";
//		Node &result = parse(code);// safeMode only for web access
            result = eval(code);
            printf("» ");
//	result.interpret().print();
            result.print();
            print("");
        }
    }
}
