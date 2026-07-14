#pragma once

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>   // NULL

/*
 * Byte-level trace for on-hardware debugging of the PS/2 device.
 *
 * We have no logic analyzer, and the PS/2-to-USB dongle host hides the raw
 * set-2 bytes from Windows (it reports translated USB HID usages). So the only
 * authoritative record of what actually went on the wire is what this module
 * captures inside ps2_update. Events are stamped, dropped into a ring buffer by
 * a cheap O(1) recorder that never blocks (safe to call from the timing-
 * sensitive send/receive path), and later drained + pretty-printed from the
 * main loop over USB-CDC (printf), well away from any live PS/2 transaction.
 *
 * The whole thing sits behind PS2_TRACE_ENABLED so production / QMK builds
 * compile it out entirely (the PS2_TRACE macro and the drain become no-ops).
 * The recorder itself depends only on <stdint.h>/<stdbool.h>/<stddef.h> to stay
 * framework-independent; only the printf pretty-printer in the .c pulls stdio.
 */

#ifndef PS2_TRACE_ENABLED
#define PS2_TRACE_ENABLED 1        // default on for the debug bring-up; -DPS2_TRACE_ENABLED=0 to strip
#endif

// Kinds of trace event. The byte value carried alongside is decoded to a
// human name (ACK, BAT, echo, E0/F0 prefix, ...) by the pretty-printer, so the
// tag only needs to say *which direction / what channel* the byte came from.
typedef enum {
    PS2_TR_TX      = 0,   // a byte we placed on the wire (a->the frame's data byte)
    PS2_TR_RX      = 1,   // a valid byte received from the host
    PS2_TR_RX_PERR = 2,   // received byte failed parity (we requested a resend); a->the raw data byte
    PS2_TR_RESEND  = 3,   // host sent 0xFE asking us to resend last byte; a->the byte re-sent
    PS2_TR_BAT     = 4,   // BAT-complete announced (a == 0xAA), power-on or post-reset
    PS2_TR_NOTE    = 5,   // generic marker; a->a PS2_TR_NOTE_* subcode below
} ps2_trace_tag;

// Subcodes for PS2_TR_NOTE (semantic state markers, not wire bytes).
typedef enum {
    PS2_TR_NOTE_RESET      = 0,   // host reset (0xFF) accepted, BAT restarting
    PS2_TR_NOTE_ENABLE     = 1,   // scanning enabled  (0xF4)
    PS2_TR_NOTE_DISABLE    = 2,   // scanning disabled (0xF5)
    PS2_TR_NOTE_DEFAULTS   = 3,   // set defaults      (0xF6)
    PS2_TR_NOTE_QUEUE_CLR  = 4,   // outgoing key queue flushed by a host byte
    PS2_TR_NOTE_TX_ABORT   = 5,   // host pulled clock low mid-send (irq 4): our frame was aborted
} ps2_trace_note;

typedef struct {
    uint32_t t_us;    // timestamp supplied by the caller (ps2_update's now_us)
    uint8_t  tag;     // ps2_trace_tag
    uint8_t  a;       // the wire byte, or a PS2_TR_NOTE_* subcode for PS2_TR_NOTE
} ps2_trace_evt;

#if PS2_TRACE_ENABLED

// Hook used inside ps2_update. Cheap, non-blocking, overwrites the oldest event
// if the buffer is full (recent history is what matters during a trace).
#define PS2_TRACE(t_us, tag, a) ps2_trace_record((uint32_t)(t_us), (uint8_t)(tag), (uint8_t)(a))

#ifdef __cplusplus
extern "C" {
#endif

// Empty the buffer and reset the dropped-event counter.
void ps2_trace_reset(void);

// Record one event (O(1), never blocks). Called from the PS/2 send/receive path.
void ps2_trace_record(uint32_t t_us, uint8_t tag, uint8_t a);

// Pop the oldest pending event; false if the buffer is empty.
bool ps2_trace_pop(ps2_trace_evt *out);

// How many events were overwritten (lost) since the last reset — a gap marker.
uint32_t ps2_trace_dropped(void);

// Drain every pending event to stdout (USB-CDC) as decoded, human-readable
// lines. Call from the main loop, never from inside a PS/2 transaction.
void ps2_trace_drain_printf(void);

#ifdef __cplusplus
}
#endif

#else  /* !PS2_TRACE_ENABLED : everything compiles away */

#define PS2_TRACE(t_us, tag, a) ((void)0)

static inline void     ps2_trace_reset(void)                 { }
static inline void     ps2_trace_record(uint32_t t_us, uint8_t tag, uint8_t a) { (void)t_us; (void)tag; (void)a; }
static inline bool     ps2_trace_pop(ps2_trace_evt *out)     { (void)out; return false; }
static inline uint32_t ps2_trace_dropped(void)               { return 0; }
static inline void     ps2_trace_drain_printf(void)          { }

#endif /* PS2_TRACE_ENABLED */
