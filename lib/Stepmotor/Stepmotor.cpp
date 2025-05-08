#include "Stepmotor.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "LCD.h"

// Define GPIO pins for ULN2003
#define IN1 GPIO_NUM_19
#define IN2 GPIO_NUM_18
#define IN3 GPIO_NUM_17
#define IN4 GPIO_NUM_16

// 3.6 graden (1/100e van 360) dus 1 centibeat
#define STEPS_PER_REV 5.12
#define STEP_DELAY_MS 10



Stepmotor ::Stepmotor()
{
  initStepmotor();
  lastAnalog = 0;
}

uint8_t step_sequence_forward[4][4] = {
    {1, 1, 0, 0},
    {0, 1, 1, 0},
    {0, 0, 1, 1},
    {1, 0, 0, 1},
};

uint8_t step_sequence_backward[4][4] = {
    {0, 0, 1, 1},
    {0, 1, 1, 0},
    {1, 1, 0, 0},
    {1, 0, 0, 1},
};

uint32_t Stepmotor ::getCurrentAnalog(uint32_t centibeat)
{
  LCD lcd = LCD();
  // Bereken de nieuwe stand waar de stappenmotor op moet komen te staan
  uint32_t newAnalogRaw = centibeat % 100;

  // Haal de oude stand van de nieuwe stand af om te bepalen hoeveel stappen de motor moet maken.
  int32_t numberOfSteps = newAnalogRaw - lastAnalog;
  int newstuff = numberOfSteps;
  char car[5];
  sprintf(car, "%d", newstuff);
  lcd.printStr(car, 1, 0);

  // Zet de laatst gemeten analoge stand gelijk aan de raw output van de modulo, dat is de stand waar de klok heen loopt
  lastAnalog = newAnalogRaw;

  if (numberOfSteps < 0)
  {
    numberOfSteps = numberOfSteps * -1;
  }

  return numberOfSteps;
}

void Stepmotor ::moveStepMotor(uint8_t numberOfSteps)
{
  uint8_t (*step_sequence)[4];
  // If statement om te bepalen welke kant het meest efficient is om heen te draaien
  if (numberOfSteps < 51)
  {
    // Motor rechtsom
    step_sequence = step_sequence_forward;
  }
  else
  {
    // Motor linksom
    step_sequence = step_sequence_backward;

    // Haal de hoeveelheid stappen van 100 af om de juiste positie te krijgen
    numberOfSteps = 100 - numberOfSteps;
  }

  for (int i = 0; i < (float)(STEPS_PER_REV * numberOfSteps); i++)
  {
    for (int j = 0; j < 4; j++)
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
  // Initialisation of gpio pins used for the stepmotor
  gpio_config_t io_conf =
      {
          .pin_bit_mask = (1ULL << IN1) | (1ULL << IN2) | (1ULL << IN3) | (1ULL << IN4),
          .mode = GPIO_MODE_OUTPUT,
          .pull_up_en = GPIO_PULLUP_DISABLE,
          .pull_down_en = GPIO_PULLDOWN_DISABLE,
          .intr_type = GPIO_INTR_DISABLE};
  gpio_config(&io_conf);
}
