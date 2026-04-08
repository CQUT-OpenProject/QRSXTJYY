#ifndef MORSE_APP_H
#define MORSE_APP_H

#include <stdbool.h>
#include <stdint.h>

void board_init(void);
void board_led_set(bool on);
bool board_key_up_pressed(void);
bool board_key1_pressed(void);
bool board_key0_pressed(void);
void board_delay_ms(uint32_t ms);

void morse_app_run(void);

#endif
