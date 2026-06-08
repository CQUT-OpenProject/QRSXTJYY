#include "board.h"

#define INPUT_BUFFER_LEN 40U
#define SEND_INTERVAL_MS 100U

typedef struct {
    uint16_t year;
    uint8_t month;
    uint8_t day;
    uint8_t week;
} DAT;

typedef struct {
    uint8_t hour;
    uint8_t minute;
    uint8_t second;
    uint16_t ms;
} TIME;

typedef struct {
    DAT dat;
    TIME time;
} NOW;

static bool is_leap_year(uint16_t year)
{
    return ((year % 4U == 0U) && (year % 100U != 0U)) || (year % 400U == 0U);
}

static uint8_t month_days(uint16_t year, uint8_t month)
{
    switch (month) {
        case 1U:
        case 3U:
        case 5U:
        case 7U:
        case 8U:
        case 10U:
        case 12U:
            return 31U;
        case 4U:
        case 6U:
        case 9U:
        case 11U:
            return 30U;
        case 2U:
            if (is_leap_year(year)) {
                return 29U;
            }
            return 28U;
        default:
            return 30U;
    }
}

static uint8_t calc_week(uint16_t year, uint8_t month, uint8_t day)
{
    uint32_t days = 0U;
    uint16_t y = 2000U;
    uint8_t m = 1U;

    while (y < year) {
        if (is_leap_year(y)) {
            days += 366U;
        } else {
            days += 365U;
        }
        y++;
    }

    while (m < month) {
        days += month_days(year, m);
        m++;
    }

    days += day - 1U;

    /* 2000-01-01 是星期六，这里 0 表示 SUN，6 表示 SAT。 */
    return (uint8_t)((6U + days) % 7U);
}

static const char *week_name(uint8_t week)
{
    switch (week) {
        case 0U:
            return "SUN";
        case 1U:
            return "MON";
        case 2U:
            return "TUE";
        case 3U:
            return "WED";
        case 4U:
            return "THU";
        case 5U:
            return "FRI";
        case 6U:
            return "SAT";
        default:
            return "UNK";
    }
}

static void now_init(NOW *now)
{
    now->dat.year = 2026U;
    now->dat.month = 5U;
    now->dat.day = 31U;
    now->dat.week = calc_week(now->dat.year, now->dat.month, now->dat.day);
    now->time.hour = 0U;
    now->time.minute = 0U;
    now->time.second = 0U;
    now->time.ms = 0U;
}

static void now_add_day(NOW *now)
{
    now->dat.day++;
    now->dat.week = (uint8_t)((now->dat.week + 1U) % 7U);

    if (now->dat.day > month_days(now->dat.year, now->dat.month)) {
        now->dat.day = 1U;
        now->dat.month++;
    }

    if (now->dat.month > 12U) {
        now->dat.month = 1U;
        now->dat.year++;
    }
}

static void now_add_second(NOW *now)
{
    now->time.second++;

    if (now->time.second >= 60U) {
        now->time.second = 0U;
        now->time.minute++;
    }

    if (now->time.minute >= 60U) {
        now->time.minute = 0U;
        now->time.hour++;
    }

    if (now->time.hour >= 24U) {
        now->time.hour = 0U;
        now_add_day(now);
    }
}

static void now_tick_ms(NOW *now)
{
    now->time.ms++;

    if (now->time.ms >= 1000U) {
        now->time.ms = 0U;
        now_add_second(now);
    }
}

static void now_set(NOW *now, uint16_t year, uint8_t month, uint8_t day, uint8_t hour, uint8_t minute, uint8_t second, uint16_t ms)
{
    now->dat.year = year;
    now->dat.month = month;
    now->dat.day = day;
    now->dat.week = calc_week(year, month, day);
    now->time.hour = hour;
    now->time.minute = minute;
    now->time.second = second;
    now->time.ms = ms;
}

static void write_uint_padded(uint16_t data, uint8_t width)
{
    char buf[5];
    uint8_t idx = width;

    if (width > sizeof(buf)) {
        width = sizeof(buf);
        idx = width;
    }

    while (idx > 0U) {
        idx--;
        buf[idx] = (char)('0' + (data % 10U));
        data /= 10U;
    }

    idx = 0U;
    while (idx < width) {
        board_usart1_write_byte((uint8_t)buf[idx]);
        idx++;
    }
}

static void send_now(const NOW *now)
{
    write_uint_padded(now->dat.year, 4U);
    board_usart1_write_byte('-');
    write_uint_padded(now->dat.month, 2U);
    board_usart1_write_byte('-');
    write_uint_padded(now->dat.day, 2U);
    board_usart1_write_byte(',');
    board_usart1_write_str(week_name(now->dat.week));
    board_usart1_write_byte(',');
    write_uint_padded(now->time.hour, 2U);
    board_usart1_write_byte(':');
    write_uint_padded(now->time.minute, 2U);
    board_usart1_write_byte(':');
    write_uint_padded(now->time.second, 2U);
    board_usart1_write_byte('.');
    write_uint_padded(now->time.ms, 3U);
    board_usart1_write_str("\r\n");
}

static bool read_number(const uint8_t *data, uint32_t len, uint32_t *idx, uint8_t width, uint16_t *result)
{
    uint8_t count = 0U;
    uint16_t value = 0U;

    if ((*idx + width) > len) {
        return false;
    }

    while (count < width) {
        uint8_t byte = data[*idx];

        if (byte < '0' || byte > '9') {
            return false;
        }

        value = (uint16_t)(value * 10U + (uint16_t)(byte - '0'));
        (*idx)++;
        count++;
    }

    *result = value;
    return true;
}

static bool expect_byte(const uint8_t *data, uint32_t len, uint32_t *idx, uint8_t byte)
{
    if (*idx >= len || data[*idx] != byte) {
        return false;
    }

    (*idx)++;
    return true;
}

static bool check_datetime(uint16_t year, uint8_t month, uint8_t day, uint8_t hour, uint8_t minute, uint8_t second, uint16_t ms)
{
    if (month < 1U || month > 12U) {
        return false;
    }

    if (day < 1U || day > month_days(year, month)) {
        return false;
    }

    if (hour > 23U || minute > 59U || second > 59U || ms > 999U) {
        return false;
    }

    return true;
}

static bool parse_set_time(const uint8_t *data, uint32_t len, NOW *now)
{
    uint32_t idx = 0U;
    uint16_t year;
    uint16_t month;
    uint16_t day;
    uint16_t hour;
    uint16_t minute;
    uint16_t second;
    uint16_t ms;

    while (idx < len && (data[idx] == ' ' || data[idx] == '\t' || data[idx] == '"')) {
        idx++;
    }

    if (!expect_byte(data, len, &idx, '>')) {
        return false;
    }

    if (!read_number(data, len, &idx, 4U, &year)) {
        return false;
    }
    if (!expect_byte(data, len, &idx, '-')) {
        return false;
    }
    if (!read_number(data, len, &idx, 2U, &month)) {
        return false;
    }
    if (!expect_byte(data, len, &idx, '-')) {
        return false;
    }
    if (!read_number(data, len, &idx, 2U, &day)) {
        return false;
    }

    if (idx >= len || (data[idx] != ' ' && data[idx] != ',')) {
        return false;
    }
    idx++;

    if (!read_number(data, len, &idx, 2U, &hour)) {
        return false;
    }
    if (!expect_byte(data, len, &idx, ':')) {
        return false;
    }
    if (!read_number(data, len, &idx, 2U, &minute)) {
        return false;
    }
    if (!expect_byte(data, len, &idx, ':')) {
        return false;
    }
    if (!read_number(data, len, &idx, 2U, &second)) {
        return false;
    }
    if (!expect_byte(data, len, &idx, '.')) {
        return false;
    }
    if (!read_number(data, len, &idx, 3U, &ms)) {
        return false;
    }

    while (idx < len && (data[idx] == ' ' || data[idx] == '\t' || data[idx] == '"')) {
        idx++;
    }

    if (idx != len) {
        return false;
    }

    if (!check_datetime(year, (uint8_t)month, (uint8_t)day, (uint8_t)hour, (uint8_t)minute, (uint8_t)second, ms)) {
        return false;
    }

    now_set(now, year, (uint8_t)month, (uint8_t)day, (uint8_t)hour, (uint8_t)minute, (uint8_t)second, ms);
    return true;
}

static void process_line(const uint8_t *data, uint32_t len, NOW *now)
{
    if (parse_set_time(data, len, now)) {
        board_usart1_write_str("OK\r\n");
        send_now(now);
    } else {
        board_usart1_write_str("ERR\r\n");
    }
}

static void handle_serial_input(NOW *now, uint8_t *input, uint32_t *input_len)
{
    uint8_t data;

    while (board_usart1_read_byte(&data)) {
        if (data == '\r' || data == '\n') {
            if (*input_len > 0U) {
                process_line(input, *input_len, now);
                *input_len = 0U;
            }
        } else if (*input_len < INPUT_BUFFER_LEN) {
            input[*input_len] = data;
            (*input_len)++;
        } else {
            *input_len = 0U;
            board_usart1_write_str("ERR\r\n");
        }
    }
}

int main(void)
{
    NOW now;
    uint8_t input[INPUT_BUFFER_LEN];
    uint32_t input_len = 0U;
    uint16_t send_count = 0U;

    board_init();
    now_init(&now);

    send_now(&now);

    while (1) {
        handle_serial_input(&now, input, &input_len);

        if (board_tim2_1ms_arrived()) {
            now_tick_ms(&now);
            send_count++;

            if (send_count >= SEND_INTERVAL_MS) {
                send_count = 0U;
                send_now(&now);
            }
        }
    }
}
