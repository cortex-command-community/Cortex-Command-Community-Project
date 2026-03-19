project('luajit', 'c', version: '5.1.5')

# PUC-Lua 5.1.5 — drop-in replacement for LuaJIT 2.1 in interpreter mode.
# Used for the Emscripten/WASM build where LuaJIT has no architecture backend.
#
# API is Lua 5.1 compatible (same as LuaJIT 2.1 in interpreter mode).
# LuaJIT-specific extensions (jit.*, ffi.*) are unavailable but mods rarely use them.

cc = meson.get_compiler('c')
libm = cc.find_library('m', required: false)

lua_sources = files(
  'src/lapi.c',
  'src/lauxlib.c',
  'src/lbaselib.c',
  'src/lcode.c',
  'src/ldblib.c',
  'src/ldebug.c',
  'src/ldo.c',
  'src/ldump.c',
  'src/lfunc.c',
  'src/lgc.c',
  'src/linit.c',
  'src/liolib.c',
  'src/llex.c',
  'src/lmathlib.c',
  'src/lmem.c',
  'src/loadlib.c',
  'src/lobject.c',
  'src/lopcodes.c',
  'src/loslib.c',
  'src/lparser.c',
  'src/lstate.c',
  'src/lstring.c',
  'src/lstrlib.c',
  'src/ltable.c',
  'src/ltablib.c',
  'src/lundump.c',
  'src/ltm.c',
  'src/lvm.c',
  'src/lzio.c',
)

lua_inc = include_directories('src')

lua_args = [
  '-DLUA_USE_POSIX',
  '-DLUA_USE_DLOPEN',
  '-w',  # suppress warnings
]

libluajit = static_library('lua51',
  lua_sources,
  include_directories: lua_inc,
  c_args: lua_args,
  dependencies: [libm],
  build_by_default: true,
)

# Expose under the 'luajit' name so the rest of the build system works unchanged.
luajit_source_dir = lua_inc

luajit_dep = declare_dependency(
  link_with: libluajit,
  include_directories: lua_inc,
  compile_args: lua_args,
)

meson.override_dependency('luajit', luajit_dep)
