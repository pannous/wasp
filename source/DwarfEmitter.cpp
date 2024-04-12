//#include "DwarfEmitter.h"
#include "dwarf.h"
#include "Code.h"

//byte OFFSET = 0x0;
//byte OFFSET = 0x18;
//byte OFFSET = 0x28;
//byte OFFSET = 0x30;
//byte OFFSET = 0x38;
//byte OFFSET = 0x40;
byte OFFSET = 0x42;


/* 0x00000639:   DW_TAG_structure_type
                DW_AT_name	("target.Target.Os.LinuxVersionRange")
                DW_AT_byte_size	(0x54)
                DW_AT_alignment	(4)

0x00000640:     DW_TAG_member
                  DW_AT_name	("range")
                  DW_AT_type	(0x000005dc "SemanticVersion.Range")
                  DW_AT_alignment	(4)
                  DW_AT_data_member_location	(0x00)
                  */

//typedef unsigned char byte;
// VALIDATE via
// llvm-dwarfdump -a --verify main.wasm
// llvm-dwarfdump -a --verbose main.wasm

// then DEBUG with
// lldb -- wasmtime run -D debug-info  main.wasm
// settings set plugin.jit-loader.gdb.enable on
// p __vmctx->set()
// breakpoint set --name tttt
// b tttt
// run

// 0x001031b4034 wasp_main:
// 0x001031b40b8 _start ?
uint main_start = 0x44;
uint main_end = 0x5d;

// 0x00105178060 tttt:
uint tttt_start = 0x5e;
uint tttt_end = 0x7e;


// OFFSET 0x0000004a in .debug_line section AHA!!!
List<byte> getDwarf5LineTable() {
	List<byte> dwarf5_bytes = {
			0x00, 0x05, // 5 bytes length of the extended opcode that follows:
			DW_LNE_set_address,
			(byte) (main_start - OFFSET), 0x00, 0x00, 0x00, // start_address
			DW_LNS_set_file, 0x00, // 0
			DW_LNS_advance_line, 6, // implicit main block starts after(!) function tttt
			DW_LNS_copy,
//			0x12, // address += 0, line += 0, is_stmt
//			0x13, // address += 0, line += 1
//			0x20, // address += 1, line += 0
//			0x21, // address += 1, line += 1
//			0x2e, // address += 2, line += 0
//			0x2f, // address += 2, line += 1
//			0x3c, // address += 3, line += 0
//			0x3e, // address += 3, line += 1
//			0x4a, // address += 4, line += 0
//			0x4b, // address += 4, line += 1

//			000044 func[0] <wasp_main>:
			DW_LNS_negate_stmt,
			0x20, // address += 1, line += 0
//			000045: 01 7f                      | local[0] type=i32
			0x2e, // address += 2, line += 0
//			000047: 01 7e                      | local[1] type=i64
			0x2e, // address += 2, line += 0
//			000049: 41 03                      | i32.const 3
			DW_LNS_set_column, 0x01, // 10
			DW_LNS_negate_stmt,
			0x2e, // address += 2, line += 0
			DW_LNS_set_column, 0x02, // 10
			0x2e, // address += 2, line += 0  call 1 <tttt>
			DW_LNS_set_column, 0x03, // 10
			DW_LNS_negate_stmt,
			0x4a, // address += 4, line += 0  nop * 4
			DW_LNS_advance_line, 0x01, // line += 1
			DW_LNS_set_column, 0x01, // 10
			DW_LNS_negate_stmt,
			0x2e, // address += 2, line += 0  i32.const 3
			DW_LNS_set_column, 0x02, // 10
			0x2e, // address += 2, line += 0  call 1 <tttt>
			DW_LNS_set_column, 0x03, // 10
			DW_LNS_negate_stmt,
			0x4a, // address += 4, line += 0  nop * 4
			0x20, // address += 1, line += 0 // i64.extend_i32_s nop
			0x20, // address += 1, line += 0 // nop
			DW_LNS_negate_stmt,
			0x20, // address += 1, line += 0 // return
//			DW_LNS_set_prologue_end,
//			DW_LNS_const_add_pc, // NO operands!
			0x00, 0x01, 0x01, // DW_LNE_end_sequence

			// tttt
			0x00, 0x05, // 5 bytes length of the extended opcode that follows:
			DW_LNE_set_address, (byte) (tttt_start - OFFSET), 0x00, 0x00, 0x00, // 0x0000000000000063
			DW_LNS_set_file, 0x00, // 0
			DW_LNS_advance_line, 1, // line += 1 for // comment
			DW_LNS_copy,
//			00005e func[1] <tttt>:
			DW_LNS_negate_stmt,
			0x20, // address += 1, line += 0
//			00005f: 01 7e                      | local[1] type=i64
			0x2e, // address += 2, line += 0  i32.const 3
//			000061: 01 7e                      | local[2] type=i64
			0x2e, // address += 2, line += 0  i32.const 3
//			000063: 20 00                      | local.get 0 <j>
			DW_LNS_negate_stmt,
			0x2e, // address += 2, line += 0  i32.const 3
//			000065: 41 01                      | i32.const 1
			0x2f, // address += 2, line += 1  i32.const 3
//			000067: 6a                         | i32.add
			0x20, // address += 1, line += 0
//			000068: ac                         | i64.extend_i32_s
			0x20, // address += 1, line += 0
//			000069: 01                         | nop
			0x20, // address += 1, line += 0
//			00006a: 22 01                      | local.tee 1
			0x2f, // address += 2, line += 1
//			00006c: 20 01                      | local.get 1 <x>
			0x2e, // address += 2, line += 0
//			00006e: a7                         | i32.wrap_i64
			0x20, // address += 1, line += 0
//			00006f: 01                         | nop
			0x20, // address += 1, line += 0
//			000070: 0f                         | return
			0x20, // address += 1, line += 0
//			000071: 0b                         | end
			0x20, // address += 1, line += 0
//			DW_LNE_end_sequence,
//			DW_LNS_set_prologue_end,
//			DW_LNS_const_add_pc, // NO operands!
			0x00, 0x01, 0x01 // DW_LNE_end_sequence
	};
	return dwarf5_bytes;
}


List<byte> dwarf4_bytes = {
//		0x00,
		0x05,                   // 5 bytes length of the extended opcode that follows:
		DW_LNE_set_address,
		(byte) (0x4A - OFFSET), 0x00, 0x00, 0x00, // start_address
		DW_LNS_advance_line, 0x13, // line += 19
		DW_LNS_copy, // DW_LNE_end_sequence,
		DW_LNS_set_prologue_end,
		DW_LNS_set_column, 0x02,
//			DW_LNS_advance_pc, 0x0a, // address += 10
		0x3d, // address += 5, line += 1, is_stmt
		DW_LNS_set_column, 0x01,
		0xc9, // address += 13
		DW_LNS_advance_pc, 0x08, // address += 8
		0x00, 0x01, 0x01, 0x00, // DW_LNE_end_sequence + reset

		0x05, // 5 bytes length of the extended opcode that follows:
		DW_LNE_set_address, (byte) (0x63 - OFFSET), 0x00, 0x00, 0x00, // 0x0000000000000063
		DW_LNS_advance_line, 0x17, // line += 23
		DW_LNE_end_sequence,
		DW_LNS_set_column, 0x05,
		DW_LNS_set_prologue_end,
		DW_LNS_set_column, 0x0a, // 10
		DW_LNS_const_add_pc, // NO operands!
		0xbb, // address += 29, line += 1, is_stmt
		DW_LNS_set_column, 0x0b, // 11
		DW_LNS_negate_stmt,
		0x74, // address += 7, line += 0
		DW_LNS_set_column, 0x06,
		0xac, // address += 11
		DW_LNS_set_column, 0x09,
		DW_LNS_negate_stmt,
		0x75, // address += 7, line += 1, is_stmt
//			DW_LNS_advance_pc, 0x02, // address += 2
		DW_LNS_negate_stmt,
//			DW_LNS_set_column, 2,
		0x74, // address += 7, line += 0
		DW_LNS_advance_pc, 0x04, // address += 4
		0x00, 0x01, 0x01 // DW_LNE_end_sequence
};
/*
0x0000000000000099     26      9      1   0             0       0  is_stmt
0x00000000000000a0     26      2      1   0             0       0
0x00000000000000a4     26      2      1   0             0       0  end_sequence
 */

byte DW_OP_WASM_location = 0xED;     // takes 1 byte before DW_OP_stack_value 0x9f
byte DW_OP_WASM_location_int = 0xEE; // takes 4 bytes __stack_pointer
enum {
	DW_OP_WASM_local = 0,
	DW_OP_WASM_global_leb = 1,
	DW_OP_WASM_stack = 2,
	DW_OP_WASM_global_u32 = 3
};
// https://dwarfstd.org/
// https://dwarfstd.org/doc/DWARF4.pdf
// https://dwarfstd.org/doc/DWARF5.pdf

// DWARF fission has advantages in linking and wasm size (https://developer.chrome.com/blog/faster-wasm-debugging) as it
// separates the debug_info section into a separate file leaving a DW_TAG_skeleton_unit in the main wasm file.
// .dwp / .dwo https://github.com/bytecodealliance/wasmtime/pull/8055

/* Language Code ???
 • Use a Placeholder Code: Select a language code that is "close enough" in terms of language features, syntax, or semantics. This is not ideal but can be a temporary workaround. It allows you to use existing tools and infrastructure with minimal changes, although it may cause confusion or misinterpretation of some debugging information.
 • Vendor Extension: DWARF allows for vendor-specific extensions, including the use of language codes in a specific range reserved for vendor-specific use. You can define a unique code for your language within this range. This approach requires coordination with the tools that will consume the DWARF data (debuggers, profilers, etc.) to ensure they correctly interpret the vendor-specific code.
 • DWARF Custom Attributes: Instead of relying solely on the language code, use DWARF's mechanism for custom attributes to provide additional information about the language. This allows you to use a generic language code but still convey specific details about your language through these attributes. *
*/


/*
 * Contents of section Custom: DWARF 5
0000294: 0d2e 6465 6275 675f 6162 6272 6576 0111  ..debug_abbrev..
00002a4: 0125 2513 0503 2572 1710 171b 2511 0155  .%%...%r....%..U
00002b4: 2373 1774 1700 0002 2e01 111b 1206 4018  #s.t..........@.
00002c4: 0325 3a0b 3b0b 2719 4913 3f19 0000 0305  .%:.;.'.I.?.....
00002d4: 0002 1803 253a 0b3b 0b49 1300 0004 3400  ....%:.;.I....4.
00002e4: 0218 0325 3a0b 3b0b 4913 0000 052e 0011  ...%:.;.I.......
00002f4: 1b12 0640 1803 253a 0b3b 0b49 133f 1900  ...@..%:.;.I.?..
0000304: 0006 2400 0325 3e0b 0b0b 0000 00         ..$..%>......
*/
Code emit_dwarf_debug_abbrev() {
	Code code;
	/* Contents of section Custom: DWARF 4
000025d: 0d2e 6465 6275 675f 6162 6272 6576 0111  ..debug_abbrev..
000026d: 0125 0e13 0503 0e10 171b 0e11 0155 1700  .%...........U..
	 .debug_abbrev contents:
Abbrev table for offset: 0x00000000
[1] DW_TAG_compile_unit	DW_CHILDREN_yes
DW_AT_producer	DW_FORM_strp
DW_AT_language	DW_FORM_data2
DW_AT_name	    DW_FORM_strp
DW_AT_stmt_list	DW_FORM_sec_offset
DW_AT_comp_dir	DW_FORM_strp
DW_AT_low_pc	DW_FORM_addr
DW_AT_ranges	DW_FORM_sec_offset

*/
	code += (byte) 0x01; // first abbrev
	code += (byte) 0x11; // DW_TAG_compile_unit
	code += (byte) 0x01; // DW_CHILDREN_yes
	code += (byte) 0x25; // DW_AT_producer :
	code += (byte) 0x0e; // DW_FORM_strp // code += (byte) 0x25; // DW_FORM_strx1 in DWARF 5
	code += (byte) 0x13; // DW_AT_language :
	code += (byte) 0x05; // DW_FORM_data2
	code += (byte) 0x03; // DW_AT_name :
	code += (byte) 0x0e; // DW_FORM_strp // code += (byte) 0x25; // DW_FORM_strx1 in DWARF 5
//code += (byte) 0x72; // DW_AT_str_offsets_base  DWARF 5
//code += (byte) 0x17; // DW_FORM_sec_offset
	code += (byte) 0x10; // DW_AT_stmt_list :
	code += (byte) 0x17; // DW_FORM_sec_offset
	code += (byte) 0x1b; // DW_AT_comp_dir :
	code += (byte) 0x0e; // DW_FORM_strp // code += (byte) 0x25; // DW_FORM_strx1 in DWARF 5
	code += (byte) 0x11; // DW_AT_low_pc :
	code += (byte) 0x01; // DW_FORM_addr
	code += (byte) 0x55; // DW_AT_ranges :
	code += (byte) 0x17; // DW_FORM_sec_offset
	code += (byte) 0x00; // end of abbrev
	code += (byte) 0x00; // end of abbrev


	/* Contents of section Custom: DWARF 4 continued
000027d: 0002 2e01 1101 1206 4018 030e 3a0b 3b0b  ........@...:.;.
000028d: 2719 4913 3f19 0000
*/
	code += (byte) 0x02; // second abbrev
	code += (byte) 0x2e; // DW_TAG_subprogram
	code += (byte) 0x00; // DW_CHILDREN_yes ? no
	code += (byte) 0x11; // DW_AT_low_pc :
	code += (byte) 0x01; // DW_FORM_addr
	code += (byte) 0x12; // DW_AT_high_pc :
	code += (byte) 0x06; // DW_FORM_data4
	code += (byte) 0x40; // DW_AT_frame_base :
	code += (byte) 0x18; // DW_FORM_exprloc
	code += (byte) 0x03; // DW_AT_name :
	code += (byte) 0x0e; // DW_FORM_strp // code += (byte) 0x25; // DW_FORM_strx1 in DWARF 5
	code += (byte) 0x3a; // DW_AT_decl_file :
	code += (byte) 0x0b; // DW_FORM_data1
	code += (byte) 0x3b; // DW_AT_decl_line :
	code += (byte) 0x0b; // DW_FORM_data1
	code += (byte) 0x27; // DW_AT_prototyped :
	code += (byte) 0x19; // DW_FORM_flag_present
	code += (byte) 0x49; // DW_AT_type :
	code += (byte) 0x13; // DW_FORM_ref4
	code += (byte) 0x3f; // DW_AT_external :
	code += (byte) 0x19; // DW_FORM_flag_present
	code += (byte) 0x00; // end of abbrev
	code += (byte) 0x00; // end of abbrev
	/*

   [2] DW_TAG_subprogram	DW_CHILDREN_yes
   DW_AT_low_pc	DW_FORM_addr
   DW_AT_high_pc	DW_FORM_data4
   DW_AT_frame_base	DW_FORM_exprloc
   DW_AT_name	DW_FORM_strp
   DW_AT_decl_file	DW_FORM_data1
   DW_AT_decl_line	DW_FORM_data1
   DW_AT_prototyped	DW_FORM_flag_present
   DW_AT_type	DW_FORM_ref4
   DW_AT_external	DW_FORM_flag_present
	*/


	code += (byte) 0x03; // fifth abbrev moved to 3rd
	code += (byte) 0x2e; // DW_TAG_subprogram
	code += (byte) 0x01; // DW_CHILDREN_yes
	code += (byte) 0x11; // DW_AT_abstract_origin :
	code += (byte) 0x01; // DW_FORM_ref4
	code += (byte) 0x12; // DW_AT_low_pc :
	code += (byte) 0x06; // DW_FORM_data4
	code += (byte) 0x40; // DW_AT_high_pc :
	code += (byte) 0x18; // DW_FORM_exprloc
	code += (byte) 0x03; // DW_AT_name :
	code += (byte) 0x0e; // DW_FORM_strp // code += (byte) 0x25; // DW_FORM_strx1 in DWARF 5
	code += (byte) 0x3a; // DW_AT_decl_file :
	code += (byte) 0x0b; // DW_FORM_data1
	code += (byte) 0x3b; // DW_AT_decl_line :
	code += (byte) 0x0b; // DW_FORM_data1
	code += (byte) 0x49; // DW_AT_type :
	code += (byte) 0x13; // DW_FORM_ref4
	code += (byte) 0x3f; // DW_AT_external :
	code += (byte) 0x19; // DW_FORM_flag_present
	code += (byte) 0x00; // end of abbrev
	code += (byte) 0x00; // end of abbrev
/* [5] DW_TAG_subprogram	DW_CHILDREN_no  VS   [2] DW_TAG_subprogram	DW_CHILDREN_yes & DW_AT_prototyped
	DW_AT_low_pc	DW_FORM_addr
	DW_AT_high_pc	DW_FORM_data4
	DW_AT_frame_base	DW_FORM_exprloc
	DW_AT_name	DW_FORM_strp
	DW_AT_decl_file	DW_FORM_data1
	DW_AT_decl_line	DW_FORM_data1
	DW_AT_type	DW_FORM_ref4
	DW_AT_external	DW_FORM_flag_present
 */

	/* Contents of section Custom: DWARF 4 continued
	 *                          0305 0002 1803 0e3a  '.I.?..........:
   000029d: 0b3b 0b49 1300                             >......
	*/
	code += (byte) 0x04; // third abbrev moved to 4th
	code += (byte) 0x05; // DW_TAG_formal_parameter
	code += (byte) 0x00; // DW_CHILDREN_no
	code += (byte) 0x02; // DW_AT_location :
	code += (byte) 0x18; // DW_FORM_exprloc
	code += (byte) 0x03; // DW_AT_name :
	code += (byte) 0x0e; // DW_FORM_strp // code += (byte) 0x25; // DW_FORM_strx1 in DWARF 5
	code += (byte) 0x3a; // DW_AT_decl_file :
	code += (byte) 0x0b; // DW_FORM_data1
	code += (byte) 0x3b; // DW_AT_decl_line :
	code += (byte) 0x0b; // DW_FORM_data1
	code += (byte) 0x49; // DW_AT_type :
	code += (byte) 0x13; // DW_FORM_ref4
	code += (byte) 0x00; // end of abbrev
	code += (byte) 0x00; // end of abbrev
/*                         0004 3400 0218 030e 3a0b  .;.I....4.....:.
   00002ad: 3b0b 4913 0000 052e 0011 0112 0640 1803  ;.I..........@..
   00002bd: 0e3a 0b3b 0b49 133f 1900 0006 2400 030e  .:.;.I.?....$...
   00002cd: 3e0b 0b0b 0000 00 */
	code += (byte) 0x05; // fourth abbrev
	code += (byte) 0x34; // DW_TAG_variable
	code += (byte) 0x00; // DW_CHILDREN_no
	code += (byte) 0x02; // DW_AT_location :
	code += (byte) 0x18; // DW_FORM_exprloc
	code += (byte) 0x03; // DW_AT_name :
	code += (byte) 0x0e; // DW_FORM_strp // code += (byte) 0x25; // DW_FORM_strx1 in DWARF 5
	code += (byte) 0x3a; // DW_AT_decl_file :
	code += (byte) 0x0b; // DW_FORM_data1
	code += (byte) 0x3b; // DW_AT_decl_line :
	code += (byte) 0x0b; // DW_FORM_data1
	code += (byte) 0x49; // DW_AT_type :
	code += (byte) 0x13; // DW_FORM_ref4
	code += (byte) 0x00; // end of abbrev
	code += (byte) 0x00; // end of abbrev
/*
	 * [4] DW_TAG_variable	DW_CHILDREN_no
   DW_AT_location	DW_FORM_exprloc
   DW_AT_name	DW_FORM_strp
   DW_AT_decl_file	DW_FORM_data1
   DW_AT_decl_line	DW_FORM_data1
   DW_AT_type	DW_FORM_ref4
	 */


	code += (byte) 0x06; // sixth abbrev
	code += (byte) 0x24; // DW_TAG_base_type
	code += (byte) 0x00; // DW_CHILDREN_no
	code += (byte) 0x03; // DW_AT_name :
	code += (byte) 0x0e; // DW_FORM_strp // code += (byte) 0x25; // DW_FORM_strx1 in DWARF 5
	code += (byte) 0x3e; // DW_AT_encoding :
	code += (byte) 0x0b; // DW_FORM_data1
	code += (byte) 0x0b; // DW_AT_byte_size :
	code += (byte) 0x0b; // DW_FORM_data1
	code += (byte) 0x00; // end of abbrev
	code += (byte) 0x00; // end of abbrev
/*
 * [6] DW_TAG_base_type	DW_CHILDREN_no
	DW_AT_name	DW_FORM_strp
	DW_AT_encoding	DW_FORM_data1
	DW_AT_byte_size	DW_FORM_data1
	 */

	code += (byte) 0x00; // final end of abbrev
	return createSection(custom_section, encodeVector(Code(".debug_abbrev") + code));
}


Code emit_dwarf_debug_info() { // DWARF 4
	Code code;
//	0x00000000: Compile Unit: length = 0x00000077, format = DWARF32, version = 0x0004, abbr_offset = 0x0000, addr_size = 0x04 (next unit at 0x0000007b)
	code += (short) 0x04; // format = DWARF32 version = 0x0004
//	code += (short) 0x05; // version = 0x0004 // DW_UT_*` value for this unit is not supported yet
	code += (short) 0x00; // abbr_offset = 0x0000
	code += (short) 0x00;
	code += (byte) 0x04; // addr_size = 0x04 32-bit addresses


	// start of compile unit DW_TAG_compile_unit
	code += (byte) 0x01; // unit_type = DW_UT_compile OR [1] DW_TAG_compile_unit abbreviated as 0x01 OK
	code += (uint) 0x0000005d; //  DW_AT_producer DW_FORM_strp LINK
	code += (short) 0x001d; // 0x00 DW_LANG_C11 value 0x1D OK DW_FORM_data2
	code += (uint) 0x00000012; //  DW_AT_name	    DW_FORM_strp "…/main.c"
	code += (uint) 0x00000000; // DW_AT_stmt_list offset into .debug_line
	code += (uint) 0x00000030; // DW_AT_comp_dir DW_FORM_strp "…/cmake-build-debug-gdb"
	code += (uint) 0x00000000; // DW_AT_low_pc
	code += (uint) 0x00000000; // DW_AT_ranges

	uint base_type_int = 0x26; //	implicit offset ^^ int
	code += (byte) 0x06; // abbreviated type 6 DW_TAG_base_type
	code += (uint) 0x00000007; // DW_AT_name	("int") // DW_FORM_strp .debug_str[0x00000007] = "int"
	code += (byte) 0x05; // DW_AT_encoding: DW_ATE_signed // DW_FORM_data1
	code += (byte) 0x04; // DW_AT_byte_size:	(0x04) // DW_FORM_data1

	uint base_type_int64 = 0x2d; // implicit offset ^^ int64 long
	code += (byte) 0x06; // abbreviated type 6 DW_TAG_base_type
	code += (uint) 40;   // DW_AT_name	("int") // DW_FORM_strp .debug_str[] = "int64"
	code += (byte) DW_ATE_signed; // DW_AT_encoding: DW_ATE_signed // DW_FORM_data1
	code += (byte) 0x08; // DW_AT_byte_size:	(0x04) // DW_FORM_data1

	uint base_type_pointer = 0x34; // implicit offset ^^ 32/64? bit pointer
	code += (byte) 0x06; // abbreviated type 6 DW_TAG_base_type
	code += (uint) 40;   // DW_AT_name	("int") // DW_FORM_strp .debug_str[] = "int64"
	code += (byte) DW_ATE_address; // DW_AT_encoding: DW_ATE_signed // DW_FORM_data1
	code += (byte) 0x04; // DW_AT_byte_size // DW_FORM_data1


	// main
	code += (byte) 0x02; // ( abbreviated type DW_TAG_subprogram [2] ) // main
	code += (uint) main_start - OFFSET;//	DW_AT_low_pc DW_FORM_addr
	code += (uint) main_end -
	        main_start;// DW_AT_high_pc	offset (0x000000c0)  DW_FORM_data4 // 76 0000 00  hex(0xc0 - 0x4a) OK!!!
	code += (byte) 0x07;// length of DW_FORM_exprloc:
	code += (byte) DW_OP_WASM_location;// := 0xED ;; available DWARF extension code 0x0 0x3, DW_OP_stack_value 0x9f OK ) DW_FORM_exprloc
	code += (byte) DW_OP_WASM_global_u32; // 0x03
	code += (uint) 0; // function offset?
	code += (byte) DW_OP_stack_value; // 0x9F
//	code += (uint) 0x02;// DW_AT_name	("main") // DW_FORM_strp todo: wasp_main
	code += (uint) 46;// DW_AT_name	("wasp_main")
	code += (byte) 0x00; //DW_AT_decl_file "main.wasp"	("/Users/me/dev/apps/wasp/main.c") // DW_FORM_data1 means just 1 byte
	code += (byte) 0x14;// DW_AT_decl_line	(20) // 0x0b DW_FORM_data1 means just 1 byte
	code += base_type_int64;//	DW_AT_type	Type of subroutine return, 'address' of the typedef above
//		DW_AT_external	(true) // DW_FORM_flag_present

// 'tttt'
	code += (byte) 0x03; // abbreviated type ;
	code += (uint) tttt_start - OFFSET; // DW_AT_low_pc DW_FORM_addr
	code += (uint) tttt_end - tttt_start; // DW_AT_high_pc offset (0x000000c0)  DW_FORM_data4
	code += (byte) 0x04; // length of DW_FORM_exprloc:
	code += (byte) DW_OP_WASM_location; // := 0xED ;; available DWARF extension code 0x0 0x3, DW_OP_stack_value 0x9f OK ) DW_FORM_exprloc
	code += (byte) 0;    // DW_OP_WASM_local ?  0,1,2 work 3 hit end 4… fail  R_WASM_FUNCTION_OFFSET_I32 ?
	code += (byte) 0x1;// ? //	DW_AT_frame_base [DW_FORM_exprloc]	// 0… 0x7F ok, 0x80 : Hit the end of input before it was expected
	code += (byte) DW_OP_stack_value;//  0x9f vs  DW_OP_WASM_location_int; // 0xEE
//	DW_AT_name [DW_FORM_strp]	( .debug_str[0x0000000b] = "tttt")
	code += (uint) 0x0000000b; // DW_AT_name	("tttt") // DW_FORM_strp
	code += (byte) 0x00; // DW_AT_decl_file[DW_FORM_data1] "main.wasp"	("/Users/me/dev/apps/wasp/main.c") // DW_FORM_data1 means just 1 byte
	code += (byte) 0x00; // DW_AT_decl_line[DW_FORM_data1]	(25)
	code += base_type_int; // DW_AT_type[DW_FORM_ref4]	( "int") // Type of subroutine return
//	DW_AT_external [DW_FORM_flag_present]	(true)

/*
 * To enable the recovery of the values of variables, parameters, statics, etc... of a debuggee program at runtime, DWARF has location descriptions (see 2.6 of [DWARF]). There are four kinds of base, non-composite location description:
    Empty location descriptions (see 2.6.1.1.1 of [DWARF]) are used for optimized-away variables, or data that is otherwise unavailable.
    Memory location descriptions (see 2.6.1.1.2 of [DWARF]) are used when a value is located at some address in memory.
    Register location descriptions (see 2.6.1.1.3 of [DWARF]) are used when a value is located in a register.
    Implicit location descriptions (see 2.6.1.1.4 of [DWARF]) are used when a value does not have any runtime representation, but has a known value anyways.
Each of these location descriptions are applicable to values in WebAssembly, and may be used as they normally are, except for the third: register location descriptions. WebAssembly does not have registers per se. Instead, it has three distinct kinds of virtual registers (globals, locals, and the operand stack) and may use up to 232 - 1 instances of each virtual register.
 */

//	DW_OP_fbreg: in WASM three distinct kinds of virtual registers (globals, locals, and the operand stack)

// j JIT mapped to register $w2
	code += (byte) 0x04; // abbreviated type 3  DW_TAG_formal_parameter; 'j'
	code += (byte) 0x03; // length of DW_FORM_exprloc:
	code += (byte) DW_OP_WASM_location; // := 0xED ;; available DWARF extension code 0x0 0x3, DW_OP_stack_value 0x9f OK ) DW_FORM_exprloc
	code += (byte) DW_OP_WASM_stack; // stack from bottom:0
	code += (byte) 0x0;// ? //	DW_AT_frame_base [DW_FORM_exprloc]	// 0… 0x7F ok, 0x80 : Hit the end of input before it was expected
//	code += (byte) DW_OP_stack_value;//  0x9f vs  DW_OP_WASM_location_int; // 0xEE
//	code += (byte) 0x02; // length of DW_AT_location DW_FORM_exprloc:
//	code += (byte) DW_OP_fbreg; // offset to DW_AT_frame_base:
//	code += (byte) 0x0c; // + 12
	code += (uint) 0x10; // DW_AT_name	("j") // DW_FORM_strp [16]
	code += (byte) 0x00; // DW_AT_decl_file (1) "main.wasp"	("/Users/me/dev/apps/wasp/main.c") // DW_FORM_data1 means just 1 byte
	code += (byte) 24;   // DW_AT_decl_line	(20) DW_FORM_data1 means just 1 byte
	code += base_type_int; // DW_AT_type	(0x00000026 "int")

	// x JIT mapped to register $w5
	code += (byte) 0x05; // abbreviated type 5 DW_TAG_variable
	code += (byte) 0x04; // length of DW_FORM_exprloc:
	code += (byte) DW_OP_regx;
	code += (byte) 0x05; // DW_OP_reg5
	code += (byte) DW_OP_piece;
	code += (byte) 0x04; // 4 bytes

//	code += (byte) 0x01; // length of DW_FORM_exprloc:
//	code += (byte) DW_OP_reg2;
//	code += (byte) 0x03; // length of DW_FORM_exprloc:
//	code += (byte) DW_OP_WASM_location; // := 0xED ;; available DWARF extension code 0x0 0x3, DW_OP_stack_value 0x9f OK ) DW_FORM_exprloc
//	code += (byte) DW_OP_WASM_local;    // DW_AT_frame_base / DW_OP_WASM_global_local ?  0,1,2 work 3 hit end 4… fail
//	code += (byte) 0x1;// ? //	DW_AT_frame_base [DW_FORM_exprloc]	// 0… 0x7F ok, 0x80 : Hit the end of input before it was expected
//	code += (byte) DW_OP_stack_value;//  0x9f vs  DW_OP_WASM_location_int; // 0xEE
//	code += (byte) 0x02; // length of DW_AT_location DW_FORM_exprloc:
//	code += (byte) DW_OP_fbreg; // offset to DW_AT_frame_base:
//	code += (byte) 0x08; // + 12
	code += (uint) 0x00; // DW_AT_name	("x") // DW_FORM_strp
	code += (byte) 0x00; // DW_AT_decl_file (1) "main.wasp"	("/Users/me/dev/apps/wasp/main.c") // DW_FORM_data1 means just 1 byte
//	code += (byte) 0x0d; // DW_AT_decl_line	(13) DW_FORM_data1 means just 1 byte
	code += (byte) 25; // DW_AT_decl_line	(20) DW_FORM_data1 means just 1 byte
	code += base_type_int64; // DW_AT_type	(0x00000026 "int64") ≠ int32 == 0x7F



	// a aaaa
	code += (byte) 0x05; // abbreviated type 4 DW_TAG_variable 'a'
//	code += (byte) 0x01; // length of DW_FORM_exprloc:
//	code += (byte) DW_OP_reg2;
	code += (byte) 0x03; // length of DW_FORM_exprloc:
	code += (byte) DW_OP_WASM_location; // := 0xED ;; available DWARF extension code 0x0 0x3, DW_OP_stack_value 0x9f OK ) DW_FORM_exprloc
	code += (byte) DW_OP_WASM_local;    // DW_AT_frame_base / DW_OP_WASM_global_local ?  0,1,2 work 3 hit end 4… fail
	code += (byte) 0x0;// ? //	DW_AT_frame_base [DW_FORM_exprloc]	// 0… 0x7F ok, 0x80 : Hit the end of input before it was expected
//	code += (byte) DW_OP_stack_value;//  0x9f vs  DW_OP_WASM_location_int; // 0xEE
//	code += (byte) DW_OP_regval_type;
//	code += (byte) DW_AT_type_int; //DW_AT_type_int64; // LEB128
//	code += (byte) 0x02; // length of DW_AT_location DW_FORM_exprloc:
//	code += (byte) DW_OP_fbreg; // offset to DW_AT_frame_base:
//	code += (byte) 0x10; // + 12
	code += (uint) 48; // DW_AT_name	("a") // DW_FORM_strp
	code += (byte) 0x00; // DW_AT_decl_file "main.wasp"
	code += (byte) 24; // DW_AT_decl_line
	code += base_type_int; // DW_AT_type	(0x0000002d "int64") ≠ int32 == 0x7F

	code += (byte) 0x00; // NULL unindent children of DW_TAG_subprogram tttt

	code += (byte) 0x00; // NULL end of compile unit

	Code len = Code(code.length, false);
	return createSection(custom_section, encodeVector(Code(".debug_info") + len + code));
}

List<byte> getDwarf5LineTable();

Code emit_dwarf_debug_line() {

	Code code;
//	prologue += (short) 0x0004; // format = DWARF32 version 4

	code += (short) 0x0005; // format = DWARF32 version 5
	code += (byte) 0x4;// address_size: 4 (DWARF5!)
	code += (byte) 0; // seg_select_size: (DWARF5!)

	Code prolog;
	prolog += (byte) 0x01; // min_inst_length
	prolog += (byte) 0x01; // max_ops_per_inst maximum_operations_per_instruction
	prolog += (byte) 0x01; // default_is_stmt
	prolog += (byte) 0xfb; // line_base = -5
	prolog += (byte) 0x0e; // line_range = 14
	prolog += (byte) 0x0d; // opcode_base = 13
	prolog += (byte) 0x00; // standard_opcode_lengths[DW_LNS_copy] = 0
	prolog += (byte) 0x01; // standard_opcode_lengths[DW_LNS_advance_pc] = 1
	prolog += (byte) 0x01; // standard_opcode_lengths[DW_LNS_advance_line] = 1
	prolog += (byte) 0x01; // standard_opcode_lengths[DW_LNS_set_file] = 1
	prolog += (byte) 0x01; // standard_opcode_lengths[DW_LNS_set_column] = 1
	prolog += (byte) 0x00; // standard_opcode_lengths[DW_LNS_negate_stmt] = 0
	prolog += (byte) 0x00; // standard_opcode_lengths[DW_LNS_set_basic_block] = 0
	prolog += (byte) 0x00; // standard_opcode_lengths[DW_LNS_const_add_pc] = 0
	prolog += (byte) 0x01; // standard_opcode_lengths[DW_LNS_fixed_advance_pc] = 1
	prolog += (byte) 0x00; // standard_opcode_lengths[DW_LNS_set_prologue_end] = 0
	prolog += (byte) 0x00; // standard_opcode_lengths[DW_LNS_set_epilogue_begin] = 0
	prolog += (byte) 0x01; // standard_opcode_lengths[DW_LNS_set_isa] = 1

	prolog += (byte) 0x01; // DWARF 5: dir ?
	prolog += (byte) 0x01; // DWARF 5: file ?

// embedded code

	prolog += (byte) 0x1f; // DW_FORM_line_strp
//	code += (byte) 0x02;  // 2 entries
	prolog += (byte) 0x01;  // 1 entry
	prolog += (uint) 0x00000000; // include_directories[  0] =  .debug_line_str[0x00000000] = "/opt/wasm/c-wasm-debug"
//	code += (uint) 0x00000035; // include_directories[  1] =  .debug_line_str[0x00000035] = "/opt/wasm/c-wasm-debug/cmake-build-debug-gdb"
//	code += (byte) 0x04; // DW_MACRO_end_file ?

//	0401 1f02 0f05 1e81 40   ELUSIVE PREAMBLE
	prolog += (byte) 0x04; // DW_FORM_block4 DW_EH_PE_udata8 DW_UT_skeleton ??
	prolog += (byte) 0x01; // file entry 0

	prolog += (byte) 0x1f; // DW_FORM_line_strp AGAIN LATER!
	prolog += (byte) 0x02;  // 2 entries

	prolog += (byte) 0x0f; // DW_FORM_udata DW_AT_element_list ??
	prolog += (byte) 0x05;
	prolog += (byte) 0x1e; // DW_AT_default_value / DW_FORM_data16
	prolog += (byte) 0x81;
	prolog += (byte) 0x40;

// now the files
// 00005ed: 1f0217 0000 0000    66e6 2fd1 4c40 0588 a9dd  ......f./.L@....
// 000063a: 90da 38f1 5994	  6200 0000
	prolog += (byte) 0x1f; // DW_FORM_line_strp
	prolog += (byte) 0x01;  // 1 entry
	prolog += (uint) 16; // file_names[  0] =  .debug_line_str[0x00000017] = "main.c"
	prolog += (byte) 0x00; // dir_index
	List<byte> md5_checksum = {0x66, 0xe6, 0x2f, 0xd1, 0x4c, 0x40, 0x05, 0x88, 0xa9, 0xdd, 0x90, 0xda, 0x38, 0xf1, 0x59,
	                           0x94};
	prolog += md5_checksum; // 16 bytes
	prolog += (uint) 23 + 3/*wasp*/; // source: .debug_line_str[0x00000062] = <main.c code>

	/* file_names[  0]:
			  name:  .debug_line_str[0x00000017] = "/opt/wasm/c-wasm-debug/main.c"
		 dir_index: 0
	  md5_checksum: 66e62fd14c400588a9dd90da38f15994
			source:  .debug_line_str[0x00000062] = <main.c code>
*/


	// external code
//	code += Code("/opt/wasm/c-wasm-debug", false, true);
//	code += (byte) 0x00;
//	code += Code("main.c", false, true);
//	code += (byte) 0x01; // dir_index
//	code += (uint) 0x00000000; // mod_time + length ?

	code += (uint) prolog.length;;// (uint) 0x35; // prologue_length = 0x35
	code += prolog;

//	code += dwarf4_bytes;//
	code += getDwarf5LineTable();
	Code len = Code(code.length, false);
	return createSection(custom_section, encodeVector(Code(".debug_line") + len + code));
}


/*
	// Call helper to emit dir and file sections.
	d.writeDirFileTables(unit, lsu)

	// capture length at end of file names.
	headerend = lsu.Size()
	unitlen := lsu.Size() - unitstart

	// Output the state machine for each function remaining.
	for _, s := range unit.Textp {
		fnSym := loader.Sym(s)
		_, _, _, lines := d.ldr.GetFuncDwarfAuxSyms(fnSym)

		// Chain the line symbol onto the list.
		if lines != 0 {
			syms = append(syms, lines)
			unitlen += int64(len(d.ldr.Data(lines)))
		}
	}

	if d.linkctxt.HeadType == objabi.Haix {
		addDwsectCUSize(".debug_line", unit.Lib.Pkg, uint64(unitlen))
	}

	if isDwarf64(d.linkctxt) {
		lsu.SetUint(d.arch, unitLengthOffset+4, uint64(unitlen)) // +4 because of 0xFFFFFFFF
		lsu.SetUint(d.arch, headerLengthOffset, uint64(headerend-headerstart))
	} else {
		lsu.SetUint32(d.arch, unitLengthOffset, uint32(unitlen))
		lsu.SetUint32(d.arch, headerLengthOffset, uint32(headerend-headerstart))
	}

	return syms
}*/

Code emit_dwarf_debug_str() {
	Code code;
	List<String> stringList = {"x", "main", "int", "tttt", "j", "/Users/me/wasp/main.c", "int64", "wasp_main", "a", "b",
	                           "/Users/me/wasp/", "a", "bla", "bla", "bla", "bla", "blablablablablablablablablabla"};

	for (String s: stringList) {
		code += Code((chars) s.data, false, true);
	}
	return createSection(custom_section, encodeVector(Code(".debug_str") + code));
};

Code emit_dwarf_debug_ranges() {
	Code code;
	code += (uint) main_start - OFFSET; // Address of tttt() {
	code += (uint) main_end - OFFSET; // Address of tttt() }
	code += (uint) tttt_start - OFFSET; // Address of main() {
	code += (uint) tttt_end - OFFSET; // Address of main() }
	code += (uint) 0x00000000; // End of list
	code += (uint) 0x00000000; // End of list?
	return createSection(custom_section, encodeVector(Code(".debug_ranges") + code));
}

Code emit_dwarf_external_debug_info() {
	Code code;
	return createSection(custom_section, encodeVector(
			Code("external_debug_info") + Code("main.dwo"))); // relative path or URL to main.dwo
}

// source file names and embedded source code
Code emit_dwarf_debug_line_str() {
	Code code;
	List<String> stringList = {"/Users/me/wasp/", "main.c", readFile("main.c")};
//	List<String> stringList = {"/Users/me/wasp/", "main.wasp", readFile("main.wasp")};
	for (String s: stringList) {
		code += Code((chars) s.data, false, true);
	}
	return createSection(custom_section, encodeVector(Code(".debug_line_str") + code));

}


Code emit_dwarf_debug_debug_addr() { // since DWARF 5
	Code code;
//DW_OP_addrx
//		The DW_OP_addrx operation has a single operand that encodes an unsigned LEB128 value, which is a zero-based index into the .debug_addr section, where a machine address is stored. This index is relative to the value of the DW_AT_addr_base attribute of the associated compilation unit.
//		For example, if the DW_AT_addr_base attribute has the value 0x1000, and the operand of the DW_OP_addrx operation has the value 0x01, then the address is 0x1001.
	return createSection(custom_section, encodeVector(Code(".debug_addr") + code));
}

Code emitDwarfSections() {
	Code code;
//	code += emit_dwarf_external_debug_info(); // split separate debug info OR include:
	code += emit_dwarf_debug_info();
	code += emit_dwarf_debug_ranges();
	code += emit_dwarf_debug_abbrev();// kinda struct defines for DW_TAG_compile_unit, DW_TAG_subprogram
	code += emit_dwarf_debug_line();
	code += emit_dwarf_debug_str();
	code += emit_dwarf_debug_line_str(); // source file names and embedded source code
//	code += emit_dwarf_debug_rnglists(); // DWARF 5
//	code.save("dwarf_sections.wasm");
	return code;
}


// Function to encode a single integer using VLQ
String encodeVLQ(uint num) {
	const chars base64_chars =
			"ABCDEFGHIJKLMNOPQRSTUVWXYZ"
			"abcdefghijklmnopqrstuvwxyz"
			"0123456789+/";

	unsigned int n = num;
	if (num < 0) {
		n = (n << 1) | 1; // Set the low bit if negative
	} else {
		n <<= 1; // Just shift left to leave the low bit zero
	}

	String result;
	do {
		unsigned int digit = n & 31; // Take the low 5 bits
		n >>= 5;
		if (n > 0) { // More digits to come
			digit |= 32;
		}
		result += (base64_chars[digit]);
	} while (n > 0);

	return result;
}

struct SourceMapping {
	int wasm_byte_offset, source_file_index, original_line, original_column, name_index = 0;
};

String generateMappings(const List<SourceMapping> &mappings_data) {
	String mappings;
	int last_generated_line = 0;
	int last_generated_column = 0;
	int last_source = 0;
	int last_original_line = 0;
	int last_original_column = 0;

	for (auto &mapping: mappings_data) {

		if (!mappings.empty()) {
			mappings += ",";
		}

		// Delta encoding each field
		mappings += encodeVLQ(mapping.wasm_byte_offset - last_generated_column);
		mappings += encodeVLQ(mapping.source_file_index - last_source);
		mappings += encodeVLQ(mapping.original_line - last_original_line);
		mappings += encodeVLQ(mapping.original_column - last_original_column);
		if (mapping.name_index) { /* index zero = NONE, skip optional name */
			mappings += encodeVLQ(mapping.name_index);
		}

		last_generated_column = mapping.wasm_byte_offset;
		last_source = mapping.source_file_index;
		last_original_line = mapping.original_line;
		last_original_column = mapping.original_column;
	}

	return mappings;
}

[[nodiscard]]
String generateSourceMap(List<String> names, const List<SourceMapping> &mappings_data) {
	String json = R"(
	{"version":3,"sources":["main.wasp"],"names":[%s],"mappings":"%s"};
	)";
	String mappings = generateMappings(mappings_data);
	return json % names.join(",") % mappings;
}

void testSourceMap() {
	List<String> names = {"ø" /* index zero = NONE, skip optional name */, "j", "x", "tttt", "main"};
	List<SourceMapping> mappings = {{0, 0, 1, 0, 0},
	                                {1, 0, 2, 0, 1},
	                                {2, 0, 3, 0, 2},
	                                {3, 0, 4, 0, 3},
	                                {4, 0, 5, 0, 4}};
	let maps = generateSourceMap(names, mappings);
	print(maps);
}