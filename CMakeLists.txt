CMAKE_MINIMUM_REQUIRED(VERSION 3.31.0)
project(triumsh C)

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

if(MSVC)
	target_compile_options(trish PRIVATE /W4)
else()
	target_compile_options(trish PRIVATE -Wall -Wextra -Wpedantic)
endif()
