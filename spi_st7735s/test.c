#include <stdio.h>      
#include <unistd.h>    
#include <stdint.h>
#include "app_init.h" 
#include "cmsis_os2.h" 
#include "gpio.h"    
#include "errno.h"  
#include "osal_debug.h"
#include "soc_osal.h"    
#include "pinctrl.h"
#include "st7735s_ws63.h"

#include "time.h"

unsigned short color1[128*160];
unsigned short color2[128*160];
static void Smart_TimeKeeper_Task(void *arg)
{
    // (void)arg;

    // printf("start init LCD...\n");
    // lcdInit(); // 调用LCD初始化函数
    // printf("LCD init succeed\n");
    
    // // 调用 lcdClear 函数将屏幕清为黑色
    // printf("lcd clear partial black...\n");
    // lcdClearPartial(BLACK); // 确保 BLACK 宏在 st7735s_ws63.h 或相关头文件中被定义
    // printf("clear succeed.\n");
    
    // // 如果您想验证是否能画点，可以在清屏后添加以下代码（可选）：
    // // usleep(1000000); // 延时1秒，观察黑屏效果
    // // LCD_DrawPoint(64, 80, RED); // 在屏幕中心画一个红点 (需要 RED 宏定义)
    // // printf("绘制红点完成！\n");

    // // (void)arg;

    // // printf("开始初始化LCD...\n");
    // // lcdInit();
    // // printf("LCD初始化完成\n");
    
    // // // 只执行一次清屏操作
    // // printf("执行清屏...\n");
    // // lcdClear(BLACK);
    // // printf("清屏完成\n");
    
    // // 简单的测试图案
    // // printf("绘制测试图案...\n");
    // // LCD_DrawLine(0, 0, 128, 160, RED);
    // // LCD_DrawLine(0, 160, 128, 0, BLUE);
    // // printf("测试完成!\n");
    // while (1)
    // {
    //     // 保持任务运行，您可以根据需要在此处添加其他显示逻辑
    //     for(int i = 0 ;i < 128*160 ; i++)
    //     {
    //         color1[i] = BRED;
    //         color2[i] = GREEN;
    //     }
    //     time_t new;
    //     time_t old;
    //     while (1)
    //     {
    //         // time(&old);
    //         LCD_Fill(0,0,128,160,color1);
            
    //         lcdClear(YELLOW);
    //         usleep(50000);
    //         lcdClear(BRRED);
    //         usleep(50000);
    //         lcdClear(DARKBLUE);
    //         usleep(50000);
    //         lcdClear(YELLOW);
    //         usleep(50000);
    //         lcdClear(BRRED);
    //         usleep(50000);
    //         lcdClear(DARKBLUE);
    //         usleep(50000);
    //         lcdClear(YELLOW);
    //         usleep(50000);
    //         lcdClear(BRRED);
    //         usleep(50000);
    //         lcdClear(DARKBLUE);
    //         usleep(50000);
    //         lcdClear(BRRED);
    //         usleep(50000);
    //         time(&new);
    //         //printf("old : %lu ms , new : % lu ms\n",old,new);
    //         uint32_t duration = (new - old);
    //         float fps = 1000.0f / duration;
    //         printf("Full screen refresh time: %lu ms, FPS: %.2f\n", duration, fps);
    //         usleep(150000);
    //         LCD_Fill(0,0,128,160,color2);
    //         usleep(100000);
    //     }
    //     usleep(5000000); // 延时5秒，防止任务退出
    // }
    (void)arg;

    printf("开始初始化LCD...\n");
    lcdInit(); // 调用LCD初始化函数
    printf("LCD初始化完成\n");
    
    // 初始化 color1 和 color2 数组（如果需要，请取消注释）
    for(int i = 0 ;i < 128*160 ; i++)
    {
        color1[i] = BRRED; // 例如，用亮红色填充
        color2[i] = YELLOW; // 例如，用黄色填充
    }

    printf("开始3次屏幕刷新循环...\n");
    // 循环刷新屏幕3次
    int refresh_count;
    for (refresh_count = 0; refresh_count < 3; refresh_count++)
    {
        // 第一次填充
        // 注意：这里将填充范围修正为 0 到 127 (X轴) 和 0 到 159 (Y轴)
        // 确保与屏幕的 128x160 实际分辨率匹配
        LCD_Fill(0, 0, 127, 159, color1);
        usleep(150000); // 延时150毫秒

        // 多次清屏以演示效果
        lcdClear(YELLOW);
        usleep(50000);
        lcdClear(BRRED);
        usleep(50000);
        lcdClear(DARKBLUE);
        usleep(50000);
        
        // 第二次填充
        LCD_Fill(0, 0, 127, 159, color2);
        usleep(100000); // 延时100毫秒
    }

    printf("3次刷新循环完成，清屏为黑并显示文本！\n");
    
    // 清空屏幕为黑屏
    lcdClear(BLACK);
    usleep(500000); // 延时一段时间让用户看到黑屏

    // 显示“yb nb !!!”居中
    // 文本内容: "yb nb !!!" (9个字符)
    // 假设使用默认的16号字体（LCD_ShowString中的size参数为16），
    // 16号字体通常每个字符宽度为8像素，高度为16像素。
    // 文本总宽度 = 9字符 * 8像素/字符 = 72像素
    // 屏幕宽度 = 128像素
    // 居中X坐标 = (屏幕宽度 - 文本总宽度) / 2 = (128 - 72) / 2 = 56 / 2 = 28

    // 屏幕高度 = 160像素
    // 居中Y坐标 = (屏幕高度 - 字体高度) / 2 = (160 - 16) / 2 = 144 / 2 = 72

    // 注意：LCD_ShowString内部也需要应用X偏移2，Y偏移3。
    // 这里的(x,y)是逻辑坐标，LCD_ShowString函数内部应将其转换为物理GRAM坐标。
    LCD_ShowString(32, 72, 64, 16, 16, (unsigned char*)"Badou♥", WHITE, BLACK);
    
    usleep(5000000); // 文本显示5秒钟

    // 任务结束后进入无限循环，防止任务退出
    while(1)
    {
        osDelay(1000); // 延时1秒
    }

}

static void Smart_TimeKeeper_Entry(void)
{
    osal_task *task_handle = NULL;
    osal_kthread_lock();
    task_handle = osal_kthread_create((osal_kthread_handler)Smart_TimeKeeper_Task, 0, "Smart_TimeKeeper_Task", 0x1000);
    if (task_handle != NULL) {
        osal_kthread_set_priority(task_handle, 17);
    }
    osal_kthread_unlock();
}

app_run(Smart_TimeKeeper_Entry);
