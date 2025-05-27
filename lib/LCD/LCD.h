#ifndef LCD_H
#define LCD_H

#include <stdint.h>
#include <string.h>
#include <stdio.h>

extern "C"
{
    #include "i2c_lcd.h"
}

//comandos for text on screen
#define CONNECTING 0x01
#define CONNECTED 0x02
#define SYNCED 0x03
#define SYNCING 0x04
#define AUTOMODE 0x05
#define MANUAL 0x06
#define SYNCFAIL 0x07
#define SETTINGTIME 0x08
#define TIMESET 0x09

#ifdef __cplusplus
class LCD
{
    public:
        LCD(); //constructor
        void printStr(char str[], uint8_t row, uint8_t col); 
        void sendComannd(uint8_t cmd);
        void printInt(uint8_t data, uint8_t row, uint8_t col);
    protected:

    private:
};
#endif // __cplusplus

#endif /* LCD_H */
