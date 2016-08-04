# libclipboard cmake configuration file

cmake_minimum_required(VERSION 2.8)
project(libclipboard)

# Set compiler flags
if (CMAKE_COMPILER_IS_GNUCC OR APPLE)
    set(GCC_COMPILE_FLAGS "-std=c99 -Wall -pedantic -g")

    execute_process(COMMAND $ {CMAKE_C_COMPILER} -dumpversion
                    OUTPUT_VARIABLE GCC_VERSION)
    if (GCC_VERSION VERSION_GREATER 4.9 OR GCC_VERSION VERSION_EQUAL 4.9)
        set(GCC_COMPILE_FLAGS "${GCC_COMPILE_FLAGS} -fdiagnostics-color=auto")
    endif()
    set(CMAKE_C_FLAGS  "${CMAKE_C_FLAGS} ${GCC_COMPILE_FLAGS}")
endif()

if (CMAKE_COMPILER_IS_GNUCXX OR APPLE)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -Wall -pedantic -g")
endif()


# Dependencies
if (APPLE)
    set_source_files_properties(src/clipboard_cocoa.c PROPERTIES COMPILE_FLAGS "-x objective-c")
    link_libraries("-framework Cocoa")
elseif (UNIX)
    include (FindPkgConfig)
    pkg_check_modules(X11 xcb REQUIRED)
    find_package (Threads REQUIRED)

    include_directories(${X11_INCLUDE_DIR})
    link_libraries(${X11_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT})
endif()

# Include directories
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

# Source files
set(HEADERS
    include/libclipboard.h
)

set(SOURCE
    src/clipboard_win32.c
    src/clipboard_x11.c
    src/clipboard_cocoa.c
    src/clipboard_common.c
)

# Set the output folders
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# Make the library
option (BUILD_SHARED_LIBS "Build shared libraries instead of static libraries" OFF)
add_library(clipboard ${HEADERS} ${SOURCE})

# Testing mode?
option (test "Enable libclipboard unit testing (default:off)" OFF)
if (test)
    # gtest does not play well with pthreads and mingw
    if (MINGW)
        # CMake is case sensitive for 'ON CACHE BOOL'...
        set(gtest_disable_pthreads ON CACHE BOOL "Disable gtest pthreads (default:on)")
    elseif (MSVC)
        # Runtime library conflicts
        set(gtest_force_shared_crt ON CACHE BOOL "Always use dynamic runtime library (default:on)" )
    endif()
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/third_party/googletest EXCLUDE_FROM_ALL)
    enable_testing()
    include_directories(${gtest_SOURCE_DIR}/include ${gtest_SOURCE_DIR})
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/test)
endif()

# Build sample executables?
option (samples "Build libclipboard sample executables (default:off)" OFF)
if (samples)
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/samples)
endif()

# Install options
install(TARGETS clipboard
        RUNTIME DESTINATION bin
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib)
install(FILES ${HEADERS} DESTINATION include)

# uninstall target
configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake_uninstall.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake"
    IMMEDIATE @ONLY)

add_custom_target(uninstall
    COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake)