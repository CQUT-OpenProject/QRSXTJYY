# EXP_4 SERIAL_TRANSMISSION

> [!NOTE]
> 由于 debug 版本运行速度较慢，在 115200 波特率下可能会丢失连续接收的串口数据。

```bash
cargo embed --release
```

## 串口测试

打开 CH340 对应串口，配置为：

* 波特率：115200
* 数据位：8
* 停止位：1
* 无校验（8N1）

按键消息仅在按键状态发生变化时发送：

```text
KEY0 keydown
KEY0 keyup
KEY1 keydown
KEY1 keyup
WK_UP keydown
WK_UP keyup
```

串口还实现了简单计算器功能，每次输入一行简单表达式，例如：

```text
123+400
```

开发板返回结果：

```text
123+400=523
```
