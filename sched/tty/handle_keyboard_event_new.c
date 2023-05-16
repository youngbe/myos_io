#define UNVISIBLE_BUF_SIZE 0x200000
char unvisible_buf[UNVISIBLE_BUF_SIZE];
size_t unvisible_used = 0;
size_t unvisible_i = 0;

void handle_keyboard_code()
{
    // 将获取到的键盘事件数组转化为等效的 偏移ascii数组（字符串）
    char str[STR_BUF_SIZE];
    size_t str_size = 0;
    static bool is_cap = false;
    for (size_t i = 0; i < key_codes_num; ++i) {
        // 按钮
        const uint16_t key = key_codes[i] >> 1;
        const bool is_press = (key_codes[i] & 1) == 1;
        if (key == CAP && is_press && !keymap[CAP])
            // 如果是按下大写的情况
            is_cap = !is_cap;

        // 更新每个按键的按下/释放情况
        keymap[key] = is_press;
        if (!is_press)
            // 如果仅是按键释放的话，不用做什么
            continue;

        if (key == BACK && is_press) {
            // backspace按下
            if (unvisible_i == 0)
                continue;
            --unvisible_i;
            --unvisible_used;
            str[str_size++] = '\b';
            str[str_size++] = '\x7f';
            continue;
        } else if (key == CUR_LEFT && is_press) {
            // 向左方向键按下
            if (unvisible_i == 0)
                continue;
            --unvisible_i;
            str[str_size++] = '\b';
            continue;
        } else if (key == CUR_RIGHT && is_press) {
            // 向右方向键按下
            if (unvisible_i == unvisible_used)
                continue;
            str[str_size++] = unvisible_buf[unvisible_i];
            ++unvisible_i;
            continue;
        } else if (key == ENTER && is_press) {
            // 回车键按下

        }

        char temp_c;
        if (is_cap && (keymap[LEFT_SHIFT] || keymap[RIGHT_SHIFT]))
            temp_c = to_ascii_cap_shift[key];
        else if (!is_cap && (keymap[LEFT_SHIFT] || keymap[RIGHT_SHIFT]))
            temp_c = to_ascii_shift[key];
        else if (is_cap && !(keymap[LEFT_SHIFT] || keymap[RIGHT_SHIFT]))
            temp_c = to_ascii_cap[key];
        else
            temp_c = to_ascii[key];
        if (temp_c != '\0')
            c[c_num++] = temp_c;
    }
}
