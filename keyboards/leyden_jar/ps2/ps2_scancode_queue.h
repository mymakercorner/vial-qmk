#pragma once

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>   // NULL

/*
 * Single-producer / single-consumer ring buffer of length-prefixed PS/2
 * scancode blobs (the same format as the hid_to_ps2_set_*.h table entries:
 * element [0] is the byte count, followed by the scancode bytes).
 *
 * Producer (HID event -> enqueue) and consumer (the send task) both run in the
 * one polled main loop, so no locking is needed. Depends only on the standard
 * headers <stdint.h>, <stdbool.h> and <stddef.h> to stay framework-independent
 * (QMK-friendly).
 */

#define PS2_SCANCODE_QUEUE_DEPTH 16   // how many key events can be buffered
#define PS2_SCANCODE_MAX_BYTES    9   // longest length-prefixed blob (set 2 Pause = len + 8)

typedef struct {
    uint8_t buf[PS2_SCANCODE_QUEUE_DEPTH][PS2_SCANCODE_MAX_BYTES];
    uint8_t head, tail;               // head == tail => empty; one slot kept open
} ps2_scancode_queue;

static inline bool ps2_scancode_queue_empty(const ps2_scancode_queue *q) {
    return q->head == q->tail;
}

static inline bool ps2_scancode_queue_push(ps2_scancode_queue *q, const uint8_t *blob) {
    uint8_t next = (q->head + 1) % PS2_SCANCODE_QUEUE_DEPTH;
    if (next == q->tail) {
        return false;                                  // full: reject the whole blob
    }
    uint8_t len = blob[0] + 1;                          // length byte + payload
    for (uint8_t i = 0; i < len; i++) {
        q->buf[q->head][i] = blob[i];
    }
    q->head = next;
    return true;
}

static inline const uint8_t *ps2_scancode_queue_peek(ps2_scancode_queue *q) {
    return ps2_scancode_queue_empty(q) ? NULL : q->buf[q->tail];
}

static inline void ps2_scancode_queue_pop(ps2_scancode_queue *q) {
    if (!ps2_scancode_queue_empty(q)) {
        q->tail = (q->tail + 1) % PS2_SCANCODE_QUEUE_DEPTH;
    }
}

// Drop everything still queued (used when a host command preempts key output).
static inline void ps2_scancode_queue_clear(ps2_scancode_queue *q) {
    q->tail = q->head;
}
