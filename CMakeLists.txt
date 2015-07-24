CMAKE_MINIMUM_REQUIRED(VERSION 2.6)
PROJECT(iotcon C)

INCLUDE(FindPkgConfig)

SET(EXTRA_CFLAGS "-Wall -Werror-implicit-function-declaration -fvisibility=hidden")
SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${EXTRA_CFLAGS}")

SET(CLIENT ${PROJECT_NAME})
SET(DAEMON "${PROJECT_NAME}-daemon")
SET(DBUS_INTERFACE "org.tizen.${PROJECT_NAME}.dbus")

SET(OCLIB "oc")
SET(OCLOGGER "oc_logger")
SET(OCLOGGER_CORE "oc_logger_core")
SET(CA "connectivity_abstraction")
SET(OCTBSTACK "octbstack")

ADD_SUBDIRECTORY(common)
ADD_SUBDIRECTORY(lib)
ADD_SUBDIRECTORY(daemon)
ADD_SUBDIRECTORY(test)
