#ifndef IOTROAM_H
#define IOTROAM_H

#include "esp_err.h"

void iotroam_init(const char *ssid, const char *password);
void iotroam_connect();
void iotroam_disconnect();

#endif // IOTROAM_H
