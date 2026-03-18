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

#include "System.h"
#include "TimerMan.h"
#include "WindowMan.h"
#include "FrameMan.h"
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

namespace RTE {

enum class WebLoopState {
    Menu,
    Game,
    Quit
};

static WebLoopState s_WebLoopState = WebLoopState::Menu;

// Forward declarations of the loop body helpers defined in Main.cpp
void PollSDLEvents();

/// Called once per animation frame by emscripten_set_main_loop().
/// Dispatches to the appropriate menu or game loop body.
inline void WebMainLoopIteration_Impl() {
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
            g_TimerMan.PauseSim(false);
        }
        break;
    }

    // -----------------------------------------------------------------------
    case WebLoopState::Game: {
        PollSDLEvents();
        if (System::IsSetToQuit()) break;

        g_WindowMan.Update();
        g_WindowMan.ClearBackbuffer();
        g_TimerMan.Update();

        // Fixed-timestep simulation (all ticks that fit in one render frame)
        while (g_TimerMan.TimeForSimUpdate()) {
            g_PerformanceMan.NewPerformanceSample();
            g_PerformanceMan.UpdateMSPSU();
            g_TimerMan.UpdateSim();

            g_PerformanceMan.StartPerformanceMeasurement(PerformanceMan::SimTotal);

            g_LuaMan.Update();
            g_UInputMan.Update();
            g_FrameMan.Update();
            g_MovableMan.CompleteQueuedMOIDDrawings();
            g_ConsoleMan.Update();
            g_ActivityMan.Update();

            if (g_SceneMan.GetScene()) g_SceneMan.GetScene()->Update();

            g_LuaMan.ClearScriptTimings();
            g_MovableMan.Update();
            g_PerformanceMan.UpdateSortedScriptTimings(g_LuaMan.GetScriptTimings());
            g_AudioMan.Update();
            g_MusicMan.Update();
            g_ActivityMan.LateUpdateGlobalScripts();
            g_PresetMan.ClearReloadEntityPresetCalledThisUpdate();

            g_PerformanceMan.StopPerformanceMeasurement(PerformanceMan::SimTotal);
            g_UInputMan.EndFrame();

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

        // Render frame
        g_FrameMan.Draw();
        g_WindowMan.DrawPostProcessBuffer();
        g_WindowMan.UploadFrame();
        break;
    }

    case WebLoopState::Quit:
        emscripten_cancel_main_loop();
        break;
    }
}

} // namespace RTE

// Called from WebPlatform.cpp (forward-declared there)
inline void WebMainLoopIteration() {
    RTE::WebMainLoopIteration_Impl();
}

#endif // __EMSCRIPTEN__
