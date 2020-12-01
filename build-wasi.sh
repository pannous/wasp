echo check leaks
# valgrind --leak-check=full --track-origins=yes ./mark

echo build wasm
rm *.bc
rm *.o

export PATH="/opt/wasi-sdk/bin/:$PATH"
# export clang=/opt/wasm/wasi-sdk/bin/clang

# WASM env:
# export PATH="/usr/local/opt/llvm/bin:$PATH"
# export LDFLAGS="-L/usr/local/opt/llvm/lib"
# export CPPFLAGS="-I/usr/local/opt/llvm/include"
#export C_INCLUDE_PATH="/usr/include/:/usr/lib/x86_64-linux-gnu/glib-2.0/include/"
#for x in `ls -d /usr/include/*/`;do export C_INCLUDE_PATH="$C_INCLUDE_PATH:$x";done
export BINARYEN=/opt/wasm/binaryen/
export CPATH=/opt/wasm/wasi-sdk/share/wasi-sysroot/include/

# clang-10: error: cannot specify -o when generating multiple output files
#"String.cpp" "Node.cpp" "WasmHelpers.cpp"

# --target=wasm32-unknown-unknown-wasm -nostartfiles -nostdlib -allow-undefined ,--export-table,--gc-sections

#DIRECT COMPILE: SLOWER!
clang_options="-O2 -DWASM -s  -Wl,--no-entry,--export-all,--allow-undefined"
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
# $clang $clang_options -w String.cpp -o wasp.wasm #Wasp.cpp #--sysroot=/opt/wasm/wasi-sdk/share/wasi-sysroot

# COMPILE DIRECTLY from .cpp files wow WORKS!!
# -nostdlib -march=wasm  
# -lstd_v2 -lCsup -lunwind 

# exceptions.cpp polyfill for throw … in raise(chars)


clang -std=c++2a -fno-builtin -DWASM=1 -DWASI=1 -lm --sysroot=/opt/wasi-sdk/share/wasi-sysroot -w -Wl,--entry=main,--demangle,--allow-undefined String.cpp Node.cpp Wasm_Helpers.cpp Wasp.cpp Angle.cpp exceptions.cpp  wasm_emitter.cpp   -o wasp.wasm || exit


# COMPILE FROM .o files WORKS!!
# /opt/wasm/wasi-sdk-11.0/bin/clang   -std=c++2a -B=/opt/wasm/wasi-sdk-11.0/bin/ -DWASM=1 -nostdlib -march=wasm  -w -v -Xlinker -t -Xlinker --verbose -Wl,--entry=main,--demangle,--allow-undefined -L/usr/local/opt/llvm/lib CMakeFiles/String.cpp.o CMakeFiles/Node.cpp.o CMakeFiles/Wasp.cpp.o CMakeFiles/WasmHelpers.cpp.o  -o wasp.wasm 

#COMPILE VIA llc & wasm-ld
# clang_options="-DWASM  -fvisibility=hidden" # -I /usr/include/ " VS -stdlib=libc++ -nostdinc -nostdlib
# $clang -w -g -emit-llvm -isystem --target=wasm32-unknown-unknown-wasm  -Oz -c $clang_options  "Wasp.cpp" 


wasm_ld_options="--export-table --gc-sections -error-limit=0 --no-check-features --allow-undefined --entry main" #  --undefined=NIL
# llc -march=wasm32 -filetype=obj Wasp.bc -o wasp.o &> /dev/null
# wasm-ld $wasm_ld_options --demangle --allow-undefined wasp.o -o wasp.wasm || exit
# calls wasm-ld under the hood!

# wasmtime "wasp".wasm

# DEBUG:
llvm-dwarfdump wasp.wasm > wasp.dwarf
/opt/wasm/emscripten/tools/wasm-sourcemap.py wasp.wasm --dwarfdump-output=wasp.dwarf -o wasp.map
# lldb -- wasmtime -g wasp.wasm


 # --split-dwarf-file=wasp.dwarf --split-dwarf-output=wasp.map ONLY with ELF
#"--export-all  --flavor wasm
#wasm-ld $wasm_ld_options --demangle --allow-undefined CMakeFiles/wasp.dir/Wasp.cpp.o -o wasp.wasm || exit
wasm2wat wasp.wasm > wasp.wat
wasm-dis wasp.wasm > wasp.wast # wasm2wast
llvm-objdump -h wasp.wasm > wasp.llvm.objdump
llvm-dwarfdump wasp.wasm --all -o wasp.dwarfdump
wasm-objdump -hxd wasp.wasm > wasp.objdump
wasm-dis -sm wasp.wasm.map wasp.wasm -o wasp.dis
wasm-decompile wasp.wasm -o wasp.decompile # WORKS EVEN IF wasm2wat FAILS!! also very readable
# No wasm-compile exists: The format is very low-level, much like Wasm itself, so even though it looks more high level than the .wat format, it wouldn't be any more suitable for general purpose programming.

./wasm-sourcemap.py -o wasp.map wasp.wasm  

# wasmx wasp.wasm  # danger, -O0 fucks up memoryBase !
# wasmer wasp.wasm
wasmtime wasp.wasm


