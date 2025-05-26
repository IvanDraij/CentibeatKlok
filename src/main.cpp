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

#define SYNCTIME (1 << 0)

SemaphoreHandle_t xMutexCentibeat;
EventGroupHandle_t xKlokEventgroup;

uint32_t centibeatCount = 0;

static void vTaskDisplayBeat(void* pvParamters);
static void vTaskSyncNTP(void* pvParameters);

extern "C" void app_main(void)
{

    xKlokEventgroup = xEventGroupCreate();
    LCD lcd = LCD();
    TIMER centibeatTimer = TIMER();
    Stepmotor motor = Stepmotor();
    WIFI* wifi = new WIFI("iotroam", "N4B4RiiNFg");

    wifi->iotroam_connect();

    xMutexCentibeat = xSemaphoreCreateMutex();
    xTaskCreate(vTaskDisplayBeat,"7SegDis", 2048, NULL, 1, NULL);
    xTaskCreate(vTaskSyncNTP,"NTPSync", 2048,(void*)wifi, 1, NULL);


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

static void vTaskSyncNTP(void* pvParameters)
{
    WIFI* wifi = static_cast<WIFI*>(pvParameters);
    for (;;)
    {
        xEventGroupWaitBits(xKlokEventgroup, SYNCTIME, pdTRUE, pdFALSE, portMAX_DELAY); // wait for flag to get time from SNTP
        if(xSemaphoreTake(xMutexCentibeat, portMAX_DELAY)== pdTRUE) //get semaphore for protected writing
        {
            centibeatCount = wifi->getTime();// sync centibeat time with SNTP
            xSemaphoreGive(xMutexCentibeat);
        }
    }
}