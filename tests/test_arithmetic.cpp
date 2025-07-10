#include "test_common.h"

void test_arithmetic() {
    print("Testing basic arithmetic...");
    assert_emit("2+3", 5);
    assert_emit("10-3", 7);
    assert_emit("4*5", 20);
    assert_emit("15/3", 5);
    print("✓ Basic arithmetic tests passed");
}


void test_harder_arithmetic() {
    print("Testing harder arithmetic...");
    assert_emit("2+3*4", 14); // precedence
    assert_emit("10-3*2", 4); // precedence
}


int main(int argc, char **argv) {
    print("Running arithmetic tests...");
    test_arithmetic(); // May fail with error
    return 0; // Indicate success
}
