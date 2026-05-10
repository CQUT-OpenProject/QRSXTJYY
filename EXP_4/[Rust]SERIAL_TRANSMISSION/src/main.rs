#![no_std]
#![no_main]

use core::fmt::{self, Write};

use cortex_m_rt::entry;
use panic_halt as _;
use stm32f1xx_hal::pac;

const USART1_BRR: u32 = 0x45; // 配置 USART1 115200 波特率，8 MHz 时钟
const INPUT_BUFFER_LEN: usize = 64; // 串口输入缓冲区长度
const DEBOUNCE: u8 = 4;
const KEY_SAMPL_FRQ: u16 = 20000; // 按键采样频率
const WAIT_SERIAL: u16 = 40000; // 等待串口输入完成的循环次数

struct Uart1 {
    usart: pac::USART1,
}

impl Uart1 {
    // 发送一个字节
    fn write_byte(&mut self, byte: u8) {
        // 只要发送数据寄存器还没空，一直等待
        while self.usart
                  .sr.read() // 读取状态寄存器
                  .txe().bit_is_clear() {} // 发送数据寄存器非空

        // 向数据寄存器写入要发送的字节
        self.usart
            .dr
            .write(|w| w.dr().bits(u16::from(byte))); // 此接口要求传入 u16
    }

    // 接收一个字节，如果没有数据可读则返回 None
    fn read_byte(&mut self) -> Option<u8> {
        if self.usart
               .sr.read()
               .rxne().bit_is_set() // 接收数据寄存器非空
        {
            let data = self.usart.dr.read().dr().bits();
            Some((data & 0xFF) as u8) // 保留 data 的最低 8 位
        } else {
            None
        }
    }
}

impl Write for Uart1 {
    // 串口打印字符串
    fn write_str(&mut self, text: &str) -> fmt::Result {
        for byte in text.bytes() {
            self.write_byte(byte); // 发送字符串中的每个字节
        }

        Ok(()) // 写入成功
    }
}

struct Button {
    stable: bool, // 当前状态
    last_raw: bool, // 上一次读取的状态
    count: u8, // 连续读到相同原始状态的次数
}

impl Button {
    // 初始化按钮状态
    fn new(pressed: bool) -> Self {
        Self {
            stable: pressed,
            last_raw: pressed,
            count: 0,
        }
    }

    // 更新按钮状态
    fn update(&mut self, raw: bool) -> Option<bool> {
        if raw == self.last_raw { // 按钮状态无变化
            if self.count < DEBOUNCE {
                self.count += 1;
            }
        } else { // 按钮状态变化
            self.last_raw = raw;
            self.count = 0;
        }

        // 判断当前状态是否稳定
        if self.count >= DEBOUNCE && self.stable != raw {
            self.stable = raw;
            Some(raw) // 返回状态变化事件
        } else {
            None
        }
    }
}

// 初始化时钟、GPIO 和 USART1
fn setup_clock_gpio_usart(dp: &pac::Peripherals) {
    dp.RCC.apb2enr.modify(|_, w| {
        w
        .afioen().enabled()
        .iopaen().enabled()
        .iopcen().enabled()
        .usart1en().enabled()
    });

    // 禁用 JTAG,释放 PA15 给 KEY1
    dp.AFIO.mapr.modify(|_, w| unsafe {
        w.swj_cfg().bits(0b010)
    });

    // 配置 WK_UP
    dp.GPIOA.crl.modify(|r, w| unsafe {
        let mut bits = r.bits();
        // WK_UP PA0 输入模式，带上拉
        bits &= !(0xF << 0); // 清除原有配置
        bits |= 0x8 << 0;
        w.bits(bits)
    });

    // 配置 KEY0
    dp.GPIOC.crl.modify(|r, w| unsafe {
        let mut bits = r.bits();
        // KEY0 PC5 输入模式，带上拉
        bits &= !(0xF << 20);
        bits |= 0x8 << 20;
        w.bits(bits)
    });

    dp.GPIOA.odr.modify(|r, w| unsafe {
        let mut bits = r.bits();
        bits &= !(1 << 0);
        bits |= 1 << 15;
        w.bits(bits)
    });

    dp.GPIOC.odr.modify(|r, w| unsafe {
        let mut bits = r.bits();
        bits |= 1 << 5;
        w.bits(bits)
    });

    // 配置 USART1_TX、USART1_RX、KEY1
    dp.GPIOA.crh.modify(|r, w| unsafe {
        let mut bits = r.bits();

        // USART1_TX PA9 推挽输出，最大 50 MHz
        bits &= !(0xF << 4);
        bits |= 0xB << 4;

        // USART1_RX PA10 输入浮空
        bits &= !(0xF << 8);
        bits |= 0x4 << 8;

        // KEY1 PA15 输入下拉
        bits &= !(0xF << 28);
        bits |= 0x8 << 28;

        w.bits(bits)
    });

    /* 初始化 USART1 */
    dp.USART1.brr.write(|w| unsafe {
        w.bits(USART1_BRR)
    });

    dp.USART1.cr1.write(|w| {
        w
        .ue().enabled() // USART 使能
        .te().enabled() // 发送使能
        .re().enabled() // 接收使能
    });
}

fn read_key0_pressed(gpioc: &pac::GPIOC) -> bool {
    gpioc.idr.read().idr5().bit_is_clear()
}

fn read_key1_pressed(gpioa: &pac::GPIOA) -> bool {
    gpioa.idr.read().idr15().bit_is_clear()
}

fn read_wkup_pressed(gpioa: &pac::GPIOA) -> bool {
    gpioa.idr.read().idr0().bit_is_set()
}

// 发送按键消息
fn send_key_message(uart: &mut Uart1, name: &str, pressed: bool) {
    if pressed {
        let _ = write!(uart, "{name} keydown\r\n");
    } else {
        let _ = write!(uart, "{name} keyup\r\n");
    }
}

// 处理串口输入
fn handle_serial_input(uart: &mut Uart1, input: &mut [u8; INPUT_BUFFER_LEN], idx: &mut usize) -> bool {
    let mut received = false;

    // 读取所有已经收到的字节
    while let Some(byte) = uart.read_byte() {
        received = true;

        // 回车或换行
        if byte == b'\r' || byte == b'\n' {
            if *idx > 0 {
                process_line(uart, &input[..*idx]);
                *idx = 0;
            }
        // 普通字符
        } else if *idx < INPUT_BUFFER_LEN {
            input[*idx] = byte; // 保存字节到缓冲区数组
            *idx += 1;
        // 超出缓冲区长度
        } else {
            *idx = 0;
            let _ = write!(uart, "ERR\r\n");
        }
    }

    received
}

// 处理一行输入，计算结果并发送回串口
fn process_line(uart: &mut Uart1, line: &[u8]) {
    match calculate(line) {
        Some(result) => {
            write_bytes(uart, line);
            let _ = write!(uart, "={}\r\n", result);
        }
        None => {
            let _ = write!(uart, "ERR\r\n");
        }
    }
}

// 判断是否是应该忽略的边界字节（空格、制表符、引号）
fn is_ignored_edge_byte(byte: u8) -> bool {
    byte == b' ' || byte == b'\t' || byte == b'"'
}

// 计算表达式结果
fn calculate(line: &[u8]) -> Option<i32> {
    let mut left: i32 = 0;
    let mut right: i32 = 0;
    let mut operator = 0u8;
    let mut idx = 0usize;
    let mut has_left = false;
    let mut has_right = false;

    if line.is_empty() {
        return None;
    }

    // 跳过开头的空格、制表符和引号
    while idx < line.len() && is_ignored_edge_byte(line[idx]) {
        idx += 1;
    }

    // 解析左操作数
    while idx < line.len() {
        let byte = line[idx];

        // 数字字符
        if byte >= b'0' && byte <= b'9' {
            has_left = true;
            left = left * 10 + i32::from(byte - b'0');
            idx += 1;
        // 操作符
        } else {
            operator = byte;
            idx += 1;
            break;
        }
    }

    // 解析右操作数
    while idx < line.len() {
        let byte = line[idx];

        if byte >= b'0' && byte <= b'9' {
            has_right = true;
            right = right * 10 + i32::from(byte - b'0');
            idx += 1;
        } else if byte == b'=' || byte == b'?' || is_ignored_edge_byte(byte) {
            break;
        } else {
            return None;
        }
    }

    // 兼容实验要求中的 "123+4000=?"，也兼容只输入 "123+400"
    while idx < line.len() {
        let byte = line[idx];

        if byte == b'=' || byte == b'?' || is_ignored_edge_byte(byte) {
            idx += 1;
        } else {
            return None;
        }
    }

    if !has_left || !has_right || operator == 0 {
        return None;
    }

    match operator {
        b'+' => Some(left + right),
        b'-' => Some(left - right),
        b'*' => Some(left * right),
        b'/' => {
            if right == 0 {
                None
            } else {
                Some(left / right)
            }
        }
        b'\\' => {
            if right == 0 {
                None
            } else {
                Some(left / right)
            }
        }
        b'%' => {
            if right == 0 {
                None
            } else {
                Some(left % right)
            }
        }
        b'&' => Some(left & right),
        b'|' => Some(left | right),
        _ => None,
    }
}

// 把输入的字节逐个发回去
fn write_bytes(uart: &mut Uart1, data: &[u8]) {
    let mut idx = 0usize;

    while idx < data.len() {
        uart.write_byte(data[idx]);
        idx += 1;
    }
}

#[entry]
fn main() -> ! {
    let dp = pac::Peripherals::take().unwrap(); // 获取外设访问权限

    setup_clock_gpio_usart(&dp);

    let mut uart = Uart1 { usart: dp.USART1 };

    let mut key0 = Button::new(read_key0_pressed(&dp.GPIOC));
    let mut key1 = Button::new(read_key1_pressed(&dp.GPIOA));
    let mut wkup = Button::new(read_wkup_pressed(&dp.GPIOA));

    let mut input_buffer = [0u8; INPUT_BUFFER_LEN]; // 串口输入缓冲区
    let mut input_bit = 0usize; // 当前输入的字节数
    let mut input_idle_count = 0u16; // 串口空闲计数
    let mut key_sample_counter = 0u16; // 按键采样计数

    let _ = writeln!(uart, "Rust serial ready\r");

    loop {
        // 处理串口输入
        if handle_serial_input(&mut uart, &mut input_buffer, &mut input_bit) { // 收到了串口数据
            input_idle_count = 0;
        } else if input_bit > 0 { // 缓冲区已有数据
            if input_idle_count < WAIT_SERIAL {
                input_idle_count += 1;
            /*
             处理串口工具无法发送\r\n的情况
            */
            } else { // 空闲时间超过阈值，即使没收到回车换行，也处理当前输入
                process_line(&mut uart, &input_buffer[..input_bit]);
                input_bit = 0;
                input_idle_count = 0;
            }
        }

        // 处理按键状态
        if key_sample_counter >= KEY_SAMPL_FRQ {
            key_sample_counter = 0;

            if let Some(pressed) = key0.update(read_key0_pressed(&dp.GPIOC)) {
                send_key_message(&mut uart, "KEY0", pressed); // example:"KEY0 keydown\r\n" or "KEY0 keyup\r\n"
            }

            if let Some(pressed) = key1.update(read_key1_pressed(&dp.GPIOA)) {
                send_key_message(&mut uart, "KEY1", pressed);
            }

            if let Some(pressed) = wkup.update(read_wkup_pressed(&dp.GPIOA)) {
                send_key_message(&mut uart, "WK_UP", pressed);
            }
        }

        key_sample_counter += 1;
    }
}
