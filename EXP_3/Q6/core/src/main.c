#include "board.h"

int main(void)
{
    board_init();

    /* Q6: 开启 KEY0/KEY1 外部中断。 */
    board_key_exti_key0_init();
    board_key_exti_key1_init();

    while (1) {
        /* 主循环空转，按键响应在中断函数中完成。 */
        board_delay_ms(50U);
    }
}
