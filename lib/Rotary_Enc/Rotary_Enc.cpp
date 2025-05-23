#include "Rotary_Enc.h"

Rotary_Enc ::Rotary_Enc()
{
  initRotaryEnc(); // Call the initialisation
}

void Rotary_Enc::isrHandlerButton(void *arg)
{
  Rotary_Enc *self = static_cast<Rotary_Enc *>(arg);                       // Cast the object defined in main to the static class
  BaseType_t xHigherPriorityTaskWoken = pdFALSE;                           // Initialise a variable to check whether a task with high priority was waiting on the semaphore
  xSemaphoreGiveFromISR(self->buttonSemaphore, &xHigherPriorityTaskWoken); // Give the semaphore
  if (xHigherPriorityTaskWoken)
  {
    portYIELD_FROM_ISR(); // If a high priority task was waiting, immediately give way for the task.
  }
}

void Rotary_Enc::isrHandlerRotation(void *arg)
{
  Rotary_Enc *self = static_cast<Rotary_Enc *>(arg); // Cast object in class as static
  BaseType_t xHigherPriorityTaskWoken = pdFALSE;     // Clear higher priority found

  // Read out both pins
  uint8_t a = gpio_get_level(ROTARY_A_GPIO);
  uint8_t b = gpio_get_level(ROTARY_B_GPIO);

  RotationDirection dir; // Initialising enum

  // Check which way to turn
  if (a == b)
    dir = COUNTERCLOCKWISE;
  else
    dir = CLOCKWISE;

  xQueueSendFromISR(self->rotationQueue, &dir, &xHigherPriorityTaskWoken); // Add to queue using RTOS

  if (xHigherPriorityTaskWoken) // If task with higher priority has awoken, immediately go to that task
    portYIELD_FROM_ISR();
}

void Rotary_Enc ::taskEntryPointButton(void *arg) //Function to get from ISR to task
{
  static_cast<Rotary_Enc *>(arg)->taskLoopButton(); // Cast the Object made in main to the static class and run taskLoop within that task
}

void Rotary_Enc::taskEntryPointRotation(void *arg) //Function to get from ISR to task
{
  static_cast<Rotary_Enc *>(arg)->taskLoopRotation(); // Cast the Object made in main to the static class and run taskLoop within that task
}

void Rotary_Enc::taskLoopButton()
{
  while (true)
  {
    if (xSemaphoreTake(buttonSemaphore, portMAX_DELAY)) // Take semaphore
    {
      if (gpio_get_level(BUTTON_GPIO) == 0)
      {
        vTaskDelay(HALF_A_SECOND);            // Debounce so no double clicks happen
        if (gpio_get_level(BUTTON_GPIO) == 0) // Check whether button is still not pressed
        {
          ESP_LOGI("RotaryEncoderButton", "Button pressed");
          // Add more logic here
        }
      }
    }
  }
}

void Rotary_Enc::taskLoopRotation()
{
  RotationDirection dir; // Initialising enum
  uint64_t lastHandledTime = 0;
  while (true)
  {
    if (xQueueReceive(rotationQueue, &dir, portMAX_DELAY))
    {
      uint64_t now = esp_timer_get_time();  // Get time
      if (now - lastHandledTime >= FIFTYMS) // 50ms debounce
      {
        lastHandledTime = now; // Log lasthandled time in variable

        // Check whether rotary encoder was turned CW or CCW
        switch (dir)
        {
        case CLOCKWISE:
          ESP_LOGI("RotaryEncoder", "Rotated CW");
          // Put more logic here (might have to change output of function)
          break;
        case COUNTERCLOCKWISE:
          ESP_LOGI("RotaryEncoder", "Rotated CCW");
          // Put more logic here (might have to change output of function)
          break;
        default:
          break;
        }
      }
    }
  }
}

void Rotary_Enc ::initRotaryEnc()
{
  buttonSemaphore = xSemaphoreCreateBinary();                 // Create the semaphore
  rotationQueue = xQueueCreate(4, sizeof(RotationDirection)); // Create buffer for inputs of rotary encoder so no inputs go to waste

  // Configure BUTTON
  gpio_config_t btn_conf = {};
  btn_conf.intr_type = GPIO_INTR_NEGEDGE;        // Falling edge
  btn_conf.mode = GPIO_MODE_INPUT;               // Input mode
  btn_conf.pin_bit_mask = (1ULL << BUTTON_GPIO); // Which pins need initialising
  btn_conf.pull_up_en = GPIO_PULLUP_ENABLE;      // Enable pull-up resistor
  gpio_config(&btn_conf);

  // Configure ROTARY A (interrupt) and B (input)
  gpio_config_t rot_conf = {};
  rot_conf.intr_type = GPIO_INTR_ANYEDGE;                                    // Any edge for more accurate readings from rotary encoder
  rot_conf.mode = GPIO_MODE_INPUT;                                           // Input mode
  rot_conf.pin_bit_mask = (1ULL << ROTARY_A_GPIO) | (1ULL << ROTARY_B_GPIO); // Which pins need initialising
  rot_conf.pull_up_en = GPIO_PULLUP_ENABLE;                                  // Enable pull-up resistors
  gpio_config(&rot_conf);

  gpio_install_isr_service(0); // Install ISR service

  gpio_isr_handler_add(BUTTON_GPIO, isrHandlerButton, (void *)this);     // Register ISR handler for button
  gpio_isr_handler_add(ROTARY_A_GPIO, isrHandlerRotation, (void *)this); // Register ISR handler for rotary

  xTaskCreate(taskEntryPointButton, "RotaryBtnTask", 2048, this, 10, NULL);   // Create RTOS task for button
  xTaskCreate(taskEntryPointRotation, "RotaryRotTask", 2048, this, 10, NULL); // Create RTOS task for rotary
}