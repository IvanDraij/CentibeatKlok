extern "C"
{
#include <stdio.h>
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "TIMER.h"
#include "LCD.h"
#include "Stepmotor.h"
#include "esp_log.h"

}
#include "iotroam.h"

#define MODE_BUTTON_GPIO GPIO_NUM_36

uint16_t centibeatCount = 0;

bool automaticMode = true;

void vTaskModeButtonInit();
void vTaskModeButton(void *arg);
void taskLoopModeButton();


SemaphoreHandle_t switchButtonSemaphore;

extern "C" void app_main(void)
{
    
    LCD lcd = LCD();
    TIMER centibeatTimer = TIMER();
    Stepmotor motor = Stepmotor();
    WIFI wifi = WIFI("iotroam", "N4B4RiiNFg");

    void initTasks();
    
    wifi.iotroam_connect();

    while (true)
    {
        motor.moveStepMotorToCentibeat(wifi.getTime());
        vTaskDelay(200);
    }
}

void initTasks()
{
    vTaskModeButtonInit();
    switchButtonSemaphore = xSemaphoreCreateBinary();                 // Create the switchButtonSemaphore
}

void vTaskModeButtonInit()
{
    // Configure BUTTON
  gpio_config_t btn_conf = {};
  btn_conf.intr_type = GPIO_INTR_NEGEDGE;        // Falling edge
  btn_conf.mode = GPIO_MODE_INPUT;               // Input mode
  btn_conf.pin_bit_mask = (1ULL << MODE_BUTTON_GPIO); // Which pins need initialising
  btn_conf.pull_up_en = GPIO_PULLUP_ENABLE;      // Enable pull-up resistor
  gpio_config(&btn_conf);

    gpio_install_isr_service(0); // Install ISR service

    gpio_isr_handler_add(MODE_BUTTON_GPIO, vTaskModeButton, nullptr);     // Register ISR handler for button
}

void vTaskModeButton(void *arg)
{
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;                           // Initialise a variable to check whether a task with high priority was waiting on the semaphore
    xSemaphoreGiveFromISR(switchButtonSemaphore, &xHigherPriorityTaskWoken);
}

void taskLoopModeButton()
{
    while (true)
    {
        if (xSemaphoreTake(switchButtonSemaphore,portMAX_DELAY))
        {
            if (gpio_get_level(MODE_BUTTON_GPIO) == 0)
            {
                vTaskDelay(100);
                if (gpio_get_level(MODE_BUTTON_GPIO) == 0)
                {
                    ESP_LOGI("switchButtonPressed", "switchButton pressed");
                    automaticMode = !automaticMode;
                }
            }
        }
    }
}
