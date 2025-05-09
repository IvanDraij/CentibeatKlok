extern "C" {
#include <stdio.h>
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "TIMER.h"
#include "LCD.h"
}

uint16_t count = 0;

extern "C" void app_main(void)
{
    LCD lcd = LCD();
    TIMER centibeatTimer = TIMER();
    lcd.printStr("Je moder", 1, 0);

    while(true)
    {
        char ch[8];
        sprintf(ch, "%d", count);
        lcd.printStr(ch, 0,0);
        vTaskDelay(10/ portTICK_PERIOD_MS);
    }
}