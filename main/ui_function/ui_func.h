#ifndef UI_FUNC_H
#define UI_FUNC_H

#include "ui.h"
#include "hf_weather.h"

extern char *city;
extern char *adm;
extern int num_cities;
void fresh_time();
void fresh_temp_humi(float temp, float humi);
void fresh_weather(weather_info_t *weather_info);
void fresh_air(air_info_t *air_info);
void fresh_first_day(daily_forecast_t *forecast);
void fresh_second_day(daily_forecast_t *forecast);
void fresh_third_day(daily_forecast_t *forecast);
void fresh_search_city();
void fresh_dropdown_wifi_list_option(uint16_t max_ap);

#endif
