#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include "batterydisplay.h"
#include "pir.h"
#include "value.h"

LOG_MODULE_REGISTER(main);

#define DELAY_TIME K_MSEC(100)

int main(void)
{
    int ret = batterydisplay_init();
    if (ret != DK_OK) {
        LOG_ERR("Battery display initialization failed");
        return DK_ERR;
    }

    pir_init();

    set_brightness(BRIGHTNESS_LEVEL1);
    display_clear();

    while (1) {
        if (motion_detected) {
            // Turn on the battery display and show levels from 0 to 7
            for (uint8_t level = 0; level <= 7; level++) {
                display_level(level);
                k_sleep(K_MSEC(500)); // Display each level for 500 milliseconds
            }
            motion_detected = false; // Reset the motion detected flag
        } else {
            // Turn off the battery display
            display_clear();
        }

        k_sleep(DELAY_TIME);
    }

    return DK_OK;
}
