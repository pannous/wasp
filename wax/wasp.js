// let WASM_FILE = 'merged.wasm'
let WASM_FILE = 'wasp.wasm'
// let WASM_FILE = 'hello-wasi.wasm'
// let WASM_FILE = 'test-printf.wasm'
// let WASM_FILE = '../cmake-build-wasm/wasp.wasm'
// let WASM_FILE='../cmake-build-release/wasp.wasm'

var buffer;

let min_memory_size = 100 // in 64k PAGES! 65536 is upper bound => 64k*64k=4GB
let max_memory_size = 65536 // in 64k PAGES! 65536 is upper bound => 64k*64k=4GB
let memory = new WebAssembly.Memory({initial: min_memory_size, maximum: max_memory_size});

let nop = x => 0 // careful, some wasi shim needs 0!
const fd_write = function (fd, c_io_vector, iovs_count, nwritten) {
    while (iovs_count-- > 0) {
        if (fd === 0)
            console.error(String(c_io_vector) || "\n");
        else
            console.log(String(c_io_vector) || "\n");
        c_io_vector += 8
    }
    return -1; // todo
};

let imports = {
    env: {
        run_wasm, // allow wasm modules to run plugins / compiler output
        init_graphics: nop, // canvas init by default
        requestAnimationFrame: nop,
    },
    wasi_unstable: {
        fd_write,
        proc_exit: terminate,
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
let puts = x => console.log(string(x)) // char*
let prints = x => console.log(String(x)) // char**
const console_log = window.console.log;// redirect to text box
window.console.log = function (...args) {
    console_log(...args);
    args.forEach(arg => results.value += `${JSON.stringify(arg)}\n`);
}

function check(ok) {
    if (!ok) throw new Error("test failed");
}

function String(data) { // wasm<>js interop
    switch (typeof data) {
        case "string":
            let p = STACK
            new_int(STACK + 8)
            new_int(data.length)
            chars(data);
            return p;
        case "bigint":
        case "number":
        default:
            return string(read_int32(data), read_int32(data + 4))
    }
}

function string(pointer, length = -1, format = 'utf8') {
    if (length < 0) { // auto length
        while (buffer[pointer + ++length]) ;
    }
    //console.log("length:",length,"format:",format,"pointer:",pointer,"TextDecoder:",typeof(TextDecoder))
    if (typeof (TextDecoder) != 'undefined') {// WEB, text-encoding, Node 11
        const utf8_decoder = new TextDecoder('utf8');
        let decoder = format == 'utf8' ? utf8_decoder : utf16denoder
        let arr = new Uint8Array(buffer.subarray(pointer, pointer + length));
        return decoder.decode(arr)
    } else { // fallback
        buf = Buffer.from(memory.buffer) // Node only!
        buf = buf.slice(pointer, pointer + length)
        s = buf.toString(format)// utf8 or 'utf16le'
        return s
    }
}

function new_int(val) {
    while (STACK % 4) STACK++;
    let buf = new Uint32Array(memory.buffer, STACK, memory.length);
    buf[0] = val
    STACK += 4
}

function new_long(val) {
    let buf = new Uint64Array(memory.buffer, STACK / 8, memory.length);
    buf[0] = val
    STACK += 8
}

function read_int32(pointer) { // little endian
    buffer = new Uint8Array(memory.buffer, 0, memory.length);
    return buffer[pointer + 3] * 2 ** 24 + buffer[pointer + 2] * 256 * 256 + buffer[pointer + 1] * 256 + buffer[pointer];
}// todo: like this:

function read_int64(pointer) {
    buffer = new BigInt64Array(memory.buffer, pointer, memory.length);
    return buffer[0]
}


function chars(s) {
    if (!s) return 0;// MAKE SURE!
    current = STACK;
    const uint8array = new TextEncoder("utf-8").encode(s + "\0");
    buffer.set(uint8array, current);
    STACK += uint8array.length;
    return current;
}

let str = chars

function reset_heap() {
    HEAP = exports.__heap_base; // ~68000
    DATA_END = exports.__data_end
    STACK = HEAP || DATA_END;
}

function compile_and_run(code) {
    // reset at each run, discard previous data!
    reset_heap();
    exports.run(chars(code));
}

let wasm_pointer_size = 4;// 32 bit
let node_header_32 = 0x80000000
let size_of_string = 16;// todo
//    32bit in wasm TODO pad with string in 64 bit
class node {
    name = ""

    constructor(pointer) {
        this.pointer = pointer
        if (!pointer) return;//throw "avoid 0 pointer node constructor"
        check(read_int32(pointer) == node_header_32)
        pointer += 4;
        this.length = read_int32(pointer);
        pointer += 4;
        this.kind = read_int32(pointer);
        pointer += 4;
        // this.type = read_int32(pointer);
        // pointer += wasm_pointer_size;// forced 32 bit,  improved from 'undefined' upon construction
        this.child_pointer = read_int32(pointer);
        pointer += wasm_pointer_size;// LIST, not link. block body content
        // console.log(pointer,pointer%8) // must be %8=0 by now
        this.value = parseInt(read_int64(pointer));
        pointer += 8; // value.node and next are NOT REDUNDANT  label(for:password):'Passwort' but children could be merged!?
        this.name = String(pointer);
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
            list.push(new node(this.child_pointer + i * node_size));
            i++
        }
        return list
    }

    serialize() {
        if (!this.pointer) todo("only wasp nodes can be serialized");
        return string(exports.serialize(this.pointer));
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


function parse(data) {
    let node_pointer = exports.Parse(chars(data))// also calls run()!
    let nod = new node(node_pointer);
    nod.debug()
}

let Backtrace = function (print = 1) {
    try {
        throw new Error()
    } catch (ex) {
        //trimStack(ex, 1)
        if (print) console.error(ex); else return trimStack(ex)
    }
}

function terminate() {
    console.log("wasm terminate()")
    // if(sure)throw
}


// allow wasm tests/plugins to build and execute small wasm files!
// todo while wasp.wasm can successfully execute via run_wasm, it can't handle the result (until async wasm) OR :
// https://web.dev/asyncify/
async function run_wasm(buf_pointer, buf_size) {
    let wasm_buffer = buffer.subarray(buf_pointer, buf_pointer + buf_size)
    let memory2 = new WebAssembly.Memory({initial: 10, maximum: 65536});// pages à 2^16 = 65536 bytes
    // funclet.table = new WebAssembly.Table({initial: 2, element: "anyfunc"});
    let funclet = await WebAssembly.instantiate(wasm_buffer, imports, memory2)
    // funclet.instance = WebAssembly.instantiate(funclet.module, imports, funclet.memory)
    funclet.exports = funclet.instance.exports
    // funclet.memory = funclet.exports.memory || funclet.exports._memory || funclet.memory
    // funclet.buffer = new Uint8Array(funclet.memory.buffer, 0, memory.length);
    let main = funclet.instance.start || funclet.exports.main || funclet.exports.wasp_main || funclet.exports._start
    let result = main()
    console.log("GOT RESULT FROM WASM")
    console.log(parseInt(result))
    return result; // returns Promise ! Do not know how to serialize a BigInt
}

WebAssembly.instantiateStreaming(fetch(WASM_FILE), imports).then(obj => {
        instance = obj.instance
        exports = instance.exports
        HEAP = exports.__heap_base; // ~68000
        DATA_END = exports.__data_end
        STACK = HEAP || DATA_END;
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
        test()
    }
)


function test() {
    // exports.testCurrent()  // internal tests of the wasp.wasm runtime INSIDE WASM
    if (typeof (wasp_tests) !== "undefined")
        wasp_tests() // internal tests of the wasp.wasm runtime FROM JS! ≠

    let nod = parse("a : (b ,c)")
    prints(exports.serialize(nod))
    // let cmd="puts 'CYRC!'"
    // let cmd="puti 123"
    let cmd = "123"
    let ok = exports.run(chars(cmd))
    console.log(string(ok))
    ok = exports.testJString(String("FULL circle"))
    console.log(String(ok))
    exports._Z7println6String(String("full circle"))
}
