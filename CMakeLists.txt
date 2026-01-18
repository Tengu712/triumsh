CMAKE_MINIMUM_REQUIRED(VERSION 3.31.0)
project(triumsh C)

if(MSVC)
	set(COMMON_COMPILE_OPTIONS /W4)
else()
	set(COMMON_COMPILE_OPTIONS -Wall -Wextra -Wpedantic)
endif()

set(SOURCES
	src/cmdline.c
	src/exec.c
	src/main.c
)

if(WIN32)
	list(APPEND SOURCES src/exec_windows.c)
else()
	list(APPEND SOURCES src/exec_posix.c)
endif()

add_executable(trish ${SOURCES})
target_compile_options(trish PRIVATE ${COMMON_COMPILE_OPTIONS})

if(BUILD_TEST)
	add_executable(test_cursor src/cursor_test.c)
	target_compile_options(test_cursor PRIVATE ${COMMON_COMPILE_OPTIONS})
endif()
