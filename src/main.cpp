extern "C"
{
#include <stdio.h>
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "TIMER.h"
#include "LCD.h"
#include "Stepmotor.h"
#include "7SegDis.h"
}
#include "iotroam.h"

uint16_t centibeatCount = 0;

extern "C" void app_main(void)
{
    LCD lcd = LCD();
    TIMER centibeatTimer = TIMER();
    Stepmotor motor = Stepmotor();
    WIFI wifi = WIFI("iotroam", "N4B4RiiNFg");
    SegDis beatDisplay = SegDis();

    wifi.iotroam_connect();
    beatDisplay.displayNumber(4, 1);

    while (true)
    {
        motor.moveStepMotorToCentibeat(wifi.getTime());
        vTaskDelay(200);
    }
}
