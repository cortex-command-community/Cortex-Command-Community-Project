project('Cortex-Command-Community-Project', ['cpp','c'], subproject_dir: 'external/sources', default_options:['cpp_std=c++20', 'buildtype=release', 'default_library=static'], version:'7.0.0-dev', meson_version:'>=1.6.0')

#### Build environment Setup ####

compiler = meson.get_compiler('cpp')
cmake = import('cmake')
deps=[]

elfname = 'CortexCommand'
build_rpath = ''
link_args=[]
extra_args=[]
preprocessor_flags = []
override_options = []
suffix=''

if compiler.get_id() == 'clang'
  warning('LLVM clang is not directly supported due to missing features of the clang standard library. Remember to set cpp_std to gnu++20 or force libstdc++ otherwise (std::execution)')
endif

if host_machine.system() == 'darwin' and get_option('b_lto')
  error('LTO is currently unsupported on macos. Please disable with meson configure -Db_lto=false')
endif

if get_option('debug') and get_option('b_lto')
  warning('Link time optimization enabled in debug mode, debug symbols may not be fully available. Disable b_lto with "meson configure -Db_lto=false".')
endif

if compiler.get_argument_syntax()== 'gcc' # used for gcc compatible compilers
  # Build against system libraries on linux
  message('gcc detected')

  if host_machine.system() == 'linux'
    build_rpath = '$ORIGIN:$ORIGIN/../external/lib/linux/x86_64' # Set RUNPATH so that CCCP can find libfmod.so without needing to set LD_LIBRARY_PATH
  elif host_machine.system()=='darwin'
    build_rpath = '@executable_path/../external/lib/macos' # Add a new R_PATH CCCP can find libfmod.dylib on Darwin TODO: Confirm and validate this.
  endif

  #suffix = 'x86_64'
  if host_machine.system()=='linux'
    link_args += ['-Wl,--enable-new-dtags'] # Set RUNPATH instead of RPATH
  endif

  # Set preprocessor flags and cpp arguments according to build options
  buildtype_debug = get_option('debug')
  if buildtype_debug

    extra_args += ['-Wno-write-strings', '-Wno-unknown-pragmas', '-Wno-deprecated-declarations', '-Wno-misleading-indentation'] # Disable #pragma region complaints and luabind auto_ptr warning

    if get_option('sane_warnings')
      extra_args += ['-Wno-sign-compare', '-Wno-non-virtual-dtor', '-Wno-parentheses', '-Wno-overloaded-virtual', '-Wno-unused-variable', '-Wno-unused-function']
    endif

    debug_type = get_option('debug_type')
    if debug_type == 'release'
      preprocessor_flags += ['-DRELEASE_BUILD'] # enable minimal debug features
    elif debug_type == 'minimal'
      preprocessor_flags += ['-DMIN_DEBUG_BUILD', '-DDEBUGMODE'] # enable some debug features
    elif debug_type == 'full'
      preprocessor_flags += ['-DDEBUG_BUILD', '-DDEBUGMODE'] # enable all debug features; may slow down game
    endif
  else
    extra_args = ['-w'] # Disable all warnings for release builds
    preprocessor_flags += ['-DRELEASE_BUILD', '-DNDEBUG'] # disable all debug features
  endif
  if compiler.get_id() =='gcc' and compiler.version().version_compare('<9')
    link_args += ['-lstdc++fs']
  endif
elif compiler.get_argument_syntax()== 'msvc'
#TODO: add MSVC related arguments and stuff in here
  message('cl detected')
  elfname = 'Cortex Command'
  extra_args += ['-D_HAS_ITERATOR_DEBUGGING=0', '-D_HAS_AUTO_PTR_ETC=1', '-bigobj']
  add_global_arguments('-D_ITERATOR_DEBUG_LEVEL=0', language:'cpp')
  add_project_link_arguments(['winmm.lib', 'ws2_32.lib', 'dinput8.lib', 'ddraw.lib', 'dxguid.lib', 'dsound.lib', 'dbghelp.lib'], language:'cpp')
  if host_machine.cpu_family() == 'x86'
    elfname += ' x86'
  endif
  preprocessor_flags += [
    '-D_CRT_SECURE_NO_DEPRECATE',
    '-D_WINDOWS',
    '-DWIN32',
    '-DWIN32_LEAN_AND_MEAN',
    '-DNOMINMAX',
    '-DNOGDI',
    '-DNOKERNEL',
    '-DNONLS',
    '-DNOMEMMGR',
    '-DNOGDICAPMASKS',
    '-DNOVIRTUALKEYCODES',
    '-DNOWINMESSAGES',
    '-DNOWINSTYLES',
    '-DNOMETAFILE',
    '-DNOSCROLL',
    '-DNOTEXTMETRIC',
    '-DNOCOMM',
    '-DNOKANJI',
    '-DNOHELP',
    '-DNOPROFILER',
    '-DNODEFERWINDOWPOS',
    '-DNOMCX',
    '-DNOMENUS',
    '-DNOICONS',
    '-DNOKEYSTATES',
    '-DNOSYSCOMMANDS',
    '-DNORASTEROPS',
    '-DNOSHOWWINDOW',
    '-DOEMRESOURCE',
    '-DNOATOM',
    '-DNODRAWTEXT',
    '-DNOCOLOR',
    '-DNOCTLMGR',
    '-DNOMSG',
    '-DNOOPENFILE',
    '-DNOSERVICE',
    '-DNOSOUND',
    '-DNOWH',
    '-DNOWINOFFSETS'
  ]
  link_args+=['-ignore:4099', '-ignore:4217']
  buildtype_debug = get_option('debug')
  if buildtype_debug
    debug_type = get_option('debug_type')
    if debug_type == 'release'
      preprocessor_flags += ['-DDEBUG_RELEASE_BUILD'] # enable minimal debug features
    elif debug_type == 'minimal'
      preprocessor_flags += ['-DDEBUGMODE'] # enable some debug features
    elif debug_type == 'full'
      preprocessor_flags += ['-DDEBUG_BUILD', '-DDEBUGMODE'] # enable all debug features; may slow down game
    endif
  else
    preprocessor_flags += ['-DRELEASE_BUILD', '-DNDEBUG']
  endif
elif host_machine.system() == 'emscripten'
  # -------------------------------------------------------------------------
  # Emscripten / WebAssembly build
  # -------------------------------------------------------------------------
  message('Emscripten/WASM build detected')
  elfname = 'CortexCommand'

  # Emscripten link flags — mirror what emscripten/build.sh passes
  em_link_flags = [
    '-sUSE_WEBGL2=1',
    '-sFULL_ES3=1',
    '-sALLOW_MEMORY_GROWTH=1',
    '-sINITIAL_MEMORY=268435456',
    '-sMAXIMUM_MEMORY=2147483648',
    '-sEXPORTED_RUNTIME_METHODS=[\'FS\',\'callMain\',\'ccall\',\'cwrap\']',
    '-sEXPORTED_FUNCTIONS=[\'_main\']',
    '-sENVIRONMENT=web',
    '-sEXIT_RUNTIME=0',
    '-sASYNCIFY=1',
    '-sASYNCIFY_STACK_SIZE=65536',
    '--preload-file', 'Data/Base.rte@/Data/Base.rte',
    '--shell-file', meson.source_root() / 'emscripten/shell.html',
    '--pre-js',     meson.source_root() / 'emscripten/pre.js',
  ]
  link_args += em_link_flags

  extra_args += [
    '-w',               # Suppress warnings (too noisy during porting)
    '-DEMSCRIPTEN_PORT', # Our own guard macro for Emscripten-specific code paths
    '-DRELEASE_BUILD',
    '-DNDEBUG',
    # Raylib's rlgl.h/c uses OPENGL_33||OPENGL_ES2 guards throughout.
    # For WebGL2 (= OpenGL ES 3.0), define BOTH ES2 and ES3:
    # - ES3 controls shader version (#version 300 es in rlgl.c defaultVShaderCode)
    # - ES2 activates the 84+ rlgl.c code paths (rlCompileShader, rlglInit, etc.)
    # rlgl.h already does #define GRAPHICS_API_OPENGL_ES2 when ES3 is set (line 195).
    '-UGRAPHICS_API_OPENGL_33',
    '-DGRAPHICS_API_OPENGL_ES3',
    # Explicit ES2 here too (belt+suspenders, since rlgl.h sets it from ES3 anyway)
    '-DGRAPHICS_API_OPENGL_ES2',
  ]
  preprocessor_flags += ['-DRELEASE_BUILD', '-DNDEBUG', '-DEMSCRIPTEN_PORT',
                          # Raylib GL API flags — must be in preprocessor_flags to apply to C files
                          '-UGRAPHICS_API_OPENGL_33',
                          '-DGRAPHICS_API_OPENGL_ES3',
                          '-DGRAPHICS_API_OPENGL_ES2',
                         ]

  # Emscripten uses .html output suffix
  suffix = 'html'

else
  error('Using unknown compiler, please use gcc or msvc compatible compilers')
endif
add_project_arguments(preprocessor_flags, extra_args, language:'cpp')
add_project_arguments(preprocessor_flags, extra_args, language:'c')

# Additional Emscripten-only arguments that must apply to C files (e.g. rlgl.c)
if host_machine.system() == 'emscripten'
  add_global_arguments(
    '-UGRAPHICS_API_OPENGL_33',
    '-DGRAPHICS_API_OPENGL_ES3',
    '-DGRAPHICS_API_OPENGL_ES2',
    '-DEMSCRIPTEN_PORT',
    language: ['c', 'cpp']
  )
endif
add_project_link_arguments(link_args, language:'cpp')

#### Configuration ####
conf_data = configuration_data()
prefix = get_option('prefix')

conf_data.set_quoted('BASEDATAPATH', prefix/get_option('data_install_dir'))
conf_data.set('game_version', meson.project_version())

if suffix==''
  suffix=[]
  conf_data.set('EXENAME', elfname)
else
  conf_data.set('EXENAME', elfname + '.' + suffix)
endif
if get_option('install_runner')
  conf_data.set('EXERUNNER', 'CortexCommand')
  conf_data.set('EXEPATH', prefix/get_option('libdir')/'CortexCommand')
else
  conf_data.set('EXERUNNER', conf_data.get('EXENAME'))
  conf_data.set('EXEPATH', prefix/get_option('bindir'))
endif

if get_option('tracy_enable')
  add_global_arguments('-DTRACY_ENABLE', language:'cpp')
endif
if get_option('tracy_callstack') > 0
  add_global_arguments('-DTRACY_CALLSTACK='+get_option('tracy_callstack').to_string(), language:'cpp')
endif
if get_option('tracy_on_demand')
  add_global_arguments('-DTRACY_ON_DEMAND', language:'cpp')
endif
if get_option('tracy_debuginfod')
  deps += [dependency('libdebuginfod')]
  add_global_arguments('-DTRACY_DEBUGINFOD', language:'cpp')
endif

#### Platform Common External Libraries ####
if host_machine.system() in ['linux','darwin']
  deps += [
    #dependency(['sdl3']),
    #dependency('SDL3_image'),
    dependency('flac'),
    dependency('minizip'),
    dependency('threads'),
    dependency('liblz4'),
    dependency('libpng'),
    dependency('tbb'),
    dependency('gl')
  ]
  if host_machine.system()=='darwin'
    deps += dependency('appleframeworks', modules: ['Foundation'])
  endif
  bfd = compiler.find_library('bfd', required: false)
  if bfd.found()
    preprocessor_flags += '-DBACKWARD_HAS_BFD'
    deps += bfd
  endif
elif host_machine.system() == 'windows'
  zlib = subproject('zlib-ng-2.1.3')
  zlib_dep = dependency('zlib')
  minizip = subproject('minizip-ng-4.0.0')
  minizip_dep = dependency('minizip')
  libpng = subproject('libpng-1.6.40')
  libpng_dep = dependency('libpng')
  # Windows Libraries
  opengl = compiler.find_library('opengl32')
  deps += [zlib_dep, minizip_dep, libpng_dep, opengl]
elif host_machine.system() == 'emscripten'
  # Emscripten ports provide zlib and libpng as compiler/linker flags.
  # LZ4 is only used by NetworkClient/Server which are disabled for web.
  # minizip-ng is built from source below (after external subdir).
  em_zlib_dep = declare_dependency(
    compile_args: ['-sUSE_ZLIB=1'],
    link_args:    ['-sUSE_ZLIB=1'],
  )
  em_libpng_dep = declare_dependency(
    compile_args: ['-sUSE_LIBPNG=1'],
    link_args:    ['-sUSE_LIBPNG=1'],
  )
  meson.override_dependency('zlib',   em_zlib_dep)
  meson.override_dependency('libpng', em_libpng_dep)
  deps += [em_zlib_dep, em_libpng_dep]
endif


external_objects = []
external_libs = []
subdir('external')

boost_dep = declare_dependency(include_directories: include_directories('external/include/boost_1_75'))
meson.override_dependency('boost-175', boost_dep)

if host_machine.system() == 'emscripten'
  install_rpath = ''  # Emscripten has no rpath concept
  # -------------------------------------------------------------------------
  # Emscripten dependency configuration:
  #
  # - SDL2 is provided by Emscripten's own port (-sUSE_SDL=2 in link flags)
  # - Allegro 4 is replaced by AllegroCompat.h/cpp (our typed-array shim)
  # - FMOD is replaced by fmod_stub/ (no-op stub; real audio TBD)
  # - LuaJIT → compile without JIT, or replace with PUC-Lua (TBD Phase 3)
  # - RakNet → disabled; network multiplayer not available in web build
  # - Tracy → disabled (requires native OS profiling APIs)
  # - SDL3_image → not needed; we use libpng directly
  # -------------------------------------------------------------------------

  # SDL3 pre-built for Emscripten (built via emcmake separately in /tmp/sdl3-wasm).
  # The SDL3 source headers are in external/sources/SDL3-3.2.10/include.
  # The pre-built static library is at /tmp/sdl3-wasm/libSDL3.a.
  # To rebuild: bash emscripten/build_sdl3.sh
  sdl3_inc = include_directories('external/sources/SDL3-3.2.10/include')
  cc_native = meson.get_compiler('cpp')
  sdl3_lib = cc_native.find_library('SDL3', dirs: ['/tmp/sdl3-wasm'], required: true, static: true)
  sdl3_em_dep = declare_dependency(
    dependencies: [sdl3_lib],
    include_directories: [sdl3_inc],
  )
  deps += [sdl3_em_dep, boost_dep]

  # Lua 5.1.5 (PUC-Lua) — LuaJIT has no WASM/Emscripten backend.
  # PUC-Lua 5.1 is language-compatible with LuaJIT 2.1 in interpreter mode.
  lua_proj    = subproject('lua-5.1.5')
  luajit_dep  = dependency('luajit')   # lua-5.1.5 overrides 'luajit' dependency
  luajit_source_dir = lua_proj.get_variable('luajit_source_dir')
  external_inc_dirs += luajit_source_dir
  luabind_proj = subproject('luabind-0.7.1')
  luabind_dep  = dependency('luabind')
  deps += [luajit_dep, luabind_dep]

  # minizip-ng built from source (see external/sources/minizip-ng-4.0.0/meson.build)
  minizip_proj = subproject('minizip-ng-4.0.0')
  minizip_dep  = dependency('minizip')
  deps += [minizip_dep]

  # Allegro compat shim — included as source files (not a subproject dep)
  allegro_compat_inc = include_directories(
    'Source/System/allegro_web_compat'   # provides allegro.h redirect
  )
  external_inc_dirs += allegro_compat_inc

  # FMOD stub — provides fmod.hpp / fmod_errors.h no-op headers
  fmod_stub_inc = include_directories('Source/System/fmod_stub')
  external_inc_dirs += fmod_stub_inc

  # RakNet → stub dependency (empty; NetworkClient/Server compile conditionally)
  raknet_dep = declare_dependency()

  # Tracy → disabled. Tracy uses #ifndef TRACY_ENABLE to disable itself,
  # so we must UNDEFINE it (not define it as 0).
  tracy_inc = include_directories('external/sources/tracy/public')
  tracy_dep = declare_dependency(
    compile_args: ['-UTRACY_ENABLE'],  # undefine to disable all Tracy instrumentation
    include_directories: [tracy_inc],
  )
  external_inc_dirs += tracy_inc
  deps += [tracy_dep]

else
  # -------------------------------------------------------------------------
  # Native build — original dependency resolution
  # -------------------------------------------------------------------------
  sdl3_opt = cmake.subproject_options()
  sdl3_opt.add_cmake_defines({'CMAKE_POSITION_INDEPENDENT_CODE': get_option('b_staticpic'), 'BUILD_SHARED_LIBS':false, 'BUILD_STATIC_LIBS': true})
  sdl3_opt.append_compile_args('c','-DUSE_XINPUT2_KEYBOARD')
  if host_machine.system() == 'darwin'
    sdl3_dep = dependency('sdl3')
  else
    sdl3_proj = cmake.subproject('SDL3-3.2.10', options: sdl3_opt)
    sdl3_dep  = sdl3_proj.dependency('SDL3-static')
    meson.override_dependency('sdl3', sdl3_dep)
  endif

  sdl3_image_proj = subproject('SDL3_image-3.2.4')
  sdl3_image_dep  = sdl3_image_proj.get_variable('sdl3_image_dep')

  install_rpath = prefix/get_option('fmod_dir')
  # Also define for Emscripten branch (unused, but referenced in executable())
  if not is_variable('install_rpath')
    install_rpath = ''
  endif
  tracy_proj = subproject('tracy')
  tracy_dep  = dependency('tracy')

  allegro_proj = subproject('allegro 4.4.3.1-custom')
  allegro_dep  = dependency('allegro')
  loadpng_dep  = dependency('loadpng')

  luajit_proj = subproject('LuaJIT-2.1')
  luajit_dep  = dependency('luajit')
  external_inc_dirs += luajit_proj.get_variable('luajit_source_dir')

  luabind_proj = subproject('luabind-0.7.1')
  luabind_dep  = dependency('luabind')

  raknet_proj = subproject('RakNet')
  raknet_dep  = dependency('RakNet')

  deps += [sdl3_dep, sdl3_image_dep, allegro_dep, luajit_dep, loadpng_dep, raknet_dep, boost_dep, tracy_dep]
endif

#### Sources Setup ####
source_inc_dirs = []
subdir('Source')
subdir('Resources')


#### Target Definitions ####
cccpelf = executable(
  elfname, ['Source/Main.cpp', sources], include_directories:[source_inc_dirs, external_inc_dirs], cpp_pch:pch, # Sources options
  link_with:[external_libs, source_libs], objects:[external_objects], dependencies:deps,                                # Link options
  cpp_args:[extra_args, preprocessor_flags], link_args:[link_args], build_rpath:build_rpath,               # Compiler setup
  name_suffix:suffix,   # Executable name options
  build_by_default:true, # Meson options
  install: host_machine.system() != 'emscripten',
  install_rpath: host_machine.system() != 'emscripten' ? install_rpath : '',
  install_dir: get_option('install_runner') ? get_option('libdir')/'CortexCommand' : get_option('bindir'),
  win_subsystem:'windows', # Windows mark as GUI app
)


#### Installing #####
base_exclude_files = [
  'Base.rte/Settings.ini',
  'Launch Actor Editor.bat',
  'Launch Area Editor.bat',
  'Launch Assembly Editor.bat',
  'Launch Gib Editor.bat',
  'Launch Scene Editor.bat',
  'LogConsole.txt',
  'LogLoading.txt',
  'LogLoadingWarning.txt',
  'Start Dedicated Server.bat'
]

if get_option('install_data')
install_subdir('Data',
               exclude_directories:['.git', '.github'],
               exclude_files:base_exclude_files,
               install_dir:get_option('data_install_dir')/'Data',
               strip_directory:true)
endif
