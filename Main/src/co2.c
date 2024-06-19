#include "co2.h"
#include <string.h>
#include <zephyr/drivers/uart.h>

LOG_MODULE_REGISTER(co2, CONFIG_LOG_DEFAULT_LEVEL);

static const struct device *const uart_serial = DEVICE_DT_GET(DT_N_ALIAS_myserial);

static char rx_buf[MSG_SIZE];
static int rx_buf_pos = 0;
int current_ppm;
int average_ppm = 5000;
int average_on = 0;

enum uart_fsm_state_code {
    UART_FSM_IDLE,
    UART_FSM_HEADER,
    UART_FSM_DATA,
    UART_FSM_CHECKSUM,
    UART_FSM_END,
};

uint8_t fromHexadecimalToDecimal(uint8_t hexadecimalValue) {
	uint8_t decimalValue = 0;
	decimalValue += hexadecimalValue / 10 * 16;
	decimalValue += hexadecimalValue % 10;
	return decimalValue;
}

static uint8_t uart_fsm_state = UART_FSM_IDLE;

uint8_t check_uart_fsm(uint8_t reset, uint8_t read_data) {
    if (reset) {
        uart_fsm_state = UART_FSM_IDLE;
    } else {
        switch (uart_fsm_state) {
            case UART_FSM_IDLE:
                if (read_data == 0xFF) {
                    uart_fsm_state = UART_FSM_HEADER;
                }
                break;
            case UART_FSM_HEADER:
                if (read_data == 0x86) {
                    uart_fsm_state = UART_FSM_DATA;
                } else {
                    uart_fsm_state = UART_FSM_IDLE;
                }
                break;
            case UART_FSM_DATA:
                if (rx_buf_pos == MSG_SIZE - 2) {
                    uart_fsm_state = UART_FSM_CHECKSUM;
                }
                break;
            case UART_FSM_CHECKSUM:
                if (rx_buf_pos == MSG_SIZE - 1) {
                    uart_fsm_state = UART_FSM_END;
                }
                break;
            case UART_FSM_END:
                uart_fsm_state = UART_FSM_IDLE;
                break;
            default:
                uart_fsm_state = UART_FSM_IDLE;
                break;
        }
    }
    return uart_fsm_state;
}

unsigned char getCheckSum(char *packet) {
    unsigned char i, checksum = 0;
    for (i = 1; i < 8; i++) {
        checksum += packet[i];
    }
    checksum = 0xff - checksum;
    checksum += 1;
    return checksum;
}

void serial_callback(const struct device *dev, void *user_data) {
	uint8_t c, high, low;
	char checksum_ok, value_calc_flag;
	int checksum;

	if (!uart_irq_update(uart_serial)) {
		return;
	}

	if (!uart_irq_rx_ready(uart_serial)) {
		printk("No data\n");
		return;
	}

	/* read until FIFO empty */
	while (uart_fifo_read(uart_serial, &c, 1) == 1) {
		// for recovery
		if (uart_fsm_state == UART_FSM_IDLE) {
			rx_buf_pos = 0;
		}
		check_uart_fsm(0,c);

		if (rx_buf_pos >= MSG_SIZE) {
			rx_buf_pos = 0;
		}
		rx_buf[rx_buf_pos++] = c;
	}

	// calculate checksum, and compare with received checksum
	checksum = getCheckSum(rx_buf);
	checksum_ok = checksum == rx_buf[8];
	if (checksum_ok) {
		// printk("Checksum OK (%d == %d, index=%d)\n", checksum, rx_buf[8], rx_buf_pos);
	} else {
		// printk("Checksum failed (%d == %d, index=%d)\n", checksum, rx_buf[8], rx_buf_pos);
	}

	// check if we received all data and checksum is OK
	value_calc_flag = rx_buf_pos == MSG_SIZE && checksum_ok;
	if (value_calc_flag) {
		high = rx_buf[2];
		high = fromHexadecimalToDecimal(high);
		low = rx_buf[3];
		low = fromHexadecimalToDecimal(low);
		current_ppm = (high * CO2_MULTIPLIER) + low;
		printk("CO2: %d ppm (aver:%d)\n", current_ppm, average_ppm);
		// print message buffer
		// for (int i = 0; i < MSG_SIZE; i+=1) {
		// 	printk("%x ", rx_buf[i]);
		// }
		// printk("\n");
	}
}


void co2_init(void) {
    if (!device_is_ready(uart_serial)) {
        printk("UART device not found!");
        return;
    }

    int ret = uart_irq_callback_user_data_set(uart_serial, serial_callback, NULL);
    if (ret < 0) {
        if (ret == -ENOTSUP) {
            printk("Interrupt-driven UART API support not enabled\n");
        } else if (ret == -ENOSYS) {
            printk("UART device does not support interrupt-driven API\n");
        } else {
            printk("Error setting UART callback: %d\n", ret);
        }
        return;
    }
    uart_irq_rx_enable(uart_serial);
}

void co2_read(void) {
    uint8_t tx_buf[MSG_SIZE] = {0xFF, 0x01, 0x86, 0x00, 0x00, 0x00, 0x00, 0x00, 0x79};
    for (int i = 0; i < MSG_SIZE; i++) {
        uart_poll_out(uart_serial, tx_buf[i]);
    }
}

void co2_thread(void) {
    int stop_signal = 0;
    int after_higher = 0;
    average_ppm = 5000;
    average_on = 0;
    current_ppm = 0;
    while (1) {
        co2_read();
        k_sleep(K_MSEC(500));
        printk("cur_ppm: %d vs aver_ppm: %d\n",current_ppm, average_ppm);
        if(current_ppm <= 0) continue;
        if (current_ppm <= average_ppm) {
            // OUT
            stop_signal++;
            if(stop_signal >= 60/*약 30초*/ || (after_higher==1 && stop_signal >= 12/*약 6초*/)){
                average_ppm = - 1;
                break;
            }
            printk("Current PPM is Less than average %d %d\n",after_higher, stop_signal);
        } else {
            // IN
            stop_signal = 0;
            after_higher = 1;
            printk("Current PPM is Greater than average %d %d\n", after_higher, stop_signal);
        }
        if(average_on){
            average_ppm = average_ppm + current_ppm + 20;
            average_ppm /= 2;
        }else{
            average_ppm = current_ppm;
            average_on = 1;
        }

    }
    k_thread_abort(k_current_get());
}