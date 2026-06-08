#![cfg_attr(target_arch = "arm", no_std)]
#![cfg_attr(target_arch = "arm", no_main)]

#[cfg(target_arch = "arm")]
use core::fmt::{self, Write};

#[cfg(target_arch = "arm")]
use cortex_m_rt::entry;
#[cfg(target_arch = "arm")]
use panic_halt as _;
#[cfg(target_arch = "arm")]
use stm32f1xx_hal::pac;

#[cfg(target_arch = "arm")]
const USART1_BRR: u32 = 0x45; // 配置 USART1 115200 波特率，8 MHz 时钟
#[cfg(target_arch = "arm")]
const TIM2_PSC: u32 = 7; // 8 MHz / (7 + 1) = 1 MHz
#[cfg(target_arch = "arm")]
const TIM2_ARR: u32 = 999; // 1 MHz / (999 + 1) = 1 kHz，即 1 ms
#[cfg(target_arch = "arm")]
const SEND_INTERVAL_MS: u16 = 100; // 每 100ms 发送一次 NOW
#[cfg(target_arch = "arm")]
const INPUT_BUFFER_LEN: usize = 40; // 串口修改时间命令缓冲区长度

struct DAT {
    year: u16,
    month: u8,
    day: u8,
    week: u8,
}

struct TIME {
    hour: u8,
    minute: u8,
    second: u8,
    ms: u16,
}

struct NOW {
    dat: DAT,
    time: TIME,
}

impl NOW {
    fn new() -> Self {
        Self {
            dat: DAT {
                year: 2026,
                month: 5,
                day: 31,
                week: 0,
            },
            time: TIME {
                hour: 0,
                minute: 0,
                second: 0,
                ms: 0,
            },
        }
    }

    // 定时器每到 1ms 调用一次，软件时钟进位
    fn tick_ms(&mut self) {
        self.time.ms += 1;

        if self.time.ms >= 1000 {
            self.time.ms = 0;
            self.add_second();
        }
    }

    fn add_second(&mut self) {
        self.time.second += 1;

        if self.time.second >= 60 {
            self.time.second = 0;
            self.time.minute += 1;
        }

        if self.time.minute >= 60 {
            self.time.minute = 0;
            self.time.hour += 1;
        }

        if self.time.hour >= 24 {
            self.time.hour = 0;
            self.add_day();
        }
    }

    fn add_day(&mut self) {
        self.dat.day += 1;
        self.dat.week = (self.dat.week + 1) % 7;

        if self.dat.day > month_days(self.dat.year, self.dat.month) {
            self.dat.day = 1;
            self.dat.month += 1;
        }

        if self.dat.month > 12 {
            self.dat.month = 1;
            self.dat.year += 1;
        }
    }

    fn set_time(
        &mut self,
        year: u16,
        month: u8,
        day: u8,
        hour: u8,
        minute: u8,
        second: u8,
        ms: u16,
    ) {
        self.dat.year = year;
        self.dat.month = month;
        self.dat.day = day;
        self.dat.week = calc_week(year, month, day);
        self.time.hour = hour;
        self.time.minute = minute;
        self.time.second = second;
        self.time.ms = ms;
    }
}

fn is_leap_year(year: u16) -> bool {
    (year % 4 == 0 && year % 100 != 0) || year % 400 == 0
}

fn month_days(year: u16, month: u8) -> u8 {
    match month {
        1 | 3 | 5 | 7 | 8 | 10 | 12 => 31,
        4 | 6 | 9 | 11 => 30,
        2 => {
            if is_leap_year(year) {
                29
            } else {
                28
            }
        }
        _ => 30,
    }
}

fn calc_week(year: u16, month: u8, day: u8) -> u8 {
    let mut days = 0u32;
    let mut y = 2000u16;
    let mut m = 1u8;

    while y < year {
        if is_leap_year(y) {
            days += 366;
        } else {
            days += 365;
        }
        y += 1;
    }

    while m < month {
        days += u32::from(month_days(year, m));
        m += 1;
    }

    days += u32::from(day - 1);

    // 2000-01-01 是星期六，这里 0 表示 SUN，6 表示 SAT
    ((6 + days) % 7) as u8
}

fn week_name(week: u8) -> &'static str {
    match week {
        0 => "SUN",
        1 => "MON",
        2 => "TUE",
        3 => "WED",
        4 => "THU",
        5 => "FRI",
        6 => "SAT",
        _ => "UNK",
    }
}

#[cfg(target_arch = "arm")]
struct Uart1 {
    usart: pac::USART1,
}

#[cfg(target_arch = "arm")]
impl Uart1 {
    // 发送一个字节
    fn write_byte(&mut self, byte: u8) {
        while self
            .usart
            .sr
            .read() // 读取状态寄存器
            .txe()
            .bit_is_clear()
        {}

        self.usart.dr.write(|w| w.dr().bits(u16::from(byte))); // 发送一个字节
    }

    fn read_byte(&mut self) -> Option<u8> {
        if self.usart.sr.read().rxne().bit_is_set() {
            let data = self.usart.dr.read().dr().bits();
            Some((data & 0xFF) as u8)
        } else {
            None
        }
    }
}

#[cfg(target_arch = "arm")]
impl Write for Uart1 {
    // 串口打印字符串
    fn write_str(&mut self, text: &str) -> fmt::Result {
        for byte in text.bytes() {
            self.write_byte(byte);
        }

        Ok(())
    }
}

#[cfg(target_arch = "arm")]
fn setup_clock_gpio_usart_timer(dp: &pac::Peripherals) {
    dp.RCC
        .apb2enr
        .modify(|_, w| w.afioen().enabled().iopaen().enabled().usart1en().enabled());

    dp.RCC.apb1enr.modify(|_, w| w.tim2en().enabled());

    // 配置 USART1_TX、USART1_RX
    dp.GPIOA.crh.modify(|r, w| unsafe {
        let mut bits = r.bits();

        // USART1_TX PA9 推挽输出，最大 50 MHz
        bits &= !(0xF << 4);
        bits |= 0xB << 4;

        // USART1_RX PA10 输入浮空
        bits &= !(0xF << 8);
        bits |= 0x4 << 8;

        w.bits(bits)
    });

    /* 初始化 USART1 */
    dp.USART1.brr.write(|w| unsafe { w.bits(USART1_BRR) });

    dp.USART1.cr1.write(|w| {
        w.ue()
            .enabled() // USART 使能
            .te()
            .enabled() // 发送使能
            .re()
            .enabled() // 接收使能
    });

    /* 初始化 TIM2 为 1ms 溢出一次 */
    dp.TIM2.cr1.write(|w| unsafe { w.bits(0) });
    dp.TIM2.psc.write(|w| unsafe { w.bits(TIM2_PSC) });
    dp.TIM2.arr.write(|w| unsafe { w.bits(TIM2_ARR) });
    dp.TIM2.cnt.write(|w| unsafe { w.bits(0) });
    dp.TIM2.egr.write(|w| w.ug().update());
    dp.TIM2.sr.write(|w| unsafe { w.bits(0) });
    dp.TIM2.cr1.write(|w| w.cen().enabled());
}

#[cfg(target_arch = "arm")]
fn tim2_1ms_arrived(tim: &pac::TIM2) -> bool {
    if tim.sr.read().uif().is_update_pending() {
        // 清除更新标志，等待下一次 1ms 到来
        tim.sr.write(|w| w.uif().clear());
        true
    } else {
        false
    }
}

#[cfg(target_arch = "arm")]
fn send_now(uart: &mut Uart1, now: &NOW) {
    let _ = write!(
        uart,
        "{:04}-{:02}-{:02},{},{:02}:{:02}:{:02}.{:03}\r\n",
        now.dat.year,
        now.dat.month,
        now.dat.day,
        week_name(now.dat.week),
        now.time.hour,
        now.time.minute,
        now.time.second,
        now.time.ms,
    );
}

#[cfg(target_arch = "arm")]
fn handle_serial_input(
    uart: &mut Uart1,
    now: &mut NOW,
    input: &mut [u8; INPUT_BUFFER_LEN],
    idx: &mut usize,
) {
    while let Some(byte) = uart.read_byte() {
        if byte == b'\r' || byte == b'\n' {
            if *idx > 0 {
                process_line(uart, now, &input[..*idx]);
                *idx = 0;
            }
        } else if *idx < INPUT_BUFFER_LEN {
            input[*idx] = byte;
            *idx += 1;
        } else {
            *idx = 0;
            let _ = write!(uart, "ERR\r\n");
        }
    }
}

#[cfg(target_arch = "arm")]
fn process_line(uart: &mut Uart1, now: &mut NOW, line: &[u8]) {
    if let Some((year, month, day, hour, minute, second, ms)) = parse_set_time(line) {
        now.set_time(year, month, day, hour, minute, second, ms);
        let _ = write!(uart, "OK\r\n");
        send_now(uart, now);
    } else {
        let _ = write!(uart, "ERR\r\n");
    }
}

// 协议：>yyyy-mm-dd hh:mm:ss.ms 回车，例如 >2026-06-01 12:30:05.250
fn parse_set_time(line: &[u8]) -> Option<(u16, u8, u8, u8, u8, u8, u16)> {
    let mut idx = 0usize;

    while idx < line.len() && (line[idx] == b' ' || line[idx] == b'\t' || line[idx] == b'"') {
        idx += 1;
    }

    if idx >= line.len() || line[idx] != b'>' {
        return None;
    }
    idx += 1;

    let (year, next) = read_number(line, idx, 4)?;
    idx = next;
    idx = expect_byte(line, idx, b'-')?;

    let (month, next) = read_number(line, idx, 2)?;
    idx = next;
    idx = expect_byte(line, idx, b'-')?;

    let (day, next) = read_number(line, idx, 2)?;
    idx = next;

    if idx >= line.len() || (line[idx] != b' ' && line[idx] != b',') {
        return None;
    }
    idx += 1;

    let (hour, next) = read_number(line, idx, 2)?;
    idx = next;
    idx = expect_byte(line, idx, b':')?;

    let (minute, next) = read_number(line, idx, 2)?;
    idx = next;
    idx = expect_byte(line, idx, b':')?;

    let (second, next) = read_number(line, idx, 2)?;
    idx = next;
    idx = expect_byte(line, idx, b'.')?;

    let (ms, next) = read_number(line, idx, 3)?;
    idx = next;

    while idx < line.len() && (line[idx] == b' ' || line[idx] == b'\t' || line[idx] == b'"') {
        idx += 1;
    }

    if idx != line.len() {
        return None;
    }

    if !check_datetime(
        year,
        month as u8,
        day as u8,
        hour as u8,
        minute as u8,
        second as u8,
        ms,
    ) {
        return None;
    }

    Some((
        year,
        month as u8,
        day as u8,
        hour as u8,
        minute as u8,
        second as u8,
        ms,
    ))
}

fn read_number(line: &[u8], mut idx: usize, len: usize) -> Option<(u16, usize)> {
    let mut data = 0u16;
    let end = idx + len;

    if end > line.len() {
        return None;
    }

    while idx < end {
        if line[idx] < b'0' || line[idx] > b'9' {
            return None;
        }

        data = data * 10 + u16::from(line[idx] - b'0');
        idx += 1;
    }

    Some((data, idx))
}

fn expect_byte(line: &[u8], idx: usize, byte: u8) -> Option<usize> {
    if idx < line.len() && line[idx] == byte {
        Some(idx + 1)
    } else {
        None
    }
}

fn check_datetime(
    year: u16,
    month: u8,
    day: u8,
    hour: u8,
    minute: u8,
    second: u8,
    ms: u16,
) -> bool {
    if month < 1 || month > 12 {
        return false;
    }

    if day < 1 || day > month_days(year, month) {
        return false;
    }

    if hour > 23 || minute > 59 || second > 59 || ms > 999 {
        return false;
    }

    true
}

#[cfg(target_arch = "arm")]
#[entry]
fn main() -> ! {
    let dp = pac::Peripherals::take().unwrap(); // 获取外设访问权限

    setup_clock_gpio_usart_timer(&dp);

    let mut uart = Uart1 { usart: dp.USART1 };
    let mut now = NOW::new();
    let mut send_count = 0u16;
    let mut input_buffer = [0u8; INPUT_BUFFER_LEN];
    let mut input_idx = 0usize;

    send_now(&mut uart, &now);

    loop {
        handle_serial_input(&mut uart, &mut now, &mut input_buffer, &mut input_idx);

        if tim2_1ms_arrived(&dp.TIM2) {
            now.tick_ms();
            send_count += 1;

            if send_count >= SEND_INTERVAL_MS {
                send_count = 0;
                send_now(&mut uart, &now);
            }
        }
    }
}

#[cfg(not(target_arch = "arm"))]
fn main() {}
