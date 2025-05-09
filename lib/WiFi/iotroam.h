#ifndef IOTROAM_H
#define IOTROAM_H

#include <stdint.h>
#include "LCD.h"
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "esp_netif.h"

#define WIFI_SSID "iotroam"
#define WIFI_PASS "N4B4RiiNFg"

#define WIFI_CONNECTED_BIT BIT0

class iotroam
{
    public:
        iotroam(LCD screen);
        LCD lcd;
        
    protected:

    private:
    static EventGroupHandle_t wifi_event_group;
};

#endif /* IOTROAM_H */
