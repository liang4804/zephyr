/* main.c - OpenThread */

/*
 * Copyright (c) 2023 Telink
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/kernel.h>

#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(ot_main, LOG_LEVEL_DBG);

#include <openthread/thread.h>
#include <zephyr/net/net_ip.h>

static void ot_satate_changed(uint32_t flags, void *context)
{
	otInstance *instance  = (otInstance *)context;

	if (flags & OT_CHANGED_THREAD_ROLE) {
		switch (otThreadGetDeviceRole(instance)) {
		case OT_DEVICE_ROLE_CHILD:
			LOG_INF("OT child");
			break;
		case OT_DEVICE_ROLE_ROUTER:
			LOG_INF("OT router");
			break;
		case OT_DEVICE_ROLE_LEADER:
			LOG_INF("OT leader");
			break;
		case OT_DEVICE_ROLE_DISABLED:
			LOG_INF("OT disabled");
			break;
		case OT_DEVICE_ROLE_DETACHED:
			LOG_INF("OT detached");
			break;
		default:
			LOG_INF("OT unknown");
			break;
		}
	}
}

void main(void)
{
	LOG_INF("***** OpenThread SED joiner on Zephyr *****");
	LOG_INF("OT channel     %u", CONFIG_OPENTHREAD_CHANNEL);
	LOG_INF("OT pan id      %0x04x", CONFIG_OPENTHREAD_PANID);
	LOG_INF("OT pan ext id  %s", CONFIG_OPENTHREAD_XPANID);
	LOG_INF("OT network key %s", CONFIG_OPENTHREAD_NETWORKKEY);

	otInstance *instance = otInstanceInitSingle();
	otError err;

	err = otSetStateChangedCallback(instance, ot_satate_changed, instance);
	LOG_INF("otSetStateChangedCallback %s\n", otThreadErrorToString(err));

#ifdef CONFIG_OPENTHREAD_MANUAL_START

	err = otLinkSetChannel(instance, CONFIG_OPENTHREAD_CHANNEL);
	LOG_INF("otLinkSetChannel %s\n", otThreadErrorToString(err));

	err = otLinkSetPanId(instance, CONFIG_OPENTHREAD_PANID);
	LOG_INF("otLinkSetPanId %s\n", otThreadErrorToString(err));

	otExtendedPanId ext_pan_id;
	if (net_bytes_from_str(ext_pan_id.m8, sizeof(ext_pan_id.m8), (char *)CONFIG_OPENTHREAD_XPANID)) {
		LOG_INF("parse CONFIG_OPENTHREAD_XPANID failed\n");
	} else {
		err = otThreadSetExtendedPanId(instance, &ext_pan_id);
		LOG_INF("otThreadSetExtendedPanId %s\n", otThreadErrorToString(err));
	}

	otNetworkKey network_key;
	if (net_bytes_from_str(network_key.m8, sizeof(network_key.m8), (char *)CONFIG_OPENTHREAD_NETWORKKEY)) {
		LOG_INF("parse CONFIG_OPENTHREAD_NETWORKKEY failed\n");
	} else {
		err = otThreadSetNetworkKey(instance, &network_key);
		LOG_INF("otThreadSetNetworkKey %s\n", otThreadErrorToString(err));
	}

	otLinkModeConfig link_mode = {
#ifdef CONFIG_OPENTHREAD_MTD
#ifdef CONFIG_OPENTHREAD_MTD_SED
		.mRxOnWhenIdle = 0,
#else
		.mRxOnWhenIdle = 1,
#endif /* CONFIG_OPENTHREAD_MTD_SED */
		.mDeviceType = 0,
#else
		.mRxOnWhenIdle = 1,
		.mDeviceType = 1,
#endif /* CONFIG_OPENTHREAD_MTD */
		.mNetworkData = 0
	};
	err = otThreadSetLinkMode(instance, link_mode);
	LOG_INF("otThreadSetLinkMode %s\n", otThreadErrorToString(err));

#if defined(CONFIG_OPENTHREAD_MTD) && defined(CONFIG_OPENTHREAD_MTD_SED)
	err = otLinkSetPollPeriod(instance, CONFIG_OPENTHREAD_POLL_PERIOD);
	LOG_INF("otLinkSetPollPeriod %s\n", otThreadErrorToString(err));
#endif /* CONFIG_OPENTHREAD_MTD && CONFIG_OPENTHREAD_MTD_SED */

	err = otIp6SetEnabled(instance, true);
	LOG_INF("otIp6SetEnabled %s\n", otThreadErrorToString(err));

	for (;;) {
		err = otThreadSetEnabled(instance, true);
		LOG_INF("otThreadSetEnabled %s to True\n", otThreadErrorToString(err));
		k_msleep(5000);

		err = otPlatRadioSleep(instance);
		LOG_INF("otPlatRadioSleep %s\n", otThreadErrorToString(err));
		err = otThreadSetEnabled(instance, false);
		LOG_INF("otThreadSetEnabled %s to False\n", otThreadErrorToString(err));
		k_msleep(5000);
	}

#endif /* CONFIG_OPENTHREAD_MANUAL_START */
}
