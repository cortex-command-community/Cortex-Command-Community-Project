# Files that are not directly tied to loaders
src = files('IMG.c', 'IMG_WIC.c', 'IMG_stb.c')

if use_imageio_backend
  add_languages(
    'objc',
    native: false,
  )
  src += files('IMG_ImageIO.m')
  deps += dependency(
    'appleframeworks',
    modules: ['ApplicationServices'],
  )

  imageio_provided_formats = ['png', 'jpg', 'tif', 'gif', 'tga']
  if not get_option('force_builtin_bmp')
    imageio_provided_formats += 'bmp'
  endif

  foreach format_name : imageio_provided_formats
    if format_name not in backend_provided_loaders
      feature_args += '-D@0@_USES_IMAGEIO'.format(format_name.to_upper())
    endif
  endforeach

  backend_provided_loaders += imageio_provided_formats
endif

if not use_imageio_backend and host_machine.system() == 'darwin'
  feature_args += '-DSDL_IMAGE_USE_COMMON_BACKEND'
endif

if use_wic_backend
  feature_args += '-DSDL_IMAGE_USE_WIC_BACKEND'
  deps += cc.find_library('windowscodecs')
  backend_provided_loaders += ['png', 'jpg', 'tif']
endif

summary(
  {
    'stb_image': use_stbimage_backend,
    'Image I/O': use_imageio_backend,
    'Windows Imaging Component': use_wic_backend,
  },
  section: 'Backends',
)

# Loaders that don't depend on anything
self_contained_loaders = [
  'bmp',
  'gif',
  'lbm',
  'pcx',
  'pnm',
  'qoi',
  'svg',
  'tga',
  'xcf',
  'xpm',
  'xv',
]

foreach format_name : self_contained_loaders
  src += files('IMG_@0@.c'.format(format_name))
  if get_option(format_name).allowed()
    feature_args += '-DLOAD_@0@'.format(format_name.to_upper())
    summary(
      format_name,
      format_name in backend_provided_loaders ? 'handled by backend' : 'built-in loader',
      section: 'Formats',
    )
  else
    summary(
      format_name,
      'disabled',
      section: 'Formats',
    )
  endif
endforeach

# Loaders that depend on external libraries
external_dep_loaders = {
  'avif': ['libavif'],
  'jpg': ['libjpeg'],
  'jxl': ['libjxl'],
  'png': ['libpng'],
  'tif': ['libtiff-4'],
  'webp': ['libwebp', 'libwebpdemux'],
}

foreach format_name, library_names : external_dep_loaders
  src += files('IMG_@0@.c'.format(format_name))
  if format_name in backend_provided_loaders
    if get_option(format_name).allowed()
      feature_args += '-DLOAD_@0@'.format(format_name.to_upper())
      summary(
        format_name,
        'handled by backend',
        section: 'Formats',
      )
    else
      summary(
        format_name,
        'disabled',
        section: 'Formats',
      )
    endif
  else
    summary_name = format_name
    summary_value = 'handled by ' + ', '.join(library_names)

    foreach lib_name : library_names
      loader_library = dependency(
        lib_name,
        required: get_option(format_name),
      )
      if loader_library.found()
        deps += loader_library
        feature_args += '-DLOAD_@0@'.format(format_name.to_upper())
      else
        summary_value = 'disabled'
      endif
    endforeach
    summary(
      summary_name,
      summary_value,
      section: 'Formats',
    )
  endif
endforeach

jpg_save_support = get_option('jpg_save').allowed()
feature_args += '-DSDL_IMAGE_SAVE_JPG=@0@'.format(jpg_save_support.to_int())

png_save_support = get_option('png_save').allowed()
feature_args += '-DSDL_IMAGE_SAVE_PNG=@0@'.format(png_save_support.to_int())

avif_save_support = get_option('avif_save').allowed()
feature_args += '-DSDL_IMAGE_SAVE_AVIF=@0@'.format(png_save_support.to_int())

summary(
  {
    'jpg': jpg_save_support,
    'png': png_save_support,
    'avif': avif_save_support,
  },
  section: 'Image saving support',
)
