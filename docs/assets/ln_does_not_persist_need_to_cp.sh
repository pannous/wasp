cp ../../cmake-build-wasm-hosted/wasp-hosted.wasm .
cp ../../cmake-build-wasm-hosted/wasp-hosted.wasm.map .
# cp ../../cmake-build-wasm-debug/wasp.wasm wasp-debug.wasm # POINTLESS! use wasp-hosted.wasm
cp ../../cmake-build-wasm-runtime/wasp-runtime.wasm .
cp ../../cmake-build-wasm-runtime-debug/wasp-runtime.wasm wasp-runtime-debug.wasm
cp ../../cmake-build-wasm-release/wasp.wasm wasp-release.wasm
echo "runtime .wasm assets updated"