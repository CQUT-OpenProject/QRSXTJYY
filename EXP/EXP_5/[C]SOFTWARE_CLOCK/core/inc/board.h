#ifndef BOARD_H
#define BOARD_H

#include <stdbool.h>
#include <stdint.h>

/* 初始化 USART1 和 TIM2。 */
void board_init(void);

/* TIM2 每 1ms 置位一次，读到后清除标志。 */
bool board_tim2_1ms_arrived(void);

/* USART1 发送和接收接口。 */
void board_usart1_write_byte(uint8_t data);
void board_usart1_write_str(const char *text);
bool board_usart1_read_byte(uint8_t *data);

#endif
