#include "board.h"

#include "misc.h"
#include "stm32f10x_exti.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_rcc.h"

/* DS0: PA8（低电平点亮） */
#define LED0_PORT GPIOA
#define LED0_PIN GPIO_Pin_8
#define LED0_CLK RCC_APB2Periph_GPIOA

/* DS1: PD2（低电平点亮） */
#define LED1_PORT GPIOD
#define LED1_PIN GPIO_Pin_2
#define LED1_CLK RCC_APB2Periph_GPIOD

/* KEY0: PC5（低电平按下） */
#define KEY0_PORT GPIOC
#define KEY0_PIN GPIO_Pin_5
#define KEY0_CLK RCC_APB2Periph_GPIOC

/* KEY1: PA15（低电平按下） */
#define KEY1_PORT GPIOA
#define KEY1_PIN GPIO_Pin_15
#define KEY1_CLK RCC_APB2Periph_GPIOA

/* WK_UP: PA0（高电平按下） */
#define WKUP_PORT GPIOA
#define WKUP_PIN GPIO_Pin_0
#define WKUP_CLK RCC_APB2Periph_GPIOA

/* 记录 LED 当前状态，供 toggle 接口使用。 */
static bool led0_on = false;
static bool led1_on = false;

static void board_gpio_init(void)
{
    GPIO_InitTypeDef gpio_init;

    /* 打开 GPIO 与 AFIO 时钟。 */
    RCC_APB2PeriphClockCmd(LED0_CLK | LED1_CLK | KEY0_CLK | KEY1_CLK | WKUP_CLK | RCC_APB2Periph_AFIO, ENABLE);

    /* 关闭 JTAG，保留 SWD，释放 PA15 作为 KEY1 输入。 */
    GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable, ENABLE);

    /* DS0 输出配置。 */
    gpio_init.GPIO_Pin = LED0_PIN;
    gpio_init.GPIO_Mode = GPIO_Mode_Out_PP;
    gpio_init.GPIO_Speed = GPIO_Speed_2MHz;
    GPIO_Init(LED0_PORT, &gpio_init);

    /* DS1 输出配置。 */
    gpio_init.GPIO_Pin = LED1_PIN;
    gpio_init.GPIO_Mode = GPIO_Mode_Out_PP;
    gpio_init.GPIO_Speed = GPIO_Speed_2MHz;
    GPIO_Init(LED1_PORT, &gpio_init);

    /* KEY0 输入上拉。 */
    gpio_init.GPIO_Pin = KEY0_PIN;
    gpio_init.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_Init(KEY0_PORT, &gpio_init);

    /* KEY1 输入上拉。 */
    gpio_init.GPIO_Pin = KEY1_PIN;
    gpio_init.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_Init(KEY1_PORT, &gpio_init);

    /* WK_UP 输入下拉（按下时为高电平）。 */
    gpio_init.GPIO_Pin = WKUP_PIN;
    gpio_init.GPIO_Mode = GPIO_Mode_IPD;
    GPIO_Init(WKUP_PORT, &gpio_init);
}

void board_init(void)
{
    /* 刷新 SystemCoreClock，保证延时计算正确。 */
    SystemCoreClockUpdate();
    board_gpio_init();

    /* 上电后先熄灭 DS0、DS1。 */
    board_led_all_set(false, false);
}

void board_delay_ms(uint32_t ms)
{
    /* 每次循环产生 1ms 延时。 */
    while (ms-- > 0U) {
        SysTick->LOAD = (SystemCoreClock / 1000U) - 1U;
        SysTick->VAL = 0U;
        SysTick->CTRL = SysTick_CTRL_CLKSOURCE_Msk | SysTick_CTRL_ENABLE_Msk;
        while ((SysTick->CTRL & SysTick_CTRL_COUNTFLAG_Msk) == 0U) {
        }
        SysTick->CTRL = 0U;
    }
}

bool board_key0_pressed(void)
{
    /* KEY0：低电平按下。 */
    return GPIO_ReadInputDataBit(KEY0_PORT, KEY0_PIN) == Bit_RESET;
}

bool board_key1_pressed(void)
{
    /* KEY1：低电平按下。 */
    return GPIO_ReadInputDataBit(KEY1_PORT, KEY1_PIN) == Bit_RESET;
}

bool board_wkup_pressed(void)
{
    /* WK_UP：高电平按下。 */
    return GPIO_ReadInputDataBit(WKUP_PORT, WKUP_PIN) == Bit_SET;
}

void board_led0_set(bool on)
{
    led0_on = on;

    /* LED 为低电平点亮，所以 on=true 时输出低。 */
    if (on) {
        GPIO_ResetBits(LED0_PORT, LED0_PIN);
    } else {
        GPIO_SetBits(LED0_PORT, LED0_PIN);
    }
}

void board_led1_set(bool on)
{
    led1_on = on;

    /* LED 为低电平点亮，所以 on=true 时输出低。 */
    if (on) {
        GPIO_ResetBits(LED1_PORT, LED1_PIN);
    } else {
        GPIO_SetBits(LED1_PORT, LED1_PIN);
    }
}

void board_led_all_set(bool ds0_on, bool ds1_on)
{
    board_led0_set(ds0_on);
    board_led1_set(ds1_on);
}

void board_led0_toggle(void)
{
    board_led0_set(!led0_on);
}

void board_led1_toggle(void)
{
    board_led1_set(!led1_on);
}

void board_led_all_toggle(void)
{
    board_led_all_set(!led0_on, !led1_on);
}

void board_key_exti_wkup_init(void)
{
    EXTI_InitTypeDef exti_init;
    NVIC_InitTypeDef nvic_init;

    /* PA0 -> EXTI0 */
    GPIO_EXTILineConfig(GPIO_PortSourceGPIOA, GPIO_PinSource0);

    EXTI_ClearITPendingBit(EXTI_Line0);
    exti_init.EXTI_Line = EXTI_Line0;
    exti_init.EXTI_Mode = EXTI_Mode_Interrupt;
    /* WK_UP 按下为上升沿。 */
    exti_init.EXTI_Trigger = EXTI_Trigger_Rising;
    exti_init.EXTI_LineCmd = ENABLE;
    EXTI_Init(&exti_init);

    nvic_init.NVIC_IRQChannel = EXTI0_IRQn;
    nvic_init.NVIC_IRQChannelPreemptionPriority = 2;
    nvic_init.NVIC_IRQChannelSubPriority = 2;
    nvic_init.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&nvic_init);
}

void board_key_exti_key0_init(void)
{
    EXTI_InitTypeDef exti_init;
    NVIC_InitTypeDef nvic_init;

    /* PC5 -> EXTI5 */
    GPIO_EXTILineConfig(GPIO_PortSourceGPIOC, GPIO_PinSource5);

    EXTI_ClearITPendingBit(EXTI_Line5);
    exti_init.EXTI_Line = EXTI_Line5;
    exti_init.EXTI_Mode = EXTI_Mode_Interrupt;
    /* KEY0 按下为下降沿。 */
    exti_init.EXTI_Trigger = EXTI_Trigger_Falling;
    exti_init.EXTI_LineCmd = ENABLE;
    EXTI_Init(&exti_init);

    nvic_init.NVIC_IRQChannel = EXTI9_5_IRQn;
    nvic_init.NVIC_IRQChannelPreemptionPriority = 2;
    nvic_init.NVIC_IRQChannelSubPriority = 2;
    nvic_init.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&nvic_init);
}

void board_key_exti_key1_init(void)
{
    EXTI_InitTypeDef exti_init;
    NVIC_InitTypeDef nvic_init;

    /* PA15 -> EXTI15 */
    GPIO_EXTILineConfig(GPIO_PortSourceGPIOA, GPIO_PinSource15);

    EXTI_ClearITPendingBit(EXTI_Line15);
    exti_init.EXTI_Line = EXTI_Line15;
    exti_init.EXTI_Mode = EXTI_Mode_Interrupt;
    /* KEY1 按下为下降沿。 */
    exti_init.EXTI_Trigger = EXTI_Trigger_Falling;
    exti_init.EXTI_LineCmd = ENABLE;
    EXTI_Init(&exti_init);

    nvic_init.NVIC_IRQChannel = EXTI15_10_IRQn;
    nvic_init.NVIC_IRQChannelPreemptionPriority = 2;
    nvic_init.NVIC_IRQChannelSubPriority = 2;
    nvic_init.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&nvic_init);
}
