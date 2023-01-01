// can be run as a worker!

// class YieldThread { // unwind wasm, reenter through resume() after run_wasm finished
// }

// (A) ON RECEIVING DATA FROM "MAIN PAGE"
onmessage = (evt) => {
    // (A1) DO PROCESSING
    console.log("Worker has received data");
    console.log(evt.data);
    testRun()
    var result = parseInt(evt.data.a) + parseInt(evt.data.b);
    // (A2) RESPOND BACK TO "MAIN PAGE"
    postMessage(result);
};

// (B) OPTIONAL - HANDLE ERRORS
onmessageerror = (err) => {
    console.log(err);
};
onerror = (err) => {
    console.log(err);
};

let print = console.log // #!

function check(ok) {
    backtrace_line("⚠️ TEST FAILED")
    if (!ok) throw "⚠️ TEST FAILED"
}


function getBacktrace() {
    try {
        throw Error('')
    } catch (err) {
        return err;
    }
}

function backtrace_line(msg) {
    var err = getBacktrace();
    var caller_line = err.stack.split("\n")[msg ? 4 : 3];
    if (msg) console.log(msg)
    let short = caller_line.replace(/.*wax\//, "").replace(")", "");
    console.log(msg ? msg : "", short, caller_line);
}


function testParse() {
    let a = parse("a : (b ,c)")
    prints(exports.serialize(a))
    // nod.debug()
    check(a.name == "a")
    check(a.length = 2)
    check(a.children().length = 2)
    check(a.children()[1].name == "c")
    console.log("TEST OK: testParse")
    a = parse("a : 123")
    check(a.value === 123);
    check(a.Value() === 123);
    a = parse("a : 123.5")
    check(a.Value() === 123.5);
    check(a.Content === 123.5);
    // nod = parse("a : 123.4")
    // check(nod.Value()==123.4);// 123.39999999999418 WTH!!
    a = parse("a:'ok'")
    check(a.Value() === 'ok');
    a = parse("{a:'ok'}")
    check(a.Value() == 'ok');
    a = parse("a : {b:'ok'}")
    check(a.b.Value() == 'ok');
    // check(a.b == 'ok'); // todo immediate key->key flattening
    a = parse("a : {b:'hold…' c:{d=42.0,e='😼'}}")
    console.log(a.b)
    console.log(a.c)
    check(a.c.d === 42)
    check(a.c.e === '😼'); // flat keys!
    console.log("TEST OK: testParse")
}

function testString() {
    backtrace_line();
    let ok = exports.testFromJS(string("test from JS"));
    console.log(string(ok))
    prints(ok)
    check(string(ok) === "ok from WASP")
    console.log("TEST OK: testString")
}

function testReverse() {
    let cs = chars("abcd")
    exports._Z7reversePci(cs, 4)
    check(chars(cs) === "dcba")
    exports.put_string(string("abcd -> dcba full circle"))
    console.log("TEST OK: testReverse")
}

function testRun1() {
    // let cmd="puts 'CYRC!'"
    // let cmd="puti 123"
    // let cmd = "√3^2"
    // let cmd = "123"
    let cmd = "42*2/2"
    let result = exports.run(chars(cmd))
    // console.log(chars(result)) // "need asyncify for result" ;)
}

async function testRunAsync() {
    try {
        let result = await exports.testRun();
        console.log("GOT FINAL RESULT", result)
        resume = true // tests done!
    } catch (x) {
        if (x instanceof YieldThread) {
            // print("test thread yielded, re-entering after run_wasm finished")
        } else {
            throw x;
        }
    }
}

function testRun() {
    try {
        // resume = testRun // comeback here after first, 2ⁿᵈ … testRun
        exports.testRun();// going from one test to the next WITHIN WASP!
    } catch (x) {
        if (x instanceof YieldThread) {
            // print("test thread yielded, re-entering after run_wasm finished")
        } else throw x;
    }
}

function wasp_tests() {
    console.log("wasp_tests")
    // exports.puts(chars("JAAA"))
    // backtrace_line();
    testString();
    testReverse();
    testParse();
    exports.testCurrent()  // internal tests of the wasp.wasm runtime INSIDE WASM
    if (0)
        testRun1();// ONLY ONE !
    else
        testRun(); // don't do both!
}
