/**
 * WebPlatform.cpp
 *
 * Implementation of Emscripten / WebAssembly platform helpers.
 * On non-Emscripten builds the functions are compiled as no-ops so the same
 * call sites can exist in shared code without #ifdef everywhere.
 */

#include "WebPlatform.h"

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#include <emscripten/html5.h>
#endif

#include <cstdio>
#include <cstdlib>

namespace RTE {

#ifdef __EMSCRIPTEN__

// ---------------------------------------------------------------------------
// Main loop
// ---------------------------------------------------------------------------

// Forward declaration — defined in Main.cpp
void WebMainLoopIteration();

void WebPlatform_StartMainLoop(int fps, bool simulateInf) {
    emscripten_set_main_loop(WebMainLoopIteration, fps, simulateInf ? 1 : 0);
}

void WebPlatform_StopMainLoop() {
    emscripten_cancel_main_loop();
}

// ---------------------------------------------------------------------------
// Canvas2D typed-array blit
// ---------------------------------------------------------------------------

void WebPlatform_Blit8ToCanvas(const uint8_t* pixels8,
                               const uint8_t* palette,
                               int width,
                               int height) {
    // Call the JS function defined in emscripten/shell.html:
    //   ccBlit8ToCanvas(src8ptr, width, height, palPtr)
    //
    // We pass raw WASM heap offsets. The JS side wraps Module.HEAPU8 as a
    // zero-copy view, performs the palette expansion, and calls putImageData().
    EM_ASM({
        if (typeof ccBlit8ToCanvas === 'function') {
            ccBlit8ToCanvas($0, $1, $2, $3);
        }
    }, (int)(uintptr_t)pixels8, width, height, (int)(uintptr_t)palette);
}

void WebPlatform_Blit32ToCanvas(const uint8_t* pixels32,
                                int width,
                                int height) {
    EM_ASM({
        if (typeof ccBlit32ToCanvas === 'function') {
            ccBlit32ToCanvas($0, $1, $2);
        }
    }, (int)(uintptr_t)pixels32, width, height);
}

// ---------------------------------------------------------------------------
// Persistent storage (IDBFS)
// ---------------------------------------------------------------------------

void WebPlatform_SyncSavesToDisk() {
    // Flush MEMFS writes to IndexedDB asynchronously.
    EM_ASM({
        if (typeof FS !== 'undefined' && typeof IDBFS !== 'undefined') {
            FS.syncfs(false, function(err) {
                if (err) console.warn('[CC] IDBFS sync error:', err);
            });
        }
    });
}

// ---------------------------------------------------------------------------
// Window title
// ---------------------------------------------------------------------------

void WebPlatform_SetWindowTitle(const std::string& title) {
    EM_ASM({
        document.title = UTF8ToString($0);
    }, title.c_str());
}

// ---------------------------------------------------------------------------
// Fatal error
// ---------------------------------------------------------------------------

[[noreturn]] void WebPlatform_FatalError(const std::string& message) {
    // Display the error in the overlay div so the user can read it, then abort.
    EM_ASM({
        var msg = UTF8ToString($0);
        var overlay = document.getElementById('error-overlay');
        if (overlay) {
            overlay.style.display = 'block';
            overlay.textContent = 'FATAL ERROR:\n\n' + msg;
        }
        console.error('[CC FATAL]', msg);
    }, message.c_str());

    // emscripten_force_exit causes a clean WASM trap that the browser handles
    // gracefully (no alert(), no hang).
    emscripten_force_exit(1);
    // Unreachable, but satisfies [[noreturn]]
    __builtin_unreachable();
}

#else // !__EMSCRIPTEN__

// Native build stubs — all no-ops.
void WebPlatform_StartMainLoop(int, bool) {}
void WebPlatform_StopMainLoop() {}
void WebPlatform_Blit8ToCanvas(const uint8_t*, const uint8_t*, int, int) {}
void WebPlatform_Blit32ToCanvas(const uint8_t*, int, int) {}
void WebPlatform_SyncSavesToDisk() {}
void WebPlatform_SetWindowTitle(const std::string&) {}
[[noreturn]] void WebPlatform_FatalError(const std::string& message) {
    std::fprintf(stderr, "FATAL: %s\n", message.c_str());
    std::abort();
}

#endif // __EMSCRIPTEN__

} // namespace RTE
