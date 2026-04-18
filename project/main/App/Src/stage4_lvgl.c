#include "stage4_lvgl.h"

#include "board_uart.h"
#include "fsmc_lcd.h"
#include "stm32f4xx_hal.h"

#if APP_STAGE == 4
#include "lvgl.h"

#define STAGE4_HOR_RES 240U
#define STAGE4_VER_RES 240U
#define STAGE4_DRAW_LINES 20U
#define STAGE4_HANDLER_PERIOD_MS 5U

static lv_disp_draw_buf_t s_draw_buf;
static lv_color_t s_draw_buf_1[STAGE4_HOR_RES * STAGE4_DRAW_LINES];
static lv_disp_drv_t s_disp_drv;
static uint8_t s_stage4_ready = 0U;
static uint32_t s_last_tick_ms = 0U;
static uint32_t s_last_handler_ms = 0U;

static void stage4_lvgl_flush_cb(lv_disp_drv_t *drv, const lv_area_t *area, lv_color_t *color_p)
{
    (void)drv;

    fsmc_lcd_write_area_rgb565((uint16_t)area->x1,
                               (uint16_t)area->y1,
                               (uint16_t)area->x2,
                               (uint16_t)area->y2,
                               (const uint16_t *)color_p);
    lv_disp_flush_ready(drv);
}

static void stage4_create_hello_ui(void)
{
    lv_obj_t *label = lv_label_create(lv_scr_act());
    lv_label_set_text(label, "Hello LVGL");
    lv_obj_center(label);
}

void stage4_lvgl_init(void)
{
    if (s_stage4_ready != 0U)
    {
        return;
    }

    lv_init();

    lv_disp_draw_buf_init(&s_draw_buf,
                          s_draw_buf_1,
                          NULL,
                          STAGE4_HOR_RES * STAGE4_DRAW_LINES);

    lv_disp_drv_init(&s_disp_drv);
    s_disp_drv.hor_res = STAGE4_HOR_RES;
    s_disp_drv.ver_res = STAGE4_VER_RES;
    s_disp_drv.flush_cb = stage4_lvgl_flush_cb;
    s_disp_drv.draw_buf = &s_draw_buf;
    (void)lv_disp_drv_register(&s_disp_drv);

    stage4_create_hello_ui();

    s_last_tick_ms = HAL_GetTick();
    s_last_handler_ms = s_last_tick_ms;
    s_stage4_ready = 1U;
    board_uart_write("[stage4] lvgl init ok\r\n");
}

void stage4_lvgl_loop(void)
{
    uint32_t now;
    uint32_t elapsed;

    if (s_stage4_ready == 0U)
    {
        return;
    }

    now = HAL_GetTick();
    elapsed = now - s_last_tick_ms;
    if (elapsed > 0U)
    {
        lv_tick_inc(elapsed);
        s_last_tick_ms = now;
    }

    if ((now - s_last_handler_ms) >= STAGE4_HANDLER_PERIOD_MS)
    {
        (void)lv_timer_handler();
        s_last_handler_ms = now;
    }
}

#else

void stage4_lvgl_init(void)
{
}

void stage4_lvgl_loop(void)
{
}

#endif /* APP_STAGE == 4 */
