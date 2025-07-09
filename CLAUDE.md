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

### Core Build Commands

```bash
# Build the project (uses cmake)
# Build with different configurations (via CMake profiles)
cmake -DCMAKE_BUILD_TYPE=Debug
cmake -DCMAKE_BUILD_TYPE=Release
cmake -DRELEASE=1 -DNO_TESTS=1
```

### Development Scripts

### Testing

The project includes extensive tests in `source/tests.cpp`. Tests are enabled by default but can be disabled with `-DNO_TESTS=1`.

```bash
# Run tests (after building)
./wasp test
```
Generate a big lot of debug information the only important thing is that the final line is
`CURRENT TESTS PASSED`

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

**Important** Never modify any code not explicitly part of the task!
**Important** Never remove existing code unless explicitly required by the task!
**Important** Never remove existing comments unless explicitly stated as task!

## Coding Style
use `0` for `nullptr` 
**Important** never use any std:: libraries, use the custom String, List, Map provided in the project 
    or implement simple adhoc algorithms.


### Code Organization

- **source/**: All C++ source code
- **source/own_merge/**: WebAssembly binary linker utilities (forked from WABT, ignore for now)
- **test/**: Test files and sample programs, todo: move test.cpp … from source here
- **samples/**: Example Wasp/Angle programs
- **Frameworks/**: External dependencies and runtime libraries (ignore!)
- **CMakeLists.txt**: CMake build configuration files

### Memory Management

The codebase uses custom memory management:
- Aligned allocation for WASM compatibility
- Node allocation via custom `new`/`delete` operators
- Smart pointer system for cross-boundary (host/WASM) references

### WebAssembly Integration

The project has deep WebAssembly integration:
- Can compile itself to WASM
- Runtime can execute WASM modules
- Partly supports WASM component model and WIT interfaces

## Testing and Debugging


### Test Structure

Tests are located in `source/tests.cpp` and `test/` directory. The test system supports:

### Debugging

- Use `TRACE=1` for verbose debugging output
- DWARF debug info available in WASM builds
- Sanitizer support with `SANITIZE=1` until valguard is available on mac again.
- Source maps generated for WASM debugging

### Common Issues

- Memory alignment issues in WASM - ensure proper alignment in Node allocation
- Cross-compilation differences between native and WASM builds