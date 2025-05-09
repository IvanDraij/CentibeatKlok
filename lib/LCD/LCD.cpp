#include "LCD.h"

LCD::LCD()
{
    lcd_init();
    lcd_clear();
}

void LCD ::printStr(char *str, uint8_t row, uint8_t col)
{
    lcd_put_cursor(row, col); // set cursor on chosen point
    lcd_send_string(str);     // set text on screen
}

void LCD::sendComannd(uint8_t cmd)
{
    switch (cmd)
    {
    case CONNECTING:
        printStr("Connecting", 1, 6); // set connecting on left underside of screen
        break;
    case CONNECTED:
        printStr("Connected", 1, 7); // set connected on left underside of screen IS NOG TE BEPALEN IN SCHERMONTWERPEN
        break;
    case SYNCED:
        printStr("Synced", 0, 8); // set synced on left topsid of the screen
        break;
    }
}
// het uitwerken van verschillende senarios
// zoals het verbinden
// het synchroniseren
// verwisselen van modus
