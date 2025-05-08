extern "C"
{
#include <stdio.h>
#include "driver/i2c.h"
#include "LCD.h"
}

extern "C" void app_main(void)
{
    LCD lcd = LCD(); // Clear the LCD screen
    lcd.printStr("Je moder", 1, 0);
}
