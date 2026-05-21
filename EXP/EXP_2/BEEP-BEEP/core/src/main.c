#include "main.h"

#define BEEP_GPIO_PORT GPIOA
#define BEEP_GPIO_PIN GPIO_Pin_6
#define BEEP_GPIO_CLK RCC_APB2Periph_GPIOA

// 休止符
#define STOP_NOTE 0U

// 蜂鸣器占空比，用于降低音量。
// 可调范围为 1U~10000U
#define VOLUME_PER_TEN_THOUSAND 8U

extern const char music_data[];
extern const unsigned int music_data_len;

static void delay_ticks(uint32_t ticks) {
  while (ticks != 0U) {
    uint32_t chunk = (ticks > 0x00FFFFFFUL) ? 0x00FFFFFFUL : ticks;

    SysTick->LOAD = chunk - 1U;
    SysTick->VAL = 0U;
    SysTick->CTRL = SysTick_CTRL_CLKSOURCE_Msk | SysTick_CTRL_ENABLE_Msk;

    while ((SysTick->CTRL & SysTick_CTRL_COUNTFLAG_Msk) == 0U) {
    }

    SysTick->CTRL = 0U;
    ticks -= chunk;
  }
}

static void delay_us(uint32_t us) {
  uint32_t ticks_per_us = SystemCoreClock / 1000000UL;
  delay_ticks(us * ticks_per_us);
}

static void delay_ms(uint32_t ms) {
  while (ms-- != 0U) {
    delay_us(1000U);
  }
}

// 设置蜂鸣器频率
static void set_beep_hz(uint32_t frequency_hz) {
  uint32_t period_ticks;
  uint32_t duty_ticks;

  if (frequency_hz == 0U) {
    TIM_SetCompare1(TIM3, 0); // 停止发声
    return;
  }

  // 设置计数器频率为 10MHz
  period_ticks = 10000000UL / frequency_hz;
  TIM_SetAutoreload(TIM3, period_ticks - 1U);

  // 计算占空比，避免蜂鸣器音量过大
  duty_ticks = period_ticks * VOLUME_PER_TEN_THOUSAND / 10000U;
  if (duty_ticks == 0U) {
    duty_ticks = 1U; // 至少保留 1 个 tick 的高电平，避免完全无声
  }

  TIM_SetCompare1(TIM3, duty_ticks);
}

static void beep_init(void) {
  GPIO_InitTypeDef gpio_init = {0};
  TIM_TimeBaseInitTypeDef tim_init = {0};
  TIM_OCInitTypeDef oc_init = {0};

  RCC_APB2PeriphClockCmd(BEEP_GPIO_CLK, ENABLE);
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);

  // PA6 复用推挽输出 -> TIM3_CH1
  gpio_init.GPIO_Pin = BEEP_GPIO_PIN;
  gpio_init.GPIO_Speed = GPIO_Speed_50MHz;
  gpio_init.GPIO_Mode = GPIO_Mode_AF_PP;
  GPIO_Init(BEEP_GPIO_PORT, &gpio_init);

  // 初始化 TIM3
  tim_init.TIM_Period = 9999;
  tim_init.TIM_Prescaler = (SystemCoreClock / 10000000UL) - 1;
  tim_init.TIM_ClockDivision = TIM_CKD_DIV1;
  tim_init.TIM_CounterMode = TIM_CounterMode_Up;
  TIM_TimeBaseInit(TIM3, &tim_init);

  // 初始化 TIM3 CH1 PWM1 模式
  oc_init.TIM_OCMode = TIM_OCMode_PWM1;
  oc_init.TIM_OutputState = TIM_OutputState_Enable;
  oc_init.TIM_Pulse = 0;
  oc_init.TIM_OCPolarity = TIM_OCPolarity_High;
  TIM_OC1Init(TIM3, &oc_init);

  TIM_OC1PreloadConfig(TIM3, TIM_OCPreload_Enable);
  TIM_ARRPreloadConfig(TIM3, ENABLE);
  TIM_Cmd(TIM3, ENABLE);
}

// 播放指定频率和时长的音符
static void play_note(uint32_t frequency_hz, uint32_t duration_us) {
  if (frequency_hz == STOP_NOTE) {
    set_beep_hz(0U); // 停止发声
    delay_us(duration_us); // 休止符期间保持静音
    return;
  }

  set_beep_hz(frequency_hz); // 设置频率并开始发声
  delay_us(duration_us); // 持续指定时长
}

// 解析并播放音乐数据
static void play_music_file(const char *music_data, unsigned int music_len) {
  const char *p = music_data;
  const char *music_end = music_data + music_len;

  while (p < music_end && *p != '\0') {
    // 跳过空白字符
    while (p < music_end &&
           (*p == ' ' || *p == '\r' || *p == '\n' || *p == '\t')) {
      p++;
    }
    if (p >= music_end || *p == '\0')
      break;

    // 忽略以 # 开头的注释行
    if (*p == '#') {
      while (p < music_end && *p != '\n' && *p != '\0')
        p++;
      continue;
    }

    // 解析频率（Hz）
    uint32_t freq = 0;
    while (p < music_end && *p >= '0' && *p <= '9') {
      freq = freq * 10 + (*p - '0');
      p++;
    }

    // 跳过频率与时长之间的空格
    while (p < music_end && (*p == ' ' || *p == '\t'))
      p++;

    // 解析时长（us）
    uint32_t dur = 0;
    while (p < music_end && *p >= '0' && *p <= '9') {
      dur = dur * 10 + (*p - '0');
      p++;
    }

    // 跳到下一行
    while (p < music_end && *p != '\n' && *p != '\0')
      p++;

    // 时长有效时才播放当前音符
    if (dur > 0) {
      play_note(freq, dur);
    }
  }

  set_beep_hz(0U);
}

int main(void) {
  beep_init();

  while (1) {
    play_music_file(music_data, music_data_len);
    delay_ms(1000U);
  }
}
