/**
 * WebPlatform.h
 *
 * Emscripten / WebAssembly platform helpers for Cortex Command Community Project.
 *
 * Provides:
 *   - emscripten_set_main_loop() wrappers that adapt the game's blocking loops
 *     into browser-compatible per-frame callbacks.
 *   - Canvas2D typed-array blit path for the 8bpp software rasterizer output.
 *   - IDBFS sync helpers for persistent save data.
 *   - Async module fetching bridge for lazy data module loading.
 *
 * On non-Emscripten builds all public functions are no-ops or pass-throughs.
 */

#pragma once

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#include <emscripten/html5.h>
#endif

#include <cstdint>
#include <string>

namespace RTE {

    /**
     * Must be called once from main() before entering any game loop.
     * Registers the per-frame callback with the browser's requestAnimationFrame.
     *
     * @param fps          Target frames per second (0 = use browser rAF rate, i.e. 60).
     * @param simulateInf  If true, emscripten treats the loop as infinite (normal mode).
     */
    void WebPlatform_StartMainLoop(int fps = 0, bool simulateInf = true);

    /**
     * Cancel the registered main loop.  Useful for error shutdown.
     */
    void WebPlatform_StopMainLoop();

    /**
     * Blit an 8bpp indexed-color buffer to the overlay Canvas2D element.
     *
     * The palette must be a 256-entry RGBA array (1024 bytes).
     * Internally calls ccBlit8ToCanvas() in the shell JS via EM_ASM.
     *
     * On non-Emscripten builds this is a no-op (GL path handles everything).
     *
     * @param pixels8   Pointer to 8bpp pixel data (width * height bytes)
     * @param palette   Pointer to 256-entry RGBA palette (256 * 4 bytes)
     * @param width     Image width
     * @param height    Image height
     */
    void WebPlatform_Blit8ToCanvas(const uint8_t* pixels8,
                                   const uint8_t* palette,
                                   int width,
                                   int height);

    /**
     * Blit a 32bpp RGBA buffer to the overlay Canvas2D element.
     * Used for the GUI / post-process overlay layer.
     *
     * @param pixels32  Pointer to RGBA pixel data (width * height * 4 bytes)
     * @param width     Image width
     * @param height    Image height
     */
    void WebPlatform_Blit32ToCanvas(const uint8_t* pixels32,
                                    int width,
                                    int height);

    /**
     * Flush pending IDBFS writes so save games persist across reloads.
     * Should be called from the game's save-game code path.
     * Asynchronous — returns immediately; callback fires when sync completes.
     */
    void WebPlatform_SyncSavesToDisk();

    /**
     * Update the browser tab title.
     */
    void WebPlatform_SetWindowTitle(const std::string& title);

    /**
     * Display a fatal error message in the error overlay div and abort.
     * Replaces RTEAbort on web builds.
     */
    [[noreturn]] void WebPlatform_FatalError(const std::string& message);

    /**
     * Returns true when running inside a browser (Emscripten build).
     */
    inline constexpr bool WebPlatform_IsWebBuild() {
#ifdef __EMSCRIPTEN__
        return true;
#else
        return false;
#endif
    }

} // namespace RTE
