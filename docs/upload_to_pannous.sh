increase-package-version.py wasp.js 
git -c color.ui=false commit -a -m '' --allow-empty-message
git push
ssh pannous.com "cd wasp && git pull"
scp ../cmake-build-wasm-hosted/wasp-hosted.wasm pannous.com:~/wasp/docs/ &
scp ../cmake-build-wasm-runtime/wasp-runtime.wasm pannous.com:~/wasp/docs/ &
scp ../cmake-build-wasm-debug/wasp.wasm pannous.com:~/wasp/docs/wasp-debug.wasm &