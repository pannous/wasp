let print = console.log // #!

function check(ok) {
    backtrace_line("⚠️ TEST FAILED")
    if (!ok) throw "⚠️ TEST FAILED"
}

function backtrace_line(msg) {
    function getErrorObject() {
        try {
            throw Error('')
        } catch (err) {
            return err;
        }
    }

    var err = getErrorObject();
    var caller_line = err.stack.split("\n")[msg ? 4 : 3];
    if (msg) console.log(msg)
    let short = caller_line.replace(/.*wax\//, "").replace(")", "");
    console.log(msg ? msg : "", short, caller_line);
}


function testParse() {
    let nod = parse("a : (b ,c)")
    prints(exports.serialize(nod))
    check(nod.name = "a")
    check(nod.length = 2)
    check(nod.children().length = 2)
    check(nod.children()[1].name == "c")
}

function testString() {
    backtrace_line();
    let ok = exports.testFromJS(String("test from JS"));
    console.log(String(ok))
    prints(ok)
    check(String(ok) == "ok from WASP")
    console.log("TEST OK")
}

function testReverse() {
    backtrace_line();
    exports._Z7println6String(String("full circle"))
    exports.put_chars(chars("abcdefg"))
    let cs = chars("abcd")
    puts(cs)
    exports._Z7reversePci(cs, 4)
    puts(cs)
    check(string(cs) == "dcba")
    console.log(string(cs));
}

function testRun() {
    // let cmd="puts 'CYRC!'"
    // let cmd="puti 123"
    let cmd = "42"
    let result = exports.run(chars(cmd))
    console.log(string(result)) // "need asyncify for result" ;)
    expect_test_result = 42;
    // check(result==42);
    // exports._Z7println6String(String("full circle"))
}

function wasp_tests() {
    console.log("wasp_tests")
    // exports.puts(chars("JAAA"))
    // backtrace_line();
    // exports.testCurrent()  // internal tests of the wasp.wasm runtime INSIDE WASM
    // testString();
    testReverse();
    // testParse();
    // testRun()
}
