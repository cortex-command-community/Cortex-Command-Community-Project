/**
 * WebPlatform.cpp
 *
 * Implementation of Emscripten / WebAssembly platform helpers.
 * On non-Emscripten builds every function is a no-op so call sites require
 * no #ifdef guards.
 */

#include "WebPlatform.h"

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#include <emscripten/html5.h>
#include <emscripten/fetch.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <vector>
#include <string>
#include <atomic>

// minizip-ng compat API — same headers the game already uses.
#include "unzip.h"
#endif

#include <cstdlib>

namespace RTE {

#ifdef __EMSCRIPTEN__

// ---------------------------------------------------------------------------
// Main loop
// ---------------------------------------------------------------------------

void WebMainLoopIteration(); // defined in Main.cpp

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
    EM_ASM({
        if (typeof ccBlit8ToCanvas === 'function')
            ccBlit8ToCanvas($0, $1, $2, $3);
    }, (int)(uintptr_t)pixels8, width, height, (int)(uintptr_t)palette);
}

void WebPlatform_Blit32ToCanvas(const uint8_t* pixels32,
                                int width,
                                int height) {
    EM_ASM({
        if (typeof ccBlit32ToCanvas === 'function')
            ccBlit32ToCanvas($0, $1, $2);
    }, (int)(uintptr_t)pixels32, width, height);
}

// ---------------------------------------------------------------------------
// HTTP asset loading — parallel module fetch + MEMFS extract
// ---------------------------------------------------------------------------


static int s_extractSeq = 0; // monotonically increasing, gives each zip a unique name

/// Extract a zip archive from memory into Emscripten's MEMFS using the
/// minizip-ng unzip compat API (same API the game uses for .ccsave files).
static void ExtractZipToMemFS(const char* data, unsigned long size) {
    // Write the raw zip bytes to a unique temp path — important since multiple
    // parallel downloads can call this callback concurrently.
    char tmpPath[64];
    snprintf(tmpPath, sizeof(tmpPath), "/tmp/_ccmod_%d.zip", ++s_extractSeq);

    EM_ASM({ try { FS.mkdir('/tmp'); } catch(e) {} });

    FILE* f = fopen(tmpPath, "wb");
    if (!f) return;
    fwrite(data, 1, size, f);
    fclose(f);

    unzFile zf = unzOpen(tmpPath);
    if (!zf) { remove(tmpPath); return; }

    unz_global_info gi{};
    unzGetGlobalInfo(zf, &gi);

    for (unsigned long i = 0; i < gi.number_entry; ++i) {
        unz_file_info fi{};
        char filename[512]{};
        unzGetCurrentFileInfo(zf, &fi, filename, sizeof(filename), nullptr, 0, nullptr, 0);

        std::string path = std::string("/Data/") + filename;

        // Helper: ensure a MEMFS path exists (creates all components).
        // We call this via a simple EM_ASM that builds up the path incrementally.
        auto mkdirp = [](const std::string& dirPath) {
            EM_ASM({
                var full = UTF8ToString($0);
                var segs = full.split('/');
                var acc  = '';
                for (var s = 0; s < segs.length; s++) {
                    if (!segs[s]) { acc = '/'; continue; }
                    acc = acc + (acc === '/' ? '' : '/') + segs[s];
                    try { FS.mkdir(acc); } catch(e) {}
                }
            }, dirPath.c_str());
        };

        // Process entry based on type (directory or file).
        if (!path.empty() && path.back() == '/') {
            // Directory entry — just ensure it exists, do NOT open/read content.
            mkdirp(path.substr(0, path.size() - 1));
        } else {
            // Ensure parent directory exists.
            auto slash = path.rfind('/');
            if (slash != std::string::npos)
                mkdirp(path.substr(0, slash));

            int openErr = unzOpenCurrentFile(zf);
            if (openErr == UNZ_OK) {
                FILE* out = fopen(path.c_str(), "wb");
                if (out) {
                    char buf[65536];
                    int n;
                    while ((n = unzReadCurrentFile(zf, buf, sizeof(buf))) > 0)
                        fwrite(buf, 1, n, out);
                    fclose(out);
                } else {
                    EM_ASM({ console.warn('[CC] fopen failed for:', UTF8ToString($0)); }, path.c_str());
                }
                unzCloseCurrentFile(zf);
            } else {
                EM_ASM({ console.warn('[CC] unzOpenCurrentFile failed for:', UTF8ToString($0), 'err:', $1); },
                       path.c_str(), openErr);
            }
        }

        // Always advance to the next zip entry, regardless of the entry type.
        // This MUST be at the end of the loop body (not before processing) so that
        // unzOpenCurrentFile reads the same entry that unzGetCurrentFileInfo described.
        if (i + 1 < gi.number_entry)
            unzGoToNextFile(zf);
    }

    unzClose(zf);
    remove(tmpPath);
}


static std::atomic<int> s_fetchPending{0};

static void OnFetchSuccess(emscripten_fetch_t* fetch) {
    if (fetch->numBytes > 0)
        ExtractZipToMemFS(fetch->data, static_cast<unsigned long>(fetch->numBytes));
    emscripten_fetch_close(fetch);
    --s_fetchPending;
}

static void OnFetchFailure(emscripten_fetch_t* fetch) {
    EM_ASM({ console.warn('[CC] Module fetch failed: ' + UTF8ToString($0) + ' HTTP ' + $1); },
           fetch->url, fetch->status);
    emscripten_fetch_close(fetch);
    --s_fetchPending;
}

void WebPlatform_FetchModules(const std::vector<std::string>& modules) {
    if (modules.empty()) return;

    // Ensure /Data/ exists in MEMFS before the callbacks write to it.
    EM_ASM({ try { FS.mkdir('/Data'); } catch(e) {} });

    // Kick off all module downloads in parallel.
    // EMSCRIPTEN_FETCH_LOAD_TO_MEMORY (no SYNCHRONOUS flag): callbacks fire
    // from the browser event loop while main() yields in emscripten_sleep().
    // With -sSTACK_SIZE=2MB the callback stack is large enough for extraction.
    emscripten_fetch_attr_t attr;
    emscripten_fetch_attr_init(&attr);
    std::strcpy(attr.requestMethod, "GET");
    attr.attributes = EMSCRIPTEN_FETCH_LOAD_TO_MEMORY;
    attr.onsuccess  = OnFetchSuccess;
    attr.onerror    = OnFetchFailure;

    s_fetchPending.store(static_cast<int>(modules.size()));

    for (const std::string& mod : modules) {
        std::string url = "Data/" + mod + ".zip";
        emscripten_fetch(&attr, url.c_str());
    }

    // Block (via Asyncify) until all downloads and extractions complete.
    while (s_fetchPending.load() > 0)
        emscripten_sleep(16);
}

bool WebPlatform_AllModulesReady() {
    return true; // Sequential fetch in WebPlatform_FetchModules completes before returning.
}

// ---------------------------------------------------------------------------
// IDBFS persistence
// ---------------------------------------------------------------------------

static std::atomic<bool> s_idbfsReady{false};

extern "C" EMSCRIPTEN_KEEPALIVE void WebPlatform_NotifyIDBFSReady() {
    s_idbfsReady.store(true);
}

void WebPlatform_MountPersistentStorage() {
    s_idbfsReady.store(false);
    EM_ASM({
        if (typeof IDBFS === 'undefined') {
            // IDBFS not available — notify immediately and continue without persistence.
            console.warn('[CC] IDBFS unavailable; settings will not persist across reloads.');
            Module['_WebPlatform_NotifyIDBFSReady']();
            return;
        }
        try { FS.mkdir('/Userdata'); } catch(e) { /* already exists */ }
        FS.mount(IDBFS, {}, '/Userdata');
        // Populate MEMFS from IndexedDB (true = IDB → MEMFS).
        FS.syncfs(true, function(err) {
            if (err) console.warn('[CC] IDBFS initial sync error:', err);
            Module['_WebPlatform_NotifyIDBFSReady']();
        });
    });
    // Block (via Asyncify) until the IDB sync completes (max ~3 s).
    for (int waited = 0; !s_idbfsReady.load() && waited < 3000; waited += 16)
        emscripten_sleep(16);
}

void WebPlatform_SyncSavesToDisk() {
    EM_ASM({
        if (typeof FS !== 'undefined' && typeof IDBFS !== 'undefined')
            FS.syncfs(false, function(err) {
                if (err) console.warn('[CC] IDBFS sync error:', err);
            });
    });
}

// ---------------------------------------------------------------------------
// Input — Pointer Lock
// ---------------------------------------------------------------------------

void WebPlatform_RequestPointerLock() {
    emscripten_request_pointerlock("#canvas", 0);
}

void WebPlatform_ReleasePointerLock() {
    emscripten_exit_pointerlock();
}

// ---------------------------------------------------------------------------
// Display — Fullscreen
// ---------------------------------------------------------------------------

void WebPlatform_RequestFullscreen() {
    EmscriptenFullscreenStrategy strategy{};
    strategy.scaleMode                 = EMSCRIPTEN_FULLSCREEN_SCALE_STRETCH;
    strategy.canvasResolutionScaleMode = EMSCRIPTEN_FULLSCREEN_CANVAS_SCALE_HIDEF;
    strategy.filteringMode             = EMSCRIPTEN_FULLSCREEN_FILTERING_NEAREST;
    emscripten_request_fullscreen_strategy("#canvas", 0, &strategy);
}

void WebPlatform_ExitFullscreen() {
    emscripten_exit_fullscreen();
}

// ---------------------------------------------------------------------------
// Window title
// ---------------------------------------------------------------------------

void WebPlatform_SetWindowTitle(const std::string& title) {
    EM_ASM({ document.title = UTF8ToString($0); }, title.c_str());
}

// ---------------------------------------------------------------------------
// Fatal error
// ---------------------------------------------------------------------------

[[noreturn]] void WebPlatform_FatalError(const std::string& message) {
    EM_ASM({
        var msg = UTF8ToString($0);
        var overlay = document.getElementById('error-overlay');
        if (overlay) { overlay.style.display = 'block'; overlay.textContent = 'FATAL ERROR:\n\n' + msg; }
        console.error('[CC FATAL]', msg);
    }, message.c_str());
    emscripten_force_exit(1);
    __builtin_unreachable();
}

#else // !__EMSCRIPTEN__ — native stubs

void WebPlatform_StartMainLoop(int, bool) {}
void WebPlatform_StopMainLoop() {}
void WebPlatform_Blit8ToCanvas(const uint8_t*, const uint8_t*, int, int) {}
void WebPlatform_Blit32ToCanvas(const uint8_t*, int, int) {}
void WebPlatform_FetchModules(const std::vector<std::string>&) {}
bool WebPlatform_AllModulesReady() { return true; }
void WebPlatform_MountPersistentStorage() {}
void WebPlatform_SyncSavesToDisk() {}
void WebPlatform_RequestPointerLock() {}
void WebPlatform_ReleasePointerLock() {}
void WebPlatform_RequestFullscreen() {}
void WebPlatform_ExitFullscreen() {}
void WebPlatform_SetWindowTitle(const std::string&) {}
[[noreturn]] void WebPlatform_FatalError(const std::string& message) {
    std::fprintf(stderr, "FATAL: %s\n", message.c_str());
    std::abort();
}

#endif // __EMSCRIPTEN__

} // namespace RTE
