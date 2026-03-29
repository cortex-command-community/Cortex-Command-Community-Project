# Cortex Command - Feature Ownership Map

Use this file to route tasks fast when you are assigning work.
Each area lists the primary code owners (systems/files), common change types, and quick validation paths.

## 1) Engine boot, loop, and app lifecycle

| Area | Primary files | Typical changes | Validate |
|---|---|---|---|
| Boot/shutdown and loops | `Source/Main.cpp` | startup flags, manager order, loop transitions, global flow bugs | launch game, enter menu, start mission, exit cleanly |
| Timers/perf cadence | `Source/Managers/TimerMan.*`, `Source/Managers/PerformanceMan.*` | fixed-step timing, sim pacing, perf counters | observe FPS/MSPF/MSPSU behavior in debug tools |
| Global thread orchestration | `Source/Managers/ThreadMan.*` | task queue behavior, pool sizing, shutdown waits | no deadlocks on quit, no stalled async tasks |

## 2) Window, rendering, and post-process

| Area | Primary files | Typical changes | Validate |
|---|---|---|---|
| Window/display state | `Source/Managers/WindowMan.*` | fullscreen, resolution changes, vsync, display handling | resize/fullscreen toggle from settings |
| Frame composition | `Source/Managers/FrameMan.*` | draw order, split-screen layout, overlays, framebuffer behavior | menu + in-game draw correctness, split-screen |
| GL resources/pipeline | `Source/Managers/GLResourceMan.*`, `Source/Renderer/*` | texture/shader lifetime, render target behavior, draw helpers | load scene, camera movement, effects visible |
| Post effects | `Source/Managers/PostProcessMan.*`, `Data/Base.rte/Shaders/*` | post-processing settings/effects | toggle visual settings and compare output |

## 3) Input and controls

| Area | Primary files | Typical changes | Validate |
|---|---|---|---|
| Input event ingestion | `Source/Main.cpp` (`PollSDLEvents`), `Source/Managers/UInputMan.*` | keyboard/mouse/gamepad event handling, focus rules | navigate menus + play with each input type |
| Input mapping/config | `Source/System/InputMapping.*`, `Source/System/InputScheme.*`, `Source/Menus/SettingsInput*.cpp` | remapping UX, deadzones, keybind persistence | remap controls, restart, verify persisted mapping |
| Device icons and UI hints | `Source/Managers/UInputMan.*`, `Data/Base.rte/GUIs*` | icon sets and active device display | switch device, check icon updates in menus |

## 4) Audio and music

| Area | Primary files | Typical changes | Validate |
|---|---|---|---|
| Audio backend | `Source/Managers/AudioMan.*` | channel behavior, panning, priorities, global pitch | play mission with many effects and monitor audio stability |
| Music behavior | `Source/Managers/MusicMan.*`, `Source/Entities/DynamicSong.*` | transitions, context music, silence/fade logic | start/stop activities, pause/resume, check music state |
| Sound assets integration | `Source/Entities/SoundContainer.*`, `Source/Entities/SoundSet.*`, `Data/*/Sounds*` | new sound definitions, runtime playback wiring | trigger object actions that play affected sounds |

## 5) Gameplay state and activities

| Area | Primary files | Typical changes | Validate |
|---|---|---|---|
| Activity lifecycle | `Source/Managers/ActivityMan.*`, `Source/Entities/Activity.*` | start/pause/resume/restart flow, save/load behavior | menu -> mission -> pause -> resume -> restart |
| Scripted activities | `Source/Activities/GAScripted.*`, `Data/*/Activities*`, activity Lua | mission rules, scripted events | run target scenario and follow repro script |
| Editors (in-game tools) | `Source/Activities/*Editor*`, `Source/Menus/*EditorGUI*` | scene/actor/area editing workflows | launch via `-editor <EditorName>` and test actions |

## 6) World simulation and entities

| Area | Primary files | Typical changes | Validate |
|---|---|---|---|
| Scene/terrain lifecycle | `Source/Managers/SceneMan.*`, `Source/Entities/Scene.*`, `Source/Entities/SLTerrain.*` | terrain loading, placement, background/layer behavior | load several scenes, check placement and terrain state |
| Movable object runtime | `Source/Managers/MovableMan.*`, `Source/Entities/MovableObject.*` | update loops, collision side effects, object lifetime | spawn combat-heavy scene, monitor crashes/desyncs |
| Actor/device behavior | `Source/Entities/AHuman.*`, `ACrab.*`, `HDFirearm.*`, `ThrownDevice.*`, `TDExplosive.*` | unit logic, weapon handling, damage/explosion behavior | sandbox scenario with targeted actor/device interactions |
| Materials/physics helpers | `Source/Entities/Material.*`, `Source/System/SpatialPartitionGrid.*`, `Source/System/PathFinder.*` | terrain/material interactions, broad-phase or pathing | stress test AI movement and terrain destruction |

## 7) Data modules, presets, and mod loading

| Area | Primary files | Typical changes | Validate |
|---|---|---|---|
| Module discovery/order | `Source/Managers/PresetMan.*` | load order, official/mod/userdata handling, single-module mode | run with default + `-module <Mod.rte>` |
| Module parsing | `Source/System/DataModule.*`, `Source/System/Reader.*` | `Index.ini` processing, include traversal, metadata parsing | introduce controlled bad/good ini and check logs |
| Preset lookup and reflection | `Source/System/Entity.*`, `Source/Managers/PresetMan.*` | type registration, preset fetch errors, clone/memory pool logic | boot and load activity that touches changed presets |
| Content definitions | `Data/*.rte/*.ini`, `Data/*.rte/*.lua`, `Mods/*.rte/*` | balancing, loadouts, object defs, scenario content | run affected faction/activity and verify assets |

## 8) Lua runtime and bindings

| Area | Primary files | Typical changes | Validate |
|---|---|---|---|
| Lua state/runtime | `Source/Managers/LuaMan.*` | threaded states, script dispatch, error propagation | trigger scripted entities, watch console/log warnings |
| C++ bindings | `Source/Lua/LuaBindings*.cpp`, `Source/Lua/LuaAdapters.cpp` | expose methods/types/properties to scripts | write small test script that calls new API |
| Script assets | `Data/*/Scripts/*`, `Data/Base.rte/LuaIntegration/*` | gameplay script behavior or utility updates | reproduce script path and verify expected side effects |

## 9) Menus and UI

| Area | Primary files | Typical changes | Validate |
|---|---|---|---|
| Menu state machine | `Source/Managers/MenuMan.*`, `Source/Menus/TitleScreen.*`, `MainMenuGUI.*`, `PauseMenuGUI.*` | transitions, menu behavior, flow control | boot sequence, scenario selection, pause/unpause |
| Settings pages | `Source/Menus/Settings*.cpp`, `Source/Managers/SettingsMan.*` | new options, value clamping, persistence | change option, restart game, verify persisted value |
| GUI framework | `Source/GUI/*`, `Source/GUI/Wrappers/*` | controls, skinning, event routing | navigate complex menus, check interactions and layout |

## 10) Networking (currently mostly disabled in Meson)

| Area | Primary files | Typical changes | Validate |
|---|---|---|---|
| Client/server managers | `Source/Managers/NetworkClient.*`, `Source/Managers/NetworkServer.*` | protocol behavior and connection lifecycle | requires enabling in build and multiplayer test harness |
| Multiplayer activity/ui | `Source/Activities/MultiplayerGame.*`, `Source/Menus/MultiplayerGameGUI.*` | multiplayer flow and UI behavior | host/join smoke test after build integration |

Note: network-related sources are present but commented out in current Meson lists. Account for that before assigning networking tasks.

## 11) Assignment shortcuts (manager view)

- If bug mentions startup/crash before menu: assign to boot/lifecycle (`Source/Main.cpp`, manager init).
- If bug mentions missing preset/mod/content: assign to `PresetMan` + `DataModule` + relevant `Data/*.rte`.
- If bug mentions control not responding: assign to `UInputMan` + settings input GUIs.
- If bug is visual only (no gameplay impact): assign to `WindowMan`/`FrameMan`/`Renderer`.
- If bug is scripted behavior mismatch: assign to Lua bindings/runtime + script assets.

## 12) Standard acceptance checks for any subsystem ticket

1. Repro case before change and after change.
2. No regressions on startup, entering mission, pausing, and quitting.
3. Logs checked for new warnings/errors (`LogConsole.txt`, `LogLoading.txt`).
4. If data/module touched, verify at least one official activity and one mod loading path.
