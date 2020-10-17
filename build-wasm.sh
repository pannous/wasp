echo check leaks
# valgrind --leak-check=full --track-origins=yes ./mark

echo build wasm
rm wasp.wasm
# WASM env:
export PATH="/usr/local/opt/llvm/bin:$PATH"
export LDFLAGS="-L/usr/local/opt/llvm/lib"
export CPPFLAGS="-I/usr/local/opt/llvm/include"
#export C_INCLUDE_PATH="/usr/include/:/usr/lib/x86_64-linux-gnu/glib-2.0/include/"
#for x in `ls -d /usr/include/*/`;do export C_INCLUDE_PATH="$C_INCLUDE_PATH:$x";done

#
clang_options="-DWASM  -fvisibility=hidden" # -I /usr/include/ " VS -stdlib=libc++ -nostdinc -nostdlib
#"String.cpp" "Node.cpp" "WasmHelpers.cpp"
clang -w -g -emit-llvm -isystem --target=wasm32-unknown-unknown-wasm  -Oz -c $clang_options "Wasp.cpp"
llc -march=wasm32 -filetype=obj Wasp.bc -o wasp.o &> /dev/null # --split-dwarf-file=wasp.dwarf --split-dwarf-output=wasp.map ONLY with ELF
#"--export-all  --flavor wasm
wasm_ld_options="--export-table --gc-sections -error-limit=10 --entry main"
#wasm-ld $wasm_ld_options --demangle --allow-undefined CMakeFiles/wasp.dir/Wasp.cpp.o -o wasp.wasm || exit
wasm-ld $wasm_ld_options --demangle --allow-undefined wasp.o -o wasp.wasm || exit
wasm2wat wasp.wasm > wasp.wat
wasm-dis wasp.wasm > wasp.wast # wasm2wast
llvm-objdump -h wasp.wasm > wasp.llvm.objdump
llvm-dwarfdump wasp.wasm --all -o wasp.dwarfdump
wasm-objdump -hxd wasp.wasm > wasp.objdump
wasm-dis -sm wasp.wasm.map wasp.wasm -o wasp.dis
wasm-decompile wasp.wasm -o wasp.decompile # WORKS EVEN IF wasm2wat FAILS!! also very readable
# No wasm-compile exists: The format is very low-level, much like Wasm itself, so even though it looks more high level than the .wat format, it wouldn't be any more suitable for general purpose programming.

wasmx "wasp".wasm  # danger, -O0 fucks up memoryBase !

