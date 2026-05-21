#ifndef BOARD_H
#define BOARD_H

#include <stdbool.h>
#include <stdint.h>

/*
 * 板级初始化：
 * 1) 更新系统时钟变量；
 * 2) 初始化 LED 与按键 GPIO；
 * 3) 默认熄灭 DS0、DS1。
 */
void board_init(void);

/* 基于 SysTick 的毫秒延时（软件阻塞式）。 */
void board_delay_ms(uint32_t ms);

/*
 * 按键读取接口：
 * - KEY0、KEY1 为低电平按下（返回 true 表示按下）；
 * - WK_UP 为高电平按下（返回 true 表示按下）。
 */
bool board_key0_pressed(void);
bool board_key1_pressed(void);
bool board_wkup_pressed(void);

/* LED 控制接口（on=true 点亮，on=false 熄灭）。 */
void board_led0_set(bool on);
void board_led1_set(bool on);
void board_led_all_set(bool ds0_on, bool ds1_on);

/* LED 翻转接口（亮变灭，灭变亮）。 */
void board_led0_toggle(void);
void board_led1_toggle(void);
void board_led_all_toggle(void);

/* 外部中断初始化接口（分别对应 WK_UP/KEY0/KEY1）。 */
void board_key_exti_wkup_init(void);
void board_key_exti_key0_init(void);
void board_key_exti_key1_init(void);

#endif
