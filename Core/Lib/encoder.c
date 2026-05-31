#include "encoder.h"

EncoderEvent_t Encoder_Event(void) {
    // Стабильные состояния после подавления дребезга
    static bool stable_s1 = false;
    static bool stable_s2 = false;
    static bool stable_button = false;

    // Время последнего изменения сырого сигнала
    static uint32_t last_change_time_s1 = 0;
    static uint32_t last_change_time_s2 = 0;
    static uint32_t last_change_time_button = 0;

    static bool first_call = true;

    // Предыдущие стабильные состояния для детектирования фронтов
    static bool prev_s1 = false;
    static bool prev_s2 = false;
    static bool prev_button = false;

    // Сырые значения с пинов
    bool raw_s1 = Encoder_State_S1();
    bool raw_s2 = Encoder_State_S2();
    bool raw_button = Encoder_State_Button();
    uint32_t now = Encoder_GetTick();

    // Первый вызов – просто запоминаем текущие состояния
    if (first_call) {
        stable_s1 = raw_s1;
        stable_s2 = raw_s2;
        stable_button = raw_button;
        prev_s1 = stable_s1;
        prev_s2 = stable_s2;
        prev_button = stable_button;
        first_call = false;
        return ENCODER_NONE;
    }

    // ----- Фильтрация S1 -----
    if (raw_s1 != stable_s1) {
        if (now - last_change_time_s1 >= ENCODER_DEBOUNCE_MS) {
            stable_s1 = raw_s1;
        }
        last_change_time_s1 = now;
    }

    // ----- Фильтрация S2 -----
    if (raw_s2 != stable_s2) {
        if (now - last_change_time_s2 >= ENCODER_DEBOUNCE_MS) {
            stable_s2 = raw_s2;
        }
        last_change_time_s2 = now;
    }

    // ----- Фильтрация кнопки -----
    if (raw_button != stable_button) {
        if (now - last_change_time_button >= ENCODER_DEBOUNCE_MS) {
            stable_button = raw_button;
        }
        last_change_time_button = now;
    }

    // ----- Детектирование событий (только по стабильным состояниям) -----
    EncoderEvent_t event = ENCODER_NONE;

    // Обработка кнопки (фронт нажатия – предполагаем, что 1 = нажата)
    if (stable_button != prev_button) {
        if (stable_button) {
            event = ENCODER_BUTTON;
        }
        prev_button = stable_button;
    }

    // Обработка поворота энкодера
    if ((prev_s1 ^ prev_s2 ^ stable_s1 ^ stable_s2)) {
        if (prev_s2 ^ stable_s1) {
            if (event == ENCODER_NONE) event = ENCODER_LEFT;
        } else {
            if (event == ENCODER_NONE) event = ENCODER_RIGHT;
        }
        prev_s1 = stable_s1;
        prev_s2 = stable_s2;
    }

    return event;
}