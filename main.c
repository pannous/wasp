// /opt/wasi-sdk/bin/clang -g -o main.wasm main.c
// lldb -- wasmtime run -D debug-info  main.wasm
// settings set plugin.jit-loader.gdb.enable on
// p __vmctx->set()
// breakpoint set --name tttt
// b tttt
// run
//The address of the start of instance memory can be found in __vmctx->memory

// Even move PC (- edit & continue) WORKS IN IDE!

// llvm-dwarfdump -a main.wasm
// llvm-dwarfdump -a main.wasm > main.dwarf

// MAYBE in browser:
// https://github.com/jawilk/lldb2wasm
// line length: 16, 1,


int main() {
	tttt(42); // easier to set breakpoint in lldb / gdb because main is also in wasmtime
}

int tttt(int j) {
	int x = j + 1;
	return x * 2;
}
