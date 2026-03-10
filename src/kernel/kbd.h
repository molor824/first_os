#ifndef __KBD_H
#define __KBD_H

// Pushes scancode
void kbd_push(int scancode);

// Gets whether if key pressed or not
int kbd_key_state(int key);

// Gets scancode
// Returns 0 if no scancode is pushed yet
int kbd_scancode(void);

// Gets charcode
// Returns '\0' if current scancode cannot be converted to character
// Otherwise specific char
char kbd_charcode(void);

#endif