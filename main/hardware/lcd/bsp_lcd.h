#pragma once

#include "lvgl.h"
#include "driver/gpio.h"
#include "driver/spi_master.h"
#include "esp_log.h"
#include "esp_lcd_touch_ft5x06.h"
#include "esp_timer.h"
#include "esp_lcd_panel_io.h"
#include "esp_lcd_panel_vendor.h"
#include "esp_lcd_panel_ops.h"

typedef enum
{
    BSP_LCD_ROTATE_0 = 0,
    BSP_LCD_ROTATE_90,
    BSP_LCD_ROTATE_180,
    BSP_LCD_ROTATE_270,
} bsp_display_rotation_t;

// Using SPI2 in the example
#define LCD_HOST SPI2_HOST

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////// Please update the following configuration according to your LCD spec //////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#define EXAMPLE_LCD_PIXEL_CLOCK_HZ (20 * 1000 * 1000)
#define EXAMPLE_LCD_BK_LIGHT_ON_LEVEL 0
#define EXAMPLE_LCD_BK_LIGHT_OFF_LEVEL !EXAMPLE_LCD_BK_LIGHT_ON_LEVEL
#define EXAMPLE_PIN_NUM_SCLK 3
#define EXAMPLE_PIN_NUM_MOSI 5
#define EXAMPLE_PIN_NUM_MISO -1
#define EXAMPLE_PIN_NUM_LCD_DC 6
#define EXAMPLE_PIN_NUM_LCD_RST -1
#define EXAMPLE_PIN_NUM_LCD_CS 4
#define EXAMPLE_PIN_NUM_BK_LIGHT 2
#define EXAMPLE_PIN_NUM_TOUCH_CS -1

#define EXAMPLE_LCD_H_RES 240
#define EXAMPLE_LCD_V_RES 320

#define EXAMPLE_LCD_CMD_BITS 8
#define EXAMPLE_LCD_PARAM_BITS 8

#define EXAMPLE_LVGL_TICK_PERIOD_MS 2

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void example_lcd_init(int direction);
