#include "Rotary_Enc.h"

Rotary_Enc ::Rotary_Enc()
{
  initRotaryEnc(); // Call the initialisation
}

int8_t Rotary_Enc ::consumeSteps()
{
  int temp = stepsToTake;
  stepsToTake = 0;
  return temp;
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
  Rotary_Enc *self = static_cast<Rotary_Enc *>(arg); // cast Object into static class so it's accessable through RTOS
  BaseType_t xHigherPriorityTaskWoken = pdFALSE;     // Initialize higher priority task on false

  // Read the current levels of rotary encoder pins
  uint8_t signalA = gpio_get_level(ROTARY_A_GPIO);
  uint8_t signalB = gpio_get_level(ROTARY_B_GPIO);

  // Convert signals into a 2-bit value (values: 0b00 to 0b11)
  uint8_t currentState = (signalA << 1) | signalB;

  // Combine previous and current state into a 4-bit transition code, this is used to check if there has been a change in rotary encoder stance.
  uint8_t transitionCode = (self->prevABState << 2) | currentState;

  // Update the previous state for the next interrupt
  self->prevABState = currentState;

  // Look up the movement (+1, -1, or 0) using the transition code
  int8_t movement = self->encoder_state_table[transitionCode];

  if (movement != 0)
  {
    // Accumulate movement steps
    self->positionCounter += movement;

    // If we've collected 4 steps in one direction, it's a full tick
    if (self->positionCounter >= 4)
    {
      self->positionCounter = 0; // Reset counter
      RotationDirection dir = COUNTERCLOCKWISE;
      xQueueSendFromISR(self->rotationQueue, &dir, &xHigherPriorityTaskWoken);
    }
    else if (self->positionCounter <= -4)
    {
      self->positionCounter = 0; // Reset counter
      RotationDirection dir = CLOCKWISE;
      xQueueSendFromISR(self->rotationQueue, &dir, &xHigherPriorityTaskWoken);
    }
  }
  // If a higher-priority task was woken by the queue send, yield to it
  if (xHigherPriorityTaskWoken)
  {
    portYIELD_FROM_ISR();
  }
}

void Rotary_Enc ::taskEntryPointButton(void *arg) // Function to initialise the loop function for RTOS
{
  static_cast<Rotary_Enc *>(arg)->taskLoopButton(); // Cast the Object made in main to the static class and run taskLoop within that task
}

void Rotary_Enc::taskEntryPointRotation(void *arg) // Function to initialise the loop function for RTOS
{
  static_cast<Rotary_Enc *>(arg)->taskLoopRotation(); // Cast the Object made in main to the static class and run taskLoop within that task
}

void Rotary_Enc::taskLoopButton()
{
  while (true)
  {
    if (xSemaphoreTake(buttonSemaphore, portMAX_DELAY))
    {
      gpio_intr_disable(BUTTON_GPIO); // Disable ISR from button to get 0 debounce
      vTaskDelay(FIFTYMS);            // debounce
      if (gpio_get_level(BUTTON_GPIO) == 0)
      {
        ESP_LOGI("RotaryEncoderButton", "Button pressed");
        // Add RTOS semaphore here
      }
      gpio_intr_enable(BUTTON_GPIO); // Enable ISR
    }
    vTaskDelay(pdMS_TO_TICKS(10)); // Delay 10 ms to yield CPU
  }
}

void Rotary_Enc::taskLoopRotation()
{
  RotationDirection dir; // Initialising enum
  while (true)
  {
    if (xQueueReceive(rotationQueue, &dir, portMAX_DELAY))
    {
      // Check whether rotary encoder was turned CW or CCW
      switch (dir)
      {
      case CLOCKWISE:
        ESP_LOGI("RotaryEncoder", "Rotated CW");
        // Put more logic here (might have to change output of function)
        stepsToTake++;
        break;
      case COUNTERCLOCKWISE:
        ESP_LOGI("RotaryEncoder", "Rotated CCW");
        // Put more logic here (might have to change output of function)
        stepsToTake--;
        break;
      default:
        break;
      }
    }
    vTaskDelay(pdMS_TO_TICKS(10)); // Delay 10 ms to yield CPU
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
  gpio_isr_handler_add(ROTARY_A_GPIO, isrHandlerRotation, (void *)this); // Register ISR handler for rotary A
  gpio_isr_handler_add(ROTARY_B_GPIO, isrHandlerRotation, (void *)this); // Register ISR handler for rotary B

  xTaskCreate(taskEntryPointButton, "RotaryBtnTask", 2048, this, 10, NULL);   // Create RTOS task for button
  xTaskCreate(taskEntryPointRotation, "RotaryRotTask", 2048, this, 10, NULL); // Create RTOS task for rotary
}