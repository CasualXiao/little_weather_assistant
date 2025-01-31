#include "ui_func.h"
#include <sys/time.h>
#include <stdio.h>
#include "esp_log.h"
#include "bsp_net.h"

extern bool wifi_statu;

static bool is_search_in_progress = false; // 添加标志位

char *city;
char *adm;
int num_cities = 0;

const char *weekday_map[] = {
    "周一", // Monday
    "周二", // Tuesday
    "周三", // Wednesday
    "周四", // Thursday
    "周五", // Friday
    "周六", // Saturday
    "周日", // Sunday
};
// 定义一个结构体来存储解析后的日期信息
typedef struct
{
    int year;
    int month;
    int day;
    int weekday;           // 0: 周日, 1: 周一, ..., 6: 周六
    char weekday_str[10];  // 存储中文星期几
    char month_day_str[6]; // 存储 "月-日" 格式，例如 "1-17"
} parsed_date_t;

// 定义天气图像映射
typedef struct
{
    const char *weather_description;
    const lv_img_dsc_t *image;
} weather_image_map_t;

static const weather_image_map_t weather_image_map[] = {
    {"100", &ui_img_icon_100_64_png},
    {"101", &ui_img_icon_101_64_png},
    {"102", &ui_img_icon_102_64_png},
    {"103", &ui_img_icon_103_64_png},
    {"104", &ui_img_icon_104_64_png},
    {"150", &ui_img_icon_150_64_png},
    {"151", &ui_img_icon_101_64_png},
    {"152", &ui_img_icon_102_64_png},
    {"153", &ui_img_icon_153_64_png},
    {"300", &ui_img_icon_300_64_png},
    {"301", &ui_img_icon_301_64_png},
    {"302", &ui_img_icon_302_64_png},
    {"303", &ui_img_icon_303_64_png},
    {"304", &ui_img_icon_304_64_png},
    {"305", &ui_img_icon_305_64_png},
    {"306", &ui_img_icon_306_64_png},
    {"307", &ui_img_icon_307_64_png},
    {"308", &ui_img_icon_308_64_png},
    {"309", &ui_img_icon_309_64_png},
    {"310", &ui_img_icon_310_64_png},
    {"311", &ui_img_icon_311_64_png},
    {"312", &ui_img_icon_312_64_png},
    {"313", &ui_img_icon_313_64_png},
    {"314", &ui_img_icon_314_64_png},
    {"315", &ui_img_icon_315_64_png},
    {"316", &ui_img_icon_316_64_png},
    {"317", &ui_img_icon_317_64_png},
    {"318", &ui_img_icon_318_64_png},
    {"350", &ui_img_icon_350_64_png},
    {"351", &ui_img_icon_351_64_png},
    {"399", &ui_img_icon_399_64_png},
    {"400", &ui_img_icon_400_64_png},
    {"401", &ui_img_icon_401_64_png},
    {"402", &ui_img_icon_402_64_png},
    {"403", &ui_img_icon_403_64_png},
    {"404", &ui_img_icon_404_64_png},
    {"405", &ui_img_icon_405_64_png},
    {"406", &ui_img_icon_406_64_png},
    {"407", &ui_img_icon_407_64_png},
    {"408", &ui_img_icon_408_64_png},
    {"409", &ui_img_icon_409_64_png},
    {"410", &ui_img_icon_410_64_png},
    {"456", &ui_img_icon_456_64_png},
    {"457", &ui_img_icon_457_64_png},
    {"499", &ui_img_icon_499_64_png},
    {"500", &ui_img_icon_500_64_png},
    {"501", &ui_img_icon_501_64_png},
    {"502", &ui_img_icon_502_64_png},
    {"503", &ui_img_icon_503_64_png},
    {"504", &ui_img_icon_504_64_png},
    {"507", &ui_img_icon_507_64_png},
    {"508", &ui_img_icon_508_64_png},
    {"509", &ui_img_icon_509_64_png},
    {"510", &ui_img_icon_510_64_png},
    {"511", &ui_img_icon_511_64_png},
    {"512", &ui_img_icon_512_64_png},
    {"513", &ui_img_icon_513_64_png},
    {"514", &ui_img_icon_514_64_png},
    {"515", &ui_img_icon_515_64_png},
    {"900", &ui_img_icon_900_64_png},
    {"901", &ui_img_icon_901_64_png},
    {"999", &ui_img_icon_999_64_png},
    // 添加其他天气描述和对应的图像
};
// 省份直辖市
static const char *location_map[] = {
    "beijing",
    "shanghai",
    "tianjin",
    "chongqing",
    "heilongjiang",
    "jilin",
    "liaoning",
    "neimenggu",
    "hebei",
    "shanxi",
    "shaanxi",
    "shandong",
    "xinjiang",
    "xizang",
    "qinghai",
    "gansu",
    "ningxia",
    "henan",
    "jiangsu",
    "hubei",
    "zhejiang",
    "anhui",
    "fujian",
    "jiangxi",
    "hunan",
    "guizhou",
    "sichuan",
    "guangdong",
    "yunnan",
    "guangxi",
    "hainan",
    "hongkong",
    "aomen",
    "taiwan",
};

static const weather_image_map_t phasemoon_image_map[] = {
    {"800", &ui_img_icon_new_moon_36_png},
    {"801", &ui_img_icon_waxing_crescent_moon_36_png},
    {"802", &ui_img_icon_first_quarter_half_moon_36_png},
    {"803", &ui_img_icon_waxing_gibbous_moon_36_png},
    {"804", &ui_img_icon_full_moon_36_png},
    {"805", &ui_img_icon_waning_gibbous_moon_36_png},
    {"806", &ui_img_icon_last_quarter_half_moon_36_png},
    {"807", &ui_img_icon_waning_crescent_moon_36_png}};

static const int weather_image_map_size = sizeof(weather_image_map) / sizeof(weather_image_map_t);
static const int phasemoon_image_map_size = sizeof(phasemoon_image_map) / sizeof(weather_image_map_t);

/**
 * @brief   将星期转换为中文
 *
 * @param time_str       时间字符串
 * @param timeinfo       时间结构体
 */
void convert_weekday_to_chinese(char *time_str, struct tm *timeinfo)
{
    strftime(time_str, 16, "%u", timeinfo); // %u gives the day of the week (1-7, where 1 is Monday)
    int day_of_week = atoi(time_str) - 1;   // Convert to 0-6 index for the array
    // ESP_LOGI("ui_screen", "weekstr %s,int %d", time_str, day_of_week);
    snprintf(time_str, 16, "%s", weekday_map[day_of_week]);
}

/**
 * @brief   根据当前日期推算后三天的星期
 *
 * @param weekdays       存储后三天星期的数组，至少需要3个元素
 */
void get_next_three_weekdays(char weekdays[3][16])
{
    time_t now;
    struct tm timeinfo;
    time(&now);
    localtime_r(&now, &timeinfo);

    for (int i = 0; i < 3; i++)
    {
        timeinfo.tm_mday += 1;                      // 增加一天
        mktime(&timeinfo);                          // 重新计算时间结构体以处理月份和年份的进位
        strftime(weekdays[i], 16, "%u", &timeinfo); // %u gives the day of the week (1-7, where 1 is Monday)
        int day_of_week = atoi(weekdays[i]) - 1;    // Convert to 0-6 index for the array
        snprintf(weekdays[i], 16, "%s", weekday_map[day_of_week]);
    }
}

/**
 * @brief   刷新图表
 *
 * @param temp       温度
 * @param humi       湿度
 */
void fresh_temp_humi(float temp, float humi)
{
    char buf[16];
    snprintf(buf, sizeof(buf), "%ld%s", (int32_t)temp, "℃");
    lv_label_set_text(ui_LabTemp, buf);
    lv_bar_set_value(ui_BarTemp, (int32_t)temp, LV_ANIM_OFF);

    snprintf(buf, sizeof(buf), "%ld%s", (int32_t)humi, "%");
    lv_label_set_text(ui_LabHumi, buf);
    lv_bar_set_value(ui_BarHumi, (int32_t)humi, LV_ANIM_OFF);
}
/**
 * @brief   刷新时间
 *
 */
void fresh_time()
{
    time_t now;
    struct tm timeinfo;
    char time_str[16];
    time(&now);
    localtime_r(&now, &timeinfo);
    strftime(time_str, 16, "%H:%M", &timeinfo);
    lv_label_set_text(ui_LabTime, time_str);

    strftime(time_str, 16, "%Y-%m-%d", &timeinfo);
    lv_label_set_text(ui_LabDate, time_str);

    convert_weekday_to_chinese(time_str, &timeinfo);
    lv_label_set_text(ui_LabWeek, time_str);
}

/**
 * @brief   刷新天气信息
 *
 * @param weather_info      天气信息
 */
void fresh_weather(weather_info_t *weather_info)
{
    char buf[36];
    snprintf(buf, sizeof(buf), "%s%s%s%s%s", weather_info->windDir_n, weather_info->windScale_n, "级 风速 ", weather_info->windSpeed_n, "m/s");
    lv_label_set_text(ui_LabWind, buf);
}

/**
 * @brief   刷新空气质量信息
 *
 * @param air_info      空气质量信息
 */
void fresh_air(air_info_t *air_info)
{
    char buf[24];
    snprintf(buf, sizeof(buf), "%s", air_info->category_n);
    lv_label_set_text(ui_LabWeather, buf);
}

/**
 * @brief   解析日期字符串
 *
 * @param fxDate       日期字符串
 * @param parsed_date  存储解析结果的结构体
 */
void parse_date(const char *fxDate, parsed_date_t *parsed_date)
{
    // 解析日期字符串
    sscanf(fxDate, "%d-%d-%d", &parsed_date->year, &parsed_date->month, &parsed_date->day);
    ESP_LOGI("ui_screen", "year %d,month %d,day %d", parsed_date->year, parsed_date->month, parsed_date->day);

    time_t now;
    struct tm timeinfo;
    time(&now);
    localtime_r(&now, &timeinfo);

    timeinfo.tm_mday = parsed_date->day;
    timeinfo.tm_year = parsed_date->year;
    timeinfo.tm_mon = parsed_date->month - 1;
    mktime(&timeinfo); // 重新计算时间结构体以处理月份和年份的进位

    char buff[4] = {0};
    strftime(buff, 16, "%u", &timeinfo);   // %u gives the day of the week (1-7, where 1 is Monday)
    parsed_date->weekday = atoi(buff) - 1; // Convert to 0-6 index for the array
    snprintf(parsed_date->weekday_str, sizeof(parsed_date->weekday_str), "%s", weekday_map[parsed_date->weekday]);
    strftime(parsed_date->month_day_str, sizeof(parsed_date->month_day_str), "%m-%d", &timeinfo);
}

/**
 * @brief   刷新当天天气预报信息
 *
 * @param forecast      天气预报信息
 */
void fresh_first_day(daily_forecast_t *forecast)
{
    char buf[15];
    parsed_date_t parsed_date;
    parse_date(forecast[0].fxDate, &parsed_date);

    lv_label_set_text(ui_LabFirWeek, parsed_date.weekday_str);
    lv_label_set_text(ui_LabFirDate, parsed_date.month_day_str);

    snprintf(buf, sizeof(buf), "%s%s%s", forecast[0].tempMin, "~", forecast[0].tempMax);
    lv_label_set_text(ui_LabFirTemp, buf);
    snprintf(buf, sizeof(buf), "%s", forecast[0].textDay);
    lv_label_set_text(ui_LabFirWeather, buf);
    snprintf(buf, sizeof(buf), "%s", forecast[0].windDirDay);
    lv_label_set_text(ui_LabFirWind, buf);

    // 根据 weather_description 设置图像
    const lv_img_dsc_t *image = NULL;
    for (int i = 0; i < weather_image_map_size; i++)
    {
        if (strcmp(forecast[0].iconDay, weather_image_map[i].weather_description) == 0)
        {
            image = weather_image_map[i].image;
            break;
        }
    }

    // 如果找到了匹配的图像，则设置图像
    if (image != NULL)
    {
        lv_img_set_src(ui_ImgFirWeather, image);
    }
    else
    {
        // 如果没有找到匹配的图像，可以设置一个默认图像
        lv_img_set_src(ui_ImgFirWeather, &ui_img_icon_999_64_png);
    }
}

/**
 * @brief   刷新明天天气预报信息
 *
 * @param forecast      天气预报信息
 */
void fresh_second_day(daily_forecast_t *forecast)
{
    char buf[15];
    parsed_date_t parsed_date;
    parse_date(forecast[1].fxDate, &parsed_date);

    lv_label_set_text(ui_LabSecWeek, parsed_date.weekday_str);
    lv_label_set_text(ui_LabSecDate, parsed_date.month_day_str);

    snprintf(buf, sizeof(buf), "%s%s%s", forecast[1].tempMin, "~", forecast[1].tempMax);
    lv_label_set_text(ui_LabSecTemp, buf);
    snprintf(buf, sizeof(buf), "%s", forecast[1].textDay);
    lv_label_set_text(ui_LabSecWeather, buf);
    snprintf(buf, sizeof(buf), "%s", forecast[1].windDirDay);
    lv_label_set_text(ui_LabSecWind, buf);

    // 根据 weather_description 设置图像
    const lv_img_dsc_t *image = NULL;
    for (int i = 0; i < weather_image_map_size; i++)
    {
        if (strcmp(forecast[1].iconDay, weather_image_map[i].weather_description) == 0)
        {
            image = weather_image_map[i].image;
            break;
        }
    }

    // 如果找到了匹配的图像，则设置图像
    if (image != NULL)
    {
        lv_img_set_src(ui_ImgSecWeather, image);
    }
    else
    {
        // 如果没有找到匹配的图像，可以设置一个默认图像或不设置图像
        lv_img_set_src(ui_ImgSecWeather, &ui_img_icon_999_64_png); // 或者设置一个默认图像
    }
}

void fresh_refreshtime_labal()
{
    time_t now;
    struct tm timeinfo;
    char date_str[11];
    char time_str[6];
    char buf[42];
    time(&now);
    localtime_r(&now, &timeinfo);
    strftime(date_str, sizeof(date_str), "%Y-%m-%d", &timeinfo);
    strftime(time_str, sizeof(date_str), "%H:%M", &timeinfo);

    // 最后一次刷新时间 2025-01-24 18:30
    snprintf(buf, sizeof(buf), "%s %s %s", "最后一次刷新时间", date_str, time_str);
    lv_label_set_text(ui_LabRefreshTime, buf);
}
/**
 * @brief   刷新后天天气预报信息
 *
 * @param forecast      天气预报信息
 */
void fresh_third_day(daily_forecast_t *forecast)
{
    char buf[15];
    parsed_date_t parsed_date;
    parse_date(forecast[2].fxDate, &parsed_date);

    lv_label_set_text(ui_LabThiWeek, parsed_date.weekday_str);
    lv_label_set_text(ui_LabThiDate, parsed_date.month_day_str);

    snprintf(buf, sizeof(buf), "%s%s%s", forecast[2].tempMin, "~", forecast[2].tempMax);
    lv_label_set_text(ui_LabThiTemp, buf);
    snprintf(buf, sizeof(buf), "%s", forecast[2].textDay);
    lv_label_set_text(ui_LabThiWeather, buf);
    snprintf(buf, sizeof(buf), "%s", forecast[2].windDirDay);
    lv_label_set_text(ui_LabThiWind, buf);

    // 根据 weather_description 设置图像
    const lv_img_dsc_t *image = NULL;
    for (int i = 0; i < weather_image_map_size; i++)
    {
        if (strcmp(forecast[2].iconDay, weather_image_map[i].weather_description) == 0)
        {
            image = weather_image_map[i].image;
            break;
        }
    }

    // 如果找到了匹配的图像，则设置图像
    if (image != NULL)
    {
        lv_img_set_src(ui_ImgThiWeather, image);
    }
    else
    {
        // 如果没有找到匹配的图像，可以设置一个默认图像或不设置图像
        lv_img_set_src(ui_ImgThiWeather, &ui_img_icon_999_64_png); // 或者设置一个默认图像
    }
    fresh_refreshtime_labal();
}

void fresh_search_city()
{
    char buf[45];
    lv_obj_clear_flag(ui_LabInformation, LV_OBJ_FLAG_HIDDEN);
    is_search_in_progress = false; // 重置标志位
    if (num_cities == 0)
    {
        lv_label_set_text(ui_LabInformation, "未搜索到城市");
        lv_obj_add_flag(ui_ContainerCityLeft, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(ui_ContainerCityRight, LV_OBJ_FLAG_HIDDEN);
        num_cities = 0;
    }
    else if (num_cities == 1)
    {
        /* code */

        lv_label_set_text(ui_LabInformation, "为您搜索到1个城市");
        lv_obj_clear_flag(ui_ContainerCityLeft, LV_OBJ_FLAG_HIDDEN);
        snprintf(buf, sizeof(buf), "%s%s", "城市(区):", cities[0].name);
        lv_label_set_text(ui_LabCity1, buf);
        snprintf(buf, sizeof(buf), "%s%s", "上级行政区:", cities[0].adm2);
        lv_label_set_text(ui_LabSuperiorCity1, buf);
        lv_obj_add_flag(ui_ContainerCityRight, LV_OBJ_FLAG_HIDDEN);
        num_cities = 0;
    }
    else if (num_cities == 2)
    {
        lv_label_set_text(ui_LabInformation, "为您搜索到2个城市");
        lv_obj_clear_flag(ui_ContainerCityLeft, LV_OBJ_FLAG_HIDDEN);
        snprintf(buf, sizeof(buf), "%s%s", "城市(区):", cities[0].name);
        lv_label_set_text(ui_LabCity1, buf);
        snprintf(buf, sizeof(buf), "%s%s", "上级行政区:", cities[0].adm2);
        lv_label_set_text(ui_LabSuperiorCity1, buf);
        lv_obj_clear_flag(ui_ContainerCityRight, LV_OBJ_FLAG_HIDDEN);
        snprintf(buf, sizeof(buf), "%s%s", "城市(区):", cities[1].name);
        lv_label_set_text(ui_LabCity2, buf);
        snprintf(buf, sizeof(buf), "%s%s", "上级行政区:", cities[1].adm2);
        lv_label_set_text(ui_LabSuperiorCity2, buf);
        num_cities = 0;
    }
}

void btn_search_city(lv_event_t *e)
{
    // ESP_LOGI("ui_func", "btn_search_city");
    if (is_search_in_progress)
    { // 检查标志位
        lv_obj_t *ui_mbox1 = lv_msgbox_create(NULL, "warning", "Search is already in progress, ignoring new search request!", NULL, true);
        lv_obj_center(ui_mbox1);
        ESP_LOGI("ui_func", "Search is already in progress, ignoring new search request.");
        return;
    }

    city = lv_textarea_get_text(ui_TaLocal);
    adm = location_map[lv_dropdown_get_selected(ui_Dropdown1)];
    ESP_LOGI("ui_func", "lv_dropdown_get_text_index:%s", adm);
    if (city == NULL || strlen(city) == 0)
    {
        lv_obj_t *ui_mbox1 = lv_msgbox_create(NULL, "warning", "The input is empty !", NULL, true);
        lv_obj_center(ui_mbox1);
        ESP_LOGI("ui_func", "lv_textarea_get_text:NULL");
    }
    else
    {
        is_search_in_progress = true; // 设置标志位
        ESP_LOGI("ui_func", "lv_textarea_get_text:%s", city);
        xEventGroupSetBits(s_wifi_event_group, WIFI_LOCATION_SEARCH_BIT);
    }
}

void init_ui_data(lv_event_t *e)
{
    if (wifi_statu)
    {
        lv_obj_add_state(ui_SwiWifi, LV_STATE_DEFAULT | LV_STATE_CHECKED);
        _ui_image_set_property(ui_ImgWifiStatus, _UI_IMAGE_PROPERTY_IMAGE, &ui_img_icon_wifi_24_png);
        _ui_image_set_property(ui_ImgWifiStatus2, _UI_IMAGE_PROPERTY_IMAGE, &ui_img_icon_wifi_24_png);
    }
    fresh_time();
}

void local_screen_init(lv_event_t *e)
{
    lv_obj_add_flag(ui_LabInformation, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(ui_ContainerCityLeft, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(ui_ContainerCityRight, LV_OBJ_FLAG_HIDDEN);
}

void set_wifi_status_img(lv_event_t *e)
{
    lv_obj_t *target = lv_event_get_target(e);
    ESP_LOGI("ui_func", "set_wifi_status_img");
    if (lv_obj_has_state(target, LV_STATE_CHECKED))
    {
        _ui_image_set_property(ui_ImgWifiStatus, _UI_IMAGE_PROPERTY_IMAGE, &ui_img_icon_wifi_24_png);
        _ui_image_set_property(ui_ImgWifiStatus2, _UI_IMAGE_PROPERTY_IMAGE, &ui_img_icon_wifi_24_png);
    }
    else
    {
        _ui_image_set_property(ui_ImgWifiStatus, _UI_IMAGE_PROPERTY_IMAGE, &ui_img_icon_discon_wifi_24_png);
        _ui_image_set_property(ui_ImgWifiStatus2, _UI_IMAGE_PROPERTY_IMAGE, &ui_img_icon_discon_wifi_24_png);
    }
}

void set_city_setting1(lv_event_t *e)
{
    char buf[32];
    snprintf(buf, sizeof(buf), "%s", cities[0].name);
    lv_label_set_text(ui_LabLocal, buf);
    strncpy(location_id, cities[0].id, 9);
    lv_obj_t *ui_mbox1 = lv_msgbox_create(NULL, "message", "Reset to a new city!", NULL, true);
    lv_obj_center(ui_mbox1);
}
void set_city_setting2(lv_event_t *e)
{
    char buf[32];
    snprintf(buf, sizeof(buf), "%s", cities[1].name);
    lv_label_set_text(ui_LabLocal, buf);
    strncpy(location_id, cities[1].id, 9);
    lv_obj_t *ui_mbox1 = lv_msgbox_create(NULL, "message", "Reset to a new city!", NULL, true);
    lv_obj_center(ui_mbox1);
}

void fresh_dropdown_wifi_list_option(uint16_t max_ap)
{
    lv_dropdown_clear_options(ui_DropdownWifi);
    ESP_LOGI("max_ap", "max_ap: %u", max_ap);
    for (int i = 0; i < max_ap; i++)
    {
        lv_dropdown_add_option(ui_DropdownWifi, wifi_scan_result[i].wifi_ssid, i);
    }

    ESP_LOGI("Option", "Option: %u", lv_dropdown_get_option_cnt(ui_DropdownWifi));
}

void refresh_wifi_list(lv_event_t *e)
{
    ESP_LOGI("ui_func", "refresh_wifi_list");
    xEventGroupSetBits(s_wifi_event_group, WIFI_SCAN_BIT);
}
void set_wifi_list(lv_event_t *e)
{
    ESP_LOGI("ui_func", "set_wifi_list");
    lv_label_set_text_fmt(ui_LabWifiInformation, "当前连接WiFi: %s", wifi_ssid);
}
/**
 * @brief   刷新天气预报信息
 *
 * @param index 0~2
 */
void fresh_daily_information(uint8_t index)
{
    lv_label_set_text_fmt(ui_LabDailyDate, "%s", daily_forecast[index].fxDate);
    lv_label_set_text_fmt(ui_LabPhaseMoon, "%s", daily_forecast[index].moonPhase);
    lv_label_set_text_fmt(ui_LabDayWeather, "%s %s%s", daily_forecast[index].textDay, daily_forecast[index].tempMax, "℃");
    lv_label_set_text_fmt(ui_LabMoonWeather, "%s %s%s", daily_forecast[index].textNight, daily_forecast[index].tempMin, "℃");
    lv_label_set_text_fmt(ui_LabWind, "%s %s%s %s%s", daily_forecast[index].windDirDay, daily_forecast[index].windScaleDay, "级 风速", daily_forecast[index].windSpeedDay, "m/s");
    lv_label_set_text_fmt(ui_LabMoonWind, "%s %s%s %s%s", daily_forecast[index].windDirNight, daily_forecast[index].windScaleNight, "级 风速", daily_forecast[index].windSpeedNight, "m/s");
    lv_label_set_text_fmt(ui_LabSunrise, "%s  %s", "日出", daily_forecast[index].sunrise);
    lv_label_set_text_fmt(ui_LabSunset, "%s  %s", "日落", daily_forecast[index].sunset);
    lv_label_set_text_fmt(ui_LabMoonrise, "%s  %s", "月升", daily_forecast[index].moonrise);
    lv_label_set_text_fmt(ui_LabMoonset, "%s  %s", "月落", daily_forecast[index].moonset);

    // 根据 weather_description 设置图像
    const lv_img_dsc_t *image = NULL;
    for (int i = 0; i < weather_image_map_size; i++)
    {
        if (strcmp(daily_forecast[index].iconDay, weather_image_map[i].weather_description) == 0)
        {
            image = weather_image_map[i].image;
            break;
        }
    }

    // 如果找到了匹配的图像，则设置图像
    if (image != NULL)
    {
        lv_img_set_src(ui_ImgDayWeather, image);
    }
    else
    {
        // 如果没有找到匹配的图像，可以设置一个默认图像
        lv_img_set_src(ui_ImgFirWeather, &ui_img_icon_999_64_png);
    }
    image = NULL;
    for (int i = 0; i < weather_image_map_size; i++)
    {
        if (strcmp(daily_forecast[index].iconNight, weather_image_map[i].weather_description) == 0)
        {
            image = weather_image_map[i].image;
            break;
        }
    }
    if (image != NULL)
    {
        lv_img_set_src(ui_ImgNightWeather, image);
    }
    else
    {
        // 如果没有找到匹配的图像，可以设置一个默认图像
        lv_img_set_src(ui_ImgNightWeather, &ui_img_icon_999_64_png);
    }
    image = NULL;
    for (int i = 0; i < phasemoon_image_map_size; i++)
    {
        if (strcmp(daily_forecast[index].moonPhaseIcon, phasemoon_image_map[i].weather_description) == 0)
        {
            image = phasemoon_image_map[i].image;
            break;
        }
    }
    if (image != NULL)
    {
        lv_img_set_src(ui_ImgPhaseMoon, image);
    }
    else
    {
        // 如果没有找到匹配的图像，可以设置一个默认图像
        lv_img_set_src(ui_ImgPhaseMoon, &ui_img_icon_999_64_png);
    }
}
void set_daily_information(lv_event_t *e)
{
    lv_obj_t *target = lv_event_get_target(e);
    if (target == ui_BtnFirst)
    {
        LV_LOG_USER("ui_BtnFirst set_daily_information");
        fresh_daily_information(0);
    }
    else if (target == ui_BtnSecond)
    {
        LV_LOG_USER("ui_BtnSecond set_daily_information");
        fresh_daily_information(1);
    }
    else if (target == ui_BtnThird)
    {
        LV_LOG_USER("ui_BtnThird set_daily_information");
        fresh_daily_information(2);
    }
    else
    {
        LV_LOG_USER("NULL set_daily_information");
    }
}