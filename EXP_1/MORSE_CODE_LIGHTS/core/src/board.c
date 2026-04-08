#include "morse_app.h"

#include "stm32f10x.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_rcc.h"

void board_init(void)
{
    GPIO_InitTypeDef gpio_init;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOC | RCC_APB2Periph_AFIO, ENABLE);
    GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable, ENABLE);

    gpio_init.GPIO_Pin = GPIO_Pin_8;
    gpio_init.GPIO_Mode = GPIO_Mode_Out_PP;
    gpio_init.GPIO_Speed = GPIO_Speed_2MHz;
    GPIO_Init(GPIOA, &gpio_init);
    GPIO_SetBits(GPIOA, GPIO_Pin_8);

    gpio_init.GPIO_Pin = GPIO_Pin_0;
    gpio_init.GPIO_Mode = GPIO_Mode_IPD;
    GPIO_Init(GPIOA, &gpio_init);

    gpio_init.GPIO_Pin = GPIO_Pin_15;
    gpio_init.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_Init(GPIOA, &gpio_init);

    gpio_init.GPIO_Pin = GPIO_Pin_5;
    gpio_init.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_Init(GPIOC, &gpio_init);
}

void board_led_set(bool on)
{
    if (on) {
        GPIO_ResetBits(GPIOA, GPIO_Pin_8);
    } else {
        GPIO_SetBits(GPIOA, GPIO_Pin_8);
    }
}

bool board_key_up_pressed(void)
{
    return GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_0) == Bit_SET;
}

bool board_key1_pressed(void)
{
    return GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_15) == Bit_RESET;
}

bool board_key0_pressed(void)
{
    return GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_5) == Bit_RESET;
}

void board_delay_ms(uint32_t ms)
{
    while (ms-- > 0U) {
        SysTick->LOAD = (SystemCoreClock / 1000U) - 1U;
        SysTick->VAL = 0U;
        SysTick->CTRL = SysTick_CTRL_CLKSOURCE_Msk | SysTick_CTRL_ENABLE_Msk;
        while ((SysTick->CTRL & SysTick_CTRL_COUNTFLAG_Msk) == 0U) {
        }
        SysTick->CTRL = 0U;
    }
}
