#include "bsp_gxhtc3.h"
#include "bsp_myi2c.h"
#include "driver/i2c.h"
#include <math.h>

#define POLYNOMIAL 0x31 // P(x) = x^8 + x^5 + x^4 + 1 = 00110001

static const char *TAG = "GXHTC3";

uint8_t tah_data[6];

esp_err_t gxhtc3_read_id(void);

esp_err_t gxhtc3_wake_up(void);

esp_err_t gxhtc3_measure(void);

esp_err_t gxhtc3_read_tah(void);

esp_err_t gxhtc3_sleep(void);

uint8_t gxhtc3_calc_crc(uint8_t *crcdata, uint8_t len);

// I2C地址 0x70

// 读取id号 0XEFC8  1110 1111 1100 1000

/*
每一次读取数据，都会经过这四个过程
唤醒 0X3517 0011 0101 0001 0111
测量
读出
休眠 0xB098 1011 0000 1001 1000
*/

/**
 * @brief gxhtc3_get_tah 获取温湿度
 *
 * @param temp 温度
 * @param humi 湿度
 *
 * @return esp_err_t
 */
esp_err_t gxhtc3_get_tah(float *temp, float *humi)
{
    int ret;
    uint16_t rawValueTemp, rawValueHumi;
    uint8_t temp_int, humi_int;

    gxhtc3_wake_up();
    gxhtc3_measure();
    vTaskDelay(20 / portTICK_PERIOD_MS);
    gxhtc3_read_tah();
    gxhtc3_sleep();

    if ((tah_data[2] != gxhtc3_calc_crc(tah_data, 2) || (tah_data[5] != gxhtc3_calc_crc(&tah_data[3], 2))))
    {
        temp = 0;
        humi = 0;
        temp_int = 0;
        humi_int = 0;
        ret = ESP_FAIL;
    }
    else
    {
        rawValueTemp = (tah_data[0] << 8) | tah_data[1];
        rawValueHumi = (tah_data[3] << 8) | tah_data[4];
        *temp = (175.0 * (float)rawValueTemp) / 65535.0 - 45.0;
        *humi = (100.0 * (float)rawValueHumi) / 65535.0;
        temp_int = round(*temp);
        humi_int = round(*humi);
        ret = ESP_OK;
    }
    return ret;
}

/**
 * @brief gxhtc3_read_id 读取ID
 *
 * @return esp_err_t
 */
esp_err_t gxhtc3_read_id(void)
{
    esp_err_t ret;
    uint8_t data[3];

    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, 0x70 << 1 | I2C_MASTER_WRITE, true);
    i2c_master_write_byte(cmd, 0xEF, true);
    i2c_master_write_byte(cmd, 0xC8, true);
    i2c_master_stop(cmd);

    ret = i2c_master_cmd_begin(I2C_MASTER_NUM, cmd, 1000 / portTICK_PERIOD_MS);
    if (ret != ESP_OK)
    {
        goto end;
    }
    cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, 0x70 << 1 | I2C_MASTER_READ, true);
    i2c_master_read(cmd, data, 3, I2C_MASTER_LAST_NACK);
    i2c_master_stop(cmd);

    ret = i2c_master_cmd_begin(I2C_MASTER_NUM, cmd, 1000 / portTICK_PERIOD_MS);

    if (data[2] != gxhtc3_calc_crc(data, 2))
    {
        ret = ESP_FAIL;
    }
end:
    i2c_cmd_link_delete(cmd);

    return ret;
}

/**
 * @brief gxhtc3_calc_crc 计算CRC
 *
 * @param crcdata
 * @param len
 * @return uint8_t
 */
uint8_t gxhtc3_calc_crc(uint8_t *crcdata, uint8_t len)
{
    uint8_t crc = 0xFF;

    for (uint8_t i = 0; i < len; i++)
    {
        crc ^= (crcdata[i]);
        for (uint8_t j = 8; j > 0; --j)
        {
            if (crc & 0x80)
                crc = (crc << 1) ^ POLYNOMIAL;
            else
                crc = (crc << 1);
        }
    }
    return crc;
}

/**
 * @brief gxhtc3_wake_up 唤醒
 *
 * @return esp_err_t
 */
esp_err_t gxhtc3_wake_up(void)
{
    int ret;

    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, 0x70 << 1 | I2C_MASTER_WRITE, true);
    i2c_master_write_byte(cmd, 0x35, true);
    i2c_master_write_byte(cmd, 0x17, true);
    i2c_master_stop(cmd);

    ret = i2c_master_cmd_begin(I2C_MASTER_NUM, cmd, 1000 / portTICK_PERIOD_MS);

    i2c_cmd_link_delete(cmd);
    return ret;
}

/**
 * @brief gxhtc3_measure 测量
 *
 * @return esp_err_t
 */
esp_err_t gxhtc3_measure(void)
{
    int ret;

    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, 0x70 << 1 | I2C_MASTER_WRITE, true);
    i2c_master_write_byte(cmd, 0x7c, true);
    i2c_master_write_byte(cmd, 0xa2, true);
    i2c_master_stop(cmd);

    ret = i2c_master_cmd_begin(I2C_MASTER_NUM, cmd, 1000 / portTICK_PERIOD_MS);

    i2c_cmd_link_delete(cmd);
    return ret;
}

/**
 * @brief gxhtc3_read_tah 读取温湿度数据
 *
 * @return esp_err_t
 */
esp_err_t gxhtc3_read_tah(void)
{
    int ret;

    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, 0x70 << 1 | I2C_MASTER_READ, true);
    i2c_master_read(cmd, tah_data, 6, I2C_MASTER_LAST_NACK);
    i2c_master_stop(cmd);

    ret = i2c_master_cmd_begin(I2C_MASTER_NUM, cmd, 1000 / portTICK_PERIOD_MS);

    i2c_cmd_link_delete(cmd);
    return ret;
}

/**
 * @brief gxhtc3_sleep 休眠
 *
 * @return esp_err_t
 */
esp_err_t gxhtc3_sleep(void)
{
    int ret;

    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, 0x70 << 1 | I2C_MASTER_WRITE, true);
    i2c_master_write_byte(cmd, 0xB0, true);
    i2c_master_write_byte(cmd, 0x98, true);
    i2c_master_stop(cmd);

    ret = i2c_master_cmd_begin(I2C_MASTER_NUM, cmd, 1000 / portTICK_PERIOD_MS);

    i2c_cmd_link_delete(cmd);
    return ret;
}

/**
 * @brief gxhtc3_init
 *
 * @return esp_err_t
 */
esp_err_t gxhtc3_init(void)
{
    esp_err_t ret = gxhtc3_read_id();
    while (ret != ESP_OK)
    {
        ret = gxhtc3_read_id();
        ESP_LOGI(TAG, "GXHTC3 READ ID");
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
    ESP_LOGI(TAG, "GXHTC3 OK");
    return ret;
}