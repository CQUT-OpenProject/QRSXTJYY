#include "board.h"

int main(void)
{
    board_init();

    /* Q5: 仅开启 WK_UP 对应外部中断（EXTI0）。 */
    board_key_exti_wkup_init();

    while (1) {
        /* 主循环空转，按键响应在中断函数中完成。 */
        board_delay_ms(50U);
    }
}
