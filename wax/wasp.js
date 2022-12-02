// let WASM_FILE = 'merged.wasm'
let WASM_FILE = 'wasp.wasm'
// let WASM_FILE = '../cmake-build-wasm/wasp.wasm'
// let WASM_FILE='../cmake-build-release/wasp.wasm'

var buffer;

const console_log = window.console.log;
window.console.log = function (...args) {
    console_log(...args);
    args.forEach(arg => results.value += `${JSON.stringify(arg)}\n`);
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

function tests() {
    exports._Z11testCurrentv()
}

function compile_and_run(code) {
    let node = exports._Z7compile6String(code, 1)// also calls run()!
    console.log(node)
}

function run_wasm(pointer, length) {
    let app = new Uint8Array(buffer.subarray(pointer, pointer + length));
    const app_module = new WebAssembly.Module(app);
    // code_instance ≠ wasp instance
    const code_instance = WebAssembly.instantiate(app_module, imports);// await
    const code_exports = code_instance.exports;
    main = code_exports.start || code_exports.teste || code_exports.main || code_exports._start
    if (main) {
        console.log("got main")
        result = main()
    } else result = instance.exports//show what we've got
    console.log(result);
    //alert(result)
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
}

function memset() {
    console.log("TODO, use provided wasp implementation!!")
}

let nop = x => x

puts = x => console.log(string(x))

let min_memory_size = 65536 / 64 // in 64k PAGES! 65536 is upper bound => 64k*64k=4GB
let memory_size = 65536 // in 64k PAGES! 65536 is upper bound => 64k*64k=4GB
let memory = new WebAssembly.Memory({initial: min_memory_size, maximum: memory_size});
imports = {
    env: {
        memset,
        atoi: x => parseInt(x),
        puti: x => console.log(x),
        puts,
        logi: x => console.log(x),
        logc: x => console.log(String.fromCodePoint(x)),
        logs: x => console.log(string(x)),
        put_char: x => console.log(String.fromCodePoint(x)),
        printf: x => console.log(string(x)),// todo

        _Z13init_graphicsv: nop, // canvas init by default
        _Z21requestAnimationFramev: nop,
        __cxa_allocate_exception: nop,
        __cxa_guard_acquire: nop,
        __cxa_guard_release: nop,
        __cxa_throw: puts,
        __cxa_begin_catch: x => log("caught c++ exception", x),
        __cxa_demangle: nop,
        _ZSt9terminatev: terminate,
        __cxa_atexit: terminate,
        panic: terminate,
        raise: terminate,
        _Z5raisePKc: terminate, // why mangled double?
        pow: Math.pow,
        _ZdlPv: nop,//operator delete(void*) lol
        _Z7consolev: nop, // no repl for now
        _Z11testCurrentv: nop, // internal tests only during compiler development 

    },
    wasi_unstable: {
        proc_exit: terminate
    }
}

// This is googles recommended way of loading WebAssembly.
// try {
// const response = await fetch('wasp.wasm')
// const module = new WebAssembly.Module(buffer);
//const module = await WebAssembly.compileStreaming(fetch('wasp.wasm'));
// const instance = await WebAssembly.instantiate(module,});
WebAssembly.instantiateStreaming(fetch(WASM_FILE), imports).then(obj => {
        instance = obj.instance
        exports = instance.exports
        memory = exports.memory || exports._memory || memory
        buffer = new Uint8Array(memory.buffer, 0, memory.length);
        main = instance.start || exports.teste || exports.main || exports._start
        // _Z11testCurrentv
        if (main) {
            console.log("got main")
            result = main()
        } else result = instance.exports//show what we've got
        console.log(result);
        console.log(exports)

        //alert(result)
        // tests()
    }
)


function test() {
    console.log("exports")
    console.log(exports)
}

// } catch (error) {
//     console.log(error)
//     console.log("\n RUN as\n sudo python3 -m http.server")
// }
