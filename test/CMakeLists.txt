# libclipboard testing suite

# Source files
set (SOURCE
	 test_basics.cpp
)
set (HEADERS
	
)

# Add the target
add_executable (run-tests ${HEADERS} ${SOURCE})

# Link it to gtest
target_link_libraries(run-tests LINK_PRIVATE gtest gtest_main)

# Link it with libclipboard
target_link_libraries (run-tests LINK_PUBLIC clipboard)

# For `make test`
add_test(NAME libclipboard-testing
		 COMMAND ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/run-tests --gtest_color=yes
WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})

# Sigh
add_custom_target(check COMMAND ${CMAKE_CTEST_COMMAND}
                  ARGS "-V"
                  DEPENDS run-tests)