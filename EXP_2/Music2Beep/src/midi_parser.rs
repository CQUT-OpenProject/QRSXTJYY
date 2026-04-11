use midly::{MetaMessage, MidiMessage, Smf, TrackEventKind};
use std::collections::BTreeSet;
use std::fs;
use std::path::Path;

const DEFAULT_US_PER_BEAT: u64 = 500_000;
const MIN_MELODY_KEY: u8 = 55;

#[derive(Clone, Debug)]
struct Event {
    tick: u64,
    kind: EventKind,
}

#[derive(Clone, Debug)]
enum EventKind {
    NoteOn { key: u8, vel: u8 },
    NoteOff { key: u8 },
    Tempo { us_per_beat: u32 },
}

pub fn parse_midi(path: &Path) -> Result<Vec<(u32, u32)>, String> {
    // 读取并解析标准 MIDI 文件结构。
    let data = fs::read(path).map_err(|e| e.to_string())?;
    let smf = Smf::parse(&data).map_err(|e| e.to_string())?;

    // 仅支持拍号刻度（ticks per beat），暂不支持 SMPTE 时间码。
    let ticks_per_beat = match smf.header.timing {
        midly::Timing::Metrical(t) => t.as_int() as u64,
        _ => return Err("Unsupported timecode format (SMPTE not supported yet)".into()),
    };
    if ticks_per_beat == 0 {
        return Err("Invalid MIDI ticks-per-beat: 0".into());
    }

    let mut events = Vec::new();

    // 把每个轨道的 delta time 累加成绝对 tick，并收集关心的事件。
    for track in smf.tracks {
        let mut absolute_tick: u64 = 0;
        for ev in track {
            absolute_tick += ev.delta.as_int() as u64;
            match ev.kind {
                TrackEventKind::Midi {
                    channel,
                    message: MidiMessage::NoteOn { key, vel },
                } => {
                    // GM 约定 channel 10(索引9) 常用于打击乐，这里直接忽略。
                    if channel.as_int() != 9 {
                        events.push(Event {
                            tick: absolute_tick,
                            kind: EventKind::NoteOn {
                                key: key.as_int(),
                                vel: vel.as_int(),
                            },
                        });
                    }
                }
                TrackEventKind::Midi {
                    channel,
                    message: MidiMessage::NoteOff { key, vel: _ },
                } => {
                    if channel.as_int() != 9 {
                        events.push(Event {
                            tick: absolute_tick,
                            kind: EventKind::NoteOff { key: key.as_int() },
                        });
                    }
                }
                TrackEventKind::Meta(MetaMessage::Tempo(t)) => {
                    // 速度变化作为全局时间换算依据，后续按时间顺序生效。
                    events.push(Event {
                        tick: absolute_tick,
                        kind: EventKind::Tempo {
                            us_per_beat: t.as_int(),
                        },
                    });
                }
                _ => {}
            }
        }
    }

    events.sort_by_key(|e| e.tick);

    let mut current_tick = 0_u64;
    let mut us_per_beat = DEFAULT_US_PER_BEAT;
    let mut active_notes = BTreeSet::<u8>::new();
    let mut time_key = Vec::<(u8, u64)>::new();

    // 经典旋律抽取：在任意时间片，仅保留当前按下音符中的最高音。
    for ev in events {
        let tick_delta = ev.tick.saturating_sub(current_tick);
        if tick_delta > 0 {
            let us_delta = (tick_delta * us_per_beat) / ticks_per_beat;
            let selected_key = active_notes.iter().next_back().copied().unwrap_or(0);
            time_key.push((selected_key, us_delta));
            current_tick = ev.tick;
        }

        match ev.kind {
            EventKind::Tempo { us_per_beat: t } => {
                us_per_beat = t as u64;
            }
            EventKind::NoteOn { key, vel } => {
                // MIDI 中 NoteOn velocity=0 等价 NoteOff。
                if vel > 0 && key >= MIN_MELODY_KEY {
                    active_notes.insert(key);
                } else {
                    active_notes.remove(&key);
                }
            }
            EventKind::NoteOff { key } => {
                active_notes.remove(&key);
            }
        }
    }

    merge_time_key_segments(time_key)
}

fn key_to_freq(key: u8) -> u32 {
    // 12 平均律：A4(69) = 440Hz。
    let freq = 440.0_f32 * 2.0_f32.powf((key as f32 - 69.0_f32) / 12.0_f32);
    freq.round() as u32
}

fn merge_time_key_segments(time_key: Vec<(u8, u64)>) -> Result<Vec<(u32, u32)>, String> {
    let mut merged = Vec::<(u32, u64)>::new();

    // 把相邻同频率片段合并，降低输出体积并减少播放切换抖动。
    for (key, dur) in time_key {
        let freq = if key > 0 { key_to_freq(key) } else { 0 };
        if let Some(last) = merged.last_mut()
            && last.0 == freq
        {
            last.1 += dur;
        } else {
            merged.push((freq, dur));
        }
    }

    let mut out = Vec::with_capacity(merged.len());
    for (freq, dur) in merged {
        // 固件侧接口使用 u32 时长，这里提前做溢出校验。
        let dur_u32 = u32::try_from(dur).map_err(|_| {
            format!(
                "Segment duration overflow: {} us does not fit into u32",
                dur
            )
        })?;
        out.push((freq, dur_u32));
    }

    Ok(out)
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn key_to_freq_works_for_a4() {
        assert_eq!(key_to_freq(69), 440);
    }

    #[test]
    fn merge_adjacent_same_key_segments() {
        let out = merge_time_key_segments(vec![(60, 100_000), (60, 50_000), (62, 80_000)]).unwrap();
        assert_eq!(out, vec![(262, 150_000), (294, 80_000)]);
    }
}
