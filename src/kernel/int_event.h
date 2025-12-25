#ifndef __INT_EVENT_H
#define __INT_EVENT_H

#include <stdint.h>

#define INT_EVENT_QUEUE_SIZE 0x100

typedef enum {
    INT_NULL,
    INT_TIMER,
    INT_KEYBOARD
} int_type_t;

typedef struct {
    int_type_t type;
    union {
        uint8_t kbd_scancode;
    };
} int_event_t;

// Returns: 1 - popped succesfully, 0 - no element
int event_pop_front(int_event_t *event);
// Returns: 0 - no problem, -1 - removed last event
int event_push_back(int_event_t event);

#endif