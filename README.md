## 嵌入式系统及应用

> [!NOTE]
> 1. 本仓库的 C 工程使用 CMake，部分实验另有 Rust 工程。若需 Keil 工程，可下载 `工程模板(教师提供).zip` 并自行配置
> 2. 实验报告及部分工程使用智能体辅助编写，可能存在不准确的情况，所有内容仅供参考

## 目录导航

- `CCP/`：综合课程设计
- `EXP/`：实验
- `resources/`：全课程共用的参考资料和教师提供的工程模板
- `EXP/resources/`：实验指导书及开发板资料
- `stm32f1-spl-platform/`：STM32F1 SPL 平台子模块

## 实验索引

- `EXP_1`：摩尔斯灯光输出实验
- `EXP_2`：GPIO-蜂鸣器发声实验
- `EXP_3`：GPIO输入与外中断实验
- `EXP_4`：USART 串行通信实验
- `EXP_5`：软件时钟日历实验

## 使用 C 编写的工程

### 编译

安装 CMake、Python 3、Git、Make 和包含 Newlib 的 Arm GNU 工具链。初始化子模块，然后进入对应实验工程目录：

```bash
git submodule update --init --recursive
cmake --preset Debug
cmake --build --preset Debug
```

各工程默认使用 STM32F103RCT6 和原有时钟配置；也可将预设中的 `Debug` 换成 `Release`。

### 烧录（OpenOCD）

在实验工程目录执行（将 `FIRMWARE` 替换为工程名）：

```bash
openocd -f stm32f1-spl-platform/tools/openocd/stlink.cfg -c "program build/platform/Debug/FIRMWARE.elf verify reset exit"
```

### 串口烧录（stm32flash）

进入生成 `elf` 文件的构建目录，并把 `elf` 转成 `bin`：

```bash
arm-none-eabi-objcopy -O binary *.elf SERIAL_TRANSMISSION.bin
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
stm32flash -b 115200 -w *SERIAL_TRANSMISSION*.bin -v -g 0x0 /dev/tty.usbserial-XXXX
```
