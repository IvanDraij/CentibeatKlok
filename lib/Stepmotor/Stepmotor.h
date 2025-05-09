#ifndef MY_STEPMOTOR_H
#define MY_STEPMOTOR_H
#include <stdint.h>

class Stepmotor
{
private:
    
public:
    Stepmotor();
    uint32_t getCurrentAnalog(uint32_t);
    void initStepmotor();
    void moveStepMotor(uint8_t, uint8_t);
    uint32_t lastAnalog;
    enum Mode {
        Forward,
        Backward
    };
};
#endif
