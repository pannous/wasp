echo needs wat2wasm from https://github.com/WebAssembly/wabt
wat2wasm --relocatable lib.wat
wat2wasm --relocatable main.wat
wasm-ld  --verbose --trace -E --entry main lib.wasm main.wasm -o linked.wasm
hexdump -C linked.wasm
echo expect '42' as result:
wasmer -i main linked.wasm