#include "stage5_boot_pt.h"

#include "app_stage.h"
#include "board_uart.h"
#include "fsmc_lcd.h"
#include "stm32f4xx_hal.h"

#if APP_STAGE == 5

#include "lvgl.h"
#include "pt.h"

#define STAGE5_HOR_RES 240U
#define STAGE5_VER_RES 240U
#define STAGE5_DRAW_LINES 20U
#define STAGE5_HANDLER_PERIOD_MS 5U

#define STAGE5_FADE_STEP 5U
#define STAGE5_FADE_STEP_MS 20U
#define STAGE5_FADE_HOLD_MS 500U
#define STAGE5_RESTART_GAP_MS 200U

static lv_disp_draw_buf_t s_draw_buf;
static lv_color_t s_draw_buf_1[STAGE5_HOR_RES * STAGE5_DRAW_LINES];
static lv_disp_drv_t s_disp_drv;

static struct pt s_anim_pt;

static lv_obj_t *s_title_label = NULL;
static uint8_t s_title_opa = 0U;
static uint8_t s_stage5_ready = 0U;
static uint32_t s_last_tick_ms = 0U;
static uint32_t s_last_handler_ms = 0U;
static uint32_t s_wait_start_ms = 0U;
static uint32_t s_wait_duration_ms = 0U;

static void stage5_lvgl_flush_cb(lv_disp_drv_t *drv, const lv_area_t *area, lv_color_t *color_p)
{
    (void)drv;

    fsmc_lcd_write_area_rgb565((uint16_t)area->x1,
                               (uint16_t)area->y1,
                               (uint16_t)area->x2,
                               (uint16_t)area->y2,
                               (const uint16_t *)color_p);
    lv_disp_flush_ready(drv);
}

static void stage5_animation_wait_start(uint32_t duration_ms)
{
    s_wait_start_ms = HAL_GetTick();
    s_wait_duration_ms = duration_ms;
}

static uint8_t stage5_animation_wait_expired(void)
{
    return (uint8_t)(((HAL_GetTick() - s_wait_start_ms) >= s_wait_duration_ms) ? 1U : 0U);
}

static void stage5_apply_title_opa(uint8_t opa)
{
    if (s_title_label != NULL)
    {
        lv_obj_set_style_text_opa(s_title_label, (lv_opa_t)opa, LV_PART_MAIN);
    }
}

static void stage5_create_ui(void)
{
    lv_obj_t *screen = lv_scr_act();
    lv_obj_t *hint_label;

    lv_obj_set_style_bg_color(screen, lv_color_black(), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(screen, LV_OPA_COVER, LV_PART_MAIN);

    s_title_label = lv_label_create(screen);
    lv_label_set_text(s_title_label, "LVGL Boot");
    lv_obj_set_style_text_color(s_title_label, lv_color_white(), LV_PART_MAIN);
    lv_obj_set_style_text_opa(s_title_label, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_center(s_title_label);

    hint_label = lv_label_create(screen);
    lv_label_set_text(hint_label, "Protothreads");
    lv_obj_set_style_text_color(hint_label, lv_color_white(), LV_PART_MAIN);
    lv_obj_set_style_text_opa(hint_label, (lv_opa_t)160U, LV_PART_MAIN);
    lv_obj_align(hint_label, LV_ALIGN_CENTER, 0, 28);
}

static PT_THREAD(stage5_boot_anim_thread(struct pt *pt))
{
    PT_BEGIN(pt);

    while (1)
    {
        s_title_opa = 0U;
        stage5_apply_title_opa(s_title_opa);

        while (s_title_opa < 255U)
        {
            stage5_animation_wait_start(STAGE5_FADE_STEP_MS);
            PT_WAIT_UNTIL(pt, stage5_animation_wait_expired() != 0U);

            if (s_title_opa >= (uint8_t)(255U - STAGE5_FADE_STEP))
            {
                s_title_opa = 255U;
            }
            else
            {
                s_title_opa = (uint8_t)(s_title_opa + STAGE5_FADE_STEP);
            }

            stage5_apply_title_opa(s_title_opa);
        }

        stage5_animation_wait_start(STAGE5_FADE_HOLD_MS);
        PT_WAIT_UNTIL(pt, stage5_animation_wait_expired() != 0U);

        while (s_title_opa > 0U)
        {
            stage5_animation_wait_start(STAGE5_FADE_STEP_MS);
            PT_WAIT_UNTIL(pt, stage5_animation_wait_expired() != 0U);

            if (s_title_opa <= STAGE5_FADE_STEP)
            {
                s_title_opa = 0U;
            }
            else
            {
                s_title_opa = (uint8_t)(s_title_opa - STAGE5_FADE_STEP);
            }

            stage5_apply_title_opa(s_title_opa);
        }

        stage5_animation_wait_start(STAGE5_RESTART_GAP_MS);
        PT_WAIT_UNTIL(pt, stage5_animation_wait_expired() != 0U);
    }

    PT_END(pt);
}

void stage5_boot_pt_init(void)
{
    if (s_stage5_ready != 0U)
    {
        return;
    }

    lv_init();

    lv_disp_draw_buf_init(&s_draw_buf,
                          s_draw_buf_1,
                          NULL,
                          STAGE5_HOR_RES * STAGE5_DRAW_LINES);

    lv_disp_drv_init(&s_disp_drv);
    s_disp_drv.hor_res = STAGE5_HOR_RES;
    s_disp_drv.ver_res = STAGE5_VER_RES;
    s_disp_drv.flush_cb = stage5_lvgl_flush_cb;
    s_disp_drv.draw_buf = &s_draw_buf;
    (void)lv_disp_drv_register(&s_disp_drv);

    stage5_create_ui();
    PT_INIT(&s_anim_pt);

    s_last_tick_ms = HAL_GetTick();
    s_last_handler_ms = s_last_tick_ms;
    s_stage5_ready = 1U;

    board_uart_write("[stage5] lvgl + protothreads init ok\r\n");
}

void stage5_boot_pt_loop(void)
{
    uint32_t now;
    uint32_t elapsed;

    if (s_stage5_ready == 0U)
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

    (void)PT_SCHEDULE(stage5_boot_anim_thread(&s_anim_pt));

    if ((now - s_last_handler_ms) >= STAGE5_HANDLER_PERIOD_MS)
    {
        (void)lv_timer_handler();
        s_last_handler_ms = now;
    }
}

#else

void stage5_boot_pt_init(void)
{
}

void stage5_boot_pt_loop(void)
{
}

#endif /* APP_STAGE == 5 */
