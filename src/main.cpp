extern "C" {
#include <stdio.h>
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "esp_timer.h"
#include "LCD.h"
}

uint16_t count = 0;

void timer_callback(void *param)
{
    count++;
}

extern "C" void app_main(void)
{
    LCD lcd = LCD();
    const esp_timer_create_args_t my_timer_args = 
    {
        .callback = &timer_callback,
        .name = "Timer Interrupt"
    };

    esp_timer_handle_t timer_handler;
    esp_timer_create(&my_timer_args, &timer_handler);
    esp_timer_start_periodic(timer_handler, 10000);  // One Second = 1000000 micro second

    while(true)
    {
        char ch[8];
        sprintf(ch, "%d", count);
        lcd.printStr(ch, 0,0);
        vTaskDelay(10/ portTICK_PERIOD_MS);
    }

}