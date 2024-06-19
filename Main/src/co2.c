#include "co2.h"

LOG_MODULE_REGISTER(co2, CONFIG_LOG_DEFAULT_LEVEL);

static const struct device *const uart_serial = DEVICE_DT_GET(DT_N_ALIAS_myserial);

static char rx_buf[MSG_SIZE];
static int rx_buf_pos = 0;
int glob_ppm;

#define CO2_THRESHOLD 1600

enum uart_fsm_state_code {
    UART_FSM_IDLE,
    UART_FSM_HEADER,
    UART_FSM_DATA,
    UART_FSM_CHECKSUM,
    UART_FSM_END,
};

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
        printk("irq_update Error\n");
        return;
    }

    if (!uart_irq_rx_ready(uart_serial)) {
        printk("irq_ready: No data\n");
        return;
    }

    while (uart_fifo_read(uart_serial, &c, 1) == 1) {
        if (uart_fsm_state == UART_FSM_IDLE) {
            rx_buf_pos = 0;
        }
        check_uart_fsm(0, c);

        if (rx_buf_pos >= MSG_SIZE) {
            rx_buf_pos = 0;
        }
        rx_buf[rx_buf_pos++] = c;
    }

    if (uart_fsm_state == UART_FSM_END) {
        checksum = getCheckSum(rx_buf);
        checksum_ok = (checksum == rx_buf[8]);
        if (checksum_ok) {
            printk("Checksum OK (%d == %d, index=%d)\n", checksum, rx_buf[8], rx_buf_pos);

            value_calc_flag = (rx_buf_pos == MSG_SIZE);
            if (value_calc_flag) {
                high = rx_buf[2];
                low = rx_buf[3];
                int ppm = (high * CO2_MULTIPLIER) + low;
                printk("CO2: %d ppm (high = %d, low = %d)\n", ppm, high, low);
                
                glob_ppm = ppm;

                for (int i = 0; i < MSG_SIZE; i++) {
                    printk("%x ", rx_buf[i]);
                }
                printk("\n");
            }
        } else {
            printk("Checksum failed (%d == %d, index=%d)\n", checksum, rx_buf[8], rx_buf_pos);
        }
        check_uart_fsm(1, 0); // reset
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
    while (1) {
        co2_read();
        if (glob_ppm < CO2_THRESHOLD) {
            // OUT
            printk("less than threshold\n");
        } else {
            // IN
            printk("greater than threshold\n");
        }
        k_sleep(K_MSEC(1000));
    }
}