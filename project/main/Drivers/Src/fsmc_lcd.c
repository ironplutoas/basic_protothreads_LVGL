#include "fsmc_lcd.h"

#define LCD_WIDTH 240U
#define LCD_HEIGHT 240U

#define LCD_RST_PORT GPIOD
#define LCD_RST_PIN GPIO_PIN_3
#define LCD_BL_PORT GPIOF
#define LCD_BL_PIN GPIO_PIN_9

#define LCD_BASE_ADDR ((uint32_t)(0x68000000U | 0x0003FFFEU))

typedef struct
{
    __IO uint8_t REG8;
    __IO uint8_t RESERVED;
    __IO uint8_t RAM8;
    __IO uint16_t RAM16;
} lcd_bus_t;

static SRAM_HandleTypeDef s_hsram = {0};
static uint8_t s_lcd_ready = 0U;

#define LCD_BUS ((lcd_bus_t *)LCD_BASE_ADDR)

static void lcd_write_reg(uint8_t reg)
{
    LCD_BUS->REG8 = reg;
}

static void lcd_write_data8(uint8_t data)
{
    LCD_BUS->RAM8 = data;
}

static void lcd_write_pixel_rgb565(uint16_t rgb565)
{
    lcd_write_data8((uint8_t)(rgb565 >> 8));
    lcd_write_data8((uint8_t)(rgb565 & 0xFFU));
}

static void lcd_write_cmd_data8(uint8_t cmd, uint8_t data)
{
    lcd_write_reg(cmd);
    lcd_write_data8(data);
}

static void lcd_set_address_window(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2)
{
    lcd_write_reg(0x2A);
    lcd_write_data8((uint8_t)(x1 >> 8));
    lcd_write_data8((uint8_t)(x1 & 0xFFU));
    lcd_write_data8((uint8_t)(x2 >> 8));
    lcd_write_data8((uint8_t)(x2 & 0xFFU));

    lcd_write_reg(0x2B);
    lcd_write_data8((uint8_t)(y1 >> 8));
    lcd_write_data8((uint8_t)(y1 & 0xFFU));
    lcd_write_data8((uint8_t)(y2 >> 8));
    lcd_write_data8((uint8_t)(y2 & 0xFFU));

    lcd_write_reg(0x2C);
}

static void lcd_control_gpio_init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    __HAL_RCC_GPIOD_CLK_ENABLE();
    __HAL_RCC_GPIOF_CLK_ENABLE();

    HAL_GPIO_WritePin(LCD_RST_PORT, LCD_RST_PIN, GPIO_PIN_SET);
    HAL_GPIO_WritePin(LCD_BL_PORT, LCD_BL_PIN, GPIO_PIN_RESET);

    GPIO_InitStruct.Pin = LCD_RST_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(LCD_RST_PORT, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = LCD_BL_PIN;
    HAL_GPIO_Init(LCD_BL_PORT, &GPIO_InitStruct);
}

static HAL_StatusTypeDef lcd_bus_init(void)
{
    FSMC_NORSRAM_TimingTypeDef read_timing = {0};
    FSMC_NORSRAM_TimingTypeDef write_timing = {0};

    s_hsram.Instance = FSMC_NORSRAM_DEVICE;
    s_hsram.Extended = FSMC_NORSRAM_EXTENDED_DEVICE;
    s_hsram.Init.NSBank = FSMC_NORSRAM_BANK3;
    s_hsram.Init.DataAddressMux = FSMC_DATA_ADDRESS_MUX_DISABLE;
    s_hsram.Init.MemoryType = FSMC_MEMORY_TYPE_SRAM;
    s_hsram.Init.MemoryDataWidth = FSMC_NORSRAM_MEM_BUS_WIDTH_8;
    s_hsram.Init.BurstAccessMode = FSMC_BURST_ACCESS_MODE_DISABLE;
    s_hsram.Init.WaitSignalPolarity = FSMC_WAIT_SIGNAL_POLARITY_LOW;
    s_hsram.Init.WrapMode = FSMC_WRAP_MODE_DISABLE;
    s_hsram.Init.WaitSignalActive = FSMC_WAIT_TIMING_BEFORE_WS;
    s_hsram.Init.WriteOperation = FSMC_WRITE_OPERATION_ENABLE;
    s_hsram.Init.WaitSignal = FSMC_WAIT_SIGNAL_DISABLE;
    s_hsram.Init.ExtendedMode = FSMC_EXTENDED_MODE_ENABLE;
    s_hsram.Init.AsynchronousWait = FSMC_ASYNCHRONOUS_WAIT_DISABLE;
    s_hsram.Init.WriteBurst = FSMC_WRITE_BURST_DISABLE;
    s_hsram.Init.PageSize = FSMC_PAGE_SIZE_NONE;

    read_timing.AddressSetupTime = 0x0FU;
    read_timing.AddressHoldTime = 0x00U;
    read_timing.DataSetupTime = 60U;
    read_timing.BusTurnAroundDuration = 0x00U;
    read_timing.CLKDivision = 0x00U;
    read_timing.DataLatency = 0x00U;
    read_timing.AccessMode = FSMC_ACCESS_MODE_A;

    write_timing.AddressSetupTime = 9U;
    write_timing.AddressHoldTime = 0x00U;
    write_timing.DataSetupTime = 8U;
    write_timing.BusTurnAroundDuration = 0x00U;
    write_timing.CLKDivision = 0x00U;
    write_timing.DataLatency = 0x00U;
    write_timing.AccessMode = FSMC_ACCESS_MODE_A;

    return HAL_SRAM_Init(&s_hsram, &read_timing, &write_timing);
}

static void lcd_hard_reset(void)
{
    HAL_GPIO_WritePin(LCD_RST_PORT, LCD_RST_PIN, GPIO_PIN_RESET);
    HAL_Delay(100U);
    HAL_GPIO_WritePin(LCD_RST_PORT, LCD_RST_PIN, GPIO_PIN_SET);
    HAL_Delay(100U);
}

static void lcd_init_sequence(void)
{
    lcd_write_cmd_data8(0x36, 0x00);
    lcd_write_cmd_data8(0x3A, 0x65);

    lcd_write_reg(0xB2);
    lcd_write_data8(0x0C);
    lcd_write_data8(0x0C);
    lcd_write_data8(0x00);
    lcd_write_data8(0x33);
    lcd_write_data8(0x33);

    lcd_write_cmd_data8(0xB7, 0x35);
    lcd_write_cmd_data8(0xBB, 0x37);
    lcd_write_cmd_data8(0xC0, 0x2C);
    lcd_write_cmd_data8(0xC2, 0x01);
    lcd_write_cmd_data8(0xC3, 0x12);
    lcd_write_cmd_data8(0xC4, 0x20);
    lcd_write_cmd_data8(0xC6, 0x0F);

    lcd_write_reg(0xD0);
    lcd_write_data8(0xA4);
    lcd_write_data8(0xA1);

    lcd_write_reg(0xE0);
    lcd_write_data8(0xD0);
    lcd_write_data8(0x04);
    lcd_write_data8(0x0D);
    lcd_write_data8(0x11);
    lcd_write_data8(0x13);
    lcd_write_data8(0x2B);
    lcd_write_data8(0x3F);
    lcd_write_data8(0x54);
    lcd_write_data8(0x4C);
    lcd_write_data8(0x18);
    lcd_write_data8(0x0D);
    lcd_write_data8(0x0B);
    lcd_write_data8(0x1F);
    lcd_write_data8(0x23);

    lcd_write_reg(0xE1);
    lcd_write_data8(0xD0);
    lcd_write_data8(0x04);
    lcd_write_data8(0x0C);
    lcd_write_data8(0x11);
    lcd_write_data8(0x13);
    lcd_write_data8(0x2C);
    lcd_write_data8(0x3F);
    lcd_write_data8(0x44);
    lcd_write_data8(0x51);
    lcd_write_data8(0x2F);
    lcd_write_data8(0x1F);
    lcd_write_data8(0x1F);
    lcd_write_data8(0x20);
    lcd_write_data8(0x23);

    lcd_write_reg(0x21);
    lcd_write_reg(0x11);
    HAL_Delay(120U);
    lcd_write_reg(0x29);
}

void HAL_SRAM_MspInit(SRAM_HandleTypeDef *hsram)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    if (hsram->Instance != FSMC_NORSRAM_DEVICE)
    {
        return;
    }

    __HAL_RCC_FSMC_CLK_ENABLE();
    __HAL_RCC_GPIOD_CLK_ENABLE();
    __HAL_RCC_GPIOE_CLK_ENABLE();
    __HAL_RCC_GPIOG_CLK_ENABLE();

    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF12_FSMC;

    GPIO_InitStruct.Pin = GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_4 | GPIO_PIN_5 |
                          GPIO_PIN_13 | GPIO_PIN_14 | GPIO_PIN_15;
    HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = GPIO_PIN_7 | GPIO_PIN_8 | GPIO_PIN_9 | GPIO_PIN_10;
    HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = GPIO_PIN_10;
    HAL_GPIO_Init(GPIOG, &GPIO_InitStruct);
}

HAL_StatusTypeDef fsmc_lcd_init(void)
{
    if (s_lcd_ready != 0U)
    {
        return HAL_OK;
    }

    lcd_control_gpio_init();

    if (lcd_bus_init() != HAL_OK)
    {
        return HAL_ERROR;
    }

    lcd_hard_reset();
    lcd_init_sequence();
    s_lcd_ready = 1U;
    fsmc_lcd_fill_color(LCD_COLOR_WHITE);

    HAL_GPIO_WritePin(LCD_BL_PORT, LCD_BL_PIN, GPIO_PIN_SET);

    return HAL_OK;
}

void fsmc_lcd_fill_color(uint16_t rgb565)
{
    uint32_t i;
    uint32_t pixel_count = LCD_WIDTH * LCD_HEIGHT;

    if (s_lcd_ready == 0U)
    {
        return;
    }

    lcd_set_address_window(0U, 0U, (uint16_t)(LCD_WIDTH - 1U), (uint16_t)(LCD_HEIGHT - 1U));
    for (i = 0U; i < pixel_count; ++i)
    {
        lcd_write_pixel_rgb565(rgb565);
    }
}

void fsmc_lcd_write_area_rgb565(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, const uint16_t *pixels)
{
    uint32_t i;
    uint32_t pixel_count;
    uint16_t width;
    uint16_t height;

    if (s_lcd_ready == 0U || pixels == NULL)
    {
        return;
    }

    if (x1 > x2 || y1 > y2)
    {
        return;
    }

    if (x1 >= LCD_WIDTH || y1 >= LCD_HEIGHT)
    {
        return;
    }

    if (x2 >= LCD_WIDTH)
    {
        x2 = (uint16_t)(LCD_WIDTH - 1U);
    }

    if (y2 >= LCD_HEIGHT)
    {
        y2 = (uint16_t)(LCD_HEIGHT - 1U);
    }

    width = (uint16_t)(x2 - x1 + 1U);
    height = (uint16_t)(y2 - y1 + 1U);
    pixel_count = (uint32_t)width * (uint32_t)height;

    lcd_set_address_window(x1, y1, x2, y2);
    for (i = 0U; i < pixel_count; ++i)
    {
        lcd_write_pixel_rgb565(pixels[i]);
    }
}
