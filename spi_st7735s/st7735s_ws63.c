#include <stdio.h>
#include <unistd.h>

#include "app_init.h"
#include "cmsis_os2.h"
#include "pinctrl.h"
#include "gpio.h"
#include "spi.h"

#include "st7735s_ws63.h"
#include "font.h"
#include "hz_16x16.h"
#include "hz_32x32.h"

// SPI初始化
void spiInit(void){
    uint32_t ret;
    uapi_spi_deinit(LCD_SPI_BUS_ID);

    // SPI引脚复用关系配置
    uapi_pin_set_mode(SPI0_PIN_CSN,3);
    uapi_pin_set_mode(SPI0_PIN_CLK,3);
    uapi_pin_set_mode(SPI0_PIN_RXD,3);
    uapi_pin_set_mode(SPI0_PIN_TXD,3);

    // 设置SCK引脚驱动能力
    uapi_pin_set_ds(SPI0_PIN_CLK,PIN_DS_2);
    
    // 配置SPI
    spi_attr_t spi_cfg_basic_info = {
        .is_slave = false, //主机模式 
		.slave_num = 1,
		.bus_clk = 8000000,
		.freq_mhz = 8,
        .clk_polarity  = 0, // 极性0
        .clk_phase     = 0,  // 相位0
		.spi_frame_format = HAL_SPI_FRAME_FORMAT_STANDARD,
        .frame_size = 0x7,   // 位宽为8bit
        .tmod = 0, //传输模式设置为收发模式
        .frame_format   = SPI_CFG_FRAME_FORMAT_MOTOROLA_SPI, // 摩托罗拉协议 (SPI标准模式)
		.sste = 0,
    };
    // SPI扩展属性 (如果不需要特殊配置，可设置为NULL)
     spi_extra_attr_t spi_extra_attr = { 0 }; // 默认值，不配置DMA/中断等

    // 初始化SPI主机
    ret = uapi_spi_init(LCD_SPI_BUS_ID, &spi_cfg_basic_info, &spi_extra_attr);
    if(ret != ERRCODE_SUCC) {
        printf("spi_init fail: %x\n", ret);
    }
    uapi_spi_set_loop_back_mode(LCD_SPI_BUS_ID,0);
    uapi_spi_set_irq_mode(LCD_SPI_BUS_ID,0,NULL,NULL); // 设置不使用中断模式
    printf("spi ret:%d\n!!!!!!!!!!!!!!!!!!\n", ret);
}

void lcdGpioInit(void){
    uapi_gpio_init();
    // GPIO引脚复用关系配置
    uapi_pin_set_mode(LCD_PIN_RES,0);
    uapi_pin_set_mode(LCD_PIN_DC,0);
    uapi_pin_set_mode(LCD_PIN_BLK,2);

    uapi_gpio_set_dir(LCD_PIN_RES,GPIO_DIRECTION_OUTPUT);
    uapi_gpio_set_dir(LCD_PIN_DC,GPIO_DIRECTION_OUTPUT);
    uapi_gpio_set_dir(LCD_PIN_BLK,GPIO_DIRECTION_OUTPUT);

    SET_LCD_BLK; // 默认打开背光
}

void spiWrite(unsigned char data){
	//printf(">> Entering spiWrite function (Data to send: 0x%02X)\n", data);
	CLR_LCD_CSN;
	spi_xfer_data_t xfer = {
        .tx_buff = &data,
        .tx_bytes = 1,
        .rx_buff = NULL,
        .rx_bytes = 0,
        .cmd = 0,
        .addr = 0,
        .reserved = {0},
    };
    uint32_t ret;
    int retry_count = 0;
    const int max_retries = 3;
    
    do {
        // 第三个参数0表示超时时间，请根据您的uapi_spi_master_write API文档确认其含义。
        // 通常0可能表示非阻塞或无限等待。
        ret = uapi_spi_master_write(LCD_SPI_BUS_ID,&xfer,0); 
        if(ret != ERRCODE_SUCC) {
            printf("   spi write failed, error code: 0x%x, retry: %d\n", ret, retry_count);
            usleep(5000); // 延时5ms后重试
            retry_count++;
        }
    } while(ret != ERRCODE_SUCC && retry_count < max_retries);
    
    if(ret != ERRCODE_SUCC) {
        printf("!!! spi write single byte 0x%02X failed after %d retries!!!\n", data, max_retries);
    } else {
        //printf("   Successfully sent 0x%02X via SPI.\n", data);
    }

    // !!! 关键：拉高CSN，释放从机 !!!
    SET_LCD_CSN; 
    //printf("<< Exiting spiWrite function.\n");


    // spi_xfer_data_t xfer = {
	// 	.tx_buff = &data,
	// 	.tx_bytes = 1,
	// 	.rx_buff = NULL,
	// 	.rx_bytes = 0,
	// 	.cmd = 0,
	// 	.addr = 0,
	// 	.reserved = {0},
	// };
	// uint32_t ret;
    // int retry_count = 0;
    // const int max_retries = 3;
    
    // do {
    //     ret = uapi_spi_master_write(LCD_SPI_BUS_ID,&xfer,0);
    //     if(ret != ERRCODE_SUCC) {
    //         printf("spi write failed, error code: 0x%x, retry: %d\n", ret, retry_count);
    //         usleep(5000); // 延时5ms后重试
    //         retry_count++;
    //     }
    // } while(ret != ERRCODE_SUCC && retry_count < max_retries);
    
    // if(ret != ERRCODE_SUCC) {
    //     printf("spi write sigle failed after %d retries!!!\n", max_retries);
    // }
}

// void lcdInit(void)
// {
// 	lcdGpioInit();
// 	usleep(50000); // 增加延时
// 	spiInit();
// 	usleep(50000); // 增加延时

// 	SET_LCD_RES;
// 	usleep(10000);
// 	CLR_LCD_RES;
// 	usleep(10000);
// 	SET_LCD_RES;
// 	usleep(200000);

// 	lcdSelectRegister(0x11); // Sleep out
// 	usleep(120000);

// 	lcdSelectRegister(0xB1);
// 	// lcdWriteDataU8(0x05);
// 	// lcdWriteDataU8(0x3C);
// 	// lcdWriteDataU8(0x3C);

// 	// lcdSelectRegister(0xB2);
// 	// lcdWriteDataU8(0x05);
// 	// lcdWriteDataU8(0x3C);
// 	// lcdWriteDataU8(0x3C);

// 	// lcdSelectRegister(0xB3);
// 	// lcdWriteDataU8(0x05);
// 	// lcdWriteDataU8(0x3C);
// 	// lcdWriteDataU8(0x3C);
// 	// lcdWriteDataU8(0x05);
// 	// lcdWriteDataU8(0x3C);
// 	// lcdWriteDataU8(0x3C);

// 	// lcdSelectRegister(0xB4);
// 	// lcdWriteDataU8(0x03);

// 	// lcdSelectRegister(0xC0);
// 	// lcdWriteDataU8(0x28);
// 	// lcdWriteDataU8(0x08);
// 	// lcdWriteDataU8(0x04);

// 	// lcdSelectRegister(0xC1);
// 	// lcdWriteDataU8(0XC0);

// 	// lcdSelectRegister(0xC2);
// 	// lcdWriteDataU8(0x0D);
// 	// lcdWriteDataU8(0x00);

// 	// lcdSelectRegister(0xC3);
// 	// lcdWriteDataU8(0x8D);
// 	// lcdWriteDataU8(0x2A);

// 	// lcdSelectRegister(0xC4);
// 	// lcdWriteDataU8(0x8D);
// 	// lcdWriteDataU8(0xEE);

// 	// lcdSelectRegister(0xC5);
// 	// lcdWriteDataU8(0x1A);

// 	// lcdSelectRegister(0x36);
// 	// lcdWriteDataU8(0xC0);

// 	// lcdSelectRegister(0xE0);
// 	// lcdWriteDataU8(0x04);
// 	// lcdWriteDataU8(0x22);
// 	// lcdWriteDataU8(0x07);
// 	// lcdWriteDataU8(0x0A);
// 	// lcdWriteDataU8(0x2E);
// 	// lcdWriteDataU8(0x30);
// 	// lcdWriteDataU8(0x25);
// 	// lcdWriteDataU8(0x2A);
// 	// lcdWriteDataU8(0x28);
// 	// lcdWriteDataU8(0x26);
// 	// lcdWriteDataU8(0x2E);
// 	// lcdWriteDataU8(0x3A);
// 	// lcdWriteDataU8(0x00);
// 	// lcdWriteDataU8(0x01);
// 	// lcdWriteDataU8(0x03);
// 	// lcdWriteDataU8(0x13);

// 	// lcdSelectRegister(0xE1);
// 	// lcdWriteDataU8(0x04);
// 	// lcdWriteDataU8(0x16);
// 	// lcdWriteDataU8(0x06);
// 	// lcdWriteDataU8(0x0D);
// 	// lcdWriteDataU8(0x2D);
// 	// lcdWriteDataU8(0x26);
// 	// lcdWriteDataU8(0x23);
// 	// lcdWriteDataU8(0x27);
// 	// lcdWriteDataU8(0x27);
// 	// lcdWriteDataU8(0x25);
// 	// lcdWriteDataU8(0x2D);
// 	// lcdWriteDataU8(0x3B);
// 	// lcdWriteDataU8(0x00);
// 	// lcdWriteDataU8(0x01);
// 	// lcdWriteDataU8(0x04);
// 	// lcdWriteDataU8(0x13);

// 	// lcdSelectRegister(0x3A);
// 	// lcdWriteDataU8(0x05);

// 	// lcdSelectRegister(0x29);
// }

void lcdInit(void)
{
    lcdGpioInit(); // GPIO初始化
    usleep(50000); // 增加延时
    spiInit();     // SPI初始化
    usleep(50000); // 增加延时

    // 硬件复位序列
    SET_LCD_RES;
    usleep(10000);
    CLR_LCD_RES;
    usleep(10000);
    SET_LCD_RES;
    usleep(200000); // 复位后需要较长延时

    // 以下是根据 "2. 1.8寸TFT彩屏SPI接口规格书.pdf" 提供的初始化序列
    lcdSelectRegister(0x11); // Sleep out (解除睡眠模式)
    usleep(120000);          // 必须延时120ms

    lcdSelectRegister(0xB1); // Frame Rate Control 1 (正常模式下的帧率控制)
    lcdWriteDataU8(0x05);    // (0x05) 5分频
    lcdWriteDataU8(0x3C);    // (0x3C)
    lcdWriteDataU8(0x3C);    // (0x3C)

    lcdSelectRegister(0xB2); // Frame Rate Control 2 (空闲模式下的帧率控制)
    lcdWriteDataU8(0x05);    // (0x05)
    lcdWriteDataU8(0x3C);    // (0x3C)
    lcdWriteDataU8(0x3C);    // (0x3C)

    lcdSelectRegister(0xB3); // Frame Rate Control 3 (部分模式/全色模式下的帧率控制)
    lcdWriteDataU8(0x05);    // (0x05)
    lcdWriteDataU8(0x3C);    // (0x3C)
    lcdWriteDataU8(0x3C);    // (0x3C)
    lcdWriteDataU8(0x05);    // (0x05)
    lcdWriteDataU8(0x3C);    // (0x3C)
    lcdWriteDataU8(0x3C);    // (0x3C)

    lcdSelectRegister(0xB4); // Display Inversion Control (显示反转控制)
    lcdWriteDataU8(0x03);    // (0x03)

    lcdSelectRegister(0xC0); // Power Control 1 (电源控制1)
    lcdWriteDataU8(0x28);    // (0x28)
    lcdWriteDataU8(0x08);    // (0x08)
    lcdWriteDataU8(0x04);    // (0x04)

    lcdSelectRegister(0xC1); // Power Control 2 (电源控制2)
    lcdWriteDataU8(0XC0);    // (0xC0)

    lcdSelectRegister(0xC2); // Power Control 3 (Normal Mode)
    lcdWriteDataU8(0x0D);    // (0x0D)
    lcdWriteDataU8(0x00);    // (0x00)

    lcdSelectRegister(0xC3); // Power Control 4 (Idle Mode)
    lcdWriteDataU8(0x8D);    // (0x8D)
    lcdWriteDataU8(0x2A);    // (0x2A)

    lcdSelectRegister(0xC4); // Power Control 5 (Partial Mode)
    lcdWriteDataU8(0x8D);    // (0x8D)
    lcdWriteDataU8(0xEE);    // (0xEE)

    lcdSelectRegister(0xC5); // VCOM Control 1 (VCOM电压控制)
    lcdWriteDataU8(0x1A);    // (0x1A)

    // *** 关键修正 ***
    lcdSelectRegister(0x36); // Memory Access Control (内存访问控制)
    // 根据规格书，对于您的屏幕，正确的参数是 0x00
    // 之前使用 0xC0 可能是导致花屏的主要原因
    lcdWriteDataU8(0xC0);    // (0x00) - 禁用所有翻转，RGB颜色顺序

    lcdSelectRegister(0xE0); // Positive Gamma Correction (正伽马校正)
    lcdWriteDataU8(0x04);
    lcdWriteDataU8(0x22);
    lcdWriteDataU8(0x07);
    lcdWriteDataU8(0x0A);
    lcdWriteDataU8(0x2E);
    lcdWriteDataU8(0x30);
    lcdWriteDataU8(0x25);
    lcdWriteDataU8(0x2A);
    lcdWriteDataU8(0x28);
    lcdWriteDataU8(0x26);
    lcdWriteDataU8(0x2E);
    lcdWriteDataU8(0x3A);
    lcdWriteDataU8(0x00);
    lcdWriteDataU8(0x01);
    lcdWriteDataU8(0x03);
    lcdWriteDataU8(0x13);

    lcdSelectRegister(0xE1); // Negative Gamma Correction (负伽马校正)
    lcdWriteDataU8(0x04);
    lcdWriteDataU8(0x16);
    lcdWriteDataU8(0x06);
    lcdWriteDataU8(0x0D);
    lcdWriteDataU8(0x2D);
    lcdWriteDataU8(0x26);
    lcdWriteDataU8(0x23);
    lcdWriteDataU8(0x27);
    lcdWriteDataU8(0x27);
    lcdWriteDataU8(0x25);
    lcdWriteDataU8(0x2D);
    lcdWriteDataU8(0x3B);
    lcdWriteDataU8(0x00);
    lcdWriteDataU8(0x01);
    lcdWriteDataU8(0x04);
    lcdWriteDataU8(0x13);

    lcdSelectRegister(0x3A); // Color Mode (颜色模式设置)
    lcdWriteDataU8(0x05);    // (0x05) - 16-bit/pixel (65K色)

    // 注意：规格书在 0x3A 之后紧接着 0x29，没有 0x2A 和 0x2B
    // lcdSetAddress 内部会发送 0x2A 和 0x2B 命令，所以这里不需要额外发送
    // Column Address Set (设置列地址范围) - lcdSetAddress会处理
    // lcdSelectRegister(0x2A); 
    // lcdWriteDataU16(0x0000); // Start column = 0
    // lcdWriteDataU16(0x007F); // End column = 127 (for 128px width)

    // Row Address Set (设置行地址范围) - lcdSetAddress会处理
    // lcdSelectRegister(0x2B);
    // lcdWriteDataU16(0x0000); // Start row = 0
    // lcdWriteDataU16(0x009F); // End row = 159 (for 160px height)

    lcdSelectRegister(0x29); // Display On (打开显示)
    usleep(100000);          // 延时100ms
}

// 写入命令
void lcdSelectRegister(unsigned char data)
{
	//printf(">> Entering lcdSelectRegister (Command: 0x%02X)\n", data);
    CLR_LCD_DC; // DC拉低，表示写入命令
    spiWrite(data);
    // 注意：这里不需要 SET_LCD_DC;，因为spiWrite函数调用结束后会释放CSN，
    // 下一个操作（可能是数据写入）会重新设置DC和CSN。
    //printf("<< Exiting lcdSelectRegister.\n");

	// CLR_LCD_DC;
	// spiWrite(data);
	// SET_LCD_DC;
}

// 写入一字节数据
void lcdWriteDataU8(unsigned char data)
{
	//printf(">> Entering lcdWriteDataU8 (Data: 0x%02X)\n", data);
    SET_LCD_DC; // DC拉高，表示写入数据
    spiWrite(data);
    //printf("<< Exiting lcdWriteDataU8.\n");

	// spiWrite(data);
}

// 写入两字节数据
// void lcdWriteDataU16(unsigned short data)
// {
// 	// lcdWriteDataU8(data>>8);
// 	// lcdWriteDataU8(data);
// 	// unsigned char send_data[2];
// 	// send_data[0] = (data >> 8);
// 	// send_data[1] = data;
// 	uint8_t data_ =(uint8_t)data;
// 	spi_xfer_data_t xfer;
//     xfer.tx_buff = &data_;
//     xfer.rx_buff = NULL;
//     xfer.tx_bytes = 2;
// 	uint32_t ret = uapi_spi_master_write(LCD_SPI_BUS_ID, &xfer, 2);
// 	if (ret != ERRCODE_SUCC)
// 	{
// 		printf("spi_master_write fail: %x\n", ret);
// 	}
// }
void lcdWriteDataU16(unsigned short data)
{
	//printf(">> Entering lcdWriteDataU16 (Data: 0x%04X)\n", data);
    SET_LCD_DC; // DC拉高，表示写入数据
    spiWrite(data >> 8);  // 发送高字节
    spiWrite(data & 0xFF); // 发送低字节
    //printf("<< Exiting lcdWriteDataU16.\n");
	// spiWrite(data >> 8); // Send high byte
	// spiWrite(data & 0xFF); // Send low byte
}

// 设置光标位置
void lcdSetAddress(unsigned short x1, unsigned short y1, unsigned short x2, unsigned short y2)
{
	lcdSelectRegister(0x2A);
	lcdWriteDataU8(x1 >> 8);
	lcdWriteDataU8(x1 & 0xFF);
	lcdWriteDataU8(x2 >> 8);
	lcdWriteDataU8(x2 & 0xFF);

	lcdSelectRegister(0x2B);
	lcdWriteDataU8(y1 >> 8);
	lcdWriteDataU8(y1 & 0xFF);
	lcdWriteDataU8(y2 >> 8);
	lcdWriteDataU8(y2 & 0xFF);

	lcdSelectRegister(0x2C);
}

// 清屏
void lcdClear(unsigned short color)
{
	lcdSetAddress(2, 1, 129, 162);

	for (unsigned char i = 0; i < 128; i++)
	{
		for (unsigned char j = 0; j < 160; j++)
		{
			lcdWriteDataU16(color);
		}
	}
}

void lcdClearPartial(unsigned short color) {
    lcdSetAddress(0, 0, 129, 160); // 清空前10行 (0-9)
    for (unsigned short i = 0; i < 128; i++) {
        for (unsigned short j = 0; j < 160; j++) { // 只循环10次
            lcdWriteDataU16(color);
        }
    }
}

// LCD画点
void LCD_DrawPoint(unsigned short x, unsigned short y, unsigned short color)
{
	lcdSetAddress(x, y, x, y); // 设置光标位置
	lcdWriteDataU16(color);
}

// 快速画点
// x,y:坐标
// color:颜色
void LCD_Fast_DrawPoint(unsigned short x, unsigned short y, unsigned short color)
{
	(void)x;
	(void)y;
	lcdWriteDataU16(color);
}

// LCD开启显示
void LCD_DisplayOn(void)
{
	lcdSelectRegister(0x29); // 开启显示
}
// LCD关闭显示
void LCD_DisplayOff(void)
{
	lcdSelectRegister(0x28); // 关闭显示
}

// 在指定区域内填充单个颜色
//(sx,sy),(ex,ey):填充矩形对角坐标,区域大小为:(ex-sx+1)*(ey-sy+1)
// color:要填充的颜色
// void LCD_Fill(unsigned short sx, unsigned short sy, unsigned short ex, unsigned short ey, unsigned short *color)
// {
// 	uint32_t ret;
// 	unsigned short xlen = 0;
// 	unsigned short ylen = 0;

// 	xlen = ex - sx + 1;
// 	ylen = ey - sy + 1;
// 	int xylen = xlen * ylen;

// 	lcdSetAddress(sx, sy, ex, ey);
// 	// for (i = 0; i < xylen;i++)
// 	// {
// 	// 	unsigned short senddata = color[i];
// 	// 	color[i] = ((senddata & 0xff) << 8) | (senddata >> 8);
// 	// }
// 	uint8_t data_ = (uint8_t)color;
// 	spi_xfer_data_t xfer;
//     xfer.tx_buff = &data_;
//     xfer.rx_buff = NULL;
//     xfer.tx_bytes = xylen*2;
// 	ret = uapi_spi_master_write(LCD_SPI_BUS_ID, &xfer, xylen * 2);
// 	if(ret != ERRCODE_SUCC){
// 		printf("LCD fill failed!!!\n");
// 	}
// }

void LCD_Fill(unsigned short sx, unsigned short sy, unsigned short ex, unsigned short ey, unsigned short *color)
{
    unsigned short xlen = ex - sx + 1;
    unsigned short ylen = ey - sy + 1;
    unsigned int total_pixels = xlen * ylen;
    unsigned int i;

    lcdSetAddress(sx+2, sy+1, ex+2, ey+3);

    for (i = 0; i < total_pixels; i++)
    {
        lcdWriteDataU16(color[i]);
    }
}

// 画线
// x1,y1:起点坐标
// x2,y2:终点坐标
void LCD_DrawLine(unsigned short x1, unsigned short y1, unsigned short x2, unsigned short y2, unsigned short color)
{
	unsigned short t;
	int xerr = 0, yerr = 0, delta_x, delta_y, distance;
	int incx, incy, uRow, uCol;
	delta_x = x2 - x1; // 计算坐标增量
	delta_y = y2 - y1;
	uRow = x1;
	uCol = y1;

	// 设置单步方向
	if (delta_x > 0)
	{
		incx = 1;
	}
	else if (delta_x == 0) // 垂直线
	{
		incx = 0;
	}
	else
	{
		incx = -1;
		delta_x = -delta_x;
	}

	if (delta_y > 0)
	{
		incy = 1;
	}
	else if (delta_y == 0) // 水平线
	{
		incy = 0;
	}
	else
	{
		incy = -1;
		delta_y = -delta_y;
	}

	if (delta_x > delta_y) // 选取基本增量坐标轴
	{
		distance = delta_x;
	}
	else
	{
		distance = delta_y;
	}

	for (t = 0; t <= distance + 1; t++) // 画线输出
	{
		LCD_DrawPoint(uRow, uCol, color); // 画点
		xerr += delta_x;
		yerr += delta_y;
		if (xerr > distance)
		{
			xerr -= distance;
			uRow += incx;
		}

		if (yerr > distance)
		{
			yerr -= distance;
			uCol += incy;
		}
	}
}

void LCD_DrawRectangle(unsigned short x1, unsigned short y1, unsigned short x2, unsigned short y2, unsigned short color)
{
	LCD_DrawLine(x1, y1, x2, y1, color);
	LCD_DrawLine(x1, y1, x1, y2, color);
	LCD_DrawLine(x1, y2, x2, y2, color);
	LCD_DrawLine(x2, y1, x2, y2, color);
}

// 在指定位置画一个指定大小的圆
//(x,y):中心点
// r    :半径
void LCD_Draw_Circle(unsigned short x0, unsigned short y0, unsigned char r, unsigned short color)
{
	int a, b;
	int di;
	a = 0;
	b = r;
	di = 3 - (r << 1); // 判断下个点位置的标志
	while (a <= b)
	{
		LCD_DrawPoint(x0 + a, y0 - b, color);
		LCD_DrawPoint(x0 + b, y0 - a, color);
		LCD_DrawPoint(x0 + b, y0 + a, color);
		LCD_DrawPoint(x0 + a, y0 + b, color);
		LCD_DrawPoint(x0 - a, y0 + b, color);
		LCD_DrawPoint(x0 - b, y0 + a, color);
		LCD_DrawPoint(x0 - a, y0 - b, color);
		LCD_DrawPoint(x0 - b, y0 - a, color);
		a++;
		// 使用Bresenham算法画圆
		if (di < 0)
		{
			di += 4 * a + 6;
		}
		else
		{
			di += 10 + 4 * (a - b);
			b--;
		}
	}
}

// 在指定位置画一个指定大小的圆
//(x,y):中心点
// r    :半径
void LCD_DrawFullCircle(unsigned short Xpos, unsigned short Ypos, unsigned short Radius, unsigned short Color)
{
	uint16_t x, y, r = Radius;
	for (y = Ypos - r; y < Ypos + r; y++)
	{
		for (x = Xpos - r; x < Xpos + r; x++)
		{
			if (((x - Xpos) * (x - Xpos) + (y - Ypos) * (y - Ypos)) <= r * r)
			{
				LCD_DrawPoint(x, y, Color);
			}
		}
	}
}

// 在指定位置显示一个字符
// x,y:起始坐标
// num:要显示的字符:" "--->"~"
// size:字体大小 12/16/24
// mode:叠加方式(1)还是非叠加方式(0)
void LCD_ShowChar(unsigned short x, unsigned short y, unsigned char num, unsigned char size, unsigned char mode, unsigned short pen_color, unsigned short back_color)
{
	unsigned char temp, t1, t;
	unsigned short y0 = y;
	unsigned char csize = (size / 8 + ((size % 8) ? 1 : 0)) * (size / 2); // 得到字体一个字符对应点阵集所占的字节数
	num = num - ' ';													  // 得到偏移后的值（ASCII字库是从空格开始取模，所以-' '就是对应字符的字库）

	for (t = 0; t < csize; t++)
	{
		if (size == 12) // 调用1206字体
		{
			temp = asc2_1206[num][t];
		}
		else if (size == 16) // 调用1608字体
		{
			temp = asc2_1608[num][t];
		}
		else if (size == 24) // 调用2412字体
		{
			temp = asc2_2412[num][t];
		}
		else
			return; // 没有的字库

		for (t1 = 0; t1 < 8; t1++)
		{
			if (temp & 0x80)
			{
				LCD_DrawPoint(x, y, pen_color);
			}
			else if (mode == 0)
			{
				LCD_DrawPoint(x, y, back_color);
			}
			temp <<= 1;
			y++;

			if (y >= 240) // 超区域
			{
				return;
			}

			if ((y - y0) == size)
			{
				y = y0;
				x++;
				if (x >= 320) // 超区域
				{
					return;
				}
				break;
			}
		}
	}
}

// 显示字符串
// x,y:起点坐标
// width,height:区域大小
// size:字体大小
//*p:字符串起始地址
void LCD_ShowString(unsigned short x, unsigned short y, unsigned short width, unsigned short height, unsigned char size, unsigned char *p, unsigned short pen_color, unsigned short back_color)
{
	unsigned char x0 = x;
	width += x;
	height += y;
	while ((*p <= '~') && (*p >= ' ')) // 判断是不是非法字符!
	{
		if (x >= width)
		{
			x = x0;
			y += size;
		}

		if (y >= height) // 退出
		{
			break;
		}

		LCD_ShowChar(x, y, *p, size, 0, pen_color, back_color);
		x += size / 2;
		p++;
	}
}

// 程序汉字显示
// 说明：汉字是用取模软件制作好的 指定的汉字 汉字大小是16x16	 即32个字节存储一个汉字
//  		这部分汉字是存在程序空间 所以汉字的多少直接影响程序空间的剩余量
//       主要方便于就显示几个指定的汉字来说就不用调用字库了
// x  y  要显示汉字的坐标
// c[2]  汉字的数据 一个汉字两个字节表示
// dcolor 汉字的颜色   bgcolor 背景颜色
void GUI_sprintf_hz1616(unsigned short x, unsigned short y, unsigned char c[2], unsigned short pen_color, unsigned short back_color)
{
	unsigned char i, j, k, m; // 定义临时变量
	unsigned short x0, y0;
	x0 = x;
	y0 = y;
	lcdSetAddress(x, y, x + 16 - 1, y + 16 - 1); // 选择坐标位置

	for (k = 0; k < 64; k++) // 64表示自建汉字库中的个数，如超过，可以扩增
	{
		if ((code_GB_16[k].Index[0] == c[0]) && (code_GB_16[k].Index[1] == c[1])) // 寻找对应汉字	 一个汉字需要两个字节
		{
			for (i = 0; i < 32; i++) // 32个字节 每个字节都要一个点一个点处理 所以是处理了32X8次
			{
				m = code_GB_16[k].Msk[i]; // 读取对应字节数据
				for (j = 0; j < 8; j++)	  // 显示一个字节  一个字节8位 也就是8个点
				{
					// 判断是否是要写入点
					if ((m & 0x80) == 0x80)
					{
						LCD_Fast_DrawPoint(x, y, pen_color); // 如果是 给字体颜色
					}
					else
					{
						LCD_Fast_DrawPoint(x, y, back_color); // 如果不是 为背景色  给颜色
					}
					m <<= 1; // 左移一位  判断下一点
					x++;

					if (x - x0 == 16)
					{
						x = x0;
						y++;
						if (y - y0 >= 16)
						{
							return;
						}
					}
				}
			}
		}
	}
}

// 显示字符串
// x1 y1 显示的初始位置
//*str 要显示的数据
// dcolor 显示字符的颜色
// bgcolor 显示字符的背景颜色
void GUI_sprintf_hz16x(unsigned short x1, unsigned short y1, unsigned char *str, unsigned short pen_color, unsigned short back_color)
{
	unsigned char l = 0;
	while (*str)
	{
		if (*str >= 0x80)
		{
			GUI_sprintf_hz1616(x1 + l * 8, y1, (unsigned char *)str, pen_color, back_color);
			str += 2;
			l += 2;
		}
	}
}

// 显示32x32单个汉字
//(x，y) 是显示内容的左上角坐标，c[2]表示汉字的两个字节
// dcolor 汉字的颜色   bgcolor 背景颜色
// 说明：在指定位置上显示一个32x32的汉字
void GUI_sprintf_hz3232(unsigned short x, unsigned short y, unsigned char c[2], unsigned short pen_color, unsigned short back_color)
{
	unsigned char i, j, k;
	unsigned short x0, y0;
	x0 = x;
	y0 = y;

	lcdSetAddress(x, y, x + 32 - 1, y + 32 - 1); // 选择坐标位置

	for (k = 0; k < 100; k++) // 自建汉字库（k最大值和汉字库中汉字个数有关），循环查询内码
	{
		if ((code_GB_32[k].Index[0] == c[0]) && (code_GB_32[k].Index[1] == c[1])) // 寻找要显示的汉字
		{
			for (i = 0; i < 128; i++) // 1个汉字要128个字节点来显示
			{
				unsigned char m = code_GB_32[k].Msk[i]; // 一个字节一个字节的显示
				for (j = 0; j < 8; j++)
				{
					// 判断是否是要写入点
					if ((m & 0x80) == 0x80) // 如果是,给字体颜色
					{
						LCD_Fast_DrawPoint(x, y, pen_color);
					}
					else // 如果不是,为背景色
					{
						LCD_Fast_DrawPoint(x, y, back_color);
					}
					m <<= 1;
					x++;
					if (x - x0 == 32)
					{
						x = x0;
						y++;
						if (y - y0 >= 32)
						{
							return;
						}
					}
				}
			}
		}
	}
}

// 显示32x32汉字 和32x16 字母串
// x1，y1是显示内容的左上角坐标，
// dcolor 汉字的颜色   bgcolor 背景颜色
//*str 要显示的数据
void GUI_sprintf_hz32x(unsigned short x1, unsigned short y1, unsigned char *str, unsigned short pen_color, unsigned short back_color)
{
	unsigned char l = 0;
	while (*str)
	{
		if (*str >= 0x80)
		{
			GUI_sprintf_hz3232(x1 + l * 8, y1, (unsigned char *)str, pen_color, back_color);
			l += 4;
			str += 2;
		}
	}
}
