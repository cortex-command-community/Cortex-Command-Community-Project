project('sdl3_image', 'c', version: '3.2.4')

cc = meson.get_compiler('c')

sdl3_dep = dependency('sdl3', version: '>=3.2.4')
deps = [sdl3_dep]
inc = include_directories('include')

feature_args = []
extra_library_args = []

if host_machine.system() == 'windows' and get_option('default_library') != 'static'
  extra_library_args += '-DDLL_EXPORT'
endif

# These are enabled by default it seems.
#img_args = ['-DWANT_LIBPNG']
#if jpg_dep.found()
#  img_args += '-DWANT_JPEGLIB'
#endif

# Backends provide support for certain formats, allowing not to depend on external library or use potentially better optimized algorithm
backend_provided_loaders = []

use_imageio_backend = get_option('use_imageio').require(
  host_machine.system() == 'darwin',
  error_message: 'Image I/O backend is exclusive to MacOS',
).allowed()
use_wic_backend = get_option('use_wic').require(
  host_machine.system() == 'windows',
  error_message: 'WIC backend is exclusive to Windows',
).allowed()

native_backend_present = use_imageio_backend or use_wic_backend

# In meson 1.1.0+ this can be done using .disable_auto_if(native_backend_present).enabled()
use_stbimage_backend = native_backend_present ? get_option('use_stbimage').enabled() : get_option(
  'use_stbimage',
).allowed()
if use_stbimage_backend
  feature_args += '-DUSE_STBIMAGE'
  backend_provided_loaders += ['png', 'jpg']
endif

subdir('src')

full_library_args = [feature_args, extra_library_args]

img_lib = library(
  'sdl3_image',
  src,
  c_args: full_library_args,
  objc_args: full_library_args,
  include_directories: inc,
  gnu_symbol_visibility: 'hidden',
  dependencies: deps,
)

sdl3_image_dep = declare_dependency(
  include_directories: inc,
  link_with: img_lib,
)
