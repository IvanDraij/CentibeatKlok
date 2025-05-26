#include "Stepmotor.h"
#include "esp_task_wdt.h"

Stepmotor ::Stepmotor()
{
  initStepmotor();
}

uint8_t step_sequence_forward[AMOUNT_OF_STEPS][AMOUNT_OF_COILS] = {
    {1, 1, 0, 0},
    {0, 1, 0, 0},
    {0, 1, 1, 0},
    {0, 0, 1, 0},
    {0, 0, 1, 1},
    {0, 0, 0, 1},
    {1, 0, 0, 1},
    {1, 0, 0, 0},
};

uint8_t step_sequence_backward[AMOUNT_OF_STEPS][AMOUNT_OF_COILS] = {
    {0, 0, 1, 1},
    {0, 0, 1, 0},
    {0, 1, 1, 0},
    {0, 1, 0, 0},
    {1, 1, 0, 0},
    {1, 0, 0, 0},
    {1, 0, 0, 1},
    {0, 0, 0, 1},
};

uint32_t totalStepsTakenInADay = 0;
uint8_t rotationPerCentibeat;

uint8_t Stepmotor ::calculateSteps(uint32_t centibeat)
{
  // This function determines howmany steps must be taken to reach the wanted position
  // The function does this by calculating it with the total amount of centibeats

  // Calculate the right position of the clock by taking the total amount of centibeats and doing modulo 100 to only get the numbers below 100.
  uint8_t newClockPosVal = centibeat % MAXSTEPS;

  // To determine howmany steps must be taken, subtract the last value from the current value.
  int8_t numberOfSteps = newClockPosVal - previousClockPosVal;

  // Put the new raw value into the last logged value.
  previousClockPosVal = newClockPosVal;

  // If numberOfSteps is smaller than 0, that means the previous amount of centibeat > current amount. Substract the negative number from 100.
  // To get the right amount of steps
  if (numberOfSteps < 0)
  {
    numberOfSteps = MAXSTEPS + numberOfSteps;
  }

  return numberOfSteps;
}

void Stepmotor::moveStepMotor(uint8_t numberOfSteps, uint8_t motorRotation)
{
  uint8_t (*step_sequence)[AMOUNT_OF_COILS]; // Change the sequence array to the desired rotation
  if (motorRotation == Forward)
  {
    step_sequence = step_sequence_forward;
  }
  else
  {
    step_sequence = step_sequence_backward;
  }

  static int sequenceIndex = 0; // Index to keep track of what the last step in the sequence was

  for (int step = 0; step < numberOfSteps; step++) // Loop to go through the total amount of steps needed
  {
    esp_task_wdt_reset();
    totalStepsTakenInADay++;
    if (totalStepsTakenInADay != 0 && ((totalStepsTakenInADay % MAXSTEPS) % MODULO25 == 0)) // Using modulo 25 to deteremine if the amount of steps is dividable by 25
    {
      rotationPerCentibeat = (AMOUNT_OF_INNER_ROTATION_PER_CENTIBEAT - STEPREDUCTION); // If it's step 25, remove 8 steps to keep the motor running accurately
    }
    else
    {
      rotationPerCentibeat = AMOUNT_OF_INNER_ROTATION_PER_CENTIBEAT;
    }

    if (totalStepsTakenInADay == ONEDAY) // When the stepcounter reaches 100000, reset it to 0
    {
      totalStepsTakenInADay = 0;
    }

    for (int i = 0; i < rotationPerCentibeat; i++) // Loop to take 1 inner step within the stepmotor
    {
      esp_task_wdt_reset(); // reset watchdog
      gpio_set_level(IN1, step_sequence[sequenceIndex][0]);
      gpio_set_level(IN2, step_sequence[sequenceIndex][1]);
      gpio_set_level(IN3, step_sequence[sequenceIndex][2]);
      gpio_set_level(IN4, step_sequence[sequenceIndex][3]);

      vTaskDelay(pdMS_TO_TICKS(STEP_DELAY_MS));

      sequenceIndex = (sequenceIndex + 1) % AMOUNT_OF_STEPS; // Adding 1 to the sequence, doing modulo 8 to keep the value beneath 8 at all times
    }
  }
}

void Stepmotor ::initStepmotor()
{
  // Initialisation of the pins used by the stepmotor
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