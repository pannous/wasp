cp ../../cmake-build-wasm-hosted/wasp-hosted.wasm .
cp ../../cmake-build-wasm-hosted/wasp-hosted.wasm.map .
cp ../../cmake-build-wasm-debug/wasp.wasm wasp-debug.wasm
cp ../../cmake-build-wasm-runtime/wasp-runtime.wasm .
cp ../../cmake-build-wasm-release/wasp.wasm . ||
cp ../../cmake-build-wasm-release/wasp.wasm.wasm wasp-release.wasm
echo "runtime .wasm assets updated"