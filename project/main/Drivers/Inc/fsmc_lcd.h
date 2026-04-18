#ifndef __FSMC_LCD_H
#define __FSMC_LCD_H

#include "stm32f4xx_hal.h"

#define LCD_COLOR_WHITE  0xFFFFU
#define LCD_COLOR_BLACK  0x0000U
#define LCD_COLOR_RED    0xF800U
#define LCD_COLOR_GREEN  0x07E0U
#define LCD_COLOR_BLUE   0x001FU
#define LCD_COLOR_YELLOW 0xFFE0U

HAL_StatusTypeDef fsmc_lcd_init(void);
void fsmc_lcd_fill_color(uint16_t rgb565);

#endif /* __FSMC_LCD_H */
