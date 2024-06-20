#include <zephyr/sys/util.h>

#include "button.h"

int btn_flag = 0;

static struct gpio_callback button0_cb_data;
static struct gpio_callback button1_cb_data;
static struct gpio_callback button2_cb_data;
static struct gpio_callback button3_cb_data;

static struct k_work_q blink_work_queue;
static K_THREAD_STACK_DEFINE(blink_work_stack, 1024);

static struct k_work blink_led0_work;
static struct k_work blink_led1_work;
static struct k_work blink_led2_work;
static struct k_work blink_led3_work;

static bool led0_blinking = false;
static bool led1_blinking = false;
static bool led2_blinking = false;
static bool led3_blinking = false;

void blink_led(struct k_work *work)
{
    const struct gpio_dt_spec *led;
    bool *blinking;
    uint32_t duration_ms = 3000; 

    if (work == &blink_led0_work) {
        led = &led0;
        blinking = &led0_blinking;
    } else if (work == &blink_led1_work) {
        led = &led1;
        blinking = &led1_blinking;
    } else if (work == &blink_led2_work) {
        led = &led2;
        blinking = &led2_blinking;
    } else if (work == &blink_led3_work) {
        led = &led3;
        blinking = &led3_blinking;
    } else {
        return;
    }

    int ret;
    for (uint32_t i = 0; i < (duration_ms / 500) && *blinking; i++) {
        ret = gpio_pin_toggle_dt(led);
        if (ret < 0) {
            return;
        }
        k_msleep(500); // Delay for 500 milliseconds (0.5 seconds)
    }
    *blinking = false;
    gpio_pin_set_dt(led, 0); // Turn off the LED after the duration
}

void button0_callback(const struct device *dev, struct gpio_callback *cb, uint32_t pins)
{
    // printk("Button 0 pressed\n");
    btn_flag = 0;
    if (!led0_blinking) {
        led0_blinking = true;
        k_work_init(&blink_led0_work, blink_led);
        k_work_submit_to_queue(&blink_work_queue, &blink_led0_work);
    }
}

void button1_callback(const struct device *dev, struct gpio_callback *cb, uint32_t pins)
{
    // printk("Button 1 pressed\n");
    btn_flag = 1; 
    if (!led1_blinking) {
        led1_blinking = true;
        k_work_init(&blink_led1_work, blink_led);
        k_work_submit_to_queue(&blink_work_queue, &blink_led1_work);
    }
}

void button2_callback(const struct device *dev, struct gpio_callback *cb, uint32_t pins)
{
    // printk("Button 2 pressed\n");
    btn_flag = 2;
    if (!led2_blinking) {
        led2_blinking = true;
        k_work_init(&blink_led2_work, blink_led);
        k_work_submit_to_queue(&blink_work_queue, &blink_led2_work);
    }
}

void button3_callback(const struct device *dev, struct gpio_callback *cb, uint32_t pins)
{
    // printk("Button 3 pressed\n");
    btn_flag = 3;
    if (!led3_blinking) {
        led3_blinking = true;
        k_work_init(&blink_led3_work, blink_led);
        k_work_submit_to_queue(&blink_work_queue, &blink_led3_work);
    }
    __asm volatile(
        "LDR R0, =0xE000ED0C\t\n"
        "LDR R1, [R0]\t\n"
        "ORR R1, R1, #(1 <<2)\t\n"
        "LDR R2 , =0x0000FFFF\t\n"
        "AND R1, R1, R2\t\n"
        "LDR R2, =0x05FA0000\t\n"
        "ORR R1, R1, R2\t\n"
        "STR R1, [R0]\t\n"
        "B .\t\n"
    );
}

int gpio_init(void)
{
    int err = GPIO_FAIL;

    // Initialize the work queue
    k_work_queue_start(&blink_work_queue, blink_work_stack,
                       K_THREAD_STACK_SIZEOF(blink_work_stack),
                       CONFIG_MAIN_THREAD_PRIORITY,
                       NULL);

    // Set button0 interrupt
    // printk("Setting button0 interrupt\n");

    err = gpio_is_ready_dt(&button0);
    if (!err) {
        printk("Error gpio_is_ready_dt led0 pin %d\n", err);
        return GPIO_FAIL;
    }

    err = gpio_pin_configure_dt(&button0, GPIO_INPUT | GPIO_PULL_UP);
    if (err < 0) {
        printk("Error configuring button0 pin %d\n", err);
        return GPIO_FAIL;
    }

    err = gpio_pin_interrupt_configure_dt(&button0, GPIO_INT_EDGE_TO_ACTIVE);
    if (err != 0) {
        printk("Error configuring interrupt on button0 pin %d\n", err);
        return GPIO_FAIL;
    }
    gpio_init_callback(&button0_cb_data, button0_callback, BIT(button0.pin));
    gpio_add_callback(button0.port, &button0_cb_data);

    // Set button1 interrupt
    // printk("Setting button1 interrupt\n");
    err = gpio_is_ready_dt(&button1);
    if (!err) {
        printk("Error gpio_is_ready_dt led1 pin %d\n", err);
        return GPIO_FAIL;
    }

    err = gpio_pin_configure_dt(&button1, GPIO_INPUT | GPIO_PULL_UP);
    if (err < 0) {
        printk("Error configuring button1 pin %d\n", err);
        return GPIO_FAIL;
    }

    err = gpio_pin_interrupt_configure_dt(&button1, GPIO_INT_EDGE_TO_ACTIVE);
    if (err != 0) {
        printk("Error configuring interrupt on button1 pin %d\n", err);
        return GPIO_FAIL;
    }
    gpio_init_callback(&button1_cb_data, button1_callback, BIT(button1.pin));
    gpio_add_callback(button1.port, &button1_cb_data);

    // Set button2 interrupt
    // printk("Setting button2 interrupt\n");
    err = gpio_is_ready_dt(&button2);
    if (!err) {
        printk("Error gpio_is_ready_dt led2 pin %d\n", err);
        return GPIO_FAIL;
    }

    err = gpio_pin_configure_dt(&button2, GPIO_INPUT | GPIO_PULL_UP);
    if (err < 0) {
        printk("Error configuring button2 pin %d\n", err);
        return GPIO_FAIL;
    }

    err = gpio_pin_interrupt_configure_dt(&button2, GPIO_INT_EDGE_TO_ACTIVE);
    if (err != 0) {
        printk("Error configuring interrupt on button2 pin %d\n", err);
        return GPIO_FAIL;
    }
    gpio_init_callback(&button2_cb_data, button2_callback, BIT(button2.pin));
    gpio_add_callback(button2.port, &button2_cb_data);

    // Set button3 interrupt
    // printk("Setting button3 interrupt\n");
    err = gpio_is_ready_dt(&button3);
    if (!err) {
        printk("Error gpio_is_ready_dt led3 pin %d\n", err);
        return GPIO_FAIL;
    }

    err = gpio_pin_configure_dt(&button3, GPIO_INPUT | GPIO_PULL_UP);
    if (err < 0) {
        printk("Error configuring button3 pin %d\n", err);
        return GPIO_FAIL;
    }

    err = gpio_pin_interrupt_configure_dt(&button3, GPIO_INT_EDGE_TO_ACTIVE);
    if (err != 0) {
        printk("Error configuring interrupt on button3 pin %d\n", err);
        return GPIO_FAIL;
    }
    gpio_init_callback(&button3_cb_data, button3_callback, BIT(button3.pin));
    gpio_add_callback(button3.port, &button3_cb_data);

    // Set led0
    err = gpio_is_ready_dt(&led0);
    if (!err) {
        printk("Error gpio_is_ready_dt led0 pin %d\n", err);
        return GPIO_FAIL;
    }

    err = gpio_pin_configure_dt(&led0, GPIO_OUTPUT_INACTIVE);
    if (err < 0) {
        printk("Error %d: failed to configure LED0 pin\n", err);
        return GPIO_FAIL;
    }

    // Set led1
    err = gpio_is_ready_dt(&led1);
    if (!err) {
        printk("Error gpio_is_ready_dt led1 pin %d\n", err);
        return GPIO_FAIL;
    }

    err = gpio_pin_configure_dt(&led1, GPIO_OUTPUT_INACTIVE);
    if (err < 0) {
        printk("Error %d: failed to configure LED1 pin\n", err);
        return GPIO_FAIL;
    }

    // Set led2
    err = gpio_is_ready_dt(&led2);
    if (!err) {
        printk("Error gpio_is_ready_dt led2 pin %d\n", err);
        return GPIO_FAIL;
    }

    err = gpio_pin_configure_dt(&led2, GPIO_OUTPUT_INACTIVE);
    if (err < 0) {
        printk("Error %d: failed to configure LED2 pin\n", err);
        return GPIO_FAIL;
    }

    // Set led3
    err = gpio_is_ready_dt(&led3);
    if (!err) {
        printk("Error gpio_is_ready_dt led3 pin %d\n", err);
        return GPIO_FAIL;
    }

    err = gpio_pin_configure_dt(&led3, GPIO_OUTPUT_INACTIVE);
    if (err < 0) {
        printk("Error %d: failed to configure LED3 pin\n", err);
        return GPIO_FAIL;
    }

    return GPIO_OK;
}