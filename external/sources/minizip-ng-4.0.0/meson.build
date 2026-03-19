project('minizip', ['c', 'cpp'])

libname = 'minizip'
lib_include_dir = include_directories('include', 'src')

cc = meson.get_compiler('c')
cxx = meson.get_compiler('cpp')

if host_machine.system() == 'emscripten'
  # Build minizip-ng from source for Emscripten (no prebuilt binary available).
  # Use the POSIX subset — Windows and Apple platform files are excluded.
  minizip_sources = files(
    'src/mz_compat.c',
    'src/mz_crypt.c',
    'src/mz_os.c',
    'src/mz_os_posix.c',
    'src/mz_strm.c',
    'src/mz_strm_buf.c',
    'src/mz_strm_mem.c',
    'src/mz_strm_os_posix.c',
    'src/mz_strm_pkcrypt.c',
    'src/mz_strm_split.c',
    'src/mz_strm_zlib.c',
    'src/mz_zip.c',
    'src/mz_zip_rw.c',
  )
  minizip_args = [
    '-DMZ_ZIP_NO_ENCRYPTION',  # Disable AES (no wzaes.c)
    '-DMZ_ZIP_NO_BZIP2',       # Disable bzip2
    '-DMZ_ZIP_NO_LZMA',        # Disable lzma
    '-DMZ_COMPAT_VERSION=110', # Enable minizip 1.x compat layer
    '-DZLIB_COMPAT',           # Use standard zlib.h (not zlib-ng.h) with Emscripten port
    '-sUSE_ZLIB=1',            # Use Emscripten's zlib port
  ]
  zlib_dep = dependency('zlib')
  libminizip = static_library('minizip',
    minizip_sources,
    include_directories: lib_include_dir,
    c_args: minizip_args,
    dependencies: [zlib_dep],
  )
  lib_dep = declare_dependency(
    link_with: libminizip,
    include_directories: lib_include_dir,
    compile_args: minizip_args,
    dependencies: [zlib_dep],
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