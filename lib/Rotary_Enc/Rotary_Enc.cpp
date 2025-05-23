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
  Rotary_Enc *self = static_cast<Rotary_Enc *>(arg); // cast Object into static class so it's accessable through RTOS
  BaseType_t xHigherPriorityTaskWoken = pdFALSE;     // Initialize higher priority task on false

  // Step 1: Read the current state of the rotary encoder pins
  uint8_t signalA = gpio_get_level(ROTARY_A_GPIO); // Reads 0 or 1
  uint8_t signalB = gpio_get_level(ROTARY_B_GPIO); // Reads 0 or 1

  // Step 2: Determine the current position of the encoder as a number between 0 and 3
  // The combination of A and B gives us 4 possible states: 0 (00), 1 (01), 2 (10), 3 (11)
  uint8_t currentPosition = 0;

  if (signalA == 0 && signalB == 0)
    currentPosition = 0;
  else if (signalA == 0 && signalB == 1)
    currentPosition = 1;
  else if (signalA == 1 && signalB == 0)
    currentPosition = 2;
  else if (signalA == 1 && signalB == 1)
    currentPosition = 3;

  // Step 3: Combine previous and current positions to figure out movement
  // This creates a unique number between 0 and 15 that represents the transition
  int transitionCode = (self->prevABState * 4) + currentPosition;

  // Step 4: Save current position for next time
  self->prevABState = currentPosition;

  // Step 5: Check how the encoder moved using the transition code
  int movement = self->encoder_state_table[transitionCode]; // Result will be -1, 0, or +1

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