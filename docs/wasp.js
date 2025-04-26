/* 
* WASP: WebAssembly Programming Language API/ABI
* version="1.0.27",
* This file contains the javascript counterpoint to the WASP runtime,
* offering host functions to wasi/wasp modules, like download() and run_wasm()
* Converts wasm types to/from JS objects via node() and string() as a shim for wasm GC types
* */
let Wasp = {}
let WASP_COMPILER = 'assets/wasp-hosted.wasm' // hard link 4MB with tests and shortcuts, 6.6MB with linker!
// let WASP_COMPILER = 'assets/wasp-hosted-release.wasm' // hard link 300k without tests
// let WASP_COMPILER = 'assets/wasp-release-debug.wasm' // hard link 300k without tests
// let WASP_COMPILER = 'assets/wasp-release.wasm' // hard link 300k without tests currently not working!
let WASP_RUNTIME = 'wasp-runtime.wasm' // 100kb now in :
let lib_folder_url = "assets/lib/"
// let lib_folder_url = "https://pannous.github.io/wasp/lib/"

let runtime_bytes = null; // for reflection or linking
let needs_runtime = false; // set per app!
// const use_big_runtime = true; // use compiler as runtime for now
const use_big_runtime = false; // link / use small runtime IN compiler
const run_tests = true; // todo NOT IN PRODUCTION!
// const run_tests = false;
let app_module;
let kinds = {}

// MAX_MEM is NOT affected by -Wl,--initial-memory=117964800 NOR by this: HOW THEN??
let min_memory_size = 100 // in 64k PAGES! 65536 is upper bound => 64k*64k=4GB
let max_memory_size = 65536 // in 64k PAGES! 65536 is upper bound => 64k*64k=4GB
let memory = new WebAssembly.Memory({initial: min_memory_size, maximum: max_memory_size});
// this memory object is ONLY USED if wasm file imports memory and doesn't provide its own, hopefully exported!

let wasm_pointer_size = 4;// 32 bit todo: 64 bit on demand

// see [smart-pointers](https://github.com/pannous/wasp/wiki/smart-pointer)
let string_header_32 = 0x10000000
let array_header_32 = 0x40000000
let ref_header_32 = 0x60000000
let node_header_32 = 0x80000000

function binary_hack(binary_as_text) {
  let binary = new Uint8Array(binary_as_text.length);
  for (let i = 0; i < binary_as_text.length; i++) {
    binary[i] = binary_as_text.charCodeAt(i);
  }
  return binary
}

var download_async = (url) => fetch(url).then(res => res.text())

function download(url, binary = false, mem = app.memory) {
  if (typeof url != "string") url = chars(url, mem) // unless wasm test
  // debug("download", url)
  let xhr = new XMLHttpRequest();
  // if (binary) xhr.responseType = 'arraybuffer'; // not allowed for sync requests
  if (binary) xhr.overrideMimeType('text/plain; charset=x-user-defined'); // Prevent UTF-8 decoding
  xhr.open('GET', url, false);
  xhr.send();
  if (xhr.status === 200)
    return binary ? bytes(binary_hack(xhr.response)) : chars(xhr.responseText.trim(), mem) // to be used in WASM as string! use fetch() in JS
  else
    throw new Error(`Failed to download ${url}: ${xhr.status} ${xhr.statusText}`);
  return null;
}

function format(object) {
  if (object instanceof node)
    return object.serialize()
  if (typeof (object) == 'object')
    return JSON.stringify(object);
  return object;
}

function error(msg) {
  // if (typeof results !== 'undefined')
  if (msg instanceof WebAssembly.CompileError) {
    // results.value += "\n⚠️ COMPILE ERROR:\n";
    results.value += "\n" + msg.stack + "\n";
  } else if (msg instanceof Error) {
    results.value += "\n⚠️ ERROR: " + msg + "\n";
    results.value += msg.stack + "\n";
    throw msg
  } else
    throw new Error("⚠️ ERROR: " + msg)
}

let nop = x => 0 // careful, some wasi shim needs 0!

const fd_write = function (fd, c_io_vector, iovs_count, nwritten) {
  while (iovs_count-- > 0) {
    let text = string(c_io_vector);
    if (fd === 0)
      error(text + "\n");
    else {
      results.value += text + "\n";
      debug(text + "\n");
    }
    c_io_vector += 8
  }
  return -1; // todo
};


function getWasmFunclet(funclet_name, size_p) {
  let file = lib_folder_url + chars(funclet_name)
  if (!file.endsWith(".wasm")) file += ".wasm"
  let [pointer, bytes_size] = download(file, binary = true, memory) // load into compiler!
  debug("getWasmFunclet", chars(funclet_name), pointer, bytes_size)
  set_int(size_p, bytes_size)
  return pointer
}

function parse(data) {
  let node_pointer = compiler_exports.Parse(chars(data))// also calls run()!
  let nod = new node(node_pointer);
  return nod
}

function terminate() {
  debug("wasm terminate()")
  // if(sure)
  throw new Error("wasm terminate() via proc_exit")
}

function createHtml(parent, data) { // via emitHtml
  let innerHtml = chars(data, app.memory);
  element = document.createElement("div");
  element.innerHTML = innerHtml;
  if (!parent) parent = document.body;
  parent.appendChild(element);
  return element;
}

function matrix_multiply(a, b, k = 1) {
  warn("vector.dot / mul shim via webgpu-blas until wasm vector proposal is available")
  a = node(a, memory)
  b = node(b, memory)
  // todo other types
  if (a.kind == kinds.list && a.type == kinds.float32)
    a.kind = kinds.vector // todo add offset and length
  if (a.kind == kinds.buffer && a.type == kinds.float32)
    a.kind = kinds.vector
  if (b.kind == kinds.buffer && b.type == kinds.float32)
    b.kind = kinds.vector
  if (b.kind == kinds.matrix)
    k = b.row_count
  if (b.kind == kinds.matrix)
    k = b.column_count
  if (a.kind != kinds.vector || b.kind != kinds.vector)
    throw new Error("dot product only for vectors")
  // efficient implementation, the condition m % 32 === 0 && n % 64 === 0 && k % 4 === 0 && alpha === 1.0 have to met.

  include("https://github.com/milhidaka/webgpu-blas/releases/download/v1.2.3/webgpublas.js")
  let array_a = new Float32Array(buffer, a.offset, a.length)
  let array_b = new Float32Array(buffer, b.offset, b.length)

  const alpha = 1.0; // ?
  let m = a.length / k
  let n = b.length / k
  // await
  const result = webgpublas.sgemm(m, n, k, alpha, array_a, array_b);
  debug(result); // m*n row-major matrix (Float32Array)
  return result
}

/**
 * Returns a BigInt smart pointer of the given object.
 * ⚠️ as opposed to smartValue which returns the value of the smart pointer.
 *
 * @param {Object} object - The object to create a smart pointer for.
 * @param {WebAssembly.Memory} [mem=memory] - The memory to associate with the object. Defaults to `memory`.
 * @returns {BigInt} A BigInt representing the smart pointer to the object.
 */
function smartResult(object, mem = memory) {
  last_result = object
  if (typeof object === "number")
    return BigInt(object)
  // if (typeof object === "string" && !isNaN(object))
  //     return BigInt(object)
  // if (typeof object === "object")
  //     return -123;//
  try {
    // print("smartResult: ", object, "trying to parse to BigInt")
    let reflect = {}
    for (let prop in object) {
      if (typeof object[prop] === "function") {
        // reflect[prop] = object[prop].toString()
      } else {
        reflect[prop] = object[prop]
      }
    }
    let typed = {}
    let typ = typeof object
    if (typ === "object") typ = object.constructor.name
    typed[typ] = reflect
    // serialized=JSON.stringify(object)
    if (typeof object === "string") serialized = object
    else serialized = JSON.stringify(typed)
    // serialized=JSON.stringify(reflect)

    debug("smartResult: typeof", typeof object, ": ", object, " =>", serialized)
    last_result = serialized
    let wrapped = string(serialized, app.memory)
    // let wrapped = chars(serialized, mem)
    // string_header_32 => json_header_32 to be parsed
    let big_wrap = BigInt(wrapped) | BigInt(string_header_32) << BigInt(32)
    debug("smartResult: ", object, "as smart type BigInt", hex(big_wrap));
    return big_wrap // BigInt(wrapped) // to be consumed by wasm, potentially returned to wasp via main and wasm_done
  } catch (ex) {
    // todo error handling via print!
    console.error("smartResult: ", object, "failed to parse to BigInt", ex)
    return -1
  }
}

const refToIndexMap = new Map();

async function storeObject() {
// (table (export "externref_table") 1 externref)
  const table = instance.exports.externref_table;

// Store a JavaScript object in the table
  const ref = {message: "Hello from externref!"};
  const index = table.length; // Get the current table size
  table.grow(1); // Expand table by one slot
  table.set(index, ref);
  refToIndexMap.set(ref, index);
  debug("Stored object index:", index); // Outputs the index where `ref` is stored
  // NOW WE CAN PASS THE INDEX TO WASM AS SMARTY !

  // Retrieve the object by index
  const retrievedObj = table.get(index); // Retrieve the object by index directly! ref=>object!
  debug(retrievedObj.message); // Outputs: "Hello from externref!"
  /*
  ;; Retrieve the externref from the table and call the logger
  (call $print_externref (table.get (local.get 0)))
  (call $cast_to_node (table.get (local.get 0)))
   */
}

function getExternRefIndex(ref) {
  return refToIndexMap.get(ref); // Retrieve the index for a given ref
}

async function storeFunction() {
//   (table (export "funcref_table") 10 funcref)
// Access the exported table
  const table = app.exports.funcref_table;

// Store a JavaScript function in the table
  const externalFunction = () => debug("Hello from JavaScript!");
  const index = table.length; // Use current length to append
  table.grow(1); // Expand table size
  table.set(index, externalFunction);

// Retrieve and call the function by ID (index)
  const retrievedFunction = table.get(index);
  retrievedFunction(); // Outputs: "Hello from JavaScript!"

  /* (func (export "call_by_index") (param i32)
  ;; Call the function stored at the table index provided
  call_indirect (type $type) (local.get 0)
) */
  app.exports.call_by_index(index);
}

function getExternRefPropertyValue(ref, prop0) {
  let prop = chars(prop0, app.memory)
  debug("CALLING getExternRefPropertyValue", ref, prop)
  if (ref && typeof ref[prop] !== 'undefined') {
    let val = ref[prop];
    if (typeof val === 'function') val = val.bind(ref)() // vs invokeExternRef
    debug("getExternRefPropertyValue OK ", ref, prop, val, typeof val)
    return smartResult(val, app.memory)
  } else if (ref && typeof ref.getAttribute === 'function') {
    let attribute = ref.getAttribute(prop);
    debug("getExternRefPropertyValue OK! ", ref, prop, attribute)
    return smartResult(attribute, app.memory)
  } else {
    throw new Error(`'${prop}' is not a property of the provided reference`);
  }
}

function setExternRefPropertyValue(ref, property, value_node) {
  let field = chars(property, app.memory)
  let value = smartValue(value_node, 0, app.memory)// node(value_node,app.memory).value
  print("CALLING setExternRefPropertyValue", ref, field, value)
  if (ref && typeof ref.setAttribute === 'function') {
    ref.setAttribute(field, value);
  } else {
    ref[field] = value // may create it
    // throw new Error(`'${field}' is not a property of the provided reference`);
  }
}

const jsStringPolyfill = {
  "charCodeAt": (s, i) => s.charCodeAt(i),
  "compare": (s1, s2) => {
    if (s1 < s2) return -1;
    if (s1 > s2) return 1;
    return 0;
  },
  "concat": (s1, s2) => s1 + s2,
  "equals": (s1, s2) => s1 === s2,
  "fromCharCode": (i) => String.fromCharCode(i),
  "length": (s) => s.length,
  "substring": (s, a, b) => s.substring(a, b),
};


let resume; // callback function resuming after run_wasm finished
class YieldThread {
} // unwind wasm, re-enter through resume() after run_wasm finished

function getRidOfDependency(x) {
  throw new Error("getRidOfDependency")
}

let imports = {
  "wasm:js-string": jsStringPolyfill, // ignored when provided as WebAssembly.compile(bytes, { builtins: ['js-string'] });
  vector: { // todo: use wasm vector proposal when available, using webgpu-blas as a shim
    dot: (a, b) => {
      matrix_multiply(a, b, 1)
    },
    mul: matrix_multiply,
    matrix_multiply
  },
  env: { // MY_WASM custom wasp helpers
    memory, // optionally provide js Memory … alternatively use exports.memory in js, see below
    HEAP_END: new WebAssembly.Global({value: "i32", mutable: true}, 0),// todo: use as HEAP_END
    grow_memory: x => memory.grow(1), // à 64k … NO NEED, host grows memory automagically!
    async_yield: x => { // called from inside wasm, set callback handler resume before!
      throw new YieldThread() // unwind wasm, re-enter through resume() after run_wasm
    },
    // the following dependencies only appear when using the linker!!
    vsnprintf: getRidOfDependency,
    stat: getRidOfDependency,
    strerror: getRidOfDependency,
    fseek: getRidOfDependency,
    perror: getRidOfDependency,
    ftell: getRidOfDependency,
    fread: getRidOfDependency,
    ferror: getRidOfDependency,
    abort: getRidOfDependency,
    isprint: getRidOfDependency,
    snprintf: getRidOfDependency,
    // std::__2::basic_string<char, std::__2::char_traits<char>, std::__2::allocator<char> >::~basic_string()
    __cxa_pure_virtual: getRidOfDependency,
    _ZNSt3__212basic_stringIcNS_11char_traitsIcEENS_9allocatorIcEEE6__initEPKcm: getRidOfDependency,
    _ZNSt20bad_array_new_lengthC1Ev: getRidOfDependency,// todo get rid of implicit new []
    _ZNSt20bad_array_new_lengthD1Ev: getRidOfDependency,
    __ZNSt3__212basic_stringIcNS_11char_traitsIcEENS_9allocatorIcEEED1Ev: getRidOfDependency, // WTH
    _ZNSt3__212basic_stringIcNS_11char_traitsIcEENS_9allocatorIcEEED1Ev: getRidOfDependency,
    _ZNSt3__212basic_stringIcNS_11char_traitsIcEENS_9allocatorIcEEE9push_backEc: getRidOfDependency,
    _ZNSt3__212basic_stringIcNS_11char_traitsIcEENS_9allocatorIcEEE6appendEPKc: getRidOfDependency,
    // the above dependencies only appear when using the linker!!

    /* run_wasm: async (x, y) => { */ // Cannot convert [object Promise] to a BigInt
    run_wasm: (x, y) => {
      try {
        run_wasm(x, y) // todo: await
        return BigInt(42)
      } catch (ex) {
        wasm_buffer = buffer.subarray(x, x + y)
        // download(wasm_buffer, "emit.wasm", "wasm") // resume
        error(ex)
      }
    }, // allow wasm modules to run plugins / compiler output
    assert_expect: x => {
      value = new node(x).Value()
      try {
        print("assert_expect", x, value)
      } catch (ignore) {
      }
      if (expect_test_result)
        error("already expecting value " + expect_test_result + " -> " + x + "\n Did you emit in testCurrent?")
      expect_test_result = value
    },
    // HTML DOM JS functions
    // download: new WebAssembly.Suspending(download_async),
    download,
    getWasmFunclet,
    init_graphics: nop, // canvas init by default
    requestAnimationFrame: nop, // todo
    __assert_fail: nop, // todo remove
    getenv: x => {
      debug("getenv", x, chars(x, typeof (app) != 'undefined' ? app.memory : memory));
      return 0
    }, // todo
    fopen: x => {
      debug("fopen", x);
      return 0
    }, // todo WASI / NOT
    fprintf: (pipe, format, ...args) => {
      debug("fprintf", pipe, format, args);
      console.log(string(format), string(args))
      // printf(format, args)
      return 0
    }, // todo WASI / NOT
    fgetc: x => {
      debug("fgetc", x);
      return 0
    }, // todo WASI / NOT
    fclose: x => {
      debug("fclose", x);
      return 0
    }, // todo WASI / NOT
    getDocumentBody: () => document.body,
    addScript: (scriptContent) => {
      let script = document.createElement('script');
      script.textContent = chars(scriptContent, app.memory);
      debug("script", script.textContent)
      document.body.appendChild(script);
    },
    createHtml,
    createHtmlElement: (parent, tag, id) => {
      let element = document.createElement(chars(tag, app.memory));// string
      if (id) element.id = chars(id);
      parent = parent || document.body;
      parent.appendChild(element);
      return element;
    },
    getElementById: pointer => {
      let id = chars(pointer, app.memory)
      let object = document.getElementById(id)
      debug("getElementById", pointer, id, "=>", object)
      return object // automatically cast to (extern)ref
    },
    // getExternRefPropertyValue: async (ref, prop0) => { async not working in wasm!
    getExternRefPropertyValue,
    setExternRefPropertyValue,
    invokeExternRef: (ref, fun0, params0) => {
      let fun = chars(fun0, app.memory)
      let params = chars(params0, app.memory) // node(params0, app.memory)
      print("invokeExternRef", ref, fun, params)
      // Check if 'fun' is a valid method of 'ref'
      if (ref && typeof ref[fun] === 'function') {
        // Call the method with the provided parameters
        if (Array.isArray(params))
          result = ref[fun](...params);
        else
          result = ref[fun](params);
        print("invokeExternRef RESULT", result)
        return smartResult(result, app.memory);
      } else {
        // Handle the case where 'fun' is not a valid method
        throw new Error(`'${fun}' is not a function of the provided reference`);
      }
    },

    exit: terminate, // should be wasi.proc_exit!
    pow: (x, y) => { // via pow.wasm funclet => never called here IF LINKED!
      debug("pow", x, y);
      return x ** y
    },
    print: x => print(smartValue(x, app.memory)), // todo: right memory!
    puti: x => debug(x), // allows debugging of ints without format String allocation!
    js_demangle: x => chars(demangle(chars(x))),
    __cxa_demangle: (name, buf, len, status_p) => chars(demangle(chars(name))),
  },
  wasi_unstable: {
    fd_write, // printf
    proc_exit: terminate, // all threads
    // ignore the rest for now
    args_sizes_get: x => 0,
    args_get: nop,
    environ_get: nop,
    environ_sizes_get: nop,
    fd_fdstat_get: nop,
    fd_prestat_get: nop,
    fd_prestat_dir_name: nop,
    fd_close: nop,
    fd_seek: nop
  }
}
imports.wasi_snapshot_preview1 = imports.wasi_unstable // fuck wasmedge!
Wasp = imports // provide global Wasp object for debugging, fill with wasi functions and runtime exports later!

const console_log = window.console.log

function debug(...args) {
  // if(debugging)
  console_log.apply(console, args); // Call the original debug
}

let todo = x => console.error("TODO", x)
let puts = x => print(chars(x)) // char*
let prints = x => print(string(x)) // char**
// const print = window.console.log ;// redirect to text box ! results.value += text + "\n";
// window.console.log =
const print = function (...args) {
  const formattedArgs = args.map(arg => {
    if (typeof arg === "string" || typeof arg === "number")
      return arg; // No modification needed
    else
      return JSON.stringify(arg, null, 2); // Format non-string/number values
  });
  results.value += formattedArgs.join(' ') + '\n';
  console_log.apply(console, args); // Call the original debug
};
// window.console.log = print// redirect ALL to text box

// #if not TRACE
// redirect after testing!
// window.debug = function (...args) {
//     console_log(...args);
//     args.forEach(arg => results.value += `${format(arg)}\n`);
// }

function check(ok, msg) {
  if (!ok) throw new Error("⚠️ TEST FAILED! " + (msg || ""));
}

function check_eq(a, b, msg) {
  if (a != b) throw new Error("⚠️ TEST FAILED! " + a + "≠" + b + "  " + (msg || ""));
}

function debugMemory(pointer, num, mem) {
  if (!mem) mem = memory
  let buffer = new Uint8Array(mem.buffer, pointer, num);
  let i = 0
  while (num-- > 0) {
    let x = buffer[i++]
    if (i < 3 || x)
      debug("CHAR AT", i, hex(x), chr(x), x)
  }
}

function string(data, mem = memory) { // wasm<>js interop
  switch (typeof data) {
    case "string":
      // todo use HEAP_END of APP, not of compiler! lol
      while (HEAP_END % 8) HEAP_END++ // align to 8 bytes
      let p = HEAP_END
      new_int(HEAP_END + 12, mem) // pointer to chars = string_start + 20  TODO 12 vs 20 in c++/ABI
      new_int(data.length, mem)
      new_int(string_header_32, mem)
      // new_long(0, mem) // 8 byte *codepoints 64 bit in c++
      // new_int(0, mem) // 4 byte *codepoints 32 bit in WASM !!!
      // new_int(-1, mem) // codepoint_count
      // new_int(0, mem) // shared reference
      chars(data, mem);
      return p;
    case "bigint":
    case "number":
    default:
      if (typeof data === "bigint") {
        if (data > 0xFFFFFFFFn) todo("bigint pointer");
        data = Number(data);
      }
      let pointer = read_int32(data, mem);
      let length = read_int32(data + 4, mem);
      let kind = read_int32(data + 8, mem); // todo not part of string ABI!
      if (kind == length) ;//todo("string_header_32 kind==length HOW?");
      else if (kind != string_header_32) {
        // debug("TODO missing string_header_32 kind", kind, pointer, length, data)
        // todo mixing string_header_32 for strings and other chars BAD!!
        if (!kind) return load_chars(pointer, length, mem);// todo messed up ABI!!
        return load_chars(data, -1, mem);
      }
      // else debug("found string_header_32", pointer, length, data)
      if (!pointer) {
        debug("NO chars to read")
        debugMemory(data - 10, 20, mem)
      }
      // debug("pointer, length, mem", pointer, length, mem)
      let cs = load_chars(pointer, length, mem);
      return cs
  }
}

function load_chars(pointer, length = -1, module_memory = 0, format = 'utf8') {
  if (!module_memory) module_memory = memory
  if (pointer === 0) return
  if (typeof pointer == "string") return chars(s)
  // debug("string",pointer,length)
  if (length < 0) { // auto length
    let buffer = new Uint8Array(module_memory.buffer, pointer, module_memory.length);
    while (buffer[++length]) ;// strlen ;)
  }
  //debug("length:",length,"format:",format,"pointer:",pointer,"TextDecoder:",typeof(TextDecoder))
  if (typeof (TextDecoder) != 'undefined') {// WEB, text-encoding, Node 11
    const utf8_decoder = new TextDecoder('utf8');
    let decoder = format == 'utf8' ? utf8_decoder : utf16denoder
    let arr = new Uint8Array(module_memory.buffer, pointer, length);
    return decoder.decode(arr)
  } else { // fallback
    buf = Buffer.from(module_memory.buffer) // Node only!
    buf = buf.slice(pointer, pointer + length)
    s = buf.toString(format)// utf8 or 'utf16le'
    return s
  }
}

function bytes(data, mem = memory) {
  if (!data) return 0;// MAKE SURE!
  buffer = new Uint8Array(mem.buffer, HEAP_END, data.length); // wasm linear memory
  buffer.set(data, 0); // copy data to wasm linear memory
  let c = HEAP_END
  HEAP_END += data.length;
  // debug("bytes", data.length, HEAP_END, buffer)
  return [c, data.length]
}

function chars(data, mem = memory) {
  if (!data) return 0;// MAKE SURE!
  if (typeof data != "string") return load_chars(data, -1, mem)
  const uint8array = new TextEncoder("utf-8").encode(data + "\0");
  return bytes(uint8array, mem)[0] // pointer without length
}

function set_int(address, val, mem = memory) {
//    memory.setUint32(address + 0, val, true); // ok ?
  let buf = new Uint32Array(mem.buffer, address, 4);
  buf[0] = val
}

function new_int(val, mem = memory) {
  while (HEAP_END % 4) HEAP_END++;
  let buf = new Uint32Array(mem.buffer, HEAP_END, 4);
  buf[0] = val
  HEAP_END += 4
}


function new_long(val) {
  // memory.setUint32(addr + 0, val, true);
  // memory.setUint32(addr + 4, Math.floor(val / 4294967296), true);
  let buf = new Uint64Array(memory.buffer, HEAP_END, mem.length); // todo: /8??
  buf[0] = val
  HEAP_END += 8
}

function read_byte(pointer, mem = memory) {
  // return mem.buffer[pointer]
  let buffer = new Int8Array(mem.buffer, pointer, 1);
  return buffer[0]
}

function read_int32(pointer, mem = memory) {
  // return mem.getUint32(addr + 0, true); // ok?
  buffer = new Uint8Array(mem.buffer, 0, mem.length);
  return buffer[pointer + 3] * 2 ** 24 + buffer[pointer + 2] * 256 * 256 + buffer[pointer + 1] * 256 + buffer[pointer];
}

// function read_int32(pointer, mem) {
// ONLY WORKS if pointer is 4 byte aligned!
// todo align ALL ints …!
// if (!mem) mem = memory
// start offset of Int32Array should be a multiple of 4
//     let buffer = new Int32Array(mem.buffer, pointer, 4);
//     return buffer[0]
// }

// getInt64
function read_int64(pointer, mem = memory) { // little endian
  // const low = mem.getUint32(pointer + 0, true);
  // const high = mem.getInt32(pointer + 4, true);
  // return low + high * 4294967296; // ok ?
  let buffer = new BigInt64Array(mem.buffer, pointer, 8);
  return buffer[0]
}

// reset at each run, discard previous data!
// NOT COMPATIBLE WITH ASYNC CALLS!
function reset_heap() {
  print("Todo reset_heap, currently BAD! And UNNECESSARY: tests() ok after 1000 runs!!")
  return
  // todo: this is for the compiler, not runtime/app!
  HEAP = compiler_exports.__heap_base; // ~68000 Todo: INCREASE after load_runtime_bytes() !?
  DATA_END = compiler_exports.__data_end
  HEAP_END = HEAP || DATA_END;
  // HEAP_END += 0x100000 * run++; // todo
  // compiler_exports.setHeapEnd(HEAP_END); BAD!!
}

function compile_and_run(code) {
  // results.value = '';
  if (typeof compiler_exports === 'undefined')
    load_compiler()
  // reset_heap();
  expect_test_result = false
  try {
    compiler_exports.run(chars(code));
  } catch (err) {
    error(err)
  }
}

function reinterpretInt64AsFloat64(n) { // aka reinterpret_cast long to double
  const intArray = new BigInt64Array(1);
  intArray[0] = BigInt(n)
  const floatArray = new Float64Array(intArray.buffer);
  return floatArray[0]
}

function loadKindMap() {
  for (let i = 0; i < 255; i++) {
    let kinda = compiler_exports.kindName(i); // might be stripped
    // let kinda = compiler_exports.kindName(i);
    let kindName = chars(kinda)
    if (!kinda || !kindName) continue
    kinds[kindName] = i
    kinds[i] = kindName
  }
  // debug("kinds:")
  // debug(kinds)
}

function isGroup(kind) {
  if (kind == kinds.group) return true;
  if (kind == kinds.pattern) return true;
  if (kind == kinds.object) return true;//!
  if (kind == kinds.expression) return true;
  // if(this.kind == kinds.list) return true;
  return false;
}

//    32bit in wasm TODO pad with string in 64 bit
class node {
  name = "" // explicit fields yield order
  Kind = ""
  Content = 0
  Childs = []

  constructor(pointer, mem = memory) {
    this.memory = mem // may be lost, or does JS GC keep it?
    this.pointer = pointer
    if (!pointer) return;//throw "avoid 0 pointer node constructor"
    if (typeof pointer === "bigint") {
      if (pointer > 0xFFFFFFFFn) todo("bigint pointer");
      pointer = Number(pointer);
    }
    // check_eq(read_int32(pointer, mem), node_header_32, "read_int32(pointer, mem)==node_header_32 @pointer:" + pointer)
    pointer += 4;
    this.length = read_int32(pointer, mem);
    pointer += 4;
    this.kind = read_int32(pointer, mem);
    pointer += 4;
    // this.type = read_int32(pointer,mem);
    // pointer += wasm_pointer_size;// forced 32 bit,  improved from 'undefined' upon construction
    this.child_pointer = read_int32(pointer, mem);
    pointer += wasm_pointer_size;// LIST, not link. block body content
    // debug(pointer,pointer%8) // must be %8=0 by now
    this.value = read_int64(pointer, mem);// BIGINT!! NOT the same as this.Value() or this.Content !!
    if (typeof this.value === "bigint" && this.value < 0x100000000n)
      this.value = Number(this.value);
    pointer += 8; // value.node and next are NOT REDUNDANT  label(for:password):'Passwort' but children could be merged!?
    this.name = string(pointer, mem);
    // post processing
    this[this.name] = this; // make a:1 / {a:1} indistinguishable
    for (let child of this.children()) {
      if (child.kind === kinds.key)
        this[child.name] = child.Value() // flat values
      else if (child.kind === kinds.long)
        this[child.name] = child.value
      else if (child.kind === kinds.real)
        this[child.name] = child.Content
      else if (child.kind === kinds.string)
        this[child.name] = child.Content
      else
        this[child.name] = child
    }
    this.Kind = kinds[this.kind]
    this.Content = this.Value()
    if (this.length == 0 && this.kind == kinds.key)
      if (this.Content instanceof node)
        this[this.Content.name] = this.Content // make a:b:1 / a:{b:1} indistinguishable
    // pointer += size_of_string; // todo
    // this.meta = 0;
    // pointer += wasm_pointer_size//  LINK, not list. attributes meta modifiers decorators annotations
    // previous fields must be aligned to int64!
  }

  children() {
    let list = []
    let l = this.length;
    let i = 0
    let node_size = compiler_exports.size_of_node()
    while (l-- > 0) {
      list.push(new node(this.child_pointer + i * node_size, this.memory));
      i++
    }
    if (this.length)
      this.Childs = list
    return list
  }

  valueOf() {
    return this.serialize()
  }

  Value() {
    if (this.kind == kinds.nil) return null;
    if (this.kind == kinds.long) return Number(this.value);
    if (this.kind == kinds.bool) return Boolean(this.value);
    if (this.kind == kinds.string) return string(this.value) || this.name;
    if (this.kind == kinds.real) return reinterpretInt64AsFloat64(this.value);
    // if (this.kind == 124) return reinterpretInt64AsFloat64(this.value);
    if (this.kind == kinds.node) return new node(this.value); //.Value();
    if (this.kind == kinds.codepoint) return String.fromCodePoint(this.value)
    if (this.kind == kinds.unknown) return this.Content || this.name;// todo? bug?
    if (this.kind == kinds.reference) return this.Content || this.Childs;
    if (this.kind == kinds.object) return this.Content || this.Childs;
    if (isGroup(this.kind)) return this.Content || this.Childs;
    if (this.kind == 0) return this.Content || this.Childs;// todo null node probably bug!
    if (this.kind == kinds.key) {
      let val = new node(this.value);
      if (val.kind == kinds.string) return string(val.value);// or just name
      return val;
    }
    throw new Error("Node kind not yet supported in js: " + this.kind + " : " + kinds[this.kind] + " value: " + this.value)
    // return this.value;
  }

  serialize() {
    if (!this.pointer) todo("only wasp nodes can be serialized");
    if (!this.memory == memory)
      todo("needs to serialize inside the correct memory!") // app.serialize()
    return chars(compiler_exports.serialize(this.pointer));
  }

  toString() {
    return this.serialize()
  }

  debug() {
    debug(this.serialize());
    debug(this);
    debug(this.children());
    debug(this.name, ":",)
    for (let childe of this.children()) {
      debug(childe.name)
    }
  }
}


// Function to download data to a file
function download_file(data, filename, type) {
  const file = new Blob([data], {type: type});
  if (window.navigator.msSaveOrOpenBlob) // IE10+
    window.navigator.msSaveOrOpenBlob(file, filename);
  else { // Others
    const a = document.createElement("a"),
      url = URL.createObjectURL(file);
    a.href = url;
    a.download = filename;
    document.body.appendChild(a);
    a.click();
    setTimeout(function () {
      document.body.removeChild(a);
      window.URL.revokeObjectURL(url);
    }, 0);
  }
}


// holdup, this list layout is not compatible with the list layout pointer, length, kind, value
function read_array(data, mem) {
  debug("read_array", data)
  let array_header = read_int32(data, mem)
  check(array_header == array_header_32, "array_header_32")
  let kind = read_int32(data + 4, mem)
  let length = read_int32(data + 8, mem)
  let value_kind = read_int32(data + 12, mem)
  let start = data + 16;// continuous mode!  read_int32(data+16, memory)
  debug("array start", start)
  debug("array length", length)
  debug("array kind", kind, kinds[kind])
  debug("value_kind", value_kind, kinds[value_kind])
  let Kind = new node(value_kind, mem)
  debug("Kind", Kind)
  let item_size = 1;
  if (value_kind == kinds.byte) item_size = 1;
  if (value_kind == kinds.short) item_size = 2;
  if (value_kind == kinds.int) item_size = 4;
  if (value_kind == kinds.float) item_size = 4;
  if (value_kind == kinds.long) item_size = 8;
  if (value_kind == kinds.real) item_size = 8;
  if (value_kind == kinds.codepoints) item_size = 4;
  if (length > 1000) throw "array too long"
  let array = []
  let value = 0;
  for (let i = 0; i < length; i++) {
    if (item_size == 1) value = read_byte(start + i * item_size, mem)
    else if (item_size == 4) value = read_int32(start + i * item_size, mem)
    else todo("general array read")
    // todo: serialize as node[] to avoid logic duplication!? space expensive! not THAT many cases (yet?)
    array.push(value)
  }
  debug("array", array)
  return array
}

/**
 * Reads a value from the given smartpointer.
 * ⚠️ as opposed to smartResult which creates a smart pointer
 *
 * @param {any} data0 - The input data to process.
 * @param {number} type - The type of the data (e.g., `int32`).
 * @param {WebAssembly.Memory} memory - The memory context used for processing.
 * @returns {any} The processed smart value.
 */
function smartValue(data0, type /*int32*/, memory) {
// READS smartValue vs smartResult
  // debug("smartNode")
  type = data0 >> BigInt(32) // shift 32 bits ==
  let data = Number(BigInt.asIntN(32, data0))// drop high bits
  if (type == string_header_32 || type == string_header_32 >> 8)
    return string(data, memory) || load_chars(data, length = -1, memory, format = 'utf8')
  if (type == array_header_32 || type == array_header_32 >> 8)
    return read_array(data, memory)
  if (type == 0 && data0 > 0xFFFFFFFFn) return data0
  if (type == 0) return data
  if (type == node_header_32 || type == node_header_32 >> 8)
    return new node(data, memory)
  let nod = new node(compiler_exports.smartNode(data0, memory));
  if (nod.kind == kinds.real || nod.kind == kinds.bool || nod.kind == kinds.long || nod.kind == kinds.codepoint)
    return nod.Value() // primitives independent of memory
  // if (nod.kind == kinds.object)
  //     return new node(nod.value)
  error("TODO emit.wasm values in wasp.wasm for kind " + nod.Kind + " 0x" + hex(type));
  // return {data: data, type: type}
}


// allow wasm tests/plugins to build and execute small wasm files!
// todo while wasp.wasm can successfully execute via run_wasm, it can't handle the result (until async wasm) OR :
// https://web.dev/asyncify/
var expect_test_result = 0; // set before running in semi sync tests!


// todo don't confuse app (wasp runtime) with app_instance (compiled wasm module)
// todo wasm_data = fetch(WASP_FILE) vs runtime_bytes = fetch(WASP_RUNTIME)
// wasp compiler and wasp runtime are different things here!
async function soft_link_runtime() {
  // SOFT LINK TO RUNTIME : just add it in instantiate (vs compile hard link)
  const memory = new WebAssembly.Memory({initial: 16384, maximum: 65536});
  const table = new WebAssembly.Table({initial: 2, element: "anyfunc"});
  try {
    runtime_module = await WebAssembly.compile(runtime_bytes, {builtins: ['js-string']}) // todo: only once for all wasm apps?
    // runtime_imports= {env: {memory: memory, table: table}}
    runtime_imports = imports
    let runtime_instance = await WebAssembly.instantiate(runtime_module, runtime_imports) // , memory
    app_imports = {env: runtime_instance.exports}
    app_instance = await WebAssembly.instantiate(module, imports, runtime_instance.memory)
    app_exports = app_instance.exports
    //  runtime_instance.exports
    // global.
    app_instance = app_instance
    app_instance.module = module // global module already used by node!
    app_exports = app_exports
    main = app_exports.wasp_main
    if (main) {
      debug("Calling start function:", main);
      result = main()
    } else
      result = "NO MAIN! Entry function main not found."
    print(result)
  } catch (ex) {
    console.error((ex));
    return
  }
}


function moduleReflection(wasm_data) {
  WabtModule().then(wabt => {
    let module = wabt.readWasm(wasm_data, {readDebugNames: true});
    module.generateNames();
    module.applyNames();
    let output = module.toText({foldExprs: false, inlineExport: false});
    debug(output);
  })
}

let bigHex = x => x >= 0n ? x.toString(16) : (0xFFFFFFFFn + x + 1n).toString(16);
let hex = x => typeof x === 'bigint' ? bigHex(x) : x >= 0 ? x.toString(16) : (0xFFFFFFFF + x + 1).toString(16)
let chr = x => String.fromCodePoint(x) // chr(65)=chr(0x41)='A' char
// debug wasm memory
function binary_diff(old_mem, new_mem) {
  debug("binary_diff")
  old_mem = new Uint8Array(old_mem, 0, old_mem.length)
  new_mem = new Uint8Array(new_mem, 0, new_mem.length)
  if (old_mem.length != new_mem.length)
    debug("old_mem.length!=new_mem.length", old_mem.length, new_mem.length);
  let badies = 0;
  for (let i = 0; i < old_mem.length && badies < 1000; i++) {
    let x = old_mem[i];
    let y = new_mem[i];
    if (x && x != y) {
      debug("DIFF AT", i, ":", hex(x), hex(y), " ", chr(x), chr(y))
      badies++
    }
  }
}

// minimal demangler for wasm functions does not offer full reflection
getArguments = function (func) {
  const symbols = func.toString();
  let start, end, register;
  // debug("getArguments",func,symbols)
  // start = symbols.indexOf('function');
  // if (start !== 0 && start !== 1) return undefined;
  start = symbols.indexOf('(', start);
  end = symbols.indexOf(')', start);
  const args = [];
  symbols.substring(start + 1, end - start - 1).split(',').forEach(function (argument) {
    args.push(argument);
  });
  return args;
};


// reflection of all wasp functions to compiler, so it can link them
function register_wasp_functions(exports) {
  // console_log("⚠️ register_wasp_functions DEACTIVATED")
  // return
  exports = exports || compiler_instance.exports || runtime_exports
  for (let name in exports) {
    if (name.startsWith("_Zl")) continue // operator"" literals
    let func = exports[name]
    if (typeof func == "function") {
      // WebAssembly.Function({parameters: ["i32"], results: ["i32"]}, func)

      let demangled = demangle(name)
      // console_log(func.name, func)
      // if(demangled == "loor")
      //   debug(name, "⇨", demangled)
      exports[demangled] = func
      if (!demangled.match("<") && !demangled.match("\\[")
        && !demangled.match(":: ") && !demangled.match("~"))// no generics yet
        compiler_instance.exports.registerWasmFunction(chars(demangled), chars(name)) // derive signature from name
    }
  }
}

function addSynonyms(exports) {
  for (let name in exports) {
    let func = exports[name]
    Wasp[name] = func
    if (typeof func == "function") {
      let demangled = demangle(name)
      if (demangled != name) {
        // if (!exports[demangled]) {
        exports[demangled] = func
        Wasp[demangled] = func // keep signature for polymorphic calls
        const short = demangled.substr(0, demangled.lastIndexOf("("));
        Wasp[short] = func
      }
    }
  }
  return exports
}

// runtime_bytes for linking small wasp programs with runtime. The result SHOULD BE standalone wasm!
// this is NOT NEEDED when
// 1. the wasm module is already standalone and doesn't import any functions from the runtime
// 2. the host environment uses the full wasp.wasm (with compiler) as runtime
function load_runtime_bytes() {
  if (runtime_bytes) return warn("runtime_bytes already loaded")
  fetch(WASP_RUNTIME).then(resolve => resolve.arrayBuffer()).then(buffer => {
      runtime_bytes = buffer
    if (typeof (compiler_exports) == 'undefined')
        console.error("compiler needs to be loaded before runtime")
      copy_runtime_bytes_to_compiler()
      WebAssembly.instantiate(runtime_bytes, imports).then(obj => {
        runtime_instance = obj.instance
        runtime_exports = runtime_instance.exports
        // addSynonyms(runtime_exports) // better use full names from compiler!
        wasp_ready()
        // debug(obj.instance.exports._ZN6StringC2EPKcb)
        // getArguments(obj.instance.exports._ZN6StringC2EPKcb)
      }).catch(err => {
        error(err)
      })
    }
  )
}


// let compiler link/merge emitted wasm with small runtime
function copy_runtime_bytes_to_compiler() {
  syncHeap()
  let length = runtime_bytes.byteLength
  let pointer = HEAP_END
  let src = new Uint8Array(runtime_bytes, 0, length);
  let dest = new Uint8Array(memory.buffer, HEAP_END, length);
  dest.set(src) // memcpy ⚠️ todo MAY FUCK UP compiler bytes!!!
  print("HEAP", compiler_exports.getHeapEnd(), HEAP_END);
  HEAP_END += length
  syncHeap()
  // return // ⚠️
  HEAP_END += 5000000 // extra space for demangle Todo ⚠️ re-check if parsed Module ≈5MB !
  print("HEAP BEFORE parseRuntime", compiler_exports.getHeapEnd(), HEAP_END);
  compiler_exports.parseRuntime(pointer, length) // sets HEAP_END too!
  print("HEAP AFTER parseRuntime", compiler_exports.getHeapEnd(), HEAP_END);
  syncHeap()
}

function syncHeap() {
  neu = compiler_exports.getHeapEnd()
  if (neu > HEAP_END) HEAP_END = neu
  else compiler_exports.setHeapEnd(HEAP_END) // resync!
}

// todo: CONFUSION between runtime, compiler and app!
// todo: unified/multi memory possible without components?
// WHICH memory, HEAP etc… => node() string() of which memory ??
// todo let app IMPORT its memory when using runtime as a module!
// todo let app EXPORT its memory when being standalone!
// todo how can IMPORTED memory be cleanly initialized with data?
function load_compiler() {
  if (typeof compiler_exports !== 'undefined') {
    debug("compiler_exports already loaded")
    return
  }
  WASP_COMPILER_BYTES = fetch(WASP_COMPILER)
  WebAssembly.instantiateStreaming(WASP_COMPILER_BYTES, imports).then(obj => {
      compiler_instance = obj.instance
      compiler_exports = compiler_instance.exports
      // global.
      addSynonyms(compiler_exports)
    // runtime_exports = compiler_exports
      HEAP = compiler_exports.__heap_base; // ~68000
      DATA_END = compiler_exports.__data_end
      HEAP_END = HEAP || DATA_END || runtime_exports.__heap_end;
      HEAP_END += 0x100000
      memory = compiler_exports.memory || compiler_exports._memory || memory
      buffer = new Uint8Array(memory.buffer, 0, memory.length);
      main = compiler_instance.start || compiler_exports.teste || compiler_exports.main || compiler_exports.wasp_main || compiler_exports._start
      main = compiler_instance._Z11testCurrentv || main
      if (main) {
        debug("got main")
        result = main()
      } else {
        console.error("missing main function in wasp module!")
        result = compiler_instance.exports//show what we've got
      }
    debug(result);
    compiler_ready()
    }
  ).catch(err => {
    console.error(err)
  })
}

async function run_wasm(buf_pointer, buf_size) {
  try { // WE WANT PURE STACK TRACE
    wasm_buffer = buffer.subarray(buf_pointer, buf_pointer + buf_size)
    // wasm_to_wat(wasm_buffer)
    // download_file(wasm_buffer, "compiled.wasm", "wasm")

    app_module = await WebAssembly.compile(wasm_buffer, {builtins: ['js-string']})
    if (WebAssembly.Module.imports(app_module).length > 0) {
      needs_runtime = true // todo WASI or Wasp runtime?
      print(app_module) // visible in browser console, not in terminal
      Wasp.download = download
      // print(WebAssembly.Module.customSections(app_module)) // Argument 1 is required ?
    } else
      needs_runtime = false

    if (needs_runtime) { // runtime_bytes should be linked by/inside compiler!
      app = await WebAssembly.instantiate(wasm_buffer, imports, memory) // todo: tweaked imports if it calls out
      // app = await WebAssembly.instantiate(wasm_buffer, runtime_imports, runtime_instance.memory) // todo: tweaked imports if it calls out
      print("compiled wasm app/script loaded")
    } else {
      let memory2 = new WebAssembly.Memory({initial: 10, maximum: 65536});// pages à 2^16 = 65536 bytes
      app = await WebAssembly.instantiate(wasm_buffer, imports, memory2) // todo: tweaked imports if it calls out
    }
    app.exports = app.instance.exports
    app.memory = app.exports.memory || app.exports._memory || app.memory
    let main = app.exports.wasp_main || app.exports.main || app.instance.start || app.exports._start
    results.value = "" // reset error messages
    let result = main()
    debug("GOT raw ", hex(result))
    if (result < -0x100000000 || result > 0x100000000) {
      if (!app.memory)
        error("NO app.memory")
      result = smartValue(result, 0, app.memory)
      //  result lives in emit.wasm!
      debug("GOT node ", result)
      // result = nod.Value()
    }
    if (expect_test_result) {
      debug("EXPECT", expect_test_result, "GOT", result) //  RESULT FROM emit.WASM
      if (Array.isArray(expect_test_result) && Array.isArray(result)) {
        for (let i = 0; i < result.length; i++)
          check(+expect_test_result[i] == +result[i])
      } else if (expect_test_result != result) {
        STOP = 1
        download_file(wasm_buffer, "emit.wasm", "wasm") // resume
        check(expect_test_result == result)
      }
      expect_test_result = 0
      if (resume) setTimeout(resume, 1);
    }
    results.value += "\nresult:\n"
    if (typeof results != "undefined") //
      results.value += result
    results.value += "\n" // JSON.stringify( Do not know how to serialize a BigInt
    return result; // useless, returns Promise!
  } catch (ex) {
    //throw new Error(`Error in run_wasm: ${error.message}\nStack: ${error.stack}`);
    print(app.exports)
    console.error(ex)
    error(ex)
  }
}


// function wasm_to_wat(buffer) {
//   try {
//     const parsed = window.WebAssemblyParser2.decode(buffer);
//     debug(parsed)
//     // editor.setValue(parsed.toText());
//
//     wabtFeatures = {
//       exceptions: true,
//       mutable_globals: true,
//       sat_float_to_int: true,
//       sign_extension: true,
//       simd: true,
//       threads: true,
//       multi_value: true,
//       tail_call: true,
//       bulk_memory: true,
//       reference_types: true,
//       annotations: true,
//       gc: true,
//       memory64: true,
//     }
//
//     const module = wabt.readWasm(buffer, {readDebugNames: true}, wabtFeatures);
//     module.generateNames();
//     module.applyNames();
//     const result = module.toText({foldExprs: true, inlineExport: true});
//     // debug(result);
//     editor.setValue(result)
//   } catch (e) {
//     debug("wasm_to_wat error")
//     debug(e) // but don't overwrite the editor
//   }
// }


function readFile() {// upload via classic html, not wasp
  debug("readFile")
  const file = input_file.files[0]
  const reader = new FileReader
  reader.addEventListener('load', () => {
    debug("readFile load")
    debug(reader.result)
    if (typeof code_input !== 'undefined')
      code_input.innerHTML = reader.result
    if (typeof editor !== 'undefined')
      editor.setValue(reader.result)
  })
  reader.readAsText(file, 'UTF-8')
}


// var work = new Worker("wasp_tests.js");
// work.postMessage({ a:8, b:9 });
// work.onmessage = (evt) => { debug(evt.data); };

async function test() {
  try {
    if (typeof (WebAssembly.promising) != 'undefined') {
      // The WebAssembly.promising function takes a WebAssembly function, as exported by a WebAssembly instance, and returns a JavaScript function that returns a Promise. The returned Promise will be resolved by the result of invoking the exported WebAssembly function.
      var test_async = WebAssembly.promising(exports.test_async)
      test_async().then(result => {
        debug("test_async result", result)
      })
    }
    if (typeof (wasp_tests) !== "undefined")
      await wasp_tests() // internal tests of the wasp.wasm runtime FROM JS! ≠
  } catch (x) {
    if (x instanceof YieldThread)
      debug("⚠️ CANNOT USE assert_emit in wasp_tests() or testCurrent() ONLY via testRun()")
    throw x
  }
}

function compiler_ready() {
  if (!use_big_runtime) try {
    load_runtime_bytes()
  } catch (x) {
    error("CANNOT LOAD runtime_bytes")
  }
  else wasp_ready()
}

function wasp_ready() {
  debug("wasp is ready")
  // moduleReflection(wasm_data);
  loadKindMap()
  try {
    register_wasp_functions(compiler_instance.exports)
  } catch (err) {
    error(err)
  }
  // testRun1()
  if (run_tests)
    setTimeout(test, 1);// make sync
}

load_compiler()
