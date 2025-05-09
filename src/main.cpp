extern "C" {
#include <stdio.h>
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "esp_timer.h"
#include "LCD.h"
}

uint16_t count = 0;
uint16_t beatCount = 0;
uint16_t seconds = 0;

void centibeat_timer_callback(void *param)
{
    // if (count >= 99)
    // {
    //     beatCount++;
    //     count = 0;
    // }
    // else   
        count++;
}

void second_timer_callback(void *param)
{
    seconds++;
}

void centibeat_timer_init(void)
{
    const esp_timer_create_args_t centibeat_timer_args = 
    {
        .callback = &centibeat_timer_callback,
        .name = "Centibeat timer Interrupt"
    };

    esp_timer_handle_t centibeat_timer_handler;
    esp_timer_create(&centibeat_timer_args, &centibeat_timer_handler);
    esp_timer_start_periodic(centibeat_timer_handler, 864000);  // One Second = 1000000 micro second
}

void second_timer_init(void)
{
    const esp_timer_create_args_t secondtimer_args = 
    {
        .callback = &second_timer_callback,
        .name = "second timer"
    };
    esp_timer_handle_t second_timer_handler;
    esp_timer_create(&secondtimer_args, &second_timer_handler);
    esp_timer_start_periodic(second_timer_handler,1000000);
}

extern "C" void app_main(void)
{
    LCD lcd = LCD();
    centibeat_timer_init();
    second_timer_init();


    while(true)
    {
        char ch[8];
        sprintf(ch, "%d", count);
        lcd.printStr(ch, 0,0);
        char chB[8];
        sprintf(chB, "%d", seconds);
        lcd.printStr(chB, 1, 0);
        vTaskDelay(10/ portTICK_PERIOD_MS);
    }
}