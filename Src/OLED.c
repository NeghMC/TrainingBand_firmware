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

typedef struct {
	uint8_t x;
	uint8_t y;
} point_t;

static point_t cursor;

static uint8_t screenBuffer[512];

void OLED_cmd(uint8_t cmd) {
	I2C_reserve();
	I2C_WriteReg(OLED_ADDRESS, OLED_COMMAND, &cmd, 1);
	I2C_release();
}

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

static point_t calculate_center_text_start_point(uint8_t y, uint8_t len, uint8_t size) {
	return (point_t){len * 6 * size, y};
}

static void setPixel(int x, int y, uint8_t color) {
	if(x < 0 || x >= OLED_WIDTH || y < 0 || y >= OLED_HEIGHT)
			return;

	if(color)	screenBuffer[x + (y >> 3) * OLED_WIDTH] |= (1 << (y & 0x7));
	else		screenBuffer[x + (y >> 3) * OLED_WIDTH] &= ~(1 << (y & 0x7));

}

void OLED_fillRect(int x, int y, int w, int h, uint8_t color) {
	for(uint8_t i = 0; i <= w; ++i)
		for(uint8_t j = 0; j <= h; ++j)
			setPixel(x+i, y+j, color);
}

uint8_t OLED_putc(point_t p, char c, uint8_t size, uint8_t color) {
	uint8_t line, j = 0;

	for(int8_t i = 0; i < 5; ++i) {
		line = font[(c * 5) + i];

		if(line != 0) {
			for(uint8_t k = 0; k < 8; k++) {
				if(line & 0x1) {
					if(size == 1)	setPixel(p.x + i, p.y + k, color);
					else			OLED_fillRect(p.x + i * size, p.y + k * size, size, size, color);
				}
				line >>= 1;
			}
			j = i;
		}
	}
	return j;
}

void OLED_puts(point_t p, char * s, uint8_t size, uint8_t color) {
	cursor = p;

	while(*s) {
		uint8_t toMove = OLED_putc(cursor, *s, size, color);
		cursor.x += toMove + size;
		s++;

/*		// new line
		if(cursor.x >= OLED_WIDTH - (5*size)) {
			cursor.y += 8*size + 1;
			cursor.x = x;
		}
*/
	}
}

void OLED_drawBitmap(int x, int y, const uint8_t * bitmap, uint8_t w, uint8_t h, uint8_t color, uint8_t override) {

	uint8_t i, j;

	if(override) {
		for(j = 0; j < h; j++)
			for(i = 0; i < w; i++)
				setPixel(x+i, y+j, bitmap[((j+1) * i) / 8] & (1<<(((j+1) * i) % 8))); // TODO
	} else {
		for(j = 0; j < h; j++)
			for(i = 0; i < w; i++)
				//if(bitmap[j * byteWidth + i / 8] & (128 >> (i & 7)))
					setPixel(x+i, y+i, color);
	}
}

char beat[10];


extern const uint8_t emptyBattery[];

void OLED_task(void * p) {
	OLED_init();
	OLED_clear();
	OLED_drawBitmap(3, 3, emptyBattery, 17, 11, 1, 1);
	//OLED_puts((point_t){20,20}, "hello", 1, 1);
	OLED_display();



	for(;;);

	pulseData_t newPuls, oldPuls = {0};

	for(;;) {
		newPuls = HR_getCurrentPuls();
		if(oldPuls.id != newPuls.id) {
			OLED_clear();
			fix16_to_str(newPuls.average, beat, 2);
			OLED_puts(calculate_center_text_start_point(4, strlen(beat), 2), beat, 2, 1);
			OLED_display();
			oldPuls = newPuls;
		}
		vTaskDelay(500);
	}
}

#define DIS_STACK_SIZE 250
static StackType_t disStack[DIS_STACK_SIZE];
static StaticTask_t disTask;

void OLED_createTask(void) {
	xTaskCreateStatic(OLED_task,"OT", DIS_STACK_SIZE, NULL, tskIDLE_PRIORITY+1, disStack, &disTask);
}
