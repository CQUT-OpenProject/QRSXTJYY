cmake_minimum_required(VERSION 3.22)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS ON)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug)
endif()

project(Q4 C ASM)
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

file(GLOB_RECURSE FW_SOURCES CONFIGURE_DEPENDS "drivers/STM32F10x_StdPeriph_Driver/src/*.c")
file(GLOB_RECURSE CMSIS_SOURCES CONFIGURE_DEPENDS "drivers/cmsis/*.c")

set(STARTUP_SCRIPT "${CMAKE_CURRENT_SOURCE_DIR}/startup/startup_stm32f103xe.s")

set(APP_COMMON_SOURCES
        core/src/main.c
        core/src/board.c
        core/src/stm32f10x_it.c
        core/src/system_stm32f10x.c
)

add_executable(${PROJECT_NAME}
        ${APP_COMMON_SOURCES}
        ${FW_SOURCES}
        ${CMSIS_SOURCES}
        ${STARTUP_SCRIPT}
)

target_include_directories(${PROJECT_NAME} PRIVATE
        core/inc
        drivers/cmsis
        drivers/STM32F10x_StdPeriph_Driver/inc
)

target_compile_definitions(${PROJECT_NAME} PRIVATE
        USE_STDPERIPH_DRIVER
        STM32F10X_HD
)

target_compile_options(${PROJECT_NAME} PRIVATE
        -ffunction-sections
        -fdata-sections
)

target_link_options(${PROJECT_NAME} PRIVATE
        -specs=nosys.specs
        -Wl,-Map=${PROJECT_NAME}.map
        -Wl,--gc-sections
)

list(REMOVE_ITEM CMAKE_C_IMPLICIT_LINK_LIBRARIES ob)
