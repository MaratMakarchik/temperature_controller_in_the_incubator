#include "main.h"
#include "encoder.h"
#include "stm32f1xx_hal.h"

extern TIM_HandleTypeDef htim2;

uint8_t Encoder_State_S1(void) {
    return (HAL_GPIO_ReadPin(ENCODER_S1_GPIO_Port, ENCODER_S1_Pin) == GPIO_PIN_RESET);
}

uint8_t Encoder_State_S2(void) {
    return (HAL_GPIO_ReadPin(ENCODER_S2_GPIO_Port, ENCODER_S2_Pin) == GPIO_PIN_RESET);
}

uint8_t Encoder_State_Button(void) {
    return (HAL_GPIO_ReadPin(ENCODER_KEY_GPIO_Port, ENCODER_KEY_Pin) == GPIO_PIN_RESET);
}

uint32_t Encoder_GetTick(void) {
    return HAL_GetTick();
}

// Перехватываем прерывания от таймеров
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim) {
    // Проверяем, что прерывание пришло именно от нашего TIM2
    if (htim->Instance == TIM2) {
        Encoder_Scan(); // Опрашиваем энкодер
    }
}

/*
 HAL_TIM_Base_Start_IT(&htim2); // запуск прерывания для энкодера
 *while{
  EncoderEvent_t state = Encoder_GetEvent();
    if (state != ENCODER_NONE) {

      // Вместо медленного FillScreen лучше очищать только область текста,
      // но для простоты пока оставляем здесь, так как это больше не ломает энкодер.
      ST7735_FillScreen(ST7735_BLACK);

      if (state == ENCODER_LEFT) {
        ST7735_WriteString(10, 10, "ЛЕВО", Font_7x10_RU, ST7735_WHITE, ST7735_BLACK, 2);
      }
      else if (state == ENCODER_RIGHT) {
        ST7735_WriteString(10, 10, "ПРАВО", Font_7x10_RU, ST7735_WHITE, ST7735_BLACK, 2);
      }
      else if (state == ENCODER_BUTTON) {
        ST7735_WriteString(10, 10, "КНОПКА", Font_7x10_RU, ST7735_WHITE, ST7735_BLACK, 2);
      }
 * }
 */