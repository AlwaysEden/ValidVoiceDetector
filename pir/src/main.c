/*
 * Copyright (c) 2016 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>

/* 1000 msec = 1 sec */
#define SLEEP_TIME_MS   1000
#define PIR_NODE DT_ALIAS(pir)

#if DT_NODE_HAS_STATUS(PIR_NODE, okay)
static const struct gpio_dt_spec pir = GPIO_DT_SPEC_GET(PIR_NODE, gpios);
static struct gpio_callback pir_callback;
int val;
void pir_detect(const struct device *dev, struct gpio_callback *cb,
		    uint32_t pins)
{
  printk("PIR sensor value: %d\n", val);
}

#else
#error "PIR sensor is not defined in the device tree"
#endif

int main(void)
{
    int ret;

    if (!device_is_ready(pir.port)) {
        printk("PIR sensor GPIO device not ready\n");
        return 0;
    }
		printk("Done with ready check\n");
    ret = gpio_pin_configure_dt(&pir, GPIO_INPUT);
    if (ret < 0) {
        printk("Failed to configure PIR sensor GPIO: %d\n", ret);
        return 0;
    }
		ret = gpio_pin_interrupt_configure_dt(&pir,GPIO_INT_EDGE_TO_ACTIVE);
		if (ret != 0) {
			printk("Error %d: failed to configure interrupt on %s pin %d\n", ret, pir.port->name, pir.pin);
			return 0;
		}
		// gpio_init_callback(&pir_callback, pir_detect, BIT(pir.pin));
		// gpio_add_callback(pir.port, &pir_callback);
		printk("Set up button at %s pin %d\n", pir.port->name, pir.pin);

		printk("Done with Configure\n");
    while (1) {
        val = gpio_pin_get_dt(&pir);
        if (val < 0) {
            printk("Failed to read PIR sensor GPIO: %d\n", val);
        } else {
            printk("PIR sensor value: %d\n", val);
        }
				printk("Done with get\n");

        k_sleep(K_MSEC(100));
    }
		return 0;
}