#ifndef ENCODER_H
#define ENCODER_H

#include <stdint.h>
#include <stdbool.h>

typedef enum {
    ENCODER_NONE,
    ENCODER_LEFT,
    ENCODER_RIGHT,
    ENCODER_BUTTON
} EncoderEvent_t;

// Флаг, указывающий, что инициализация выполнена (используется в прерывании)
extern bool flag_init;

// Функция инициализации (вызывается один раз в main)
void Encoder_init(void);

// Эту функцию вызываем в прерывании таймера строго каждые 1 мс
void Encoder_Scan(void);

// Эту функцию вызываем в основном цикле для получения событий
EncoderEvent_t Encoder_GetEvent(void);

// Аппаратно-зависимые функции чтения пинов (реализованы в hal_encoder.c)
extern bool Encoder_State_S1(void);
extern bool Encoder_State_S2(void);
extern bool Encoder_State_Button(void);

// Количество последовательных одинаковых отсчётов для подавления дребезга
// При вызове Scan каждую 1 мс это значение равно времени в мс
#define ENCODER_DEBOUNCE_MS  5

#endif // ENCODER_H