export PATH=/opt/wasm/wasi-sdk/bin/:$PATH

# --target=wasm32 
clang -Qn -nostdlib -Wl,--relocatable,--export-all -o lib.wasm lib.c
clang -Qn -nostdlib -Wl,--relocatable -o main.wasm main.c

wasm-ld --entry main lib.wasm main.wasm -o linked.wasm

# $clang adds two parameters (argc, argv) to main by convention:
# wasmer -i main linked.wasm ||  
wasmer -i main linked.wasm  -- 1 2