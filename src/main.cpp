extern "C" {
#include <stdio.h>
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "TIMER.h"
#include "LCD.h"
#include "Stepmotor.h"
}
#include "iotroam.h"

uint16_t centibeatCount = 0;

extern "C" void app_main(void)
{
    LCD lcd = LCD();
    TIMER centibeatTimer = TIMER();
    lcd.printStr("Je moder", 1, 0);
    Stepmotor motor = Stepmotor();
    //motor.moveStepMotorToCentibeat(30);
    
    iotroam_init("iotroam", "N4B4RiiNFg");
    iotroam_connect();
    //vTaskDelay(pdMS_TO_TICKS(1000));
    //iotroam_disconnect();
    //vTaskDelay(pdMS_TO_TICKS(1000));

    while(true)
    {
        char ch[8];
        sprintf(ch, "%d", centibeatCount);
        lcd.printStr(ch, 0,12);
        motor.moveStepMotorToCentibeat(centibeatCount);
        vTaskDelay(10/ portTICK_PERIOD_MS);
    }
}
