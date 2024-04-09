//#include "DwarfEmitter.h"
#include "dwarf.h"
typedef unsigned char byte;

//byte OFFSET = 0x4D; // The end of an address range must not be before the beginning
//byte OFFSET = 0x4C; // XX why not -''-
//byte OFFSET = 0x4B; // XX why not -''-
//byte OFFSET = 0x4A; // XX
//byte OFFSET = 0x49; // XX

// 7 bytes valid region:
//byte OFFSET = 0x48; // Breakpoint reached: tttt  Stack: tttt main.c:24 YAY
byte OFFSET = 0x47; // in the range of 0x7b … 0x7f _start
//byte OFFSET = 0x46;
//byte OFFSET = 0x41;

//byte OFFSET = 0x40; // XX
//byte OFFSET = 0x44 - 0x19;
//byte OFFSET = 0x3A; // XX
//byte OFFSET = 0x36; // takes tttt for main!!! NOT OK ;)
//byte OFFSET = 0x30; // -''-
//byte OFFSET = 0x2F; // -''-
//byte OFFSET = 0x2E; // last 'good' offset
//byte OFFSET = 0x2B; // XX
//byte OFFSET = 0x20; // XX
//byte OFFSET = 0x10; // XX
//byte OFFSET = 0;
//byte OFFSET = -0x10;
//byte OFFSET = -0x20;


// VALIDATE via
// llvm-dwarfdump -a main.wasm

// then DEBUG with
// lldb -- wasmtime run -D debug-info  main.wasm
// settings set plugin.jit-loader.gdb.enable on
// p __vmctx->set()
// breakpoint set --name tttt
// b tttt
// run

byte DW_OP_WASM_location = 0xED;     // takes 1 byte before DW_OP_stack_value 0x9f
byte DW_OP_WASM_location_int = 0xEE; // takes 4 bytes __stack_pointer
enum {
	DW_OP_WASM_global_local = 0,
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
#include "Code.h"

Code emit_dwarf_debug_info() { // DWARF 4
	Code code;
//	0x00000000: Compile Unit: length = 0x00000077, format = DWARF32, version = 0x0004, abbr_offset = 0x0000, addr_size = 0x04 (next unit at 0x0000007b)
	code += (byte) 0x00;
	code += (byte) 0x00; // format = DWARF32
	code += (byte) 0x00;
	code += (byte) 0x04; // version = 0x0004
	code += (byte) 0x00;
	code += (byte) 0x00; // abbr_offset = 0x0000
	code += (byte) 0x00;
	code += (byte) 0x00;
	code += (byte) 0x00; // ??
	code += (byte) 0x04; // addr_size = 0x04
	// start of compile unit DW_TAG_compile_unit
// NUMBERS are little endian 7300 0000 = 0x00000073
//	00001ac: 0b2e 6465 6275 675f 696e 666f 7700 0000  ..debug_infow...
//	00001bc: 0400 0000 0000 04 // offset 0b = 11 bytes from start of compile unit
//	00001bc:                  01 5d00 0000 1d00 1200  ........].......
//	00001cc: 0000 0000 0000 3000 0000 0000 0000 0000  ......0.........
//	00001dc: 0000
//	00001dc:      024a 0000 0076 0000 0004 ed00 039f  ...J...v........
//	00001ec: 0200 0000 010b 7300 0000 0302 910c 1000  ......s.........
//	00001fc: 0000 010b 7300 0000 0402 9108 0000 0000  ....s...........
//	000020c: 010d 7300 0000 0005 c100 0000 5100 0000  ..s.........Q...
//	000021c: 04ed 0002 9f0b 0000 0001 1373 0000 0006  ...........s....
//	000022c: 0700 0000 0504 00

	code += (byte) 0x01; // unit_type = DW_UT_compile OR [1] DW_TAG_compile_unit abbreviated as 0x01 OK
	code += (uint) 0x0000005d; //  DW_AT_producer DW_FORM_strp LINK
	code += (short) 0x001d; // 0x00 DW_LANG_C11 value 0x1D OK DW_FORM_data2
	code += (uint) 0x00000012; //  DW_AT_name	    DW_FORM_strp "…/main.c"
	code += (uint) 0x00000000; // DW_AT_stmt_list
	code += (uint) 0x00000030; // DW_AT_comp_dir DW_FORM_strp "…/cmake-build-debug-gdb"
	code += (uint) 0x00000000; // DW_AT_low_pc
	code += (uint) 0x00000000; // DW_AT_ranges

	/*
	  DW_TAG_compile_unit
			  	DW_AT_producer	DW_FORM_strp	("clang version 16.0.0 (https://github.com/llvm/llvm-project 434575c026c81319b393f64047025b54e69e24c2)")
			  	DW_AT_language	DW_FORM_data2	(DW_LANG_C11)
			  	DW_AT_name	    DW_FORM_strp	("/Users/me/dev/apps/wasp/main.c")
			  	DW_AT_stmt_list	DW_FORM_sec_offset	(0x00000000)
			  	DW_AT_comp_dir	DW_FORM_strp	("/Users/me/dev/apps/wasp/cmake-build-debug-gdb")
			  	DW_AT_low_pc	DW_FORM_addr	(0x00000000)
			  	DW_AT_ranges	DW_FORM_sec_offset	(0x00000000
				 [0x0000004a, 0x000000c0)
				 [0x000000c1, 0x00000112))
				 */

//	00001dc:      024a 0000 0076 0000 0004 ed00 039f  ...J...v........
//	00001ec: 0200 0000 010b 7300 0000 0302 910c 1000  ......s.........
//	00001fc: 0000 010b 7300 0000 0402 9108 0000 0000  ....s...........
	/*
	 * 0x00000026:   */
	code += (byte) 0x02; // ( abbreviated type DW_TAG_subprogram [2] ) // main
	code += (uint) 0x0000004a - OFFSET;//	DW_AT_low_pc DW_FORM_addr
	code += (uint) 0x62 - 0x4a;// DW_AT_high_pc	offset (0x000000c0)  DW_FORM_data4 // 76 0000 00  hex(0xc0 - 0x4a) OK!!!

	code += (byte) 0x04;// length of DW_FORM_exprloc:
	code += (byte) DW_OP_WASM_location;// := 0xED ;; available DWARF extension code 0x0 0x3, DW_OP_stack_value 0x9f OK ) DW_FORM_exprloc
	code += (byte) 0; // DW_AT_frame_base / DW_OP_WASM_global_local ?
	code += (byte) 3; // NO EFFECT? // func#3?  TI_GLOBAL_RELOC / / DW_OP_WASM_global_u32 / 1 byte __stack_pointer ?  main/$_start=0x02 OK
	code += (byte) DW_OP_stack_value; // 0x9F

	code += (uint) 0x02;// DW_AT_name	("main") // DW_FORM_strp
	code += (byte) 0x01; //DW_AT_decl_file	("/Users/me/dev/apps/wasp/main.c") // DW_FORM_data1 means just 1 byte
	code += (byte) 0x14;// DW_AT_decl_line	(20) // 0x0b DW_FORM_data1 means just 1 byte
//	code += (byte) 0x0b;// DW_AT_decl_line	(11) // 0x0b DW_FORM_data1 means just 1 byte
//		DW_AT_prototyped	(true) // DW_FORM_flag_present NOT THERE / 0x01 comes BEFORE 0x0b !!!
	code += (uint) 0x00000073;//	DW_AT_type	( "int")
//		DW_AT_external	(true) // DW_FORM_flag_present

/*
[3] DW_TAG_formal_parameter	DW_CHILDREN_no
	DW_AT_location	DW_FORM_exprloc
	DW_AT_name	DW_FORM_strp
	DW_AT_decl_file	DW_FORM_data1
	DW_AT_decl_line	DW_FORM_data1
	DW_AT_type	DW_FORM_ref4
								  0302 910c 1000  ......s.........
00001fc: 0000 010b 7300 0000
                             0402 9108 0000 0000  ....s...........
000020c: 010d 7300 0000 0005 c100 0000 5100 0000  ..s.........Q...
000021c: 04ed 0002 9f0b 0000 0001 1373 0000 0006  ...........s....
000022c: 0700 0000 0504 00

 0x0000003e:     DW_TAG_formal_parameter
                  DW_AT_location	(DW_OP_fbreg  +12 0x91 0x0c)
                  DW_AT_name	("j")
                  DW_AT_decl_file	("/Users/me/dev/apps/wasp/main.c")
                  DW_AT_decl_line	(11)
                  DW_AT_type	(0x00000073 "int")
*/
//code += (byte)DW_TAG_formal_parameter;
	code += (byte) 0x03; // abbreviated type 3
	code += (byte) 0x02; // length of DW_AT_location DW_FORM_exprloc:
	code += (byte) DW_OP_fbreg;
	code += (byte) 0x0c; // + 12
	code += (uint) 0x10; // DW_AT_name	("j") // DW_FORM_strp
	code += (byte) 0x01; // DW_AT_decl_file (1)	("/Users/me/dev/apps/wasp/main.c") // DW_FORM_data1 means just 1 byte
//	code += (byte) 0x0b; // DW_AT_decl_line	(11) DW_FORM_data1 means just 1 byte
	code += (byte) 0x13; // DW_AT_decl_line	(20) DW_FORM_data1 means just 1 byte
	code += (uint) 0x73; // DW_AT_type	(0x00000073 "int")

/*
[4] DW_TAG_variable	DW_CHILDREN_no
	DW_AT_location	DW_FORM_exprloc
	DW_AT_name	DW_FORM_strp
	DW_AT_decl_file	DW_FORM_data1
	DW_AT_decl_line	DW_FORM_data1
	DW_AT_type	DW_FORM_ref4

 0x0000004c:     DW_TAG_variable
      DW_AT_location	(DW_OP_fbreg +8)
      DW_AT_name	("x")
      DW_AT_decl_file	("/Users/me/dev/apps/wasp/main.c")
      DW_AT_decl_line	(13)
      DW_AT_type	(0x00000073 "int")
 */
	code += (byte) 0x04; // abbreviated type 4
	code += (byte) 0x02; // length of DW_AT_location DW_FORM_exprloc:
	code += (byte) DW_OP_fbreg;
	code += (byte) 0x08; // + 8
	code += (uint) 0x00; // DW_AT_name	("x") // DW_FORM_strp
	code += (byte) 0x01; // DW_AT_decl_file (1)	("/Users/me/dev/apps/wasp/main.c") // DW_FORM_data1 means just 1 byte
//	code += (byte) 0x0d; // DW_AT_decl_line	(13) DW_FORM_data1 means just 1 byte
	code += (byte) 0x13; // DW_AT_decl_line	(20) DW_FORM_data1 means just 1 byte
	code += (uint) 0x73; // DW_AT_type	(0x00000073 "int") ≠ int32 == 0x7F

	code += (byte) 0x00; // NULL why?



//	[5] DW_TAG_subprogram	DW_CHILDREN_no
//	DW_AT_low_pc	DW_FORM_addr
//	DW_AT_high_pc	DW_FORM_data4
//	DW_AT_frame_base	DW_FORM_exprloc
//	DW_AT_name	DW_FORM_strp
//	DW_AT_decl_file	DW_FORM_data1
//	DW_AT_decl_line	DW_FORM_data1
//	DW_AT_type	DW_FORM_ref4
//	DW_AT_external	DW_FORM_flag_present

	List<byte> others = {0x05, (byte) (0x63 - OFFSET), 0x00, 0x00, 0x00, (byte) (0xa4 - 0x63), 0x00, 0x00, 0x00,
	                     0x04, 0xed, 0x00, 0x02, 0x9f, 0x0b, 0x00, 0x00, 0x00, 0x01, 0x13, 0x73, 0x00, 0x00, 0x00, 0x06,
	                     0x07, 0x00, 0x00, 0x00, 0x05, 0x04, 0x00};
	code += others;
	/*

	[5] DW_TAG_subprogram	DW_CHILDREN_no
	DW_AT_low_pc	DW_FORM_addr
	DW_AT_high_pc	DW_FORM_data4
	DW_AT_frame_base	DW_FORM_exprloc
	DW_AT_name	DW_FORM_strp
	DW_AT_decl_file	DW_FORM_data1
	DW_AT_decl_line	DW_FORM_data1
	DW_AT_type	DW_FORM_ref4
	DW_AT_external	DW_FORM_flag_present

[6] DW_TAG_base_type	DW_CHILDREN_no
	DW_AT_name	DW_FORM_strp
	DW_AT_encoding	DW_FORM_data1
	DW_AT_byte_size	DW_FORM_data1
 */


//	code = encodeVector(code);
	Code len = Code(code.length - 3);// why -3 ??
	// todo maybe add 0x00000000 to the end of code
	return createSection(custom_section, encodeVector(Code(".debug_info") + len + code));
}


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
	code += (byte) 0x01; // DW_CHILDREN_yes
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
	code += (byte) 0x00; // end of ?
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

	/* Contents of section Custom: DWARF 4 continued
	 *                          0305 0002 1803 0e3a  '.I.?..........:
   000029d: 0b3b 0b49 1300                             >......
	*/
	code += (byte) 0x03; // third abbrev
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
	code += (byte) 0x04; // fourth abbrev
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
	code += (byte) 0x05; // fifth abbrev
	code += (byte) 0x2e; // DW_TAG_subprogram
	code += (byte) 0x00; // DW_CHILDREN_no
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

Code emit_dwarf_debug_line() {
	Code code;
//	let start_address = 0x0000004A;

	code += (uint) 0x00000074; // length as little endian
	code += (short) 0x0004; // format = DWARF32 version 4
	code += (uint) 0x35; // prologue_length = 0x35
	code += (byte) 0x01; // min_inst_length
	code += (byte) 0x01; // max_ops_per_inst maximum_operations_per_instruction
	code += (byte) 0x01; // default_is_stmt
	code += (byte) 0xfb; // line_base = -5
	code += (byte) 0x0e; // line_range = 14
	code += (byte) 0x0d; // opcode_base = 13
	code += (byte) 0x00; // standard_opcode_lengths[DW_LNS_copy] = 0
	code += (byte) 0x01; // standard_opcode_lengths[DW_LNS_advance_pc] = 1
	code += (byte) 0x01; // standard_opcode_lengths[DW_LNS_advance_line] = 1
	code += (byte) 0x01; // standard_opcode_lengths[DW_LNS_set_file] = 1
	code += (byte) 0x01; // standard_opcode_lengths[DW_LNS_set_column] = 1
	code += (byte) 0x00; // standard_opcode_lengths[DW_LNS_negate_stmt] = 0
	code += (byte) 0x00; // standard_opcode_lengths[DW_LNS_set_basic_block] = 0
	code += (byte) 0x00; // standard_opcode_lengths[DW_LNS_const_add_pc] = 0
	code += (byte) 0x01; // standard_opcode_lengths[DW_LNS_fixed_advance_pc] = 1
	code += (byte) 0x00; // standard_opcode_lengths[DW_LNS_set_prologue_end] = 0
	code += (byte) 0x00; // standard_opcode_lengths[DW_LNS_set_epilogue_begin] = 0
	code += (byte) 0x01; // standard_opcode_lengths[DW_LNS_set_isa] = 1
	code += Code("/opt/wasm/c-wasm-debug", false, true);
	code += (byte) 0x00;
	code += Code("main.c", false, true);
	code += (byte) 0x01; // dir_index
	code += (uint) 0x00000000; // mod_time length ?

	List<byte> bytes = {
			0x05,                   // 5 bytes length of the extended opcode that follows:
			DW_LNE_set_address,
			(byte) (0x4A - OFFSET), 0x00, 0x00, 0x00, // start_address
			DW_LNS_advance_line, 0x13, // line += 19
			DW_LNE_end_sequence,
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

	code += bytes;
	return createSection(custom_section, encodeVector(Code(".debug_line") + code));
}

Code emit_dwarf_debug_lineX() {
	Code code;
//	let start_address = 0x0000004A;
//	let start_address = 0x0000004A - OFFSET;

	code += (uint) 0x000000A6;  // length as little endian
	code += (short) 0x0004;     // format = DWARF32 version 4
	code += (uint) 0x35;        // prologue_length = 0x35
	code += (byte) 0x01;        // min_inst_length
	code += (byte) 0x01;        // max_ops_per_inst maximum_operations_per_instruction
	code += (byte) 0x01;        // default_is_stmt
	code += (byte) 0xfb;        // line_base = -5
	code += (byte) 0x0e;        // line_range = 14
	code += (byte) 0x0d;        // opcode_base = 13
	code += (byte) 0x00;        // standard_opcode_lengths[DW_LNS_copy] = 0
	code += (byte) 0x01;        // standard_opcode_lengths[DW_LNS_advance_pc] = 1
	code += (byte) 0x01;        // standard_opcode_lengths[DW_LNS_advance_line] = 1
	code += (byte) 0x01;        // standard_opcode_lengths[DW_LNS_set_file] = 1
	code += (byte) 0x00;        // standard_opcode_lengths[DW_LNS_set_column] = 0
	code += (byte) 0x00;        // standard_opcode_lengths[DW_LNS_negate_stmt] = 0
	code += (byte) 0x00;        // standard_opcode_lengths[DW_LNS_set_basic_block] = 0
	code += (byte) 0x01;        // standard_opcode_lengths[DW_LNS_const_add_pc] = 1
	code += (byte) 0x00;        // standard_opcode_lengths[DW_LNS_fixed_advance_pc] = 0
	code += (byte) 0x00;        // standard_opcode_lengths[DW_LNS_set_prologue_end] = 0
	code += (byte) 0x01;        // standard_opcode_lengths[DW_LNS_set_epilogue_begin] = 1

	code += Code("/opt/wasm/c-wasm-debug", false, true);
	code += (byte) 0x00;
	code += Code("main.c", false, true);
	code += (byte) 0x01;        // dir_index
	code += (uint) 0x00000000;  // mod_time length

	List<byte> bytes = {
			0x05,                   // 5 bytes length of the extended opcode that follows:
			DW_LNE_set_address,
			(byte) (0x4A - OFFSET), 0x00, 0x00, 0x00, // start_address
			DW_LNS_advance_line,
			0x0D,                   // line += 13
			0x01,
			0x05,
			DW_LNS_advance_pc,
			0x0A,                   // address += 10
			0x3D,                   // line += 3, address += 5
			DW_LNS_advance_pc,
			0x08,                   // address += 8
			0x00,
			0x01,
			0x01,
			0x00,

			0x05,                   // 5 bytes length of the extended opcode that follows:
			DW_LNE_set_address,
			(byte) (0x57 - OFFSET), 0x00, 0x00, 0x00, // address = 0x57
			DW_LNS_advance_line,
			0x13,                   // line += 19
			0x01,
			0x05,
			DW_LNS_negate_stmt,
			0x0A,
			0x08,
			0xBB,                   // line += 1, address += 29
			0x05,
			0x07,
			0xAD,                   // line += 1, address += 22
			0x08,
			0x83,                   // line += 0, address += 16
			0x05,
			0x04,
			0x08,
			0x83,                   // line += 0, address += 16
			0x05,
			0x07,
			0x08,
			0x83,                   // line += 0, address += 16
			0x05,
			0x04,
			0x08,
			0x83,                   // line += 0, address += 16
			0x05,
			0x07,
			0x08,
			0x83,                   // line += 0, address += 16
			0x05,
			0x04,
			0x06,
			0x74,                   // line += 0, address += 7
			0x05,
			0x09,
			0x06,
			0x08,
			0x4B,                   // line += 3, address += 5
			0x05,
			0x0B,
			0x06,
			0x74,                   // line += 0, address += 7
			0x05,
			DW_LNS_advance_pc,
			0xAC,                   // address += 172
			DW_LNS_advance_pc,
			0x04,                   // address += 4
			0x00,
			0x01,
			0x01,
			0x00,
			0x05,                   // 5 bytes length of the extended opcode that follows:
			DW_LNE_set_address,
			0x2F, 0x01, 0x00, 0x00, // address = 0x12F
			DW_LNS_advance_line,
			0x1F,                   // line += 31
			0x01,
			0x05,
			DW_LNS_advance_pc,
			0x0A,                   // address += 10
			DW_LNS_advance_pc,
			0x29,                   // address += 41
			0x13,
			0x05,
			0x0E,
			0xC9,                   // line += 1, address += 25
			0x05,
			0x0C,
			0x06,
			0x82,                   // line += 0, address += 15
			0x05,
			DW_LNS_advance_pc,
			0xAC,                   // address += 172
			DW_LNS_advance_pc,
			0x17,                   // address += 23
			0x00,
			0x01,
			0x01
	};

	code += bytes;
	return createSection(custom_section, encodeVector(Code(".debug_line") + code));
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
//	List<String> stringList = {"x", "tttt", "int", "main", "j", "/Users/me/dev/apps/wasp/main.c",
//	                           "/Users/me/dev/apps/wasp/","bla","bla","bla","bla","bla","bla"};
// flipped tttt, main!!
	List<String> stringList = {"x", "main", "int", "tttt", "j", "/Users/me/dev/apps/wasp/main.c",
	                           "/Users/me/dev/apps/wasp/", "bla", "bla", "bla", "bla", "bla", "bla"};
	List<String> stringListX = {"x", "tttt", "int", "main", "j", "/Users/me/dev/apps/wasp/main.c",
	                            "/Users/me/dev/apps/wasp/cmake-build-debug-gdb.clang", "version16.0.0",
	                            "(https://github.com/llvm/llvm-project434575c026c81319b393f64047025b54e69e24c2)."};
//
	for (String s: stringList) {
		code += Code((chars) s.data, false, true);
	}
	return createSection(custom_section, encodeVector(Code(".debug_str") + code));
};

Code emit_dwarf_debug_ranges() {
	Code code;
	/*
	 * Contents of section Custom: DWARF 4
/*.debug_ranges contents:
00000000 0000004a 000000c0
00000000 000000c1 00000112
00000000 <End of list>*/
//	code.pushBigEndian(0x00000000); // implicit start
//	code.pushBigEndian(0x0000004a); // Address of tttt() {
//	code.pushBigEndian(0x000000c0); // Address of tttt() }
//	code.pushBigEndian(0x000000c1); // Address of main() {
//	code.pushBigEndian(0x00000112); // Address of main() }
//	code.pushBigEndian(0x00000000); // End of list
	code += (uint) 0x0000004a - OFFSET; // Address of tttt() {
	code += (uint) 0x00000062 - OFFSET; // Address of tttt() }
	code += (uint) 0x00000063 - OFFSET; // Address of main() {
	code += (uint) 0x000000a4 - OFFSET; // Address of main() }
	code += (uint) 0x00000000; // End of list
	code += (uint) 0x00000000; // End of list?

// I don't know how 4a and c0 are calculated. They are not in the .wasm file: nor in the source code: 326 chars to tttt()
// 0000c3 func[3] <tttt>:
// 0000c4: 0e 7f                      | local[1..14] type=i32

	return createSection(custom_section, encodeVector(Code(".debug_ranges") + code));
}

Code emit_dwarf_external_debug_info() {
	Code code;
	return createSection(custom_section, encodeVector(
			Code("external_debug_info") + Code("main.dwo"))); // relative path or URL to main.dwo
}

Code emitDwarfSections() {
	Code code;
//	code += emit_dwarf_external_debug_info(); // split separate debug info OR include:
	code += emit_dwarf_debug_info();
	code += emit_dwarf_debug_ranges();
	code += emit_dwarf_debug_abbrev();// kinda struct defines for DW_TAG_compile_unit, DW_TAG_subprogram
	code += emit_dwarf_debug_line();
	code += emit_dwarf_debug_str();
//	code.save("dwarf_sections.wasm");
	return code;
}
