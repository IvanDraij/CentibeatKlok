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
Rotary_Enc *RotEnc;
static void vTaskReadRotary(void *);

extern "C" void
app_main(void)
{
    LCD lcd = LCD();
    TIMER centibeatTimer = TIMER();
    Stepmotor motor = Stepmotor();
    RotEnc = new Rotary_Enc();
    xMutexCentibeat = xSemaphoreCreateMutex();
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
    uint32_t localCentibeat = 0;
    for (;;)
    {
        if (!RotEnc->automatic)
        {
            localCentibeat += RotEnc->consumeSteps();

            if (xSemaphoreTake(xMutexCentibeat, portMAX_DELAY) == pdTRUE)
            {
                centibeatCount += localCentibeat;
                ESP_LOGI("Centibeat", "Count %u", centibeatCount);
                xSemaphoreGive(xMutexCentibeat);
            }
            localCentibeat = 0;
        }
        vTaskDelay(pdMS_TO_TICKS(10)); // Delay 10 ms to yield CPU
    }
}