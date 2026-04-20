#include "stm32f10x_it.h"

#include "board.h"
#include "stm32f10x_exti.h"

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

void EXTI0_IRQHandler(void)
{
    /* 先判断是否真的是 EXTI0 触发。 */
    if (EXTI_GetITStatus(EXTI_Line0) != RESET) {
        /* Q5 逻辑：每按一次 WK_UP，DS0 和 DS1 同时翻转。 */
        board_led_all_toggle();

        /* 清中断标志位，避免重复进入。 */
        EXTI_ClearITPendingBit(EXTI_Line0);
    }
}
