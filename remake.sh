# IF cmake fails to traverse to subdirectories:
# echo cmake TODO MESSES WITH CLions CMAKE config!!
# changed all cmake . && make || ninja -v => make || ninja -v
cd cmake-build-wasm-runtime/
# cmake . && make
make || ninja -v
cp wasp.wasm wasp-runtime.wasm 
cp wasp.wasm ../bin/wasp-runtime.wasm 
cd -
cd cmake-build-release/
make || ninja -v
cd -
cd cmake-build-debug/
make || ninja -v
cd -
cd cmake-build-default/       
make || ninja -v
cd -
cd cmake-build-release/       
make || ninja -v
cp wasp.wasm ../bin/mac/wasp
cp wasp.wasm ../bin/mac/arm/wasp
cd -
cd cmake-build-wasm/          
make || ninja -v
cp wasp.wasm wasp-debug.wasm 
cp wasp.wasm ../bin/wasp-debug.wasm 
cd -
cd cmake-build-wasm-release/  
make || ninja -v
cp wasp.wasm wasp-compiler.wasm 
cp wasp.wasm ../bin/wasp-compiler.wasm 
cd -
ln -s cmake-build-debug/wasp .


