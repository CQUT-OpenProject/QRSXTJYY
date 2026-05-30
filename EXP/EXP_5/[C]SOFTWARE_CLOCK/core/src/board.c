#include "board.h"

#include "stm32f10x_gpio.h"
#include "stm32f10x_rcc.h"
#include "stm32f10x_tim.h"
#include "stm32f10x_usart.h"

static void board_gpio_usart_init(void)
{
    GPIO_InitTypeDef gpio_init;
    USART_InitTypeDef usart_init;

    /* 打开 GPIOA、AFIO 和 USART1 时钟。 */
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO | RCC_APB2Periph_GPIOA | RCC_APB2Periph_USART1, ENABLE);

    /* PA9: USART1_TX 复用推挽输出。 */
    gpio_init.GPIO_Pin = GPIO_Pin_9;
    gpio_init.GPIO_Speed = GPIO_Speed_50MHz;
    gpio_init.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_Init(GPIOA, &gpio_init);

    /* PA10: USART1_RX 浮空输入。 */
    gpio_init.GPIO_Pin = GPIO_Pin_10;
    gpio_init.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOA, &gpio_init);

    USART_StructInit(&usart_init);
    usart_init.USART_BaudRate = 115200U;
    usart_init.USART_WordLength = USART_WordLength_8b;
    usart_init.USART_StopBits = USART_StopBits_1;
    usart_init.USART_Parity = USART_Parity_No;
    usart_init.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    usart_init.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;

    USART_Init(USART1, &usart_init);
    USART_Cmd(USART1, ENABLE);
}

static void board_tim2_init(void)
{
    TIM_TimeBaseInitTypeDef tim_init;

    /* TIM2 挂在 APB1，系统 72MHz 时 APB1 定时器时钟仍为 72MHz。 */
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);

    TIM_TimeBaseStructInit(&tim_init);
    tim_init.TIM_Prescaler = 72U - 1U;      /* 72MHz / 72 = 1MHz */
    tim_init.TIM_Period = 1000U - 1U;       /* 1MHz / 1000 = 1kHz，也就是 1ms */
    tim_init.TIM_ClockDivision = TIM_CKD_DIV1;
    tim_init.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseInit(TIM2, &tim_init);

    TIM_ClearFlag(TIM2, TIM_FLAG_Update);
    TIM_Cmd(TIM2, ENABLE);
}

void board_init(void)
{
    SystemCoreClockUpdate();
    board_gpio_usart_init();
    board_tim2_init();
}

bool board_tim2_1ms_arrived(void)
{
    if (TIM_GetFlagStatus(TIM2, TIM_FLAG_Update) != RESET) {
        TIM_ClearFlag(TIM2, TIM_FLAG_Update);
        return true;
    }

    return false;
}

void board_usart1_write_byte(uint8_t data)
{
    USART_SendData(USART1, data);
    while (USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET) {
    }
}

void board_usart1_write_str(const char *text)
{
    while (*text != '\0') {
        board_usart1_write_byte((uint8_t)*text);
        text++;
    }
}

bool board_usart1_read_byte(uint8_t *data)
{
    if (USART_GetFlagStatus(USART1, USART_FLAG_RXNE) == RESET) {
        return false;
    }

    *data = (uint8_t)(USART_ReceiveData(USART1) & 0xFFU);
    return true;
}
