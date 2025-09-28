#ifndef _ST7735S_WS63_H_
#define _ST7735S_WS63_H_

// GPIO
#define LCD_PIN_RES 3 // GPIO_03 0
#define LCD_PIN_DC 14 // GPIO_14 0
#define LCD_PIN_BLK 4 // GPIO_04 2

// SPI0
#define LCD_SPI_BUS_ID SPI_BUS_0
#define SPI0_PIN_CSN 10  // SPI0_CS0_N 3
#define SPI0_PIN_CLK 7 // SPI0_SCK 3
#define SPI0_PIN_RXD 11 // SPI0_IN 3
#define SPI0_PIN_TXD 9  // SPI0_OUT 3

// GPIO_
#define LCD_PIN_RES 9 // GPIO_03 0
#define LCD_PIN_DC 14 // GPIO_14 0
#define LCD_PIN_BLK 10 // GPIO_04 2

// SPI1
#define SPI1_PIN_CSN 0  // SPI1_CS0_N 3
#define SPI1_PIN_CLK 6 // SPI1_SCK 3
#define SPI1_PIN_RXD 1 // SPI1_IO0 3
#define SPI1_PIN_TXD 3  // SPI1_OUT 3

#define SET_LCD_RES uapi_gpio_set_val(LCD_PIN_RES,GPIO_LEVEL_HIGH)
#define SET_LCD_DC 	uapi_gpio_set_val(LCD_PIN_DC, GPIO_LEVEL_HIGH)
#define SET_LCD_BLK  uapi_gpio_set_val(LCD_PIN_BLK, GPIO_LEVEL_HIGH)
#define SET_LCD_CSN  uapi_gpio_set_val(SPI0_PIN_CSN, GPIO_LEVEL_HIGH)

#define	CLR_LCD_RES  uapi_gpio_set_val(LCD_PIN_RES, GPIO_LEVEL_LOW)
#define	CLR_LCD_DC  uapi_gpio_set_val(LCD_PIN_DC, GPIO_LEVEL_LOW)
#define CLR_LCD_BLK  uapi_gpio_set_val(LCD_PIN_BLK, GPIO_LEVEL_LOW)
#define CLR_LCD_CSN  uapi_gpio_set_val(SPI0_PIN_CSN, GPIO_LEVEL_LOW)

#define WHITE	0xFFFF
#define BLACK	0x0000	  
#define BLUE	0x001F  
#define BRED	0XF81F
#define GRED	0XFFE0
#define GBLUE	0X07FF
#define RED		0xF800
#define MAGENTA	0xF81F
#define GREEN	0x07E0
#define CYAN	0x7FFF
#define YELLOW	0xFFE0
#define BROWN	0XBC40 
#define BRRED	0XFC07 
#define GRAY	0X8430 
#define DARKBLUE	0X01CF	
#define LIGHTBLUE	0X7D7C	  
#define GRAYBLUE	0X5458

void spiInit(void);
void spiWrite(unsigned char data);

void lcdGpioInit(void);
void lcdInit(void);
void lcdSelectRegister(unsigned char data);
void lcdWriteDataU8(unsigned char data);
void lcdWriteDataU16(unsigned short data);

void lcdSetAddress(unsigned short x1, unsigned short y1, unsigned short x2, unsigned short y2);
void lcdClear(unsigned short color);
void lcdClearPartial(unsigned short color);

void LCD_DrawPoint(unsigned short x, unsigned short y, unsigned short color);


void LCD_DisplayOn(void); 
void LCD_DisplayOff(void); 
void LCD_Draw_Circle(unsigned short x0, unsigned short y0, unsigned char r, unsigned short color); //Բ
void LCD_DrawFullCircle(unsigned short Xpos, unsigned short Ypos, unsigned short Radius, unsigned short Color);
void LCD_Fill(unsigned short sx, unsigned short sy, unsigned short ex, unsigned short ey, unsigned short *color); //
void LCD_DrawLine(unsigned short x1, unsigned short y1, unsigned short x2, unsigned short y2, unsigned short color); //
void LCD_DrawRectangle(unsigned short x1, unsigned short y1, unsigned short x2, unsigned short y2, unsigned short color); //
void LCD_ShowChar(unsigned short x, unsigned short y, unsigned char num, unsigned char size, unsigned char mode, unsigned short pen_color, unsigned short back_color); //дASCIIַ
void LCD_ShowString(unsigned short x, unsigned short y, unsigned short width, unsigned short height, unsigned char size, unsigned char *p, unsigned short pen_color, unsigned short back_color); //дASCIIַ

void GUI_sprintf_hz1616(unsigned short x, unsigned short y, unsigned char c[2], unsigned short pen_color, unsigned short back_color);
void GUI_sprintf_hz16x(unsigned short x1, unsigned short y1, unsigned char *str, unsigned short dcolor, unsigned short bgcolor);

void GUI_sprintf_hz3232(unsigned short x, unsigned short y, unsigned char c[2], unsigned short pen_color, unsigned short back_color);
void GUI_sprintf_hz32x(unsigned short x1, unsigned short y1, unsigned char *str, unsigned short pen_color, unsigned short back_color);

#endif