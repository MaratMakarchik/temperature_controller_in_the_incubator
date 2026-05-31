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

// Эту функцию вызываем в прерывании таймера строго каждые 1 мс
void Encoder_Scan(void);

// Эту функцию вызываем в основном цикле while(1) для обработки событий
EncoderEvent_t Encoder_GetEvent(void);

// Аппаратно-зависимые функции
extern uint8_t Encoder_State_S1(void);
extern uint8_t Encoder_State_S2(void);
extern uint8_t Encoder_State_Button(void);

// Время подавления дребезга (теперь в количестве вызовов, т.е. миллисекундах)
#define ENCODER_DEBOUNCE_MS  5

#endif // ENCODER_H