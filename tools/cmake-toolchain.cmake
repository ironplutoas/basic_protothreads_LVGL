# tools/cmake-toolchain.cmake
# ARM Cortex-M GCC工具链配置
# 用于STM32F407开发

set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_PROCESSOR ARM)

# 跳过编译器测试（交叉编译时测试程序不能在主机运行）
set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)

# 指定交叉编译工具前缀
# 需确保arm-none-eabi-gcc在系统PATH中，或修改为完整路径
set(TOOLCHAIN_PREFIX "arm-none-eabi-")
set(CMAKE_C_COMPILER ${TOOLCHAIN_PREFIX}gcc)
set(CMAKE_CXX_COMPILER ${TOOLCHAIN_PREFIX}g++)
set(CMAKE_ASM_COMPILER ${TOOLCHAIN_PREFIX}gcc)
set(CMAKE_AR ${TOOLCHAIN_PREFIX}ar)
set(CMAKE_OBJCOPY ${TOOLCHAIN_PREFIX}objcopy)
set(CMAKE_OBJDUMP ${TOOLCHAIN_PREFIX}objdump)
set(CMAKE_SIZE ${TOOLCHAIN_PREFIX}size)

# 不搜索主机系统库
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)

# MCU特定编译选项 - STM32F407 Cortex-M4F
set(MCU_FLAGS "-mcpu=cortex-m4 -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb")

# C语言编译选项
set(CMAKE_C_FLAGS "${MCU_FLAGS} -Wall -Wextra -std=gnu11 -ffunction-sections -fdata-sections -ffreestanding -fno-builtin")

# C++编译选项
set(CMAKE_CXX_FLAGS "${MCU_FLAGS} -Wall -Wextra -std=gnu++14 -ffunction-sections -fdata-sections -ffreestanding -fno-builtin")

# 汇编编译选项
set(CMAKE_ASM_FLAGS "${MCU_FLAGS} -x assembler-with-cpp")

# Debug配置
set(CMAKE_C_FLAGS_DEBUG "-O0 -g3 -DDEBUG")
set(CMAKE_CXX_FLAGS_DEBUG "-O0 -g3 -DDEBUG")

# Release配置
set(CMAKE_C_FLAGS_RELEASE "-Os -g0 -DNDEBUG")
set(CMAKE_CXX_FLAGS_RELEASE "-Os -g0 -DNDEBUG")

# 链接选项
set(CMAKE_EXE_LINKER_FLAGS "-Wl,--gc-sections -Wl,--print-memory-usage")