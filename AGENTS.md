# Cortex Command Community Project - Agent Map

This file is a practical orientation guide for AI/code agents and non-game-dev contributors.
It explains where things live, how the runtime flows, and where to make common changes safely.

Companion docs:

- `AGENTS_FEATURE_OWNERSHIP.md` - who owns what subsystem, key files, and test paths.
- `AGENTS_WORK_INTAKE.md` - manager-friendly ticket template and assignment workflow.
- `.agents/skills/cortex-command-community-project/SKILL.md` - reusable project skill payload.

## 1) Quick context

- Engine language: C++20
- Main build system: Meson + Ninja (`meson.build` + `Source/**/meson.build`)
- Entry executable: `CortexCommand`
- Main entry point: `Source/Main.cpp`
- Core architecture style: singleton managers (`g_*Man`) + `Entity` preset system + Lua scripting

## 2) High-level repository layout

- `Source/` - engine and game code
  - `Activities/` - gameplay/editor activity logic (mission/session state)
  - `Entities/` - most game object classes (actors, weapons, particles, terrain objects, scenes)
  - `Managers/` - global systems (`WindowMan`, `FrameMan`, `AudioMan`, `SceneMan`, `MovableMan`, etc.)
  - `Menus/` - title/menu/settings/pause UI screens
  - `GUI/` - in-house GUI toolkit + wrappers + imgui integration
  - `Lua/` - C++ <-> Lua bindings and adapters
  - `Renderer/` - GL targets/shaders/draw pipeline helpers
  - `System/` - base infrastructure (serialization, file IO, runtime type info, core math/util)
- `Data/*.rte/` - official game content modules (INI/Lua/assets)
- `Mods/` - third-party module folder (`*.rte` loaded from here)
- `Userdata/` - runtime/user-generated modules and saves
- `external/` - vendored dependencies, headers, third-party sources
- `.github/workflows/` - CI build pipelines (Linux/macOS/Windows)

## 3) Build wiring

- Root `meson.build` configures compiler flags, dependencies, platform specifics, and links target executable.
- `Source/meson.build` includes all code areas and pulls each subdirectory's `meson.build`.
- Notable details:
  - SDL3 is built from subproject on non-macOS.
  - `LuaMan.cpp` is built as its own static library with permissive flags.
  - Some modules are present in code but disabled in Meson lists (for example network and achievements in current config).

## 4) Runtime boot sequence (important)

Main flow is in `Source/Main.cpp`:

1. Initialize Allegro/SDL and base `System`.
2. Construct and initialize managers in a strict order (`InitializeManagers()`).
3. Parse command-line flags (`-cout`, `-ext-validate`, `-module`, `-editor`).
4. Load data modules (`g_PresetMan.LoadAllDataModules()`).
5. Initialize menu/activity state and enter menu/game loops.
6. Shutdown: wait for thread pools, destroy managers, quit SDL/Allegro.

If something fails early, check manager init order and module loading first.

## 5) Main game loop mental model

`RunGameLoop()` has a fixed-step simulation pattern:

- Poll SDL events -> queue input/window events.
- Update window and frame timers.
- While simulation step is due:
  - update Lua, input, frame manager, console, activity, scene, movable objects, audio/music
  - process activity state transitions (pause/menu/restart/resume)
- Draw phase:
  - `g_FrameMan.Draw()`
  - post-process + upload via `g_WindowMan`

This split is important when fixing "logic vs render" bugs.

## 6) Most important systems and where to touch

- Application/window/display:
  - `Source/Managers/WindowMan.*`
  - `Source/Managers/FrameMan.*`
  - `Source/Renderer/*`
- Input:
  - `Source/Managers/UInputMan.*`
  - SDL events are consumed in `PollSDLEvents()` in `Source/Main.cpp`
- Audio/music:
  - `Source/Managers/AudioMan.*`
  - `Source/Managers/MusicMan.*`
  - asset wrappers in `Source/Entities/SoundContainer.*`, `SoundSet.*`
- Gameplay state:
  - `Source/Managers/ActivityMan.*`
  - concrete activities in `Source/Activities/*`
- World and entities:
  - `Source/Managers/SceneMan.*` (terrain/scene lifecycle)
  - `Source/Managers/MovableMan.*` (active movable objects)
  - `Source/Entities/*` (core object hierarchy)
- Data and mod content:
  - `Source/Managers/PresetMan.*`
  - `Source/System/DataModule.*`
  - content in `Data/*.rte/*` and `Mods/*.rte/*`
- Scripting:
  - `Source/Managers/LuaMan.*`
  - bindings in `Source/Lua/LuaBindings*.cpp`

## 7) Content/module loading model

- Official modules are loaded first from `PresetMan::c_OfficialModules`.
- Unofficial modules are loaded from `Mods/` if enabled.
- Userdata modules are loaded last (auto-created if missing).
- INI-driven definitions become `Entity` presets and are resolved through `PresetMan`/`DataModule`.

When debugging "missing asset/preset" issues, inspect:

- module order in `Source/Managers/PresetMan.cpp`
- module `Index.ini` and include chain
- class/type name and preset name mismatches

## 8) Core code patterns to expect

- Singleton access macros: `g_WindowMan`, `g_SceneMan`, etc.
- Reflection-like registration and pooled allocation via `Entity::ClassInfo` macros in `Source/System/Entity.h`.
- Serialization via `Serializable` + `Reader`/`Writer` (`Source/System/Serializable.h`, `Reader.*`, `Writer.*`).
- Heavy usage of ownership comments in headers; trust those comments when changing pointer logic.

## 9) Fast paths for common tasks

- Add or modify gameplay object behavior:
  1. Edit class in `Source/Entities/`.
  2. Update manager interactions if lifecycle changes (`MovableMan`, `SceneMan`, `ActivityMan`).
  3. If script-facing, update `Source/Lua/LuaBindings*.cpp`.
- Add a new manager-level feature:
  1. Implement in relevant `Source/Managers/*`.
  2. Wire init/update/shutdown in `Source/Main.cpp` if needed.
  3. Add settings plumbing in `SettingsMan` when configurable.
- Add or adjust content:
  1. Edit module `Data/<Module>.rte/Index.ini` and included INIs/Lua.
  2. Validate type/preset names against C++ bindings.
  3. Launch game and watch loading logs.

## 10) Useful local commands

- Incremental rebuild: `ninja -C build-dev`
- Run game: `./build-dev/CortexCommand`
- Reconfigure debug build:
  - `CC="ccache gcc" CXX="ccache g++" meson setup build-dev --reconfigure --buildtype=debug`
- Optional command-line diagnostics:
  - `./build-dev/CortexCommand -cout`
  - `./build-dev/CortexCommand -ext-validate`
  - `./build-dev/CortexCommand -module <SomeMod.rte>`
  - `./build-dev/CortexCommand -editor <EditorName>`

## 11) Good first files for any investigation

1. `Source/Main.cpp`
2. `Source/Managers/ActivityMan.*`
3. `Source/Managers/PresetMan.*`
4. `Source/System/DataModule.*`
5. `Source/Managers/SceneMan.*`
6. `Source/Managers/MovableMan.*`
7. `Source/Managers/LuaMan.*`

## 12) Notes for manager-style collaboration

- Ask for changes in terms of gameplay outcome ("what player should experience"), then map to systems.
- Prefer small, verifiable slices: one subsystem at a time.
- For each task, define:
  - expected player-visible behavior
  - affected module(s)
  - how to validate (menu path, scenario, log, repro steps)

That keeps implementation focused and reviewable even without deep engine internals knowledge.
