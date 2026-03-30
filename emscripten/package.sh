#!/usr/bin/env bash
# =============================================================================
# Cortex Command WASM — Post-link packaging step
#
# Meson's Emscripten support doesn't propagate all emcc link flags cleanly.
# This script takes the compiled WASM+JS from build-web/ and re-links it
# using emcc with the full set of Emscripten flags to produce:
#   - CortexCommand.html  (with embedded shell)
#   - CortexCommand.js    (loader)
#   - CortexCommand.wasm  (game binary)
#   - CortexCommand.data  (preloaded Base.rte assets)
#
# Usage: bash emscripten/package.sh
# Run from repo root after `meson setup build-web && ninja -C build-web`
# =============================================================================

set -euo pipefail

REPO_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
BUILD_DIR="$REPO_ROOT/build-web"
SHELL_HTML="$REPO_ROOT/emscripten/shell.html"
PRE_JS="$REPO_ROOT/emscripten/pre.js"
DATA_DIR="$REPO_ROOT/Data"

# Activate emsdk if emcc isn't on PATH
if ! command -v emcc &>/dev/null; then
    if [ -f "$HOME/emsdk/emsdk_env.sh" ]; then
        source "$HOME/emsdk/emsdk_env.sh"
    else
        echo "ERROR: emcc not found. Source emsdk_env.sh first."
        exit 1
    fi
fi

echo "==> Packaging CortexCommand WASM with Emscripten flags..."
echo "    Source: $BUILD_DIR/CortexCommand.js + CortexCommand.wasm"

# Collect ALL object files Meson compiled
OBJS=$(find "$BUILD_DIR/CortexCommand.js.p" -name "*.o" | sort)
LIBS=(
    "$BUILD_DIR/Source/Lua/liblua_bindings.a"
    "$BUILD_DIR/Source/Managers/libLuaMan.a"
    "$BUILD_DIR/external/sources/lua-5.1.5/liblua51.a"
    "$BUILD_DIR/external/sources/luabind-0.7.1/libluabind071.a"
    "$BUILD_DIR/external/sources/minizip-ng-4.0.0/libminizip.a"
    "/tmp/sdl3-wasm/libSDL3.a"
)

# Emscripten flags
EM_FLAGS=(
    # WebGL2 / SDL3
    "-sUSE_WEBGL2=1"
    "-sFULL_ES3=1"
    # Memory — IMPORTED_MEMORY required when linking against SDL3 static lib
    # which may have been built with different memory settings
    "-sALLOW_MEMORY_GROWTH=1"
    "-sINITIAL_MEMORY=268435456"
    "-sMAXIMUM_MEMORY=2147483648"
    "-sIMPORTED_MEMORY"
    # Exports
    "-sEXPORTED_RUNTIME_METHODS=[FS,callMain,ccall,cwrap]"
    "-sEXPORTED_FUNCTIONS=[_main]"
    "-sENVIRONMENT=web"
    "-sEXIT_RUNTIME=0"
    # Error handling
    "-sERROR_ON_UNDEFINED_SYMBOLS=0"  # already linked cleanly
    # Asyncify for blocking load patterns (used by FetchModules + MountPersistentStorage)
    # Increase WASM call stack — std::filesystem + zip extraction overflow the default 64KB
    "-sSTACK_SIZE=2097152"          # 2MB main WASM stack
    "-sASYNCIFY=1"
    "-sASYNCIFY_STACK_SIZE=131072"  # 128KB Asyncify suspend/resume buffer
    # Persistent storage via IndexedDB (required for IDBFS filesystem type)
    "-lidbfs.js"
    # emscripten_fetch API for HTTP module loading
    "-sFETCH=1"
    # Dependencies via ports
    "-sUSE_ZLIB=1"
    "-sUSE_LIBPNG=1"
    # Base.rte is preloaded with ALL files including audio.
    # This makes the initial download larger (~115MB) but ensures all Base.rte
    # sounds are available immediately for the menu and core gameplay.
    "--preload-file" "$DATA_DIR/Base.rte@/Data/Base.rte"
    # Shell and pre.js
    "--shell-file" "$SHELL_HTML"
    "--pre-js"     "$PRE_JS"
    # Optimisation
    "-O2"
)

OUTPUT="$BUILD_DIR/CortexCommand.html"

echo "==> Running emcc link step..."
emcc \
    $OBJS \
    "${LIBS[@]}" \
    -lm \
    "${EM_FLAGS[@]}" \
    -o "$OUTPUT" \
    2>&1

# ---------------------------------------------------------------------------
# Generate per-module zip files for HTTP lazy loading.
# These are served directly by the dev server alongside CortexCommand.html.
# The game fetches them in parallel via WebPlatform_FetchModules() before
# loading each module; Base.rte is already embedded above.
# ---------------------------------------------------------------------------
echo "==> Packaging per-module zips for lazy HTTP loading..."

LAZY_MODULES=(
  "Coalition.rte" "Imperatus.rte" "Techion.rte" "Dummy.rte"
  "Ronin.rte" "Browncoats.rte" "Uzira.rte" "MuIlaak.rte" "Missions.rte"
)

mkdir -p "$BUILD_DIR/Data"

for MODULE in "${LAZY_MODULES[@]}"; do
  ZIP="$BUILD_DIR/Data/${MODULE}.zip"
  SRC="$DATA_DIR/${MODULE}"
  if [[ -d "$SRC" ]]; then
    echo "    Zipping $MODULE (with audio)..."
    rm -f "$ZIP"
    (cd "$DATA_DIR" && find "$MODULE" -type f | zip -q "$ZIP" -@)
    SIZE=$(du -sh "$ZIP" 2>/dev/null | awk '{print $1}')
    echo "    $ZIP ($SIZE)"
  else
    echo "    WARNING: $SRC not found, skipping"
  fi
done

# Base.rte audio is included in the preload .data file — no separate zip needed.

echo ""
echo "==> Done!"
echo "    $OUTPUT"
echo "    $(ls -lh "$BUILD_DIR"/CortexCommand.{html,js,wasm,data} 2>/dev/null | awk '{print $5, $9}')"
echo ""
echo "==> To test: bash emscripten/serve.py build-web 8080"
echo "    Then open http://localhost:8080/CortexCommand.html"
