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
        printStr("Connecting", 1, 6); // set connecting on left underside of screen
        break;
    case CONNECTED:
        printStr("Connected", 1, 7); // set connected on left underside of screen IS NOG TE BEPALEN IN SCHERMONTWERPEN
        break;
    case SYNCED:
        printStr("Synced", 1, 8); // set synced on left bottomside of the screen
        break;
    case SYNCING:
        printStr("Synching",1,8);
        break;
    case AUTOMODE:
        printStr("A",0,14);         // set an A in the right top corner
        break;
    case MANUAL:
        printStr("M",0,14);
        break;
    case SYNCFAIL:
        printStr("Sync failed",1,0);
        break;
    case SETTINGTIME:
        printStr("SETTING TIME",1,0);
        break;
    case TIMESET:
        printStr("TIME SET",1,0);
        break;
    }
}

void LCD::printInt(uint8_t data, uint8_t row, uint8_t col){
    char izard[8];
    sprintf(izard, "%d", data);
    printStr(izard, row, col);
}
// het uitwerken van verschillende senarios
// zoals het verbinden
// het synchroniseren
// verwisselen van modus
