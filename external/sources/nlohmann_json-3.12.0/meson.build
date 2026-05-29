project('nlohmann_json', 'cpp', version: '3.12.0')

nlohmann_json_dep = declare_dependency(
  include_directories: include_directories('.'),
)

meson.override_dependency('nlohmann_json', nlohmann_json_dep)
