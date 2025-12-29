# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Overview

Wasp is a unified programming language that combines a new notation for both markup/object data and code.
It serves as the foundation for the higher-order programming language Angle. The project includes:

- **Wasp**: A unified data notation (superset of JSON5 with arbitrary braces (),[],{} …)
- **Angle**: A programming language using Wasp as its data format
- **WebAssembly compilation**: Compiles to efficient WASM modules

## Build System and Common Commands

The project uses CMake as the primary build system with extensive configuration options.

### Quick Start Build

```bash
# Build the project (uses cmake)
# Build with different configurations (via CMake profiles)
cmake -DCMAKE_BUILD_TYPE=Debug
cmake -DCMAKE_BUILD_TYPE=Release
cmake -DRELEASE=1 -DNO_TESTS=1
```
these can be found in `cmake-build-*` directories, we work mostly in cmake-build-debug !
./wasp -> cmake-build-debug/wasp

### CMake Configuration Options

Key CMake variables (set via `-D` flags):
- `CMAKE_BUILD_TYPE=Debug|Release` - Build configuration
- `RELEASE=1` - Release build (implies `NO_TESTS=1`)
- `NO_TESTS=1` - Disable tests
- `RUNTIME_ONLY=1` - Build runtime only (no compiler/emitter)
- `DEBUG=1` - Enable debug symbols and DWARF output
- `TRACE=1` - Enable verbose tracing
- `STRICT=1` - All warnings as errors (enabled by default)
- `SANITIZE=1` - Enable sanitizers (address, undefined)
- `SDL=1` - Include graphics support
- `WASM=1` - Build as WebAssembly (set via toolchain)

### Testing

Never mock, always test on real data with real functions and functionality.

The project includes extensive tests in `tests/tests.cpp`. Tests are enabled by default but can be disabled with `-DNO_TESTS=1`.

```bash
# Run tests (after each change and building)
./wasp test
```

Test output generates extensive debug information. The only important thing is that the final 2 lines shows:
`251`
`CURRENT TESTS PASSED`
The number of TESTS PASSED must not go down!

## Architecture Overview

### Core Components

**Node System (`source/Node.h/.cpp`)**:
- Central AST/data structure representing all Wasp constructs
- Union-based value storage supporting multiple types (int64, double, string, node references)
- Smart memory management with custom allocators
- Supports both object-like and array-like access patterns

**String System (`source/String.h/.cpp`)**:
- High-performance string implementation optimized for parsing
- UTF-8/Unicode support with efficient character iteration
- Memory-efficient storage with copy-on-write semantics

**Parser (`source/Wasp.h/.cpp`)**:
- Recursive descent parser for Wasp syntax
- Operator precedence handling
- Support for multiple syntactic forms (markup, expressions, blocks)

**Code Generation (`source/wasm_emitter.h/.cpp`)**:
- Direct WebAssembly bytecode generation
- Multiple optimization levels
- Debug information (DWARF) support

**Runtime Abstraction (`source/wasm_helpers.h/.cpp`)**:
- Abstraction layer for different WASM runtimes
- Host function bindings
- Memory management between host and WASM

### Key Design Patterns

**Unified Data Model**: Everything is a Node - from simple values to complex ASTs. This enables powerful metaprogramming and consistent data manipulation.

**Smart Pointers**: Custom 64-bit smart pointer system for type encoding in WASM environments until structs work as intercom.


## Development Guidelines

### Important Constraints

- **Never modify code not explicitly part of the task** - Keep changes minimal and focused
- **Never remove existing code** unless explicitly required by the task
- **Never remove existing comments** unless explicitly stated as task

### Coding Style

- Use `0` instead of `nullptr`
- use #pragma once
- use trace("msg") for debugging
- **Never use std:: libraries** - Use the custom `String`, `List`, `Map` provided in the project, or implement simple ad-hoc algorithms
- Use meaningful names for self-documenting code (minimize trivial comments)
- Never manually add (ffi) Signatures instead derive them dynamically by reflection

### Code Organization

- **source/**: All C++ source code
  - **source/Node.h/.cpp**: Central AST/data structure
  - **source/String.h/.cpp**: Custom string implementation
  - **source/Wasp.h/.cpp**: Parser
  - **source/Angle.h/.cpp**: Angle language implementation
  - **source/wasm_emitter.h/.cpp**: WebAssembly code generation
  - **source/wasm_reader.cpp**: WASM binary reader
  - **source/wasm_helpers.h/.cpp**: Runtime abstraction layer
  - **source/own_merge/**: WebAssembly binary linker utilities (forked from WABT)
- **tests/**: All test files and test headers
  - **tests/tests.cpp**: Main test file (includes test_angle.cpp, test_wasm.cpp)
  - **tests/test_angle.cpp**: Angle language tests
  - **tests/test_wasm.cpp**: WebAssembly tests
  - **tests/test_browser.cpp**: Browser/WASM tests
  - **tests/test_*.h**: Modular test headers organized by functionality (13 modules)
- **samples/**: Example Wasp/Angle programs
- **bin/**: Compiled binaries for different platforms
- **Frameworks/**: External dependencies and runtime libraries
- **cmake-build-*/**: CMake build directories for different configurations

### Memory Management

The codebase uses custom memory management:
- Aligned allocation for WASM compatibility
- Node allocation via custom `new`/`delete` operators
- Smart pointer system for cross-boundary (host/WASM) references

### WebAssembly Integration

The project has deep WebAssembly integration:
- Can compile itself to WASM
- Runtime can execute WASM modules via multiple engines (WasmEdge, Wasmtime, WASM3, V8, etc.)
- Supports WASM component model and WIT interfaces (first-class citizen)
- Native WASM bytecode generation with optimization levels
- Can produce extremely small WASM files (<1kb for simple programs without stdlib)

### WASM Runtime Options

The native runtime can be configured with different WASM engines (via CMake):
- **WASMEDGE=1**: Fastest, easiest, default choice
- **WASMTIME=1**: Fast but can have stability issues
- **WASM3=1**: Good API with backtrace support (project unmaintained)
- **V8=1**: Full browser engine
- **WEBAPP=1**: WebView for Electron-like standalone apps

## Testing and Debugging

### Test Structure

Tests are located in `source/tests.cpp` (which includes `test_angle.cpp` and `test_wasm.cpp`).
- Tests use custom assertion macros from `asserts.h`
- Test functions like `assert_emit()`, `assert_equals()`, `check_is()`
- Tests cover parsing, type system, code generation, and WASM execution

### Debugging Tools

**Debug Flags:**
- `TRACE=1` - Verbose debugging output
- `DEBUG=1` - Enable debug symbols and DWARF info
- `SANITIZE=1` - Address and undefined behavior sanitizers (Mac/Linux)
- `VERBOSE=1` - Verbose linker output

**Debug Commands:**
```bash
# Build with debug info
cmake -B build -DDEBUG=1 . && make -C build

# Build with sanitizers
cmake -B build -DSANITIZE=1 . && make -C build

# Use lldb for debugging
lldb -- ./build/wasp test
```

## **WASM Debugging:**
- DWARF debug info available in WASM builds (`-g3 -gdwarf-4`)
- Source maps generated for browser debugging
- Use `wasmtime -D debug-info=y` to enable debug info at runtime

In case some inexplicable error occurs, debug wasm files with `wasm2wat --no-check --enable-all --ignore-custom-section-errors`  and `wasm-objdump -x -d -s`
debug wasm files with hexdump if they are illformed. compare with a minimal working wasm.

### Common Issues
- in @source/wasmtime_runner.cpp we used a patched wasmtime wasm-c-api to read struct properties
- After each modification run ./wasp test And commit if it works
- NEVER create or modify tests in ./tests/ folder unless Explicitly instructed. Instead try to work with The existing tests or if absolutely necessary create new temporary tests in ./probes/ folder
- use typeName() To get the name of a Kind / Type / ValueType
- don't create random ./docs/ or .md files unless explicitly instructed
- stay dependency free! never add extra dependencies (without asking) because we compile to wasm 