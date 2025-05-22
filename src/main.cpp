extern "C"
{
#include <stdio.h>
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "TIMER.h"
#include "LCD.h"
#include "Stepmotor.h"
}
#include "iotroam.h"

uint16_t centibeatCount = 0;

extern "C" void app_main(void)
{
    EventGroupHandle_t xCreatedEventGroup;
    xCreatedEventGroup = xEventGroupCreate();

    EventBits_t uxBits;

    
    LCD lcd = LCD();
    TIMER centibeatTimer = TIMER(&xCreatedEventGroup);
    Stepmotor motor = Stepmotor();
    WIFI wifi = WIFI("iotroam", "N4B4RiiNFg");
    
    wifi.iotroam_connect();

    while (true)
    {
        
        //motor.moveStepMotorToCentibeat(wifi.getTime());
        //vTaskDelay(200);
    }
}
