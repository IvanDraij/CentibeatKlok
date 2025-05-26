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

#define modeSwitchButtonPriority 3
#define MODE_BUTTON_GPIO GPIO_NUM_36
#define usStackDepthModeSwitchButton 2048
#define hundredmsDelay 100

uint16_t centibeatCount = 0;

bool automaticMode = true;

void initTasks();
void initButtonInterrupt();
void vTaskModeButton(void *arg);
void vTaskLoopModeButton(void *arg);
void showModeOnDisplay(bool automaticMode);



SemaphoreHandle_t switchButtonSemaphore;



extern "C" void app_main(void)
{
    LCD lcd = LCD();
    TIMER centibeatTimer = TIMER();
    Stepmotor motor = Stepmotor();
    WIFI wifi = WIFI("iotroam", "N4B4RiiNFg");

    initTasks();
    
    wifi.iotroam_connect();

    while (true)
    {
    if (automaticMode)
        {
            char tempArray[1] = {'A'};
            lcd.printStr(tempArray,0,14);
        }
    else
        {
            char tempArray[1] = {'M'};
            lcd.printStr(tempArray,0,14);
        }


    motor.moveStepMotorToCentibeat(wifi.getTime());
    vTaskDelay(pdMS_TO_TICKS(200));
    }
}

void initTasks()
{
    initButtonInterrupt();
    switchButtonSemaphore = xSemaphoreCreateBinary();                 // Create the switchButtonSemaphore
    xTaskCreate(vTaskLoopModeButton,"changeModeButton",usStackDepthModeSwitchButton,NULL,modeSwitchButtonPriority,NULL);
}

void initButtonInterrupt()
{
    // Configure BUTTON
  gpio_config_t btn_conf = {};
  btn_conf.intr_type = GPIO_INTR_NEGEDGE;        // Falling edge
  btn_conf.mode = GPIO_MODE_INPUT;               // Input mode
  btn_conf.pin_bit_mask = (1ULL << MODE_BUTTON_GPIO); // Which pins need initialising
  btn_conf.pull_up_en = GPIO_PULLUP_ENABLE;      // Enable pull-up resistor
  gpio_config(&btn_conf);

    gpio_install_isr_service(0); // Install ISR service

    gpio_isr_handler_add(MODE_BUTTON_GPIO, vTaskModeButton, nullptr);
}

void IRAM_ATTR vTaskModeButton(void *arg)
{
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    xSemaphoreGiveFromISR(switchButtonSemaphore, &xHigherPriorityTaskWoken);
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}


void vTaskLoopModeButton(void *arg)
{
    while (true)
    {
        if (xSemaphoreTake(switchButtonSemaphore,portMAX_DELAY))
        {
            if (gpio_get_level(MODE_BUTTON_GPIO) == 0)
            {
                vTaskDelay(pdMS_TO_TICKS(hundredmsDelay));
                if (gpio_get_level(MODE_BUTTON_GPIO) == 0)
                {
                    ESP_LOGI("switchButtonPressed", "switchButton pressed");
                    automaticMode = !automaticMode;
                    showModeOnDisplay(automaticMode);
                }
            }
        }
    }
}
