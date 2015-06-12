SET(IC_DBUS "ic-dbus")

FIND_PROGRAM(GDBUS_CODEGEN NAMES gdbus-codegen)

ADD_CUSTOM_COMMAND(
	OUTPUT dbus
	COMMAND ${GDBUS_CODEGEN} --generate-c-code ${IC_DBUS}
			--interface-prefix org.tizen.iotcon.
			--c-namespace ic
			${IC_DBUS}.xml
	DEPENDS ${IC_DBUS}.xml)

ADD_CUSTOM_TARGET(GENERATED_DBUS_CODE DEPENDS dbus)
