## 嵌入式系统及应用实验

> [!NOTE]
> 1. 仓库内提交的实验均为 CMake 工程。若需 Keil 工程，请前往 `resources/` 文件夹下载工程模板并自行配置。
> 2. 实验报告及部分工程（如 Rust 与 C 共存的）使用智能体辅助编写，可能存在不准确的情况。所有内容仅供参考。

### 实验内容

- `EXP_1`：摩尔斯灯光输出实验
- `EXP_2`：GPIO-蜂鸣器发声实验
- `EXP_3`：GPIO输入与外中断实验
- `EXP_4`：USART串行通信实验

### 文件夹结构

- `resources/`：实验指导书、工程模板等
- `utils/`：标准库函数、烧录配置文件

### 使用 C 编写的工程

#### 编译

进入对应实验工程目录：

```bash
cmake -G "Unix Makefiles" -S . -B build/manual-arm-debug -DCMAKE_BUILD_TYPE=Debug -DCMAKE_TOOLCHAIN_FILE=cmake/gcc-arm-none-eabi.cmake
cd build/manual-arm-debug
cmake --build .
```

#### 烧录（OpenOCD）

进入 `elf` 文件的构建目录：

```bash
openocd -f ../../../utils/c/tools/openocd/STM32F103RCT6.cfg -c "program SERIAL_TRANSMISSION.elf verify reset exit"
```

#### 串口烧录（stm32flash）

进入生成 `elf` 文件的构建目录，并把 `elf` 转成 `bin`：

```bash
arm-none-eabi-objcopy -O binary SERIAL_TRANSMISSION.elf SERIAL_TRANSMISSION.bin
```

连接开发板时，使用板载 `USB_232` 口，并确认 `P4` 处的 `RXD/PA9`、`TXD/PA10` 已接通。

查看串口设备名：

```bash
ls /dev/tty.usb*
```

测试是否连上 STM32 串口 Bootloader：

```bash
stm32flash /dev/tty.usbserial-XXXX
```

确认连接正常后，再执行写入、校验并跳转运行：

```bash
stm32flash -b 115200 -w SERIAL_TRANSMISSION.bin -v -g 0x0 /dev/tty.usbserial-XXXX
```

### 使用 Rust 编写的工程

#### 编译与烧录

```bash
rustup target add thumbv7m-none-eabi
cargo build --release
cargo embed --release
```

若未安装 `cargo-embed`，先执行：

```bash
cargo install cargo-embed --locked
```
