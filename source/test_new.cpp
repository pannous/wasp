#include "Util.h"
#include "List.h"
#include "Node.h"
#include "Wasp.h"
#include "Angle.h"
#include "String.h"
#include "Map.h"
#include "tests.h"
#include "asserts.h"

// New isolated test functions that can be run individually via CTest

void test_basic_arithmetic() {
    print("Testing basic arithmetic...");
    assert_emit("2+3", 5);
    assert_emit("10-3", 7);
    assert_emit("4*5", 20);
    assert_emit("15/3", 5);
    print("✓ Basic arithmetic tests passed");
}

void test_string_operations() {
    print("Testing string operations...");
    assert_emit("'say ' + 0.", "say 0.");
    assert_emit("'hello'", "hello"); // char comparison for now
    assert_emit("`${1+1}`", 2);
    print("✓ String operations tests passed");
}

void test_function_definitions() {
    print("Testing function definitions...");
    assert_emit("def add(a,b): a+b; add(2,3)", 5);
    assert_emit("def square(x): x*x; square(4)", 16);
    print("✓ Function definition tests passed");
}

void test_variables() {
    print("Testing variables...");
    assert_emit("x=42; x", 42);
    assert_emit("y=3; y", 3);
    print("✓ Variable tests passed");
}

void test_fibonacci() {
    print("Testing fibonacci...");
    assert_emit("fib := it < 2 ? it : fib(it - 1) + fib(it - 2); fib(10)", 55);
    print("✓ Fibonacci tests passed");
}

// Main test runner that can run all tests or individual tests
int main(int argc, char **argv) {
    print("Running new isolated tests...");
    // working dir :  $CMakeCurrentLocalGenerationDir$ ?
    try {
        if (argc > 1) {
            // Run specific test
            String test_name = argv[1];
            if (test_name == "arithmetic") {
                test_basic_arithmetic();
            } else if (test_name == "strings") {
                test_string_operations();
            } else if (test_name == "functions") {
                test_function_definitions();
            } else if (test_name == "variables") {
                test_variables();
            } else if (test_name == "fibonacci") {
                test_fibonacci();
            } else {
                print("Unknown test: "s + test_name);
                print("Available tests: arithmetic, strings, functions, variables, fibonacci");
                return 1;
            }
        } else {
            // Run all tests
            test_basic_arithmetic();
            test_string_operations();
            test_function_definitions();
            test_variables();
            test_fibonacci();
        }
        
        print("All tests passed!");
        return 0;
    } catch (const char *err) {
        print("Test failed with error: ");
        print(err);
        return 1;
    } catch (String err) {
        print("Test failed with error: ");
        print(err);
        return 1;
    } catch (const Error &err) {
        print("Test failed with error: ");
        print(err.message);
        return 1;
    }
}