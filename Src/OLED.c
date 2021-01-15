/*
 * OLED.c
 *
 *  Created on: 15 sty 2021
 *      Author: NeghM
 */

#include <OLED.h>
#include <I2C.h>
#include <power_men.h>
#include <FreeRTOS.h>
#include <task.h>

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

static uint8_t buffer[512 + 1] = {
		OLED_DATA
};

static uint8_t * screenBuffer;

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

void OLED_cmd(uint8_t cmd) {
	uint8_t com[] = {OLED_COMMAND, cmd};
	I2C_Transmit(com, 2, OLED_ADDRESS);
}

void OLED_init() {
	I2C_init();
	peryph_en();
	vTaskDelay(50);
	OLED_cmd(SSD1306_DISPLAYOFF);
				OLED_cmd(SSD1306_SETDISPLAYCLOCKDIV);
				OLED_cmd(REFRESH_MID);

				OLED_cmd(SSD1306_SETDISPLAYOFFSET);
				OLED_cmd(0x0);
				OLED_cmd(SSD1306_SETSTARTLINE | 0x0);

				OLED_cmd(SSD1306_CHARGEPUMP);
				// wewnetrzne zasilanie 9v
				OLED_cmd(0x14);
				//OLED_cmd(0x10); // zewnetrzne

				OLED_cmd(SSD1306_MEMORYMODE); // horizontal memory mode
				OLED_cmd(0x00);
				OLED_cmd(SSD1306_SEGREMAP | 0x1);
				OLED_cmd(SSD1306_COMSCANDEC);

				OLED_cmd(SSD1306_SETCONTRAST);
				// wewnetrzne zasilanie 9v
				OLED_cmd(0xCF);
				//OLED_cmd(0x9F); // zewnetrzne

				OLED_cmd(SSD1306_SETPRECHARGE);

				// x32
				OLED_cmd(SSD1306_SETMULTIPLEX);
				OLED_cmd(0x3F);
				OLED_cmd(SSD1306_SETCOMPINS);
				OLED_cmd(0x12);

				OLED_cmd(SSD1306_SETVCOMDETECT);
				OLED_cmd(0x40);

				OLED_cmd(SSD1306_DISPLAYALLON_RESUME);
				OLED_cmd(SSD1306_NORMALDISPLAY);

				OLED_cmd(SSD1306_DISPLAYON);

	//I2C_Transmit(configFrame, sizeof(configFrame), OLED_ADDRESS);
	screenBuffer = buffer + 1;
}

void OLED_clear() {
	for(int i = 0; i < 512; ++i)
		screenBuffer[i] = 0;
}

void OLED_display(void) {
	OLED_cmd(SSD1306_SETLOWCOLUMN | 0x0);
	OLED_cmd(SSD1306_SETHIGHCOLUMN | 0x0);
	OLED_cmd(SSD1306_SETSTARTLINE | 0x0);
	I2C_Transmit(buffer, 512, OLED_ADDRESS);
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
