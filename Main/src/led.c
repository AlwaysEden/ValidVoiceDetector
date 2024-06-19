#include "led.h"

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

    for(int i = 16; i>= 0; i--){
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

// [0]
// * * * * * * * * * * * * * * * *  0 ~ 15
// * * * * * * * * * * * * * * * * 16 ~ 31
// * * * * * * * * * * * * * * * * 32 ~ 47
// * * * * * * * * * * * * * * * * 48 ~ 63
// * * * * * * * * * * * * * * * * 64 ~ 79
// * * * * * * * * * * * * * * * * 80 ~ 95
// * * * * * * * * * * * * * * * * 96 ~ 111
// * * * * * * * * * * * * * * * * 112 ~ 127

void led_on_idx(int prev_idx, int current_idx)
{
    led_off_all();
    // if(prev_idx < current_idx){
        for(int i = prev_idx; i < current_idx; i++){
            led_on(led,i); //1
            led_on(led,i+16); //17
            led_on(led,i+32);
            led_on(led,i+48);
            led_on(led,i+64);
            led_on(led,i+80);
            led_on(led,i+96);
            led_on(led,i+112);
        }
    // }
    // else if(prev_idx >= current_idx){
    //     for(int i = prev_idx; i >= current_idx; i--){
    //         led_off(led,i); //1
    //         led_off(led,i+16); //17
    //         led_off(led,i+32);
    //         led_off(led,i+48);
    //         led_off(led,i+64);
    //         led_off(led,i+80);
    //         led_off(led,i+96);
    //         led_off(led,i+112);
    //     }

    // }

}
