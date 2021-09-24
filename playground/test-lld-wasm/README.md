# test-lld-wasm
try to merge/concat/link/combine two wasm files via wasm-ld  
`compile.sh`  
`compile-via-wat.sh`  

Maybe it's not a bug, but incorrect usage from our side. We ran into similar problems as this blog post:
<https://iandouglasscott.com/2019/07/18/experimenting-with-webassembly-dynamic-linking-with-clang/>

```
wasm-ld --entry main lib.wasm main.wasm -o linked.wasm
wasm-ld: error: entry symbol not defined (pass --no-entry to supress): main
```

Update: It works
