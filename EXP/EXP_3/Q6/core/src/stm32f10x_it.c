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

void EXTI9_5_IRQHandler(void)
{
    /* KEY0 在 EXTI5，归属于 EXTI9_5 中断组。 */
    if (EXTI_GetITStatus(EXTI_Line5) != RESET) {
        board_led0_toggle();
        EXTI_ClearITPendingBit(EXTI_Line5);
    }
}

void EXTI15_10_IRQHandler(void)
{
    /* KEY1 在 EXTI15，归属于 EXTI15_10 中断组。 */
    if (EXTI_GetITStatus(EXTI_Line15) != RESET) {
        board_led1_toggle();
        EXTI_ClearITPendingBit(EXTI_Line15);
    }
}
