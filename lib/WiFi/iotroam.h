#ifndef IOTROAM_H
#define IOTROAM_H

#include "esp_err.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_log.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "nvs_flash.h"
#include <string.h>
#include "LCD.h"

// Event group bits for Wi-Fi connection status
#define WIFI_SUCCESS 1 << 0
#define WIFI_FAILURE 1 << 1
#define MAX_FAILURES 10  // Max number of reconnect attempts before giving up

void iotroam_init(const char *ssid, const char *password);
void iotroam_connect();
void iotroam_disconnect();

#endif // IOTROAM_H
