#include <glib.h>
#include <iotcon.h>
#include "test-log.h"

int main()
{
	int ret;
	GMainLoop *loop;

	char *device_name = "device_name";
	char *host_name = "host_name";
	char *device_uuid = "device_uuid";
	char *content_type = "content_type";
	char *version = "version";
	char *manufacturer_name = "manuf_name";
	char *manufacturer_url = "manuf_url";
	char *model_number = "model_number";
	char *date_of_manufacture = "date_of_manufacture";
	char *platform_version = "platform_version";
	char *firmware_version = "firmware_version";
	char *support_url = "support_url";

	loop = g_main_loop_new(NULL, FALSE);

	iotcon_initialize(IOTCON_ALL_INTERFACES, IOTCON_RANDOM_PORT);

	ret = iotcon_register_device_info(device_name,
			host_name,
			device_uuid,
			content_type,
			version,
			manufacturer_name,
			manufacturer_url,
			model_number,
			date_of_manufacture,
			platform_version,
			firmware_version,
			support_url);
	if (IOTCON_ERROR_NONE != ret) {
		ERR("iotcon_register_device_info() Fail(%d)", ret);
		return -1;
	}

	g_main_loop_run(loop);

	g_main_loop_unref(loop);

	return 0;
}




