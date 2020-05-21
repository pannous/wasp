//
// Created by me on 18.05.20.
//
#define _main_

#include "Wasp.cpp"

int main(int argp, char **argv) {
	register_global_signal_exception_handler();
	try {
		init();
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

