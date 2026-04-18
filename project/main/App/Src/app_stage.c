#include "app_stage.h"

#include "main.h"
#include "board_uart.h"
#include "fsmc_lcd.h"
#include "stage4_lvgl.h"
#include "stage5_boot_pt.h"

static uint32_t s_last_tick = 0U;
static uint8_t s_stage_banner_sent = 0U;
#if APP_STAGE == 3
static uint8_t s_stage3_color_idx = 0U;
static const uint16_t k_stage3_colors[] = {
    LCD_COLOR_RED,
    LCD_COLOR_GREEN,
    LCD_COLOR_BLUE,
    LCD_COLOR_WHITE
};
#endif

static void stage_send_banner(void)
{
#if APP_STAGE >= 2
#if APP_STAGE == 2
    board_uart_write("\r\n[stage2] uart printf is ready.\r\n");
#elif APP_STAGE == 3
    board_uart_write("\r\n[stage3] fsmc lcd bring-up running.\r\n");
#elif APP_STAGE == 4
    board_uart_write("\r\n[stage4] lvgl hello running.\r\n");
#elif APP_STAGE == 5
    board_uart_write("\r\n[stage5] boot animation + protothreads running.\r\n");
#else
    board_uart_write("\r\n[stageX] unsupported stage value.\r\n");
#endif
#endif
}

void app_stage_init(void)
{
#if APP_STAGE >= 2
    (void)board_uart_init();
#endif

#if APP_STAGE == 3
    if (fsmc_lcd_init() == HAL_OK)
    {
        board_uart_write("[stage3] lcd init ok\r\n");
        fsmc_lcd_fill_color(k_stage3_colors[s_stage3_color_idx]);
    }
    else
    {
        board_uart_write("[stage3] lcd init failed\r\n");
    }
#elif APP_STAGE == 4
    if (fsmc_lcd_init() == HAL_OK)
    {
        stage4_lvgl_init();
    }
    else
    {
        board_uart_write("[stage4] lcd init failed\r\n");
    }
#elif APP_STAGE == 5
    if (fsmc_lcd_init() == HAL_OK)
    {
        stage5_boot_pt_init();
    }
    else
    {
        board_uart_write("[stage5] lcd init failed\r\n");
    }
#endif

    stage_send_banner();
    s_stage_banner_sent = 1U;
    s_last_tick = HAL_GetTick();
}

void app_stage_loop(void)
{
#if APP_STAGE == 1
    uint32_t now = HAL_GetTick();
    if ((now - s_last_tick) >= 500U)
    {
        HAL_GPIO_TogglePin(STAGE1_GPIO_PORT, STAGE1_GPIO_PIN);
        s_last_tick = now;
    }
#elif APP_STAGE == 2
    uint32_t now = HAL_GetTick();
    if ((now - s_last_tick) >= 1000U)
    {
        HAL_GPIO_TogglePin(STAGE1_GPIO_PORT, STAGE1_GPIO_PIN);
        board_uart_write("[stage2] heartbeat\r\n");
        s_last_tick = now;
    }
#elif APP_STAGE == 3
    uint32_t now = HAL_GetTick();
    if ((now - s_last_tick) >= 500U)
    {
        s_stage3_color_idx = (uint8_t)((s_stage3_color_idx + 1U) % 4U);
        fsmc_lcd_fill_color(k_stage3_colors[s_stage3_color_idx]);
        board_uart_write("[stage3] color cycle\r\n");
        s_last_tick = now;
    }
#elif APP_STAGE == 4
    stage4_lvgl_loop();
#elif APP_STAGE == 5
    stage5_boot_pt_loop();
#else
    if (s_stage_banner_sent == 0U)
    {
        stage_send_banner();
        s_stage_banner_sent = 1U;
    }
    HAL_Delay(10);
#endif
}
