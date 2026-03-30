# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

Cortex Command Community Project (CCCP) — a C++ 2D side-scrolling game. Licensed under GNU AGPL v3. The `emscripten-port` branch adds a WebAssembly port targeting browser play, with a long-term goal of online multiplayer.

## Build Commands

### Native Linux/macOS Build
```bash
meson setup build                    # release (default)
meson setup --buildtype=debug build  # debug
ninja -C build
./CortexCommand                      # run (link libfmod first, see README)
```
macOS requires GCC: `env CC=gcc-13 CXX=g++-13 meson setup build`

### Emscripten/WASM Build
```bash
source ~/emsdk/emsdk_env.sh
bash emscripten/build.sh             # configure + build
bash emscripten/build.sh --debug     # debug build (assertions, safe heap)
bash emscripten/build.sh --serve     # build + serve at localhost:8080
# Or manually:
meson setup build-web --cross-file emscripten/cross_compilation.ini -Demscripten_port=true -Dtracy_enable=false
ninja -C build-web
bash emscripten/package.sh
python3 emscripten/serve.py build-web 8083
```

### Key Meson Options
- `debug_type`: `full` | `minimal` (default) | `release`
- `tracy_enable`: profiler (default true, must be false for WASM)
- `emscripten_port`: auto-set by cross-compilation file

There is no test suite. Verification is done through gameplay and CI builds (GitHub Actions: Meson on Linux/macOS, MSBuild on Windows).

## Architecture

### Singleton Manager Pattern

The engine is organized around ~24 singleton managers, all in `Source/Managers/`. Each is constructed and initialized in a strict order in `Source/Main.cpp:InitializeManagers()`. Accessed globally via `g_ManagerName` macros (e.g. `g_WindowMan`, `g_FrameMan`).

**Key managers:**
- **WindowMan**: SDL3 window, GL context, frame upload pipeline
- **FrameMan**: Backbuffers (8bpp + 32bpp), split-screen composition
- **PostProcessMan**: GL post-processing, glow effects
- **GLResourceMan**: OpenGL resource lifecycle
- **PresetMan**: Entity templates and DataModule loading from `.rte` zips
- **SceneMan**: Terrain, backgrounds, spatial partitioning
- **ActivityMan**: Game mode lifecycle (GameActivity, EditorActivity, etc.)
- **LuaMan**: Lua script execution with per-entity state
- **UInputMan**: Keyboard, mouse, gamepad input
- **AudioMan / MusicMan**: Sound via FMOD (stubbed on WASM)
- **MovableMan**: Physics entity management
- **ThreadMan**: Thread pools via BS::thread_pool

### Dual Rendering Pipeline

Two rendering paths coexist:
1. **Allegro 4 (CPU)**: Legacy 8bpp indexed-color + 32bpp backbuffers. The GUI system renders here. Abstracted via `Source/GUI/Wrappers/AllegroBitmap` and `AllegroScreen`.
2. **OpenGL 3.3 / ES3 (GPU)**: Modern rendering via GLAD + raylib's rlgl (`Source/Renderer/`). Shaders, post-processing, framebuffers.

`WindowMan::UploadFrame` composites both layers: uploads CPU backbuffer to GL texture, uses a staging texture copy (avoiding feedback loops), then composites via the ScreenBlit shader.

### Emscripten/WASM Layer

All web-specific code is behind `#ifdef __EMSCRIPTEN__` guards. The platform abstraction lives in `Source/System/WebPlatform.h/.cpp`:
- **Main loop**: `emscripten_set_main_loop` / requestAnimationFrame
- **Asset loading**: Modules fetched lazily as zips via `emscripten_fetch`, extracted into MEMFS (no monolithic .data blob for non-base modules)
- **Persistence**: IDBFS mount at `/Userdata` for saves/settings
- **Canvas2D overlay**: `WebPlatform_Blit32ToCanvas()` pushes GUI buffer to a Canvas2D element layered on WebGL
- **Asyncify**: Blocking C++ calls (file I/O, HTTP fetches) yield to the browser event loop

Key constraints: no threads by default, WebGL ES2/ES3 subset only, no direct filesystem, GPU skinning disabled (`RL_SUPPORT_MESH_GPU_SKINNING`), LuaJIT replaced with PUC-Lua 5.1.5 (no JIT in WASM).

Emscripten tooling lives in `emscripten/`: `build.sh`, `cross_compilation.ini`, `pre.js` (JS glue), `shell.html` (page template), `serve.py` (dev server with COOP/COEP headers), `package.sh`.

### Data-Driven Content System

Game content is organized into DataModules (`.rte` directories/zips) loaded by `PresetMan`. Entity definitions use INI-style files parsed by `Reader`/`Writer`. Lua scripting via luabind provides mod support — bindings in `Source/Lua/LuaBindings*.cpp` (7 modules covering Activities, Entities, Managers, GUI, Input, Primitives, System).

### GUI System

Legacy Allegro 4 GUI (`Source/GUI/`): `GUIControl` base class, `GUIManager` for panel stacking/focus, `GUISkin` for theming. Menu screens in `Source/Menus/`. ImGui (`Source/GUI/imgui/`) is used for debug/editor overlays, separate from the game GUI.

## Key Conventions

- C++20 standard. GCC >=13 required (clang lacks `std::execution` support).
- Namespace: `RTE` for all game code.
- Managers follow Construct → Initialize → (game loop) → Destroy lifecycle.
- Conditional compilation: `__EMSCRIPTEN__` for web, `_WIN32` for Windows, `DEBUG_BUILD` for debug features.
- External dependencies live in `external/sources/` (built as meson subprojects) and `external/include/` (header-only).
