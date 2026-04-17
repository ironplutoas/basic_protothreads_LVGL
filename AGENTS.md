# AGENTS.md

This file provides guidance to Codex (Codex.ai/code) when working with code in this repository.

## Project Overview

STM32F407ZGT6 embedded development workspace using VSCode + CMake + ARM GCC + Ninja. Target hardware is "星火一号" development board with ST7789V3 LCD (240x240, FSMC interface) and on-board ST-Link V2.1 debugger.

## Build Commands

Each project in `projects/` is a standalone CMake project. Configure and build from project directory:

```bash
cd projects/01_blink_led
cmake -G Ninja -DCMAKE_TOOLCHAIN_FILE=../../tools/cmake-toolchain.cmake .
ninja
```

Or use CMake presets from root:
```bash
cmake --preset default
cmake --build build/default
```

VSCode: `Ctrl+Shift+B` → "Build Project" (requires CMake Tools extension and selecting a project via `cmake.sourceDirectory` in settings.json)

## Flash Commands

OpenOCD via ST-Link:
```bash
openocd -f tools/openocd-stlink.cfg -c "program <project_name>.elf verify reset exit"
```

VSCode: `Ctrl+Shift+P` → "Tasks: Run Task" → "Flash to STM32"

## Debug Commands

VSCode Cortex-Debug extension required. Configuration in `.vscode/launch.json`:
- "STM32 Debug (ST-Link)" - launch debugging
- "STM32 Attach (ST-Link)" - attach to running target

SVD file: `resources/STM32F407.svd` for peripheral register visualization.

## Project Architecture

```
projects/
  01_blink_led/      # Minimal system verification (GPIO only)
  02_uart_printf/    # UART debug output
  03_fsmc_lcd/       # FSMC LCD driver (ST7789)
  04_lvgl_hello/     # LVGL HelloWorld
  05_boot_animation/ # Final demo: LVGL logo fade animation

resources/
  hal/               # STM32F4 HAL library + CMSIS headers
    Include/         # CMSIS core_cm4.h, stm32f407xx.h
    STM32F4xx_HAL_Driver/
    Src/             # system_stm32f4xx.c
    startup_stm32f407xx.s  # Modified startup (calls main directly)
  lvgl/              # LVGL 8.x source
  protothreads/      # Protothreads coroutine library
```

Each project has:
- `CMakeLists.txt` - standalone build config (must set CMAKE_TOOLCHAIN_FILE before project())
- `STM32F407ZGTx_FLASH.ld` - linker script (1MB Flash @ 0x08000000, 128KB RAM @ 0x20000000)
- `Core/Inc/` - stm32f4xx_hal_conf.h (HAL module enables, clock defines), main.h
- `Core/Src/` - main.c, stm32f4xx_it.c, stm32f4xx_hal_msp.c, project-specific drivers

## Key Technical Details

**MCU Configuration:**
- Cortex-M4F with FPU: `-mcpu=cortex-m4 -mfpu=fpv4-sp-d16 -mfloat-abi=hard`
- External crystal: 8MHz (HSE_VALUE=8000000U)
- System clock: 168MHz (configured in SystemInit)

**HAL Configuration (stm32f4xx_hal_conf.h):**
Must define:
- `HSI_VALUE 16000000U` - internal clock
- `HSE_VALUE 8000000U` - external crystal
- `assert_param()` macro - HAL parameter validation
- HAL module enables: `HAL_GPIO_MODULE_ENABLED`, `HAL_RCC_MODULE_ENABLED`, etc.

**Startup File Modification:**
The startup file at `resources/hal/startup_stm32f407xx.s` is modified:
- Removed `__libc_init_array` call (no standard C library init)
- Changed `bl entry` to `bl main` (direct jump to main)
- Required because of `-nostartfiles` linker option

**LVGL Configuration (lv_conf.h):**
- `LV_COLOR_DEPTH 16` - RGB565
- `LV_HOR_RES_MAX 240`, `LV_VER_RES_MAX 240` - screen size
- `LV_MEM_SIZE (32 * 1024)` - heap size

**Hardware Notes:**
- PF9 controls LCD backlight (NOT a LED - true LED is SK6805 RGB array requiring special driver)
- FSMC Bank1 SRAM4 used for LCD interface (address 0x6C000000)
- ST-Link V2.1 on-board for debug/flash

## Common Issues

1. **HSI_VALUE undeclared**: Add `#define HSI_VALUE 16000000U` to stm32f4xx_hal_conf.h
2. **assert_param implicit declaration**: Define assert_param macro in hal_conf.h
3. **undefined reference to 'entry'**: Use modified startup file (bl main instead of bl entry)
4. **LV_CONF_PATH issues**: LVGL auto-finds lv_conf.h in include paths; don't use quotes in -D define
5. **FSMC macro undeclared**: STM32F4 HAL uses different enum names than F7; check ll_fsmc.h

## Environment Requirements

- ARM GCC Toolchain 10.3+ (`arm-none-eabi-gcc`)
- Ninja build tool
- OpenOCD (xPack distribution recommended)
- VSCode extensions: C/C++, CMake Tools, Cortex-Debug