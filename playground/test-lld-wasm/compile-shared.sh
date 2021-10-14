# sudo -S ls
export PATH=/opt/wasm/wasi-sdk/bin/:$PATH

echo whatever “shared” means, it doesnt work yet, BUT it works without “shared”, see ./compile.sh

clang -Qn --target=wasm32 -nostdlib -Wl,--relocatable,--shared,--export-all -o lib.wasm lib.c
clang -Qn --target=wasm32 -nostdlib -Wl,--relocatable,--shared,--export-all -o main.wasm main.c

wasm-ld --whole-archive -y test --export-all --import-memory --shared --no-entry  lib.wasm main.wasm -o linked.wasm
# wasm-objdump -h linked.wasm
# wasm-objdump -x linked.wasm
# wasm2wat --no-check --enable-all --ignore-custom-section-errors  linked.wasm
# --export=main doesn't work because main is somehow renamed to main.1 !
# clang adds two parameters (argc, argv) to main by convention:
# wasmer -i main linked.wasm ||  
wasmer -i main linked.wasm  -- 1 2

# ls -l linked.wasm
# wasmx linked.wasm