extern "C"
{
#include <stdio.h>
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "TIMER.h"
#include "LCD.h"
#include "Stepmotor.h"
#include "Rotary_Enc.cpp"
}
#include "iotroam.h"

uint16_t centibeatCount = 0;
SemaphoreHandle_t xMutexCentibeat;
TaskHandle_t xRotEncTaskHandle = NULL;


extern "C" void app_main(void)
{
    LCD lcd = LCD();
    TIMER centibeatTimer = TIMER();
    Stepmotor motor = Stepmotor();
    xTaskCreate(vTaskReadRotary, "RotaryTask", 2048, NULL, 2, &xRotEncTaskHandle);

    // iotroam_init("iotroam", "N4B4RiiNFg");
    // iotroam_connect();

    // WIFI wifi = WIFI();
    // Rotary_Enc rotenc = Rotary_Enc();

    while (true)
    {
        // motor.moveStepMotorToCentibeat(wifi.getTime());
        // vTaskDelay(200);
    }
}

static void vTaskReadRotary(void *pvParameters)
{
    Rotary_Enc RotEnc = Rotary_Enc();
    uint32_t localCentibeat = 0;
    for (;;)
    {
        if (!RotEnc.automatic)
        {
            localCentibeat = centibeatCount;
            localCentibeat += RotEnc.stepsToTake;
            // localCentibeat
            if (xSemaphoreTake(xMutexCentibeat, portMAX_DELAY) == pdTRUE) // when the semaphore is free update the local centibeat
            {
                centibeatCount = localCentibeat;
                xSemaphoreGive(xMutexCentibeat); // free semaphore
            }
        }
    }
}