set (TEST_SOURCES tests.c)
if (WIN32)
    set(LIB_EXT ".dll.a")
else()
    set(LIB_EXT ".a")
endif()
  
function(append_library LIB_PATH LIB_NAME)
    if(EXISTS "${CMAKE_BINARY_DIR}/Criterion/subprojects/${LIB_PATH}/lib${LIB_NAME}${LIB_EXT}")
        list(APPEND CRITERION_LIBRARIES "${CMAKE_BINARY_DIR}/Criterion/subprojects/${LIB_PATH}/lib${LIB_NAME}${LIB_EXT}")
    else()
        execute_process(
            COMMAND find "${CMAKE_BINARY_DIR}/Criterion" -name "lib${LIB_NAME}${LIB_EXT}"
            OUTPUT_VARIABLE FOUND_LIB
            OUTPUT_STRIP_TRAILING_WHITESPACE
        )
        if(FOUND_LIB)
            message(STATUS "Found lib${LIB_NAME}${LIB_EXT} at: ${FOUND_LIB}")
            list(APPEND CRITERION_LIBRARIES "${FOUND_LIB}")
        else()
            message(WARNING "lib${LIB_NAME}${LIB_EXT} not found in Criterion subprojects.")
        endif()
    endif()
    set(CRITERION_LIBRARIES "${CRITERION_LIBRARIES}" PARENT_SCOPE)
endfunction()

# Usage examples
append_library("nanomsg" "nanomsg")
append_library("nanopb" "protobuf_nanopb_static")
append_library("boxfort/src" "boxfort")
append_library("libgit2" "git2")

function(copy_dll DLL_FULL_PATH DLL_NAME)
    if(WIN32)
        # Construct the full path to the DLL
        set(DLL_PATH "${CMAKE_BINARY_DIR}/${DLL_FULL_PATH}/${DLL_NAME}.dll")

        # Add a custom command to copy the DLL if it exists
        add_custom_command(TARGET unit_tests POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E cmake_echo_color --green --bold "Attempting to copy ${DLL_NAME}.dll"
            COMMAND bash -c "if [[ -f '${DLL_PATH}' ]]; then ${CMAKE_COMMAND} -E copy_if_different '${DLL_PATH}' '$<TARGET_FILE_DIR:unit_tests>'; fi"
        )
    endif()
endfunction()




include_directories(SYSTEM "${CRITERION_INCLUDE_DIRS}")

# Add a debug message for test sources
message(STATUS "Adding executable for unit_tests with sources: ${TEST_SOURCES}")

add_executable (unit_tests "${TEST_SOURCES}")
# Add a debug message
message(STATUS "Linking unit_tests with libraries: fmem, ${CRITERION_LIBRARIES}")

target_link_libraries (unit_tests fmem "${CRITERION_LIBRARIES}")

copy_dll("Criterion/subprojects/nanomsg/nanomsg/" "nanomsg")
copy_dll("Criterion/subprojects/nanopb/nanopb/" "protobuf_nanopb_static")
copy_dll("Criterion/subprojects/boxfort/src/" "boxfort")
copy_dll("Criterion/subprojects/libgit2/" "libgit2")
copy_dll("" "libfmem")  # For libfmem.dll in the build directory


add_test (unit_tests unit_tests)
