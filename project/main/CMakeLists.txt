cmake_minimum_required(VERSION 3.20)

if(NOT CMAKE_TOOLCHAIN_FILE)
    message(FATAL_ERROR "Please set CMAKE_TOOLCHAIN_FILE before project()")
endif()

project(main C ASM)

set(TARGET_NAME stm32_lvgl_learning)
set(PROJECT_DIR ${CMAKE_CURRENT_SOURCE_DIR})
set(ROOT_DIR ${PROJECT_DIR}/../..)
set(HAL_DIR ${ROOT_DIR}/resources/hal)
set(LINKER_SCRIPT ${PROJECT_DIR}/STM32F407ZGTx_FLASH.ld)

if(NOT DEFINED APP_STAGE)
    set(APP_STAGE 1)
endif()

set(APP_SOURCES
    ${PROJECT_DIR}/Core/Src/main.c
    ${PROJECT_DIR}/Core/Src/stm32f4xx_it.c
    ${PROJECT_DIR}/Core/Src/stm32f4xx_hal_msp.c
    ${PROJECT_DIR}/Drivers/Src/board_uart.c
    ${PROJECT_DIR}/App/Src/app_stage.c
)

set(HAL_SOURCES
    ${HAL_DIR}/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal.c
    ${HAL_DIR}/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_cortex.c
    ${HAL_DIR}/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_dma.c
    ${HAL_DIR}/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_dma_ex.c
    ${HAL_DIR}/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_flash.c
    ${HAL_DIR}/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_flash_ex.c
    ${HAL_DIR}/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_flash_ramfunc.c
    ${HAL_DIR}/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_gpio.c
    ${HAL_DIR}/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_pwr.c
    ${HAL_DIR}/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_pwr_ex.c
    ${HAL_DIR}/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_rcc.c
    ${HAL_DIR}/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_rcc_ex.c
    ${HAL_DIR}/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_uart.c
)

set(CMSIS_SOURCES
    ${HAL_DIR}/Src/system_stm32f4xx.c
    ${HAL_DIR}/startup_stm32f407xx.s
)

add_executable(${TARGET_NAME}
    ${APP_SOURCES}
    ${HAL_SOURCES}
    ${CMSIS_SOURCES}
)

target_include_directories(${TARGET_NAME} PRIVATE
    ${PROJECT_DIR}/Core/Inc
    ${PROJECT_DIR}/Drivers/Inc
    ${PROJECT_DIR}/App/Inc
    ${HAL_DIR}/Include
    ${HAL_DIR}/STM32F4xx_HAL_Driver/Inc
    ${HAL_DIR}/STM32F4xx_HAL_Driver/Inc/Legacy
)

target_compile_definitions(${TARGET_NAME} PRIVATE
    STM32F407xx
    USE_HAL_DRIVER
    APP_STAGE=${APP_STAGE}
)

target_link_options(${TARGET_NAME} PRIVATE
    -T${LINKER_SCRIPT}
    -Wl,-Map=${TARGET_NAME}.map
    -Wl,--gc-sections
    -Wl,--print-memory-usage
    --specs=nano.specs
    --specs=nosys.specs
    -nostartfiles
)

add_custom_command(TARGET ${TARGET_NAME} POST_BUILD
    COMMAND ${CMAKE_OBJCOPY} -O ihex $<TARGET_FILE:${TARGET_NAME}> ${TARGET_NAME}.hex
    COMMAND ${CMAKE_OBJCOPY} -O binary $<TARGET_FILE:${TARGET_NAME}> ${TARGET_NAME}.bin
    COMMAND ${CMAKE_SIZE} $<TARGET_FILE:${TARGET_NAME}>
)
