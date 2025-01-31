#pragma once
#include "esp_err.h"
#include "esp_log.h"

esp_err_t gxhtc3_init(void);

esp_err_t gxhtc3_get_tah(float *temp, float *humi);
