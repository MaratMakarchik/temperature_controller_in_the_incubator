#ifndef FONTS_H
#define FONTS_H

#include <stdint.h>

// Структура, описывающая шрифт
typedef struct {
    const uint8_t width;      // Ширина символа
    const uint8_t height;     // Высота символа
    const uint16_t *data;     // Массив пикселей
} FontDef;

// Экспортируем наши шрифты для использования в других файлах
extern FontDef Font_7x10_RU;
extern FontDef Font_11x18_RU; // Можно добавить шрифты большего базового размера

#endif // FONTS_H