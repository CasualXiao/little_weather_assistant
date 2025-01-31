#pragma once

#define KEY "ac81c985a21143c1a8cd252a7a876bb8"
// 天气数据结构体
typedef struct
{
    char temp_n[4];      // 当前温度  "26"
    char humidity_n[4];  // 当前湿度  "27"
    char feelsLike_n[4]; // 体感温度  "27"
    char precip_n[4];    // 降水量       "0.0"
    char icon_n[4];      // 天气图标  "101"
    char windScale_n[4]; // 风力等级    "2"
    char windDir_n[12];  // 风向      "南风"
    char weather_n[8];   // 天气     "多云"
    char windSpeed_n[4]; // 风速      "8"
} weather_info_t;

// 空气数据结构体 "level": "2",
typedef struct
{
    char aqi_n[4];       // "aqi": "67",
    char pm10_n[4];      // "pm10": "74",
    char pm2p5_n[4];     // "pm2p5": "48",
    char no2_n[4];       // "no2": "18",
    char so2_n[4];       // "so2": "5",
    char co_n[4];        // "co": "0.9",
    char category_n[24]; // "category": "良",
    char primary_n[8];   // "primary": "PM2.5",
    char o3_n[4];        // "o3": "63"
} air_info_t;

typedef struct
{
    char fxDate[12];       // 日期 "2025-01-17"
    char moonPhase[12];    // 月相 "亏凸月"
    char textDay[12];      // 白天天气描述 "晴"
    char windDirDay[12];   // 白天风向 "西北风"
    char textNight[12];    // 夜间天气描述 "多云"
    char windDirNight[12]; // 夜间风向 "西风"

    char precip[8];        // 降水量 "0.0"
    char moonPhaseIcon[4]; // 月相图标 "805"

    char pressure[8]; // 气压 "1021"
    char tempMax[4];  // 最高温度 "8"

    char sunrise[8]; // 日出时间 "07:30"
    char tempMin[4]; // 最低温度 "-3"

    char sunset[8];  // 日落时间 "17:15"
    char iconDay[4]; // 白天天气图标 "100"

    char moonrise[8];  // 月升时间 "20:46"
    char iconNight[4]; // 夜间天气图标 "151"

    char moonset[8];    // 月落时间 "09:40"
    char wind360Day[4]; // 白天风向角度 "225"

    char windScaleNight[8]; // 夜间风力等级 "1-3"
    char windSpeedNight[4]; // 夜间风速 "3"

    char windScaleDay[8]; // 白天风力等级 "1-3"
    char windSpeedDay[4]; // 白天风速 "3"

    char wind360Night[4]; // 夜间风向角度 "180"
    char humidity[4];     // 湿度 "29"
    char vis[4];          // 能见度 "25"
    char cloud[4];        // 云量 "0"
    char uvIndex[4];      // 紫外线指数 "3"
} daily_forecast_t;

typedef struct
{
    char name[32];
    char id[16];
    char adm2[16];
} city_info_t;
extern weather_info_t weather_now;
extern air_info_t air_now;
extern daily_forecast_t daily_forecast[3];
extern city_info_t cities[2];
extern char location_id[];
void weather_now_url(void);
void weather_air_url(void);
void weather_forecast_get(void);
void weather_location_get(const char *location, const char *adm, int *num_cities);