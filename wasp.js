identity = x => x

function resizeCanvas() {
    canvas.width = window.innerWidth || document.documentElement.clientWidth || document.body.clientWidth;
    canvas.height = window.innerHeight || document.documentElement.clientHeight || document.body.clientHeight;
    WIDTH = canvas.width;
    HEIGHT = canvas.height;
    imageData = ctx.createImageData(WIDTH, HEIGHT);
}

function loop() {
    paint()
    window.requestAnimationFrame(loop);
}


function paint() {
    let data = imageData.data
    for (var i = 0; i < data.length; i += 4) {
        data[i] = Math.random() * 255 // red
        data[i + 1] = Math.random() * 255 // green
        data[i + 2] = Math.random() * 255 // blue
        data[i + 3] = 255 // else transparent!!!
    }
    ctx.putImageData(imageData, 0.5, 0.5);
}


function readFile() {// via classic html, not wasp
    const file = input_file.files[0]
    const reader = new FileReader
    reader.addEventListener('load', () => {
        code_input.innerHTML = reader.result
    })
    reader.readAsText(file, 'UTF-8')
}

function print(...x) {
    log(...x);// to stdout and window title!
    console.log(...x);
    results.value += x + "\n"
}


ignore = x => x
nop = x => ignore(x)

let memory_size = 1024// pages à 64kb // 4GB!
// let memoryBase = 1024 * 16
// let table_size = 294

let memory = new WebAssembly.Memory({initial: memory_size, maximum: 65336});

function loadBytes(byte_start, len) {
    print("TODO loadBytes")
    return new Uint8Array(start);
}

function trimStack(x) {
    return x
}

let backtrace = function (print = 1) {
    try {
        throw new Error()
    } catch (ex) {
        if (print) console.error(trimStack(ex, 1)); else return trimStack(ex)
    }
}

function raise(msg) {
    print(msg)
    exit(0)
}

let memset = function (ptr, value, num) {
    memory[ptr] = value// todo SHOULD BE DONE INSIDE WASM!?
    log('todo("memset")', ptr, value, num)
    raise("memset should be part of wasp!?!")
}
let paintWasmToCanvas = (offset = 0) => {
    let len = canvas.width * canvas.height * 4
    data = new Uint8Array(memory.buffer, offset, len + offset);
    // data = new Uint8Array(memory.buffer, 0, memory.buffer.byteLength);
    // imageData.data.set(new Uint8ClampedArray(memory.buffer, 0,10000 ));// memory.length));
    for (var i = 0; i < data.length; i += 4) {
        // data[i] = Math.random() * 255
        data[i + 3] = 255;//Math.random() * 255
    }
    imageData.data.set(data);
    ctx.putImageData(imageData, 0, 0);
}
let terminate = console.log // todo: what else?
let todo_remove = x => console.log("TODO todo_remove ", x)
let imports = {// todo : reduce to absolute minium
    wasi_unstable: {proc_exit: terminate},
    env: {
        memory,
        // table,
        square: x => x * x, // usually native wasm, better test with 'grow'!
        pow: Math.pow, // usually wasp lib, better test other function
        powd: Math.pow,
        powf: Math.pow,
        powi: (a, b) => {
            res = a;
            while (b-- > 1) res *= a;
            return BigInt(res);
        },
        _Z3powdd: Math.pow,
        atoi: x => parseInt(x),
        _Z10precedence6String: todo_remove,
        fopen: nop,// WASI
        fseek: nop,
        ftell: nop,
        fread: nop,
        system: nop,
        access: nop,
        _Z5raisePKc: print,
        _Z5tracePKc: nop,
        _ZdlPv: nop,//operator delete(void*) lol
        _ZSt9terminatev: terminate,
        panic: terminate,
        raise,
        __cxa_begin_catch: nop,
        __cxa_atexit: print,
        __cxa_demangle: identity,
        __cxa_guard_acquire: nop,//!?
        __cxa_guard_release: nop,//

        put_char: print,
        puts: print,
        puti: print,
        putf: print,
        putc: print,
        logc: print,
        logs: print,
        logi: print, // puti vs math logarithm: !!
        logf: Math.log,
        logd: Math.log,
        printf: print,
        puts: print,
        putchar: print,
        memset,
        paint: paintWasmToCanvas,
        init_graphics: nop, // canvas init'ed by default
        requestAnimationFrame: paintWasmToCanvas,
        _Z21requestAnimationFramev: nop, // why sometimes mangled?
        _Z13init_graphicsv: nop,
        _Z7consolev: print, //huh?
        _Z8run_wasmPhi: (bytes, len) => wasmx(loadBytes(bytes, len)),// full circle yay!
        ext_memcpy: nop,
        sum: (x, y) => x + y,
    }
};

async function load_wasp_runtime() {
    const module = await WebAssembly.compileStreaming(fetch('wasp_runtime.wasm'));
    const instance = await WebAssembly.instantiate(module, imports, memory);
    memory = instance.memory || module.memory || instance.exports.memory || memory
}


async function load_wasp_compiler() {
    const module = await WebAssembly.compileStreaming(fetch('wasp_compiler.wasm'));
    wasp_compiler = await WebAssembly.instantiate(module, imports, memory);
    wasp_compiler_memory = wasp_compiler.memory || module.memory || wasp_compiler.exports.memory || memory
}

function compile(wasp_string) {
    let charged = wasp_compiler.exports.analyze(wasp_string);// Node &
    // let {length, type, binary} = load_wasp_compiler.exports.emit(charged);// todo multi-value c++
    let code = load_wasp_compiler.exports.emit(charged);// Code
    let length = wasp_compiler_memory[code + 8]
    let data = wasp_compiler_memory[code + 12]
    let offset = data;
    let binary = new Uint8Array(wasp_compiler_memory.buffer, offset, length + offset);
    // data = new Uint8Array(memory.buffer, 0, memory.buffer.byteLength);
    return binary;
}

async function run(wasp_string) {
    let binary = compile(wasp_string)
    wasm_done = result => results.text = result
    execute(binary) // async 
}

let wasm_done = x => x // unused callback

async function execute(code) {
    try {
        print("running wasm generated by wasp compiler");
        let module = await WebAssembly.compile(code)// global
        const instance = await WebAssembly.instantiate(module, imports, memory);
        memory = instance.memory || module.memory || instance.exports.memory || memory
        if (instance.exports.main) {
            result = instance.exports.main()
            print(result)
            wasm_done(result);
        } else {
            wasm_error("NO MAIN!");
        }
    } catch (error) {
        print('COMPILE ERROR', error.message)
        wasm_error(error.message);
    }
}

window.onload = async function () {
    await load_wasp_compiler();
    resizeCanvas();
    let code = compile(code_input.value);
    execute(code);
};

