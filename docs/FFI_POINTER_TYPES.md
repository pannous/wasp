# FFI Pointer Type Handling

## The Problem

When calling native C libraries via FFI, we encounter complex pointer types like:
- `SDL_Window*` - Opaque window handle
- `SDL_Renderer*` - Opaque renderer handle
- `struct foo*` - Pointer to struct
- `void*` - Generic pointer

How should Wasp represent these in WASM?

## Solution Options

### 1. Opaque Pointers as i64 (Current Approach) ✅

**Implementation:**
```cpp
// All pointers map to i64
if (c_type.contains("*")) return i64;
```

**Pros:**
- Simple and efficient
- No marshalling overhead
- Works with all WASM runtimes
- Perfect for opaque handles (SDL, OpenGL, etc.)
- Pointer arithmetic works if needed

**Cons:**
- No type safety (can pass wrong pointer type)
- Can't inspect struct fields
- Assumes 64-bit pointers (fine for modern systems)

**Use Cases:**
- FFI with libraries using opaque pointers (SDL, GLFW, curl)
- When we never access the pointed-to data
- When library provides all access functions

### 2. WASM externref

**Implementation:**
```wasm
(param (ref extern)) (result (ref extern))
```

**Pros:**
- Type-safe references
- Garbage collected
- Can't accidentally do pointer arithmetic
- Part of WASM GC proposal

**Cons:**
- Requires WASM GC support
- Can't convert to/from integers
- More complex marshalling
- Not all runtimes support it yet

**Use Cases:**
- When type safety is critical
- Objects that need GC
- Modern WASM-first libraries

### 3. WASM GC Structs

**Implementation:**
```wasm
(type $SDL_Rect (struct (field $x i32) (field $y i32) ...))
```

**Pros:**
- Type-safe with field access
- Garbage collected
- Can inspect/modify fields

**Cons:**
- Need to know struct layout
- Complex marshalling (copy fields)
- Struct layout may differ (padding, alignment)
- C struct may change between versions

**Use Cases:**
- When we need to access struct fields
- Simple, stable struct types
- WASM-first applications

### 4. Linear Memory Structs

**Implementation:**
```javascript
// Allocate in WASM linear memory
let ptr = malloc(sizeof_SDL_Rect);
store_i32(ptr + 0, x);  // Write x field
store_i32(ptr + 4, y);  // Write y field
```

**Pros:**
- Full C compatibility
- Pointer arithmetic works
- Can access all fields
- Same layout as C

**Cons:**
- Manual memory management
- Need to know exact struct layout
- Alignment issues
- Memory safety concerns

**Use Cases:**
- When we need full C struct compatibility
- Direct memory manipulation
- Performance-critical code

## Recommendation for FFI

**Use opaque i64 pointers for:**
- Library handles (SDL_Window*, FILE*, etc.)
- Anything you don't need to inspect
- Third-party C libraries

**Use WASM GC structs for:**
- Simple data structures you control
- When you need field access
- New WASM-first code

**Use linear memory for:**
- Direct C interop (shared buffers)
- Performance-critical paths
- When struct layout is guaranteed stable

## Current Implementation

```cpp
// ffi_header_parser.h
inline Type mapCTypeToWasp(String c_type) {
    // Strings are special - need null termination
    if (c_type.contains("char*")) return charp;

    // All other pointers are opaque handles
    if (c_type.contains("*")) return i64;

    // Value types
    if (c_type == "int") return int32t;
    if (c_type == "double") return float64t;
    // ...
}
```

## Example: SDL_CreateWindow

```c
// C Declaration:
SDL_Window* SDL_CreateWindow(const char* title, int x, int y,
                              int w, int h, Uint32 flags);

// Detected Signature:
//   params: [charp, i32, i32, i32, i32, i32]
//   return: i64

// Wasp Usage:
import SDL_CreateWindow from 'SDL2'
window = SDL_CreateWindow("My Window", 0, 0, 800, 600, 0)
// window is i64 (opaque handle)
```

The window pointer is never dereferenced in Wasp - it's just passed back to other SDL functions.

## Future Enhancements

1. **Type Aliases** - Allow naming pointer types:
   ```wasp
   type Window = ptr
   type Renderer = ptr
   ```

2. **Smart Pointers** - Track ownership:
   ```wasp
   window: owned<SDL_Window*>  // Auto-call SDL_DestroyWindow
   ```

3. **Struct Reflection** - Parse C headers for struct layouts:
   ```c
   // Auto-generate from: typedef struct { int x, y, w, h; } SDL_Rect;
   SDL_Rect{ x: 10, y: 20, w: 100, h: 50 }
   ```

## Conclusion

For FFI with native libraries, **opaque i64 pointers** are the right choice:
- Simple, fast, compatible
- Perfect for libraries like SDL, OpenGL, curl
- No need to understand library internals
- Just pass handles between functions

Save complex types (externref, GC structs) for WASM-first code where you control the types.
