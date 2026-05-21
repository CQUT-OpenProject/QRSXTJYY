#include "board.h"

int main(void)
{
    board_init();

    while (1) {
        /*
         * Q2:
         * 1) WK_UP 按下：DS0 亮，DS1 灭；
         * 2) WK_UP 未按：DS0 灭，DS1 跟随 KEY1（按下亮，松开灭）。
         */
        if (board_wkup_pressed()) {
            board_led_all_set(true, false);
        } else {
            board_led0_set(false);
            board_led1_set(board_key1_pressed());
        }

        board_delay_ms(5U);
    }
}
