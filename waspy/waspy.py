from wasmtime import Store, Module, Instance, Func

# Load the WASM binary
# file="module.wasm"
file="../test.wasm"
with open(file, "rb") as f:
    wasm_bytes = f.read()

# Set up the environment
store = Store()
module = Module(store.engine, wasm_bytes)
instance = Instance(store, module, [])

# Call the exported 'main' function
main_func = instance.exports(store)["_start"]
ok = main_func(store)
print(ok)