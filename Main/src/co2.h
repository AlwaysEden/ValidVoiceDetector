#ifndef CO2_H
#define CO2_H

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/logging/log.h>
#include <zephyr/drivers/uart.h>

#define RECEIVE_TIMEOUT 1000
#define MSG_SIZE 9
#define CO2_MULTIPLIER 256

void co2_init(void);
void co2_read(void);
void co2_thread(void);

#endif // CO2_H
