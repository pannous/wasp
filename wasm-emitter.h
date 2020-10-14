class Code;
class TransformedProgram;
Code& emitter(TransformedProgram* ast);
// https://webassembly.github.io/spec/core/binary/modules.html#sections
enum Section {
	custom = 0,
	type = 1,
	import = 2,
	func = 3,
	table = 4,
	memory_section = 5,
	global = 6,
	exports = 7,
	start = 8,
	element = 9,
	code_section = 10, // 0x0a
	data = 11
};

// https://webassembly.github.io/spec/core/binary/types.html
enum Valtype {
	i32 = 0x7f,
	f32 = 0x7d
};

// https://webassembly.github.io/spec/core/binary/types.html#binary-blocktype
enum Blocktype {
	void_block = 0x40
};

// https://webassembly.github.io/spec/core/binary/instructions.html
enum Opcodes {
//	start = 0x00,
	start_function = 0x00,
//	unreachable = 0x00,
	block = 0x02,
	loop = 0x03,
	br = 0x0c,
	br_if = 0x0d,
	end_block = 0x0b,
	return_block = 0x0f,
	call = 0x10,
	get_local = 0x20,
	set_local = 0x21,
	i32_store_8 = 0x3a,
	i32_auto = 0x41,
	f32_auto = 0x43,
	i32_eqz = 0x45,
	i32_eq = 0x46,
	f32_eq = 0x5b,
	f32_lt = 0x5d,
	f32_gt = 0x5e,
	i32_and = 0x71,
	f32_add = 0x92,
	f32_sub = 0x93,
	f32_mul = 0x94,
	f32_div = 0x95,
	i32_trunc_f32_s = 0xa8
};
//char start_function=0x00;//unreachable strange convention
extern char unreachable;//=0x00;//unreachable strange convention