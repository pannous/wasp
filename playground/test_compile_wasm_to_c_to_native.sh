# https://github.com/WebAssembly/wabt/tree/main/wasm2c

wasm2c test.wasm -o test_generated.c
cc -o wasp_native wasp_wrapper.c test_generated.c wasm-rt-impl.c && ./wasp_native