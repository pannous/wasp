// todo: move smartpointer stuff here
typedef int usize;
typedef int u32;
struct AssemblyScriptData {
	usize mmInfo;// 	-20 	usize 	Memory manager info
	usize gcInfo;// 	-16 	usize 	Garbage collector info
	usize gcInfo2;// 	-12 	usize 	Garbage collector info
	u32 rtId;// 	-8 	u32 	Unique id of the concrete class
	u32 rtSize;// 	-4 	u32 	Size of the data following the header
	void *payload;// 0 	Payload starts here
} AssemblyScriptData;

struct WaspData { // ⚠️ the first fiels are shared among all Wasp classes, the payload only includes extra data!!
	int header; // internal type node, string, list/array todo: merge with type!?
	int type;   // type of node / array ≈ generics
//	int refcount; // todo? or via wasm 2.0 ? see  Memory manager / Garbage collector info
//	int size; todo: add to all classes 1. to align 4*8 bytes 2. to allow extra fields after 'array' 3. total size or extra fields??
	int length; // of array types, 0 otherwise
	void *payload;// 0 	Payload starts here  e.g. chars of string, items of list, children of node
//	void *extra; // extra fields in classes are only inferable through reflection/inspection. The size field of WaspData sets the limit
} WaspData;

struct RustData {
};
//https://faultlore.com/blah/rust-layouts-and-abis/

// https://github.com/WebAssembly/component-model/blob/main/design/mvp/CanonicalABI.md
struct Canonical_ABI_string {
	int length;
	int pointer;
} Canonical_ABI_string;
