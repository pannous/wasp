# WASP Docker Build Environment

This directory contains Docker configuration for building the WASP WebAssembly Programming Language in a consistent, reproducible environment.

## Quick Start

```bash
# Build the hosted WASM version (default)
./build.sh

# Build in debug mode with development tools
./build.sh --debug --dev

# Run tests
./build.sh test

# Build runtime-only version
./build.sh wasp-runtime

# Clean all build artifacts
./build.sh clean
```

## Docker Images

The build system provides multiple Docker image targets:

### Production (`wasp-build`)
- Optimized for release builds
- Minimal dependencies
- Fast compilation
- Used by default

### Development (`wasp-dev`)  
- Debug builds
- Additional development tools (gdb, valgrind, wabt)
- Interactive shell access
- Code debugging capabilities

### Runtime (`wasp-runtime`)
- Builds runtime-only version
- Smaller output binaries
- No compiler/emitter code

## Build Targets

| Target | Description | Output |
|--------|-------------|---------|
| `wasp-hosted` | WASM version with host functions | `wasp-hosted.wasm` |
| `wasp-runtime` | Runtime-only WASM version | `wasp-runtime.wasm` |
| `wasp` | Native executable | `wasp` |
| `test` | Run test suite | Test results |
| `clean` | Clean build artifacts | - |

## Environment Variables

- `CMAKE_BUILD_TYPE`: `Release` or `Debug`
- `WASM_TARGET`: Target to build (`wasp-hosted`, `wasp-runtime`, etc.)
- `RUNTIME_ONLY`: Set to `1` for runtime-only builds

## Dependencies

The Docker images include:

- **WASI SDK**: WebAssembly System Interface SDK for WASM compilation
- **WasmEdge**: WebAssembly runtime for execution and testing
- **CMake**: Build system
- **Clang/LLVM**: Compiler toolchain
- **WABT**: WebAssembly Binary Toolkit (development image only)

## Docker Compose Profiles

```bash
# Production builds
docker compose --profile production up wasp-build

# Development environment
docker compose --profile development up wasp-dev

# Runtime-only builds  
docker compose --profile runtime up wasp-runtime

# Test execution
docker compose --profile test up wasp-test
```

## Volume Mounts

Build caches are persisted using Docker volumes:
- `wasp-build-cache`: Production build cache
- `wasp-dev-cache`: Development build cache  
- `wasp-runtime-cache`: Runtime build cache
- `wasp-test-cache`: Test build cache

## Interactive Development

```bash
# Start development container with shell
docker compose --profile development run --rm wasp-dev bash

# Inside container:
cd build
cmake -DCMAKE_BUILD_TYPE=Debug -DWASM=1 -DMY_WASM=1 ..
make wasp-hosted
```

## Native Building

To build without Docker (requires local dependencies):

```bash
./build.sh --native
```

## Troubleshooting

### Clean Everything
```bash
./build.sh clean
docker system prune -f
```

### Check Docker Status
```bash
docker compose ps
docker compose logs wasp-build
```

### Manual Container Access
```bash
docker compose --profile development run --rm wasp-dev bash
```

## Build Artifacts

Output files are created in the `build/` directory:
- `wasp-hosted.wasm` - Hosted WebAssembly build
- `wasp-runtime.wasm` - Runtime-only WebAssembly build
- `wasp` - Native executable
- `*.wasm.map` - Source maps for debugging

## Continuous Integration

The Docker environment is designed for CI/CD pipelines:

```yaml
# Example GitHub Actions usage
- name: Build WASP
  run: ./build.sh wasp-hosted

- name: Run Tests  
  run: ./build.sh test
```