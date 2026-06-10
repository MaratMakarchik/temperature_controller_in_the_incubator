//
// Created by Makar Makarov on 30.05.2026.
//

#include "main.h"
#include "st7735.h"
#include "stm32f1xx_hal.h" // Ваша версия HAL

extern SPI_HandleTypeDef hspi1; // Ваш настроенный SPI

void ST7735_SPI_Send(uint8_t *data, uint16_t len) {
    HAL_SPI_Transmit(&hspi1, data, len, HAL_MAX_DELAY);
}

void ST7735_SetCS(uint8_t state) {
    HAL_GPIO_WritePin(TFT_CS_GPIO_Port, TFT_CS_Pin, state ? GPIO_PIN_SET : GPIO_PIN_RESET);
}

void ST7735_SetDC(uint8_t state) {
    HAL_GPIO_WritePin(TFT_D_C_GPIO_Port, TFT_D_C_Pin, state ? GPIO_PIN_SET : GPIO_PIN_RESET);
}

void ST7735_SetRST(uint8_t state) {
    HAL_GPIO_WritePin(TFT_RESET_GPIO_Port, TFT_RESET_Pin, state ? GPIO_PIN_SET : GPIO_PIN_RESET);
}

void ST7735_DelayMs(uint32_t ms) {
    HAL_Delay(ms);
}