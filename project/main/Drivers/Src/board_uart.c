#include "board_uart.h"

#include <string.h>

static UART_HandleTypeDef s_huart1;
static uint8_t s_uart_ready = 0U;

HAL_StatusTypeDef board_uart_init(void)
{
    s_huart1.Instance = USART1;
    s_huart1.Init.BaudRate = 115200;
    s_huart1.Init.WordLength = UART_WORDLENGTH_8B;
    s_huart1.Init.StopBits = UART_STOPBITS_1;
    s_huart1.Init.Parity = UART_PARITY_NONE;
    s_huart1.Init.Mode = UART_MODE_TX_RX;
    s_huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
    s_huart1.Init.OverSampling = UART_OVERSAMPLING_16;

    if (HAL_UART_Init(&s_huart1) != HAL_OK)
    {
        s_uart_ready = 0U;
        return HAL_ERROR;
    }

    s_uart_ready = 1U;
    return HAL_OK;
}

HAL_StatusTypeDef board_uart_write(const char *text)
{
    size_t len;

    if (s_uart_ready == 0U || text == NULL)
    {
        return HAL_ERROR;
    }

    len = strlen(text);
    if (len == 0U)
    {
        return HAL_OK;
    }

    return HAL_UART_Transmit(&s_huart1, (uint8_t *)text, (uint16_t)len, HAL_MAX_DELAY);
}

void HAL_UART_MspInit(UART_HandleTypeDef *huart)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    if (huart->Instance != USART1)
    {
        return;
    }

    __HAL_RCC_USART1_CLK_ENABLE();
    __HAL_RCC_GPIOA_CLK_ENABLE();

    GPIO_InitStruct.Pin = GPIO_PIN_9 | GPIO_PIN_10;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF7_USART1;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
}

void HAL_UART_MspDeInit(UART_HandleTypeDef *huart)
{
    if (huart->Instance != USART1)
    {
        return;
    }

    __HAL_RCC_USART1_CLK_DISABLE();
    HAL_GPIO_DeInit(GPIOA, GPIO_PIN_9 | GPIO_PIN_10);
}
