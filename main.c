/////////////// ;)
int tttt(int j) {
	char *x = "abcd";// j + 1;
//	void* y = &x;
	return x == 0;// (int)x;
}
int wasp_main() {
	tttt(3);
	return tttt(7);
}

// wasm-tools addr2line main.wasm 0x50