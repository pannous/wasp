echo check leaks
# valgrind --leak-check=full --track-origins=yes ./mark

echo build wasm

# WASM env:
# export PATH="/usr/local/opt/llvm/bin:$PATH"
# export LDFLAGS="-L/usr/local/opt/llvm/lib"
# export CPPFLAGS="-I/usr/local/opt/llvm/include"
#export C_INCLUDE_PATH="/usr/include/:/usr/lib/x86_64-linux-gnu/glib-2.0/include/"
#for x in `ls -d /usr/include/*/`;do export C_INCLUDE_PATH="$C_INCLUDE_PATH:$x";done
clang=/opt/wasm/wasi-sdk-11.0/bin/clang
export BINARYEN=/opt/wasm/binaryen/
export CPATH=/opt/wasm/swift-wasm/usr/share/wasi-sysroot/include/

# clang-10: error: cannot specify -o when generating multiple output files

# -fdebug-pass-arguments -fdebug-pass-structure 
# --target=wasm32-unknown-emscripten-wasm
# -Wl…,… == linker flags for wasm-ld
# --relocatable
# --print-gc-sections
# --demangle
# --export-all
  # --mllvm <value>        Options to pass to LLVM
  #   --import-memory        Import memory from the environment
  # --import-table         Import function table from the environment
  # --initial-memory=<value>                         Initial size of the linear memory
  #   --features=<value>
clang_options="-Os -g  --target=wasm32-unknown-unknown--wasm -nostartfiles -nostdlib"
# https://prereleases.llvm.org/10.0.0/rc6/tools/clang/docs/ClangCommandLineReference.html
wasm_ld_options="--entry main --export-table --gc-sections -error-limit=0 --no-check-features --allow-undefined " 

$clang $clang_options t.c -o t.o
wasm-ld --entry=main $wasm_ld_options --demangle t.o -o t.wasm || exit

# DEBUG:
llvm-dwarfdump t.wasm > t.dwarf

# --split-dwarf-file=t.dwarf --split-dwarf-output=t.map ONLY with ELF
#"--export-all  --flavor wasm
#wasm-ld $wasm_ld_options --demangle --allow-undefined CMakeFiles/t.dir/t.cpp.o -o t.wasm || exit
wasm2wat t.wasm > t.auto.wat
wasm-dis t.wasm > t.wast # wasm2wast
llvm-objdump -h t.wasm > t.llvm.objdump
llvm-dwarfdump t.wasm --all -o t.dwarfdump
wasm-objdump -hxd t.wasm > t.objdump
wasm-dis -sm t.wasm.map t.wasm -o t.dis
wasm-decompile t.wasm -o t.decompile # WORKS EVEN IF wasm2wat FAILS!! also very readable
# No wasm-compile exists: The format is very low-level, much like Wasm itself, so even though it looks more high level than the .wat format, it wouldn't be any more suitable for general purpose programming.


#  NO LONGER NECESSARY: clang -g sould work
/opt/wasm/emscripten/tools/wasm-sourcemap.py t.wasm --dwarfdump-output=t.dwarf -o t.map
# ./wasm-sourcemap.py -o t.map t.wasm  
# lldb -- wasmtime -g t.wasm

wasmx t.wasm  # danger, -O0 fucks up memoryBase !

