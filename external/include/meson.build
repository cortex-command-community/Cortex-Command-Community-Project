if host_machine.system() == 'emscripten'
  # Emscripten build: exclude fmod (replaced by stub) and LZ4 (unused: networking disabled)
  external_inc_dirs = [include_directories('boost_1_75',
                                           'glad-2.0.0-beta',
                                           'glm-0.9.9.8',
                                           'thread-pool-3.5.0/include',
                                           'hopscotch-map-2.3.1/include',)]
else
  external_inc_dirs = [include_directories('boost_1_75',
                                           'fmod-2.2.13',
                                           'fmod-2.2.13',
                                           'glad-2.0.0-beta',
                                           'glm-0.9.9.8',
                                           'thread-pool-3.5.0/include',
                                           'hopscotch-map-2.3.1/include',)]
  if host_machine.system() in ['linux', 'darwin']
  endif
  if host_machine.system() == 'windows'
    external_inc_dirs += include_directories('LZ4-1.9.3/LZ4',)
  endif
endif
