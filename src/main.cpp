/*
 * Copyright (c) 2012-2014 Wind River Systems, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/kernel.h>
#include <zephyr/usb/usb_device.h>
#include <zephyr/device.h>
#include <zephyr/drivers/sensor.h>
#include "ei.h"
#include "imu.h"
#include "forwarder.h"

int main(void)
{
  usb_enable(NULL);
	imu_init();
  k_forwarder();
	return 0;
}