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
extern "C"
{
#include <time.h>
#include <sys/time.h>
#include "esp_log.h"
#include "esp_sntp.h"
#include <inttypes.h>
#include <math.h>
}

// Event group bits for Wi-Fi connection status
#define WIFI_SUCCESS 1 << 0
#define WIFI_FAILURE 1 << 1
#define MAX_FAILURES 10 // Max number of reconnect attempts before giving up

void iotroam_init(const char *ssid, const char *password);
void iotroam_connect();
void iotroam_disconnect();

class WIFI
{
private:
  void initNTP();
  const char *TAG = "NTP";
  char strftime_buf[64];

public:
  WIFI();
  uint32_t printTime();
};

#endif // IOTROAM_H
