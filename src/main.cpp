extern "C"
{
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
    // lcd.printStr("Je moder", 1, 0);
    Stepmotor motor = Stepmotor();
    // motor.moveStepMotorToCentibeat(25);

    iotroam_init("iotroam", "N4B4RiiNFg");
    iotroam_connect();

    WIFI wifi = WIFI();

    while (true)
    {
        motor.moveStepMotorToCentibeat(wifi.printTime());
        vTaskDelay(200);
    }
}
