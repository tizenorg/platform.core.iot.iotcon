CMAKE_MINIMUM_REQUIRED(VERSION 2.6)
PROJECT(iotcon)

INCLUDE(FindPkgConfig)

SET(EXTRA_CFLAGS "-Wall -Werror-implicit-function-declaration -fvisibility=hidden -fPIE")
SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${EXTRA_CFLAGS}")

SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x -fvisibility=hidden")
SET(CMAKE_EXE_LINKER_FLAGS "-Wl,--hash-style=both -pie")

SET(CLIENT ${PROJECT_NAME})
SET(DAEMON "${PROJECT_NAME}-daemon")
SET(DBUS_INTERFACE "org.tizen.${PROJECT_NAME}")

ADD_SUBDIRECTORY(lib)
ADD_SUBDIRECTORY(daemon)
ADD_SUBDIRECTORY(test)
