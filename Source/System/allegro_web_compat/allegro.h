/**
 * allegro.h — Emscripten build redirect
 *
 * When building with Emscripten this file is found first on the include path
 * (the emscripten_inc_dirs in meson.build prepend this directory). It
 * redirects to our AllegroCompat shim instead of the real Allegro 4 headers.
 *
 * Native builds never see this directory because the Allegro 4 subproject
 * provide its own headers earlier in the include path.
 */
#pragma once
#include "AllegroCompat.h"
