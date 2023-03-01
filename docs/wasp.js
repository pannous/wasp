// let WASP_FILE = 'merged.wasm'
let Wasp={}
let WASP_FILE = 'wasp.wasm'
let WASP_RUNTIME = 'wasp-runtime.wasm'
// let WASP_FILE = 'hello-wasi.wasm'
// let WASP_FILE = 'test-printf.wasm'
// let WASP_FILE = '../cmake-build-wasm/wasp.wasm'
// let WASP_FILE='../cmake-build-release/wasp.wasm'

// MAX_MEM is NOT affected by -Wl,--initial-memory=117964800 NOR by this: HOW THEN??
let min_memory_size = 100 // in 64k PAGES! 65536 is upper bound => 64k*64k=4GB
let max_memory_size = 65536 // in 64k PAGES! 65536 is upper bound => 64k*64k=4GB
let memory = new WebAssembly.Memory({initial: min_memory_size, maximum: max_memory_size});
// this memory object is NEVER USED if wasm file does not import memory and provides its own, hopefully exported!

let wasm_pointer_size = 4;// 32 bit

// see [smart-pointers](https://github.com/pannous/wasp/wiki/smart-pointer)
let string_header_32 = 0x10000000
let array_header_32 = 0x40000000
let node_header_32 = 0x80000000


function format(object) {
    if (object instanceof node)
        return object.serialize()
    if (typeof (object) == 'object')
        return JSON.stringify(object);
    return object;
}

function error(msg) {
    // throw new Error("⚠️ ERROR: " + msg)
    throw new Error("ERROR: " + msg)
}

let nop = x => 0 // careful, some wasi shim needs 0!

const fd_write = function (fd, c_io_vector, iovs_count, nwritten) {
    while (iovs_count-- > 0) {
        if (fd === 0)
            console.error(string(c_io_vector) || "\n");
        else
            console.log(string(c_io_vector) || "\n");
        c_io_vector += 8
    }
    return -1; // todo
};


function parse(data) {
    let node_pointer = exports.Parse(chars(data))// also calls run()!
    let nod = new node(node_pointer);
    return nod
}

function terminate() {
    console.log("wasm terminate()")
    // if(sure)throw
}

var resume; // callback function resuming after run_wasm finished
class YieldThread { // unwind wasm, reenter through resume() after run_wasm finished
}

let imports = {
    env: { // MY_WASM custom wasp helpers
        memory, // optionally provide js Memory … alternatively use exports.memory in js, see below
        heap_end: new WebAssembly.Global({value: "i32", mutable: true}, 0),// todo: use as heap_end
        grow_memory: x => memory.grow(1), // à 64k … NO NEED, host grows memory automagically!
        run_wasm: async (x, y) => {
            try {
                return await run_wasm(x, y)
            } catch (ex) {
                wasm_buffer = buffer.subarray(x, x + y)
                download(wasm_buffer, "emit.wasm", "wasm") // resume
                error(ex)
            }
        }, // allow wasm modules to run plugins / compiler output
        assert_expect: x => {
            if (expect_test_result)
                error("already expecting value " + expect_test_result + " -> " + x)
            expect_test_result = new node(x).Value()
        },
        async_yield: x => { // called from inside wasm, set callback handler resume before!
            throw new YieldThread() // unwind wasm, reenter through resume() after run_wasm
        },
        init_graphics: nop, // canvas init by default
        requestAnimationFrame: nop,
        exit: terminate, // should be wasi.proc_exit!
        pow: (x, y) => x ** y,
        puti: x => console.log(x), // allows debugging of ints without format String allocation!
        js_demangle: x => x,
        _Z7compile6Stringb: nop, // todo bug! why is this called?
        _Z9read_wasmPhi: nop, // todo bug! why is this called?
        _Z11loadRuntimev: nop, // todo bug! why is this called?
        _Z10testWasmGCv: nop, // todo bug! why is this called?
        _Z11testAllWasmv: nop, // todo bug! why is this called?
        _Z11testAllEmitv: nop, // todo bug! why is this called?
        _Z12testAllAnglev: nop, // todo bug! why is this called?
    },
    wasi_unstable: {
        fd_write,
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

let todo = x => console.error("TODO", x)
let puts = x => console.log(chars(x)) // char*
let prints = x => console.log(string(x)) // char**
const print = window.console.log;// redirect to text box
const console_log = window.console.log;// redirect to text box

// #if not TRACE
// redirect after testing!
// window.console.log = function (...args) {
//     console_log(...args);
//     args.forEach(arg => results.value += `${format(arg)}\n`);
// }

function check(ok, msg) {
    if (!ok) throw new Error("⚠️ TEST FAILED! " + (msg || ""));
}

function debugMemory(pointer, num, mem) {
    if (!mem) mem = memory
    let buffer = new Uint8Array(mem.buffer, pointer, num);
    let i = 0
    while (num-- > 0) {
        let x = buffer[i++]
        if (i < 3 || x)
            console.log("CHAR AT", i, hex(x), chr(x), x)
    }
}

function string(data) { // wasm<>js interop
    switch (typeof data) {
        case "string":
            while (heap_end % 8) heap_end++
            let p = heap_end
            new_int(heap_end + 8)
            new_int(data.length)
            chars(data);
            return p;
        case "bigint":
        case "number":
        default:
            let pointer = read_int32(data);
            let length = read_int32(data + 4);
            if (!pointer) {
                console.log("NO chars to read")
                debugMemory(data - 10, 20)
            }
            let cs = load_chars(pointer, length);
            return cs
    }
}

function load_chars(pointer, length = -1, format = 'utf8', module_memory) {
    if (!module_memory) module_memory = memory
    if (pointer === 0) return
    if (typeof pointer == "string") return chars(s)
    // console.log("string",pointer,length)
    if (length < 0) { // auto length
        let buffer = new Uint8Array(module_memory.buffer, pointer, module_memory.length);
        while (buffer[++length]) ;// strlen ;)
    }
    //console.log("length:",length,"format:",format,"pointer:",pointer,"TextDecoder:",typeof(TextDecoder))
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

function chars(data) {
    if (!data) return 0;// MAKE SURE!
    if (typeof data != "string") return load_chars(data)
    const uint8array = new TextEncoder("utf-8").encode(data + "\0");
    buffer = new Uint8Array(memory.buffer, heap_end, uint8array.length);
    buffer.set(uint8array, 0);
    let c = heap_end
    heap_end += uint8array.length;
    return c;
}

function set_int(address, val) {
    let buf = new Uint32Array(memory.buffer, address, 4);
    buf[0] = val
}

function new_int(val) {
    while (heap_end % 4) heap_end++;
    let buf = new Uint32Array(memory.buffer, heap_end, 4);
    buf[0] = val
    heap_end += 4
}

function new_long(val) {
    let buf = new Uint64Array(memory.buffer, heap_end / 8, memory.length);
    buf[0] = val
    heap_end += 8
}

function read_byte(pointer, mem) {
    if (!mem) mem = memory
    // return mem.buffer[pointer]
    let buffer = new Int8Array(mem.buffer, pointer, 1);
    return buffer[0]
}

function read_int32(pointer, mem) {
    if (!mem) mem = memory
    buffer = new Uint8Array(mem.buffer, 0, mem.length);
    return buffer[pointer + 3] * 2 ** 24 + buffer[pointer + 2] * 256 * 256 + buffer[pointer + 1] * 256 + buffer[pointer];
}

// function read_int32(pointer, mem) {
// ONLY WORKS if pointer is 4 byte aligned!
// todo align ALL ints …!
//     if (!mem) mem = memory
// start offset of Int32Array should be a multiple of 4
//     let buffer = new Int32Array(mem.buffer, pointer, 4);
//     return buffer[0]
// }

function read_int64(pointer, mem) { // little endian
    if (!mem) mem = memory
    let buffer = new BigInt64Array(mem.buffer, pointer, 8);
    return buffer[0]
}

// reset at each run, discard previous data!
// NOT COMPATIBLE WITH ASYNC CALLS!
function reset_heap() {
    HEAP = exports.__heap_base; // ~68000
    DATA_END = exports.__data_end
    heap_end = HEAP || DATA_END;
    heap_end += 0x100000 * run++; // todo
}

function compile_and_run(code) {
    // reset_heap();
    expect_test_result = false
    exports.run(chars(code));
}

function reinterpretInt64AsFloat64(n) { // aka reinterpret_cast long to double
    const intArray = new BigInt64Array(1);
    intArray[0] = BigInt(n)
    const floatArray = new Float64Array(intArray.buffer);
    return floatArray[0]
}

let kinds = {}

function loadKindMap() {
    for (let i = 0; i < 255; i++) {
        let kinda = exports.kindName(i);
        let kindName = chars(kinda)
        if (!kinda || !kindName) continue
        kinds[kindName] = i
        kinds[i] = kindName
    }
    // console.log("kinds:")
    // console.log(kinds)
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

    constructor(pointer, mem) {
        if (!mem) mem = memory
        this.memory = mem // may be lost, or does JS GC keep it?
        this.pointer = pointer
        if (!pointer) return;//throw "avoid 0 pointer node constructor"
        check(read_int32(pointer, mem) == node_header_32, "node_header_32")
        pointer += 4;
        this.length = read_int32(pointer, mem);
        pointer += 4;
        this.kind = read_int32(pointer, mem);
        pointer += 4;
        // this.type = read_int32(pointer,mem);
        // pointer += wasm_pointer_size;// forced 32 bit,  improved from 'undefined' upon construction
        this.child_pointer = read_int32(pointer, mem);
        pointer += wasm_pointer_size;// LIST, not link. block body content
        // console.log(pointer,pointer%8) // must be %8=0 by now
        this.value = parseInt(read_int64(pointer, mem));
        pointer += 8; // value.node and next are NOT REDUNDANT  label(for:password):'Passwort' but children could be merged!?
        this.name = string(pointer, mem);
        // post processing
        this[this.name] = this; // make a:1 / {a:1} indistinguishable
        for (var child of this.children()) {
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
        let node_size = exports.size_of_node()
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
        if (this.kind == kinds.string) return string(this.value) || this.name;
        if (this.kind == kinds.long) return Number(this.value);
        if (this.kind == kinds.bool) return Boolean(this.value);
        if (this.kind == kinds.real) return reinterpretInt64AsFloat64(this.value);
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
            todo("needs to serialize inside the correct memory!") // funclet.serialize()
        return chars(exports.serialize(this.pointer));
    }

    toString() {
        return this.serialize()
    }

    debug() {
        console.log(this.serialize());
        console.log(this);
        console.log(this.children());
        console.log(this.name, ":",)
        for (var childe of this.children()) {
            console.log(childe.name)
        }
    }
}


// Function to download data to a file
function download(data, filename, type) {
    var file = new Blob([data], {type: type});
    if (window.navigator.msSaveOrOpenBlob) // IE10+
        window.navigator.msSaveOrOpenBlob(file, filename);
    else { // Others
        var a = document.createElement("a"),
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
    console.log("read_array", data)
    let array_header = read_int32(data, mem)
    check(array_header == array_header_32, "array_header_32")
    let kind = read_int32(data + 4, mem)
    let length = read_int32(data + 8, mem)
    let value_kind = read_int32(data + 12, mem)
    let start = data + 16;// continuous mode!  read_int32(data+16, memory)
    console.log("array start", start)
    console.log("array length", length)
    console.log("array kind", kind, kinds[kind])
    console.log("value_kind", value_kind, kinds[value_kind])
    let Kind = new node(value_kind, mem)
    console.log("Kind", Kind)
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
    console.log("array", array)
    return array
}

function smartNode(data0, type /*int32*/, memory) {
    // console.log("smartNode")
    type = data0 >> BigInt(32) // shift 32 bits ==
    let data = Number(BigInt.asIntN(32, data0))// drop high bits
    if (type == string_header_32 || type == string_header_32 >> 8)
        return load_chars(data, length = -1, format = 'utf8', memory)
    if (type == array_header_32 || type == array_header_32 >> 8)
        return read_array(data, memory)
    let nod = new node(exports.smartNode(data0));
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


async function link_runtime() {
    const memory = new WebAssembly.Memory({initial: 16384, maximum: 65536});
    const table = new WebAssembly.Table({initial: 2, element: "anyfunc"});
    try {
        runtime_module = await WebAssembly.compile(runtime_bytes)
        // runtime_imports= {env: {memory: memory, table: table}}
        runtime_imports = imports
        let runtime_instance = await WebAssembly.instantiate(runtime_module, runtime_imports) // , memory
        app_imports = {env: runtime_instance.exports}
        app_instance = await WebAssembly.instantiate(module, imports, runtime_instance.memory)
        app_exports = instance.exports
        //  runtime_instance.exports
        global.app_instance = instance
        global.app_instance.module = module // global module already used by node!
        global.app_exports = app_exports
        main = app_exports.wasp_main
        if (main) {
            console.log("Calling start function:", main);
            result = main()
        } else
            result = "NO MAIN! Entry function main not found."
        print(result)
    } catch (ex) {
        console.error((ex));
        return
    }
}

var needs_runtime = false;
var RUNTIME_BYTES = null
var app_module

async function run_wasm(buf_pointer, buf_size) {
    let wasm_buffer = buffer.subarray(buf_pointer, buf_pointer + buf_size)
    // download(wasm_buffer, "emit.wasm", "wasm")

    app_module = await WebAssembly.compile(wasm_buffer)
    if (WebAssembly.Module.imports(app_module).length > 0) {
        needs_runtime = true
        print(app_module) // visible in browser console, not in terminal
        print(WebAssembly.Module.customSections(app_module))
        STOP = 1
        return
    } else
        needs_runtime = false

    if (needs_runtime) {
        print("needs_runtime runtime loading")
        if (!RUNTIME_BYTES) // Cannot compile WebAssembly.Module from an already read Response TODO reuse!
            RUNTIME_BYTES = await fetch(WASP_RUNTIME)
        let runtime_instance = await WebAssembly.instantiateStreaming(RUNTIME_BYTES, {
            wasi_unstable: {
                fd_write,
                proc_exit: terminate, // all threads
                args_get: nop, // ignore the rest for now
                args_sizes_get: x => 0,
            },
        })
        print("runtime loaded")
        // addSynonyms(runtime_instance.exports)
        // let runtime_imports = {env: runtime_instance.exports}
        let runtime_imports = {env: {square: x => x * x, pow: Math.pow}}
        app = await WebAssembly.instantiate(wasm_buffer, runtime_imports, runtime_instance.memory) // todo: tweaked imports if it calls out
        print("app loaded")

    } else {
        let memory2 = new WebAssembly.Memory({initial: 10, maximum: 65536});// pages à 2^16 = 65536 bytes
        app = await WebAssembly.instantiate(wasm_buffer, imports, memory2) // todo: tweaked imports if it calls out
    }
    app.exports = app.instance.exports
    app.memory = app.exports.memory || app.exports._memory || app.memory
    let main = app.exports.wasp_main || app.exports.main || app.instance.start || app.exports._start
    let result = main()
    // console.log("GOT raw ", result)
    if (-0x100000000 > result || result > 0x100000000) {
        if (!app.memory)
            error("NO funclet.memory")
        result = smartNode(result, 0, app.memory)
        //  result lives in emit.wasm!
        // console.log("GOT nod ", nod)
        // result = nod.Value()
    }
    console.log("EXPECT", expect_test_result, "GOT", result) //  RESULT FROM emit.WASM
    if (expect_test_result) {
        if (Array.isArray(expect_test_result) && Array.isArray(result)) {
            for (let i = 0; i < result.length; i++)
                check(+expect_test_result[i] == +result[i])
        } else if (expect_test_result != result) {
            STOP = 1
            download(wasm_buffer, "emit.wasm", "wasm") // resume
            check(expect_test_result == result)
        }
        expect_test_result = 0
        if (resume) setTimeout(resume, 1);
    }
    results.value = result // JSON.stringify( Do not know how to serialize a BigInt
    return result; // useless, returns Promise!
    // } catch (ex) {
    //     console.error(ex)
    // }
}

wasm_data = fetch(WASP_FILE)
WebAssembly.instantiateStreaming(wasm_data, imports).then(obj => {
    instance = obj.instance
    exports = instance.exports
    addSynonyms(exports)
    HEAP = exports.__heap_base; // ~68000
    DATA_END = exports.__data_end
    heap_end = HEAP || DATA_END;
    heap_end += 0x100000
    memory = exports.memory || exports._memory || memory
    buffer = new Uint8Array(memory.buffer, 0, memory.length);
    main = instance.start || exports.teste || exports.main || exports.wasp_main || exports._start
        main = instance._Z11testCurrentv || main
        if (main) {
            console.log("got main")
            result = main()
        } else {
            console.error("missing main function in wasp module!")
            result = instance.exports//show what we've got
        }
    console.log(result);
    wasp_ready()
    }
)

// .catch(err => console.error(err))

function moduleReflection(wasm_data) {
    WabtModule().then(wabt => {
        let module = wabt.readWasm(wasm_data, {readDebugNames: true});
        module.generateNames();
        module.applyNames();
        let output = module.toText({foldExprs: false, inlineExport: false});
        console.log(output);
    })
}

let hex = x => x >= 0 ? x.toString(16) : (0xFFFFFFFF + x + 1).toString(16)
let chr = x => String.fromCodePoint(x) // chr(65)=chr(0x41)='A' char
function binary_diff(old_mem, new_mem) {
    console.log("binary_diff")
    old_mem = new Uint8Array(old_mem, 0, old_mem.length)
    new_mem = new Uint8Array(new_mem, 0, new_mem.length)
    if (old_mem.length != new_mem.length)
        console.log("old_mem.length!=new_mem.length", old_mem.length, new_mem.length);
    var badies = 0
    for (let i = 0; i < old_mem.length && badies < 1000; i++) {
        let x = old_mem[i];
        let y = new_mem[i];
        if (x && x != y) {
            console.log("DIFF AT", i, ":", hex(x), hex(y), " ", chr(x), chr(y))
            badies++
        }
    }
}


function copy_runtime_bytes() {
    let length = runtime_bytes.byteLength
    let pointer = heap_end
    let src = new Uint8Array(runtime_bytes, 0, length);
    let dest = new Uint8Array(memory.buffer, heap_end, length);
    dest.set(src) // memcpy
    heap_end += length
    exports.testRuntime(pointer, length)
}

getArguments = function (func) {
    var symbols = func.toString(), start, end, register;
    // console.log("getArguments",func,symbols)
    // start = symbols.indexOf('function');
    // if (start !== 0 && start !== 1) return undefined;
    start = symbols.indexOf('(', start);
    end = symbols.indexOf(')', start);
    var args = [];
    symbols.substr(start + 1, end - start - 1).split(',').forEach(function (argument) {
        args.push(argument);
    });
    return args;
};


// reflection of all wasp functions to compiler, so it can link them
function register_wasp_functions(exports) {
    // console_log("⚠️ register_wasp_functions DEACTIVATED")
    // return
    exports = exports || instance.exports
    // console_log(demangle("_Z6printfPKcS0_i"))
    // console_log(demangle("_ZN6StringC2EPKhj"))
    // console_log(demangle("_ZN6StringC2EPhj"))
    // console_log(demangle("_Z2If4NodeS_"))
    // return
    for (let name in exports) {
        if (name.startsWith("_Zl")) continue // operator"" literals
        let func = exports[name]
        if (typeof func == "function") {
            // WebAssembly.Function({parameters: ["i32"], results: ["i32"]}, func)

            let demangled = demangle(name)
            // console_log(func.name, func)
            // console.log(name, "⇨", demangled)
            exports[demangled] = func
            if (!demangled.match("<") && !demangled.match("\\[")
                && !demangled.match(":: ") && !demangled.match("~"))// no generics yet
                instance.exports.registerWasmFunction(chars(demangled), chars(name)) // derive signature from name
        }
    }
}

function addSynonyms(exports) {
    for (let name in exports) {
        let func = exports[name]
        Wasp[name] = func
        if (typeof func == "function") {
            let demangled = demangle(name)
            if(demangled!= name){
            // if (!exports[demangled]) {
                exports[demangled] = func
                Wasp[demangled] = func
                var short=demangled.substr(0,demangled.lastIndexOf("("))
                Wasp[short] = func
                if(name.match("reverse")){
                    print("reverse",name,demangled,short)
                }
            }
        }
    }
    return exports
}

// runtime_bytes for linking small wasp programs with runtime
function load_runtime_bytes() {
    fetch(WASP_RUNTIME).then(resolve => resolve.arrayBuffer()).then(buffer => {
        runtime_bytes = buffer
        WebAssembly.instantiate(runtime_bytes, imports).then(obj => {
            //  (func (;5;) (type 5) (param i32 i32 i32) (result i32)
            // console.log(obj.instance.exports._ZN6StringC2EPKcb)
            // console.log(obj.instance.exports._ZN6StringC2EPKcb.length)
            // console.log(obj.instance.exports._ZN6StringC2EPKcb.arguments)
            // console.log(obj.instance.exports._ZN6StringC2EPKcb.getArguments())
            // getArguments(obj.instance.exports._ZN6StringC2EPKcb)
        })
        // copy_runtime_bytes()
        }
    )
}


// var work = new Worker("wasp_tests.js");
// work.postMessage({ a:8, b:9 });
// work.onmessage = (evt) => { console.log(evt.data); };

async function test() {
    try {
        if (typeof (wasp_tests) !== "undefined")
            await wasp_tests() // internal tests of the wasp.wasm runtime FROM JS! ≠
    } catch (x) {
        if (x instanceof YieldThread)
            console.log("⚠️ CANNOT USE assert_emit in wasp_tests() or testCurrent() ONLY via testRun()")
        throw x
    }
}

function wasp_ready() {
    console.log("wasp is ready")
    // moduleReflection(wasm_data);
    loadKindMap()
    // load_runtime_bytes()
    register_wasp_functions(instance.exports)
    // testRun1()
    setTimeout(test, 1);// make sync
}
