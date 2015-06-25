#include <glib.h>
#include <iotcon.h>
#include "test.h"

static void _get_device_info(iotcon_device_info_s info, void *user_data)
{
	INFO("name : %s", info.name);
	INFO("host_name : %s", info.host_name);
	INFO("uuid : %s", info.uuid);
	INFO("content_type : %s", info.content_type);
	INFO("version : %s", info.version);
	INFO("manuf_name : %s", info.manuf_name);
	INFO("manuf_url : %s", info.manuf_url);
	INFO("model_number : %s", info.model_number);
	INFO("date_of_manufacture : %s", info.date_of_manufacture);
	INFO("platform_ver : %s", info.platform_ver);
	INFO("firmware_ver : %s", info.firmware_ver);
	INFO("support_url : %s", info.support_url);
}

int main()
{
	GMainLoop *loop;

	loop = g_main_loop_new(NULL, FALSE);

	/* iotcon initialize */
	iotcon_initialize();

	iotcon_get_device_info(IOTCON_MULTICAST_ADDRESS, _get_device_info, NULL);

	g_main_loop_run(loop);

	g_main_loop_unref(loop);

	return 0;
}
