#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>

#include "app_init.h"
#include "cmsis_os2.h"
#include "osal_debug.h"
#include "soc_osal.h"
#include "errno.h"
#include "errcode.h"
#include "pinctrl.h"
#include "common_def.h"


#include "i2c.h"
#include "gpio.h"

#include "aht20.h"
#include "fan.h"

FanState g_fanState = FAN_OFF;

FanState GetFanState(void) {
    return g_fanState;
}

void TemperFanInit(void) {
    uapi_pin_set_mode(TH_I2C_SCL_PIN, TH_I2C_PIN_MODE);
    uapi_pin_set_mode(TH_I2C_SDA_PIN, TH_I2C_PIN_MODE);
    uapi_pin_set_mode(FAN_GPIO4_PIN, FAN_GPIO4_GPIO_MODE); //FI
    uapi_pin_set_mode(FAN_GPIO5_PIN, FAN_GPIO5_GPIO_MODE); //BI

    uapi_gpio_set_dir(FAN_GPIO4_PIN, GPIO_DIRECTION_OUTPUT);
    uapi_gpio_set_val(FAN_GPIO4_PIN,GPIO_LEVEL_LOW);

    uapi_gpio_set_dir(FAN_GPIO5_PIN, GPIO_DIRECTION_OUTPUT);
    uapi_gpio_set_val(FAN_GPIO5_PIN,GPIO_LEVEL_LOW);

}

void AutoFreshFan(void) {
    uint32_t retval = 0;
    float humidity = 0.0f;
    float temperature = 0.0f;
    static char templine[32] = {0};
    static char humiline[32] = {0};

    while (IOT_SUCCESS != AHT20_Calibrate()) {
        printf("AHT20 sensor init failed!\r\n");
        usleep(MS_PER_S);
    }

    retval = AHT20_StartMeasure();
    if (retval != IOT_SUCCESS) {
        printf("trigger measure failed!\r\n");
    }

    retval = AHT20_GetMeasureResult(&temperature, &humidity);
    if (retval != IOT_SUCCESS) {
        printf("get data failed!\r\n");
    }

    printf("print temp and humi\n");
    int ret = sprintf(templine, ": %.2f", temperature);
    if (ret < 0) {
        printf("temp failed\r\n");
    }
    ret = sprintf(humiline, ": %.2f", humidity);
    if (ret < 0) {
        printf("humi failed\r\n");
    }
    printf("temp = %s, humi = %s\r\n", templine, humiline);
    // printf("temp:%.2f , humi:%.2f\n", temperature, humidity);
    
    if (temperature > 28.00f) {
        FreshFanOn();
    } else {
        FreshFanOff();
    }
    osDelay(25);
}

void FreshFanOn(void) {
    printf("fan running!\n");
    uapi_gpio_set_val(FAN_GPIO4_PIN,GPIO_LEVEL_HIGH);
    uapi_gpio_set_val(FAN_GPIO5_PIN,GPIO_LEVEL_LOW);
    usleep(100);
    g_fanState = FAN_RUNNING;
}

void FreshFanOff(void) {
    uapi_gpio_set_val(FAN_GPIO4_PIN,GPIO_LEVEL_HIGH);
    uapi_gpio_set_val(FAN_GPIO5_PIN,GPIO_LEVEL_HIGH);
    // uapi_pwm_close(4);
    usleep(100);
    g_fanState = FAN_OFF;
}

void FreshFanStop(void) {
    uapi_gpio_set_val(FAN_GPIO4_PIN,GPIO_LEVEL_HIGH);
    uapi_gpio_set_val(FAN_GPIO5_PIN,GPIO_LEVEL_HIGH);
    usleep(100);
    g_fanState = FAN_OFF;
}