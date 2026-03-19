/**
 * glad/gl.h — Emscripten redirect passthrough
 *
 * This file is first on the include path for Emscripten builds.
 * It redirects to the real GLAD header plus adds WebGL2 compatibility stubs.
 */
#pragma once

/* Include the real GLAD header via the external include path */
#include "../../../../external/include/glad-2.0.0-beta/glad/gl.h"

/* Additional stubs for desktop-only GL enums used by rlgl.c */
#ifndef GL_BLEND_ADVANCED_COHERENT_KHR
#define GL_BLEND_ADVANCED_COHERENT_KHR    0x9285
#define GL_MULTIPLY_KHR                   0x9294
#define GL_SCREEN_KHR                     0x9295
#define GL_OVERLAY_KHR                    0x9296
#define GL_DARKEN_KHR                     0x9297
#define GL_LIGHTEN_KHR                    0x9298
#define GL_COLORDODGE_KHR                 0x9299
#define GL_COLORBURN_KHR                  0x929A
#define GL_HARDLIGHT_KHR                  0x929B
#define GL_SOFTLIGHT_KHR                  0x929C
#define GL_DIFFERENCE_KHR                 0x929E
#define GL_EXCLUSION_KHR                  0x92A0
#define GL_HSL_HUE_KHR                    0x92AD
#define GL_HSL_SATURATION_KHR             0x92AE
#define GL_HSL_COLOR_KHR                  0x92AF
#define GL_HSL_LUMINOSITY_KHR             0x92B0
#endif
