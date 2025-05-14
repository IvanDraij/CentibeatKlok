#ifndef MY_STEPMOTOR_H
#define MY_STEPMOTOR_H
#include <stdint.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "LCD.h"

// Define GPIO pins for ULN2003
#define IN1 GPIO_NUM_19
#define IN2 GPIO_NUM_18
#define IN3 GPIO_NUM_17
#define IN4 GPIO_NUM_16

#define STEPS_PER_CENTIBEAT 5.12 // Full 360 of the clock takes 512 steps, A full 360 is a beat, to get 1 centibeat: 512/100 = 5.12
#define STEP_DELAY_MS 10
#define AMOUNT_OF_COILS 4

class Stepmotor
{
private:
    uint8_t calculateSteps(uint32_t);
    void initStepmotor();
    void moveStepMotor(uint8_t, uint8_t);
    enum Mode
    {
        Forward,
        Backward
    };

public:
    Stepmotor();
    uint32_t lastAnalog;
    void moveStepMotorToCentibeat(uint32_t);
};
#endif
