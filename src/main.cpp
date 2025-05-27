extern "C"
{
#include <stdio.h>
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "freertos/semphr.h"
#include "TIMER.h"
#include "LCD.h"
#include "Stepmotor.h"
#include "7SegDis.h"
}
#include "iotroam.h"
#include "esp_task_wdt.h"

#define HUNDREDBEATS      10000

EventGroupHandle_t xCreatedEventGroup;
SemaphoreHandle_t xMutexCentibeat;
TaskHandle_t xTimerTaskHandle = NULL;

uint32_t centibeatCount = 0;

static void vTaskDisplayBeat(void* pvParamters);
static void vTaskDisplayCentibeat(void* pvParameters);
static void vTaskTimer(void* pvParamters);

extern "C" void app_main(void)
{
    xCreatedEventGroup = xEventGroupCreate();

    EventBits_t uxBits;
    
    LCD lcd = LCD();

    WIFI wifi = WIFI("iotroam", "N4B4RiiNFg");
    wifi.iotroam_connect();

    centibeatCount = wifi.getTime();
    
    xMutexCentibeat = xSemaphoreCreateMutex();

    xTaskCreate(vTaskDisplayBeat,"7SegDis", 2048, NULL, 1, NULL);
    xTaskCreate(vTaskDisplayCentibeat,"stepper", 2048, NULL, 3, NULL);
    xTaskCreate(vTaskTimer, "TimingTask", 2048, NULL, 2, &xTimerTaskHandle);
    TIMER centibeatTimer = TIMER();
}

static void vTaskDisplayBeat(void* pvParamters)
{
    SegDis beatDisplay = SegDis();
    uint32_t localCentibeat = 0;
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

static void vTaskDisplayCentibeat(void* pvParameters)
{
    Stepmotor motor = Stepmotor();
    uint32_t localCentibeat= 0;
    for (;;)
    {
       if(xSemaphoreTake(xMutexCentibeat, portMAX_DELAY) == pdTRUE)// when the semaphore is free update the local centibeat
        {
            if(localCentibeat != centibeatCount)
            {
                localCentibeat = centibeatCount; //change the local time to the global time
            }
            xSemaphoreGive(xMutexCentibeat); //free semaphore
        } 
        motor.moveStepMotorToCentibeat(localCentibeat); //always set stepmotor to the local time
    }
}
static void vTaskTimer(void* pvParamters)
{ 
    for (;;)
    {
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY); // wait until timer gives signal
        //run the task
        if(xSemaphoreTake(xMutexCentibeat, portMAX_DELAY)== pdTRUE) //get the mutex to change centibeat
        {
            centibeatCount++;
            if (centibeatCount == HUNDREDBEATS)
            {
                xEventGroupSetBits(xCreatedEventGroup, (1 << 0));// every hundred beats sync with ntp
            }
            xSemaphoreGive(xMutexCentibeat);
        }
    }
}