#include "board.h"

#include "stm32f10x_gpio.h"
#include "stm32f10x_rcc.h"
#include "stm32f10x_usart.h"

#define KEY0_PORT GPIOC
#define KEY0_PIN GPIO_Pin_5
#define KEY0_CLK RCC_APB2Periph_GPIOC

#define KEY1_PORT GPIOA
#define KEY1_PIN GPIO_Pin_15
#define KEY1_CLK RCC_APB2Periph_GPIOA

#define WKUP_PORT GPIOA
#define WKUP_PIN GPIO_Pin_0
#define WKUP_CLK RCC_APB2Periph_GPIOA

static void board_gpio_init(void)
{
    GPIO_InitTypeDef gpio_init;

    /* 打开 AFIO、GPIOA、GPIOC、USART1 时钟。 */
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO | RCC_APB2Periph_GPIOA | KEY0_CLK | KEY1_CLK | WKUP_CLK | RCC_APB2Periph_USART1, ENABLE);

    /* 关闭 JTAG，保留 SWD，释放 PA15。 */
    GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable, ENABLE);

    /* PA9: USART1_TX 复用推挽输出。 */
    gpio_init.GPIO_Pin = GPIO_Pin_9;
    gpio_init.GPIO_Speed = GPIO_Speed_50MHz;
    gpio_init.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_Init(GPIOA, &gpio_init);

    /* PA10: USART1_RX 浮空输入。 */
    gpio_init.GPIO_Pin = GPIO_Pin_10;
    gpio_init.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOA, &gpio_init);

    /* KEY0: 上拉输入，按下为低电平。 */
    gpio_init.GPIO_Pin = KEY0_PIN;
    gpio_init.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_Init(KEY0_PORT, &gpio_init);

    /* KEY1: 上拉输入，按下为低电平。 */
    gpio_init.GPIO_Pin = KEY1_PIN;
    gpio_init.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_Init(KEY1_PORT, &gpio_init);

    /* WK_UP: 下拉输入，按下为高电平。 */
    gpio_init.GPIO_Pin = WKUP_PIN;
    gpio_init.GPIO_Mode = GPIO_Mode_IPD;
    GPIO_Init(WKUP_PORT, &gpio_init);
}

static void board_usart1_init(void)
{
    USART_InitTypeDef usart_init;

    /* 先装入一套默认参数，再改成本实验需要的 115200 8N1。 */
    USART_StructInit(&usart_init);
    usart_init.USART_BaudRate = 115200U;
    usart_init.USART_WordLength = USART_WordLength_8b;
    usart_init.USART_StopBits = USART_StopBits_1;
    usart_init.USART_Parity = USART_Parity_No;
    usart_init.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    usart_init.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;

    USART_Init(USART1, &usart_init);
    /* 打开串口，后面就可以直接发和收。 */
    USART_Cmd(USART1, ENABLE);
}

void board_init(void)
{
    /* 刷新系统时钟变量，给延时和波特率配置使用。 */
    SystemCoreClockUpdate();
    board_gpio_init();
    board_usart1_init();
}

void board_delay_ms(uint32_t ms)
{
    while (ms-- > 0U) {
        /* 让 SysTick 跑 1ms，到了就退出。 */
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
    /* KEY0 按下为低电平。 */
    return GPIO_ReadInputDataBit(KEY0_PORT, KEY0_PIN) == Bit_RESET;
}

bool board_key1_pressed(void)
{
    /* KEY1 按下为低电平。 */
    return GPIO_ReadInputDataBit(KEY1_PORT, KEY1_PIN) == Bit_RESET;
}

bool board_wkup_pressed(void)
{
    /* WK_UP 按下为高电平。 */
    return GPIO_ReadInputDataBit(WKUP_PORT, WKUP_PIN) == Bit_SET;
}

void board_usart1_write_byte(uint8_t data)
{
    /* 先把一个字节写到发送数据寄存器。 */
    USART_SendData(USART1, data);
    /* 等待发送数据寄存器空，说明这个字节已经送进去了。 */
    while (USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET) {
    }
}

void board_usart1_write_str(const char *text)
{
    /* 把字符串里的字符一个一个发出去。 */
    while (*text != '\0') {
        board_usart1_write_byte((uint8_t)*text);
        text++;
    }
}

bool board_usart1_read_byte(uint8_t *data)
{
    /* 没有收到新字节时直接返回 false。 */
    if (USART_GetFlagStatus(USART1, USART_FLAG_RXNE) == RESET) {
        return false;
    }

    /* 读出 1 个字节，保存到调用者给的变量里。 */
    *data = (uint8_t)(USART_ReceiveData(USART1) & 0xFFU);
    return true;
}
