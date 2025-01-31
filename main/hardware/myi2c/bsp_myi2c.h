#pragma once
#include "esp_err.h"

#define I2C_MASTER_SCL_IO GPIO_NUM_1 /*!< GPIO number used for I2C master clock  */
#define I2C_MASTER_SDA_IO GPIO_NUM_0 /*!< GPIO number used for I2C master data  */
#define I2C_MASTER_NUM 0             /*!< I2C 0，c3只有一路 */
#define I2C_MASTER_FREQ_HZ 400000    /*!< I2C 通信速率*/
#define I2C_MASTER_TX_BUF_DISABLE 0  /*!< I2C master doesn't need buffer */
#define I2C_MASTER_RX_BUF_DISABLE 0  /*!< I2C master doesn't need buffer */
#define I2C_MASTER_TIMEOUT_MS 1000   /*!< I2C 最大超时时间*/

esp_err_t i2c_master_init(void);
