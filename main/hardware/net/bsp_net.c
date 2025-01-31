#include "bsp_net.h"

#include <string.h>
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_err.h"
#include "esp_log.h"
#include "nvs_flash.h"

// #define ESP_WIFI_SSID "CMCC-4Qfy-2.4G"
// #define ESP_WIFI_PASS "RDDDD2EX"

#define DEFAULT_SCAN_LIST_SIZE 5

static const char *TAG = "WIFI";
static int s_retry_num = 0;
uint8_t wifi_ssid[30] = "CMCC-4Qfy-2.4G";
uint8_t wifi_passwd[25] = "RDDDD2EX";

// 定义AP信息数组和AP数量
static uint16_t number = DEFAULT_SCAN_LIST_SIZE;
static wifi_ap_record_t ap_info[DEFAULT_SCAN_LIST_SIZE];
static uint16_t ap_count = 0;

bool wifi_statu = false;
/* FreeRTOS event group to signal when we are connected*/
EventGroupHandle_t s_wifi_event_group;
wifi_scan_t wifi_scan_result[5];

static void print_auth_mode(int authmode);

static void print_cipher_type(int pairwise_cipher, int group_cipher);

/**
 * @brief 用于初始化nvs
 */
void init_nvs(void)
{
    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        ESP_ERROR_CHECK(nvs_flash_erase());
        err = nvs_flash_init();
    }
    ESP_ERROR_CHECK(err);
}

/**
 * @brief  事件处理函数
 *
 * @param arg           自定义数据
 * @param event_base    事件类型
 * @param event_id      事件ID
 * @param event_data    事件数据
 */
static void event_handler(void *arg, esp_event_base_t event_base,
                          int32_t event_id, void *event_data)
{
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START)
    {
        esp_wifi_connect();
    }
    else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED)
    {
        if (s_retry_num < EXAMPLE_ESP_MAXIMUM_RETRY)
        {
            esp_wifi_connect();
            s_retry_num++;
            ESP_LOGI(TAG, "retry to connect to the AP");
        }
        else
        {
            // if (s_retry_num > 10)
            // {
            //     esp_restart(); // wifi重启ESP32
            // }
            xEventGroupClearBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
            xEventGroupSetBits(s_wifi_event_group, WIFI_FAIL_BIT);
            wifi_statu = false;
        }
        ESP_LOGI(TAG, "connect to the AP fail");
    }
    else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP)
    {
        ip_event_got_ip_t *event = (ip_event_got_ip_t *)event_data;
        ESP_LOGI(TAG, "got ip:" IPSTR, IP2STR(&event->ip_info.ip));
        s_retry_num = 0;
        xEventGroupClearBits(s_wifi_event_group, WIFI_FAIL_BIT);
        xEventGroupSetBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
        wifi_statu = true;
    }
}

/**
 * @brief 初始化WiFi
 *
 */
void init_wifi(void)
{

    s_wifi_event_group = xEventGroupCreate();

    // 初始化网络接口
    ESP_ERROR_CHECK(esp_netif_init());
    // 创建默认的事件循环
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    // 创建默认的STA网络接口
    esp_netif_t *sta_netif = esp_netif_create_default_wifi_sta();
    // 确保STA网络接口被成功创建
    assert(sta_netif);

    // 初始化WiFi配置
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    // 初始化WiFi
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
    // 清零AP信息数组
    memset(ap_info, 0, sizeof(ap_info));

    esp_event_handler_instance_t instance_any_id;
    esp_event_handler_instance_t instance_got_ip;
    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT,
                                                        ESP_EVENT_ANY_ID,
                                                        &event_handler, NULL,
                                                        &instance_any_id));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT,
                                                        IP_EVENT_STA_GOT_IP,
                                                        &event_handler,
                                                        NULL,
                                                        &instance_got_ip));

    wifi_config_t wifi_config = {
        .sta = {

            .threshold.authmode = ESP_WIFI_SCAN_AUTH_MODE_THRESHOLD,
            .sae_pwe_h2e = ESP_WIFI_SAE_MODE,
            .sae_h2e_identifier = EXAMPLE_H2E_IDENTIFIER,
        },
    };
    memcpy(wifi_config.sta.ssid, wifi_ssid, sizeof(wifi_ssid));
    memcpy(wifi_config.sta.password, wifi_passwd, sizeof(wifi_passwd));

    // 设置WiFi工作模式为STA
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));

    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
    // 启动WiFi
    ESP_ERROR_CHECK(esp_wifi_start());

    ESP_LOGI(TAG, "wifi_init_sta finished.");

    /* Waiting until either the connection is established (WIFI_CONNECTED_BIT) or connection failed for the maximum
     * number of re-tries (WIFI_FAIL_BIT). The bits are set by event_handler() (see above) */
    EventBits_t bits = xEventGroupWaitBits(s_wifi_event_group,
                                           WIFI_CONNECTED_BIT | WIFI_FAIL_BIT,
                                           pdFALSE,
                                           pdFALSE,
                                           portMAX_DELAY);

    /* xEventGroupWaitBits() returns the bits before the call returned, hence we can test which event actually
     * happened. */
    if (bits & WIFI_CONNECTED_BIT)
    {
        ESP_LOGI(TAG, "connected to ap SSID:%s password:%s",
                 wifi_config.sta.ssid, wifi_config.sta.password);
    }
    else if (bits & WIFI_FAIL_BIT)
    {
        ESP_LOGI(TAG, "Failed to connect to SSID:%s, password:%s",
                 wifi_config.sta.ssid, wifi_config.sta.password);
    }
    else
    {
        ESP_LOGE(TAG, "UNEXPECTED EVENT");
    }
}

/**
 * @brief  WiFi扫描
 *
 * @return uint16_t
 */
uint16_t wifi_scan(void)
{
    // 开始WiFi扫描
    esp_wifi_scan_start(NULL, true);
    // 打印最大可存储的AP数量
    ESP_LOGI(TAG, "Max AP number ap_info can hold = %u", number);
    // 获取扫描到的AP记录
    ESP_ERROR_CHECK(esp_wifi_scan_get_ap_records(&number, ap_info));
    // 获取扫描到的AP数量
    ESP_ERROR_CHECK(esp_wifi_scan_get_ap_num(&ap_count));
    // 打印实际扫描到的AP数量
    ESP_LOGI(TAG, "Total APs scanned = %u, actual AP number ap_info holds = %u", ap_count, number);
    // 遍历AP信息数组，打印每个AP的详细信息
    for (int i = 0; i < number; i++)
    {
        ESP_LOGI(TAG, "SSID \t\t%s", ap_info[i].ssid);
        ESP_LOGI(TAG, "RSSI \t\t%d", ap_info[i].rssi);
        print_auth_mode(ap_info[i].authmode);
        if (ap_info[i].authmode != WIFI_AUTH_WEP)
        {
            print_cipher_type(ap_info[i].pairwise_cipher, ap_info[i].group_cipher);
        }
        ESP_LOGI(TAG, "Channel \t\t%d\n", ap_info[i].primary);
        // wifi_scan_result[i].wifi_ssid = ap_info[i].ssid;
        strcpy(wifi_scan_result[i].wifi_ssid, (const char *)ap_info[i].ssid);
    }
    return number;
}

/**
 * @brief 检查Wi-Fi状态
 *
 * @return true
 * @return false
 */
bool isWifiConnected(void)
{
    BaseType_t bits = xEventGroupGetBits(s_wifi_event_group);
    // 检查Wi-Fi状态是否为已连接状态
    if (bits & WIFI_CONNECTED_BIT)
        return true;
    else if (bits & WIFI_FAIL_BIT)
        return false;
    return false;
}

/**
 * @brief 更改WiFi配置并尝试连接
 *
 * @param new_ssid 新的SSID
 * @param ssid_size 新的SSID长度
 * @param new_password 新的密码
 * @param pass_size 新的密码长度
 */
void changeWiFiConfig(uint8_t *new_ssid, size_t ssid_size, uint8_t *new_password, size_t pass_size)
{
    // 先检查Wi-Fi是否已经连接
    if (!isWifiConnected())
    {
        printf("Wi-Fi is disconnected. Changing configuration...\n");
        // 更新配置
        wifi_config_t wifi_config = {
            .sta = {
                .threshold.authmode = ESP_WIFI_SCAN_AUTH_MODE_THRESHOLD,
                .sae_pwe_h2e = ESP_WIFI_SAE_MODE,
                .sae_h2e_identifier = EXAMPLE_H2E_IDENTIFIER,
            },
        };
        memcpy(wifi_config.sta.ssid, new_ssid, ssid_size);
        memcpy(wifi_config.sta.password, new_password, pass_size);
        ESP_ERROR_CHECK(esp_wifi_disconnect());
        // 应用新配置
        ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
        ESP_ERROR_CHECK(esp_wifi_start());
        esp_wifi_connect();
        ESP_LOGI(TAG, "try connected to ap SSID:%s password:%s",
                 wifi_config.sta.ssid, wifi_config.sta.password);
    }
    else
    {
        printf("Wi-Fi is connected. No need to change configuration.\n");
    }
}

static void print_auth_mode(int authmode)
{
    switch (authmode)
    {
    case WIFI_AUTH_OPEN:
        ESP_LOGI(TAG, "Authmode \tWIFI_AUTH_OPEN");
        break;
    case WIFI_AUTH_OWE:
        ESP_LOGI(TAG, "Authmode \tWIFI_AUTH_OWE");
        break;
    case WIFI_AUTH_WEP:
        ESP_LOGI(TAG, "Authmode \tWIFI_AUTH_WEP");
        break;
    case WIFI_AUTH_WPA_PSK:
        ESP_LOGI(TAG, "Authmode \tWIFI_AUTH_WPA_PSK");
        break;
    case WIFI_AUTH_WPA2_PSK:
        ESP_LOGI(TAG, "Authmode \tWIFI_AUTH_WPA2_PSK");
        break;
    case WIFI_AUTH_WPA_WPA2_PSK:
        ESP_LOGI(TAG, "Authmode \tWIFI_AUTH_WPA_WPA2_PSK");
        break;
    case WIFI_AUTH_ENTERPRISE:
        ESP_LOGI(TAG, "Authmode \tWIFI_AUTH_ENTERPRISE");
        break;
    case WIFI_AUTH_WPA3_PSK:
        ESP_LOGI(TAG, "Authmode \tWIFI_AUTH_WPA3_PSK");
        break;
    case WIFI_AUTH_WPA2_WPA3_PSK:
        ESP_LOGI(TAG, "Authmode \tWIFI_AUTH_WPA2_WPA3_PSK");
        break;
    case WIFI_AUTH_WPA3_ENT_192:
        ESP_LOGI(TAG, "Authmode \tWIFI_AUTH_WPA3_ENT_192");
        break;
    default:
        ESP_LOGI(TAG, "Authmode \tWIFI_AUTH_UNKNOWN");
        break;
    }
}

static void print_cipher_type(int pairwise_cipher, int group_cipher)
{
    switch (pairwise_cipher)
    {
    case WIFI_CIPHER_TYPE_NONE:
        ESP_LOGI(TAG, "Pairwise Cipher \tWIFI_CIPHER_TYPE_NONE");
        break;
    case WIFI_CIPHER_TYPE_WEP40:
        ESP_LOGI(TAG, "Pairwise Cipher \tWIFI_CIPHER_TYPE_WEP40");
        break;
    case WIFI_CIPHER_TYPE_WEP104:
        ESP_LOGI(TAG, "Pairwise Cipher \tWIFI_CIPHER_TYPE_WEP104");
        break;
    case WIFI_CIPHER_TYPE_TKIP:
        ESP_LOGI(TAG, "Pairwise Cipher \tWIFI_CIPHER_TYPE_TKIP");
        break;
    case WIFI_CIPHER_TYPE_CCMP:
        ESP_LOGI(TAG, "Pairwise Cipher \tWIFI_CIPHER_TYPE_CCMP");
        break;
    case WIFI_CIPHER_TYPE_TKIP_CCMP:
        ESP_LOGI(TAG, "Pairwise Cipher \tWIFI_CIPHER_TYPE_TKIP_CCMP");
        break;
    case WIFI_CIPHER_TYPE_AES_CMAC128:
        ESP_LOGI(TAG, "Pairwise Cipher \tWIFI_CIPHER_TYPE_AES_CMAC128");
        break;
    case WIFI_CIPHER_TYPE_SMS4:
        ESP_LOGI(TAG, "Pairwise Cipher \tWIFI_CIPHER_TYPE_SMS4");
        break;
    case WIFI_CIPHER_TYPE_GCMP:
        ESP_LOGI(TAG, "Pairwise Cipher \tWIFI_CIPHER_TYPE_GCMP");
        break;
    case WIFI_CIPHER_TYPE_GCMP256:
        ESP_LOGI(TAG, "Pairwise Cipher \tWIFI_CIPHER_TYPE_GCMP256");
        break;
    default:
        ESP_LOGI(TAG, "Pairwise Cipher \tWIFI_CIPHER_TYPE_UNKNOWN");
        break;
    }

    switch (group_cipher)
    {
    case WIFI_CIPHER_TYPE_NONE:
        ESP_LOGI(TAG, "Group Cipher \tWIFI_CIPHER_TYPE_NONE");
        break;
    case WIFI_CIPHER_TYPE_WEP40:
        ESP_LOGI(TAG, "Group Cipher \tWIFI_CIPHER_TYPE_WEP40");
        break;
    case WIFI_CIPHER_TYPE_WEP104:
        ESP_LOGI(TAG, "Group Cipher \tWIFI_CIPHER_TYPE_WEP104");
        break;
    case WIFI_CIPHER_TYPE_TKIP:
        ESP_LOGI(TAG, "Group Cipher \tWIFI_CIPHER_TYPE_TKIP");
        break;
    case WIFI_CIPHER_TYPE_CCMP:
        ESP_LOGI(TAG, "Group Cipher \tWIFI_CIPHER_TYPE_CCMP");
        break;
    case WIFI_CIPHER_TYPE_TKIP_CCMP:
        ESP_LOGI(TAG, "Group Cipher \tWIFI_CIPHER_TYPE_TKIP_CCMP");
        break;
    case WIFI_CIPHER_TYPE_SMS4:
        ESP_LOGI(TAG, "Group Cipher \tWIFI_CIPHER_TYPE_SMS4");
        break;
    case WIFI_CIPHER_TYPE_GCMP:
        ESP_LOGI(TAG, "Group Cipher \tWIFI_CIPHER_TYPE_GCMP");
        break;
    case WIFI_CIPHER_TYPE_GCMP256:
        ESP_LOGI(TAG, "Group Cipher \tWIFI_CIPHER_TYPE_GCMP256");
        break;
    default:
        ESP_LOGI(TAG, "Group Cipher \tWIFI_CIPHER_TYPE_UNKNOWN");
        break;
    }
}
