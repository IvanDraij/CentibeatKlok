#include "7SegDis.h"

SegDis::SegDis()
{
    SegInit();
} 

void SegDis::displayNumber(uint8_t number,uint8_t display)
{
    static uint8_t prevDis = 0;// keeps track of the last display
    gpio_set_level(displayPins[prevDis], 1);// turns off the last display
    gpio_set_level(displayPins[display], 0); // turns on the new display
    for (uint8_t i = 0; i < SEGMENTS; i++)
    {
        gpio_set_level(segmentPins[i], numberDisplay[number][i]); //sets segment on or off depending on the numbers
    }
    prevDis = display; // save the current display in the last display
}

void SegDis::SegInit()
{
    gpio_config_t io_conf =
      {
          .pin_bit_mask = (1ULL << PINA) | (1ULL << PINB) | (1ULL << PINC) | (1ULL << PIND) | (1ULL << PINE) |(1ULL << PINF) |(1ULL << PING) | //set all segement and display pins on output
                          (1ULL << PIND1) |(1ULL << PIND2) | (1ULL << PIND3) | (1ULL << PIND4),
          .mode = GPIO_MODE_OUTPUT,
          .pull_up_en = GPIO_PULLUP_DISABLE,
          .pull_down_en = GPIO_PULLDOWN_DISABLE,
          .intr_type = GPIO_INTR_DISABLE
        };
  gpio_config(&io_conf);
  for (uint8_t i = 0; i < DISPLAYS; i++)
  {
    gpio_set_level(displayPins[i], 1); // turn off all displays
  }
}
