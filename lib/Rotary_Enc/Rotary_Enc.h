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
#define HALF_A_SECOND (pdMS_TO_TICKS(50))

#define ROTARY_A_GPIO GPIO_NUM_34 // S1 (CLK) on D34
#define ROTARY_B_GPIO GPIO_NUM_35 // S2 (DT) on D35
#define FIFTYMS 50000

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

private:
  gpio_num_t buttonPin;
  QueueHandle_t rotationQueue;
  uint64_t lastRotationTime = 0; // For debouncing rotations
  void initRotaryEnc();
  static void isrHandlerButton(void *arg);
  static void isrHandlerRotation(void *arg);
  static void taskEntryPointButton(void *arg);
  static void taskEntryPointRotation(void *arg);
};

#endif