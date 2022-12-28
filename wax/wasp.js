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

imports = {
    env: {
        run_wasm,
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

let puts = x => console.log(string(x))
const console_log = window.console.log;// redirect to text box
window.console.log = function (...args) {
    console_log(...args);
    args.forEach(arg => results.value += `${JSON.stringify(arg)}\n`);
}

function String(pointer) {
    switch (typeof pointer) {
        case "string":
            let p = STACK
            new_int(STACK + 8)
            new_int(pointer.length)
            chars(pointer);
            return p;
        case "bigint":
        case "number":
        default:
            return string(int32(pointer), int32(pointer + 4))
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

function int32(pointer) { // little endian
    buffer = new Uint8Array(memory.buffer, 0, memory.length);
    return buffer[pointer + 3] * 2 ** 24 + buffer[pointer + 2] * 256 * 256 + buffer[pointer + 1] * 256 + buffer[pointer];
}


function chars(s) {
    if (!s) return -1; //0 // 0 = 0 MAKE SURE!
    current = STACK;
    const uint8array = new TextEncoder("utf-8").encode(s + "\0");
    buffer.set(uint8array, current);
    STACK += uint8array.length;
    return current;
}

let str = chars

function compile_and_run(code) {
    exports.run(chars(code));
    // let node = exports._Z7compile6Stringb(chars(code), 1)// also calls run()!
    // console.log(node)
}

function parse(data) {
    let node = exports._Z5parse6String13ParserOptions(chars(data), 1)// also calls run()!
    console.log(node)
}

var stdout = '';
write = function (s) {
    stdout += s;
    for (var i; -1 !== (i = stdout.indexOf('\n')); stdout = stdout.slice(i + 1))
        console.log(stdout.slice(0, i));
};

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
    // exports.testCurrent()
    // let cmd="puts 'CYRC!'"
    // let cmd="puti 123"
    let cs = chars("abcd")
    exports._Z7reversePci(cs, 4)
    puts(cs)
    console.log(string(cs));
    let cmd = "123"
    let ok = exports.run(chars(cmd))
    console.log(string(ok))
    ok = exports.testJString(String("FULL circle"))
    console.log(String(ok))
    exports._Z7println6String(String("full circle"))
}
