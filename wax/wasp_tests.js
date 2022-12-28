function check(ok) {
    if (!ok) throw "test failed"
}

function wasp_tests() {
    let cs = chars("abcd")
    exports._Z7reversePci(cs, 4)
    puts(cs)
    check(string(cs) == "dcba")
    console.log(string(cs));
}
