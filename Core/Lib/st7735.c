//
// Created by Makar Makarov on 30.05.2026.
//

#include "st7735.h"

// Вспомогательные функции отправки (без изменений)
static void ST7735_SendCommand(uint8_t cmd) {
    ST7735_SetDC(0);
    ST7735_SetCS(0);
    ST7735_SPI_Send(&cmd, 1);
    ST7735_SetCS(1);
}

static void ST7735_SendData(uint8_t *data, uint16_t len) {
    ST7735_SetDC(1);
    ST7735_SetCS(0);
    ST7735_SPI_Send(data, len);
    ST7735_SetCS(1);
}

void ST7735_Init(void) {
    // Аппаратный сброс
    ST7735_SetRST(0);
    ST7735_DelayMs(50);
    ST7735_SetRST(1);
    ST7735_DelayMs(50);

    // Базовая инициализация (сокращенный список для примера)
    ST7735_SendCommand(0x11); // Sleep out
    ST7735_DelayMs(120);

    ST7735_SendCommand(0x3A); // Color mode
    uint8_t color_mode = 0x05; // 16-bit/pixel
    ST7735_SendData(&color_mode, 1);

    ST7735_SendCommand(0x36); // Memory access control (Ориентация)
    uint8_t madctl = 0xA0;    // Настройки для ландшафтной/портретной ориентации
    ST7735_SendData(&madctl, 1);

    ST7735_SendCommand(0x29); // Display ON
}

void ST7735_SetWindow(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1) {
    uint8_t data[4];

    ST7735_SendCommand(0x2A); // Column addr set
    data[0] = 0x00; data[1] = x0; data[2] = 0x00; data[3] = x1;
    ST7735_SendData(data, 4);

    ST7735_SendCommand(0x2B); // Row addr set
    data[0] = 0x00; data[1] = y0; data[2] = 0x00; data[3] = y1;
    ST7735_SendData(data, 4);

    ST7735_SendCommand(0x2C); // Write to RAM
}

void ST7735_FillRect(uint8_t x, uint8_t y, uint8_t w, uint8_t h, uint16_t color) {
    // 1. Задаем окно отрисовки
    ST7735_SetWindow(x, y, x + w - 1, y + h - 1);

    ST7735_SetDC(1);
    ST7735_SetCS(0);

    // 2. Вычисляем общее количество пикселей для закраски
    uint32_t total_pixels = w * h;

    // 3. Создаем буфер порций. 64 пикселя = 128 байт.
    // Размер 128 байт легко помещается в стек любой STM32 и не расходует кучу RAM.
#define CHUNK_SIZE 64
    uint8_t tx_buf[CHUNK_SIZE * 2];

    uint8_t color_high = color >> 8;
    uint8_t color_low = color & 0xFF;

    // Заполняем буфер шаблоном нашего цвета один раз
    for (uint16_t i = 0; i < CHUNK_SIZE; i++) {
        tx_buf[i * 2]     = color_high;
        tx_buf[i * 2 + 1] = color_low;
    }

    // 4. Отправляем данные порциями
    while (total_pixels > 0) {
        uint16_t pixels_to_send = (total_pixels > CHUNK_SIZE) ? CHUNK_SIZE : total_pixels;

        // Отправляем (количество пикселей * 2 байта) за один вызов
        ST7735_SPI_Send(tx_buf, pixels_to_send * 2);

        total_pixels -= pixels_to_send;
    }

    ST7735_SetCS(1);
}

void ST7735_FillScreen(uint16_t color) {
    ST7735_FillRect(0, 0, 160, 128, color); // Измените разрешение под ваш дисплей
}

// Отрисовка символа с поддержкой масштабирования и прозрачности
void ST7735_DrawChar(uint8_t x, uint8_t y, char ch, FontDef font, uint16_t color, uint16_t bgcolor, uint8_t scale) {
    uint8_t char_index = (uint8_t)ch;
    if (char_index < 32) return;

    // Оптимизированный быстрый вывод для масштаба 1:1
    if (scale == 1) {
        ST7735_SetWindow(x, y, x + font.width - 1, y + font.height - 1);
        ST7735_SetDC(1);
        ST7735_SetCS(0);

        for (uint32_t i = 0; i < font.height; i++) {
            uint16_t b = font.data[(char_index - 32) * font.height + i];
            for (uint32_t j = 0; j < font.width; j++) {
                if ((b >> (font.width - 1 - j)) & 1) {
                    uint8_t data[] = {color >> 8, color & 0xFF};
                    ST7735_SPI_Send(data, 2);
                } else {
                    // Если цвет фона совпадает с цветом текста - фон не рисуем (игнорируем для scale=1 при SetWindow)
                    // Но SetWindow требует отправки всех пикселей.
                    uint8_t data[] = {bgcolor >> 8, bgcolor & 0xFF};
                    ST7735_SPI_Send(data, 2);
                }
            }
        }
        ST7735_SetCS(1);
    }
    // Вывод с масштабированием (каждый пиксель превращается в квадрат)
    else {
        for (uint32_t i = 0; i < font.height; i++) {
            uint16_t b = font.data[(char_index - 32) * font.height + i];
            for (uint32_t j = 0; j < font.width; j++) {
                if ((b >> (font.width - 1 - j)) & 1) {
                    ST7735_FillRect(x + (j * scale), y + (i * scale), scale, scale, color);
                } else if (bgcolor != color) {
                    // Хак: Если bgcolor == color, мы считаем фон ПРОЗРАЧНЫМ и не закрашиваем его
                    ST7735_FillRect(x + (j * scale), y + (i * scale), scale, scale, bgcolor);
                }
            }
        }
    }
}

// Вывод строки (с поддержкой кириллицы UTF-8 и масштаба)
void ST7735_WriteString(uint8_t x, uint8_t y, const char* str, FontDef font, uint16_t color, uint16_t bgcolor, uint8_t scale) {
    uint8_t cursor_x = x;

    while (*str) {
        uint8_t c = *str++;

        // Декодер UTF-8 -> CP1251
        if (c >= 0xC0) {
            uint8_t c2 = *str++;
            if (c == 0xD0 && c2 >= 0x90 && c2 <= 0xBF) {
                c = c2 - 0x90 + 192; // А-Я, а-п
            } else if (c == 0xD1 && c2 >= 0x80 && c2 <= 0x8F) {
                c = c2 - 0x80 + 240; // р-я
            } else if (c == 0xD0 && c2 == 0x81) {
                c = 168; // Ё
            } else if (c == 0xD1 && c2 == 0x91) {
                c = 184; // ё
            }
        }

        ST7735_DrawChar(cursor_x, y, c, font, color, bgcolor, scale);
        cursor_x += font.width * scale;

        // Перенос строки
        if (cursor_x + (font.width * scale) > 160) {
            cursor_x = x;
            y += font.height * scale;
        }
    }
}

// Вывод изображения из массива RGB565
void ST7735_DrawImage(uint8_t x, uint8_t y, uint8_t w, uint8_t h, const uint16_t *data) {
    ST7735_SetWindow(x, y, x + w - 1, y + h - 1);
    ST7735_SetDC(1);
    ST7735_SetCS(0);

    // STM32 использует Little-Endian, а дисплей ожидает Big-Endian.
    // Если массив картинки не был заранее подготовлен со свапом байтов,
    // мы делаем это "на лету" перед отправкой по SPI.
    for (uint32_t i = 0; i < (w * h); i++) {
        uint8_t buf[2];
        buf[0] = data[i] >> 8;   // High byte
        buf[1] = data[i] & 0xFF; // Low byte
        ST7735_SPI_Send(buf, 2);
    }

    ST7735_SetCS(1);
}