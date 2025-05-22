#include "iotroam.h"

static const char *TAG = "iotroam"; // Tag for ESP logging
const char *SNTP = "SNTP";          // Tag for SNTP logging

// Global variables
static EventGroupHandle_t wifi_event_group; // Used to wait for connection result
static int s_retry_num = 0;                 // Counter for reconnect attempts
static char saved_ssid[32];                 // Stores the SSID
static char saved_password[64];             // Stores the password

WIFI ::WIFI(const char *ssid, const char *password)
{
    iotroam_init(ssid, password);
    initSNTP();
}

// Wi-Fi event handler: handles connect/disconnect logic
static void wifi_event_handler(void *arg, esp_event_base_t event_base,
                               int32_t event_id, void *event_data)
{
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START)
    {
        // When station starts, attempt to connect
        lcd_put_cursor(0, 0);
        lcd_send_string("Connecting     ");
        esp_wifi_connect();
    }
    else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED)
    {
        // On disconnection, try again unless retry limit exceeded
        if (s_retry_num < MAX_FAILURES)
        {
            esp_wifi_connect();
            s_retry_num++;
        }
        else
        {
            // Set failure bit after max retries
            xEventGroupSetBits(wifi_event_group, WIFI_FAILURE);
        }
    }
}

// IP event handler: called when ESP gets IP from the router
static void ip_event_handler(void *arg, esp_event_base_t event_base,
                             int32_t event_id, void *event_data)
{
    if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP)
    {
        ip_event_got_ip_t *event = (ip_event_got_ip_t *)event_data;

        // Log IP to serial console
        ESP_LOGI(TAG, "Got IP: " IPSTR, IP2STR(&event->ip_info.ip));

        s_retry_num = 0; // Reset retry counter

        // Display IP on LCD
        lcd_put_cursor(1, 0);
        char ip_str[16];
        sprintf(ip_str, IPSTR, IP2STR(&event->ip_info.ip));
        lcd_send_string(ip_str);

        // Signal connection success
        xEventGroupSetBits(wifi_event_group, WIFI_SUCCESS);
    }
}

// Initializes Wi-Fi with given SSID and password
void WIFI::iotroam_init(const char *ssid, const char *password)
{
    if (!ssid || !password)
        return; // Guard against null pointers

    // Save credentials to global variables
    strncpy(saved_ssid, ssid, sizeof(saved_ssid) - 1);
    strncpy(saved_password, password, sizeof(saved_password) - 1);

    // Initialize NVS (non-volatile storage)
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        // If NVS is corrupted/full, erase and retry
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    // Initialize networking and event loop
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_sta(); // Create default STA interface

    // Initialize Wi-Fi driver
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    // Register event handlers for Wi-Fi and IP events
    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_event_handler, NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &ip_event_handler, NULL));
}

// Starts connection attempt using previously initialized credentials
void WIFI::iotroam_connect()
{
    wifi_event_group = xEventGroupCreate(); // Create event group for connection result

    wifi_config_t wifi_config = {0};
    strncpy((char *)wifi_config.sta.ssid, saved_ssid, sizeof(wifi_config.sta.ssid));
    strncpy((char *)wifi_config.sta.password, saved_password, sizeof(wifi_config.sta.password));

    // Security and PMF (Protected Management Frames) settings
    wifi_config.sta.threshold.authmode = WIFI_AUTH_WPA2_PSK;
    wifi_config.sta.pmf_cfg.capable = true;
    wifi_config.sta.pmf_cfg.required = false;

    // Set Wi-Fi mode and configuration
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());

    // Wait for success or failure (blocking)
    EventBits_t bits = xEventGroupWaitBits(wifi_event_group,
                                           WIFI_SUCCESS | WIFI_FAILURE,
                                           pdFALSE,
                                           pdFALSE,
                                           portMAX_DELAY);

    vEventGroupDelete(wifi_event_group); // Clean up after waiting

    // Notify user via LCD
    if (bits & WIFI_SUCCESS)
    {
        lcd_put_cursor(0, 0);
        lcd_send_string("Connected     ");
    }
    else
    {
        lcd_put_cursor(0, 0);
        lcd_send_string("Failed     ");
    }
}

void WIFI ::initSNTP() // Initializing of the SNTP connection
{
    ESP_LOGI("SNTP", "Initializing SNTP");
    esp_sntp_setoperatingmode(SNTP_OPMODE_POLL);
    esp_sntp_setservername(0, "3.nl.pool.ntp.org");
    esp_sntp_init();
}

uint32_t WIFI ::getTime()
{
    time_t now;
    struct tm timeinfo = {0, 0, 0, 0, 0, 0, 0, 0, 0}; // Putting the whole struct on 0 to counteract warnings
    uint8_t retry = 0;

    // Comparing tm_year to 2025-1900 since tm_year starts counting the years since 1900
    while ((timeinfo.tm_year < CURRENTYEAR) && (retry < MAX_FAILURES)) // Trying to fetch time for 10 times
    {
        retry++;
        time(&now);                          // Getting amount of seconds since 1970
        gmtime_r(&now, &timeinfo);           // Transforming the input to YY:MM:DD:HH:MM:SS in UTC
        timeinfo.tm_hour += CEST_CORRECTION; // Adding 2 hrs to transform UTC to CEST
        ESP_LOGI(SNTP, "Waiting... %s", asctime(&timeinfo));
        vTaskDelay(TWOSECONDS);
    }

    if (retry == MAX_FAILURES) // If the time isn't fetched properly log the raw time (only for debugging purposes)
    {
        ESP_LOGI(SNTP, "Raw time: %" PRIu64, (uint64_t)now);
        return 1;
    }

    // Calculate seconds since midnight
    uint32_t seconds_since_midnight = timeinfo.tm_hour * SECONDS_IN_HOUR +
                                      timeinfo.tm_min * SECONDS_IN_MINUTE +
                                      timeinfo.tm_sec;

    // Convert to centibeats (1 centibeat = 0.864 seconds) and rounding so the function doesnt return a double.
    double centibeats = round(seconds_since_midnight / CENTIBEAT_MULTIPLIER);

    strftime(strftime_buf, sizeof(strftime_buf), "%c", &timeinfo);  // Adding info to a printable string
    ESP_LOGI(SNTP, "Current time: %s", strftime_buf);               // Logging current time
    ESP_LOGI(SNTP, "Current time in centibeats: %.0f", centibeats); // Logging current centibeats
    return centibeats;
}