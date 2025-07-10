#include "test_common.h"

void test_string_basics() {
    print("Testing string basics ...");
    assert_emit("'hello'", "hello"); // char comparison for now
    print("✓ String operations tests passed");
}

void test_string_operations() {
    print("Testing string operations...");
    assert_emit("'say ' + 0.", "say 0.");
    assert_emit("'hello'", "hello"); // char comparison for now
    assert_emit("`${1+1}`", 2);
    print("✓ String operations tests passed");
}


int main(int argc, char **argv) {
    print("Running string tests...");
    try {
        test_string_operations();
        print("All string tests passed successfully.");
    } catch (const std::exception &e) {
        printf("string tests failed: %s,",e.what());
        return 1; // Indicate failure
    }
    return 0; // Indicate success
}