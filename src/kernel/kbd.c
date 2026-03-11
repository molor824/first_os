#include "kbd.h"
#include "keys.h"
#include <stdint.h>

#define KEYMAP_SIZE 0x200

typedef enum bucket {
    BUCKET_EMPTY,
    BUCKET_FULL,
    BUCKET_GHOST
} bucket_t;

static int s_scancode;

static uint8_t s_key_pressed[KEYMAP_SIZE];
static int s_key_type[KEYMAP_SIZE];
static uint8_t s_key_bucket[KEYMAP_SIZE];

static int kbd_get_key_index(int key) {
    int index = key % KEYMAP_SIZE;
    int cursor = index;
    int empty_index = -1;
    do {
        switch (s_key_bucket[cursor]) {
            case BUCKET_FULL:
                if (s_key_type[cursor] == key)
                    return cursor;
                break;
            case BUCKET_GHOST:
                empty_index = cursor;
                break;
            default:
                if (empty_index != -1)
                    return empty_index;
                else
                    return cursor;
        }
        cursor = (cursor + 1) % KEYMAP_SIZE;
    } while (cursor != index);
    return empty_index;
}
static int kbd_set_state(int key, int pressed) {
    int index = kbd_get_key_index(key);
    if (index == -1) return 0;
    s_key_bucket[index] = BUCKET_FULL;
    s_key_type[index] = key;
    s_key_pressed[index] = pressed;
    return 1;
}
int kbd_key_state(int key) {
    int index = kbd_get_key_index(key);
    if (index == -1) return 0;
    if (s_key_bucket[index] != BUCKET_FULL) return 0;
    return s_key_pressed[index];
}

void kbd_push(int scancode) {
    if (s_scancode == 0xe0) {
        s_scancode = scancode | 0xe000;
    } else {
        s_scancode = scancode;
    }
    if (s_scancode != 0xe0) {
        kbd_set_state(s_scancode & ~0x80, (s_scancode & 0x80) == 0);
    }
}

static char lowercase_map[128] = {
    [KEY_TILDE] = '`',[KEY_MINUS] = '-',[KEY_EQUALS] = '=',[KEY_LBRACKET] = '[',[KEY_RBRACKET] = ']',[KEY_BACKSLASH] = '\\',[KEY_COLON] = ';',
    [KEY_COMMA] = ',',[KEY_DOT] = '.',[KEY_SLASH] = '/',[KEY_SPACE] = ' ',[KEY_ENTER] = '\n',[KEY_BACKSPACE] = '\b',[KEY_QUOTE] = '\'',
    [KEY_0] = '0',[KEY_1] = '1',[KEY_2] = '2',[KEY_3] = '3',[KEY_4] = '4',[KEY_5] = '5',[KEY_6] = '6',[KEY_7] = '7',[KEY_8] = '8',[KEY_9] = '9',
    [KEY_Q] = 'q',[KEY_W] = 'w',[KEY_E] = 'e',[KEY_R] = 'r',[KEY_T] = 't',[KEY_Y] = 'y',[KEY_U] = 'u',[KEY_I] = 'i',[KEY_O] = 'o',[KEY_P] = 'p',
    [KEY_A] = 'a',[KEY_S] = 's',[KEY_D] = 'd',[KEY_F] = 'f',[KEY_G] = 'g',[KEY_H] = 'h',[KEY_J] = 'j',[KEY_K] = 'k',[KEY_L] = 'l',
    [KEY_Z] = 'z',[KEY_X] = 'x',[KEY_C] = 'c',[KEY_V] = 'v',[KEY_B] = 'b',[KEY_N] = 'n',[KEY_M] = 'm', [KEY_TAB] = '\t'
};
static char uppercase_map[128] = {
    [KEY_TILDE] = '~',[KEY_MINUS] = '_',[KEY_EQUALS] = '+',[KEY_LBRACKET] = '{',[KEY_RBRACKET] = '}',[KEY_BACKSLASH] = '|',[KEY_COLON] = ':',
    [KEY_COMMA] = '<',[KEY_DOT] = '>',[KEY_SLASH] = '?',[KEY_SPACE] = ' ',[KEY_ENTER] = '\n',[KEY_BACKSPACE] = '\b',[KEY_QUOTE] = '"',
    [KEY_0] = ')',[KEY_1] = '!',[KEY_2] = '@',[KEY_3] = '#',[KEY_4] = '$',[KEY_5] = '%',[KEY_6] = '^',[KEY_7] = '&',[KEY_8] = '*',[KEY_9] = '(',
    [KEY_Q] = 'Q',[KEY_W] = 'W',[KEY_E] = 'E',[KEY_R] = 'R',[KEY_T] = 'T',[KEY_Y] = 'Y',[KEY_U] = 'U',[KEY_I] = 'I',[KEY_O] = 'O',[KEY_P] = 'P',
    [KEY_A] = 'A',[KEY_S] = 'S',[KEY_D] = 'D',[KEY_F] = 'F',[KEY_G] = 'G',[KEY_H] = 'H',[KEY_J] = 'J',[KEY_K] = 'K',[KEY_L] = 'L',
    [KEY_Z] = 'Z',[KEY_X] = 'X',[KEY_C] = 'C',[KEY_V] = 'V',[KEY_B] = 'B',[KEY_N] = 'N',[KEY_M] = 'M', [KEY_TAB] = '\t'
};

char kbd_charcode(void) {
    if (s_scancode >= 0x80 || s_scancode < 0) return 0;
    if (kbd_key_state(KEY_LSHIFT) || kbd_key_state(KEY_RSHIFT))
        return uppercase_map[s_scancode];
    else
        return lowercase_map[s_scancode];
}