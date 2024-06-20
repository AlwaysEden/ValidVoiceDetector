/*
 * Copyright (c) 2016 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/adc.h>
#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>
#include <zephyr/sys/util.h>
#include <zephyr/logging/log.h>

#include "batterydisplay.h"
#include "pir.h"
#include "value.h"
#include "led.h"
#include "co2.h"
#include "button.h"

//******* Button *******
extern int btn_flag;
//***************

/* ****** PIR Setting ****** */
//PIR Define
/* 1000 msec = 1 sec */
#define SLEEP_TIME_MS   1000
#define PIR_NODE DT_ALIAS(pir)
LOG_MODULE_REGISTER(pir);
#if DT_NODE_HAS_STATUS(PIR_NODE, okay)
static const struct gpio_dt_spec pir = GPIO_DT_SPEC_GET(PIR_NODE, gpios);
static struct gpio_callback pir_cb_data;
volatile bool motion_detected = false;
int val;
void pir_detected(const struct device *dev, struct gpio_callback *cb, uint32_t pins)
{
    motion_detected = true;
    LOG_INF("Motion detected!");
}
#else
#error "PIR sensor is not defined in the device tree"
#endif
/* ************************* */

/* ****** Sound Sensor Setting ****** */
// Sound Sensor Define
K_THREAD_STACK_DEFINE(sound_thread_stack, 1024);
k_tid_t sound_thread_id;

#define MAX_SENSORVALUE 1023
#define MIN_SENSORVALUE 15
#define SENSOR_INVALID_VALUE 65500
#if !DT_NODE_EXISTS(DT_PATH(zephyr_user)) || \
	!DT_NODE_HAS_PROP(DT_PATH(zephyr_user), io_channels)
#error "No suitable devicetree overlay specified"
#endif

#define DT_SPEC_AND_COMMA(node_id, prop, idx) \
	ADC_DT_SPEC_GET_BY_IDX(node_id, idx),

/* Data of ADC io-channels specified in devicetree. */
static const struct adc_dt_spec adc_channels[] = {
	DT_FOREACH_PROP_ELEM(DT_PATH(zephyr_user), io_channels,
			     DT_SPEC_AND_COMMA)
};

long map(long x, long in_min, long in_max, long out_min, long out_max)
{
        return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

long minimize64(long x){
	return x*64/1023;
}
long minimize15(long x){
	return x*15/1023;
}
/* *********************************** */

/* ****** Co2 Setting (for thread) ****** */
#define CO2_THREAD_STACK_SIZE 1024
#define CO2_THREAD_PRIORITY 5

K_THREAD_STACK_DEFINE(co2_thread_stack, CO2_THREAD_STACK_SIZE);
struct k_thread co2_thread_data;
k_tid_t co2_thread_id;
extern int average_ppm;


void soundSensor_Start(void *arg1, void *arg2, void *arg3){
	uint16_t buf;
	uint32_t sound_value = 0;
	int err;
	struct adc_sequence sequence = {
		.buffer = &buf,
		/* buffer size in bytes, not number of samples */
		.buffer_size = sizeof(buf),
								// .oversampling = 0,
								// .calibrate = 0,
	};
	uint32_t current_sound_level = 0;
	int64_t start_time, current_time;
	start_time = k_uptime_get();
	while (1) {
		(void)adc_sequence_init_dt(&adc_channels[0], &sequence);
		err = adc_read(adc_channels[0].dev, &sequence);
		if (err < 0) {
			printk("Could not read (%d)\n", err);
			k_sleep(K_MSEC(50));
			continue;
		}

		sound_value = (int32_t)buf;
		if(sound_value >= SENSOR_INVALID_VALUE){
			k_sleep(K_MSEC(50));
			continue;
		}
		// if(btn_flag == 1){
			// current_sound_level = minimize64(sound_value);
		// }else{
			current_sound_level = map(sound_value, 0, MAX_SENSORVALUE, 0, MIN_SENSORVALUE);
			// current_sound_level = minimize15(sound_value);


		// }

		printk("sound_value : %d current: %d\n", sound_value,current_sound_level);

		if(btn_flag==0 &&current_sound_level <= 4){
			k_sleep(K_MSEC(100));
			continue;
		}else {
			led_on_idx(btn_flag, current_sound_level);
		}
		if(average_ppm == -1){
				// printk("Sound Sensor Break\n");
				led_off_all();
				break;
		}
		k_sleep(K_MSEC(100));
	}
	k_thread_abort(k_current_get());
}

void pir_init(void)
{
    int rc;

    if (!device_is_ready(pir.port)) {
        LOG_ERR("PIR sensor GPIO device not ready");
        return;
    }

    rc = gpio_pin_configure_dt(&pir, GPIO_INPUT);
    if (rc < 0) {
        LOG_ERR("Failed to configure PIR sensor GPIO: %d", rc);
        return;
    }

    rc = gpio_pin_interrupt_configure_dt(&pir, GPIO_INT_EDGE_TO_ACTIVE);
    if (rc != 0) {
        LOG_ERR("Error %d: failed to configure interrupt on %s pin %d", rc, pir.port->name, pir.pin);
        return;
    }

    gpio_init_callback(&pir_cb_data, pir_detected, BIT(pir.pin));
    gpio_add_callback(pir.port, &pir_cb_data);
    LOG_INF("PIR sensor initialized and callback set");
}

void asm_pir_init(){
	__asm volatile(
			"PUSH {LR}\t\n"
			"BL pir_init\t\n"
			"POP {LR}\t\n"
			"BX LR\t\n"
	);
}

void battery_led_on(){
	for (uint8_t level = 0; level <= 7; level++) {
    display_level(level);
  }
}

int main(void)
{
	int ret;
	int err;
	err = gpio_init();
	if (err != GPIO_OK)
	{
		printk("Error gpio_init %d\n", err);
		return 0;
	}
	display_clear();

	/* ****** Sound Sensor Setting ****** */
	/* Configure channels individually prior to sampling. */
	for (size_t i = 0U; i < ARRAY_SIZE(adc_channels); i++) {
		if (!adc_is_ready_dt(&adc_channels[i])) {
			printk("ADC controller device %s not ready\n", adc_channels[i].dev->name);
			return 0;
		}

		err = adc_channel_setup_dt(&adc_channels[i]);
		if (err < 0) {
			printk("Could not setup channel #%d (%d)\n", i, err);
			return 0;
		}
	}
	/* *********** Sound Sensor ************/
	asm_pir_init();
	set_brightness(BRIGHTNESS_LEVEL1);
	co2_init();
	struct k_thread sound_thread_data;

	// printk("Done with Configure\n");

	while (1) {
		// printk("Running\n");
		if (motion_detected) {
			battery_led_on();
			sound_thread_id = k_thread_create(&sound_thread_data, sound_thread_stack,
                              K_THREAD_STACK_SIZEOF(sound_thread_stack),
                              soundSensor_Start, NULL, NULL, NULL,
                              5, 0, K_NO_WAIT);
			co2_thread_id = k_thread_create(&co2_thread_data, co2_thread_stack,
                    K_THREAD_STACK_SIZEOF(co2_thread_stack),
                    (k_thread_entry_t) co2_thread,
                    NULL, NULL, NULL,
                    CO2_THREAD_PRIORITY, 0, K_NO_WAIT);
			k_thread_start(sound_thread_id);
			k_thread_start(co2_thread_id);
			// soundSensor_Start();
			// printk("PIR sensor value: %d\n", val);

			k_thread_join(&sound_thread_data, K_FOREVER);
			k_thread_join(&co2_thread_data, K_FOREVER);
			motion_detected = false;
			display_clear();
			k_sleep(K_MSEC(100));
		}
	}

	return 0;
}