#include "stm32f10x_it.h"

#include "board.h"
#include "stm32f10x_exti.h"

/*
 * Q7 重点：
 * EXTI9_5 与 EXTI15_10 里可能挂了多条线，
 * 因此按“逐线检查”方式处理。
 */
static void handle_grouped_exti_line(uint8_t line)
{
    uint32_t exti_line = (uint32_t)1U << line;

    /* 该线未触发则直接返回。 */
    if (EXTI_GetITStatus(exti_line) == RESET) {
        return;
    }

    /* 只对实验中用到的线执行动作。 */
    if (line == 5U) {
        board_led0_toggle();
    }

    if (line == 15U) {
        board_led1_toggle();
    }

    /* 清除当前线的中断标志。 */
    EXTI_ClearITPendingBit(exti_line);
}

void NMI_Handler(void)
{
}

void HardFault_Handler(void)
{
    while (1) {
    }
}

void MemManage_Handler(void)
{
    while (1) {
    }
}

void BusFault_Handler(void)
{
    while (1) {
    }
}

void UsageFault_Handler(void)
{
    while (1) {
    }
}

void SVC_Handler(void)
{
}

void DebugMon_Handler(void)
{
}

void PendSV_Handler(void)
{
}

void SysTick_Handler(void)
{
}

void EXTI9_5_IRQHandler(void)
{
    uint8_t line;

    /* 遍历该中断组的每一条线。 */
    for (line = 5U; line <= 9U; ++line) {
        handle_grouped_exti_line(line);
    }
}

void EXTI15_10_IRQHandler(void)
{
    uint8_t line;

    /* 遍历该中断组的每一条线。 */
    for (line = 10U; line <= 15U; ++line) {
        handle_grouped_exti_line(line);
    }
}
