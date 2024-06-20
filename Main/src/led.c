#include "led.h"
#define MAX_LED_NUM 128
#define MAX_LED_MATRIX_IDX 10
#define MAX_LED_MATRIX_NUM 64
int number_led_matrix_arr [MAX_LED_MATRIX_IDX+1][MAX_LED_MATRIX_NUM+1]= {
{ // 0
0,0,0,1,1,1,0,0,
0,0,1,0,0,0,1,0,
0,0,1,0,0,0,1,0,
0,0,1,0,0,0,1,0,
0,0,1,0,0,0,1,0,
0,0,1,0,0,0,1,0,
0,0,1,0,0,0,1,0,
0,0,0,1,1,1,0,0
},
{ // 1
0,0,0,0,1,0,0,0,
0,0,0,1,1,0,0,0,
0,0,1,0,1,0,0,0,
0,0,0,0,1,0,0,0,
0,0,0,0,1,0,0,0,
0,0,0,0,1,0,0,0,
0,0,0,0,1,0,0,0,
0,0,1,1,1,1,1,0
},
{ // 2
0,0,0,0,1,0,0,0,
0,0,0,1,0,1,0,0,
0,0,1,0,0,0,1,0,
0,0,0,0,0,1,0,0,
0,0,0,0,1,0,0,0,
0,0,0,1,0,0,0,0,
0,0,1,0,0,0,0,0,
0,0,1,1,1,1,1,0
},
{ // 3
0,0,1,1,1,1,0,0,
0,0,0,0,0,0,1,0,
0,0,0,0,0,0,1,0,
0,0,1,1,1,1,0,0,
0,0,0,0,0,0,1,0,
0,0,0,0,0,0,1,0,
0,0,0,0,0,0,1,0,
0,0,1,1,1,1,0,0
},
{ // 4
0,0,0,0,0,1,0,0,
0,0,0,0,1,1,0,0,
0,0,0,1,0,1,0,0,
0,0,1,0,0,1,0,0,
0,1,1,1,1,1,1,0,
0,0,0,0,0,1,0,0,
0,0,0,0,0,1,0,0,
0,0,0,0,0,1,0,0
},
{ // 5
0,0,1,1,1,1,1,0,
0,0,1,0,0,0,0,0,
0,0,1,0,0,0,0,0,
0,0,1,1,1,1,0,0,
0,0,0,0,0,0,1,0,
0,0,0,0,0,0,1,0,
0,0,1,0,0,0,1,0,
0,0,0,1,1,1,0,0
},
{ // 6
0,0,0,1,1,1,0,0,
0,0,1,0,0,0,0,0,
0,0,1,0,0,0,0,0,
0,0,1,1,1,1,0,0,
0,0,1,0,0,0,1,0,
0,0,1,0,0,0,1,0,
0,0,1,0,0,0,1,0,
0,0,0,1,1,1,0,0
},
{ // 7
0,0,1,1,1,1,1,0,
0,0,1,0,0,0,1,0,
0,0,1,0,0,0,1,0,
0,0,1,0,0,0,1,0,
0,0,0,0,0,0,1,0,
0,0,0,0,0,0,1,0,
0,0,0,0,0,0,1,0,
0,0,0,0,0,0,1,0
},
{ // 8
0,0,0,1,1,1,0,0,
0,0,1,0,0,0,1,0,
0,0,1,0,0,0,1,0,
0,0,0,1,1,1,0,0,
0,0,1,0,0,0,1,0,
0,0,1,0,0,0,1,0,
0,0,1,0,0,0,1,0,
0,0,0,1,1,1,0,0
},
{ // 9
0,0,0,1,1,1,0,0,
0,0,1,0,0,0,1,0,
0,0,1,0,0,0,1,0,
0,0,1,0,0,0,1,0,
0,0,0,1,1,1,1,0,
0,0,0,0,0,0,1,0,
0,0,0,0,0,0,1,0,
0,0,0,0,0,0,1,0
}
};

int led_init(void)
{
    if(!device_is_ready(led)){
        printk("LED device %s is not ready\n", led->name);
    }

    return 0;
}
//MAX_LED_NUM
void led_off_all(void)
{
    for(int i = 15; i>= 0; i--){
        led_off(led,i); //1
        led_off(led,i+16); //17
        led_off(led,i+32);
        led_off(led,i+48);
        led_off(led,i+64);
        led_off(led,i+80);
        led_off(led,i+96);
        led_off(led,i+112);
    }
}

void led_on_idx(int btn_flag, int current_idx)
{
    if(btn_flag == 0){
        for(int i = 0; i <= current_idx; i++){
            led_on(led,i); //1
            led_on(led,i+16); //17
            led_on(led,i+32);
            led_on(led,i+48);
            led_on(led,i+64);
            led_on(led,i+80);
            led_on(led,i+96);
            led_on(led,i+112);
            k_sleep(K_MSEC(10));
        }
        for(int i = current_idx; i >= 0; i--){
            led_off(led,i); //1
            led_off(led,i+16); //17
            led_off(led,i+32);
            led_off(led,i+48);
            led_off(led,i+64);
            led_off(led,i+80);
            led_off(led,i+96);
            led_off(led,i+112);
            // k_sleep(K_MSEC(100));
        }
    }else if(btn_flag == 1){
        int tens = current_idx / 10;
        int units = current_idx % 10;
        int num_arr_idx = 0;
        for(int i = 0; i < MAX_LED_NUM; i+=16){
            for(int j = i; j < (i+8); j++){
                if(number_led_matrix_arr[tens][num_arr_idx] == 1){
                    led_on(led, j);
                } else {
                    led_off(led, j);
                }
            num_arr_idx++;
            }
        }
        num_arr_idx = 0;
        for(int i = 0; i < MAX_LED_NUM; i+=16){
            for(int j = (i+8); j < (i+16); j++){
                if(number_led_matrix_arr[units][num_arr_idx] == 1){
                    led_on(led, j);
                } else {
                    led_off(led, j);
                }
            num_arr_idx++;
            }
        }
        k_sleep(K_MSEC(500));
        led_off_all();
    }else if(btn_flag == 2){
        for(int i = 0; i <= 15; i++){
            led_on(led,i); //1
            led_on(led,i+16); //17
            led_on(led,i+32);
            led_on(led,i+48);
            led_on(led,i+64);
            led_on(led,i+80);
            led_on(led,i+96);
            led_on(led,i+112);
        }
        k_sleep(K_MSEC(500));
        led_off_all();
    }

}
