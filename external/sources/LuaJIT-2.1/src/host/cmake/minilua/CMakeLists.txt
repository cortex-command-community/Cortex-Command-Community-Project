cmake_minimum_required(VERSION 3.0...4.0)

project(minilua)

set(CMAKE_POSITION_INDEPENDENT_CODE OFF)

find_library(LIBM_LIBRARIES NAMES m)

add_executable(minilua ../../minilua.c)
if (LIBM_LIBRARIES)
  target_link_libraries(minilua ${LIBM_LIBRARIES})
endif()

if (MSVC)
  set_target_properties(minilua PROPERTIES RUNTIME_OUTPUT_DIRECTORY $<1:${CMAKE_CURRENT_BINARY_DIR}>)
endif()
