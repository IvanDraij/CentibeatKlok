extern "C"
{
#include <stdio.h>
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "TIMER.h"
#include "LCD.h"
#include "Stepmotor.h"
#include "esp_log.h"
#include "7SegDis.h"
}
#include "iotroam.h"

#define modeSwitchButtonPriority 3
#define MODE_BUTTON_GPIO GPIO_NUM_36
#define usStackDepthModeSwitchButton 2048
#define hundredmsDelay 100

SemaphoreHandle_t xMutexCentibeat;

uint32_t centibeatCount = 0;

static void vTaskDisplayBeat(void* pvParamters);

bool automaticMode = true;

void initTasks(LCD *lcd);
void initButtonInterrupt();
void ISR_switchModeButton(void *arg);
void vTaskLoopModeButton(void *arg);

SemaphoreHandle_t switchButtonSemaphore;

extern "C" void app_main(void)
{
    LCD lcd = LCD();
    
    Stepmotor motor = Stepmotor();
    WIFI wifi = WIFI("iotroam", "N4B4RiiNFg");
    initTasks(&lcd);
    xMutexCentibeat = xSemaphoreCreateMutex();
    xTaskCreate(vTaskDisplayBeat,"7SegDis", 2048, NULL, 1, NULL);

    TIMER centibeatTimer = TIMER();
    wifi.iotroam_connect();
}

void initTasks(LCD *lcd) // initialises tasks
{
    initButtonInterrupt();
    switchButtonSemaphore = xSemaphoreCreateBinary();                           // Create the switchButtonSemaphore
    xTaskCreate(vTaskLoopModeButton,"changeModeButton",usStackDepthModeSwitchButton,lcd,modeSwitchButtonPriority,NULL);
}

void initButtonInterrupt() // initialises the mode switch button interrupt
{
    // Configure BUTTON
  gpio_config_t btn_conf = {};
  btn_conf.intr_type = GPIO_INTR_NEGEDGE;                                       // Falling edge
  btn_conf.mode = GPIO_MODE_INPUT;                                              // Input mode
  btn_conf.pin_bit_mask = (1ULL << MODE_BUTTON_GPIO);                           // Which pins need initialising
  gpio_config(&btn_conf);

  gpio_install_isr_service(0);                                                  // Install ISR service

  gpio_isr_handler_add(MODE_BUTTON_GPIO, ISR_switchModeButton, nullptr);             // calls ISR_switchModeButton when interrupt happens
}

void ISR_switchModeButton(void *arg) // function that is called when button interrupt happens
{
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;                              // check if higher priority task is running    
    xSemaphoreGiveFromISR(switchButtonSemaphore, &xHigherPriorityTaskWoken);    // gives semaphore for switchModeButton
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);                               // checks if the semaphore task has higher priority than other tasks
}


void vTaskLoopModeButton(void *arg) // loop funtion waiting for switch mode button semaphore
{

    LCD *lcd = static_cast<LCD*>(arg);

    while (true)
    {
        if (xSemaphoreTake(switchButtonSemaphore,portMAX_DELAY))                // wait for semaphore availability
        {
            if (gpio_get_level(MODE_BUTTON_GPIO) == 0)
            {
                vTaskDelay(pdMS_TO_TICKS(hundredmsDelay));                      // debounce
                if (gpio_get_level(MODE_BUTTON_GPIO) == 0)
                {
                    ESP_LOGI("switchButtonPressed", "switchButton pressed");    // serial monitoring
                    automaticMode = !automaticMode;                             // flips the mode bool
                    lcd->sendComannd(automaticMode ? AUTOMODE : MANUAL);        // depending on the automaticMode bool the command will be AUTOMODE or MANUAL

                }
            }
        }
    }
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