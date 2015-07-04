#include <glib.h>
#include <iotcon.h>
#include "test.h"

static void _get_platform_info(iotcon_platform_info_s info, void *user_data)
{
	INFO("platform_id : %s", info.platform_id);
	INFO("manuf_name : %s", info.manuf_name);
	INFO("manuf_url : %s", info.manuf_url);
	INFO("model_number : %s", info.model_number);
	INFO("date_of_manufacture : %s", info.date_of_manufacture);
	INFO("platform_ver : %s", info.platform_ver);
	INFO("os_ver : %s", info.os_ver);
	INFO("hardware_ver : %s", info.hardware_ver);
	INFO("firmware_ver : %s", info.firmware_ver);
	INFO("support_url : %s", info.support_url);
	INFO("system_time : %s", info.system_time);
}

int main()
{
	GMainLoop *loop;

	loop = g_main_loop_new(NULL, FALSE);

	/* iotcon initialize */
	iotcon_initialize();

	iotcon_get_platform_info(IOTCON_MULTICAST_ADDRESS, _get_platform_info, NULL);

	g_main_loop_run(loop);

	g_main_loop_unref(loop);

	/* iotcon deinitialize */
	iotcon_deinitialize();

	return 0;
}
