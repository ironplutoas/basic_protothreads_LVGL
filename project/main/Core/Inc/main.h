#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32f4xx_hal.h"

#define STAGE1_GPIO_PORT GPIOF
#define STAGE1_GPIO_PIN  GPIO_PIN_9

void Error_Handler(void);

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
