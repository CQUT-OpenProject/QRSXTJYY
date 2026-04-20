#include "board.h"

int main(void)
{
    board_init();

    while (1) {
        /* Q1: KEY0 按下时 DS0 亮，松开时 DS0 灭。 */
        if (board_key0_pressed()) {
            board_led0_set(true);
        } else {
            board_led0_set(false);
        }

        /* 本题不使用 DS1，保持熄灭。 */
        board_led1_set(false);

        /* 适当延时，降低轮询频率。 */
        board_delay_ms(5U);
    }
}
