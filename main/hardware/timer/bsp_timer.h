#pragma once

#include "driver/gptimer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include <time.h>
#include "freertos/queue.h"

#define RESOLUTION_HZ 1000000 // 定时器的分辨率
#define ALARM_COUNT 1000000   // 触发中断的目标计数值

/**
 * @函数说明        定时器初始化配置
 * @函数返回        创建的定时器回调队列
 */
void init_timer();

// bool getLocalTime(struct tm *info, uint32_t ms);
