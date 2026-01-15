CMAKE_MINIMUM_REQUIRED(VERSION 4.0.0)
project(triumsh C)

set(SOURCES src/main.c)

if(WIN32)
	list(APPEND SOURCES src/exec_windows.c)
endif()

add_executable(trish ${SOURCES})
