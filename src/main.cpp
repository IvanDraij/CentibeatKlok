extern "C"
{
#include <stdio.h>
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "TIMER.h"
#include "LCD.h"
#include "Stepmotor.h"
}
#include "iotroam.h"

#define MODE_BUTTON_GPIO GPIO_NUM_36

uint16_t centibeatCount = 0;

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
    xSemaphoreGiveFromISR()
}
