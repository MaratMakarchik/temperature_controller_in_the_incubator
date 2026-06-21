#include "main.h"
#include "encoder.h"
#include "stm32f1xx_hal.h"

extern TIM_HandleTypeDef htim2;

// Возвращаем true, если нажато (активный уровень – GND, т.е. RESET)
bool Encoder_State_S1(void) {
    return (HAL_GPIO_ReadPin(ENCODER_S1_GPIO_Port, ENCODER_S1_Pin) == GPIO_PIN_RESET);
}

bool Encoder_State_S2(void) {
    return (HAL_GPIO_ReadPin(ENCODER_S2_GPIO_Port, ENCODER_S2_Pin) == GPIO_PIN_RESET);
}

bool Encoder_State_Button(void) {
    return (HAL_GPIO_ReadPin(ENCODER_KEY_GPIO_Port, ENCODER_KEY_Pin) == GPIO_PIN_RESET);
}

// Прерывание таймера (вызывается каждые 1 мс)
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim) {
    if (htim->Instance == TIM2 && flag_init == true) {
        Encoder_Scan();
    }
}