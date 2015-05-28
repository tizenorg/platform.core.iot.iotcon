#include <glib.h>
#include <iotcon.h>
#include "test-log.h"

static void _get_device_info(iotcon_device_info_h info, void *user_data)
{
	INFO("device_name : %s", iotcon_device_info_get_device_name(info));
	INFO("host_name : %s", iotcon_device_info_get_host_name(info));
	INFO("device_uuid : %s", iotcon_device_info_get_device_uuid(info));
	INFO("content_type : %s", iotcon_device_info_get_content_type(info));
	INFO("version : %s", iotcon_device_info_get_version(info));
	INFO("manufacturer_name : %s", iotcon_device_info_get_manufacturer_name(info));
	INFO("manufacturer_url : %s", iotcon_device_info_get_manufacturer_url(info));
	INFO("model_number : %s", iotcon_device_info_get_model_number(info));
	INFO("date_of_manufacture : %s", iotcon_device_info_get_date_of_manufacture(info));
	INFO("platform_version : %s", iotcon_device_info_get_platform_version(info));
	INFO("firmware_version : %s", iotcon_device_info_get_firmware_version(info));
	INFO("support_url : %s", iotcon_device_info_get_support_url(info));
}

int main()
{
	GMainLoop *loop;

	loop = g_main_loop_new(NULL, FALSE);

	iotcon_initialize(IOTCON_ALL_INTERFACES, IOTCON_RANDOM_PORT);

	iotcon_get_device_info(IOTCON_MULTICAST_ADDRESS, _get_device_info, NULL);

	g_main_loop_run(loop);

	g_main_loop_unref(loop);

	return 0;
}
