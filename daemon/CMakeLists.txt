INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/common)
INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/lib/include)

FILE(GLOB DAEMON_SRCS *.c ${CMAKE_SOURCE_DIR}/common/*.c)
SET(DAEMON_SRCS ${DAEMON_SRCS} ${CMAKE_SOURCE_DIR}/common/ic-dbus.c)

SET_SOURCE_FILES_PROPERTIES(${CMAKE_SOURCE_DIR}/common/ic-dbus.c
	PROPERTIES GENERATED TRUE)

SET(PKG_MODULES gio-2.0 dlog gio-unix-2.0 capi-system-system-settings capi-system-info capi-network-bluetooth uuid)

IF(TZ_VER_3)
	SET(PKG_MODULES ${PKG_MODULES} cynara-client cynara-session cynara-creds-gdbus)
ENDIF(TZ_VER_3)

pkg_check_modules(daemon_pkgs REQUIRED ${PKG_MODULES})
pkg_check_modules(iotivity_pkgs REQUIRED iotivity)

FOREACH(flag ${daemon_pkgs_CFLAGS_OTHER})
	IF(${flag} MATCHES "\\-D+")
		ADD_DEFINITIONS(${flag})
	ENDIF(${flag} MATCHES "\\-D+")
ENDFOREACH(flag)

FOREACH(flag ${iotivity_pkgs_CFLAGS_OTHER})
	IF(${flag} MATCHES "\\-D+")
		ADD_DEFINITIONS(${flag})
	ENDIF(${flag} MATCHES "\\-D+")
ENDFOREACH(flag)

INCLUDE_DIRECTORIES(${daemon_pkgs_INCLUDE_DIRS})
INCLUDE_DIRECTORIES(${iotivity_pkgs_INCLUDE_DIRS})
LINK_DIRECTORIES(${daemon_pkgs_LIBRARY_DIRS})
LINK_DIRECTORIES(${iotivity_pkgs_LIBRARY_DIRS})
LINK_DIRECTORIES(${CMAKE_SOURCE_DIR}/daemon/iotivity-bt)

SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fPIE")
SET(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,--hash-style=both -pie")
ADD_DEFINITIONS("-DIOTCON_DBUS_INTERFACE=\"${DBUS_INTERFACE}\"")

ADD_EXECUTABLE(${DAEMON} ${DAEMON_SRCS})
ADD_DEPENDENCIES(${DAEMON} GENERATED_DBUS_CODE)

SET(IOTIVITY_LIBS
		liboctbstack.a
		libocsrm.a
		libc_common.a
		libroutingmanager.a
		libconnectivity_abstraction.a
		libcoap.a
		liboc_logger_core.a
		m
		pthread
)

TARGET_LINK_LIBRARIES(${DAEMON} ${IOTIVITY_LIBS} ${daemon_pkgs_LIBRARIES})

INSTALL(TARGETS ${DAEMON} DESTINATION ${BIN_INSTALL_DIR})
