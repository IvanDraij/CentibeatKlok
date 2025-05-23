#include "7SegDis.h"

SegDis::SegDis()
{
    SegInit();
} 

void SegDis::displayNumber(uint8_t number,uint8_t display)
{
    static uint8_t prevDis = 0; // keeps track of the previous display
    gpio_set_level(displayPins[prevDis], DISPLAYOFF);    // turns off the previous display
    
    for (uint8_t i = 0; i < SEGMENTS; i++) // set segments to corresonding number
    {
        gpio_set_level(segmentPins[i], numberDisplay[number][i]); 
    }
    gpio_set_level(displayPins[display], DISPLAYON); // turns on the new display
    prevDis = display; // save the current display in the last display
}

void SegDis::SegInit()
{
    gpio_config_t io_conf =
      {
          .pin_bit_mask = (1ULL << PINA) | (1ULL << PINB) | (1ULL << PINC) | (1ULL << PIND) | (1ULL << PINE) |(1ULL << PINF) |(1ULL << PING) | //set all segement and display pins on output
                          (1ULL << PIND1) | (1ULL << PIND2) | (1ULL << PIND3) | (1ULL << PIND4),
          .mode = GPIO_MODE_OUTPUT,
          .pull_up_en = GPIO_PULLUP_DISABLE,
          .pull_down_en = GPIO_PULLDOWN_DISABLE,
          .intr_type = GPIO_INTR_DISABLE
        };
  gpio_config(&io_conf);
  for (uint8_t i = 0; i < DISPLAYS; i++)
  {
    gpio_set_level(displayPins[i], DISPLAYOFF); // turn off all displays
  }
}
void SegDis::displayBeat(uint32_t centibeat)
{
    uint32_t beats = centibeat / CENTIBEATTOBEAT; // to extract beat from centibeats
    for (uint8_t i = 0; i < DISPLAYS; i++)
    {
        if(beats > 0) //when the number is done stop
        {
            displayNumber(beats % TEN, LASTDISPLAY-i); // begin at the last display
            beats /= TEN; // devide by ten to get the next: tens hundreds and thousands
            vTaskDelay(pdMS_TO_TICKS(BRIGHTNESSDELAY)); // small delay to get the brghtness up
        }
    }
}
