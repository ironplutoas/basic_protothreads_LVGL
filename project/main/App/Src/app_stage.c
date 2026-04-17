#include "app_stage.h"

#include "main.h"
#include "board_uart.h"

static uint32_t s_last_tick = 0U;
static uint8_t s_stage_banner_sent = 0U;

static void stage_send_banner(void)
{
#if APP_STAGE >= 2
#if APP_STAGE == 2
    board_uart_write("\r\n[stage2] uart printf is ready.\r\n");
#elif APP_STAGE == 3
    board_uart_write("\r\n[stage3] fsmc lcd stage placeholder.\r\n");
#elif APP_STAGE == 4
    board_uart_write("\r\n[stage4] lvgl hello stage placeholder.\r\n");
#else
    board_uart_write("\r\n[stage5] boot animation + protothreads placeholder.\r\n");
#endif
#endif
}

void app_stage_init(void)
{
#if APP_STAGE >= 2
    (void)board_uart_init();
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
#else
    if (s_stage_banner_sent == 0U)
    {
        stage_send_banner();
        s_stage_banner_sent = 1U;
    }
    HAL_Delay(10);
#endif
}
