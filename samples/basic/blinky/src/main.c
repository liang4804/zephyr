/*
 * Copyright (c) 2016 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/zephyr.h>

/* CPU based sleep - sleep modes are inactive but delay */
static void sleep_cpu_load_ms(uint32_t ms)
{
	volatile uint64_t counter = (uint64_t)ms * 3432;

	while (counter) {
		counter--;
	}
}

void main(void)
{
	printk("main started\n");
	sleep_cpu_load_ms(5000);
	printk("guard expiration\n");

	uint32_t counter = 0;

	while (1) {
		printk("run %u\n", counter++);
		k_msleep(1000);
	}
}
