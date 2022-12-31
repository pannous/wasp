// let WASM_FILE = 'merged.wasm'
let WASM_FILE = 'wasp.wasm'
let WASM_RUNTIME = 'wasp-runtime.wasm'
// let WASM_FILE = 'hello-wasi.wasm'
// let WASM_FILE = 'test-printf.wasm'
// let WASM_FILE = '../cmake-build-wasm/wasp.wasm'
// let WASM_FILE='../cmake-build-release/wasp.wasm'

// MAX_MEM is NOT affected by -Wl,--initial-memory=117964800 NOR by this: HOW THEN??
let min_memory_size = 100 // in 64k PAGES! 65536 is upper bound => 64k*64k=4GB
let max_memory_size = 65536 // in 64k PAGES! 65536 is upper bound => 64k*64k=4GB
let memory = new WebAssembly.Memory({initial: min_memory_size, maximum: max_memory_size});

// this memory object is NEVER USED if wasm file does not import memory and provides its own, hopefully exported!

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

function memcpy(src, srcOffset, dst, dstOffset, length) {
    src = src.subarray || src.slice ? src : src.buffer
    dst = dst.subarray || dst.slice ? dst : dst.buffer
    src = srcOffset ? src.subarray ?
        src.subarray(srcOffset, length && srcOffset + length) :
        src.slice(srcOffset, length && srcOffset + length) : src
    if (dst.set) {
        dst.set(src, dstOffset)
    } else {
        for (var i = 0; i < src.length; i++) {
            dst[i + dstOffset] = src[i]
        }
    }
    return dst
}


function wasp_module_reflection(bufp, sizep) {
    let length = runtime_bytes.byteLength
    while (heap_end % 8) heap_end++;
    let buf = heap_end
    let dest = new Uint32Array(memory.buffer, heap_end, length);
    memcpy(runtime_bytes, 0, dest, 0, length)
    set_int(bufp, heap_end);
    set_int(sizep, length);
    heap_end += length
    return buf;
}


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
        run_wasm, // allow wasm modules to run plugins / compiler output
        assert_expect: x => expect_test_result = new node(x).Value(),
        async_yield: x => { // called from inside wasm, set callback handler resume before!
            throw new YieldThread() // unwind wasm, reenter through resume() after run_wasm
        },
        init_graphics: nop, // canvas init by default
        requestAnimationFrame: nop,
        powi: (x, y) => x ** y,
        puti: x => console.log(x), // allows debugging of ints without format String allocation!
        wasp_module_reflection
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
let puts = x => console.log(chars(x)) // char*
let prints = x => console.log(string(x)) // char**
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
            return chars(read_int32(data), read_int32(data + 4))
    }
}

function load_chars(pointer, length = -1, format = 'utf8') {
    if (pointer === 0) return
    if (typeof pointer == "string") return chars(s)
    // console.log("string",pointer,length)
    if (length < 0) { // auto length
        let buffer = new Uint8Array(memory.buffer, pointer, memory.length);
        while (buffer[++length]) ;// strlen ;)
    }
    //console.log("length:",length,"format:",format,"pointer:",pointer,"TextDecoder:",typeof(TextDecoder))
    if (typeof (TextDecoder) != 'undefined') {// WEB, text-encoding, Node 11
        const utf8_decoder = new TextDecoder('utf8');
        let decoder = format == 'utf8' ? utf8_decoder : utf16denoder
        let arr = new Uint8Array(memory.buffer, pointer, length);
        return decoder.decode(arr)
    } else { // fallback
        buf = Buffer.from(memory.buffer) // Node only!
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

function read_int32(pointer) { // little endian
    buffer = new Uint8Array(memory.buffer, 0, memory.length);
    return buffer[pointer + 3] * 2 ** 24 + buffer[pointer + 2] * 256 * 256 + buffer[pointer + 1] * 256 + buffer[pointer];
}// todo: like this:

function read_int64(pointer) {
    buffer = new BigInt64Array(memory.buffer, pointer, memory.length);
    return buffer[0]
}


var run = 1;

function reset_heap() {
    HEAP = exports.__heap_base; // ~68000
    DATA_END = exports.__data_end
    heap_end = HEAP || DATA_END;
    heap_end += 0x100000 * run++; // todo
}

function compile_and_run(code) {
    // reset at each run, discard previous data!
    // reset_heap(); NOT COMPATIBLE WITH ASYNC CALLS!
    exports.run(chars(code));
}

let wasm_pointer_size = 4;// 32 bit
let node_header_32 = 0x80000000

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

//    32bit in wasm TODO pad with string in 64 bit
class node {
    name = "" // explicit fields yield order
    Kind = ""
    Content = 0
    Childs = []

    constructor(pointer) {
        this.pointer = pointer
        if (!pointer) return;//throw "avoid 0 pointer node constructor"
        check(read_int32(pointer) == node_header_32, "node_header_32")
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
        this.name = string(pointer);
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
            list.push(new node(this.child_pointer + i * node_size));
            i++
        }
        if (this.length)
            this.Childs = list
        return list
    }

    Value() {
        if (this.kind == kinds.string) return string(this.value);
        if (this.kind == kinds.long) return Number(this.value);
        if (this.kind == kinds.bool) return Boolean(this.value);
        if (this.kind == kinds.real) return reinterpretInt64AsFloat64(this.value);
        if (this.kind == kinds.node) return new node(this.value);
        if (this.kind == kinds.reference) return this.Content || this.Childs;
        if (this.kind == kinds.object) return this.Content || this.Childs;
        if (this.kind == kinds.group) return this.Content || this.Childs;
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


// allow wasm tests/plugins to build and execute small wasm files!
// todo while wasp.wasm can successfully execute via run_wasm, it can't handle the result (until async wasm) OR :
// https://web.dev/asyncify/
var expect_test_result = 42; // set before running in semi sync tests!
async function run_wasm(buf_pointer, buf_size) {
    wasm_buffer = buffer.subarray(buf_pointer, buf_pointer + buf_size)
    // download(wasm_buffer, "emit.wasm", "wasm")
    let memory2 = new WebAssembly.Memory({initial: 10, maximum: 65536});// pages à 2^16 = 65536 bytes
    let funclet = await WebAssembly.instantiate(wasm_buffer, imports, memory2) // todo: tweaked imports if it calls out
    funclet.exports = funclet.instance.exports
    // funclet.memory = funclet.exports.memory || funclet.exports._memory || funclet.memory
    let main = funclet.exports.wasp_main || funclet.exports.main || funclet.instance.start || funclet.exports._start
    let result = main()
    if (result > 0x100000000)
        result = new node(exports.smartNode(result)).Value()
    console.log("EXPECT", expect_test_result, "GOT", result) //  RESULT FROM WASM
    if (expect_test_result || 1) {
        if (expect_test_result != result) {
            STOP = 1
            download(wasm_buffer, "emit.wasm", "wasm") // resume
        }
        check(expect_test_result == result)
        expect_test_result = 0
        if (resume) setTimeout(resume, 1);
    }
    return result; // useless, returns Promise!
}

wasm_data = fetch(WASM_FILE)
WebAssembly.instantiateStreaming(wasm_data, imports).then(obj => {
        instance = obj.instance
        exports = instance.exports
        HEAP = exports.__heap_base; // ~68000
        DATA_END = exports.__data_end
    heap_end = HEAP || DATA_END;
    heap_end += 0x10000000
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
        loadKindMap()
        setTimeout(test, 1);// make sync
    }
)

// runtime_bytes for linking small wasp programs with runtime
function load_runtime_bytes() {
    fetch(WASM_RUNTIME).then(resolve => resolve.arrayBuffer()).then(buffer => {
            runtime_bytes = buffer
            console.log(runtime_bytes)
            console.log(runtime_bytes.byteLength)
        }
    )
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

var STOP = 0

async function test() {
    if (typeof (wasp_tests) !== "undefined")
        wasp_tests() // internal tests of the wasp.wasm runtime FROM JS! ≠
    return
    let cs = chars("abcd")
    copy_of_last_state = memory.buffer.slice(0, memory.length);
    exports._Z7reversePci(cs, 4)
    binary_diff(copy_of_last_state, memory.buffer)
    return
    // var work = new Worker("wasp_tests.js");
    // work.postMessage({ a:8, b:9 });
    // work.onmessage = (evt) => { console.log(evt.data); };
    // resume=test // call this again once current test in wasp_tests is done!

    // if(code_input)
    //     compile_and_run(code_input.value);// execute index.html code input
    try {
        while (!STOP) {
            // console.log("starting new testRunAsync")
            // reset_heap()
            copy_of_last_state = memory.buffer.slice(0, memory.length);
            await testRunAsync()
            await new Promise(sleep => setTimeout(sleep, 10));
        }
    } catch (x) {
        // binary_diff(copy_of_last_state,memory.buffer)
        STOP = 1
        throw x;
    }
}
