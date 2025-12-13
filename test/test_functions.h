#pragma once

// Test functions to check wasm->runtime interaction

static int test42() {
    return 42;
}

static int test42i(int i) {
    // used in wasm runtime test
    return 42 + i;
}

static float test42f(float f) {
    return 42 + f;
}

// default args don't work in wasm! (how could they?)
static float test41ff(float f = 0) {
    return 41.4 + f;
}

static void not_ok() {
    error1(""); // assert_throws test
}
