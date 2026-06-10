#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
Генератор растровых шрифтов для встраиваемых систем (C/C++).
Создаёт массив uint16_t для моноширинного шрифта 7x10 пикселей.
Поддерживает символы ASCII и кириллицу в кодировке CP1251.
Требует установки Pillow: pip install pillow

python generate_font.py --font "C:\Windows\Fonts\Arial.ttf" --size 18 --width 11 --height 18 --output Font_11x18_RU.c
--size 18 – размер шрифта в пунктах (подбирается так, чтобы символы вписывались в 11×18).

--width 11, --height 18 – размер ячейки символа.

--start 32 (по умолчанию) – пробел.

--end 255 (по умолчанию) – последний символ CP1251.
"""

import argparse
from PIL import Image, ImageDraw, ImageFont

def cp1251_to_unicode(code):
    """Преобразует код CP1251 (0x80–0xFF) в символ Unicode."""
    if code < 0x80:
        return chr(code)                     # ASCII
    # Таблица CP1251 -> Unicode для кодов 0x80..0xFF
    cp1251_table = (
        0x0402, 0x0403, 0x201A, 0x0453, 0x201E, 0x2026, 0x2020, 0x2021,
        0x20AC, 0x2030, 0x0409, 0x2039, 0x040A, 0x040C, 0x040B, 0x040F,
        0x0452, 0x2018, 0x2019, 0x201C, 0x201D, 0x2022, 0x2013, 0x2014,
        0x0000, 0x2122, 0x0459, 0x203A, 0x045A, 0x045C, 0x045B, 0x045F,
        0x00A0, 0x040E, 0x045E, 0x0408, 0x00A4, 0x0490, 0x00A6, 0x00A7,
        0x0401, 0x00A9, 0x0404, 0x00AB, 0x00AC, 0x00AD, 0x00AE, 0x0407,
        0x00B0, 0x00B1, 0x0406, 0x0456, 0x0491, 0x00B5, 0x00B6, 0x00B7,
        0x0451, 0x2116, 0x0454, 0x00BB, 0x0458, 0x0405, 0x0455, 0x0457,
        0x0410, 0x0411, 0x0412, 0x0413, 0x0414, 0x0415, 0x0416, 0x0417,
        0x0418, 0x0419, 0x041A, 0x041B, 0x041C, 0x041D, 0x041E, 0x041F,
        0x0420, 0x0421, 0x0422, 0x0423, 0x0424, 0x0425, 0x0426, 0x0427,
        0x0428, 0x0429, 0x042A, 0x042B, 0x042C, 0x042D, 0x042E, 0x042F,
        0x0430, 0x0431, 0x0432, 0x0433, 0x0434, 0x0435, 0x0436, 0x0437,
        0x0438, 0x0439, 0x043A, 0x043B, 0x043C, 0x043D, 0x043E, 0x043F,
        0x0440, 0x0441, 0x0442, 0x0443, 0x0444, 0x0445, 0x0446, 0x0447,
        0x0448, 0x0449, 0x044A, 0x044B, 0x044C, 0x044D, 0x044E, 0x044F,
    )
    idx = code - 0x80
    if 0 <= idx < len(cp1251_table):
        return chr(cp1251_table[idx])
    return '?'  # fallback

def render_char(unicode_char, font, width, height):
    """
    Рендерит символ в битовую карту размера width x height.
    Возвращает список строк (каждая строка — список битов 0/1).
    """
    img = Image.new('1', (width, height), 0)   # чёрный фон, белый символ
    draw = ImageDraw.Draw(img)

    # Определяем bounding box символа
    try:
        bbox = draw.textbbox((0, 0), unicode_char, font=font)
    except:
        bbox = (0, 0, 0, 0)
    text_width = bbox[2] - bbox[0]
    text_height = bbox[3] - bbox[1]

    # Позиция для центрирования
    x = (width - text_width) // 2 - bbox[0]
    y = (height - text_height) // 2 - bbox[1]

    draw.text((x, y), unicode_char, font=font, fill=1)

    # Извлекаем пиксели
    pixels = [[img.getpixel((col, row)) for col in range(width)] for row in range(height)]
    return pixels

def char_to_words(pixels, width):
    """
    Преобразует битовую карту символа в список 16-битных слов.
    Каждая строка кодируется в младшие bits_width бит (бит 0 — левый пиксель).
    """
    words = []
    for row in pixels:
        word = 0
        for col, bit in enumerate(row):
            if bit:
                # бит (width-1-col) — левый пиксель становится старшим из используемых битов
                # но мы помещаем его в младшие биты для простоты
                word |= (1 << (width - 1 - col))
        words.append(word & 0xFFFF)
    return words

def generate_font(font_path, font_size, cell_width, cell_height, start_code, end_code, output_file):
    """
    Основная функция генерации C-массива.
    """
    try:
        font = ImageFont.truetype(font_path, font_size)
    except Exception as e:
        print(f"Ошибка загрузки шрифта {font_path}: {e}")
        return

    data = []
    for code in range(start_code, end_code + 1):
        # Получаем символ в кодировке CP1251 (для кодов > 127)
        if code < 128:
            char = chr(code)
        else:
            char = cp1251_to_unicode(code)

        # Рендерим символ (если не удалось — пустое изображение)
        try:
            pixels = render_char(char, font, cell_width, cell_height)
        except:
            pixels = [[0]*cell_width for _ in range(cell_height)]

        words = char_to_words(pixels, cell_width)
        data.extend(words)

    # Запись C-массива
    with open(output_file, 'w', encoding='utf-8') as f:
        f.write("// Растровый шрифт {}x{}\n".format(cell_width, cell_height))
        f.write("// Сгенерирован из {}\n".format(font_path))
        f.write("// Диапазон символов: {}..{} (CP1251)\n".format(start_code, end_code))
        f.write("static const uint16_t Font7x10_Data[] = {\n")
        # Выводим по 10 слов в строке для удобочитаемости
        words_per_line = 10
        for i in range(0, len(data), words_per_line):
            chunk = data[i:i+words_per_line]
            line = ", ".join(f"0x{word:04X}" for word in chunk)
            f.write("    " + line + ",\n")
        f.write("};\n")
        f.write("// Количество символов: {}\n".format(end_code - start_code + 1))
        f.write("// Количество слов: {}\n".format(len(data)))

    print(f"Массив успешно записан в {output_file}")

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="Генератор растровых шрифтов 7x10 для C (CP1251)")
    parser.add_argument("--font", required=True, help="Путь к TrueType-шрифту (.ttf)")
    parser.add_argument("--size", type=int, default=10, help="Размер шрифта в пунктах (по умолчанию 10)")
    parser.add_argument("--width", type=int, default=7, help="Ширина символа в пикселях (по умолч. 7)")
    parser.add_argument("--height", type=int, default=10, help="Высота символа в пикселях (по умолч. 10)")
    parser.add_argument("--start", type=int, default=32, help="Начальный код символа (по умолч. 32)")
    parser.add_argument("--end", type=int, default=255, help="Конечный код символа (по умолч. 255)")
    parser.add_argument("--output", default="font_data.c", help="Выходной C-файл (по умолч. font_data.c)")
    args = parser.parse_args()

    generate_font(
        font_path=args.font,
        font_size=args.size,
        cell_width=args.width,
        cell_height=args.height,
        start_code=args.start,
        end_code=args.end,
        output_file=args.output
    )