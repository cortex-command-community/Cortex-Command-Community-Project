/**
 * WebPlatform.h
 *
 * Emscripten / WebAssembly platform helpers for Cortex Command Community Project.
 *
 * Provides:
 *   - emscripten_set_main_loop() wrappers
 *   - Canvas2D typed-array blit path for the CPU-rendered GUI overlay
 *   - HTTP module fetching for lazy asset loading (no prebaked .data file)
 *   - IDBFS sync helpers for persistent settings and save data
 *   - Pointer lock and fullscreen request helpers
 *
 * On non-Emscripten builds all public functions are no-ops or pass-throughs.
 */

#pragma once

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#include <emscripten/html5.h>
#include <emscripten/fetch.h>
#endif

#include <cstdint>
#include <string>
#include <vector>

namespace RTE {

    // -------------------------------------------------------------------------
    // Main loop
    // -------------------------------------------------------------------------

    /// Register WebMainLoopIteration() with the browser's requestAnimationFrame.
    /// @param fps  Target FPS (0 = match display refresh rate).
    void WebPlatform_StartMainLoop(int fps = 0, bool simulateInf = true);

    void WebPlatform_StopMainLoop();

    // -------------------------------------------------------------------------
    // Canvas2D rendering
    // -------------------------------------------------------------------------

    /// Blit an 8bpp indexed-colour buffer to the overlay Canvas2D element.
    /// The palette must be a 256-entry RGBA array (1024 bytes).
    void WebPlatform_Blit8ToCanvas(const uint8_t* pixels8,
                                   const uint8_t* palette,
                                   int width,
                                   int height);

    /// Blit a 32bpp RGBA buffer to the overlay Canvas2D element.
    void WebPlatform_Blit32ToCanvas(const uint8_t* pixels32,
                                    int width,
                                    int height);

    // -------------------------------------------------------------------------
    // HTTP asset loading
    // -------------------------------------------------------------------------

    /// Download a list of module zip files from the server in parallel and
    /// extract them into MEMFS so the game can load them synchronously.
    /// Blocks (via Asyncify) until all downloads and extractions complete.
    ///
    /// @param modules  Names of modules to fetch, e.g. {"Base.rte", "Ronin.rte"}
    ///                 Each is fetched as Data/<name>.zip from the base URL.
    void WebPlatform_FetchModules(const std::vector<std::string>& modules);

    /// Returns true once WebPlatform_FetchModules has completed for all modules.
    bool WebPlatform_AllModulesReady();

    // -------------------------------------------------------------------------
    // IDBFS persistence
    // -------------------------------------------------------------------------

    /// Mount /Userdata on IDBFS and populate MEMFS from the IndexedDB store.
    /// Must be called before main() reads settings.  Blocks via Asyncify until
    /// the initial sync is complete (or a timeout expires).
    void WebPlatform_MountPersistentStorage();

    /// Flush MEMFS writes in /Userdata back to IndexedDB.
    /// Asynchronous — returns immediately; safe to call on every save.
    void WebPlatform_SyncSavesToDisk();

    // -------------------------------------------------------------------------
    // Input
    // -------------------------------------------------------------------------

    /// Request browser Pointer Lock on the game canvas.  Must be called from
    /// within a user-gesture handler (click / keydown).
    void WebPlatform_RequestPointerLock();

    /// Release Pointer Lock.
    void WebPlatform_ReleasePointerLock();

    // -------------------------------------------------------------------------
    // Display
    // -------------------------------------------------------------------------

    /// Request fullscreen on the game canvas.  Must be called from within a
    /// user-gesture handler.
    void WebPlatform_RequestFullscreen();

    /// Exit fullscreen.
    void WebPlatform_ExitFullscreen();

    /// Update the browser tab title.
    void WebPlatform_SetWindowTitle(const std::string& title);

    // -------------------------------------------------------------------------
    // Error handling
    // -------------------------------------------------------------------------

    /// Display a fatal error message in the error overlay and abort.
    [[noreturn]] void WebPlatform_FatalError(const std::string& message);

    // -------------------------------------------------------------------------
    // Utility
    // -------------------------------------------------------------------------

    inline constexpr bool WebPlatform_IsWebBuild() {
#ifdef __EMSCRIPTEN__
        return true;
#else
        return false;
#endif
    }

} // namespace RTE
