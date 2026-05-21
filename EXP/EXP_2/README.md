# Music2Beep

`Music2Beep` 用于把 MIDI 文件转换为 STM32 蜂鸣器可直接播放的 C 数据文件（`music_data`）。

## 基本命令

```bash
cargo run -- <输入.mid> [-o <输出.c>]
```

示例：

```bash
cargo run -- ./ハルジオン.mid -o ../BEEP-BEEP/core/src/music_data.c
```

## 参数说明

- `<输入.mid>`：必填，输入 MIDI 文件路径。
- `-o, --output <输出.c>`：可选，输出 C 文件路径；不填时默认与输入同名 `.c`。

## 内置默认策略（classic）

- 多音同时存在时按当前最高活跃音符输出，保留原始切分细节。
- 过滤过低音高（`key < 55`）以避免蜂鸣器低频可听性差的问题。
- 输出格式保持 `Frequency(Hz) Duration(us)`，可直接用于 `BEEP-BEEP` 工程。

生成后重新编译 `BEEP-BEEP` 工程并下载到板子即可播放。
