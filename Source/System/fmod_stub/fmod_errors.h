/**
 * FMOD error string stub for Emscripten builds.
 * Maps FMOD_RESULT codes to human-readable strings.
 */

#pragma once
#include "fmod.hpp"

inline const char* FMOD_ErrorString(FMOD_RESULT result) {
    switch (result) {
        case FMOD_OK:               return "No errors.";
        default:                    return "Unknown FMOD error (stub build).";
    }
}
