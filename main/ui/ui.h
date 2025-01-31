// This file was generated by SquareLine Studio
// SquareLine Studio version: SquareLine Studio 1.4.2
// LVGL version: 8.3.11
// Project name: SquareLine_Project

#ifndef _SQUARELINE_PROJECT_UI_H
#define _SQUARELINE_PROJECT_UI_H

#ifdef __cplusplus
extern "C" {
#endif

#include "lvgl.h"

#include "ui_helpers.h"
#include "ui_events.h"

// SCREEN: ui_MainScreen
void ui_MainScreen_screen_init(void);
void ui_event_MainScreen(lv_event_t * e);
extern lv_obj_t * ui_MainScreen;
extern lv_obj_t * ui_MainContainer;
extern lv_obj_t * ui_NowContainer;
extern lv_obj_t * ui_Container7;
void ui_event_BtnLocal(lv_event_t * e);
extern lv_obj_t * ui_BtnLocal;
extern lv_obj_t * ui_LabLocal;
extern lv_obj_t * ui_LabWeather;
extern lv_obj_t * ui_Container9;
extern lv_obj_t * ui_LabWind;
void ui_event_BtnWifi(lv_event_t * e);
extern lv_obj_t * ui_BtnWifi;
extern lv_obj_t * ui_ImgWifiStatus;
extern lv_obj_t * ui_Container8;
extern lv_obj_t * ui_LabTime;
extern lv_obj_t * ui_Container10;
extern lv_obj_t * ui_LabDate;
extern lv_obj_t * ui_LabWeek;
extern lv_obj_t * ui_Container11;
extern lv_obj_t * ui_ImgTemp;
extern lv_obj_t * ui_BarTemp;
extern lv_obj_t * ui_LabTemp;
extern lv_obj_t * ui_Container12;
extern lv_obj_t * ui_ImgHumi;
extern lv_obj_t * ui_BarHumi;
extern lv_obj_t * ui_LabHumi;
extern lv_obj_t * ui_FutureContainer;
extern lv_obj_t * ui_FirstContainer;
void ui_event_BtnFirst(lv_event_t * e);
extern lv_obj_t * ui_BtnFirst;
extern lv_obj_t * ui_LabFirWeek;
extern lv_obj_t * ui_LabFirDate;
extern lv_obj_t * ui_ImgFirWeather;
extern lv_obj_t * ui_LabFirTemp;
extern lv_obj_t * ui_LabFirWeather;
extern lv_obj_t * ui_LabFirWind;
extern lv_obj_t * ui_SecondContainer;
void ui_event_BtnSecond(lv_event_t * e);
extern lv_obj_t * ui_BtnSecond;
extern lv_obj_t * ui_LabSecWeek;
extern lv_obj_t * ui_LabSecDate;
extern lv_obj_t * ui_ImgSecWeather;
extern lv_obj_t * ui_LabSecTemp;
extern lv_obj_t * ui_LabSecWeather;
extern lv_obj_t * ui_LabSecWind;
extern lv_obj_t * ui_ThirdContainer;
void ui_event_BtnThird(lv_event_t * e);
extern lv_obj_t * ui_BtnThird;
extern lv_obj_t * ui_LabThiWeek;
extern lv_obj_t * ui_LabThiDate;
extern lv_obj_t * ui_ImgThiWeather;
extern lv_obj_t * ui_LabThiTemp;
extern lv_obj_t * ui_LabThiWeather;
extern lv_obj_t * ui_LabThiWind;
// SCREEN: ui_SetLocalScreen
void ui_SetLocalScreen_screen_init(void);
void ui_event_SetLocalScreen(lv_event_t * e);
extern lv_obj_t * ui_SetLocalScreen;
extern lv_obj_t * ui_MainContainer1;
extern lv_obj_t * ui_Container2;
void ui_event_BtnBack1(lv_event_t * e);
extern lv_obj_t * ui_BtnBack1;
extern lv_obj_t * ui_ImgBack1;
extern lv_obj_t * ui_Label1;
extern lv_obj_t * ui_Container3;
extern lv_obj_t * ui_Container4;
extern lv_obj_t * ui_Dropdown1;
void ui_event_TaLocal(lv_event_t * e);
extern lv_obj_t * ui_TaLocal;
void ui_event_BtnSearchCity(lv_event_t * e);
extern lv_obj_t * ui_BtnSearchCity;
extern lv_obj_t * ui_Image1;
extern lv_obj_t * ui_Container20;
extern lv_obj_t * ui_LabInformation;
extern lv_obj_t * ui_Container5;
extern lv_obj_t * ui_ContainerCityLeft;
extern lv_obj_t * ui_LabCity1;
extern lv_obj_t * ui_LabSuperiorCity1;
void ui_event_BtnSearch1(lv_event_t * e);
extern lv_obj_t * ui_BtnSearch1;
extern lv_obj_t * ui_Image3;
extern lv_obj_t * ui_ContainerCityRight;
extern lv_obj_t * ui_LabCity2;
extern lv_obj_t * ui_LabSuperiorCity2;
void ui_event_BtnSearch2(lv_event_t * e);
extern lv_obj_t * ui_BtnSearch2;
extern lv_obj_t * ui_Image4;
extern lv_obj_t * ui_KeybLocal;
// SCREEN: ui_SetWifiScreen
void ui_SetWifiScreen_screen_init(void);
void ui_event_SetWifiScreen(lv_event_t * e);
extern lv_obj_t * ui_SetWifiScreen;
extern lv_obj_t * ui_MainContainer3;
extern lv_obj_t * ui_Container19;
void ui_event_BtnBack2(lv_event_t * e);
extern lv_obj_t * ui_BtnBack2;
extern lv_obj_t * ui_ImgBack2;
extern lv_obj_t * ui_Label3;
extern lv_obj_t * ui_ImgWifiStatus2;
extern lv_obj_t * ui_Container1;
extern lv_obj_t * ui_Container14;
extern lv_obj_t * ui_DropdownWifi;
void ui_event_BtnRefreshWifi(lv_event_t * e);
extern lv_obj_t * ui_BtnRefreshWifi;
extern lv_obj_t * ui_Image2;
void ui_event_TaWifiPassword(lv_event_t * e);
extern lv_obj_t * ui_TaWifiPassword;
void ui_event_SwiWifi(lv_event_t * e);
extern lv_obj_t * ui_SwiWifi;
extern lv_obj_t * ui_Container13;
extern lv_obj_t * ui_LabWifiInformation;
extern lv_obj_t * ui_Container15;
extern lv_obj_t * ui_Container16;
extern lv_obj_t * ui_Container17;
extern lv_obj_t * ui_Keyboard2;
// SCREEN: ui_WeatherScreen
void ui_WeatherScreen_screen_init(void);
extern lv_obj_t * ui_WeatherScreen;
extern lv_obj_t * ui_MainContainer4;
extern lv_obj_t * ui_Container25;
void ui_event_BtnBack4(lv_event_t * e);
extern lv_obj_t * ui_BtnBack4;
extern lv_obj_t * ui_ImgBack4;
extern lv_obj_t * ui_Label4;
extern lv_obj_t * ui_LabRefreshTime;
extern lv_obj_t * ui_Container26;
extern lv_obj_t * ui_LabDailyDate;
extern lv_obj_t * ui_LabPhaseMoon;
extern lv_obj_t * ui_ImgPhaseMoon;
extern lv_obj_t * ui_Container18;
extern lv_obj_t * ui_Container6;
extern lv_obj_t * ui_Container21;
extern lv_obj_t * ui_ImgDayWeather;
extern lv_obj_t * ui_Container23;
extern lv_obj_t * ui_LabDayWeather;
extern lv_obj_t * ui_LabDayWind;
extern lv_obj_t * ui_Container22;
extern lv_obj_t * ui_LabSunrise;
extern lv_obj_t * ui_LabSunset;
extern lv_obj_t * ui_Container31;
extern lv_obj_t * ui_Container27;
extern lv_obj_t * ui_ImgNightWeather;
extern lv_obj_t * ui_Container28;
extern lv_obj_t * ui_LabMoonWeather;
extern lv_obj_t * ui_LabMoonWind;
extern lv_obj_t * ui_Container29;
extern lv_obj_t * ui_LabMoonrise;
extern lv_obj_t * ui_LabMoonset;
extern lv_obj_t * ui____initial_actions0;


LV_IMG_DECLARE(ui_img_icon_discon_wifi_24_png);    // assets/icon_discon_wifi_24.png
LV_IMG_DECLARE(ui_img_img_temp_30_png);    // assets/img_temp_30.png
LV_IMG_DECLARE(ui_img_img_humi_30_png);    // assets/img_humi_30.png
LV_IMG_DECLARE(ui_img_icon_101_64_png);    // assets/icon_101_64.png
LV_IMG_DECLARE(ui_img_icon_100_64_png);    // assets/icon_100_64.png
LV_IMG_DECLARE(ui_img_icon_back_24_png);    // assets/icon_back_24.png
LV_IMG_DECLARE(ui_img_icon_search_24_png);    // assets/icon_search_24.png
LV_IMG_DECLARE(ui_img_icon_choose_32_png);    // assets/icon_choose_32.png
LV_IMG_DECLARE(ui_img_icon_refresh_24_png);    // assets/icon_refresh_24.png
LV_IMG_DECLARE(ui_img_icon_first_quarter_half_moon_36_png);    // assets/icon_first_quarter_half_moon_36.png
LV_IMG_DECLARE(ui_img_icon_102_64_png);    // assets/icon_102_64.png
LV_IMG_DECLARE(ui_img_icon_103_64_png);    // assets/icon_103_64.png
LV_IMG_DECLARE(ui_img_icon_104_64_png);    // assets/icon_104_64.png
LV_IMG_DECLARE(ui_img_icon_150_64_png);    // assets/icon_150_64.png
LV_IMG_DECLARE(ui_img_icon_153_64_png);    // assets/icon_153_64.png
LV_IMG_DECLARE(ui_img_icon_154_64_png);    // assets/icon_154_64.png
LV_IMG_DECLARE(ui_img_icon_300_64_png);    // assets/icon_300_64.png
LV_IMG_DECLARE(ui_img_icon_301_64_png);    // assets/icon_301_64.png
LV_IMG_DECLARE(ui_img_icon_302_64_png);    // assets/icon_302_64.png
LV_IMG_DECLARE(ui_img_icon_303_64_png);    // assets/icon_303_64.png
LV_IMG_DECLARE(ui_img_icon_304_64_png);    // assets/icon_304_64.png
LV_IMG_DECLARE(ui_img_icon_305_64_png);    // assets/icon_305_64.png
LV_IMG_DECLARE(ui_img_icon_306_64_png);    // assets/icon_306_64.png
LV_IMG_DECLARE(ui_img_icon_307_64_png);    // assets/icon_307_64.png
LV_IMG_DECLARE(ui_img_icon_308_64_png);    // assets/icon_308_64.png
LV_IMG_DECLARE(ui_img_icon_309_64_png);    // assets/icon_309_64.png
LV_IMG_DECLARE(ui_img_icon_310_64_png);    // assets/icon_310_64.png
LV_IMG_DECLARE(ui_img_icon_311_64_png);    // assets/icon_311_64.png
LV_IMG_DECLARE(ui_img_icon_312_64_png);    // assets/icon_312_64.png
LV_IMG_DECLARE(ui_img_icon_313_64_png);    // assets/icon_313_64.png
LV_IMG_DECLARE(ui_img_icon_314_64_png);    // assets/icon_314_64.png
LV_IMG_DECLARE(ui_img_icon_315_64_png);    // assets/icon_315_64.png
LV_IMG_DECLARE(ui_img_icon_316_64_png);    // assets/icon_316_64.png
LV_IMG_DECLARE(ui_img_icon_317_64_png);    // assets/icon_317_64.png
LV_IMG_DECLARE(ui_img_icon_318_64_png);    // assets/icon_318_64.png
LV_IMG_DECLARE(ui_img_icon_350_64_png);    // assets/icon_350_64.png
LV_IMG_DECLARE(ui_img_icon_351_64_png);    // assets/icon_351_64.png
LV_IMG_DECLARE(ui_img_icon_399_64_png);    // assets/icon_399_64.png
LV_IMG_DECLARE(ui_img_icon_400_64_png);    // assets/icon_400_64.png
LV_IMG_DECLARE(ui_img_icon_401_64_png);    // assets/icon_401_64.png
LV_IMG_DECLARE(ui_img_icon_402_64_png);    // assets/icon_402_64.png
LV_IMG_DECLARE(ui_img_icon_403_64_png);    // assets/icon_403_64.png
LV_IMG_DECLARE(ui_img_icon_404_64_png);    // assets/icon_404_64.png
LV_IMG_DECLARE(ui_img_icon_405_64_png);    // assets/icon_405_64.png
LV_IMG_DECLARE(ui_img_icon_406_64_png);    // assets/icon_406_64.png
LV_IMG_DECLARE(ui_img_icon_407_64_png);    // assets/icon_407_64.png
LV_IMG_DECLARE(ui_img_icon_408_64_png);    // assets/icon_408_64.png
LV_IMG_DECLARE(ui_img_icon_409_64_png);    // assets/icon_409_64.png
LV_IMG_DECLARE(ui_img_icon_410_64_png);    // assets/icon_410_64.png
LV_IMG_DECLARE(ui_img_icon_456_64_png);    // assets/icon_456_64.png
LV_IMG_DECLARE(ui_img_icon_457_64_png);    // assets/icon_457_64.png
LV_IMG_DECLARE(ui_img_icon_499_64_png);    // assets/icon_499_64.png
LV_IMG_DECLARE(ui_img_icon_500_64_png);    // assets/icon_500_64.png
LV_IMG_DECLARE(ui_img_icon_501_64_png);    // assets/icon_501_64.png
LV_IMG_DECLARE(ui_img_icon_502_64_png);    // assets/icon_502_64.png
LV_IMG_DECLARE(ui_img_icon_503_64_png);    // assets/icon_503_64.png
LV_IMG_DECLARE(ui_img_icon_504_64_png);    // assets/icon_504_64.png
LV_IMG_DECLARE(ui_img_icon_507_64_png);    // assets/icon_507_64.png
LV_IMG_DECLARE(ui_img_icon_508_64_png);    // assets/icon_508_64.png
LV_IMG_DECLARE(ui_img_icon_509_64_png);    // assets/icon_509_64.png
LV_IMG_DECLARE(ui_img_icon_510_64_png);    // assets/icon_510_64.png
LV_IMG_DECLARE(ui_img_icon_511_64_png);    // assets/icon_511_64.png
LV_IMG_DECLARE(ui_img_icon_512_64_png);    // assets/icon_512_64.png
LV_IMG_DECLARE(ui_img_icon_513_64_png);    // assets/icon_513_64.png
LV_IMG_DECLARE(ui_img_icon_514_64_png);    // assets/icon_514_64.png
LV_IMG_DECLARE(ui_img_icon_515_64_png);    // assets/icon_515_64.png
LV_IMG_DECLARE(ui_img_icon_900_64_png);    // assets/icon_900_64.png
LV_IMG_DECLARE(ui_img_icon_901_64_png);    // assets/icon_901_64.png
LV_IMG_DECLARE(ui_img_icon_999_64_png);    // assets/icon_999_64.png
LV_IMG_DECLARE(ui_img_icon_connect_24_png);    // assets/icon_connect_24.png
LV_IMG_DECLARE(ui_img_icon_full_moon_36_png);    // assets/icon_full_moon_36.png
LV_IMG_DECLARE(ui_img_icon_last_quarter_half_moon_36_png);    // assets/icon_last_quarter_half_moon_36.png
LV_IMG_DECLARE(ui_img_icon_new_moon_36_png);    // assets/icon_new_moon_36.png
LV_IMG_DECLARE(ui_img_icon_waning_crescent_moon_36_png);    // assets/icon_waning_crescent_moon_36.png
LV_IMG_DECLARE(ui_img_icon_waning_gibbous_moon_36_png);    // assets/icon_waning_gibbous_moon_36.png
LV_IMG_DECLARE(ui_img_icon_waxing_crescent_moon_36_png);    // assets/icon_waxing_crescent_moon_36.png
LV_IMG_DECLARE(ui_img_icon_waxing_gibbous_moon_36_png);    // assets/icon_waxing_gibbous_moon_36.png
LV_IMG_DECLARE(ui_img_icon_wifi_24_png);    // assets/icon_wifi_24.png



LV_FONT_DECLARE(ui_font_Font20);



void ui_init(void);

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif
