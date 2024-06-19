#include "pir.h"
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(pir);

#define PIR_NODE DT_ALIAS(pir)

static const struct gpio_dt_spec pir = GPIO_DT_SPEC_GET(PIR_NODE, gpios);
static struct gpio_callback pir_cb_data;
volatile bool motion_detected = false;

void pir_detected(const struct device *dev, struct gpio_callback *cb, uint32_t pins)
{
    motion_detected = true;
    LOG_INF("Motion detected!");
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
