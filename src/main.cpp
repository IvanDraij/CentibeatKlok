extern "C"
{
#include <stdio.h>
#include "driver/i2c.h"
#include "LCD.h"

}
#include "iotroam.h"
extern "C" void app_main(void)
{
    LCD lcd = LCD(); // Clear the LCD screen
    iotroam_init("EspTestNetwork", "TestTest");
    iotroam_connect();
    vTaskDelay(pdMS_TO_TICKS(1000));
    //iotroam_disconnect();
    vTaskDelay(pdMS_TO_TICKS(1000));
    while(1){}
    return;
}
