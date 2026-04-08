#include "morse_app.h"

#include "stm32f10x.h"

typedef struct {
    const char *pattern;
    uint32_t inter_word_gap_ms;
} MorseWord;

static const uint32_t DOT_MS = 200U;
static const uint32_t DASH_MS = DOT_MS * 3U;
static const uint32_t SYMBOL_GAP_MS = DOT_MS;
static const uint32_t LETTER_GAP_MS = DOT_MS * 3U;
static const uint32_t WORD_GAP_MS = DOT_MS * 7U;
static const uint32_t POLL_MS = 20U;
static const uint32_t DEBOUNCE_MS = 20U;

static const MorseWord WORD_SOS[] = {
    {"...", LETTER_GAP_MS},
    {"---", LETTER_GAP_MS},
    {"...", 0U},
};

static const MorseWord WORD_YES[] = {
    {"-.--", LETTER_GAP_MS},
    {".", LETTER_GAP_MS},
    {"...", 0U},
};

static const MorseWord WORD_NO[] = {
    {"-.", LETTER_GAP_MS},
    {"---", 0U},
};

static void delay_units(uint32_t time_ms)
{
    board_delay_ms(time_ms);
}

static void emit_symbol(char symbol)
{
    board_led_set(true);
    delay_units(symbol == '.' ? DOT_MS : DASH_MS);
    board_led_set(false);
    delay_units(SYMBOL_GAP_MS);
}

static void emit_letter(const MorseWord *word)
{
    const char *cursor = word->pattern;

    while (*cursor != '\0') {
        emit_symbol(*cursor++);
    }

    if (word->inter_word_gap_ms > 0U) {
        delay_units(word->inter_word_gap_ms - SYMBOL_GAP_MS);
    }
}

static void emit_message(const MorseWord *words, uint32_t word_count)
{
    uint32_t i;

    for (i = 0U; i < word_count; ++i) {
        emit_letter(&words[i]);
    }

    delay_units(WORD_GAP_MS);
}

static void wait_for_key_release(bool (*key_pressed)(void))
{
    while (key_pressed()) {
        delay_units(POLL_MS);
    }
}

static bool sample_key(bool (*key_pressed)(void))
{
    if (!key_pressed()) {
        return false;
    }

    delay_units(DEBOUNCE_MS);
    return key_pressed();
}

void morse_app_run(void)
{
    SystemCoreClockUpdate();
    board_init();
    board_led_set(false);

    while (1) {
        if (sample_key(board_key_up_pressed)) {
            emit_message(WORD_SOS, sizeof(WORD_SOS) / sizeof(WORD_SOS[0]));
            wait_for_key_release(board_key_up_pressed);
        } else if (sample_key(board_key1_pressed)) {
            emit_message(WORD_YES, sizeof(WORD_YES) / sizeof(WORD_YES[0]));
            wait_for_key_release(board_key1_pressed);
        } else if (sample_key(board_key0_pressed)) {
            emit_message(WORD_NO, sizeof(WORD_NO) / sizeof(WORD_NO[0]));
            wait_for_key_release(board_key0_pressed);
        } else {
            delay_units(POLL_MS);
        }
    }
}
