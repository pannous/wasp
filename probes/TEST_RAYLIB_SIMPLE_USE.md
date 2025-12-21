# Test for samples/raylib_simple_use.wasp

## Purpose
Tests the functionality demonstrated in `samples/raylib_simple_use.wasp`:
- `import raylib` (whole module import) instead of individual function imports
- Raylib FFI integration
- Window creation, drawing operations, and cleanup

## Test File
`probes/test_raylib_simple_use.wasp` - Modified version without infinite loop

## Key Differences from Original Sample
- **Original**: Uses `while(1>0)` infinite loop with `sleep(500)`
- **Test**: Draws exactly 3 frames then exits with code 42
- **Reason**: Enables automated testing without requiring manual window closure

## What is Tested
✅ Module-level import: `import raylib`
✅ Window initialization: `InitWindow(800, 600, "...")`
✅ FPS control: `SetTargetFPS(60)`
✅ Drawing operations:
  - `BeginDrawing()` / `EndDrawing()`
  - `ClearBackground(0xFFF5F5F5)` - RAYWHITE color
  - `DrawText(...)` - Text rendering with DARKGRAY color
  - `DrawCircle(400, 300, 80.0, 0xFF2E1990)` - MAROON circle
✅ Resource cleanup: `CloseWindow()`
✅ Return value: 42 (success indicator)

## Running the Test

```bash
./cmake-build-debug/wasp probes/test_raylib_simple_use.wasp
```

Expected output:
- Raylib initialization messages
- Brief window display with rendered content
- Exit code 42

## Test Results

### ✅ PASSED (2025-12-21)
```
Exit code: 42
Raylib successfully initialized
Window created: 800x600
3 frames rendered with circle and text
Clean shutdown
```

## Notes
- Test requires raylib to be installed at `/opt/homebrew/lib/libraylib.dylib` (macOS)
- Window appears briefly during test execution
- All raylib functions are automatically available via `import raylib`
- Color values use RGBA packed as i32 in little-endian format (0xAABBGGRR)
