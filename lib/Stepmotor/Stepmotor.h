#ifndef MY_STEPMOTOR_H
#define MY_STEPMOTOR_H
#include <stdint.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"

// Define GPIO pins for ULN2003
#define IN1 GPIO_NUM_19
#define IN2 GPIO_NUM_18
#define IN3 GPIO_NUM_17
#define IN4 GPIO_NUM_16

#define STEP_DELAY_MS 10
//360 degrees is 512 steps, 1 centibeat = 512/100 = 5,12 steps. Motor is on half step so there's an accuracy of 1/8 = 0,125. 5,12/0,125 = 40,96. Rounded up it's 41.
#define AMOUNT_OF_INNER_ROTATION_PER_CENTIBEAT 41
#define AMOUNT_OF_STEPS 8 //For half step mode
#define AMOUNT_OF_COILS 4 //Amount of coils in ROHS 28BYJ48
#define MAXSTEPS 100 //Max amount of steps for the stepmotor to take + 1.
#define MODULO25 25

class Stepmotor
{
private:
    static uint8_t sequenceIndex;
    uint32_t totalStepsTakenInADay;
    uint8_t calculateSteps(uint32_t);
    void initStepmotor();
    void moveStepMotor(uint8_t, uint8_t);
    enum motorRotation
    {
        Forward,
        Backward
    };
    uint8_t previousClockPosVal = 0;


public:
    Stepmotor();
    void moveStepMotorToCentibeat(uint32_t);
    uint8_t rotationPerCentibeat;
};
#endif
