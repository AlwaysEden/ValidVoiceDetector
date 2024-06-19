#ifndef PIR_H
#define PIR_H

#include <zephyr/drivers/gpio.h>

extern volatile bool motion_detected;

void pir_init(void);
void pir_detected(const struct device *dev, struct gpio_callback *cb, uint32_t pins);

#endif // PIR_H
