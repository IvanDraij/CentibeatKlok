#include "LCD.h"


LCD::LCD()
{
    lcd_init();
    lcd_clear();
}
void LCD ::printStr(char str[], uint8_t row, uint8_t col)
{
    lcd_put_cursor(row, col); // set cursor on chosen point
    lcd_send_string(str);     // set text on screen
}

void LCD::sendComannd(uint8_t cmd)
{
    switch (cmd)
    {
    case CONNECTING:
        printStr("Connecting  ", 0, 0); // set connecting on left topside of screen
        break;
    case CONNECTED:
        printStr("Connected   ", 0, 0);
        break;
    case SYNCED:
        printStr("Synced", 1, 0); // set synced on left topsid of the screen
        break;
     case AUTOMODE:
        printStr("A",0,14);         // set an A in the right top corner
        break;
    case MANUAL:
        printStr("M",0,14);
        break;
    case INIT:
        printStr("Turn hand to 0", 0, 1);
        printStr("Press to confirm", 1, 0);
        break;
    case FAILED:
        printStr("Failed      ", 0, 0);
        break;
    case CLEAR:
        lcd_clear();
        break;
    }
    
}

void LCD::printInt(uint32_t data, uint8_t row, uint8_t col){
    char izard[8];
    sprintf(izard, "%lu", data);
    printStr(izard, row, col);
}
// het uitwerken van verschillende senarios
// zoals het verbinden
// het synchroniseren
// verwisselen van modus
