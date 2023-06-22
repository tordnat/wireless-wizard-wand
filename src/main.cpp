/*
 * Copyright (c) 2012-2014 Wind River Systems, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/device.h>
#include <zephyr/drivers/sensor.h>
#include <zephyr/usb/usb_device.h>
#include "ei.h"
#include "imu.h"
#include "forwarder.h"
#include "zigbee.h"

LOG_MODULE_REGISTER(main, LOG_LEVEL_INF);

int main(void)
{
  usb_enable(NULL);
  LOG_INF("Wireless Wizard Wand started on %s", CONFIG_BOARD);
	imu_init();
  k_zigbee();
  k_ei();
	return 0;
}