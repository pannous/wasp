# IF cmake fails to traverse to subdirectories:
# echo cmake TODO MESSES WITH CLions CMAKE config!!
# changed all cmake && make => make
cd cmake-build-release/
make
cd -
cd cmake-build-debug/
make
cd -
cd cmake-build-default/       
make
cd -
cd cmake-build-release/       
make
cp wasp.wasm ../bin/mac/wasp
cp wasp.wasm ../bin/mac/arm/wasp
cd -
cd cmake-build-wasm/          
make
cp wasp.wasm wasp-debug.wasm 
cp wasp.wasm ../bin/wasp-debug.wasm 
cd -
cd cmake-build-wasm-release/  
make
cp wasp.wasm wasp-compiler.wasm 
cp wasp.wasm ../bin/wasp-compiler.wasm 
cd -
cd cmake-build-wasm-runtime/
make
cp wasp.wasm wasp-runtime.wasm 
cp wasp.wasm ../bin/wasp-runtime.wasm 
cd -
ln -s cmake-build-debug/wasp .


