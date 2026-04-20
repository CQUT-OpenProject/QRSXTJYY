#include "board.h"

int main(void)
{
    /* 记录“上一拍”按键状态，用于检测“新按下一次”。 */
    bool key0_last = false;
    bool key1_last = false;
    bool wkup_last = false;

    board_init();

    while (1) {
        bool key0_now = board_key0_pressed();
        bool key1_now = board_key1_pressed();
        bool wkup_now = board_wkup_pressed();

        /* 只在“从未按下 -> 按下”瞬间翻转一次 DS0。 */
        if (key0_now && !key0_last) {
            board_led0_toggle();
        }

        /* 只在“从未按下 -> 按下”瞬间翻转一次 DS1。 */
        if (key1_now && !key1_last) {
            board_led1_toggle();
        }

        /* 只在“从未按下 -> 按下”瞬间翻转 DS0、DS1。 */
        if (wkup_now && !wkup_last) {
            board_led_all_toggle();
        }

        /* 更新上一拍状态，供下次比较。 */
        key0_last = key0_now;
        key1_last = key1_now;
        wkup_last = wkup_now;

        board_delay_ms(5U);
    }
}
