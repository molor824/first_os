#include "int_event.h"
#include <stddef.h>

int_event_t event_queue[INT_EVENT_QUEUE_SIZE];
size_t queue_position = 0;
size_t queue_size = 0;

int event_pop_front(int_event_t *event) {
    if (queue_size == 0) return 0;
    // CRITICAL SECTION START
    __asm__ volatile("cli");
    queue_size--;
    if (event) *event = event_queue[(queue_position + queue_size) % INT_EVENT_QUEUE_SIZE];
    // CRITICAL SECTION END
    __asm__ volatile("sti");
    return 1;
}
int event_push_back(int_event_t event) {
    if (queue_size >= INT_EVENT_QUEUE_SIZE) {
        // CRITICAL SECTION START
        __asm__ volatile("cli");
        event_queue[queue_position] = event;
        queue_size = INT_EVENT_QUEUE_SIZE;
        queue_position = (queue_position + 1) % INT_EVENT_QUEUE_SIZE;
        // CRITICAL SECTION END
        __asm__ volatile("sti");
        return -1;
    }
    // CRITICAL SECTION START
    __asm__ volatile("cli");
    event_queue[(queue_position + queue_size) % INT_EVENT_QUEUE_SIZE] = event;
    queue_size++;
    // CRITICAL SECTION END
    __asm__ volatile("sti");
    return 0;
}
