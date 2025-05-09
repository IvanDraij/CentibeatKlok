extern "C"
{
#include <stdio.h>
#include "driver/i2c.h"
#include "LCD.h"
#include "Stepmotor.h"
}

extern "C" void app_main(void)
{
    LCD lcd = LCD(); // Clear the LCD screen
    lcd.printStr("Je moder", 1, 0);
    Stepmotor motor = Stepmotor();
    motor.moveStepMotor(motor.getCurrentAnalog(99), motor.Forward);
}
