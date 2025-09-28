#include <stdio.h>
#include <unistd.h>
#include "cmsis_os2.h"

#define DELAY_500MS    500000
#define MS_PER_S 1000
#define AHT20_BAUDRATE (400 * 1000)
#define AHT20_I2C_ADDR 0x0

#define DEAD_TIME_US       2       // 软件死区时间（微秒）
#define TH_I2C_SCL_PIN 15
#define TH_I2C_SDA_PIN 16
#define FAN_GPIO4_PIN 4 //FI
#define FAN_GPIO5_PIN 5 //BI
#define FAN_GPIO4_GPIO_MODE 2
#define FAN_GPIO5_GPIO_MODE 4
#define TH_I2C_PIN_MODE 2
#define IOT_SUCCESS    0

/* 风扇运行状态 */
typedef enum {
    FAN_OFF,
    FAN_RUNNING,
    FAN_COASTING,
    FAN_ACCELERATING
} FanState;

/* PWM通道电平定义 */
typedef enum {
    PWM_ACTIVE_HIGH,
    PWM_ACTIVE_LOW
} PwmPolarity;

extern FanState g_fanState;

//硬件初始化
void TemperFanInit(void);

void AutoFreshFan(void);

void FreshFanOn(void);

void FreshFanOff(void);

void FreshFanStop(void);

FanState GetFanState(void);