#ifndef SEGDIS
#define SEGDIS

#include <stdio.h>
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"

#define PINA GPIO_NUM_27
#define PINB GPIO_NUM_32
#define PINC GPIO_NUM_33
#define PIND GPIO_NUM_13
#define PINE GPIO_NUM_14
#define PINF GPIO_NUM_4
#define PING GPIO_NUM_2
#define PIND1 GPIO_NUM_23
#define PIND2 GPIO_NUM_5
#define PIND3 GPIO_NUM_25
#define PIND4 GPIO_NUM_26

#define NUMBERS 10
#define SEGMENTS 7
#define DISPLAYS 4
#define BRIGHTNESSDELAY 5
#define CENTIBEATTOBEAT 100
#define TEN 10
#define DISPLAYON 0
#define DISPLAYOFF 1
#define LASTDISPLAY 3


const bool numberDisplay[NUMBERS][SEGMENTS]
{
    {1, 1, 1, 1, 1, 1, 0 }, //0
    {0, 1, 1, 0, 0, 0, 0 }, //1
    {1, 1, 0, 1, 1, 0, 1 }, //2
    {1, 1, 1, 1, 0, 0, 1 }, //3
    {0, 1, 1, 0, 0, 1, 1 }, //4
    {1, 0, 1, 1, 0, 1, 1 }, //5
    {1, 0, 1, 1, 1, 1, 1 }, //6
    {1, 1, 1, 0, 0, 0, 0 }, //7
    {1, 1, 1, 1, 1, 1, 1 }, //8
    {1, 1, 1, 1, 0, 1, 1 }  //9
};

static const gpio_num_t segmentPins[SEGMENTS] = {
    PINA, PINB, PINC, PIND, PINE, PINF, PING
};

static const gpio_num_t displayPins[DISPLAYS] = {
    PIND1, PIND2, PIND3, PIND4
};

class SegDis
{
private:
    void SegInit();
    void displayNumber(uint8_t number, uint8_t display);
public:
    SegDis();
    void displayBeat(uint32_t centibeat);
    
};

#endif
