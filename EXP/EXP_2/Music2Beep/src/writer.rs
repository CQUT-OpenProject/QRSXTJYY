use std::fs::File;
use std::io::{BufWriter, Write};
use std::path::Path;

const FIXED_SYMBOL: &str = "music_data";

fn escape_c_string(input: &str) -> String {
    // 仅转义 C 字符串中必须处理的字符，保证头部注释可安全嵌入。
    let mut out = String::with_capacity(input.len());
    for ch in input.chars() {
        match ch {
            '\\' => out.push_str("\\\\"),
            '"' => out.push_str("\\\""),
            _ => out.push(ch),
        }
    }
    out
}

pub fn write_c_data(
    path: &Path,
    notes: &[(u32, u32)],
    source_name: &str,
) -> Result<(), std::io::Error> {
    // 直接输出为 const char[]，固件可按文本行逐行解析。
    let file = File::create(path)?;
    let mut writer = BufWriter::new(file);

    let escaped_source = escape_c_string(source_name);

    writeln!(writer, "#include <stdint.h>")?;
    writeln!(writer)?;
    writeln!(writer, "const char {}[] =", FIXED_SYMBOL)?;
    writeln!(writer, "    \"# Music2Beep Generated\\n\"")?;
    writeln!(writer, "    \"# Source: {}\\n\"", escaped_source)?;

    // 总时长累加在长音频下可能超过 u32。
    let total_us: u64 = notes.iter().map(|&(_, d)| d as u64).sum();
    writeln!(
        writer,
        "    \"# Total Duration: {:.2}s\\n\"",
        total_us as f64 / 1_000_000.0
    )?;
    writeln!(writer, "    \"# Format: Frequency(Hz) Duration(us)\\n\"")?;

    // 每行一个音符片段：频率(Hz) + 持续时间(us)。
    for &(freq, dur) in notes {
        writeln!(writer, "    \"{} {}\\n\"", freq, dur)?;
    }
    writeln!(writer, "    ;")?;
    writeln!(writer)?;
    writeln!(
        writer,
        "const unsigned int {}_len = (unsigned int)(sizeof({}) - 1U);",
        FIXED_SYMBOL, FIXED_SYMBOL
    )?;

    Ok(())
}
