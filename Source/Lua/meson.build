lua_sources = files(
'LuaAdapters.cpp',
'LuaBindingsActivities.cpp',
'LuaBindingsEntities.cpp',
'LuaBindingsGUI.cpp',
'LuaBindingsInput.cpp',
'LuaBindingsManagers.cpp',
'LuaBindingsMisc.cpp',
'LuaBindingsSystem.cpp',
'LuaBindingsPrimitives.cpp',
'LuaBridgeRegistry.cpp',
'LuaObjectWrapper.cpp'
)

luabindings_cpp_args = []
defaults_override = []
if compiler.get_argument_syntax() == 'gcc'
elif compiler.get_argument_syntax() == 'msvc'
  luabindings_cpp_args += ['-bigobj']
else
error('Unsupported compiler')
endif

lua_bindings = static_library('lua_bindings', lua_sources, dependencies:deps, include_directories: [source_inc_dirs, external_inc_dirs], cpp_args: luabindings_cpp_args, cpp_pch:pch, override_options: defaults_override)
source_libs += lua_bindings
