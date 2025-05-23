extern "C"
{
#include <stdio.h>
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "TIMER.h"
#include "LCD.h"
#include "Stepmotor.h"
#include "7SegDis.h"
}
#include "iotroam.h"

SemaphoreHandle_t xMutexCentibeat;

uint32_t centibeatCount = 0;

static void vTaskDisplayBeat(void* pvParamters);

extern "C" void app_main(void)
{
    LCD lcd = LCD();
    TIMER centibeatTimer = TIMER();
    Stepmotor motor = Stepmotor();
    WIFI wifi = WIFI("iotroam", "N4B4RiiNFg");

    wifi.iotroam_connect();
    xMutexCentibeat = xSemaphoreCreateMutex();
    xTaskCreate(vTaskDisplayBeat,"7SegDis", 2048, NULL, 1, NULL);


    // while (true)
    // {
    //     // motor.moveStepMotorToCentibeat(wifi.getTime());
    //     // vTaskDelay(200);
    // }
}
static void vTaskDisplayBeat(void* pvParamters)
{
    SegDis beatDisplay = SegDis();
    uint32_t localCentibeat= 0;// TEST MOET 0 ZIJN
    for (;;)
    {
        if(xSemaphoreTake(xMutexCentibeat, portMAX_DELAY) == pdTRUE)// when the semaphore is free update the local centibeat
        {
            localCentibeat = centibeatCount;
            xSemaphoreGive(xMutexCentibeat);
        } 
        beatDisplay.displayBeat(localCentibeat);
    }
}