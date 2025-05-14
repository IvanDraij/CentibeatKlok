#include "Stepmotor.h"

Stepmotor ::Stepmotor()
{
  initStepmotor();
  lastAnalog = 0;
}

uint8_t step_sequence_forward[AMOUNT_OF_COILS][AMOUNT_OF_COILS] = {
    {1, 1, 0, 0},
    {0, 1, 1, 0},
    {0, 0, 1, 1},
    {1, 0, 0, 1},
};

uint8_t step_sequence_backward[AMOUNT_OF_COILS][AMOUNT_OF_COILS] = {
    {0, 0, 1, 1},
    {0, 1, 1, 0},
    {1, 1, 0, 0},
    {1, 0, 0, 1},
};

uint8_t Stepmotor ::calculateSteps(uint32_t centibeat)
{
  // This function determines howmany steps must be taken to reach the wanted position
  // The function does this by calculating it with the total amount of centibeats

  // Calculate the right position of the clock by taking the total amount of centibeats and doing modulo 100 to only get the numbers below 100.
  uint8_t newClockPosVal = centibeat % maxSteps;

  // To determine howmany steps must be taken, subtract the last value from the current value.
  int8_t numberOfSteps = newClockPosVal - lastAnalog;

  // Put the new raw value into the last logged value.
  lastAnalog = newClockPosVal;

  // If numberOfSteps is smaller than 0, that means the previous amount of centibeat > current amount. Substract the negative number from 100.
  // To get the right amount of steps
  if (numberOfSteps < 0)
  {
    numberOfSteps = maxSteps + numberOfSteps;
  }

  return numberOfSteps;
}

void Stepmotor ::moveStepMotor(uint8_t numberOfSteps, uint8_t motorRotation)
{
  // Function for turning the motors, can both be counterclockwise and clockwise, the Enum will be used to determining the direction.

  uint8_t (*step_sequence)[AMOUNT_OF_COILS];
  // Determine whether the motor turns clockwise or not
  if (motorRotation == Forward)
  {
    // Motor clockwise
    step_sequence = step_sequence_forward;
  }
  else // This will be used when the clock needs to turn counterclockwise (for rotary encoder)
  {
    // Motor counterclockwise
    step_sequence = step_sequence_backward;
  }

  // Turning the motors
  for (int i = 0; i < (float)(STEPS_PER_CENTIBEAT * numberOfSteps); i++)
  {
    for (int j = 0; j < AMOUNT_OF_COILS; j++)
    {
      gpio_set_level(IN1, step_sequence[j][0]);
      gpio_set_level(IN2, step_sequence[j][1]);
      gpio_set_level(IN3, step_sequence[j][2]);
      gpio_set_level(IN4, step_sequence[j][3]);
      vTaskDelay(pdMS_TO_TICKS(STEP_DELAY_MS));
    }
  }
}

void Stepmotor ::initStepmotor()
{
  // Initialisatie van de pins gebruikt door de stappenmotor
  gpio_config_t io_conf =
      {
          .pin_bit_mask = (1ULL << IN1) | (1ULL << IN2) | (1ULL << IN3) | (1ULL << IN4),
          .mode = GPIO_MODE_OUTPUT,
          .pull_up_en = GPIO_PULLUP_DISABLE,
          .pull_down_en = GPIO_PULLDOWN_DISABLE,
          .intr_type = GPIO_INTR_DISABLE};
  gpio_config(&io_conf);
}

void Stepmotor ::moveStepMotorToCentibeat(uint32_t centibeat)
{
  moveStepMotor(calculateSteps(centibeat), Forward);
}