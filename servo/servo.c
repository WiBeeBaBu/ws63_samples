#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdint.h>

#include "app_init.h"
#include "cmsis_os2.h"
#include "osal_debug.h"
#include "soc_osal.h"
#include "errno.h"
#include "errcode.h"
#include "pinctrl.h"

#include "systick.h"
#include "adc.h"
#include "adc_porting.h"

#include "gpio.h"
#include "time.h"
#include "watchdog.h"
#include "watchdog_porting.h"
#include "security_unified_porting.h"

#include "wiper.h"

void set_angle(unsigned int duty)
{
    unsigned int time = FREQ_TIME;

    uapi_gpio_set_val(WIPER_GPIO6_PIN,GPIO_LEVEL_HIGH);
    uapi_systick_delay_us(duty);
    uapi_gpio_set_val(WIPER_GPIO6_PIN,GPIO_LEVEL_LOW);
    uapi_systick_delay_us(time - duty);
}

void set_angle_seat(unsigned int duty)
{
    unsigned int time = FREQ_TIME;

    uapi_gpio_set_val(SEAT_GPIO7_PIN,GPIO_LEVEL_HIGH);
    uapi_systick_delay_us(duty);
    uapi_gpio_set_val(SEAT_GPIO7_PIN,GPIO_LEVEL_LOW);
    uapi_systick_delay_us(time - duty);
}

void engine_turn_left(void)
{
    for (int i = 0; i < 20; i++) {
        set_angle(2500);
    }
}

void engine_turn_right(void)
{
    for (int i = 0; i <20; i++) {
        set_angle(1350);
    }
}

void regress_middle(void)
{
    for (int i = 0; i <20; i++) {
        set_angle(1575);
    }
}

void SweepCycle(void)
{
    engine_turn_right();
    uapi_watchdog_kick();
    engine_turn_left();
    uapi_watchdog_kick();
}

void WiperReset(void)
{
    engine_turn_left();
    uapi_watchdog_kick();
}


void WiperInit(void)
{

    errcode_t ret_dog = uapi_watchdog_init(5000);
    if(ret_dog!=IOT_SUCCESS){
        printf("watchdog init failed");
    }
    ret_dog = uapi_watchdog_enable(WDT_MODE_RESET);
    if(ret_dog!=IOT_SUCCESS){
        printf("watchdog enable failed");
    }
    /* 雨量传感器配置 */
    uapi_adc_init(ADC_CLOCK_NONE);
    /* 舵机配置 */
    if(uapi_pin_set_mode(WIPER_GPIO6_PIN,WIPER_GPIO6_GPIO_MODE)!=IOT_SUCCESS){
        printf("GPIO7 set mode failed!\n");
        return;
    }
    if(uapi_gpio_set_dir(WIPER_GPIO6_PIN, GPIO_DIRECTION_OUTPUT)!=IOT_SUCCESS){
        printf("GPIO7 set dir failed!\n");
        return;
    }
}

void AutoWiperStart(void)
{
    uint32_t ret;
    uint16_t data;
    static char rain_level[32] = {0};
    float voltage;

    float vlt_min = VLT_MIN;

    WiperReset();
    printf("AutoWiperStart\n");
    while (1) {
        // data=2048;
        ret = adc_port_read(RAIN_GPIO8_ADC_CHANNEL_2, &data);
        if (ret != IOT_SUCCESS) {
            printf("ADC Read Fail\n");
            continue;
        }else printf("ADC Read Succ\n");
        printf("adc original data %d mv\r\n", data);
        voltage = (float)data * 1.8 * 4 / 4096.0;  /* vlt * 1.8 * 4 / 4096.0 is to convert codeword to voltage */      
        voltage = voltage>vlt_min?voltage:vlt_min;
        int retval = sprintf(rain_level, ": %.2f", voltage);
        if (retval < 0) {
            printf("rain failed\r\n");
        }
        printf("rain_level = %s \r\n", rain_level);
        if (voltage > 1) {
            SweepCycle();
            osDelay(50);
            SweepCycle();
        } else if (voltage < 1) {
            WiperReset();
            lying();
            osDelay(50);
            sleeping();
            osDelay(50);
            sitting();
            osDelay(50);
        }
        osDelay(50);
        uapi_watchdog_kick();
    }
}

void sitting(void) {
    for (int i=0; i < 20; i++) {
        set_angle_seat(1575);
    } 
}

void lying(void) {
    for (int i=0; i < 20; i++) {
        set_angle_seat(2500);
    } 
}

void sleeping(void) {
    for (int i=0; i < 20; i++) {
        set_angle_seat(1875);
    } 
}

void SeatReset(void) {
    for (int i=0; i < 20; i++) {
        set_angle_seat(1575);
    } 
}

void SeatCtrlInit(void) {
    if(uapi_pin_set_mode(SEAT_GPIO7_PIN, SEAT_GPIO7_GPIO_MODE)!=IOT_SUCCESS){
        printf("GPIO7 set mode failed!\n");
        return;
    };
    if(uapi_gpio_set_dir(SEAT_GPIO7_PIN, GPIO_DIRECTION_OUTPUT)!=IOT_SUCCESS){
        printf("GPIO7 set dir failed!\n");
        return;
    }
}