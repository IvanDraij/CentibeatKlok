#ifndef MY_ROTARY_ENC_H
#define MY_ROTARY_ENC_H

#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "esp_timer.h"

#define BUTTON_GPIO GPIO_NUM_39 // VN pin
#define FIFTYMS (pdMS_TO_TICKS(50))

#define ROTARY_A_GPIO GPIO_NUM_34 // S1 (CLK) on D34
#define ROTARY_B_GPIO GPIO_NUM_35 // S2 (DT) on D35
extern TaskHandle_t xRotEncTaskHandle;

enum RotationDirection
{
  NONE,
  CLOCKWISE,
  COUNTERCLOCKWISE
};

class Rotary_Enc
{
public:
  Rotary_Enc();
  void taskLoopButton();
  void taskLoopRotation();
  SemaphoreHandle_t buttonSemaphore;
  int8_t stepsToTake = 0; // Amount of steps stepmotor needs to take
  uint8_t automatic = 0;

private:
  gpio_num_t buttonPin;
  QueueHandle_t rotationQueue;
  uint8_t prevABState = 0;    // Track previous A/B state
  int8_t positionCounter = 0; // counts steps per detent
  const int8_t encoder_state_table[16] = {
      0, -1, 1, 0,
      1, 0, 0, -1,
      -1, 0, 0, 1,
      0, 1, -1, 0};
  void initRotaryEnc();
  static void isrHandlerButton(void *arg);
  static void isrHandlerRotation(void *arg);
  static void taskEntryPointButton(void *arg);
  static void taskEntryPointRotation(void *arg);
};

#endif