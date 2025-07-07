cp ../../cmake-build-wasm-hosted/wasp-hosted.wasm . || echo "no wasm-hosted"
cp ../../cmake-build-wasm-hosted/wasp-hosted.wasm.map . || echo "no wasm-hosted.map"
# cp ../../cmake-build-wasm-debug/wasp.wasm wasp-debug.wasm # POINTLESS! use wasp-hosted.wasm
cp ../../cmake-build-wasm-runtime/wasp-runtime.wasm . || echo "no wasm-runtime"
cp ../../cmake-build-wasm-runtime-debug/wasp-runtime.wasm wasp-runtime-debug.wasm || echo "no wasm-runtime-debug"
cp ../../cmake-build-wasm-release/wasp.wasm wasp-release.wasm || echo "no wasm-release"
echo "runtime .wasm assets updated"