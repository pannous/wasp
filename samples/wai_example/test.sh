# https://wasmer.io/posts/wasmer-takes-webassembly-libraries-manistream-with-wai
cargo add wai-bindgen-rust
cat calculator.wai
cargo expand
wapm login
cargo wapm
wapm publish
wasmer add --pip wai/tutorial-01
wasmer add --yarn wai/tutorial-01