#include "encoder.h"
#include "main.h"
// Обязательно volatile, так как переменная меняется в прерывании, а читается в main
static volatile EncoderEvent_t current_event = ENCODER_NONE;

static bool stable_s1 = false;
static bool stable_s2 = false;
static bool stable_button = false;

static uint8_t debounce_s1 = 0;
static uint8_t debounce_s2 = 0;
static uint8_t debounce_button = 0;

static bool prev_s1 = false;
static bool prev_button = false;

void Encoder_Scan(void) {



    bool raw_s1 = Encoder_State_S1();
    bool raw_s2 = Encoder_State_S2();
    bool raw_button = Encoder_State_Button();

    // ----- Фильтрация S1 -----
    if (raw_s1 != stable_s1) {
        if (++debounce_s1 >= ENCODER_DEBOUNCE_MS) {
            stable_s1 = raw_s1;
            debounce_s1 = 0;
        }
    } else {
        debounce_s1 = 0;
    }

    // ----- Фильтрация S2 -----
    if (raw_s2 != stable_s2) {
        if (++debounce_s2 >= ENCODER_DEBOUNCE_MS) {
            stable_s2 = raw_s2;
            debounce_s2 = 0;
        }
    } else {
        debounce_s2 = 0;
    }

    // ----- Фильтрация кнопки -----
    if (raw_button != stable_button) {
        if (++debounce_button >= ENCODER_DEBOUNCE_MS) {
            stable_button = raw_button;
            debounce_button = 0;
        }
    } else {
        debounce_button = 0;
    }

    // ----- Детектирование кнопки (по фронту нажатия) -----
    if (stable_button && !prev_button) {
        current_event = ENCODER_BUTTON;
    }
    prev_button = stable_button;

    // ----- Классическое декодирование фаз -----
    // Ловим спадающий фронт на S1 и по состоянию S2 определяем направление.
    // Это гарантирует ровно 1 триггер на 1 физический щелчок (detent) энкодера EC11.
    if (prev_s1 && !stable_s1) {
        if (stable_s2) {
            current_event = ENCODER_RIGHT;
        } else {
            current_event = ENCODER_LEFT;
        }
    }
    prev_s1 = stable_s1;
}

EncoderEvent_t Encoder_GetEvent(void) {
    // Атомарно считываем и сбрасываем событие
    EncoderEvent_t ev = current_event;
    current_event = ENCODER_NONE;
    return ev;
}