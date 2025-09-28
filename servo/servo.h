#include <stdio.h>
#include <unistd.h>
#include <stdint.h>

#define WIPER_GPIO6_PIN 6
#define SEAT_GPIO7_PIN 7
#define RAIN_GPIO8_PIN 8
#define WIPER_GPIO6_GPIO_MODE 0
#define SEAT_GPIO7_GPIO_MODE 0
#define RAIN_GPIO8_GPIO_MODE 0
#define RAIN_GPIO8_ADC_CHANNEL_2 1
#define FREQ_TIME 20000
#define DELAY_US     (100000)
#define ADC_LENGTH   (20)
#define VLT_MIN      (0.218)
#define IOT_SUCCESS    0

//旋转特定角度
void set_angle(unsigned int duty);

void set_angle_seat(unsigned int duty);
//左转
void engine_turn_left(void);
//右转
void engine_turn_right(void);
//居中
void regress_middle(void);

void SweepCycle(void);
//雨刮器复位
void WiperReset(void);
//硬件初始化
void WiperInit(void);
//自动雨刮器开始
void AutoWiperStart(void);

void sitting(void);
void lying(void);
void sleeping(void);
void SeatReset(void);
void SeatCtrlInit(void);
