#include "7SegDis.h"

SegDis::SegDis()
{
    SegInit();
} 

void SegDis::displayNumber(uint8_t number,uint8_t display)
{
    static uint8_t prevDis = 0;
    gpio_set_level(displayPins[prevDis], 1);
    gpio_set_level(displayPins[display], 0);
    for (uint8_t i = 0; i < NUMBERS; i++)
    {
        gpio_set_level(segmentPins[i], numberDisplay[number][i]);
    }
    prevDis = display;
}

void SegDis::SegInit()
{
    gpio_config_t io_conf =
      {
          .pin_bit_mask = (1ULL << PINA) | (1ULL << PINB) | (1ULL << PINC) | (1ULL << PIND) | (1ULL << PINE) |(1ULL << PINF) |(1ULL << PING) |
                          (1ULL << PIND1) |(1ULL << PIND2) | (1ULL << PIND3) | (1ULL << PIND4),
          .mode = GPIO_MODE_OUTPUT,
          .pull_up_en = GPIO_PULLUP_DISABLE,
          .pull_down_en = GPIO_PULLDOWN_DISABLE,
          .intr_type = GPIO_INTR_DISABLE
        };
  gpio_config(&io_conf);
  for (uint8_t i = 0; i < DISPLAYS; i++)
  {
    gpio_set_level(displayPins[i], 1);
  }
  
}