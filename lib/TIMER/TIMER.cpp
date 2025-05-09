#include "TIMER.h"

extern uint16_t count;

void centibeat_timer_callback(void *param)
{
    count++;
    if (count == 10000) // 100 beats
    {
        //NTP CODE
    }
}



TIMER::TIMER()
{
    TIMER::centibeat_timer_init();
}

void TIMER::centibeat_timer_init()
{
    const esp_timer_create_args_t centibeat_timer_args = 
    {
        .callback = &centibeat_timer_callback,
        .name = "Centibeat timer Interrupt",
    };

    esp_timer_handle_t centibeat_timer_handler;
    esp_timer_create(&centibeat_timer_args, &centibeat_timer_handler);
    esp_timer_start_periodic(centibeat_timer_handler, 864000);  // One Second = 1000000 micro second
}