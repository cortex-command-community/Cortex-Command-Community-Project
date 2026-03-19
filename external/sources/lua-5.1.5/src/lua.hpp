/**
 * lua.hpp — Emscripten build redirect
 * Luabind includes this C++ wrapper for Lua. Redirect to PUC-Lua 5.1 headers.
 */
#pragma once

extern "C" {
#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
}
