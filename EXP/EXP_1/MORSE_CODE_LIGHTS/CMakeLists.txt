cmake_minimum_required(VERSION 3.22)
project(MORSE_CODE_LIGHTS LANGUAGES C ASM)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(PLATFORM_DIR "${CMAKE_CURRENT_SOURCE_DIR}/stm32f1-spl-platform")
add_subdirectory("${PLATFORM_DIR}" "${CMAKE_CURRENT_BINARY_DIR}/platform")

add_executable(${PROJECT_NAME}
    core/src/main.c
    core/src/board.c
    core/src/morse_app.c
    core/src/stm32f10x_it.c
)
target_include_directories(${PROJECT_NAME} BEFORE PRIVATE core/inc)
stm32f1_configure_target(${PROJECT_NAME}
    MCU STM32F103RCT6
    SYSTEM_SOURCE core/src/system_stm32f10x.c
)
