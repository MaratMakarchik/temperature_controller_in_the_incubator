#include "encoder.h"
#include "main.h"

// Глобальный флаг инициализации (определение)
bool flag_init = false;

// Текущее событие (используется как кольцевой буфер, но пока храним только последнее)
static volatile EncoderEvent_t current_event = ENCODER_NONE;

// Стабильные (отфильтрованные) состояния сигналов
static bool stable_s1 = false;
static bool stable_s2 = false;
static bool stable_button = false;

// Счётчики для антидребезга (число последовательных совпадений)
static uint8_t debounce_s1 = 0;
static uint8_t debounce_s2 = 0;
static uint8_t debounce_button = 0;

// Предыдущие состояния для детектирования фронтов
static bool prev_s1;
static bool prev_button = false;

// Инициализация: запоминаем текущие состояния и устанавливаем флаг
void Encoder_init(void) {
    // Считываем "сырые" значения и считаем их стабильными
    stable_s1 = Encoder_State_S1();
    stable_s2 = Encoder_State_S2();
    stable_button = Encoder_State_Button();

    prev_s1 = stable_s1;
    prev_button = stable_button;

    // Сбрасываем счётчики
    debounce_s1 = 0;
    debounce_s2 = 0;
    debounce_button = 0;

    flag_init = true;
}

// Основная функция сканирования (вызывается по таймеру каждые 1 мс)
void Encoder_Scan(void) {
    bool raw_s1 = Encoder_State_S1();
    bool raw_s2 = Encoder_State_S2();
    bool raw_button = Encoder_State_Button();

    // ----- Фильтрация S1 (подсчёт последовательных совпадений) -----
    if (raw_s1 == stable_s1) {
        debounce_s1 = 0;                // совпадает – сбрасываем счётчик
    } else {
        if (++debounce_s1 >= ENCODER_DEBOUNCE_MS) {
            stable_s1 = raw_s1;         // подтвердили изменение
            debounce_s1 = 0;
        }
    }

    // ----- Фильтрация S2 -----
    if (raw_s2 == stable_s2) {
        debounce_s2 = 0;
    } else {
        if (++debounce_s2 >= ENCODER_DEBOUNCE_MS) {
            stable_s2 = raw_s2;
            debounce_s2 = 0;
        }
    }

    // ----- Фильтрация кнопки -----
    if (raw_button == stable_button) {
        debounce_button = 0;
    } else {
        if (++debounce_button >= ENCODER_DEBOUNCE_MS) {
            stable_button = raw_button;
            debounce_button = 0;
        }
    }

    // ----- Детектирование кнопки (по фронту нажатия) -----
    if (stable_button && !prev_button) {
        current_event = ENCODER_BUTTON;
    }
    prev_button = stable_button;

    // ----- Декодирование направления энкодера -----
    // Используем классический метод: ловим спадающий фронт на S1,
    // и по состоянию S2 определяем направление.
    // Если за один вызов изменились оба канала – игнорируем (защита от ошибок).
    bool s1_changed = (prev_s1 != stable_s1);
    bool s2_changed = (prev_s2 != stable_s2); // используем старую stable_s2

    if (s1_changed && !s2_changed) {
        // Спадающий фронт на S1
        if (prev_s1 && !stable_s1) {
            if (stable_s2) {
                current_event = ENCODER_RIGHT;
            } else {
                current_event = ENCODER_LEFT;
            }
        }
        // Можно также обработать нарастающий фронт, но для EC11 достаточно спадающего
    }

    // Обновляем предыдущие состояния для следующего вызова
    prev_s1 = stable_s1;
    // prev_s2 больше не нужен, но для симметрии можно обновлять
    // prev_s2 = stable_s2; // не используется
}

// Атомарное получение и сброс события
EncoderEvent_t Encoder_GetEvent(void) {
    EncoderEvent_t ev;
    // Защита от прерывания, которое может изменить current_event
    __disable_irq();
    ev = current_event;
    current_event = ENCODER_NONE;
    __enable_irq();
    return ev;
}