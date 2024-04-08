//#include "DwarfEmitter.h"
#include "dwarf.h"

typedef unsigned char byte;
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

// llvm-dwarfdump main.wasm

// DWARF fission has advantages in linking and wasm size (https://developer.chrome.com/blog/faster-wasm-debugging) as it
// separates the debug_info section into a separate file leaving a DW_TAG_skeleton_unit in the main wasm file.
// .dwp / .dwo https://github.com/bytecodealliance/wasmtime/pull/8055

/* Language Code ???
 • Use a Placeholder Code: Select a language code that is "close enough" in terms of language features, syntax, or semantics. This is not ideal but can be a temporary workaround. It allows you to use existing tools and infrastructure with minimal changes, although it may cause confusion or misinterpretation of some debugging information.
 • Vendor Extension: DWARF allows for vendor-specific extensions, including the use of language codes in a specific range reserved for vendor-specific use. You can define a unique code for your language within this range. This approach requires coordination with the tools that will consume the DWARF data (debuggers, profilers, etc.) to ensure they correctly interpret the vendor-specific code.
 • DWARF Custom Attributes: Instead of relying solely on the language code, use DWARF's mechanism for custom attributes to provide additional information about the language. This allows you to use a generic language code but still convey specific details about your language through these attributes. *
DW_TAG_access_declaration
DW_TAG_array_type
DW_TAG_atomic_type
DW_TAG_base_type
DW_TAG_call_site
DW_TAG_call_site_parameter
DW_TAG_catch_block
DW_TAG_class_type
DW_TAG_coarray_type
DW_TAG_common_block
DW_TAG_common_inclusion
DW_TAG_compile_unit
DW_TAG_condition
DW_TAG_const_type
DW_TAG_constant
DW_TAG_dwarf_procedure
DW_TAG_dynamic_type
DW_TAG_entry_point
DW_TAG_enumeration_type
DW_TAG_enumerator
DW_TAG_file_type
DW_TAG_formal_parameter
DW_TAG_friend
DW_TAG_generic_subrange
DW_TAG_immutable_type
DW_TAG_imported_declaration
DW_TAG_imported_module
DW_TAG_imported_unit
DW_TAG_inheritance
DW_TAG_inlined_subroutine
DW_TAG_interface_type
DW_TAG_label
DW_TAG_lexical_block
DW_TAG_member
DW_TAG_module
DW_TAG_namelist
DW_TAG_namelist_item
DW_TAG_namespace
DW_TAG_packed_type
DW_TAG_partial_unit
DW_TAG_pointer_type
DW_TAG_ptr_to_member_type
DW_TAG_reference_type
DW_TAG_restrict_type
DW_TAG_rvalue_reference_type
DW_TAG_set_type
DW_TAG_shared_type
DW_TAG_skeleton_unit
DW_TAG_string_type
DW_TAG_structure_type
DW_TAG_subprogram
DW_TAG_subrange_type
DW_TAG_subroutine_type
DW_TAG_template_alias
DW_TAG_template_type_parameter
DW_TAG_template_value_parameter
DW_TAG_thrown_type
DW_TAG_try_block
DW_TAG_typedef
DW_TAG_type_unit
DW_TAG_union_type
DW_TAG_unspecified_parameters
DW_TAG_unspecified_type
DW_TAG_variable
DW_TAG_variant
DW_TAG_variant_part
DW_TAG_volatile_type
DW_TAG_with_stmt
 */


#include "Code.h"

/*


.debug_info contents:
0x00000000: Compile Unit: length = 0x000001d4, format = DWARF32, version = 0x0004, abbr_offset = 0x0000, addr_size = 0x04 (next unit at 0x000001d8)

0x0000000b: DW_TAG_compile_unit
              DW_AT_producer    ("clang version 16.0.0 (https://github.com/llvm/llvm-project 434575c026c81319b393f64047025b54e69e24c2)")
              DW_AT_language    (DW_LANG_C_plus_plus_14)
              DW_AT_name        ("main.cc")
              DW_AT_stmt_list   (0x00000000)
              DW_AT_comp_dir    ("/opt/wasm/c-wasm-debug")
              DW_AT_low_pc      (0x00000000)
              DW_AT_ranges      (0x00000000
                 [0x00000055, 0x000001fb)
                 [0x00000223, 0x0000045f))

0x00000026:   DW_TAG_variable
                DW_AT_type      (0x00000033 "const char[2]")
                DW_AT_decl_file ("/opt/wasm/c-wasm-debug/main.cc")
                DW_AT_decl_line (7)
                DW_AT_location  (DW_OP_addr 0x433)

0x00000033:   DW_TAG_array_type
                DW_AT_type      (0x0000003f "const char")

0x00000038:     DW_TAG_subrange_type
                  DW_AT_type    (0x0000004b "__ARRAY_SIZE_TYPE__")
                  DW_AT_count   (0x02)

0x0000003e:     NULL

0x0000003f:   DW_TAG_const_type
                DW_AT_type      (0x00000044 "char")

0x00000044:   DW_TAG_base_type
                DW_AT_name      ("char")
                DW_AT_encoding  (DW_ATE_signed_char)
                DW_AT_byte_size (0x01)

0x0000004b:   DW_TAG_base_type
                DW_AT_name      ("__ARRAY_SIZE_TYPE__")
                DW_AT_byte_size (0x08)

 0x0000065e:   DW_TAG_typedef
                DW_AT_type	(0x00000669 "long long")
                DW_AT_name	("int64_t")
                DW_AT_decl_file	("wasisdk://v20.0+threads/build/install/opt/wasi-sdk/share/wasi-sysroot/include/bits/alltypes.h")
                DW_AT_decl_line	(145)

0x00000669:   DW_TAG_base_type
                DW_AT_name	("long long")
                DW_AT_encoding	(DW_ATE_signed)
                DW_AT_byte_size	(0x08)


 0x0000069c:   DW_TAG_typedef
                DW_AT_type	(0x000006a7 "__int128")
                DW_AT_name	("ti_int")
                DW_AT_decl_file	("wasisdk://v20.0+threads/src/llvm-project/compiler-rt/lib/builtins/int_types.h")
                DW_AT_decl_line	(79)

0x000006a7:   DW_TAG_base_type
                DW_AT_name	("__int128")
                DW_AT_encoding	(DW_ATE_signed)
                DW_AT_byte_size	(0x10)


 0x00000a32:     DW_TAG_structure_type
                  DW_AT_byte_size	(0x10)
                  DW_AT_decl_file	("wasisdk://v20.0+threads/src/llvm-project/compiler-rt/lib/builtins/int_types.h")
                  DW_AT_decl_line	(97)

0x00000a36:       DW_TAG_member
                    DW_AT_name	("low")
                    DW_AT_type	(0x00000776 "du_int")
                    DW_AT_decl_file	("wasisdk://v20.0+threads/src/llvm-project/compiler-rt/lib/builtins/int_types.h")
                    DW_AT_decl_line	(99)
                    DW_AT_data_member_location	(0x00)

0x00000a42:       DW_TAG_member
                    DW_AT_name	("high")
                    DW_AT_type	(0x00000776 "du_int")
                    DW_AT_decl_file	("wasisdk://v20.0+threads/src/llvm-project/compiler-rt/lib/builtins/int_types.h")
                    DW_AT_decl_line	(100)
                    DW_AT_data_member_location	(0x08)


0x00000a50:   DW_TAG_pointer_type
                DW_AT_type	(0x000009f9 "tu_int")

0x00000a55:   DW_TAG_const_type
                DW_AT_type	(0x0000069c "ti_int")


0x00000a81:   DW_TAG_subprogram
                DW_AT_low_pc	(dead code)
                DW_AT_high_pc	(0x0000004d)
                DW_AT_frame_base	(DW_OP_WASM_location 0x0 0x5, DW_OP_stack_value)
                DW_AT_GNU_all_call_sites	(true)
                DW_AT_name	("__udivti3")
                DW_AT_decl_file	("wasisdk://v20.0+threads/src/llvm-project/compiler-rt/lib/builtins/udivti3.c")
                DW_AT_decl_line	(19)
                DW_AT_prototyped	(true)
                DW_AT_type	(0x00000ad4 "tu_int")
                DW_AT_external	(true)

0x00000a99:     DW_TAG_formal_parameter
                  DW_AT_name	("a")
                  DW_AT_decl_file	("wasisdk://v20.0+threads/src/llvm-project/compiler-rt/lib/builtins/udivti3.c")
                  DW_AT_decl_line	(19)
                  DW_AT_type	(0x00000ad4 "tu_int")

0x00000aa4:     DW_TAG_formal_parameter
                  DW_AT_name	("b")
                  DW_AT_decl_file	("wasisdk://v20.0+threads/src/llvm-project/compiler-rt/lib/builtins/udivti3.c")
                  DW_AT_decl_line	(19)
                  DW_AT_type	(0x00000ad4 "tu_int")

0x00000aaf:     DW_TAG_GNU_call_site
                  DW_AT_abstract_origin	(0x00000ab9 "__udivmodti4")
                  DW_AT_low_pc	(dead code)

0x00000ab8:     NULL



 0x000008ff:         DW_TAG_variable
                      DW_AT_location	(0x00000622:
                         [0x1000000a9, 0x1000000ca): DW_OP_WASM_location 0x0 0x9, DW_OP_stack_value
                         [0x1000000f4, 0x1000000f6): DW_OP_WASM_location 0x2 0x0, DW_OP_stack_value
                      DW_AT_abstract_origin	(0x00000733 "rhat")
 */


Code emit_dwarf_debug_info5() {
	Code code;
//	Contents of section Custom:
//	00001ab: 0b2e 6465 6275 675f 696e 666f 6300 0000  ..debug_infoc...
//	00001bb: 0500 0104 0000 0000 0100 1d00 0108 0000  ................
//	00001cb: 0000 0000 0002 0000 0000 0008 0000 000c  ................
//	00001db: 0000 0002 0076 0000 0004 ed00 039f 0301  .....v..........
//	00001eb: 0b62 0000 0003 0291 0c06 010b 6200 0000  .b..........b...
//	00001fb: 0402 9108 0701 0d62 0000 0000 0501 5100  .......b......Q.
//	000020b: 0000 04ed 0002 9f05 0113 6200 0000 0604  ..........b.....
//	000021b: 0504 00

/* .debug_info contents:
0x00000000: Compile Unit: length = 0x00000063, format = DWARF32, version = 0x0005, unit_type = DW_UT_compile, abbr_offset = 0x0000, addr_size = 0x04 (next unit at 0x00000067)

0x0000000c: DW_TAG_compile_unit
              DW_AT_producer	("clang version 16.0.0 (https://github.com/llvm/llvm-project 434575c026c81319b393f64047025b54e69e24c2)")
              DW_AT_language	(DW_LANG_C11) // 0x1d
              DW_AT_name	("/opt/wasm/c-wasm-debug/main.c") // 0x00 0x01
              DW_AT_str_offsets_base	(0x00000008) // OK
              DW_AT_stmt_list	(0x00000000)
              DW_AT_comp_dir	("/opt/wasm/c-wasm-debug/cmake-build-debug-gdb") // 0x02 ?
              DW_AT_low_pc	(0x00000000)
              DW_AT_ranges	(indexed (0x0) // INDIRECT:
                    rangelist = 0x00000010
                 [0x0000004a, 0x000000c0)
                 [0x000000c1, 0x00000112))
              DW_AT_addr_base	(0x00000008) OK
              DW_AT_rnglists_base	(0x0000000c) OK

 //     0000 0002 0076 0000 0004 ed00 039f 0301  .....v..........
 ////	0b62 0000 0003 02  910c 06 010b 6200 0000  .b..........b...
0x00000027:   DW_TAG_subprogram  // MUST BE INDIRECT!
                DW_AT_low_pc	(0x0000004a)
                DW_AT_high_pc	(0x000000c0)
                DW_AT_frame_base	(DW_OP_WASM_location 0x0 0x3, DW_OP_stack_value 0x9F OK)
                DW_AT_name	("tttt") // 0x03
                DW_AT_decl_file	("/opt/wasm/c-wasm-debug/main.c") // 0x01
                DW_AT_decl_line	(11) // 0x0b OK!!
                DW_AT_prototyped	(true) // NOT THERE / 0x01 comes BEFORE 0x0b !!!
                DW_AT_type	(0x00000062 "int") // OK
                DW_AT_external	(true) // NOT THERE

0x00000039:     DW_TAG_formal_parameter // MUST BE INDIRECT!
                  DW_AT_location	(DW_OP_fbreg == 0x91 +12 == 0x0c) OK !!
                  DW_AT_name	("j") // 06
                  DW_AT_decl_file	("/opt/wasm/c-wasm-debug/main.c") // 0x01 OK
                  DW_AT_decl_line	(11) // 0x0b OK
                  DW_AT_type	(0x00000062 "int") OK
                  */
	return code;
}

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
	code += (uint) 0x5d; // 0x000000 DW_AT_producer DW_FORM_strp LINK
	code += (short) 0x1d; // 0x00 DW_LANG_C11 value 0x1D OK DW_FORM_data2
	code += (uint) 0x12; // 0x000000 DW_AT_name	    DW_FORM_strp "…/main.c"
	code += (uint) 0x00; // DW_AT_stmt_list 0x00000000
	code += (uint) 0x30; // DW_AT_comp_dir DW_FORM_strp "…/cmake-build-debug-gdb"
	code += (uint) 0x00; // DW_AT_low_pc	(0x00000000)
	code += (uint) 0x00; // DW_AT_ranges(0x00000000)

	/*
	  DW_TAG_compile_unit
			  	DW_AT_producer	DW_FORM_strp	("clang version 16.0.0 (https://github.com/llvm/llvm-project 434575c026c81319b393f64047025b54e69e24c2)")
			  	DW_AT_language	DW_FORM_data2	(DW_LANG_C11)
			  	DW_AT_name	    DW_FORM_strp	("/opt/wasm/c-wasm-debug/main.c")
			  	DW_AT_stmt_list	DW_FORM_sec_offset	(0x00000000)
			  	DW_AT_comp_dir	DW_FORM_strp	("/opt/wasm/c-wasm-debug/cmake-build-debug-gdb")
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
	code += (byte) 0x02; // ( abbreviated type DW_TAG_subprogram [2] )
	code += (uint) 0x0000004a;//	DW_AT_low_pc DW_FORM_addr
	code += (uint) 0xc0 - 0x4a;//DW_AT_high_pc	(0x000000c0)  DW_FORM_data4 // 76 0000 00  hex(0xc0 - 0x4a) OK!!!

	code += (byte) 0x04;// length of DW_FORM_exprloc:
	code += (byte) DW_OP_WASM_location;// := 0xED ;; available DWARF extension code 0x0 0x3, DW_OP_stack_value 0x9f OK ) DW_FORM_exprloc
	code += (byte) 0; // DW_AT_frame_base / DW_OP_WASM_global_local ?
	code += (byte) 3; // func#3?  TI_GLOBAL_RELOC / / DW_OP_WASM_global_u32 / 1 byte __stack_pointer ?  main/$_start=0x02 OK
	code += (byte) DW_OP_stack_value; // 0x9F

	code += (uint) 0x02;// DW_AT_name	("tttt") // DW_FORM_strp
	code += (byte) 0x01; //DW_AT_decl_file	("/opt/wasm/c-wasm-debug/main.c") // DW_FORM_data1 means just 1 byte
	code += (byte) 0x0b;// DW_AT_decl_line	(11) // 0x0b DW_FORM_data1 means just 1 byte
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
                  DW_AT_decl_file	("/opt/wasm/c-wasm-debug/main.c")
                  DW_AT_decl_line	(11)
                  DW_AT_type	(0x00000073 "int")
*/
//code += (byte)DW_TAG_formal_parameter;
	code += (byte) 0x03; // abbreviated type 3
	code += (byte) 0x02; // length of DW_AT_location DW_FORM_exprloc:
	code += (byte) DW_OP_fbreg;
	code += (byte) 0x0c; // + 12
	code += (uint) 0x10; // DW_AT_name	("j") // DW_FORM_strp
	code += (byte) 0x01; // DW_AT_decl_file (1)	("/opt/wasm/c-wasm-debug/main.c") // DW_FORM_data1 means just 1 byte
	code += (byte) 0x0b; // DW_AT_decl_line	(11) DW_FORM_data1 means just 1 byte
	code += (uint) 0x73; // DW_AT_type	(0x00000073 "int")

/*
[4] DW_TAG_variable	DW_CHILDREN_no
	DW_AT_location	DW_FORM_exprloc
	DW_AT_name	DW_FORM_strp
	DW_AT_decl_file	DW_FORM_data1
	DW_AT_decl_line	DW_FORM_data1
	DW_AT_type	DW_FORM_ref4
 */
	code += (byte) 0x04; // abbreviated type 4
	code += (byte) 0x02; // length of DW_AT_location DW_FORM_exprloc:
	code += (byte) DW_OP_fbreg;
	code += (byte) 0x08; // + 8
	code += (uint) 0x00; // DW_AT_name	("x") // DW_FORM_strp
	code += (byte) 0x01; // DW_AT_decl_file (1)	("/opt/wasm/c-wasm-debug/main.c") // DW_FORM_data1 means just 1 byte
	code += (byte) 0x0d; // DW_AT_decl_line	(13) DW_FORM_data1 means just 1 byte
	code += (uint) 0x73; // DW_AT_type	(0x00000073 "int")

	code += (byte) 0x00; // NULL why?

	List<byte> others = {0x05, 0xc1, 0x00, 0x00, 0x00, 0x51, 0x00, 0x00, 0x00,
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
	// The .debug_line DWARF section maps instruction pointers to source locations.
	//	 offset of an instruction relative within the Code section of the WebAssembly file.

	/* Contents of section Custom: DWARF 4
00002d7: 0b2e 6465 6275 675f 6c69 6e65 8900 0000  ..debug_line....
00002e7: 0400 3500 0000 0101 01fb 0e0d 0001 0101  ..5.............
00002f7: 0100 0000 0100 0001 2f6f 7074 2f77 6173  ......../opt/was
0000307: 6d2f 632d 7761 736d 2d64 6562 7567 0000  m/c-wasm-debug..
0000317: 6d61 696e 2e63 0001 0000 0000 0502 4a00  main.c........J.
0000327: 0000 030b 0105 0d0a 08bb 0511 0674 0513  .............t..
0000337: 7405 1274 050f 7405 0974 0675 050a 0674  t..t..t..t.u...t
0000347: 0507 ac05 0c06 7505 0e06 7405 05ac 0204  ......u...t.....
0000357: 0001 0100 0502 c100 0000 0313 0105 050a  ................
0000367: 0229 13c9 021b 0001 01                   .).......
*/
	code += (uint) 0x00000089; // length little as endian
	code += (short) 0x0004; // format = DWARF32 version 4
	code += (uint) 0x35; // prologue_length = 0x35
	code += (byte) 0x01; // min_inst_length
	code += (byte) 0x01; // max_ops_per_inst
	code += (byte) 0x01; // default_is_stmt
	code += (byte) 0xfb; // line_base = -5 // ???
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
	code += Code("/opt/wasm/c-wasm-debug", false,
	             true); // 0x2f 0x6f 0x70 0x74 0x2f 0x77 0x61 0x73 0x6d 0x2f 0x63 0x2d 0x77 0x61 0x73 0x6d 0x2d 0x64 0x65 0x62 0x75 0x67 0x00
	code += (byte) 0x00; // another zero byte already there ^^
	code += Code("main.c", false, true); // 0x6d 0x61 0x69 0x6e 0x2e 0x63 00
	code += (byte) 0x01; // dir_index
	code += (uint) 0x00000000; // mod_time length ?

	/*
0502 4a00 0000 030b 0105 0d0a 08bb 0511 0674 0513  ..J..............t..
	 */
//	address += (opcode - opcode_base) / line_range * minimum_instruction_length
//	line += line_base + (opcode - opcode_base) % line_range
	code += (byte) 0x05; // ?
	code += (byte) 0x02; // ?
//	maybe we need to add / subtract these:
//	code += (byte) 0xfb; // line_base = -5 // ???
//	code += (byte) 0x0e; // line_range = 14
//	code += (byte) 0x0d; // opcode_base = 13

	/*
Address            Line   Column File   ISA Discriminator OpIndex Flags
------------------ ------ ------ ------ --- ------------- ------- -------------
0x000000000000004a     12      0      1   0             0       0  is_stmt
0x0000000000000067     13     13      1   0             0       0  is_stmt prologue_end
0x000000000000006e     13     17      1   0             0       0
	 */

	code += (uint) 0x4a; // Address
	List<byte> bytes = {0x03, 0x0b, 0x01, 0x05, 0x0d, 0x0a, 0x08, 0xbb, 0x05, 0x11, 0x06, 0x74, 0x05, 0x13,
	                    0x74, 0x05, 0x12, 0x74, 0x05, 0x0f, 0x74, 0x05, 0x09, 0x74, 0x06, 0x75, 0x05, 0x0a, 0x06, 0x74,
	                    0x05, 0x07, 0xac, 0x05, 0x0c, 0x06, 0x75, 0x05, 0x0e, 0x06, 0x74, 0x05, 0x05, 0xac, 0x02, 0x04,
	                    0x00, 0x01, 0x01, 0x00, 0x05, 0x02, 0xc1, 0x00, 0x00, 0x00, 0x03, 0x13, 0x01, 0x05, 0x05, 0x0a,
	                    0x02, 0x29, 0x13, 0xc9, 0x02, 0x1b, 0x00, 0x01, 0x01};
	code += bytes;
//code += (byte) 0x0c; // Line
//	0x0000000000000067 NEVER APPEARS!!
/*
.debug_line contents:
debug_line[0x00000000]
Line table prologue:
    total_length: 0x00000089
          format: DWARF32
         version: 4
 prologue_length: 0x00000035
 min_inst_length: 1
max_ops_per_inst: 1
 default_is_stmt: 1
       line_base: -5
      line_range: 14
     opcode_base: 13
standard_opcode_lengths[DW_LNS_copy] = 0
standard_opcode_lengths[DW_LNS_advance_pc] = 1
standard_opcode_lengths[DW_LNS_advance_line] = 1
standard_opcode_lengths[DW_LNS_set_file] = 1
standard_opcode_lengths[DW_LNS_set_column] = 1
standard_opcode_lengths[DW_LNS_negate_stmt] = 0
standard_opcode_lengths[DW_LNS_set_basic_block] = 0
standard_opcode_lengths[DW_LNS_const_add_pc] = 0
standard_opcode_lengths[DW_LNS_fixed_advance_pc] = 1
standard_opcode_lengths[DW_LNS_set_prologue_end] = 0
standard_opcode_lengths[DW_LNS_set_epilogue_begin] = 0
standard_opcode_lengths[DW_LNS_set_isa] = 1
include_directories[  1] = "/opt/wasm/c-wasm-debug"
file_names[  1]:
           name: "main.c"
      dir_index: 1
       mod_time: 0x00000000
         length: 0x00000000
11: int tttt(int j)
12: {
13:    int x = j + j*j
14:    x = x+1;
 …

Address            Line   Column File   ISA Discriminator OpIndex Flags
------------------ ------ ------ ------ --- ------------- ------- -------------
0x000000000000004a     12      0      1   0             0       0  is_stmt // {
0x0000000000000067     13     13      1   0             0       0  is_stmt prologue_end // int x =
0x000000000000006e     13     17      1   0             0       0
0x0000000000000075     13     19      1   0             0       0
0x000000000000007c     13     18      1   0             0       0
0x0000000000000083     13     15      1   0             0       0
0x000000000000008a     13      9      1   0             0       0
0x0000000000000091     14      9      1   0             0       0  is_stmt
0x0000000000000098     14     10      1   0             0       0
0x00000000000000a3     14      7      1   0             0       0
0x00000000000000aa     15     12      1   0             0       0  is_stmt
0x00000000000000b1     15     14      1   0             0       0
0x00000000000000bc     15      5      1   0             0       0
0x00000000000000c0     15      5      1   0             0       0  end_sequence
0x00000000000000c1     20      0      1   0             0       0  is_stmt
0x00000000000000ea     21      5      1   0             0       0  is_stmt prologue_end
0x00000000000000f7     22      5      1   0             0       0  is_stmt
0x0000000000000112     22      5      1   0             0       0  is_stmt end_sequence
*/
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
	List<String> stringList = {"x", "tttt", "int", "main", "j", "/opt/wasm/c-wasm-debug/main.c",
	                           "/opt/wasm/c-wasm-debug/cmake-build-debug-gdb.clang", "version16.0.0",
	                           "(https://github.com/llvm/llvm-project434575c026c81319b393"};//f64047025b54e69e24c2)."};
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
	code += (uint) 0x0000004a; // Address of tttt() {
	code += (uint) 0x000000c0; // Address of tttt() }
	code += (uint) 0x000000c1; // Address of main() {
	code += (uint) 0x00000112; // Address of main() }
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
	code.save("dwarf.wasm");
	return code;
}
