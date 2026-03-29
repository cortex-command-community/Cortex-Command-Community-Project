---
name: cortex-command-community-project
description: Use when working in the Cortex Command Community Project repository and you need fast subsystem routing, safe code change workflow, and reproducible build/debug validation on Debian, Ubuntu, or WSL2.
---

# Cortex Command Community Project

## Overview

Use this skill as the project operating manual for coding tasks in `Cortex-Command-Community-Project`.

- Engine stack: C++20, Meson + Ninja, entry executable `CortexCommand`.
- Runtime model: singleton managers (`g_*Man`) + `Entity` presets + Lua scripting.
- First read for most tasks: `Source/Main.cpp`, `Source/Managers/PresetMan.*`, `Source/System/DataModule.*`.

## Quick Start (Debian/Ubuntu/WSL2)

Run from repository root.

```bash
sudo apt-get install build-essential libflac++-dev luajit-5.1-dev liblua5.1-dev libminizip-dev liblz4-dev libpng++-dev libtbb-dev ninja-build python3-pip
sudo python3 -m pip install meson

CC="ccache gcc" CXX="ccache g++" meson setup build-dev --reconfigure --buildtype=debug
ninja -C build-dev

./build-dev/CortexCommand
./build-dev/CortexCommand -cout
./build-dev/CortexCommand -ext-validate
./build-dev/CortexCommand -module <SomeMod.rte>
./build-dev/CortexCommand -editor <EditorName>
```

WSL notes:
- WSL2 Ubuntu 22.04 is known-good.
- Building from `/mnt/c/...` works, but Linux filesystem paths usually perform better.
- Same Linux commands apply.

## Architecture Map

- `Source/Main.cpp`: process boot, manager init order, flags, loop dispatch, shutdown.
- `Source/Managers/*`: global systems (window, frame, activity, scene, movable, audio, lua, presets, etc.).
- `Source/Entities/*`: gameplay objects and simulation entities.
- `Source/System/*`: serialization, IO, data modules, base utilities.
- `Source/Lua/*`: Lua bindings/adapters.
- `Data/*.rte/*`: official content modules.
- `Mods/*.rte/*`: unofficial content modules.
- `Userdata/*`: runtime/user-generated modules and saves.

Boot sequence in `Source/Main.cpp`:
1. Initialize SDL/Allegro and base system.
2. Initialize managers in strict order.
3. Parse launch flags.
4. Load data modules (`g_PresetMan.LoadAllDataModules()`).
5. Start menu/activity loops.
6. Shutdown thread pools and managers cleanly.

Game loop model (`RunGameLoop()`):
- Event polling/input queueing.
- Fixed-step simulation updates (Lua/input/frame/activity/scene/movables/audio).
- Draw (`g_FrameMan.Draw()`) then post-process/upload (`g_WindowMan`).

## Module and Content Loading Model

- Official modules load first (`PresetMan::c_OfficialModules`).
- Mods from `Mods/` load after official modules if enabled.
- Userdata modules load last and may be auto-created.
- INI definitions become `Entity` presets resolved through `PresetMan` + `DataModule`.

When content fails to load:
- Verify module order in `Source/Managers/PresetMan.cpp`.
- Verify each module `Index.ini` include chain.
- Verify class names and preset names match C++ registration/bindings.

## Subsystem Routing Guide

Use symptom-first routing:

- Startup crash/black screen/quit deadlock -> `Source/Main.cpp`, `Source/Managers/WindowMan.*`, `Source/Managers/FrameMan.*`, `Source/Managers/ThreadMan.*`.
- Missing mod/preset/content -> `Source/Managers/PresetMan.*`, `Source/System/DataModule.*`, affected `Data/*.rte` or `Mods/*.rte` files.
- Input not responding/remap issues -> `Source/Main.cpp` (`PollSDLEvents`), `Source/Managers/UInputMan.*`, `Source/System/InputMapping.*`, `Source/System/InputScheme.*`, `Source/Menus/SettingsInput*.cpp`.
- Visual artifacts/render-only issues -> `Source/Managers/FrameMan.*`, `Source/Managers/WindowMan.*`, `Source/Managers/PostProcessMan.*`, `Source/Renderer/*`.
- Script callback/binding mismatch -> `Source/Managers/LuaMan.*`, `Source/Lua/LuaBindings*.cpp`, `Data/*/Scripts/*`.
- Gameplay lifecycle/pause/restart bugs -> `Source/Managers/ActivityMan.*`, `Source/Activities/*`, `Source/Entities/Activity.*`.
- Scene/object lifetime/collision bugs -> `Source/Managers/SceneMan.*`, `Source/Managers/MovableMan.*`, `Source/Entities/MovableObject.*`.
- Audio/music behavior issues -> `Source/Managers/AudioMan.*`, `Source/Managers/MusicMan.*`, `Source/Entities/SoundContainer.*`, `Source/Entities/SoundSet.*`.

## Key Manager Ownership Map

- `WindowMan`/`FrameMan`/`PostProcessMan`: display state, frame composition, post effects.
- `UInputMan`: runtime input ingestion and active-device behavior.
- `AudioMan`/`MusicMan`: effect channels, panning, music transitions.
- `ActivityMan`: activity lifecycle and state transitions.
- `SceneMan`: terrain/scene lifecycle.
- `MovableMan`: active movable object update/lifetime.
- `PresetMan`: module discovery/order and preset registry lookup.
- `LuaMan`: Lua states, dispatch, error propagation.
- `MenuMan` + settings/menu GUIs: front-end flow and settings persistence.

Treat ownership as routing guidance, not strict gatekeeping.

## Common File Touchpoints By Issue Type

- Boot or manager-order regressions: `Source/Main.cpp`, `Source/Managers/*`.
- New configurable manager behavior: target `Source/Managers/<X>Man.*` + `Source/Managers/SettingsMan.*` + relevant `Source/Menus/Settings*.cpp`.
- Entity behavior changes: `Source/Entities/*` + lifecycle managers (`MovableMan`, `SceneMan`, `ActivityMan`) + bindings if script-facing.
- New/changed Lua API surface: `Source/Lua/LuaBindings*.cpp`, `Source/Lua/LuaAdapters.cpp`, `Source/Managers/LuaMan.*`.
- Preset parsing/load problems: `Source/System/DataModule.*`, `Source/System/Reader.*`, `Source/Managers/PresetMan.*`, module `Index.ini` files.

## Safe Implementation Workflow

1. Restate player-visible outcome and choose a single primary subsystem.
2. Reproduce first with exact steps and logs (`LogConsole.txt`, `LogLoading.txt`).
3. Trace root cause before coding (manager order, load order, lifecycle boundaries, bindings).
4. Apply minimal diff in scoped files; avoid cross-subsystem refactors unless required.
5. If touching script-facing C++ behavior, update bindings and script/content callsites in same change.
6. Rebuild and run targeted validation path plus core regressions.

Scoping rules:
- Prefer one player-visible behavior per ticket.
- Avoid mixing rendering + gameplay + content in one PR unless tightly coupled.
- Any `Main.cpp` or manager core change requires explicit regression checklist.

## Debugging Checklist

- Confirm whether bug is logic-step or render-step (fixed update vs draw path).
- For startup failures, inspect manager init/shutdown ordering in `Source/Main.cpp`.
- For content failures, inspect module order + include chain + preset/class name mismatches.
- For scripting failures, verify Lua binding exists and script path actually executes.
- For performance/jank spikes, inspect timer/performance managers and expensive loops in active subsystem.
- Always compare before/after logs (`LogConsole.txt`, `LogLoading.txt`) for new warnings.

## Validation Matrix

Baseline for every PR:
- Build succeeds: `ninja -C build-dev`.
- Game reaches menu: `./build-dev/CortexCommand`.
- At least one in-game scenario on affected path.
- No new relevant log warnings/errors.

Area-specific checks:
- Input: keyboard + mouse + one gamepad path.
- Modules/content: one official module path + one mod path.
- Activity/gameplay: start -> pause -> resume -> restart -> exit.
- Window/render: windowed + fullscreen + resolution change.
- Lua/bindings: execute script path that calls changed API.

## PR Readiness Checks

- Ticket includes player-visible outcome, scope, and acceptance criteria.
- Diff is scoped to declared subsystem(s) and avoids unrelated cleanup.
- Repro steps are included for before and after behavior.
- Validation evidence is present (commands run + scenario/menu path used).
- Compatibility notes included when relevant (save/mod/performance risk).

Priority shorthand:
- `P0`: crash, startup failure, hard lock, data loss.
- `P1`: major blocker/regression in common gameplay path.
- `P2`: functional bug with workaround.
- `P3`: polish/refactor/non-blocking improvement.

## Work Intake Template

Use this when creating implementation tickets:

```md
Title:

Player-visible outcome:
-

Current behavior:
-

Scope:
- In scope:
- Out of scope:

Likely subsystem(s):
-

Repro / validation path:
1.
2.
3.

Acceptance criteria:
- [ ]
- [ ]

Risk notes:
- Save compatibility?
- Mod compatibility?
- Performance risk?
```
