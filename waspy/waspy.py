from wasmtime import Store, Module, Linker, FuncType, Func, ValType, Instance
import struct
import os

store = Store()


def compile(program):
	cmd="wasp compile '"+program+"'"
	# cmd="wasp '"+program+"'" # and RUN!
	ok=os.system(cmd)
	print(ok)


params = {
	"bla": 123,
	"blaf": 123.4,
	"dada": {'a': 1, 'b': 2},
	"test": "hello",
	"name": "world",
}


def puts(x):
	print(readString(x))


def getElementById(id):
	name = readString(id)
	return name  # name as id!


def toString(ref):
	return writeString(params[ref])

def toLong(ref):
		return params[ref]

def toReal(ref):
		return params[ref]

def toNode(ref):
		return params[ref] # TODO !

def readString(address, length=-1):
	if length == -1:
		length += 1
		while memory.read(store, address + length, address + length + 1)[0] and length < 10000:
			length += 1  # 0 terminated string
	# print(f"readString address={address} length={length}")
	return memory.read(store, address, address + length).decode('utf-8')


HEAP_END = 0


def writeString(text):
	global HEAP_END
	text = str(text)
	text_bytes = text.encode('utf-8')
	address = HEAP_END
	memory.write(store, text_bytes, address)
	HEAP_END += len(text_bytes) + 1  # 0-termination
	return address


def concat(a, b):
	result = readString(a) + readString(b)
	# print(f"concat: '{result}'")
	return writeString(result)


string_header_32 = 0x10000000
string_header_32b = 0x40100000 # todo
array_header_32 = 0x40000000
ref_header_32 = 0x60000000
node_header_32 = 0x80000000


def smart_value(data0: int):  # , memory
	type_ = data0 >> 32
	data = int(data0 & 0xFFFFFFFF)

	if type_ == string_header_32 or type_ == (string_header_32 >> 8):
		return readString(data) # todo extract length
	if type_ == string_header_32b:
		return readString(data, length=11)
	if type_ == array_header_32 or type_ == (array_header_32 >> 8):
		return "todo read_array(data, memory)"
	if type_ == 0 and data0 > 0xFFFFFFFF:
		return data0
	if type_ == 0:
		return data
	if type_ == node_header_32 or type_ == (node_header_32 >> 8):
		return "todo Node(data, memory)"
	# if type == 0x406ED999:
	float64 = struct.unpack('>d', struct.pack('>Q', data0))[0]
	return float64
	raise Exception(f"TODO smart_value for type 0x{type_:08X} data {data} 0x{data0:016X}")
	# nod = Node(compiler_exports.smartNode(data0, memory))
	# if nod.kind in {kinds.real, kinds.bool, kinds.long, kinds.codepoint}:
	#     return nod.value()
	# raise Exception(f"TODO emit.wasm values in wasp.wasm for kind {nod.kind} 0x{type_:08X}")

def nop():
	return 0

i32 = ValType.i32()
i64 = ValType.i64()
f64 = ValType.f64()
f32 = ValType.f32()
chars = ValType.i32()
nodep = ValType.i32()
externref = ValType.externref()
env = {
	# "name" : ([param_types],[returns],fun),
	"getenv": ([chars], [chars], nop),
	"fopen": ([chars,i32], [i32], nop), # TODO: use wasi !?
	"fprintf": ([i32,chars,chars], [i32], nop), # TODO: use wasi !?
	"fgetc": ([i32], [i32], nop), # TODO: use wasi !?
	"fclose": ([i32], [i32], nop), # TODO: use wasi !?
	"exit": ([i32], [], nop), # TODO: use wasi !?
	"toString": ([externref], [chars], toString),
	"toNode": ([externref], [nodep], toNode),
	"toLong": ([externref], [i64], toLong),
	"toReal": ([externref], [f64], toReal),
	"getElementById": ([chars], [externref], getElementById),
	"concat": ([chars, chars], [chars], concat),
	"print": ([chars], [], puts)
}

imports = []

linker = Linker(store.engine)
linker.define_wasi()
for key, value in env.items():
	linker.define(store, "env", key, Func(store, FuncType(value[0], value[1]), value[2]))

global memory
def run_wasm(wasm_bytes):
	global memory
	module = Module(store.engine, wasm_bytes)
	# instance = Instance(store, module, imports)
	instance = linker.instantiate(store, module)

	main_func = instance.exports(store)["_start"]
	memory = instance.exports(store)["memory"]
	ok = main_func(store)
	# print(ok)
	print(smart_value(ok))
	return smart_value(ok)

if __name__ == "__main__":
	compile('2. * $blaf')
	# compile('`hello $name`')
	# compile('`hello $bla`')
	# compile('2 * $bla')
	# compile('2 * $blaf')
	# compile('2. * $bla') # "don't know how to convert 123 to f64" todo hack!
	# compile('`hello ${1 + $bla}`')
	# compile('`hello ${1 + $bla}`')
	# compile('`hello $dada`')
	# compile('$dada')
	# compile('"hello" + $name')
	compile('"hello" + $bla')
	# compile('"hello" + "hi"')

	file = "test.wasm" # ^^ compiled
	# file="module.wasm"
	# file = "../test.wasm"
	with open(file, "rb") as f:
		wasm_bytes = f.read()
		run_wasm(wasm_bytes)
