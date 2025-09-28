#include "barometer.h"
#include "i2c.h"
#include "pinctrl.h"
#include "soc_osal.h"
#include "cmsis_os2.h"
#include <stdio.h>
#include <time.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>

/* Kp和Kt的尺度因数 */
const uint32_t scale_factor[8] = {524288, 1572864, 3670016, 7864320, 253952, 516096, 1040384, 2088960};

baro_calibcoeff_t baro_calib;
static int32_t kp = 0;
static int32_t kt = 0;

uint8_t baro_write_byte(uint8_t reg, uint8_t data)
{
    errcode_t ret;
    uint8_t write_buffer[2] = {reg, data};
    i2c_data_t i2c_data = {0};

    i2c_data.send_buf = write_buffer;
    i2c_data.send_len = 2;
    i2c_data.receive_buf = NULL;
    i2c_data.receive_len = 0;

    ret = uapi_i2c_master_write(BARO_I2C_BUS, BARO_I2C_ADDR, &i2c_data);
    return (ret == ERRCODE_SUCC) ? 0 : 1;  // 成功返回0，失败返回1
}

uint8_t baro_read_byte(uint8_t reg)
{
    errcode_t ret;
    uint8_t read_data;
    i2c_data_t i2c_data = {0};

    // 先写寄存器地址
    i2c_data.send_buf = &reg;
    i2c_data.send_len = 1;
    i2c_data.receive_buf = NULL;
    i2c_data.receive_len = 0;
    ret = uapi_i2c_master_write(BARO_I2C_BUS, BARO_I2C_ADDR, &i2c_data);
    if (ret != ERRCODE_SUCC) {
        return 0xFF;
    }

    // 再读数据
    i2c_data.send_buf = NULL;
    i2c_data.send_len = 0;
    i2c_data.receive_buf = &read_data;
    i2c_data.receive_len = 1;
    ret = uapi_i2c_master_read(BARO_I2C_BUS, BARO_I2C_ADDR, &i2c_data);
    if (ret != ERRCODE_SUCC) {
        return 0xFF;
    }

    return read_data;
}

uint8_t baro_write_nbytes(uint8_t reg, uint8_t* data, uint8_t len)
{
    errcode_t ret;
    uint8_t *write_buffer = (uint8_t*)malloc(len + 1);  // 使用标准malloc
    i2c_data_t i2c_data = {0};

    if (write_buffer == NULL) {
        return 1;
    }

    write_buffer[0] = reg; 
    memcpy(&write_buffer[1], data, len);  // 使用标准memcpy

    i2c_data.send_buf = write_buffer;
    i2c_data.send_len = len + 1;
    i2c_data.receive_buf = NULL;
    i2c_data.receive_len = 0;

    ret = uapi_i2c_master_write(BARO_I2C_BUS, BARO_I2C_ADDR, &i2c_data);

    free(write_buffer);  // 使用标准free
    return (ret == ERRCODE_SUCC) ? 0 : 2;
}

int baro_read_nbytes(uint8_t reg, uint8_t *data, uint8_t len)
{
    errcode_t ret;
    i2c_data_t i2c_data = {0};

    // 先写寄存器地址
    i2c_data.send_buf = &reg;
    i2c_data.send_len = 1;
    i2c_data.receive_buf = NULL;
    i2c_data.receive_len = 0;
    ret = uapi_i2c_master_write(BARO_I2C_BUS, BARO_I2C_ADDR, &i2c_data);
    if (ret != ERRCODE_SUCC) {
        return 1;
    }

    // 再读数据
    i2c_data.send_buf = NULL;
    i2c_data.send_len = 0;
    i2c_data.receive_buf = data;
    i2c_data.receive_len = len;
    ret = uapi_i2c_master_read(BARO_I2C_BUS, BARO_I2C_ADDR, &i2c_data);
    return (ret == ERRCODE_SUCC) ? 0 : 2;
}

void baro_rateset(baro_sensor_e sensor, uint8_t measureRate, uint8_t oversampleRate)
{
    uint8_t reg;

    if(sensor == PRESSURE_SENSOR){
        kp = scale_factor[oversampleRate];
        baro_write_byte(SPL06_PRESSURE_CFG_REG, measureRate<<4 | oversampleRate);
        if (oversampleRate > SPL06_OVERSAMP_8)
        {
            baro_read_nbytes(SPL06_INT_FIFO_CFG_REG, &reg, 1);
            baro_write_byte(SPL06_INT_FIFO_CFG_REG, reg | 0x04);
        }
    }
    else if(sensor == TEMPERATURE_SENSOR){
        kt = scale_factor[oversampleRate];
        baro_write_byte(SPL06_TEMPERATURE_CFG_REG, measureRate<<4 | oversampleRate | 0x80); /* Using mems temperature */
        if (oversampleRate > SPL06_OVERSAMP_8)
        {
            baro_read_nbytes(SPL06_INT_FIFO_CFG_REG, &reg, 1);
            baro_write_byte(SPL06_INT_FIFO_CFG_REG, reg | 0x08);
        }
    }
}


/**
 * @brief   读取校准数据（用于计算气压值和温度值）
 * @param   无
 * @retval  无
 */
 void baro_get_calib_param(void)
 {
    uint8_t buffer[SPL06_CALIB_COEFFICIENT_LENGTH] = {0};

    baro_read_nbytes(SPL06_COEFFICIENT_CALIB_REG, buffer, SPL06_CALIB_COEFFICIENT_LENGTH);
    
    baro_calib.c0 = (int16_t)buffer[0]<<4 | buffer[1]>>4;
    baro_calib.c0 = (baro_calib.c0 & 0x0800) ? (baro_calib.c0 | 0xF000) : baro_calib.c0;

    baro_calib.c1 = (int16_t)(buffer[1] & 0x0F)<<8 | buffer[2];
    baro_calib.c1 = (baro_calib.c1 & 0x0800) ? (baro_calib.c1 | 0xF000) : baro_calib.c1;

    baro_calib.c00 = (int32_t)buffer[3]<<12 | (int32_t)buffer[4]<<4 | (int32_t)buffer[5]>>4;
    baro_calib.c00 = (int32_t)(((uint32_t)baro_calib.c00 << 8) >> 8); // 24-bit sign-extend

    baro_calib.c10 = (int32_t)(buffer[5] & 0x0F)<<16 | (int32_t)buffer[6]<<8 | (int32_t)buffer[7];
    baro_calib.c10 = (int32_t)(((uint32_t)baro_calib.c10 << 8) >> 8); // 24-bit sign-extend

    baro_calib.c01 = (int16_t)buffer[8]<<8 | buffer[9];
    baro_calib.c11 = (int16_t)buffer[10]<<8 | buffer[11];
    baro_calib.c20 = (int16_t)buffer[12]<<8 | buffer[13];
    baro_calib.c21 = (int16_t)buffer[14]<<8 | buffer[15];
    baro_calib.c30 = (int16_t)buffer[16]<<8 | buffer[17];
    
    printf("SPL06 calib:\n");
    printf("  c0  = %d\n",  (int)baro_calib.c0);
    printf("  c1  = %d\n",  (int)baro_calib.c1);
    printf("  c00 = %ld\n", (long)baro_calib.c00);
    printf("  c10 = %ld\n", (long)baro_calib.c10);
    printf("  c01 = %d\n",  (int)baro_calib.c01);
    printf("  c11 = %d\n",  (int)baro_calib.c11);
    printf("  c20 = %d\n",  (int)baro_calib.c20);
    printf("  c21 = %d\n",  (int)baro_calib.c21);
    printf("  c30 = %d\n",  (int)baro_calib.c30);
 }

 /**
 * @brief   获取温度值（℃）
 * @param   temperature:    原始的温度数据        
 * @retval  计算后 最终的补偿温度值
 */
float baro_get_temperature(int32_t temperature)
{
    float fTCompensate;
    float fTsc;

    fTsc = temperature / (float)kt;
    fTCompensate =  baro_calib.c0 * 0.5 + baro_calib.c1 * fTsc;
    return fTCompensate;
}

/**
 * @brief   获取气压值（Pa）
 * @param   pressure:       原始压力数据 
 * @param   temperature:    原始的温度数据        
 * @retval  计算后 最终的补偿气压值
 */
float baro_get_pressure(int32_t pressure, int32_t temperature)
{
    float fTsc, fPsc;
    float qua2, qua3;
    float fPCompensate;

    fTsc = temperature / (float)kt;
    fPsc = pressure / (float)kp;
    qua2 = baro_calib.c10 + fPsc * (baro_calib.c20 + fPsc* baro_calib.c30);
    qua3 = fTsc * fPsc * (baro_calib.c11 + fPsc * baro_calib.c21);
    /* qua3 = 0.9f *fTsc * fPsc * (spl06Calib.c11 + fPsc * spl06Calib.c21); */

    fPCompensate = baro_calib.c00 + fPsc * qua2 + fTsc * baro_calib.c01 + qua3;
    /* fPCompensate = spl06Calib.c00 + fPsc * qua2 + 0.9f *fTsc  * spl06Calib.c01 + qua3; */
    return fPCompensate;
}

/**
 * @brief   以m为单位,将压力转换为海平面以上高度(ASL)
 * @param   pressure:   经过计算后的压力数据,注意气压值的单位需为hpa 
 * @retval  海拔高度（m）
 */
float baro_pressure_to_asl(float pressure)
{   
    if(pressure)
    {
        return 44330.f * (powf((1015.7f / pressure), 0.190295f) - 1.0f);
    }
    else
    {
        return 0;
    }
}


void baro_get_data(baro_data_t *data){
    uint8_t raw_data[6];
    
    printf("get_data_start!!!\n");
    // 读取6字节原始数据
    if (baro_read_nbytes(SPL06_PRESSURE_MSB_REG, raw_data, 6) == 0) {
        char tempData[10];
        char pressData[10];  
        char aslData[10];
        // 组合24位原始数据
        data->praw = ((int32_t)raw_data[0] << 16) | 
                     ((int32_t)raw_data[1] << 8) | 
                     (int32_t)raw_data[2];
        data->traw = ((int32_t)raw_data[3] << 16) | 
                     ((int32_t)raw_data[4] << 8) | 
                     (int32_t)raw_data[5];
        
        printf("Raw pressure: 0x%06X, Raw temperature: 0x%06X\n", 
               (unsigned int)data->praw, (unsigned int)data->traw);
        data->pcomp = baro_get_pressure(data->praw, data->traw) / 100;
        data->tcomp = baro_get_temperature(data->traw);
        data->asl = baro_pressure_to_asl(data->pcomp);
        sprintf(tempData, "%.2f", data->tcomp);
        sprintf(pressData, "%.2f", data->pcomp);
        sprintf(aslData, "%.2f", data->asl);
        printf("Compensated pressure: %s hPa, Compensated temperature: %s C, Altitude: %s m\n", 
               pressData,tempData, aslData);
    } else {
        printf("Failed to read data from SPL06\n");
    }
    printf("get_data_end!!!\n");
}

errcode_t baro_init(){
    errcode_t ret;
    uint8_t chip_id;

    uapi_pin_set_mode(BARO_PIN_SCL,2);
    uapi_pin_set_mode(BARO_PIN_SDA,2);

    ret = uapi_i2c_master_init(BARO_I2C_BUS,100000,0);
    if (ret != ERRCODE_SUCC) {
    printf("I2C Master Init Failed!\r\n");
    }

    for (uint16_t addr = 0x03; addr <= 0x77; addr++) {
        uint8_t reg = 0x0D;
        i2c_data_t io = { .send_buf = &reg, .send_len = 1, .receive_buf = NULL, .receive_len = 0 };
        errcode_t r = uapi_i2c_master_write(BARO_I2C_BUS, addr, &io);
        if (r == ERRCODE_SUCC) {
            printf("I2C device found at 0x%02X\n", (unsigned)addr);
        }
        osal_msleep(5);
    }

    chip_id = baro_read_byte(SPL06_CHIP_ID);
    if (chip_id != SPL06_DEFAULT_CHIP_ID) {
        printf("SPL06 Chip ID Error: 0x%02X\r\n", chip_id);
        return ERRCODE_FAIL;
    }

    baro_get_calib_param();

    baro_rateset(PRESSURE_SENSOR, SPL06_MWASURE_16, SPL06_OVERSAMP_64);
    baro_rateset(TEMPERATURE_SENSOR, SPL06_MWASURE_16, SPL06_OVERSAMP_64);
    baro_write_byte(SPL06_MODE_CFG_REG, SPL06_CONTINUOUS_MODE);
    
    return ret;
}

