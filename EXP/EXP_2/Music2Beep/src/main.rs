use clap::Parser;
use std::path::PathBuf;

mod midi_parser;
mod writer;

#[derive(Parser, Debug)]
#[command(author, version, about, long_about = None)]
pub struct Args {
    /// 输入 MIDI 文件路径
    pub input: PathBuf,

    /// 输出 C 数据文件路径
    #[arg(short, long)]
    pub output: Option<PathBuf>,
}

fn main() {
    // 解析命令行参数，确定输入 MIDI 与输出路径。
    let args = Args::parse();

    println!("Processing MIDI file: {}", args.input.display());
    // 把 MIDI 事件转换成 (频率Hz, 时长us) 序列。
    let processed_notes = match midi_parser::parse_midi(&args.input) {
        Ok(notes) => notes,
        Err(e) => {
            eprintln!("Failed to parse MIDI: {}", e);
            std::process::exit(1);
        }
    };

    // 未指定输出时，默认使用与输入同名的 .c 文件。
    let output_path = args.output.clone().unwrap_or_else(|| {
        let mut p = args.input.clone();
        p.set_extension("c");
        p
    });

    println!("Writing to {}...", output_path.display());
    // 输出为 STM32 工程可直接包含的 C 字符串数据。
    match writer::write_c_data(
        &output_path,
        &processed_notes,
        args.input.to_string_lossy().as_ref(),
    ) {
        Ok(_) => println!("Success!"),
        Err(e) => {
            eprintln!("Failed to write output: {}", e);
            std::process::exit(1);
        }
    }
}
