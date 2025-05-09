#ifndef TIMER_H
#define TIMER_H

extern "C"
{
    #include "esp_timer.h"
}


#ifdef __cplusplus
class TIMER
{
    public:
        TIMER(); //Constructor
        void centibeat_timer_init(void);
    protected:

    private:
};
#endif // __cplusplus

#endif /* TIMER_H */
