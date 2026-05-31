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

// Основная функция – вызывайте её в цикле или по таймеру (рекомендуется не реже 1 кГц)
EncoderEvent_t Encoder_Event(void);

// ----- Аппаратно-зависимые функции, которые вы должны реализовать -----
// Чтение состояний выводов энкодера (возвращают 0 или 1)
extern uint8_t Encoder_State_S1(void);
extern uint8_t Encoder_State_S2(void);
extern uint8_t Encoder_State_Button(void);

// Функция для получения текущего времени в миллисекундах (или любых единицах)
// Примеры: HAL_GetTick() для STM32, millis() для Arduino.
extern uint32_t Encoder_GetTick(void);
// -------------------------------------------------------------------

// Настройка времени подавления дребезга (миллисекунды)
#define ENCODER_DEBOUNCE_MS  5

#endif // ENCODER_H