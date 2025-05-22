#ifndef TIMER_H
#define TIMER_H

extern "C"
{
    #include "esp_timer.h"
    #include "freertos/FreeRTOS.h"
    #include "freertos/event_groups.h"
}


#ifdef __cplusplus
class TIMER
{
    public:
        TIMER(EventGroupHandle_t *xEventGroup); //Constructor
        void centibeat_timer_init(void);
    protected:

    private:

};
#endif // __cplusplus

#endif /* TIMER_H */
