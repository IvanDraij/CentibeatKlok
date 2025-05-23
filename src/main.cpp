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
static void vTaskDisplayBeat(void* pvParamters);

extern "C" void app_main(void)
{
    // LCD lcd = LCD();
    // TIMER centibeatTimer = TIMER();
    // Stepmotor motor = Stepmotor();
    WIFI wifi = WIFI("iotroam", "N4B4RiiNFg");
    SegDis beatDisplay = SegDis();

    wifi.iotroam_connect();
    xTaskCreate(vTaskDisplayBeat,"7SegDis", 700, &beatDisplay, 1, NULL);


    // while (true)
    // {
    //     // motor.moveStepMotorToCentibeat(wifi.getTime());
    //     // vTaskDelay(200);
    // }
}
static void vTaskDisplayBeat(void* pvParamters)
{
    SegDis* display = (SegDis*) pvParamters;
    uint32_t localCentibeat= 7634;
    for (;;)
    {
        //if() // semaphore beschikbaar update local
        display->displayBeat(localCentibeat);
    }
}