#ifndef __BOARD_UART_H
#define __BOARD_UART_H

#include "stm32f4xx_hal.h"

HAL_StatusTypeDef board_uart_init(void);
HAL_StatusTypeDef board_uart_write(const char *text);

#endif /* __BOARD_UART_H */
