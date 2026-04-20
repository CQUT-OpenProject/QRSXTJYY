#include "board.h"

/* 按键去抖延时与主循环扫描周期。 */
#define KEY_DEBOUNCE_MS 20U
#define KEY_SCAN_MS 5U

/* KEY0 单击检测：按下稳定 + 等待释放 + 释放稳定。 */
static bool key0_click(void)
{
    if (!board_key0_pressed()) {
        return false;
    }

    /* 第一次确认：按下后延时，过滤抖动。 */
    board_delay_ms(KEY_DEBOUNCE_MS);
    if (!board_key0_pressed()) {
        return false;
    }

    /* 按住时不重复触发，等到松手。 */
    while (board_key0_pressed()) {
    }

    /* 第二次确认：松手后延时，过滤抖动。 */
    board_delay_ms(KEY_DEBOUNCE_MS);
    return true;
}

/* KEY1 单击检测流程同 KEY0。 */
static bool key1_click(void)
{
    if (!board_key1_pressed()) {
        return false;
    }

    board_delay_ms(KEY_DEBOUNCE_MS);
    if (!board_key1_pressed()) {
        return false;
    }

    while (board_key1_pressed()) {
    }

    board_delay_ms(KEY_DEBOUNCE_MS);
    return true;
}

/* WK_UP 单击检测流程同上（WK_UP 为高电平按下）。 */
static bool wkup_click(void)
{
    if (!board_wkup_pressed()) {
        return false;
    }

    board_delay_ms(KEY_DEBOUNCE_MS);
    if (!board_wkup_pressed()) {
        return false;
    }

    while (board_wkup_pressed()) {
    }

    board_delay_ms(KEY_DEBOUNCE_MS);
    return true;
}

int main(void)
{
    board_init();

    while (1) {
        /* Q4: 在去抖后的“单击事件”上做翻转。 */
        if (key0_click()) {
            board_led0_toggle();
        }

        if (key1_click()) {
            board_led1_toggle();
        }

        if (wkup_click()) {
            board_led_all_toggle();
        }

        board_delay_ms(KEY_SCAN_MS);
    }
}
