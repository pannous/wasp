//
// Created by me on 18.05.20.
//
#define _main_

#include "mark.cpp"

//
//It has clean syntax with FULLY-TYPE data model (like JSON or even better)
//It is generic and EXTENSIBLE (like XML or even better)
//It has built-in MIXED CONTENT support (like HTML5 or even better)
//It supportsHIGH-ORDER COMPOSITION (like S-expressions or even better)

int main(int argp, char **argv) {
	register_global_signal_exception_handler();
	try {
		auto s = "hello world"_;
		init();
//		test();
		testCurrent();
		return 42;
	} catch (chars err) {
		printf("\nERROR\n");
		printf("%s", err);
	} catch (String err) {
		printf("\nERROR\n");
		printf("%s", err.data);
	} catch (SyntaxError *err) {
		printf("\nERROR\n");
		printf("%s", err->data);
	}
	usleep(1000000000);
	return -1;
}

