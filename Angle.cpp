//
// Created by me on 18.05.20.
//
#define _main_
unsigned long __stack_chk_guard= 0xBAAAAAAD;
#include "Wasp.cpp"
int main(int argp, char **argv) {
#ifndef WASM
	register_global_signal_exception_handler();
#endif
	try {
		init();
		throw "HhhU";
//		testCurrent();
		return 42;
	} catch (chars err) {
		print("\nERROR\n");
		print("%s", err);
	} catch (String err) {
		print("\nERROR\n");
		print("%s", err.data);
	} catch (SyntaxError *err) {
		print("\nERROR\n");
		print("%s", err->data);
	}
#ifndef WASM
	usleep(1000000000);
#endif
	return -1;
}
int start(){ // for wasm-ld
	main(0,0);
}
int _start(){ // for wasm-ld
	main(0,0);
}

