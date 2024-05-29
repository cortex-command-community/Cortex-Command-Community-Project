external_inc_dirs = [include_directories('boost_1_75',
                                         'fmod-2.2.13',
                                         'fmod-2.2.13',
                                         'glad-2.0.0-beta',
                                         'glm-0.9.9.8',
                                         'thread-pool-3.5.0/include',
                                         'hopscotch-map-2.3.1/include',)]
if host_machine.system() in ['linux', 'darwin']
  luajit_proj = subproject('LuaJIT-2.1', default_options: ['header_only=true'])
  external_inc_dirs += luajit_proj.get_variable('luajit_source_dir')
endif
if host_machine.system() == 'windows'
  external_inc_dirs += include_directories('LZ4-1.9.3/LZ4',)
endif
