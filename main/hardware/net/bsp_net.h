#pragma once

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include <stdint.h>
#include <stddef.h>

#define EXAMPLE_ESP_MAXIMUM_RETRY 1 // 最大重连次数
#define ESP_WIFI_SAE_MODE WPA3_SAE_PWE_BOTH
#define EXAMPLE_H2E_IDENTIFIER ""
#define ESP_WIFI_SCAN_AUTH_MODE_THRESHOLD WIFI_AUTH_WPA2_PSK

/* The event group allows multiple bits for each event, but we only care about two events:
 * - we are connected to the AP with an IP
 * - we failed to connect after the maximum amount of retries */
#define WIFI_CONNECTED_BIT BIT0
#define WIFI_FAIL_BIT BIT1
#define WIFI_GET_SNTP_BIT BIT2
#define WIFI_LOCATION_SEARCH_BIT BIT3
#define WIFI_SCAN_BIT BIT4
#define WIFI_REFRESH_SETTING BIT5

typedef struct
{
    char wifi_ssid[33];
} wifi_scan_t;

extern EventGroupHandle_t s_wifi_event_group;
extern bool wifi_statu;
extern wifi_scan_t wifi_scan_result[5];
extern uint8_t wifi_ssid[];
extern uint8_t wifi_passwd[];

void init_nvs(void);
void init_wifi(void);
uint16_t wifi_scan(void);

void changeWiFiConfig(uint8_t *new_ssid, size_t ssid_size, uint8_t *new_password, size_t pass_size);
