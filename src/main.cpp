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

extern "C" void app_main(void)
{
    LCD lcd = LCD();
    TIMER centibeatTimer = TIMER();
    Stepmotor motor = Stepmotor();

    // iotroam_init("iotroam", "N4B4RiiNFg");
    // iotroam_connect();

    // WIFI wifi = WIFI();
    Rotary_Enc rotenc = Rotary_Enc();

    while (true)
    {
        // motor.moveStepMotorToCentibeat(wifi.getTime());
        // vTaskDelay(200);
    }
}

static void vTaskReadRotary(void *pvParameters)
{
    Stepmotor motor = Stepmotor();
    uint32_t localCentibeat = 0;
    for (;;)
    {
        if (xSemaphoreTake(xMutexCentibeat, portMAX_DELAY) == pdTRUE) // when the semaphore is free update the local centibeat
        {
            if (localCentibeat != centibeatCount)
            {
                localCentibeat = centibeatCount; // change the local time to the global time
            }
            xSemaphoreGive(xMutexCentibeat); // free semaphore
        }
        motor.moveStepMotorToCentibeat(localCentibeat); // always set stepmotor to the local time
    }
}