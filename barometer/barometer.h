#ifndef BAROMETER_H
#define BAROMETER_H

#include <stdint.h>
#include "errcode.h"
#include <stdio.h>
#include <time.h>
#include <string.h>
#include <stdlib.h>
/* 器件地址 */
#define SPL06_I2C_ADDR                  (0x76)  /* SDO = 0 地址为：0X76, SDO = 1 地址为：0X77 */
/* 寄存器地址 */
#define SPL06_DEFAULT_CHIP_ID           (0x10)

#define SPL06_PRESSURE_MSB_REG          (0x00)  /* Pressure MSB Register */
#define SPL06_PRESSURE_LSB_REG          (0x01)  /* Pressure LSB Register */
#define SPL06_PRESSURE_XLSB_REG         (0x02)  /* Pressure XLSB Register */
#define SPL06_TEMPERATURE_MSB_REG       (0x03)  /* Temperature MSB Reg */
#define SPL06_TEMPERATURE_LSB_REG       (0x04)  /* Temperature LSB Reg */
#define SPL06_TEMPERATURE_XLSB_REG      (0x05)  /* Temperature XLSB Reg */
#define SPL06_PRESSURE_CFG_REG          (0x06)  /* Pressure configuration Reg */
#define SPL06_TEMPERATURE_CFG_REG       (0x07)  /* Temperature configuration Reg */
#define SPL06_MODE_CFG_REG              (0x08)  /* Mode and Status Configuration */
#define SPL06_INT_FIFO_CFG_REG          (0x09)  /* Interrupt and FIFO Configuration */
#define SPL06_INT_STATUS_REG            (0x0A)  /* Interrupt Status Reg */
#define SPL06_FIFO_STATUS_REG           (0x0B)  /* FIFO Status Reg */
#define SPL06_RST_REG                   (0x0C)  /* Softreset Register */
#define SPL06_CHIP_ID                   (0x0D)  /* Chip ID Register */
#define SPL06_COEFFICIENT_CALIB_REG     (0x10)  /* Coeffcient calibraion Register */

#define SPL06_CALIB_COEFFICIENT_LENGTH  (18)
#define SPL06_DATA_FRAME_SIZE           (6)

#define SPL06_CONTINUOUS_MODE           (0x07)

#define TEMPERATURE_INTERNAL_SENSOR     (0)
#define TEMPERATURE_EXTERNAL_SENSOR     (1)

/* 测量次数 times / S */
#define SPL06_MWASURE_1                 (0x00)
#define SPL06_MWASURE_2                 (0x01)
#define SPL06_MWASURE_4                 (0x02)
#define SPL06_MWASURE_8                 (0x03)
#define SPL06_MWASURE_16                (0x04)
#define SPL06_MWASURE_32                (0x05)
#define SPL06_MWASURE_64                (0x06)
#define SPL06_MWASURE_128               (0x07)

/* 过采样率 */
#define SPL06_OVERSAMP_1                (0x00)
#define SPL06_OVERSAMP_2                (0x01)
#define SPL06_OVERSAMP_4                (0x02)
#define SPL06_OVERSAMP_8                (0x03)
#define SPL06_OVERSAMP_16               (0x04)
#define SPL06_OVERSAMP_32               (0x05)
#define SPL06_OVERSAMP_64               (0x06)
#define SPL06_OVERSAMP_128              (0x07)

#define BARO_I2C_BUS 1
#define BARO_I2C_ADDR SPL06_I2C_ADDR//I2C扫描获取
#define BARO_PIN_SDA 15 // I2C1_SDA 3
#define BARO_PIN_SCL 16 // I2C1_SCL 3

typedef enum 
{
    PRESSURE_SENSOR, 
    TEMPERATURE_SENSOR
}baro_sensor_e;

typedef struct 
{
    int32_t praw;   /* 气压原始值 */
    int32_t traw;   /* 温度原始值 */
    float   pcomp;  /* 计算后的气压值 hpa */
    float   tcomp;  /* 计算后的温度值 ℃ */
    float   asl;    /* 通过气压值转换后的海拔高度 m */
} baro_data_t;

/* 用于存储校准数据 */
typedef struct 
{
    int16_t c0;
    int16_t c1;
    int32_t c00;
    int32_t c10;
    int16_t c01;
    int16_t c11;
    int16_t c20;
    int16_t c21;
    int16_t c30;
} baro_calibcoeff_t;

errcode_t baro_init(void);
void baro_get_data(baro_data_t *data);
uint8_t baro_write_byte(uint8_t reg, uint8_t data);
uint8_t baro_read_byte(uint8_t reg);
uint8_t baro_write_nbytes(uint8_t reg, uint8_t* data, uint8_t len);
int baro_read_nbytes(uint8_t reg, uint8_t *data, uint8_t len);
void baro_rateset(baro_sensor_e sensor, uint8_t measureRate, uint8_t oversampleRate);
void baro_get_calib_param(void);
float baro_get_pressure(int32_t pressure, int32_t temperature);
float baro_get_temperature(int32_t temperature);
float baro_pressure_to_asl(float pressure);
// float baro_get_pressure();
// float baro_get_temperature();
// float baro_get_altitude();

#endif