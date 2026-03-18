/**
 * allegro/internal/aintern.h — Emscripten stub
 * AllegroTools.cpp includes this for internal Allegro blender constants.
 * We define the constants it actually uses.
 */
#pragma once

// Blender function placeholders used by set_blender_mode_ex
typedef unsigned long (*_BLENDER_FUNC)(unsigned long, unsigned long, unsigned long);
extern _BLENDER_FUNC _blender_black;

// No-op blender that returns the destination unchanged
inline unsigned long _blender_black_impl(unsigned long x, unsigned long y, unsigned long n) {
    return y;
}
