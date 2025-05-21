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
#include <time.h>
#include <sys/time.h>
#include "esp_sntp.h"
#include <math.h>

// Event group bits for Wi-Fi connection status
#define WIFI_SUCCESS 1 << 0
#define WIFI_FAILURE 1 << 1
#define MAX_FAILURES 10 // Max number of reconnect attempts before giving up
#define CURRENTYEAR (2025 - 1900)
#define TWOSECONDS (pdMS_TO_TICKS(2000))
#define CEST_CORRECTION 2
#define CENTIBEAT_MULTIPLIER 0.864
#define SECONDS_IN_HOUR 3600
#define SECONDS_IN_MINUTE 60

void iotroam_init(const char *ssid, const char *password);
void iotroam_connect();
void iotroam_disconnect();

class WIFI
{
private:
  void initSNTP();
  char strftime_buf[64];

public:
  WIFI();
  uint32_t getTime();
};

#endif // IOTROAM_H
