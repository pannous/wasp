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
    let cmd = "42*2/2"
    let result = exports.run(chars(cmd))
    console.log(chars(result)) // "need asyncify for result" ;)
    expect_test_result = 42;
    // check(result==42);
    // exports._Z7println6String(Str("full circle"))
}

function testRun() {
    try {
        resume = testRun // comeback here after first, 2ⁿᵈ … testRun
        exports.testRun();// going from one test to the next WITHIN WASP!
    } catch (x) {
        if (x instanceof YieldThreadException) {
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
    testRun()
}
