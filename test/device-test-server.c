#include <glib.h>
#include <iotcon.h>
#include "test.h"

int main()
{
	int ret;
	GMainLoop *loop;

	iotcon_device_info_s device_info = {0};

	device_info.name = "device_name";
	device_info.host_name = "host_name";
	device_info.uuid = "device_uuid";
	device_info.content_type = "content_type";
	device_info.version = "version";
	device_info.manuf_name = "manuf_name";
	device_info.manuf_url = "manuf_url";
	device_info.model_number = "model_number";
	device_info.date_of_manufacture = "date_of_manufacture";
	device_info.platform_ver = "platform_version";
	device_info.firmware_ver = "firmware_version";
	device_info.support_url = "support_url";

	loop = g_main_loop_new(NULL, FALSE);

	/* iotcon initialize */
	iotcon_initialize();

	ret = iotcon_register_device_info(device_info);
	if (IOTCON_ERROR_NONE != ret) {
		ERR("iotcon_register_device_info() Fail(%d)", ret);
		return -1;
	}

	g_main_loop_run(loop);

	g_main_loop_unref(loop);

	/* iotcon deinitialize */
	iotcon_deinitialize();

	return 0;
}




