#include "esp_tls.h"
#include "esp_crt_bundle.h"
#include "esp_http_client.h"
#include "zlib.h"
#include "cJSON.h"
#include "hf_weather.h"
#include "esp_log.h"
#include <string.h>
#include <stdio.h>

extern bool wifi_statu;
// #define MAX_HTTP_RECV_BUFFER 512
#define MAX_HTTP_OUTPUT_BUFFER 2048
#define MAX_DAILY_BUFFER 4096
static const char *TAG = "HTTP_CLIENT";

// #define QWEATHER_DAILY_URL "https://devapi.qweather.com/v7/weather/3d?&location=101160807&key=ac81c985a21143c1a8cd252a7a876bb8"
// #define QWEATHER_NOW_URL "https://devapi.qweather.com/v7/weather/now?&location=101160807&key=ac81c985a21143c1a8cd252a7a876bb8"
// #define QAIR_NOW_URL "https://devapi.qweather.com/v7/air/now?&location=101160807&key=ac81c985a21143c1a8cd252a7a876bb8"
// #define QLOCATION_SEARCH_URL "https://geoapi.qweather.com/v2/city/lookup?location=chaoyang&key=ac81c985a21143c1a8cd252a7a876bb8&adm=beijing&range=cn&number=3"
weather_info_t weather_now = {"12", "12", "12", "12", "12", "12", "12", "12", " 12"};
air_info_t air_now = {"12", "12", "12", "12", "12", "12", "12", "12", " 12"};
daily_forecast_t daily_forecast[3] = {
    {"2025-01-17", "亏凸月", "晴", "西北风", "多云", "西风", "0.0", "805", "1021", "8", "07:30", "-3", "17:15", "100", "20:46", "151", "09:40", "225", "1-3", "3", "3", "180", "29", "25", "0", "3"},
    {"2025-01-18", "满月", "多云", "东南风", "晴", "东北风", "0.0", "806", "1020", "9", "07:31", "-2", "17:16", "101", "20:47", "152", "09:41", "226", "1-3", "4", "4", "181", "30", "26", "1", "4"},
    {"2025-01-19", "新月", "阴", "西南风", "小雨", "西北风", "0.1", "807", "1019", "7", "07:32", "-1", "17:17", "102", "20:48", "153", "09:42", "227", "1-3", "5", "5", "182", "31", "27", "2", "5"}};
city_info_t cities[2] = {{"朝阳区", "101160807", "北京"}, {"和平区", "101160807", "天津"}};
char location_id[10] = "101030500";
/* Root cert for howsmyssl.com, taken from howsmyssl_com_root_cert.pem

   The PEM file was extracted from the output of this command:
   openssl s_client -showcerts -connect www.howsmyssl.com:443 </dev/null

   The CA root cert is the last cert given in the chain of certs.

   To embed it in the app binary, the PEM file is named
   in the component.mk COMPONENT_EMBED_TXTFILES variable.
*/
extern const char howsmyssl_com_root_cert_pem_start[] asm("_binary_howsmyssl_com_root_cert_pem_start");
extern const char howsmyssl_com_root_cert_pem_end[] asm("_binary_howsmyssl_com_root_cert_pem_end");

extern const char postman_root_cert_pem_start[] asm("_binary_postman_root_cert_pem_start");
extern const char postman_root_cert_pem_end[] asm("_binary_postman_root_cert_pem_end");

esp_err_t _http_event_handler(esp_http_client_event_t *evt)
{
    static char *output_buffer; // Buffer to store response of http request from event handler
    static int output_len;      // Stores number of bytes read
    switch (evt->event_id)
    {
    case HTTP_EVENT_ERROR:
        ESP_LOGD(TAG, "HTTP_EVENT_ERROR");
        break;
    case HTTP_EVENT_ON_CONNECTED:
        ESP_LOGD(TAG, "HTTP_EVENT_ON_CONNECTED");
        break;
    case HTTP_EVENT_HEADER_SENT:
        ESP_LOGD(TAG, "HTTP_EVENT_HEADER_SENT");
        break;
    case HTTP_EVENT_ON_HEADER:
        ESP_LOGD(TAG, "HTTP_EVENT_ON_HEADER, key=%s, value=%s", evt->header_key, evt->header_value);
        break;
    case HTTP_EVENT_ON_DATA:
        ESP_LOGD(TAG, "HTTP_EVENT_ON_DATA, len=%d", evt->data_len);
        /*
         *  Check for chunked encoding is added as the URL for chunked encoding used in this example returns binary data.
         *  However, event handler can also be used in case chunked encoding is used.
         */
        if (!esp_http_client_is_chunked_response(evt->client))
        {
            // If user_data buffer is configured, copy the response into the buffer
            int copy_len = 0;
            if (evt->user_data)
            {
                copy_len = MIN(evt->data_len, (MAX_HTTP_OUTPUT_BUFFER - output_len));
                if (copy_len)
                {
                    memcpy(evt->user_data + output_len, evt->data, copy_len);
                }
            }
            else
            {
                const int buffer_len = esp_http_client_get_content_length(evt->client);
                if (output_buffer == NULL)
                {
                    output_buffer = (char *)malloc(buffer_len);
                    output_len = 0;
                    if (output_buffer == NULL)
                    {
                        ESP_LOGE(TAG, "Failed to allocate memory for output buffer");
                        return ESP_FAIL;
                    }
                }
                copy_len = MIN(evt->data_len, (buffer_len - output_len));
                if (copy_len)
                {
                    memcpy(output_buffer + output_len, evt->data, copy_len);
                }
            }
            output_len += copy_len;
        }

        break;
    case HTTP_EVENT_ON_FINISH:
        ESP_LOGD(TAG, "HTTP_EVENT_ON_FINISH");
        if (output_buffer != NULL)
        {
            // Response is accumulated in output_buffer. Uncomment the below line to print the accumulated response
            // ESP_LOG_BUFFER_HEX(TAG, output_buffer, output_len);
            free(output_buffer);
            output_buffer = NULL;
        }
        output_len = 0;
        break;
    case HTTP_EVENT_DISCONNECTED:
        ESP_LOGI(TAG, "HTTP_EVENT_DISCONNECTED");
        int mbedtls_err = 0;
        esp_err_t err = esp_tls_get_and_clear_last_error((esp_tls_error_handle_t)evt->data, &mbedtls_err, NULL);
        if (err != 0)
        {
            ESP_LOGI(TAG, "Last esp error code: 0x%x", err);
            ESP_LOGI(TAG, "Last mbedtls failure: 0x%x", mbedtls_err);
        }
        if (output_buffer != NULL)
        {
            free(output_buffer);
            output_buffer = NULL;
        }
        output_len = 0;
        break;
    case HTTP_EVENT_REDIRECT:
        ESP_LOGD(TAG, "HTTP_EVENT_REDIRECT");
        esp_http_client_set_header(evt->client, "From", "user@example.com");
        esp_http_client_set_header(evt->client, "Accept", "text/html");
        esp_http_client_set_redirection(evt->client);
        break;
    }
    return ESP_OK;
}

/**
 * @brief 生成天气预报查询的URL
 */
const char *generate_qweather_daily_url(const char *location, const char *key)
{
    static char url[256];
    snprintf(url, sizeof(url), "https://devapi.qweather.com/v7/weather/3d?location=%s&key=%s", location, key);
    return url;
}
/**
 * @brief 生成天气查询的URL
 */
const char *generate_qweather_now_url(const char *location, const char *key)
{
    static char url[256];
    snprintf(url, sizeof(url), "https://devapi.qweather.com/v7/weather/now?location=%s&key=%s", location, key);
    return url;
}
/**
 * @brief 生成空气质量查询的URL
 */
const char *generate_qair_now_url(const char *location, const char *key)
{
    static char url[256];
    snprintf(url, sizeof(url), "https://devapi.qweather.com/v7/air/now?location=%s&key=%s", location, key);
    return url;
}
/**
 * @brief 生成城市查询的URL
 */
const char *generate_qsearch_location_url(const char *location, const char *key, const char *adm)
{
    static char url[256];
    snprintf(url, sizeof(url), "https://geoapi.qweather.com/v2/city/lookup?location=%s&key=%s&adm=%s&range=cn&number=2", location, key, adm);
    return url;
}
int gzipDecompress(char *src, int srcLen, char *dst, int *dstLen)
{
    z_stream strm;
    strm.zalloc = Z_NULL;
    strm.zfree = Z_NULL;
    strm.opaque = Z_NULL;

    strm.avail_in = srcLen;
    strm.avail_out = *dstLen;
    strm.next_in = (Bytef *)src;
    strm.next_out = (Bytef *)dst;

    int err = -1;
    err = inflateInit2(&strm, 31); // 初始化 31代表GZIP
    if (err == Z_OK)
    {
        err = inflate(&strm, Z_FINISH); // 解压gzip数据
        if (err == Z_STREAM_END)        // 解压成功
        {
            *dstLen = strm.total_out;
        }
        else // 解压失败
        {
            ESP_LOGW(TAG, "inflate err=!Z_OK\n");
        }
        inflateEnd(&strm);
    }
    else
    {
        ESP_LOGW(TAG, "inflateInit2 err! err=%d\n", err);
    }

    return err;
}

static esp_err_t http_request(const char *url, char *response_buffer, int *https_status, int64_t *gzip_len);

static esp_err_t parse_weather_forecast_json(const char *json_buffer, daily_forecast_t *forecast);
static esp_err_t parse_weather_json(const char *json_buffer, weather_info_t *weather);
static esp_err_t parse_air_json(const char *json_buffer, air_info_t *air);
static esp_err_t parse_location_json(const char *json_buffer, city_info_t cities[], int *num_cities);

void weather_now_url(void)
{
    if (wifi_statu)
    {
        char local_response_buffer[MAX_HTTP_OUTPUT_BUFFER] = {0};
        int https_status = 0;
        int64_t gzip_len = 0;
        int dstBufLen = 1024;
        char *dstBuf = (char *)malloc(1024);

        memset(dstBuf, 0, 1024);
        esp_err_t err = http_request(generate_qweather_now_url(location_id, KEY), local_response_buffer, &https_status, &gzip_len);

        if (err == ESP_OK)
        {
            printf("before http 199: free_heap_size = %ld\n", esp_get_free_heap_size());
            if (https_status == 200)
            {
                int ret = gzipDecompress(local_response_buffer, gzip_len, dstBuf, &dstBufLen);

                if (Z_STREAM_END == ret)
                { /* 解压成功 */
                    parse_weather_json(dstBuf, &weather_now);
                }
                else
                {
                    ESP_LOGW(TAG, "decompress failed:%d\n", ret);
                }
            }
            printf("after http 232 : free_heap_size = %ld\n", esp_get_free_heap_size());
        }
        free(dstBuf);
    }
    else
    {
        ESP_LOGW(TAG, "wifi_statu = 0 wifi disconnect");
    }
}

void weather_air_url(void)
{
    if (wifi_statu)
    {
        char local_response_buffer[MAX_HTTP_OUTPUT_BUFFER] = {0};
        int https_status = 0;
        int64_t gzip_len = 0;
        int dstBufLen = 1024;
        char *dstBuf = (char *)malloc(1024);

        memset(dstBuf, 0, 1024);
        esp_err_t err = http_request(generate_qair_now_url(location_id, KEY), local_response_buffer, &https_status, &gzip_len);

        if (err == ESP_OK)
        {
            printf("before gxhtc3_task : free_heap_size = %ld\n", esp_get_free_heap_size());
            if (https_status == 200)
            {
                int ret = gzipDecompress(local_response_buffer, gzip_len, dstBuf, &dstBufLen);

                if (Z_STREAM_END == ret)
                { /* 解压成功 */
                    parse_air_json(dstBuf, &air_now);
                }
                else
                {
                    ESP_LOGW(TAG, "decompress failed:%d\n", ret);
                }
            }
            printf("after gxhtc3_task : free_heap_size = %ld\n", esp_get_free_heap_size());
        }
        free(dstBuf);
    }
    else
    {
        ESP_LOGW(TAG, "wifi_statu = 0 wifi disconnect");
    }
}
void weather_forecast_get(void)
{
    if (wifi_statu)
    {
        char local_response_buffer[MAX_DAILY_BUFFER] = {0};
        int https_status = 0;
        int64_t gzip_len = 0;
        int dstBufLen = MAX_DAILY_BUFFER;
        char *dstBuf = (char *)malloc(MAX_DAILY_BUFFER);
        if (dstBuf == NULL)
        {
            ESP_LOGE(TAG, "malloc failed");
            return;
        }
        memset(dstBuf, 0, MAX_DAILY_BUFFER);
        esp_err_t err = http_request(generate_qweather_daily_url(location_id, KEY), local_response_buffer, &https_status, &gzip_len);
        if (err == ESP_OK)
        {
            if (https_status == 200)
            {
                int ret = gzipDecompress(local_response_buffer, gzip_len, dstBuf, &dstBufLen);
                if (Z_STREAM_END == ret)
                { /* 解压成功 */
                    parse_weather_forecast_json(dstBuf, daily_forecast);
                }
                else
                {
                    ESP_LOGI(TAG, "decompress failed:%d\n", ret);
                }
            }
            printf("after gxhtc3_task : free_heap_size = %ld\n", esp_get_free_heap_size());
        }
        free(dstBuf);
    }
    else
    {
        ESP_LOGW(TAG, "wifi_statu = 0 wifi disconnect");
    }
}

/**
 * @brief   获取城市信息
 *
 */
void weather_location_get(const char *location, const char *adm, int *num_cities)
{
    if (wifi_statu)
    {
        char local_response_buffer[MAX_HTTP_OUTPUT_BUFFER] = {0};
        int https_status = 0;
        int64_t gzip_len = 0;
        int dstBufLen = MAX_HTTP_OUTPUT_BUFFER;
        char *dstBuf = (char *)malloc(MAX_HTTP_OUTPUT_BUFFER);

        memset(dstBuf, 0, MAX_HTTP_OUTPUT_BUFFER);
        esp_err_t err = http_request(generate_qsearch_location_url(location, KEY, adm), local_response_buffer, &https_status, &gzip_len);

        if (err == ESP_OK)
        {
            printf("before gxhtc3_task : free_heap_size = %ld\n", esp_get_free_heap_size());
            if (https_status == 200)
            {
                int ret = gzipDecompress(local_response_buffer, gzip_len, dstBuf, &dstBufLen);

                if (Z_STREAM_END == ret)
                { /* 解压成功 */
                    parse_location_json(dstBuf, cities, num_cities);
                }
                else
                {
                    ESP_LOGW(TAG, "decompress failed:%d\n", ret);
                }
            }
            printf("after gxhtc3_task : free_heap_size = %ld\n", esp_get_free_heap_size());
        }
        free(dstBuf);
    }
    else
    {
        ESP_LOGW(TAG, "wifi_statu = 0 wifi disconnect");
    }
}

static esp_err_t http_request(const char *url, char *response_buffer, int *https_status, int64_t *gzip_len)
{
    esp_http_client_config_t config = {
        .url = url,
        .event_handler = _http_event_handler,
        .crt_bundle_attach = esp_crt_bundle_attach,
        .user_data = response_buffer, // Pass address of local buffer to get response
    };
    esp_http_client_handle_t client = esp_http_client_init(&config);
    esp_err_t err = esp_http_client_perform(client);

    if (err == ESP_OK)
    {
        *https_status = esp_http_client_get_status_code(client);
        *gzip_len = esp_http_client_get_content_length(client);
        ESP_LOGI(TAG, "HTTPS Status = %d, content_length = %" PRIu64, *https_status, *gzip_len);
    }
    else
    {
        ESP_LOGE(TAG, "Error perform http request %s", esp_err_to_name(err));
    }
    esp_http_client_cleanup(client);
    return err;
}

static esp_err_t parse_weather_json(const char *json_buffer, weather_info_t *weather)
{
    cJSON *root = cJSON_Parse(json_buffer);
    if (!root)
    {
        ESP_LOGE(TAG, "Failed to parse JSON");
        return ESP_FAIL;
    }
    // 检查 code 键值对
    cJSON *code_item = cJSON_GetObjectItemCaseSensitive(root, "code");
    if (!cJSON_IsString(code_item) || strcmp(code_item->valuestring, "200") != 0)
    {
        ESP_LOGE(TAG, "Invalid code value: %s", code_item ? code_item->valuestring : "null");
        cJSON_Delete(root);
        return ESP_FAIL;
    }
    cJSON *now = cJSON_GetObjectItem(root, "now");
    strcpy(weather->temp_n, cJSON_GetObjectItem(now, "temp")->valuestring);
    strcpy(weather->icon_n, cJSON_GetObjectItem(now, "icon")->valuestring);
    strcpy(weather->humidity_n, cJSON_GetObjectItem(now, "humidity")->valuestring);
    strcpy(weather->feelsLike_n, cJSON_GetObjectItem(now, "feelsLike")->valuestring);
    strcpy(weather->windDir_n, cJSON_GetObjectItem(now, "windDir")->valuestring);
    strcpy(weather->windSpeed_n, cJSON_GetObjectItem(now, "windSpeed")->valuestring);
    strcpy(weather->windScale_n, cJSON_GetObjectItem(now, "windScale")->valuestring);
    strcpy(weather->weather_n, cJSON_GetObjectItem(now, "text")->valuestring);
    strcpy(weather->precip_n, cJSON_GetObjectItem(now, "precip")->valuestring);
    cJSON_Delete(root);
    return ESP_OK;
}

static esp_err_t parse_air_json(const char *json_buffer, air_info_t *air)
{
    cJSON *root = cJSON_Parse(json_buffer);
    if (!root)
    {
        ESP_LOGE(TAG, "Failed to parse JSON");
        return ESP_FAIL;
    }
    // 检查 code 键值对
    cJSON *code_item = cJSON_GetObjectItemCaseSensitive(root, "code");
    if (!cJSON_IsString(code_item) || strcmp(code_item->valuestring, "200") != 0)
    {
        ESP_LOGE(TAG, "Invalid code value: %s", code_item ? code_item->valuestring : "null");
        cJSON_Delete(root);
        return ESP_FAIL;
    }
    cJSON *now = cJSON_GetObjectItem(root, "now");

    strcpy(air->aqi_n, cJSON_GetObjectItem(now, "aqi")->valuestring);
    strcpy(air->category_n, cJSON_GetObjectItem(now, "category")->valuestring);
    strcpy(air->primary_n, cJSON_GetObjectItem(now, "primary")->valuestring);
    strcpy(air->pm10_n, cJSON_GetObjectItem(now, "pm10")->valuestring);
    strcpy(air->pm2p5_n, cJSON_GetObjectItem(now, "pm2p5")->valuestring);
    strcpy(air->no2_n, cJSON_GetObjectItem(now, "no2")->valuestring);
    strcpy(air->so2_n, cJSON_GetObjectItem(now, "so2")->valuestring);
    strcpy(air->co_n, cJSON_GetObjectItem(now, "co")->valuestring);
    strcpy(air->o3_n, cJSON_GetObjectItem(now, "o3")->valuestring);
    cJSON_Delete(root);
    return ESP_OK;
}

static esp_err_t parse_weather_forecast_json(const char *json_buffer, daily_forecast_t *forecast)
{
    cJSON *root = cJSON_Parse(json_buffer);
    if (!root)
    {
        ESP_LOGE(TAG, "Failed to parse JSON");
        return ESP_FAIL;
    }
    // 检查 code 键值对
    cJSON *code_item = cJSON_GetObjectItemCaseSensitive(root, "code");
    if (!cJSON_IsString(code_item) || strcmp(code_item->valuestring, "200") != 0)
    {
        ESP_LOGE(TAG, "Invalid code value: %s", code_item ? code_item->valuestring : "null");
        cJSON_Delete(root);
        return ESP_FAIL;
    }
    cJSON *item;
    cJSON *daily_array = cJSON_GetObjectItemCaseSensitive(root, "daily");
    if (cJSON_IsArray(daily_array))
    {
        int num_daily = cJSON_GetArraySize(daily_array);
        for (int i = 0; i < num_daily && i < 3; i++)
        {
            cJSON *daily_item = cJSON_GetArrayItem(daily_array, i);

            item = cJSON_GetObjectItemCaseSensitive(daily_item, "fxDate");
            if (cJSON_IsString(item) && (item->valuestring != NULL))
            {
                strncpy(forecast[i].fxDate, item->valuestring, sizeof(forecast[i].fxDate) - 1);
            }

            item = cJSON_GetObjectItemCaseSensitive(daily_item, "sunrise");
            if (cJSON_IsString(item) && (item->valuestring != NULL))
            {
                strncpy(forecast[i].sunrise, item->valuestring, sizeof(forecast[i].sunrise) - 1);
            }

            item = cJSON_GetObjectItemCaseSensitive(daily_item, "sunset");
            if (cJSON_IsString(item) && (item->valuestring != NULL))
            {
                strncpy(forecast[i].sunset, item->valuestring, sizeof(forecast[i].sunset) - 1);
            }

            item = cJSON_GetObjectItemCaseSensitive(daily_item, "moonrise");
            if (cJSON_IsString(item) && (item->valuestring != NULL))
            {
                strncpy(forecast[i].moonrise, item->valuestring, sizeof(forecast[i].moonrise) - 1);
            }

            item = cJSON_GetObjectItemCaseSensitive(daily_item, "moonset");
            if (cJSON_IsString(item) && (item->valuestring != NULL))
            {
                strncpy(forecast[i].moonset, item->valuestring, sizeof(forecast[i].moonset) - 1);
            }

            item = cJSON_GetObjectItemCaseSensitive(daily_item, "moonPhase");
            if (cJSON_IsString(item) && (item->valuestring != NULL))
            {
                strncpy(forecast[i].moonPhase, item->valuestring, sizeof(forecast[i].moonPhase) - 1);
            }

            item = cJSON_GetObjectItemCaseSensitive(daily_item, "moonPhaseIcon");
            if (cJSON_IsString(item) && (item->valuestring != NULL))
            {
                strncpy(forecast[i].moonPhaseIcon, item->valuestring, sizeof(forecast[i].moonPhaseIcon) - 1);
            }

            item = cJSON_GetObjectItemCaseSensitive(daily_item, "tempMax");
            if (cJSON_IsString(item) && (item->valuestring != NULL))
            {
                strncpy(forecast[i].tempMax, item->valuestring, sizeof(forecast[i].tempMax) - 1);
            }

            item = cJSON_GetObjectItemCaseSensitive(daily_item, "tempMin");
            if (cJSON_IsString(item) && (item->valuestring != NULL))
            {
                strncpy(forecast[i].tempMin, item->valuestring, sizeof(forecast[i].tempMin) - 1);
            }

            item = cJSON_GetObjectItemCaseSensitive(daily_item, "iconDay");
            if (cJSON_IsString(item) && (item->valuestring != NULL))
            {
                strncpy(forecast[i].iconDay, item->valuestring, sizeof(forecast[i].iconDay) - 1);
            }

            item = cJSON_GetObjectItemCaseSensitive(daily_item, "textDay");
            if (cJSON_IsString(item) && (item->valuestring != NULL))
            {
                strncpy(forecast[i].textDay, item->valuestring, sizeof(forecast[i].textDay) - 1);
            }

            item = cJSON_GetObjectItemCaseSensitive(daily_item, "iconNight");
            if (cJSON_IsString(item) && (item->valuestring != NULL))
            {
                strncpy(forecast[i].iconNight, item->valuestring, sizeof(forecast[i].iconNight) - 1);
            }

            item = cJSON_GetObjectItemCaseSensitive(daily_item, "textNight");
            if (cJSON_IsString(item) && (item->valuestring != NULL))
            {
                strncpy(forecast[i].textNight, item->valuestring, sizeof(forecast[i].textNight) - 1);
            }

            item = cJSON_GetObjectItemCaseSensitive(daily_item, "wind360Day");
            if (cJSON_IsString(item) && (item->valuestring != NULL))
            {
                strncpy(forecast[i].wind360Day, item->valuestring, sizeof(forecast[i].wind360Day) - 1);
            }

            item = cJSON_GetObjectItemCaseSensitive(daily_item, "windDirDay");
            if (cJSON_IsString(item) && (item->valuestring != NULL))
            {
                strncpy(forecast[i].windDirDay, item->valuestring, sizeof(forecast[i].windDirDay) - 1);
            }

            item = cJSON_GetObjectItemCaseSensitive(daily_item, "windScaleDay");
            if (cJSON_IsString(item) && (item->valuestring != NULL))
            {
                strncpy(forecast[i].windScaleDay, item->valuestring, sizeof(forecast[i].windScaleDay) - 1);
            }

            item = cJSON_GetObjectItemCaseSensitive(daily_item, "windSpeedDay");
            if (cJSON_IsString(item) && (item->valuestring != NULL))
            {
                strncpy(forecast[i].windSpeedDay, item->valuestring, sizeof(forecast[i].windSpeedDay) - 1);
            }

            item = cJSON_GetObjectItemCaseSensitive(daily_item, "wind360Night");
            if (cJSON_IsString(item) && (item->valuestring != NULL))
            {
                strncpy(forecast[i].wind360Night, item->valuestring, sizeof(forecast[i].wind360Night) - 1);
            }

            item = cJSON_GetObjectItemCaseSensitive(daily_item, "windDirNight");
            if (cJSON_IsString(item) && (item->valuestring != NULL))
            {
                strncpy(forecast[i].windDirNight, item->valuestring, sizeof(forecast[i].windDirNight) - 1);
            }

            item = cJSON_GetObjectItemCaseSensitive(daily_item, "windScaleNight");
            if (cJSON_IsString(item) && (item->valuestring != NULL))
            {
                strncpy(forecast[i].windScaleNight, item->valuestring, sizeof(forecast[i].windScaleNight) - 1);
            }

            item = cJSON_GetObjectItemCaseSensitive(daily_item, "windSpeedNight");
            if (cJSON_IsString(item) && (item->valuestring != NULL))
            {
                strncpy(forecast[i].windSpeedNight, item->valuestring, sizeof(forecast[i].windSpeedNight) - 1);
            }

            item = cJSON_GetObjectItemCaseSensitive(daily_item, "humidity");
            if (cJSON_IsString(item) && (item->valuestring != NULL))
            {
                strncpy(forecast[i].humidity, item->valuestring, sizeof(forecast[i].humidity) - 1);
            }

            item = cJSON_GetObjectItemCaseSensitive(daily_item, "precip");
            if (cJSON_IsString(item) && (item->valuestring != NULL))
            {
                strncpy(forecast[i].precip, item->valuestring, sizeof(forecast[i].precip) - 1);
            }

            item = cJSON_GetObjectItemCaseSensitive(daily_item, "pressure");
            if (cJSON_IsString(item) && (item->valuestring != NULL))
            {
                strncpy(forecast[i].pressure, item->valuestring, sizeof(forecast[i].pressure) - 1);
            }

            item = cJSON_GetObjectItemCaseSensitive(daily_item, "vis");
            if (cJSON_IsString(item) && (item->valuestring != NULL))
            {
                strncpy(forecast[i].vis, item->valuestring, sizeof(forecast[i].vis) - 1);
            }

            item = cJSON_GetObjectItemCaseSensitive(daily_item, "cloud");
            if (cJSON_IsString(item) && (item->valuestring != NULL))
            {
                strncpy(forecast[i].cloud, item->valuestring, sizeof(forecast[i].cloud) - 1);
            }

            item = cJSON_GetObjectItemCaseSensitive(daily_item, "uvIndex");
            if (cJSON_IsString(item) && (item->valuestring != NULL))
            {
                strncpy(forecast[i].uvIndex, item->valuestring, sizeof(forecast[i].uvIndex) - 1);
            }
        }
    }
    cJSON_Delete(root);
    return ESP_OK;
}
static esp_err_t parse_location_json(const char *json_buffer, city_info_t cities[], int *num_cities)
{
    cJSON *root = cJSON_Parse(json_buffer);
    if (!root)
    {
        ESP_LOGE(TAG, "Failed to parse JSON");
        return ESP_FAIL;
    }

    // 检查 code 键值对
    cJSON *code_item = cJSON_GetObjectItemCaseSensitive(root, "code");
    if (!cJSON_IsString(code_item) || strcmp(code_item->valuestring, "200") != 0)
    {
        ESP_LOGE(TAG, "Invalid code value: %s", code_item ? code_item->valuestring : "null");
        cJSON_Delete(root);
        return ESP_FAIL;
    }

    cJSON *location_array = cJSON_GetObjectItem(root, "location");
    if (!cJSON_IsArray(location_array))
    {
        ESP_LOGE(TAG, "location is not an array");
        cJSON_Delete(root);
        return ESP_FAIL;
    }

    int num_locations = cJSON_GetArraySize(location_array);
    *num_cities = 0;

    for (int i = 0; i < num_locations && i < 2; i++) // 最多提取两个城市
    {
        cJSON *location_item = cJSON_GetArrayItem(location_array, i);

        cJSON *name = cJSON_GetObjectItemCaseSensitive(location_item, "name");
        if (cJSON_IsString(name) && (name->valuestring != NULL))
        {
            strncpy(cities[i].name, name->valuestring, sizeof(cities[i].name) - 1);
        }

        cJSON *id = cJSON_GetObjectItemCaseSensitive(location_item, "id");
        if (cJSON_IsString(id) && (id->valuestring != NULL))
        {
            strncpy(cities[i].id, id->valuestring, sizeof(cities[i].id) - 1);
        }

        cJSON *adm2 = cJSON_GetObjectItemCaseSensitive(location_item, "adm2");
        if (cJSON_IsString(adm2) && (adm2->valuestring != NULL))
        {
            strncpy(cities[i].adm2, adm2->valuestring, sizeof(cities[i].adm2) - 1);
        }

        (*num_cities)++;
    }
    cJSON_Delete(root);
    return ESP_OK;
}