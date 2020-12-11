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
#"String.cpp" "Node.cpp" "WasmHelpers.cpp"

# -fdebug-pass-arguments -fdebug-pass-structure 
clang_options="-Os -g  --target=wasm32-unknown-unknown--wasm -nostartfiles -nostdlib -Wl,--demangle,--no-entry,--export-all,--export-table,--allow-undefined"
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

# 
$clang $clang_options t.c -o t.wasm #t.cpp #--sysroot=/opt/wasm/swift-wasm/usr/share/wasi-sysroot

# COMPILE DIRECTLY from .cpp files wow WORKS!!
# clang -std=c++2a -DWASM=1 -nostdlib -march=wasm  -w -v -Wl,--entry=main,--demangle,--allow-undefined String.cpp Node.cpp WasmHelpers.cpp t.cpp -o t.wasm || exit


# COMPILE FROM .o files WORKS!!
# /opt/wasm/wasi-sdk-11.0/bin/clang   -std=c++2a -B=/opt/wasm/wasi-sdk-11.0/bin/ -DWASM=1 -nostdlib -march=wasm  -w -v -Xlinker -t -Xlinker --verbose -Wl,--entry=main,--demangle,--allow-undefined -L/usr/local/opt/llvm/lib CMakeFiles/String.cpp.o CMakeFiles/Node.cpp.o CMakeFiles/t.cpp.o CMakeFiles/WasmHelpers.cpp.o  -o t.wasm 

#COMPILE VIA llc & wasm-ld
# clang_options="-DWASM  -fvisibility=hidden" # -I /usr/include/ " VS -stdlib=libc++ -nostdinc -nostdlib
$clang -w -g -emit-llvm -isystem --target=wasm32-unknown-unknown-wasm  -Oz -c $clang_options  "t.c" 
wasm-ld $wasm_ld_options --demangle --allow-undefined t.o -o t.wasm || exit

wasm_ld_options="--export-table --gc-sections -error-limit=0 --no-check-features --allow-undefined --entry main" #  --undefined=NIL
llc -march=wasm32 -filetype=obj t.bc -o t.o &> /dev/null
wasm-ld $wasm_ld_options --demangle --allow-undefined t.o -o t.wasm || exit
# calls wasm-ld under the hood!


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

