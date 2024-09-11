
if use_pch
  pch = meson.current_source_dir()/'System/StandardIncludes.h'
else
  pch = ''
endif
sources = []
source_libs = []
source_inc_dirs += [include_directories(
  '.',
  'Activities',
  'Entities',
  'GUI',
  'GUI/Wrappers',
  'Lua',
  'Managers',
  'Menus',
  'System')]

subdir('Activities')
subdir('Entities')
subdir('GUI')
subdir('Lua')
subdir('Managers')
subdir('Menus')
subdir('System')
