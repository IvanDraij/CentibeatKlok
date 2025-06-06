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

#define TIMTETOSYNC      10000 // on the 100 beats
#define modeSwitchButtonPriority 3
#define MODE_BUTTON_GPIO GPIO_NUM_36
#define usStackDepthModeSwitchButton 2048
#define hundredmsDelay 100
#define SYNCTIME (1 << 0)
#define MAXCENTIBEATS 99999
#define LCDQUEUELENGHT 4
#define ONESTEP 1

EventGroupHandle_t xKlokEventgroup;

SemaphoreHandle_t switchButtonSemaphore;
SemaphoreHandle_t xMutexCentibeat;

TaskHandle_t xTimerTaskHandle = NULL;
TaskHandle_t xHandleInit = NULL;

QueueHandle_t xQueueLCD;

bool automaticMode = true;

uint32_t centibeatCount = 0;

static void vTaskDisplayBeat(void* pvParamters);
static void vTaskDisplayCentibeat(void* pvParameters);
static void vTaskTimer(void* pvParamters);
void initButtonInterrupt();
void ISR_switchModeButton(void *arg);
void vTaskLoopModeButton(void *arg);
static void vTaskSyncNTP(void* pvParameters);
static void vTaskReadRotary(void *);
static void vTaskPrintLCD(void* pvParameters);
static void vTaskInitKlok(void* pvParameters);

extern "C" void
app_main(void)
{
    xKlokEventgroup = xEventGroupCreate();
    xQueueLCD = xQueueCreate(LCDQUEUELENGHT, sizeof(uint8_t));

    xTaskCreate(vTaskInitKlok, "initialisation", 4096, NULL, 1, &xHandleInit);          // task for only the init of the clock
    xTaskCreatePinnedToCore(vTaskPrintLCD, "lcdHandle", 2048, NULL, 1, NULL, 0);  // create task lcd for handling lcd queue

    xEventGroupWaitBits(xKlokEventgroup, STARTKLOK, pdFALSE, pdFALSE, portMAX_DELAY); // when the rotary button is pressed
    vTaskDelete(xHandleInit); // delete the init task

    xMutexCentibeat = xSemaphoreCreateMutex();           //   mutex for safe reading and writing the CentibeatCount                      
    switchButtonSemaphore = xSemaphoreCreateBinary();                         // Create the switchButtonSemaphore

    uint8_t command = CLEAR;
    xQueueSend(xQueueLCD, &command, portMAX_DELAY); // clear the LCD
    
    command = AUTOMODE;
    xQueueSend(xQueueLCD, &command, portMAX_DELAY); // send the mode of the clock

    xTaskCreatePinnedToCore(vTaskSyncNTP,"NTPSync", 4096, NULL, 1, NULL, 0); // start the wifi and sync the wifi as soon as possible
    xEventGroupSetBits(xKlokEventgroup, SYNCTIME);
    vTaskDelay(pdMS_TO_TICKS(10));

    xTaskCreatePinnedToCore(vTaskTimer, "TimingTask", 2048, NULL, 1, &xTimerTaskHandle, 0); //start the timer

    initButtonInterrupt();
    xTaskCreatePinnedToCore(vTaskLoopModeButton,"changeModeButton",usStackDepthModeSwitchButton, NULL, modeSwitchButtonPriority, NULL, 0); // initialize en start the button
    
    xTaskCreatePinnedToCore(vTaskDisplayBeat,"7SegDis", 2048, NULL, 1, NULL,0); // initialise and start the 7SegDisplay
    xTaskCreatePinnedToCore(vTaskReadRotary, "RotaryTask", 4096, NULL, 1, NULL,0);// initialise and use the rotary encoder
    xTaskCreatePinnedToCore(vTaskDisplayCentibeat,"stepper", 4096, NULL, 1, NULL, 1);// initialise and use the steppermotor. set to its own core because the time intesive task
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
        beatDisplay.displayBeat(localCentibeat); // always display the beat
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
    TIMER centibeatTimer = TIMER();
    for (;;)
    {
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY); // wait until timer gives signal
        //run the task
        if(xSemaphoreTake(xMutexCentibeat, portMAX_DELAY)== pdTRUE) //get the mutex to change centibeat
        {
            centibeatCount++;
            if(centibeatCount > MAXCENTIBEATS)
            {
                centibeatCount = 0;
            }
            if (centibeatCount == TIMTETOSYNC)
            {
                xEventGroupSetBits(xKlokEventgroup, SYNCTIME);// every hundred beats sync with ntp
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
                    uint8_t command = automaticMode ? AUTOMODE : MANUAL;
                    xQueueSend(xQueueLCD, &command, portMAX_DELAY);       // depending on the automaticMode bool the command will be AUTOMODE or MANUAL
                    if(automaticMode)
                    {
                        xEventGroupSetBits(xKlokEventgroup, SYNCTIME);
                    }
                }
            }
        }
    }   
}

static void vTaskSyncNTP(void* pvParameters)
{
    WIFI wifi = WIFI("iotroam", "N4B4RiiNFg");
    wifi.iotroam_connect(); 
    for (;;)
    {
        xEventGroupWaitBits(xKlokEventgroup, SYNCTIME, pdTRUE, pdFALSE, portMAX_DELAY); // wait for flag to get time from SNTP
        if(xSemaphoreTake(xMutexCentibeat, portMAX_DELAY)== pdTRUE) //get semaphore for protected writing
        {
            centibeatCount = wifi.getTime();// sync centibeat time with SNTP
            xSemaphoreGive(xMutexCentibeat);
        }
    }
}
static void vTaskReadRotary(void *pvParameters)
{
    RotationDirection dir; 
    Rotary_Enc RotEnc = Rotary_Enc();
    for (;;)
    {
        if (!automaticMode) // only in manual mode
        {
            if (xQueueReceive(RotEnc.rotationQueue, &dir, portMAX_DELAY)) // when there is a rotation in the queue
            {       
                if(xSemaphoreTake(xMutexCentibeat, portMAX_DELAY) == pdTRUE ) // when the semaphore is available
                {                    
                    switch (dir)         // Check whether rotary encoder was turned CW or CCW
                    {
                        case CLOCKWISE:
                            ESP_LOGI("RotaryEncoder", "Rotated CW");
                            centibeatCount++;
                            if(centibeatCount>= MAXCENTIBEATS)// when a day is over set to zero
                            {
                                centibeatCount = 0;
                            }
                            break;
                        case COUNTERCLOCKWISE:
                            ESP_LOGI("RotaryEncoder", "Rotated CCW");
                            centibeatCount--;
                            if(centibeatCount>= MAXCENTIBEATS) // because if a unint 0-1 = max 
                            {
                                centibeatCount = MAXCENTIBEATS;
                            }
                            break;
                        default:
                            break;
                    }
                    xSemaphoreGive(xMutexCentibeat);
                }
            }
        }      
    }
}
static void vTaskPrintLCD(void* pvParameters)
{
    uint8_t recieved = 0;
    LCD lcd = LCD(); //init LCD to use only in this task
    for(;;)
    {
        if(xQueueReceive(xQueueLCD, &recieved, portMAX_DELAY)) // when a command is in the queue
        {
            lcd.sendComannd(recieved); // send command into the LCD
        }
    }
}
static void vTaskInitKlok(void* pvParameters)
{
    Stepmotor centi = Stepmotor(); // initialise stepmotor to use within initialisation phase

    uint8_t command = INIT;
    xQueueSend(xQueueLCD, &command, portMAX_DELAY); // send the init command to the display

    SegDis beat = SegDis();
    beat.displayBeat(0); // Turn on the display on 0

    Rotary_Enc rotEnc = Rotary_Enc(); // use the rotary encoder to turn the stepmotor
    RotationDirection dir; 
    motorRotation rotation = Idle; //to prevent the motor from starting
    
    for(;;)
    {
        if(xQueueReceive(rotEnc.rotationQueue, &dir, portMAX_DELAY)) // read queue
        {
            switch (dir)
                    {
                        case CLOCKWISE:
                            rotation = Forward; // when turning clock wise set rotation to Forward
                            break;
                        case COUNTERCLOCKWISE:
                            rotation = Backward; // when turning counterclock wise set rotation to Backwards
                            break;
                        default:
                            break;
            }
            centi.moveStepMotor(ONESTEP, rotation);// Set the step in the chosen rotation
        }
    }
}