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
#include "Rotary_Enc.cpp"
}
#include "iotroam.h"
#include "esp_task_wdt.h"

#define HUNDREDBEATS      10000
#define modeSwitchButtonPriority 3
#define MODE_BUTTON_GPIO GPIO_NUM_36
#define usStackDepthModeSwitchButton 2048
#define hundredmsDelay 100

EventGroupHandle_t xCreatedEventGroup;
EventGroupHandle_t xKlokEventgroup;

SemaphoreHandle_t xMutexCentibeat;
TaskHandle_t xTimerTaskHandle = NULL;
#define SYNCTIME (1 << 0)


uint32_t centibeatCount = 0;

static void vTaskDisplayBeat(void* pvParamters);
static void vTaskDisplayCentibeat(void* pvParameters);
static void vTaskTimer(void* pvParamters);
bool automaticMode = true;

void initTasks(LCD *lcd);
void initButtonInterrupt();
void ISR_switchModeButton(void *arg);
void vTaskLoopModeButton(void *arg);
static void vTaskSyncNTP(void* pvParameters);

SemaphoreHandle_t switchButtonSemaphore;

TaskHandle_t xRotEncTaskHandle = NULL;
Rotary_Enc *RotEnc;
static void vTaskReadRotary(void *);

extern "C" void
app_main(void)
{
    xCreatedEventGroup = xEventGroupCreate();
    xKlokEventgroup = xEventGroupCreate();

    LCD lcd = LCD();
    WIFI* wifi = new WIFI("iotroam", "N4B4RiiNFg");

    wifi->iotroam_connect();    

    RotEnc = new Rotary_Enc();
    xMutexCentibeat = xSemaphoreCreateMutex();
    xTaskCreate(vTaskReadRotary, "RotaryTask", 2048, NULL, 2, &xRotEncTaskHandle);

    xMutexCentibeat = xSemaphoreCreateMutex();

    initButtonInterrupt();
    switchButtonSemaphore = xSemaphoreCreateBinary();                           // Create the switchButtonSemaphore
    xTaskCreate(vTaskLoopModeButton,"changeModeButton",usStackDepthModeSwitchButton,&lcd,modeSwitchButtonPriority,NULL);

    xTaskCreate(vTaskDisplayBeat,"7SegDis", 2048, NULL, 1, NULL);
    xTaskCreate(vTaskDisplayCentibeat,"stepper", 2048, NULL, 3, NULL);
    xTaskCreate(vTaskTimer, "TimingTask", 2048, NULL, 2, &xTimerTaskHandle);
    TIMER centibeatTimer = TIMER();
    
    xTaskCreate(vTaskSyncNTP,"NTPSync", 2048,(void*)wifi, 1, NULL);

    xEventGroupSetBits(xKlokEventgroup, SYNCTIME);
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

static void vTaskReadRotary(void *pvParameters)
{
    uint32_t localCentibeat = 0;
    for (;;)
    {
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
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
    }
}