project('zlib', ['c', 'cpp'])

libname = 'zlibstatic'
lib_include_dir = include_directories('include')

cc = meson.get_compiler('c')
cxx = meson.get_compiler('cpp')

if host_machine.system() == 'emscripten'
  # Emscripten provides zlib as a port via -sUSE_ZLIB=1.
  lib_dep = declare_dependency(
    include_directories: lib_include_dir,
    compile_args: ['-sUSE_ZLIB=1'],
    link_args:    ['-sUSE_ZLIB=1'],
  )
elif host_machine.cpu_family() == 'x86_64'
  lib = cc.find_library(libname, dirs:  meson.current_source_dir()/'_Bin', required: false)
  if not lib.found()
    lib = cxx.find_library(libname, dirs:  meson.current_source_dir()/'_Bin')
  endif
  lib_dep = declare_dependency(dependencies: lib, include_directories: lib_include_dir)
else
  lib = cc.find_library(libname+'-32', dirs:  meson.current_source_dir()/'_Bin', required: false)
  if not lib.found()
    lib = cxx.find_library(libname+'-32', dirs: meson.current_source_dir()/'_Bin')
  endif
  lib_dep = declare_dependency(dependencies: lib, include_directories: lib_include_dir)
endif

meson.override_dependency(meson.project_name(), lib_dep)
