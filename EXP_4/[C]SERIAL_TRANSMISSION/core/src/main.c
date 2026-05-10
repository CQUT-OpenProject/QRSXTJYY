#include "board.h"

#define INPUT_BUFFER_LEN 64U
#define DEBOUNCE_COUNT 4U
#define WAIT_SERIAL 40000U

/* 保存按键稳定状态、上一次原始读数和消抖计数。 */
typedef struct {
    bool stable;
    bool last_raw;
    uint8_t count;
} ButtonState;

/* 把十进制整数转成字符，一个字节一个字节发到串口。 */
static void board_usart1_write_int(int32_t value)
{
    char buf[16];
    uint32_t idx = 0U;
    uint32_t pos;
    uint32_t data;

    if (value == 0) {
        board_usart1_write_byte('0');
        return;
    }

    if (value < 0) {
        board_usart1_write_byte('-');
        data = (uint32_t)(-value);
    } else {
        data = (uint32_t)value;
    }

    while (data > 0U && idx < sizeof(buf)) {
        buf[idx] = (char)('0' + (data % 10U));
        data /= 10U;
        idx++;
    }

    pos = idx;
    while (pos > 0U) {
        pos--;
        board_usart1_write_byte((uint8_t)buf[pos]);
    }
}

static bool is_ignored_edge_byte(uint8_t data)
{
    return data == ' ' || data == '\t' || data == '"';
}

static void send_key_message(const char *name, bool pressed)
{
    /* 只发实验要求里的 keydown / keyup 文案。 */
    board_usart1_write_str(name);
    if (pressed) {
        board_usart1_write_str(" keydown\r\n");
    } else {
        board_usart1_write_str(" keyup\r\n");
    }
}

static bool button_update(ButtonState *state, bool raw)
{
    /* 连续读到同一个原始状态就累加计数。 */
    if (raw == state->last_raw) {
        if (state->count < DEBOUNCE_COUNT) {
            state->count++;
        }
    } else {
        /* 原始状态变了，重新开始计数。 */
        state->last_raw = raw;
        state->count = 0U;
    }

    /* 连续多次读到同一状态后，才认为按键真的稳定变化了。 */
    if (state->count >= DEBOUNCE_COUNT && state->stable != raw) {
        state->stable = raw;
        return true;
    }

    return false;
}

static void send_line_and_result(const uint8_t *data, uint32_t len, int32_t result)
{
    uint32_t idx = 0U;

    /* 先把输入的表达式原样回发。 */
    while (idx < len) {
        board_usart1_write_byte(data[idx]);
        idx++;
    }

    /* 再补上等号和结果。 */
    board_usart1_write_byte('=');
    board_usart1_write_int(result);
    board_usart1_write_str("\r\n");
}

/* 去掉开头和结尾没用的字符，再去掉结尾的 =、? 或 =?。 */
static uint32_t normalize_line(const uint8_t *input, uint32_t input_len, uint8_t *output)
{
    uint32_t start = 0U;
    uint32_t end = input_len;
    uint32_t out_len;
    uint32_t idx = 0U;

    while (start < input_len && is_ignored_edge_byte(input[start])) {
        start++;
    }

    while (end > start && is_ignored_edge_byte(input[end - 1U])) {
        end--;
    }

    out_len = end - start;

    /* 先把整理后的输入拷贝到新缓冲区。 */
    while (idx < out_len) {
        output[idx] = input[start + idx];
        idx++;
    }

    if (out_len >= 2U && output[out_len - 2U] == '=' && output[out_len - 1U] == '?') {
        out_len -= 2U;
    } else if (out_len >= 1U && (output[out_len - 1U] == '=' || output[out_len - 1U] == '?')) {
        out_len -= 1U;
    }

    while (out_len > 0U && is_ignored_edge_byte(output[out_len - 1U])) {
        out_len--;
    }

    return out_len;
}

static bool calculate(const uint8_t *data, uint32_t len, int32_t *result)
{
    int32_t left = 0;
    int32_t right = 0;
    uint32_t idx = 0U;
    uint32_t right_start;
    uint8_t op = 0U;

    if (len == 0U) {
        return false;
    }

    /* 先读取左操作数。 */
    while (idx < len && data[idx] >= '0' && data[idx] <= '9') {
        left = left * 10 + (int32_t)(data[idx] - '0');
        idx++;
    }

    if (idx == 0U || idx >= len) {
        return false;
    }

    op = data[idx];
    idx++;
    right_start = idx;

    /* 再读取右操作数。 */
    while (idx < len) {
        if (data[idx] < '0' || data[idx] > '9') {
            return false;
        }

        right = right * 10 + (int32_t)(data[idx] - '0');
        idx++;
    }

    if (right_start == idx) {
        return false;
    }

    /* 按操作符计算结果，除 0 和格式错误都返回 false。 */
    switch (op) {
        case '+':
            *result = left + right;
            return true;
        case '-':
            *result = left - right;
            return true;
        case '*':
            *result = left * right;
            return true;
        case '/':
        case '\\':
            if (right == 0) {
                return false;
            }
            *result = left / right;
            return true;
        case '%':
            if (right == 0) {
                return false;
            }
            *result = left % right;
            return true;
        case '&':
            *result = left & right;
            return true;
        case '|':
            *result = left | right;
            return true;
        default:
            return false;
    }
}

static void process_line(const uint8_t *data, uint32_t len)
{
    int32_t result = 0;
    uint8_t normalized[INPUT_BUFFER_LEN];
    uint32_t normalized_len;

    /* 先把 123+4000=? 这种格式整理成 123+4000。 */
    normalized_len = normalize_line(data, len, normalized);
    if (normalized_len == 0U) {
        board_usart1_write_str("ERR\r\n");
        return;
    }

    /* 算成功就回发结果，失败就回 ERR。 */
    if (calculate(normalized, normalized_len, &result)) {
        send_line_and_result(normalized, normalized_len, result);
    } else {
        board_usart1_write_str("ERR\r\n");
    }
}

int main(void)
{
    ButtonState key0;
    ButtonState key1;
    ButtonState wkup;
    uint8_t input[INPUT_BUFFER_LEN];
    uint32_t input_len = 0U;
    uint32_t input_idle_count = 0U;

    board_init();

    /* 记录上电时 3 个按键的初始状态，避免一开始乱发消息。 */
    key0.stable = board_key0_pressed();
    key0.last_raw = key0.stable;
    key0.count = 0U;

    key1.stable = board_key1_pressed();
    key1.last_raw = key1.stable;
    key1.count = 0U;

    wkup.stable = board_wkup_pressed();
    wkup.last_raw = wkup.stable;
    wkup.count = 0U;

    while (1) {
        uint8_t data;

        /* 按键状态有变化时，只发送一次消息。 */
        if (button_update(&key0, board_key0_pressed())) {
            send_key_message("KEY0", key0.stable);
        }

        if (button_update(&key1, board_key1_pressed())) {
            send_key_message("KEY1", key1.stable);
        }

        if (button_update(&wkup, board_wkup_pressed())) {
            send_key_message("WK_UP", wkup.stable);
        }

        /* 把串口里已经收到的字节全部读出来。 */
        while (board_usart1_read_byte(&data)) {
            input_idle_count = 0U;

            if (data == '\r' || data == '\n') {
                /* 收到回车换行后，处理一整行表达式。 */
                if (input_len > 0U) {
                    process_line(input, input_len);
                    input_len = 0U;
                }
            } else if (input_len < INPUT_BUFFER_LEN) {
                /* 普通字符先保存到缓冲区。 */
                input[input_len] = data;
                input_len++;
            } else {
                /* 缓冲区满了，说明这一行太长，直接报错并清空。 */
                input_len = 0U;
                input_idle_count = 0U;
                board_usart1_write_str("ERR\r\n");
            }
        }

        /*
         * 有些串口工具不会发回车换行。
         * 如果已经收到一部分数据，并且后面空闲了足够久，也按一整行处理。
         */
        if (input_len > 0U) {
            if (input_idle_count < WAIT_SERIAL) {
                input_idle_count++;
            } else {
                process_line(input, input_len);
                input_len = 0U;
                input_idle_count = 0U;
            }
        }

        /* 稍微延时，降低轮询频率。 */
        board_delay_ms(1U);
    }
}
