sources += files(
'Base64/base64.cpp',
'MicroPather/micropather.cpp',
'Semver200/Semver200_comparator.cpp',
'Semver200/Semver200_modifier.cpp',
'Semver200/Semver200_parser.cpp',
'StandardIncludes.cpp',
'AllegroTools.cpp',
'Atom.cpp',
'ContentFile.cpp',
'Controller.cpp',
'GenericSavedData.cpp',
'Writer.cpp',
'Box.cpp',
'Entity.cpp',
'Vector.cpp',
'Reader.cpp',
'Color.cpp',
'InputScheme.cpp',
'RTETools.cpp',
'System.cpp',
'InputMapping.cpp',
'PathFinder.cpp',
'DataModule.cpp',
'Timer.cpp',
'RTEError.cpp',
'Matrix.cpp',
'Serializable.cpp',
'PieQuadrant.cpp',
'SpatialPartitionGrid.cpp',
'WebPlatform.cpp',
)

if host_machine.system() == 'emscripten'
  sources += files(
    'AllegroCompat.cpp',
  )
  # Provide the allegro_web_compat directory early in include search
  source_inc_dirs += include_directories('allegro_web_compat')
endif