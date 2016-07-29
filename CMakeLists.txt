# libclipboard cmake configuration file

cmake_minimum_required(VERSION 2.8)
project(libclipboard)

# Set compiler flags
if (CMAKE_COMPILER_IS_GNUCC)
    set(GCC_COMPILE_FLAGS "-std=c99 -Wall -pedantic -g")

    execute_process(COMMAND $ {CMAKE_C_COMPILER} -dumpversion
                    OUTPUT_VARIABLE GCC_VERSION)
    if (GCC_VERSION VERSION_GREATER 4.9 OR GCC_VERSION VERSION_EQUAL 4.9)
        set(GCC_COMPILE_FLAGS "${GCC_COMPILE_FLAGS} -fdiagnostics-color=auto")
    endif()
    set(CMAKE_C_FLAGS  "${CMAKE_C_FLAGS} ${GCC_COMPILE_FLAGS}")
endif()

# Dependencies
if (UNIX AND NOT APPLE)
    find_package (X11 REQUIRED)
    find_package (Threads REQUIRED)

    include_directories(${X11_INCLUDE_DIR})
    link_libraries(${X11_LIBRARIES})

    set (CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${CMAKE_THREAD_LIBS_INIT}")
    set (CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} ${CMAKE_THREAD_LIBS_INIT}")
    set (CMAKE_MODULE_LINKER_FLAGS "${CMAKE_MODULE_LINKER_FLAGS} ${CMAKE_THREAD_LIBS_INIT}")
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
    src/clipboard_common.c
)

# Set the output folders
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# Make the library
add_library(clipboard ${HEADERS} ${SOURCE})

# Testing mode?
set (test OFF CACHE BOOL "Enable libclipboard unit testing (default:off)")
if (test)
    # gtest does not play well with pthreads and mingw
    if (MINGW)
        # CMake is case sensitive for 'ON CACHE BOOL'...
        set(gtest_disable_pthreads ON CACHE BOOL "Disable gtest pthreads (default:on)")
    endif()
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/third_party/googletest)
    enable_testing()
    include_directories(${gtest_SOURCE_DIR}/include ${gtest_SOURCE_DIR})
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/test)
endif()

# Build sample executables?
set (samples OFF CACHE BOOL "Build libclipboard sample executables (default:off)")
if (samples)
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/samples)
endif()
