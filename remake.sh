# IF cmake fails to traverse to subdirectories:
cd cmake-build-release/
cmake . && make
cd -
cd cmake-build-debug/
cmake . && make
cd -
cd cmake-build-default/       
cmake . && make
cd -
cd cmake-build-release/       
cmake . && make
cp wasp.wasm ../bin/mac/wasp
cp wasp.wasm ../bin/mac/arm/wasp
cd -
cd cmake-build-wasm/          
cmake . && make
cp wasp.wasm wasp-debug.wasm 
cp wasp.wasm ../bin/wasp-debug.wasm 
cd -
cd cmake-build-wasm-release/  
cmake . && make
cp wasp.wasm wasp-compiler.wasm 
cp wasp.wasm ../bin/wasp-compiler.wasm 
cd -
cd cmake-build-wasm-runtime/
cmake . && make
cp wasp.wasm wasp-runtime.wasm 
cp wasp.wasm ../bin/wasp-runtime.wasm 
cd -
ln -s cmake-build-debug/wasp .


