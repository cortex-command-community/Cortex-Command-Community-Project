/**
 * fmod_audio_decoders.cpp
 *
 * Compilation unit for single-header audio decoder implementations.
 * dr_flac (FLAC) and stb_vorbis (OGG Vorbis) — both public domain.
 */

#define DR_FLAC_IMPLEMENTATION
#include "dr_flac.h"

// stb_vorbis: include the FULL implementation (not header-only).
// Do NOT define STB_VORBIS_HEADER_ONLY here.
#include "stb_vorbis.c"
