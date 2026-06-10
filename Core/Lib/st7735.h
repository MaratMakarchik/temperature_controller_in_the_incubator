#ifndef ST7735_H
#define ST7735_H

#include <stdint.h>
#include "fonts.h" // Подключаем наши шрифты

// Основные цвета (формат RGB565)
#define ST7735_BLACK   0x0000
#define ST7735_WHITE   0xFFFF
#define ST7735_RED     0xF800
#define ST7735_GREEN   0x07E0
#define ST7735_BLUE    0x001F
#define ST7735_YELLOW  0xFFE0

// --- Аппаратно зависимые функции (HAL Layer) ---
extern void ST7735_SPI_Send(uint8_t *data, uint16_t len);
extern void ST7735_SetCS(uint8_t state);
extern void ST7735_SetDC(uint8_t state);
extern void ST7735_SetRST(uint8_t state);
extern void ST7735_DelayMs(uint32_t ms);

// --- API Библиотеки ---
void ST7735_Init(void);
void ST7735_SetWindow(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1);
void ST7735_FillRect(uint8_t x, uint8_t y, uint8_t w, uint8_t h, uint16_t color);
void ST7735_FillScreen(uint16_t color);

// Функции работы с текстом (добавлен параметр scale)
void ST7735_DrawChar(uint8_t x, uint8_t y, char ch, FontDef font, uint16_t color, uint16_t bgcolor, uint8_t scale);
void ST7735_WriteString(uint8_t x, uint8_t y, const char* str, FontDef font, uint16_t color, uint16_t bgcolor, uint8_t scale);

// Функция вывода изображения
void ST7735_DrawImage(uint8_t x, uint8_t y, uint8_t w, uint8_t h, const uint16_t *data);

#endif // ST7735_H