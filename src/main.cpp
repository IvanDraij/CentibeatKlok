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

#define HUNDERDBEATS      10000

EventGroupHandle_t xCreatedEventGroup;
SemaphoreHandle_t xMutexCentibeat;
TaskHandle_t xYourTaskHandle = NULL;

uint32_t centibeatCount = 0;

static void vTaskDisplayBeat(void* pvParamters);
static void vTaskDisplayCentibeat(void* pvParameters);
static void vTaskTiming(void* pvParamters);

extern "C" void app_main(void)
{
    xCreatedEventGroup = xEventGroupCreate();

    EventBits_t uxBits;
    TIMER centibeatTimer = TIMER();
    LCD lcd = LCD();
    //WIFI wifi = WIFI("iotroam", "N4B4RiiNFg");

    //wifi.iotroam_connect();
    xMutexCentibeat = xSemaphoreCreateMutex();
    xTaskCreate(vTaskDisplayBeat,"7SegDis", 2048, NULL, 1, NULL);
    xTaskCreate(vTaskDisplayCentibeat,"stepper", 2048, NULL, 3, NULL);
    xTaskCreate(vTaskTiming, "TimingTask", 2048, NULL, 2, &xYourTaskHandle);
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
static void vTaskDisplayCentibeat(void* pvParameters)
{
    Stepmotor motor = Stepmotor();
    uint32_t localCentibeat= 0;// TEST MOET 0 ZIJN
    bool changed= false;
    for (;;)
    {
       if(xSemaphoreTake(xMutexCentibeat, portMAX_DELAY) == pdTRUE)// when the semaphore is free update the local centibeat
        {
            if(localCentibeat != centibeatCount){
                localCentibeat = centibeatCount;
                changed= true;
            }
            xSemaphoreGive(xMutexCentibeat);
        } 
        if(changed)
        {
        
        motor.moveStepMotorToCentibeat(localCentibeat);
        changed = false;
        }
    }
}
static void vTaskTiming(void* pvParamters)
{ 
    
    for (;;)
    {
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
        if(xSemaphoreTake(xMutexCentibeat, portMAX_DELAY)== pdTRUE)
        {
            centibeatCount++;
            if (centibeatCount == HUNDERDBEATS)
            {
                xEventGroupSetBits(xCreatedEventGroup, (1 << 0));
            }
            xSemaphoreGive(xMutexCentibeat);
        }
    }
    
}