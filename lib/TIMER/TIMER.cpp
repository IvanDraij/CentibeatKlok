#include "TIMER.h"
#define amountOfmsForOneCentibeat   864000
#define hunderBeats                 10000

extern uint16_t count;

void centibeat_timer_callback(void *param) // task to count the centibeats (not an interrupt)
{
    count++;
    if (count == hunderBeats) // 10000 centibeats
    {
        //NTP CODE
    }
}



TIMER::TIMER() // constructor that calls the init function
{
    TIMER::centibeat_timer_init();
}

void TIMER::centibeat_timer_init() //initialises the timer for the centibeat
{
    const esp_timer_create_args_t centibeat_timer_args = 
    {
        .callback = &centibeat_timer_callback,
        .name = "Centibeat timer Interrupt",
    };

    esp_timer_handle_t centibeat_timer_handler;
    esp_timer_create(&centibeat_timer_args, &centibeat_timer_handler);
    esp_timer_start_periodic(centibeat_timer_handler, amountOfmsForOneCentibeat);  // One Second = 1000000 micro second
}