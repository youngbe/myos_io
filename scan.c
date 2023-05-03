#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

// 键盘有100个按钮
#define KEY_NUM 100

enum {
    A = 1, B, C, D, E, F, G, H, I, J, K, L, M, N, O, P, Q, R, S, T, U, V, W, X, Y, Z,
    F1, F2, F3, F4, F5, F6, F7, F8, F9, F10, F11, F12,
    _1, _2, _3, _4, _5, _6, _7, _8, _9, _0,
    __1, __2, __3, __4, __5, __6, __7, __8, __9, __0,
    ESC, PRTSC, SCRLK, PAUSE, INS, HOME, PAGE_UP, DEL, END, PAGE_DOWN,
    BACK_QUOTE, SUB, EQU, BACK,
    TAB, OPEN_BRACKET, CLOSE_BRACKET, BACKSLASH,
    CAP, SEMICOLON, QUOTE, ENTER,
    LEFT_SHIFT, COMMA, DOT, SLASH, RIGHT_SHIFT,
    LEFT_CTRL, LEFT_ALT, RIGHT_ALT, RIGHT_CTRL,
    CURSOR_UP, CURSOR_DOWN, CURSOR_LEFT, CURSOR_RIGHT,
    __NUM, __SLASH, __ASTERISK, __SUB, __ADD, __ENTER, __DOT
};

// 只有一个扫描码时，使用这个map
static const uint16_t ps2_set1_map[UINT8_MAX] = {
    [0x01] = (ESC << 1) | 1,
    [0x02] = (_1 << 1) | 1,
    [0x03] = (_2 << 1) | 1,
    [0x04] = (_3 << 1) | 1,
    [0x05] = (_4 << 1) | 1,
    [0x06] = (_5 << 1) | 1,
    [0x07] = (_6 << 1) | 1,
    [0x08] = (_7 << 1) | 1,
    [0x09] = (_8 << 1) | 1,
    [0x0a] = (_9 << 1) | 1,
    [0x0b] = (_0 << 1) | 1,
    [0x0c] = (SUB << 1) | 1,
    [0x0d] = (EQU << 1) | 1,
    [0x0e] = (BACK << 1) | 1,
    [0x0f] = (TAB << 1) | 1,
    [0x10] = (Q << 1) | 1,
    [0x11] = (W << 1) | 1,
    [0x12] = (E << 1) | 1,
    [0x13] = (R << 1) | 1,
    [0x14] = (T << 1) | 1,
    [0x15] = (Y << 1) | 1,
    [0x16] = (U << 1) | 1,
    [0x17] = (I << 1) | 1,
    [0x18] = (O << 1) | 1,
    [0x19] = (P << 1) | 1,
    [0x1a] = (OPEN_BRACKET << 1) | 1,
    [0x1b] = (CLOSE_BRACKET << 1) | 1,
    [0x1c] = (ENTER << 1) | 1,
    [0x1d] = (LEFT_CTRL << 1) | 1,
    [0x1e] = (A << 1) | 1,
    [0x1f] = (S << 1) | 1,
    [0x20] = (D << 1) | 1,
    [0x21] = (F << 1) | 1,
    [0x22] = (G << 1) | 1,
    [0x23] = (H << 1) | 1,
    [0x24] = (J << 1) | 1,
    [0x25] = (K << 1) | 1,
    [0x26] = (L << 1) | 1,
    [0x27] = (SEMICOLON << 1) | 1,
    [0x28] = (QUOTE << 1) | 1,
    [0x29] = (BACK_QUOTE << 1) | 1,
    [0x2a] = (LEFT_SHIFT << 1) | 1,
    [0x2b] = (BACKSLASH << 1) | 1,
    [0x2c] = (Z << 1) | 1,
    [0x2d] = (X << 1) | 1,
    [0x2e] = (C << 1) | 1,
    [0x2f] = (V << 1) | 1,
    [0x30] = (B << 1) | 1,
    [0x31] = (N << 1) | 1,
    [0x32] = (M << 1) | 1,
    [0x33] = (COMMA << 1) | 1,
    [0x34] = (DOT << 1) | 1,
    [0x35] = (SLASH << 1) | 1,
    [0x36] = (RIGHT_SHIFT << 1) | 1,
    [0x37] = (__ASTERISK << 1) | 1,
    [0x38] = (LEFT_ALT << 1) | 1,
    [0x39] = (SPACE << 1) | 1,
    [0x3a] = (CAP << 1) | 1,
    [0x3b] = (F1 << 1) | 1,
    [0x3c] = (F2 << 1) | 1,
    [0x3d] = (F3 << 1) | 1,
    [0x3e] = (F4 << 1) | 1,
    [0x3f] = (F5 << 1) | 1,
    [0x40] = (F6 << 1) | 1,
    [0x41] = (F7 << 1) | 1,
    [0x42] = (F8 << 1) | 1,
    [0x43] = (F9 << 1) | 1,
    [0x44] = (F10 << 1) | 1,
    [0x45] = (__NUM << 1) | 1,
    [0x46] = (SCRLK << 1) | 1,
    [0x47] = (__7 << 1) | 1,
    [0x48] = (__8 << 1) | 1,
    [0x49] = (__9 << 1) | 1,
    [0x4a] = (__SUB << 1) | 1,
    [0x4b] = (__4 << 1) | 1,
    [0x4c] = (__5 << 1) | 1,
    [0x4d] = (__6 << 1) | 1,
    [0x4e] = (__ADD << 1) | 1,
    [0x4f] = (__1 << 1) | 1,
    [0x50] = (__2 << 1) | 1,
    [0x51] = (__3 << 1) | 1,
    [0x52] = (__0 << 1) | 1,
    [0x53] = (__DOT << 1) | 1,
    [0x57] = (F11 << 1) | 1,
    [0x58] = (F12 << 1) | 1,
    [0x80 + 0x01] = ESC << 1,
    [0x80 + 0x02] = _1 << 1,
    [0x80 + 0x03] = _2 << 1,
    [0x80 + 0x04] = _3 << 1,
    [0x80 + 0x05] = _4 << 1,
    [0x80 + 0x06] = _5 << 1,
    [0x80 + 0x07] = _6 << 1,
    [0x80 + 0x08] = _7 << 1,
    [0x80 + 0x09] = _8 << 1,
    [0x80 + 0x0a] = _9 << 1,
    [0x80 + 0x0b] = _0 << 1,
    [0x80 + 0x0c] = SUB << 1,
    [0x80 + 0x0d] = EQU << 1,
    [0x80 + 0x0e] = BACK << 1,
    [0x80 + 0x0f] = TAB << 1,
    [0x80 + 0x10] = Q << 1,
    [0x80 + 0x11] = W << 1,
    [0x80 + 0x12] = E << 1,
    [0x80 + 0x13] = R << 1,
    [0x80 + 0x14] = T << 1,
    [0x80 + 0x15] = Y << 1,
    [0x80 + 0x16] = U << 1,
    [0x80 + 0x17] = I << 1,
    [0x80 + 0x18] = O << 1,
    [0x80 + 0x19] = P << 1,
    [0x80 + 0x1a] = OPEN_BRACKET << 1,
    [0x80 + 0x1b] = CLOSE_BRACKET << 1,
    [0x80 + 0x1c] = ENTER << 1,
    [0x80 + 0x1d] = LEFT_CTRL << 1,
    [0x80 + 0x1e] = A << 1,
    [0x80 + 0x1f] = S << 1,
    [0x80 + 0x20] = D << 1,
    [0x80 + 0x21] = F << 1,
    [0x80 + 0x22] = G << 1,
    [0x80 + 0x23] = H << 1,
    [0x80 + 0x24] = J << 1,
    [0x80 + 0x25] = K << 1,
    [0x80 + 0x26] = L << 1,
    [0x80 + 0x27] = SEMICOLON << 1,
    [0x80 + 0x28] = QUOTE << 1,
    [0x80 + 0x29] = BACK_QUOTE << 1,
    [0x80 + 0x2a] = LEFT_SHIFT << 1,
    [0x80 + 0x2b] = BACKSLASH << 1,
    [0x80 + 0x2c] = Z << 1,
    [0x80 + 0x2d] = X << 1,
    [0x80 + 0x2e] = C << 1,
    [0x80 + 0x2f] = V << 1,
    [0x80 + 0x30] = B << 1,
    [0x80 + 0x31] = N << 1,
    [0x80 + 0x32] = M << 1,
    [0x80 + 0x33] = COMMA << 1,
    [0x80 + 0x34] = DOT << 1,
    [0x80 + 0x35] = SLASH << 1,
    [0x80 + 0x36] = RIGHT_SHIFT << 1,
    [0x80 + 0x37] = __ASTERISK << 1,
    [0x80 + 0x38] = LEFT_ALT << 1,
    [0x80 + 0x39] = SPACE << 1,
    [0x80 + 0x3a] = CAP << 1,
    [0x80 + 0x3b] = F1 << 1,
    [0x80 + 0x3c] = F2 << 1,
    [0x80 + 0x3d] = F3 << 1,
    [0x80 + 0x3e] = F4 << 1,
    [0x80 + 0x3f] = F5 << 1,
    [0x80 + 0x40] = F6 << 1,
    [0x80 + 0x41] = F7 << 1,
    [0x80 + 0x42] = F8 << 1,
    [0x80 + 0x43] = F9 << 1,
    [0x80 + 0x44] = F10 << 1,
    [0x80 + 0x45] = __NUM << 1,
    [0x80 + 0x46] = SCRLK << 1,
    [0x80 + 0x47] = __7 << 1,
    [0x80 + 0x48] = __8 << 1,
    [0x80 + 0x49] = __9 << 1,
    [0x80 + 0x4a] = __SUB << 1,
    [0x80 + 0x4b] = __4 << 1,
    [0x80 + 0x4c] = __5 << 1,
    [0x80 + 0x4d] = __6 << 1,
    [0x80 + 0x4e] = __ADD << 1,
    [0x80 + 0x4f] = __1 << 1,
    [0x80 + 0x50] = __2 << 1,
    [0x80 + 0x51] = __3 << 1,
    [0x80 + 0x52] = __0 << 1,
    [0x80 + 0x53] = __DOT << 1,
    [0x80 + 0x57] = F11 << 1,
    [0x80 + 0x58] = F12 << 1,
};
// 当第一个扫描码 == 0xE0 时，第二个扫描码使用这个map
static const uint16_t ps2_set1_map2[UINT8_MAX] = {
    [0x1c] = (__ENTER << 1) | 1,
    [0x1d] = (RIGHT_CTRL << 1) | 1,
    [0x35] = (__SLASH << 1) | 1,
    [0x38] = (RIGHT_ALT << 1) | 1,
    [0x47] = (HOME << 1) | 1,
    [0x48] = (CURSOR_UP << 1) | 1,
    [0x49] = (PAGE_UP << 1) | 1,
    [0x4b] = (CURSOR_LEFT << 1) | 1,
    [0x4d] = (CURSOR_RIGHT << 1) | 1,
    [0x4f] = (END << 1) | 1,
    [0x50] = (CURSOR_DOWN << 1) | 1,
    [0x51] = (PAGE_DOWN << 1) | 1,
    [0x52] = (INSERT << 1) | 1,
    [0x53] = (DEL << 1) | 1,
    [0x80 + 0x1c] = __ENTER << 1,
    [0x80 + 0x1d] = RIGHT_CTRL << 1,
    [0x80 + 0x35] = __SLASH << 1,
    [0x80 + 0x38] = RIGHT_ALT << 1,
    [0x80 + 0x47] = HOME << 1,
    [0x80 + 0x48] = CURSOR_UP << 1,
    [0x80 + 0x49] = PAGE_UP << 1,
    [0x80 + 0x4b] = CURSOR_LEFT << 1,
    [0x80 + 0x4d] = CURSOR_RIGHT << 1,
    [0x80 + 0x4f] = END << 1,
    [0x80 + 0x50] = CURSOR_DOWN << 1,
    [0x80 + 0x51] = PAGE_DOWN << 1,
    [0x80 + 0x52] = INSERT << 1,
    [0x80 + 0x53] = DEL << 1,
};

// return a key_event
uint16_t scan()
{
    uint8_t c;
    if (getc(&c) != 0)
        abort();

    uint16_t ret;
    if (c == 0xE0) {
        if (getc(&c) != 0)
            abort();
        if (c == 0x2A) {
            if (getc(&c) != 0 || c != 0xE0)
                abort();
            if (getc(&c) != 0 || c != 0x37)
                abort();
            ret = (PRTSC << 1) | 1;
        } else if (c == 0xB7) {
            if (getc(&c) != 0 || c != 0xE0)
                abort();
            if (getc(&c) != 0 || c != 0xAA)
                abort();
            ret = RTSC << 1;
        } else
            ret = ps2_set1_map2[c];
    } else if (c == 0xE1) {
        if (getc(&c) != 0 || c != 0x1D)
            abort();
        if (getc(&c) != 0 || c != 0x45)
            abort();
        if (getc(&c) != 0 || c != 0xE1)
            abort();
        if (getc(&c) != 0 || c != 0x9D)
            abort();
        if (getc(&c) != 0 || c != 0xC5) 
            abort();
        ret = (PAUSE << 1) | 1;
    } else
        ret = ps2_set1_map[c];

    if (!empty())
        abort();
    if (ret == 0)
        abort();

    return ret;
}


bool keymap[KEY_NUM] = {false};
char to_ascii[KEY_NUM];
char to_ascii_shift[KEY_NUM];
char to_ascii_cap[KEY_NUM];
char to_ascii_cap_shift[KEY_NUM];

void handle()
{
    uint16_t key_event;
    while (get_key_event(&key_event) != 0) {
        char c;
        // bit0 : 1 == press, 0 == release
        // assert((key >> 1) <= 103);
        keymap[key >> 1] = key & 1;
        if (key & 1) {
            if (!keymap[SHIFT] && !keymap[CAP])
                c = to_ascii[key >> 1];
            else if (keymap[SHIFT] && !keymap[CAP])
                c = to_ascii_shift[key >> 1];
            else if (!keymap[SHIFT] && keymap[CAP])
                c = to_ascii_cap[key >> 1];
            else
                c = to_ascii_cap_shift[key >> 1];
            if (c != '\0')
                put_to_read_buf(c);
        }
    }
}
