# 'echo use cmake + make'
# export C_INCLUDE_PATH="/usr/include/:/usr/lib/x86_64-linux-gnu/glib-2.0/include/"
# file_name=Angle.cpp
# pure=pure
# rem 'for x in `ls -d /usr/include/*/`;do export C_INCLUDE_PATH="$C_INCLUDE_PATH:$x";done'
# clang_options="-Wc++11-extensions -std=c++2a -Wno-main -fvisibility=hidden -I /usr/include/" # -nostdinc -nostdlib
# clang -isystem --target=wasm32-unknown-unknown-wasm -emit-llvm -Oz -c $clang_options "$file_name" -o wasp.bc || exit # ?
# llc -march=wasm32 -filetype=obj wasp.bc -o wasp.o

echo check leaks
# valgrind --leak-check=full --track-origins=yes ./mark

echo build wasm
# WASM env:
export PATH="/usr/local/opt/llvm/bin:$PATH"
export LDFLAGS="-L/usr/local/opt/llvm/lib"
export CPPFLAGS="-I/usr/local/opt/llvm/include"


# gcc -std=c++2a -dM -g -isystem --target=wasm32-unknown-unknown-wasm -emit-llvm -Oz -c -w Angle.cpp
# clang -std=c++2a -dM -g -isystem --target=wasm32-unknown-unknown-wasi -emit-llvm -Oz -c -w Angle.cpp -o angle.wasm


export C_INCLUDE_PATH="/usr/include/:/usr/lib/x86_64-linux-gnu/glib-2.0/include/"
for x in `ls -d /usr/include/*/`;do export C_INCLUDE_PATH="$C_INCLUDE_PATH:$x";done
#  -Wno-main
clang_options="-DWASM -fvisibility=hidden -I /usr/include/ " # -nostdinc -nostdlib needs stdarg.h!
# clang -w -isystem --target=wasm32-unknown-unknown-wasm -emit-llvm -Oz -c $clang_options "String.cpp" -o String.bc 
# clang -w -isystem --target=wasm32-unknown-unknown-wasm -emit-llvm -Oz -c $clang_options "Node.cpp" -o Node.bc 
# clang -w -isystem --target=wasm32-unknown-unknown-wasm -emit-llvm -Oz -c $clang_options "WasmHelpers.cpp" -o WasmHelpers.bc

clang -w -isystem --target=wasm32-unknown-unknown-wasm -emit-llvm -Oz -c $clang_options "String.cpp" "Node.cpp" "WasmHelpers.cpp" "Wasp.cpp" 
# clang -w -isystem --target=wasm32-unknown-unknown-wasm -emit-llvm -Oz -c $clang_options "Wasp.cpp" -o wasp.bc || exit # ?
llc -march=wasm32 -filetype=obj wasp.bc -o wasp.o
wasm_ld_options="--export-table --gc-sections -error-limit=0 --entry main" 
#"--export-all "
# --flavor wasm
wasm-ld $wasm_ld_options --demangle --allow-undefined wasp.o -o wasp.wasm || exit
wasm2wat wasp.wasm > wasp.wat
# wasm2wast wasp.wasm > wasp.wast
wasmx "wasp".wasm  # danger, -O0 fucks up memoryBase !

