#pragma once

#include <stdint.h>
#include <stdbool.h>

//Display resolution
#define LCD_WIDTH	160
#define LCD_HEIGHT	128

//Colors with inverted bytes
#define BLACK			0x0000
#define RED				0x00f8
#define GREEN			0xe007
#define BLUE			0x1f00
#define YELLOW			0xe0ff
#define MAGENTA			0x1ff8
#define CYAN			0xff07
#define WHITE			0xffff

void lcd_init(void); // Initializing display
void lcd_put_pixel(int x, int y, uint16_t color);
void lcd_copy(void); // Send buffer to display
void lcd_transfer_done(void);
bool lcd_is_busy(void);
void lcd_put_pixel_8(int x, int y, uint8_t color1, uint8_t color2);
void lcd_draw_image_8(int x0, int y0, int width, int height, const uint8_t* image);

