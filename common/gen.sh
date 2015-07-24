#!/bin/sh

DIR_PATH=`dirname $0`

gdbus-codegen --generate-c-code "$DIR_PATH"/ic-dbus\
	--interface-prefix org.tizen.iotcon.\
	--c-namespace ic\
	"$DIR_PATH"/ic-dbus.xml

