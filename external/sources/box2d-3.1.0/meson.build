project('box2d', 'c', version: '3.1.0', default_options: ['c_std=c17'])

box2d_inc = include_directories('include')
box2d_src_inc = include_directories('src')

box2d_sources = files(
  'src/aabb.c',
  'src/arena_allocator.c',
  'src/array.c',
  'src/bitset.c',
  'src/body.c',
  'src/broad_phase.c',
  'src/constraint_graph.c',
  'src/contact.c',
  'src/contact_solver.c',
  'src/core.c',
  'src/distance.c',
  'src/distance_joint.c',
  'src/dynamic_tree.c',
  'src/geometry.c',
  'src/hull.c',
  'src/id_pool.c',
  'src/island.c',
  'src/joint.c',
  'src/manifold.c',
  'src/math_functions.c',
  'src/motor_joint.c',
  'src/mouse_joint.c',
  'src/mover.c',
  'src/prismatic_joint.c',
  'src/revolute_joint.c',
  'src/sensor.c',
  'src/shape.c',
  'src/solver.c',
  'src/solver_set.c',
  'src/table.c',
  'src/timer.c',
  'src/types.c',
  'src/weld_joint.c',
  'src/wheel_joint.c',
  'src/world.c',
)

box2d_lib = static_library('box2d', box2d_sources,
  include_directories: [box2d_inc, box2d_src_inc],
  c_args: ['-DNDEBUG', '-DBOX2D_DISABLE_SIMD', '-D_POSIX_C_SOURCE=199309L', '-w'],
)

box2d_dep = declare_dependency(
  link_with: box2d_lib,
  include_directories: box2d_inc,
)

meson.override_dependency('box2d', box2d_dep)
