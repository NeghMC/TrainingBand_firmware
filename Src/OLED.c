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
#include <fixmath.h>

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

static struct {
	uint8_t x;
	uint8_t y;
} cursor;

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

	if(color)	screenBuffer[x + (y >> 3) * OLED_WIDTH] |= (1 << (y & 0x7));
	else		screenBuffer[x + (y >> 3) * OLED_WIDTH] &= ~(1 << (y & 0x7));

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
	cursor.x += (j ? j : 3); // je�li j == 0 to curso.xr += 3;
}

void oled_puts(int x, int y, char * s, uint8_t size, uint8_t color) {
	cursor.x = x;
	cursor.y = y;
	font_p = font;

	while(*s) {
		oled_putc(cursor.x, cursor.y, *s, size, color);
		cursor.x++;
		s++;
		if(cursor.x >= OLED_WIDTH - (5*size)) {
					cursor.y += 8*size + 1;
					cursor.x = x;
				}
	}
}

void oled_putn(int x, int y, char * s, uint8_t size, uint8_t color) {
	cursor.x = x;
	cursor.y = y;
	font_p = numbers;

	while(*s) {
		char c = (*s) - '0';
		if(c > '0' && c < '9')
			oled_putc(cursor.x, cursor.y, *s, size, color);
		cursor.x++;
		s++;
	}
}

char beat[10];

void OLED_task(void * p) {
	OLED_init();


	for(;;) {
		OLED_clear();
		fix16_to_str(beatAvg, beat, 2);
		oled_puts(0, 0, beat, 1, 1);
		OLED_display();
		vTaskDelay(1000);
	}
}

#define DIS_STACK_SIZE 250
static StackType_t disStack[DIS_STACK_SIZE];
static StaticTask_t disTask;

void OLED_createTask(void) {
	xTaskCreateStatic(OLED_task,"OT", DIS_STACK_SIZE, NULL, tskIDLE_PRIORITY+1, disStack, &disTask);
}
