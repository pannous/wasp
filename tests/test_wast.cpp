//
// Created by pannous on 12.06.20.
//

#include "wasm_helpers.h"
#include "Node.h"

chars wat = R"(module
  (table (;0;) 1 1 funcref)
  (memory (;0;) 2)
  (export "memory" (memory 0))
  (export "add1" (func 0))
  (export "wasp_main" (func $main))

  (type $ii_i (func (param i32 i32) (result i32)))
  (func $add (type $ii_i) (param i32 i32) (result i32)
    local.get 0
    local.get 1
    i32.add
    )

  (func $main (type 0) (param i32 i32) (result i32)
	  local.get 0
	  local.get 1
	  (call $add)
	  drop
	  i32.const 21
	  i32.const 21
	  call $add
	  ;;(i32.const 42)
  )
)";

void testParse() {
//	Mark::markmode();
//	const Node &node = Mark::parseFile("/Users/me/dev/wasm/test.wat");
	Node module = parse(wat);
	assert_equals(module, "module");
	printf("%s", module.toString());
	assert_equals(module.length, 8);
//	assert_equals(node.length, 12);
//	puts(node);
	assert_equals(module[0], "table");
	assert_equals(module[1], "memory");
	assert_equals(module[2], "export");
	check(module["func"].length == 2)
	check(module["func"]["$main"]["param"].length == 2);

}

void testWast() {
    use_polish_notation = true;
    testParse();
    use_polish_notation = false;
}
