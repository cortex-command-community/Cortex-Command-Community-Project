/**
 * EmscriptenMain.h
 *
 * Adapts the game's blocking main loops into Emscripten's per-frame callback
 * model without requiring pervasive #ifdef guards throughout Main.cpp.
 *
 * Design:
 *   The browser cannot block the main thread. Emscripten's
 *   emscripten_set_main_loop() registers a C function that gets called once
 *   per browser animation frame (requestAnimationFrame).
 *
 *   We use a simple state machine:
 *     LOADING   — data modules loading (one module per frame via yielding)
 *     MENU      — RunMenuLoop body executes one frame at a time
 *     GAME      — RunGameLoop body executes one frame at a time
 *     QUIT      — loop cancelled
 *
 *   On native builds this header is unused; Main.cpp uses its normal blocking
 *   while loops.
 *
 *   The global WebMainLoopIteration() function is referenced from
 *   WebPlatform.cpp via forward declaration.
 */

#pragma once

#ifdef __EMSCRIPTEN__

#include <emscripten.h>
#include <emscripten/html5.h>

#include "System.h"
#include "TimerMan.h"
#include "WindowMan.h"
#include "FrameMan.h"
#include "RenderTarget.h"
#include "UInputMan.h"
#include "AudioMan.h"
#include "MusicMan.h"
#include "MenuMan.h"
#include "ConsoleMan.h"
#include "ActivityMan.h"
#include "MovableMan.h"
#include "SceneMan.h"
#include "LuaMan.h"
#include "PerformanceMan.h"
#include "ThreadMan.h"
#include "PostProcessMan.h"
#include "PresetMan.h"
#include "LoadingScreen.h"
#include "GameActivity.h"
#include "CameraMan.h"
#include "Box2DManager.h"
#include "Constants.h"

namespace RTE {

enum class WebLoopState {
    Menu,
    Game,
    Quit
};

static WebLoopState s_WebLoopState = WebLoopState::Menu;
static bool s_AutoStartChecked = false;
static bool s_BuildTimestampLogged = false;

/// Check URL ?autostart parameter and launch directly into a skirmish game.
/// Usage: CortexCommand.html?autostart
inline bool CheckAutoStart() {
    if (s_AutoStartChecked) return false;
    s_AutoStartChecked = true;

    int hasParam = EM_ASM_INT({
        return (window.location.search.indexOf('autostart') >= 0) ? 1 : 0;
    });
    if (!hasParam) return false;

    EM_ASM({ console.log('[CC] AutoStart: launching skirmish...'); });

    // Find activity preset
    const Entity* basePreset = g_PresetMan.GetEntityPreset("GAScripted", "Skirmish Defense");
    if (!basePreset) {
        EM_ASM({ console.log('[CC] AutoStart: FAILED — could not find Skirmish Defense activity'); });
        return false;
    }
    GameActivity* game = dynamic_cast<GameActivity*>(basePreset->Clone());
    if (!game) {
        EM_ASM({ console.log('[CC] AutoStart: FAILED — clone/cast failed'); });
        return false;
    }

    // Set scene
    int sceneResult = g_SceneMan.SetSceneToLoad("Metankora Highlands", true, false);
    EM_ASM({ console.log('[CC] AutoStart: SetSceneToLoad result=' + $0); }, sceneResult);

    // Configure game
    game->SetDifficulty(Activity::DifficultySetting::EasyDifficulty);
    game->SetStartingGold(5000);
    game->ClearPlayers(false);
    game->AddPlayer(Players::PlayerOne, true, Activity::Teams::TeamOne, 0);
    game->SetCPUTeam(Activity::Teams::TeamTwo);
    game->SetTeamAISkill(Activity::Teams::TeamTwo, Activity::AISkillSetting::DefaultSkill);
    game->SetTeamTech(Activity::Teams::TeamOne, "-All-");
    game->SetTeamTech(Activity::Teams::TeamTwo, "-All-");

    g_ActivityMan.SetStartActivity(game);
    g_ActivityMan.SetRestartActivity();
    EM_ASM({ console.log('[CC] AutoStart: activity configured, restart queued'); });
    return true;
}

// PollSDLEvents is defined at global scope in Main.cpp (included before this header)

/// Called once per animation frame by emscripten_set_main_loop().
/// Dispatches to the appropriate menu or game loop body.
inline void WebMainLoopIteration_Impl() {
    if (!s_BuildTimestampLogged) {
        s_BuildTimestampLogged = true;
        EM_ASM({ console.log('[CC] Build: ' + UTF8ToString($0) + ' ' + UTF8ToString($1)); },
               __DATE__, __TIME__);
    }
    if (System::IsSetToQuit()) {
        s_WebLoopState = WebLoopState::Quit;
        emscripten_cancel_main_loop();
        return;
    }

    switch (s_WebLoopState) {

    // -----------------------------------------------------------------------
    case WebLoopState::Menu: {
        g_WindowMan.ClearBackbuffer();
        PollSDLEvents();
        if (System::IsSetToQuit()) break;

        g_WindowMan.Update();
        g_UInputMan.Update();
        g_TimerMan.Update();
        g_TimerMan.UpdateSim();
        g_AudioMan.Update();
        g_MusicMan.Update();

        if (g_WindowMan.ResolutionChanged()) {
            g_MenuMan.Reinitialize();
            g_ConsoleMan.Destroy();
            g_ConsoleMan.Initialize();
            g_LoadingScreen.CreateLoadingSplash();
            g_WindowMan.CompleteResolutionChange();
        }

        bool doneWithMenu = g_MenuMan.Update();

        // AutoStart: skip the menu and jump straight into a skirmish.
        // SetRestartActivity was already called in CheckAutoStart — the
        // pre-sim handler in the Game state will call RestartActivity().
        if (CheckAutoStart()) {
            doneWithMenu = true;
        }

        g_ConsoleMan.Update();
        g_UInputMan.EndFrame();

        g_WindowMan.GetScreenBuffer()->Begin();
        g_MenuMan.Draw();
        g_ConsoleMan.Draw(g_FrameMan.GetBackBuffer32());
        g_WindowMan.GetScreenBuffer()->End();
        g_WindowMan.UploadFrame();

        if (doneWithMenu) {
            g_MenuMan.SetIsInMenuScreen(false);
            s_WebLoopState = WebLoopState::Game;
            g_TimerMan.ResetTime();
            g_TimerMan.PauseSim(false);
            EM_ASM({ console.log('[CC] Transitioning to Game state'); });
        }
        break;
    }

    // -----------------------------------------------------------------------
    case WebLoopState::Game: {
        {
            static int loopCallCount = 0;
            ++loopCallCount;
            if (loopCallCount <= 5 || loopCallCount % 120 == 0) {
                EM_ASM({ console.log('[CC] Game loop call #' + $0); }, loopCallCount);
            }
        }
        PollSDLEvents();
        if (System::IsSetToQuit()) break;

        g_WindowMan.Update();
        g_WindowMan.ClearBackbuffer();
        g_TimerMan.Update();

        // Debug: log timer state after Update
        {
            static int timerDbg = 0;
            if (++timerDbg <= 10 || timerDbg % 300 == 0) {
                EM_ASM({ console.log('[CC] Timer: accum=' + $0 +
                         ' delta=' + $1 + ' paused=' + $2 +
                         ' realTicks=' + $3 + ' timeScale=' + $4); },
                       (double)g_TimerMan.GetSimAccumulator(),
                       (double)g_TimerMan.GetDeltaTimeTicks(),
                       (int)g_TimerMan.IsSimPaused(),
                       (double)g_TimerMan.GetRealTimeTicks(),
                       (double)g_TimerMan.GetTimeScale());
            }
        }

        // Handle pending activity restart (e.g. from autostart or scene change).
        // This must happen before the sim loop since TimeForSimUpdate() may be
        // false on the first frame after ResetTime().
        if (g_ActivityMan.ActivitySetToRestart()) {
            EM_ASM({ console.log('[CC] Game: RestartActivity (pre-sim) inActivity=' +
                     $0 + ' running=' + $1); },
                   (int)g_ActivityMan.IsInActivity(), (int)g_ActivityMan.ActivityRunning());
            g_LoadingScreen.DrawLoadingSplash();
            g_WindowMan.UploadFrame();
            bool ok = g_ActivityMan.RestartActivity();
            EM_ASM({ console.log('[CC] Game: RestartActivity returned ' + $0 +
                     ' inActivity=' + $1 + ' running=' + $2); },
                   (int)ok, (int)g_ActivityMan.IsInActivity(), (int)g_ActivityMan.ActivityRunning());
            g_TimerMan.ResetTime();
            g_TimerMan.PauseSim(false);
            g_PerformanceMan.ResetSimUpdateTimer();
            break;  // yield to browser, render next frame
        }

        // Debug: log game loop state periodically
        {
            static int gameFrameCount = 0;
            if (++gameFrameCount % 120 == 1) {
                bool simPaused = !g_TimerMan.TimeForSimUpdate(); // approximate check
                bool inActivity = g_ActivityMan.IsInActivity();
                bool activityRunning = g_ActivityMan.ActivityRunning();
                bool timeForSim = g_TimerMan.TimeForSimUpdate();
                bool restartSet = g_ActivityMan.ActivitySetToRestart();
                EM_ASM({ console.log('[CC] Game frame=' + $0 +
                         ' simPaused=' + $1 + ' inActivity=' + $2 +
                         ' running=' + $3 + ' timeForSim=' + $4 +
                         ' restart=' + $5); },
                       gameFrameCount, (int)simPaused, (int)inActivity,
                       (int)activityRunning, (int)timeForSim, (int)restartSet);
            }
        }

        // Fixed-timestep simulation (all ticks that fit in one render frame).
        // Limit to a few ticks per frame to prevent the browser from stalling
        // when the accumulator is large (e.g. after a long loading screen).
        int simTicksThisFrame = 0;
        const int maxSimTicksPerFrame = 4;
        while (g_TimerMan.TimeForSimUpdate() && simTicksThisFrame < maxSimTicksPerFrame) {
            ++simTicksThisFrame;
            EM_ASM({ console.log('[CC] Sim tick #' + $0 + ' starting...'); }, simTicksThisFrame);

            g_PerformanceMan.NewPerformanceSample();
            g_PerformanceMan.UpdateMSPSU();
            g_TimerMan.UpdateSim();

            g_PerformanceMan.StartPerformanceMeasurement(PerformanceMan::SimTotal);

            EM_ASM({ console.log('[CC] Sim: LuaMan.Update...'); });
            g_LuaMan.Update();
            EM_ASM({ console.log('[CC] Sim: UInputMan.Update...'); });
            g_UInputMan.Update();
            EM_ASM({ console.log('[CC] Sim: FrameMan.Update...'); });
            g_FrameMan.Update();
            EM_ASM({ console.log('[CC] Sim: MovableMan.CompleteQueuedMOIDDrawings...'); });
            g_MovableMan.CompleteQueuedMOIDDrawings();
            EM_ASM({ console.log('[CC] Sim: ConsoleMan.Update...'); });
            g_ConsoleMan.Update();
            EM_ASM({ console.log('[CC] Sim: ActivityMan.Update...'); });
            g_ActivityMan.Update();

            EM_ASM({ console.log('[CC] Sim: Scene.Update...'); });
            if (g_SceneMan.GetScene()) g_SceneMan.GetScene()->Update();

            EM_ASM({ console.log('[CC] Sim: MovableMan.Update...'); });
            g_LuaMan.ClearScriptTimings();
            g_MovableMan.Update();
            g_PerformanceMan.UpdateSortedScriptTimings(g_LuaMan.GetScriptTimings());
            EM_ASM({ console.log('[CC] Sim: AudioMan.Update...'); });
            g_AudioMan.Update();
            EM_ASM({ console.log('[CC] Sim: MusicMan.Update...'); });
            g_MusicMan.Update();
            g_ActivityMan.LateUpdateGlobalScripts();
            EM_ASM({ console.log('[CC] Sim: ClearReloadEntity...'); });
            g_PresetMan.ClearReloadEntityPresetCalledThisUpdate();

            g_PerformanceMan.StopPerformanceMeasurement(PerformanceMan::SimTotal);
            g_UInputMan.EndFrame();
            EM_ASM({ console.log('[CC] Sim tick #' + $0 + ' complete'); }, simTicksThisFrame);

            // Transition back to menu if the activity ended
            if (!g_ActivityMan.IsInActivity()) {
                g_TimerMan.PauseSim(true);
                if (!g_ActivityMan.ActivitySetToRestart()) {
                    g_MenuMan.HandleTransitionIntoMenuLoop();
                    g_MenuMan.SetIsInMenuScreen(true);
                    g_UInputMan.DisableKeys(false);
                    g_UInputMan.TrapMousePos(false);
                    s_WebLoopState = WebLoopState::Menu;
                    break;
                }
            }

            if (g_ActivityMan.ActivitySetToRestart()) {
                g_LoadingScreen.DrawLoadingSplash();
                g_WindowMan.UploadFrame();
                g_ActivityMan.RestartActivity();
                break;
            }
            if (g_ActivityMan.ActivitySetToResume()) {
                g_ActivityMan.ResumeActivity();
                g_PerformanceMan.ResetSimUpdateTimer();
            }
        }

        // Log sim tick count
        {
            static int renderFrameCount = 0;
            if (++renderFrameCount % 120 == 1) {
                EM_ASM({ console.log('[CC] Render: simTicks=' + $0 + ' inActivity=' + $1); },
                       simTicksThisFrame, (int)g_ActivityMan.IsInActivity());
            }
        }

        // Render frame
        g_FrameMan.Draw();

        // Toggle Box2D debug draw with F9 key
        {
            static bool f9WasDown = false;
            bool f9Down = EM_ASM_INT({ return window._ccBox2DDebug !== undefined ? window._ccBox2DDebug : 1; });
            if (!f9Down && f9WasDown) {
                // Key was released — toggle handled in JS
            }
            f9WasDown = f9Down;
            g_Box2DMan.SetDebugDraw(f9Down);
        }

        // Box2D debug overlay — draws onto the 32bpp GUI buffer
        if (g_Box2DMan.IsActive() && g_Box2DMan.IsDebugDrawEnabled()) {
            g_Box2DMan.DrawDebug();
        }

        g_WindowMan.DrawPostProcessBuffer();
        g_WindowMan.UploadFrame();
        {
            static int frameCompleteCount = 0;
            ++frameCompleteCount;
            if (frameCompleteCount <= 5 || frameCompleteCount % 120 == 0) {
                EM_ASM({ console.log('[CC] Game frame complete #' + $0); }, frameCompleteCount);
            }
        }
        break;
    }

    case WebLoopState::Quit:
        emscripten_cancel_main_loop();
        break;
    }
}

} // namespace RTE

// WebMainLoopIteration() is defined in Main.cpp (which includes this header).
// It's forward-declared in WebPlatform.cpp — defined in Main.cpp's TU.

#endif // __EMSCRIPTEN__
