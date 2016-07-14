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

# Source files
set(HEADERS
    libclipboard.h
)

set(SOURCE
    clipboard_win32.c
)

add_library(clipboard ${HEADERS} ${SOURCE})

add_executable(lctest test/clipboard_test.c)
target_link_libraries(lctest clipboard)