#ifndef BOARD_H
#define BOARD_H

#include <stdbool.h>
#include <stdint.h>

/* 初始化按键 GPIO 和 USART1。 */
void board_init(void);

/* 基于 SysTick 的阻塞式毫秒延时。 */
void board_delay_ms(uint32_t ms);

/* 读取 3 个按键当前状态。 */
bool board_key0_pressed(void);
bool board_key1_pressed(void);
bool board_wkup_pressed(void);

/* USART1 发送和接收接口。 */
void board_usart1_write_byte(uint8_t data);
void board_usart1_write_str(const char *text);
bool board_usart1_read_byte(uint8_t *data);

#endif
