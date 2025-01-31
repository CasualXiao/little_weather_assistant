
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"

#include "esp_netif_sntp.h"
#include "esp_sntp.h"

#include "bsp_myi2c.h"
#include "bsp_lcd.h"
#include "bsp_net.h"
#include "bsp_timer.h"
#include "bsp_gxhtc3.h"
#include "hf_weather.h"

#include "ui_func.h"

SemaphoreHandle_t lvgl_mux = NULL;      // 互斥信号量 保护lvgl
SemaphoreHandle_t ghxtc3_sem;           // ghxtc3 温湿度传感器采集
SemaphoreHandle_t weather_now_sem;      // 和风实时天气请求
SemaphoreHandle_t weather_forecast_sem; // 和风天气3天预报请求

static const char *TAG = "main";

extern volatile SemaphoreHandle_t sec_timersem;
static void lvgl_task(void *arg)
{
    (void)arg;
    while (1)
    {
        // 10ms重绘图像
        //  The task running lv_timer_handler should have lower priority than that running `lv_tick_inc`
        if (xSemaphoreTake(lvgl_mux, 0) == pdPASS)
        {
            lv_timer_handler();
            xSemaphoreGive(lvgl_mux);
        }
        // raise the task priority of LVGL and/or reduce the handler period can improve the performance
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}

static void secPollingTask(void *args);
static void get_time_task(void *pvParameters);
static void gxhtc3_task(void *args);
static void weather_now_Task(void *args);
static void weather_forecast_Task(void *args);
static void search_location_Task(void *args);
static void wifi_setting_Task(void *args);
void app_main(void)
{

    ESP_ERROR_CHECK(i2c_master_init());
    ESP_LOGI(TAG, "I2C initialized successfully");
    lvgl_mux = xSemaphoreCreateMutex();    // 创建互斥信号量
    ghxtc3_sem = xSemaphoreCreateBinary(); // 创建信号量
    weather_now_sem = xSemaphoreCreateBinary();
    weather_forecast_sem = xSemaphoreCreateBinary();
    // wifi
    init_nvs();
    init_wifi();
    // 定时器
    init_timer();
    // 温湿度传感器
    gxhtc3_init();
    // lcd
    example_lcd_init(BSP_LCD_ROTATE_270);                             // 初始化LCD
    xTaskCreate(get_time_task, "get_time_task", 8192, NULL, 5, NULL); // 一次性任务   获取网络时间
    EventBits_t bits = xEventGroupWaitBits(s_wifi_event_group,
                                           WIFI_GET_SNTP_BIT,
                                           pdFALSE,
                                           pdFALSE,
                                           portMAX_DELAY);
    ESP_LOGI(TAG, "WIFI_GET_SNTP_BIT");
    ui_init();
    // 创建一个秒级定时任务
    xTaskCreate(
        secPollingTask // 任务函数
        ,
        "secPollingTask" // 任务名称
        ,
        4096 // 任务堆栈大小
        ,
        NULL,
        2 // 优先级, (configMAX_PRIORITIES - 1)最高, 0最低
        ,
        NULL);
    // 创建lvgl任务
    xTaskCreate(lvgl_task, "lvgl_task", 4096, NULL, 5, NULL);
    xTaskCreate(gxhtc3_task, "gxhtc3_task", 4096, NULL, 1, NULL);
    xTaskCreate(weather_now_Task, "weather_now_Task", 8192 + 1024, NULL, 2, NULL);
    xTaskCreate(weather_forecast_Task, "weather_forecast_Task", 8192 + 2048, NULL, 2, NULL);
    xTaskCreate(search_location_Task, "search_location_Task", 8192, NULL, 2, NULL);
    xTaskCreate(wifi_setting_Task, "wifi_setting_Task", 4096, NULL, 3, NULL);
}

/**
 * @brief 秒级任务
 *
 * @param args
 */
static void secPollingTask(void *args)
{
    (void)args;
    struct tm timeinfo; // 它表示当前的时间信息，包括年、月、日、小时、分钟、秒等
    time_t now;         // 它表示当前的Unix时间戳，即从1970年1月1日00:00:00 UTC到现在的总秒数。
    uint8_t hour = 24;  // 当前小时
    uint8_t min = 60;   // 分钟计数，用于30分钟刷新实时天气

    // 由sec_timer提供信号量唤醒
    for (;;)
    {
        if (xSemaphoreTake(sec_timersem, 0) == pdTRUE)
        {
            time(&now);
            localtime_r(&now, &timeinfo);
            switch (timeinfo.tm_sec)
            {
            case 0:
                /* code */
                if (xSemaphoreTake(lvgl_mux, 0) == pdPASS)
                {
                    fresh_time();
                    xSemaphoreGive(lvgl_mux);
                }
                break;
            case 5:
                min++;
                if (min >= 30)
                {
                    min = 0;
                    ESP_LOGI(TAG, "xSemaphoreGive(weather_now_sem)");
                    xSemaphoreGive(weather_now_sem);
                }
                break;
            case 15:
                if (timeinfo.tm_hour != hour)
                {
                    hour = timeinfo.tm_hour;
                    ESP_LOGI(TAG, "xSemaphoreGive(weather_forecast_sem)");
                    xSemaphoreGive(weather_forecast_sem);
                }
                break;
            default:
                if (timeinfo.tm_sec % 2 == 0)
                    xSemaphoreGive(ghxtc3_sem);
                break;
            }
        }
        vTaskDelay(20 / portTICK_PERIOD_MS);
    }
}

// 获得日期时间 任务函数
static void get_time_task(void *pvParameters)
{
    xEventGroupWaitBits(s_wifi_event_group, WIFI_CONNECTED_BIT, pdFALSE, pdFALSE, portMAX_DELAY);
    if (wifi_statu == true)
    {
        esp_sntp_config_t config = ESP_NETIF_SNTP_DEFAULT_CONFIG("pool.ntp.org");
        esp_netif_sntp_init(&config);
        // wait for time to be set
        int retry = 0;
        const int retry_count = 6;
        while (esp_netif_sntp_sync_wait(10000 / portTICK_PERIOD_MS) == ESP_ERR_TIMEOUT && ++retry < retry_count)
        {
            ESP_LOGI(TAG, "Waiting for system time to be set... (%d/%d)", retry, retry_count);
        }

        if (retry > 5)
        {
            esp_restart(); // 没有获取到时间的话 重启ESP32
        }

        esp_netif_sntp_deinit();
        // 设置时区
        setenv("TZ", "CST-8", 1);
        tzset();
        xEventGroupSetBits(s_wifi_event_group, WIFI_GET_SNTP_BIT);
        // // 获取系统时间
        // time(&now);
        // localtime_r(&now, &timeinfo);
    }
    else
    {
        ESP_LOGI(TAG, "wifi_statu = false");
    }

    vTaskDelete(NULL);
}
/**
 * @brief 温湿度采集任务
 *
 * @param args
 */
static void gxhtc3_task(void *args)
{
    (void)args;
    esp_err_t ret;
    float temp, humi;
    while (1)
    {
        if (xSemaphoreTake(ghxtc3_sem, 0) == pdTRUE)
        {

            ret = gxhtc3_get_tah(&temp, &humi);
            if (ret != ESP_OK)
            {
                ESP_LOGE(TAG, "GXHTC3 READ TAH ERROR.");
            }
            if (xSemaphoreTake(lvgl_mux, 0) == pdPASS)
            {
                fresh_temp_humi(temp, humi);
                xSemaphoreGive(lvgl_mux);
            }
        }
        vTaskDelay(20 / portTICK_PERIOD_MS);
    }
}

static void weather_now_Task(void *args)
{
    (void)args;
    for (;;)
    {
        if (xSemaphoreTake(weather_now_sem, 0) == pdTRUE)
        {
            int i = esp_get_minimum_free_heap_size();
            ESP_LOGI("内存", "before init weather : free_heap_size = %d", i);
            printf("before init weather : free_heap_size = %ld\n", esp_get_free_heap_size());
            weather_now_url();
            weather_air_url();
            printf("after init weather : free_heap_size = %ld\n", esp_get_free_heap_size());
            if (xSemaphoreTake(lvgl_mux, 0) == pdPASS)
            {
                fresh_weather(&weather_now);
                fresh_air(&air_now);
                xSemaphoreGive(lvgl_mux);
            }
        }
        vTaskDelay(20 / portTICK_PERIOD_MS);
    }
}

static void weather_forecast_Task(void *args)
{
    (void)args;
    for (;;)
    {
        if (xSemaphoreTake(weather_forecast_sem, 0) == pdTRUE)
        {
            int i = esp_get_minimum_free_heap_size();
            ESP_LOGI("内存", "before init weather : free_heap_size = %d", i);
            printf("before init weather : free_heap_size = %ld\n", esp_get_free_heap_size());
            weather_forecast_get();
            printf("after init weather : free_heap_size = %ld\n", esp_get_free_heap_size());
            if (xSemaphoreTake(lvgl_mux, 0) == pdPASS)
            {
                // todo 冗杂
                fresh_first_day(daily_forecast);
                fresh_second_day(daily_forecast);
                fresh_third_day(daily_forecast);
                xSemaphoreGive(lvgl_mux);
            }
        }
        vTaskDelay(20 / portTICK_PERIOD_MS);
    }
}

static void search_location_Task(void *args)
{
    (void)args;
    for (;;)
    {
        EventBits_t bits = xEventGroupWaitBits(s_wifi_event_group,
                                               WIFI_LOCATION_SEARCH_BIT,
                                               pdFALSE,
                                               pdFALSE,
                                               portMAX_DELAY);
        weather_location_get(city, adm, &num_cities);
        xEventGroupClearBits(s_wifi_event_group, WIFI_LOCATION_SEARCH_BIT);
        if (xSemaphoreTake(lvgl_mux, 0) == pdPASS)
        {
            fresh_search_city();
            xSemaphoreGive(lvgl_mux);
        }
        vTaskDelay(20 / portTICK_PERIOD_MS);
    }
}

static void wifi_setting_Task(void *args)
{
    (void)args;
    uint16_t num_wifi_list = 0;
    for (;;)
    {
        EventBits_t bits = xEventGroupWaitBits(s_wifi_event_group,
                                               WIFI_SCAN_BIT || WIFI_REFRESH_SETTING,
                                               pdFALSE,
                                               pdFALSE,
                                               portMAX_DELAY);
        if (bits & WIFI_SCAN_BIT)
        {
            num_wifi_list = wifi_scan();
            xEventGroupClearBits(s_wifi_event_group, WIFI_SCAN_BIT);
            if (xSemaphoreTake(lvgl_mux, 0) == pdPASS)
            {
                fresh_dropdown_wifi_list_option(num_wifi_list);
                xSemaphoreGive(lvgl_mux);
            }
        }
        else if (bits & WIFI_REFRESH_SETTING)
        {
            // changeWiFiConfig(wifi_ssid, wifi_ssid_size, wifi_password, wifi_password_size);
            // WIFI_REFRESH_SETTING
        }
        vTaskDelay(20 / portTICK_PERIOD_MS);
    }
}