/*
 * OLED.c
 *
 *  Created on: 15 sty 2021
 *      Author: NeghM
 *      library based on: https://github.com/adafruit/Adafruit_SSD1306/blob/master/Adafruit_SSD1306.cpp
 */

#include <OLED.h>
#include <I2C.h>
#include <power_men.h>
#include <FreeRTOS.h>
#include <task.h>
#include <HR_sensor.h>

#define OLED_ADDRESS 0x3c
//lub 0x3d
//#define OLED_COMMAND 0x00
#define OLED_COMMAND 0x00
#define OLED_DATA 0x40

extern const unsigned char font[];
extern const unsigned char numbers[];

#define OLED_WIDTH 128
#define OLED_HEIGHT 64

#define SWAP(a, b) do{int16_t temp = a; a = b; b = temp;}while(0)

static int cursor;
static const unsigned char * font_p;

static uint8_t screenBuffer[512];
/*
static const uint8_t configFrame3[] = {
		OLED_COMMAND,
		SSD1306_DISPLAYOFF,

		OLED_COMMAND,
		SSD1306_SETDISPLAYCLOCKDIV,

		OLED_COMMAND,
		REFRESH_MID,

		OLED_COMMAND,
		SSD1306_SETDISPLAYOFFSET,

		OLED_COMMAND,
		0x0,

		OLED_COMMAND,
		SSD1306_SETSTARTLINE | 0x0,

		OLED_COMMAND,
		SSD1306_CHARGEPUMP,

		OLED_COMMAND,
		0x14,

		OLED_COMMAND,
		SSD1306_MEMORYMODE, // horizontal memory mode

		OLED_COMMAND,
		0x00,

		OLED_COMMAND,
		SSD1306_SEGREMAP | 0x1,

		OLED_COMMAND,
		SSD1306_COMSCANDEC,

		OLED_COMMAND,
		SSD1306_SETCONTRAST,

		OLED_COMMAND,
		0xCF,

		OLED_COMMAND,
		SSD1306_SETPRECHARGE,

		OLED_COMMAND,
		SSD1306_SETMULTIPLEX,

		OLED_COMMAND,
		0x1F,

		OLED_COMMAND,
		SSD1306_SETCOMPINS,

		OLED_COMMAND,
		0x02,

		OLED_COMMAND,
		SSD1306_SETVCOMDETECT,

		OLED_COMMAND,
		0x40,

		OLED_COMMAND,
		SSD1306_DISPLAYALLON_RESUME,

		OLED_COMMAND,
		SSD1306_NORMALDISPLAY,

		OLED_COMMAND,
		SSD1306_DISPLAYON
};
*/
void OLED_cmd(uint8_t cmd) {
	I2C_reserve();
	I2C_WriteReg(OLED_ADDRESS, OLED_COMMAND, &cmd, 1);
	I2C_release();
}
/*
void OLED_data(uint8_t data) {
	I2C_WriteReg(OLED_ADDRESS, OLED_DATA, &data, 1);
}*/

void OLED_init() {
	I2C_init();
	peryph_en();
	vTaskDelay(50);

	OLED_cmd(SSD1306_DISPLAYOFF);
	OLED_cmd(SSD1306_SETDISPLAYCLOCKDIV);
	OLED_cmd(0x80);
	OLED_cmd(SSD1306_SETDISPLAYOFFSET);
	OLED_cmd(0x00);
	OLED_cmd(SSD1306_SETSTARTLINE | 0x0);
	OLED_cmd(SSD1306_CHARGEPUMP);
	OLED_cmd(0x14);
	OLED_cmd(SSD1306_MEMORYMODE); // horizontal memory mode
	OLED_cmd(0x00);
	OLED_cmd(SSD1306_SEGREMAP | 0x1);
	OLED_cmd(SSD1306_COMSCANDEC);
	// 32
	OLED_cmd(SSD1306_SETCOMPINS);
	OLED_cmd(0x02);
	OLED_cmd(SSD1306_SETMULTIPLEX);
	OLED_cmd(0x1F);
	OLED_cmd(SSD1306_SETCONTRAST);
	OLED_cmd(0x8f);

	OLED_cmd(SSD1306_SETPRECHARGE);
	OLED_cmd(0xF1);
	OLED_cmd(SSD1306_SETVCOMDETECT);
	OLED_cmd(0x40);
	OLED_cmd(SSD1306_DISPLAYALLON_RESUME);
	OLED_cmd(SSD1306_NORMALDISPLAY);
	OLED_cmd(SSD1306_DEACTIVATE_SCROLL);
	OLED_cmd(SSD1306_DISPLAYON);

	/* mirekk
	OLED_cmd(SSD1306_DISPLAYOFF);
				OLED_cmd(SSD1306_SETDISPLAYCLOCKDIV);
				OLED_cmd(REFRESH_MID);

				OLED_cmd(SSD1306_SETDISPLAYOFFSET);
				OLED_cmd(0x0);
				OLED_cmd(SSD1306_SETSTARTLINE | 0x0);

				OLED_cmd(SSD1306_CHARGEPUMP);

				OLED_cmd(0x14); // wewnetrzne zasilanie 9v
				//OLED_cmd(0x10); // zewnetrzne

				OLED_cmd(SSD1306_MEMORYMODE); // horizontal memory mode
				OLED_cmd(0x00);
				OLED_cmd(SSD1306_SEGREMAP | 0x1);
				OLED_cmd(SSD1306_COMSCANDEC);

				OLED_cmd(SSD1306_SETCONTRAST);

				//OLED_cmd(0xCF); // wewnetrzne zasilanie 9v
				//OLED_cmd(0x9F); // zewnetrzne
				OLED_cmd(0x8f);

				OLED_cmd(SSD1306_SETPRECHARGE);

				// x32
				OLED_cmd(SSD1306_SETMULTIPLEX);
				OLED_cmd(0x1F);
				OLED_cmd(SSD1306_SETCOMPINS);
				OLED_cmd(0x02);

				//OLED_cmd(SSD1306_SETVCOMDETECT); // ?
				//OLED_cmd(0x40);

				OLED_cmd(SSD1306_DISPLAYALLON_RESUME);
				OLED_cmd(SSD1306_NORMALDISPLAY);

				OLED_cmd(SSD1306_DISPLAYON);
*/
}

void OLED_clear() {
	for(int i = 0; i < 512; ++i)
		screenBuffer[i] = 0;
}

void OLED_display(void) {
	OLED_cmd(SSD1306_PAGEADDR);
	OLED_cmd(0x00);
	OLED_cmd(0xFF);
	OLED_cmd(SSD1306_COLUMNADDR);
	OLED_cmd(0x00);
	OLED_cmd(0x7f);

	I2C_reserve();
	I2C_WriteReg(OLED_ADDRESS, OLED_DATA, screenBuffer, sizeof(screenBuffer));
	I2C_release();
}

static void oled_setPixel(int x, int y, uint8_t color) {
	if(x < 0 || x >= OLED_WIDTH || y < 0 || y >= OLED_HEIGHT)
			return;

	if(color)	screenBuffer[x + (y >> 8) * OLED_WIDTH] |= (1 << (y & 0x7));
	else		screenBuffer[x + (y >> 8) * OLED_WIDTH] &= ~(1 << (y & 0x7));

}

// algorytm Bresenhama
void oled_drawLine(int x1, int y1, int x2, int y2, uint8_t color) {

	register int8_t dx = x2 - x1;
	register int8_t dy = y2 - y1;
	register const int8_t x_step = (dx < 0) ? -1 : 1;
	register const int8_t y_step = (dy < 0) ? -1 : 1;

	dx = abs(dx);
	dy = abs(dy);

	if(dx < dy) { // sprowadzamy do warunku, os nachylona maksymalnie 45 stopni do OX
		SWAP(x1, y1);
		SWAP(x2, y2);
		SWAP(dx, dy);
	}

	oled_setPixel(x1, y1, color);
	int8_t error = dx >> 1;
	for(uint8_t i = dx; i; i--) {
		x1 += x_step;
		error -= dy;
		if(error < 0) {
			y1 += y_step;
			error += dx;
		}
		oled_setPixel(x1, y1, color);
	}
}

void oled_fillRect(int x, int y, int w, int h, uint8_t color) {
	for(uint8_t i = 0; i <= w; ++i)
		for(uint8_t j = 0; j <= h; ++j)
			oled_setPixel(x+i, y+j, color);
}

void oled_putc(int x, int y, char c, uint8_t size, uint8_t color) { // @TODO nie ma spacji
	if((x + 6 * size - 1) < 0 || x >= OLED_WIDTH || (y + 8 * size - 1) < 0 || y >= OLED_HEIGHT)
			return;

	uint8_t line, j = 0;

	for(int8_t i = 0; i < 5; ++i) { // oryginalny font jest 5 na 8
		line = font[(c * 5) + i]; // pobieramy pionow� linijk� fonta

		if(line != 0) { // rysujemy tylko 'znacace' linie
			for(uint8_t k = 0; k < 8; k++) {
				if(line & 0x1) { // je�li rysujemy pixel fonta
					if(size == 1)	oled_setPixel(x + j * size, y + k * size, color);
					else			oled_fillRect(x + j * size, y + k * size, size, size, color);
				}
				line >>= 1;
			}
			j++;
		}
	}
	cursor += (j ? j : 3); // je�li j == 0 to cursor += 3;
}

void oled_puts(int x, int y, char * s, uint8_t size, uint8_t color) {
	cursor = x;
	font_p = font;

	while(*s) {
		oled_putc(cursor, y, *s, size, color);
		cursor++;
		s++;
	}
}

void oled_putn(int x, int y, char * s, uint8_t size, uint8_t color) {
	cursor = x;
	font_p = numbers;

	while(*s) {
		char c = (*s) - '0';
		if(c > '0' && c < '9')
			oled_putc(cursor, y, *s, size, color);
		cursor++;
		s++;
	}
}

char * ftoa(double f, char * buf, int precision);


void OLED_task(void * p) {
	OLED_init();

	char str[10];

	for(;;) {
		ftoa(beatRate, str, 1);
		OLED_clear();
		oled_puts(1, 1, str, 1, 1);
		OLED_display();
		vTaskDelay(1000);
	}
}

#define DIS_STACK_SIZE 128
static StackType_t disStack[DIS_STACK_SIZE];
static StaticTask_t disTask;

void OLED_createTask(void) {
	xTaskCreateStatic(OLED_task,"OT", DIS_STACK_SIZE, NULL, tskIDLE_PRIORITY+1, disStack, &disTask);
}

#define MAX_PRECISION	(10)
static const double rounders[MAX_PRECISION + 1] =
{
	0.5,				// 0
	0.05,				// 1
	0.005,				// 2
	0.0005,				// 3
	0.00005,			// 4
	0.000005,			// 5
	0.0000005,			// 6
	0.00000005,			// 7
	0.000000005,		// 8
	0.0000000005,		// 9
	0.00000000005		// 10
};

char * ftoa(double f, char * buf, int precision)
{
	char * ptr = buf;
	char * p = ptr;
	char * p1;
	char c;
	long intPart;

	// check precision bounds
	if (precision > MAX_PRECISION)
		precision = MAX_PRECISION;

	// sign stuff
	if (f < 0)
	{
		f = -f;
		*ptr++ = '-';
	}

	if (precision < 0)  // negative precision == automatic precision guess
	{
		if (f < 1.0) precision = 6;
		else if (f < 10.0) precision = 5;
		else if (f < 100.0) precision = 4;
		else if (f < 1000.0) precision = 3;
		else if (f < 10000.0) precision = 2;
		else if (f < 100000.0) precision = 1;
		else precision = 0;
	}

	// round value according the precision
	if (precision)
		f += rounders[precision];

	// integer part...
	intPart = f;
	f -= intPart;

	if (!intPart)
		*ptr++ = '0';
	else
	{
		// save start pointer
		p = ptr;

		// convert (reverse order)
		while (intPart)
		{
			*p++ = '0' + intPart % 10;
			intPart /= 10;
		}

		// save end pos
		p1 = p;

		// reverse result
		while (p > ptr)
		{
			c = *--p;
			*p = *ptr;
			*ptr++ = c;
		}

		// restore end pos
		ptr = p1;
	}

	// decimal part
	if (precision)
	{
		// place decimal point
		*ptr++ = '.';

		// convert
		while (precision--)
		{
			f *= 10.0;
			c = f;
			*ptr++ = '0' + c;
			f -= c;
		}
	}

	// terminating zero
	*ptr = 0;

	return buf;
}
