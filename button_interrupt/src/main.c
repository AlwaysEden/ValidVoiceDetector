#include <zephyr/kernel.h>

#include "gpios.h"

extern int btn_flag;

int main(void)
{
	printk("buttoninterrupt_example\n");

	int err = GPIO_FAIL;
	err = gpio_init();
	if (err != GPIO_OK)
	{
		printk("Error gpio_init %d\n", err);
		return 0;
	}

	

	return 0;
}