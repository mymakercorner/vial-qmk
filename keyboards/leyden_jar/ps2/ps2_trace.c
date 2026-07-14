#include "ps2_trace.h"

#if PS2_TRACE_ENABLED

#include <stdio.h>

/*
 * Ring buffer of trace events. Producer (ps2_trace_record, from ps2_update) and
 * consumer (ps2_trace_pop / drain, from the main loop) both run in the one
 * polled loop, so no locking is needed — same single-threaded assumption as
 * ps2_scancode_queue. Size is a power of two so the wrap is a cheap mask.
 */
#define PS2_TRACE_CAPACITY 128u
#define PS2_TRACE_MASK     (PS2_TRACE_CAPACITY - 1u)

static ps2_trace_evt s_buf[PS2_TRACE_CAPACITY];
static uint32_t      s_tail;      // index of the oldest pending event
static uint32_t      s_count;     // number of pending events (0..CAPACITY)
static uint32_t      s_dropped;   // events overwritten because the buffer was full

void ps2_trace_reset(void) {
    s_tail    = 0;
    s_count   = 0;
    s_dropped = 0;
}

void ps2_trace_record(uint32_t t_us, uint8_t tag, uint8_t a) {
    if (s_count == PS2_TRACE_CAPACITY) {
        // Full: drop the oldest so the most recent history survives. Losing the
        // start of a burst is preferable to stalling the timing-critical path.
        s_tail = (s_tail + 1u) & PS2_TRACE_MASK;
        s_count--;
        s_dropped++;
    }
    uint32_t head = (s_tail + s_count) & PS2_TRACE_MASK;
    s_buf[head].t_us = t_us;
    s_buf[head].tag  = tag;
    s_buf[head].a    = a;
    s_count++;
}

bool ps2_trace_pop(ps2_trace_evt *out) {
    if (s_count == 0) {
        return false;
    }
    *out = s_buf[s_tail];
    s_tail = (s_tail + 1u) & PS2_TRACE_MASK;
    s_count--;
    return true;
}

uint32_t ps2_trace_dropped(void) {
    return s_dropped;
}

// Name a byte we placed on the wire (a device->host response or scancode).
static const char *ps2_trace_name_tx(uint8_t b) {
    switch (b) {
        case 0xFA: return "ACK";
        case 0xAA: return "BAT-OK / self-test passed";
        case 0xFC: return "BAT-FAIL";
        case 0xEE: return "echo reply";
        case 0xFE: return "resend reply";
        case 0xAB: return "ID byte 0 (0xAB)";
        case 0x83: return "ID byte 1 (0x83)";
        case 0xE0: return "prefix E0";
        case 0xE1: return "prefix E1";
        case 0xF0: return "break prefix F0";
        default:   return "scancode";
    }
}

// Name a byte we received: a host command (or its data byte, which we can't
// distinguish here without state, so only the command opcodes are named).
static const char *ps2_trace_name_rx(uint8_t b) {
    switch (b) {
        case 0xED: return "set LEDs (0xED)";
        case 0xEE: return "echo (0xEE)";
        case 0xF0: return "set scan set (0xF0)";
        case 0xF2: return "read ID (0xF2)";
        case 0xF3: return "set typematic (0xF3)";
        case 0xF4: return "enable (0xF4)";
        case 0xF5: return "disable (0xF5)";
        case 0xF6: return "set defaults (0xF6)";
        case 0xF7: return "set-all typematic (0xF7)";
        case 0xF8: return "set-all make/break (0xF8)";
        case 0xF9: return "set-all make (0xF9)";
        case 0xFA: return "set-all typ+mk+brk (0xFA)";
        case 0xFB: return "set-key typematic (0xFB)";
        case 0xFC: return "set-key make/break (0xFC)";
        case 0xFD: return "set-key make (0xFD)";
        case 0xFE: return "resend (0xFE)";
        case 0xFF: return "reset (0xFF)";
        default:   return "data / arg";
    }
}

static const char *ps2_trace_name_note(uint8_t sub) {
    switch (sub) {
        case PS2_TR_NOTE_RESET:     return "reset accepted, BAT restarting";
        case PS2_TR_NOTE_ENABLE:    return "scanning enabled";
        case PS2_TR_NOTE_DISABLE:   return "scanning disabled";
        case PS2_TR_NOTE_DEFAULTS:  return "defaults restored";
        case PS2_TR_NOTE_QUEUE_CLR: return "key queue flushed by host byte";
        case PS2_TR_NOTE_TX_ABORT:  return "TX ABORTED -- host pulled clock low mid-send";
        default:                    return "note";
    }
}

void ps2_trace_drain_printf(void) {
    static uint32_t prev_t_us;
    static bool     have_prev;

    ps2_trace_evt e;
    while (ps2_trace_pop(&e)) {
        // Microsecond delta from the previous printed event (wrap-safe) to make
        // frame gaps, the BAT delay, and typematic cadence visible at a glance.
        int32_t dt = have_prev ? (int32_t)(e.t_us - prev_t_us) : 0;
        prev_t_us = e.t_us;
        have_prev = true;

        switch (e.tag) {
            case PS2_TR_TX:
                printf("[%10lu +%7ld] TX     %02X  %s\n",
                       (unsigned long)e.t_us, (long)dt, e.a, ps2_trace_name_tx(e.a));
                break;
            case PS2_TR_RX:
                printf("[%10lu +%7ld] RX     %02X  %s\n",
                       (unsigned long)e.t_us, (long)dt, e.a, ps2_trace_name_rx(e.a));
                break;
            case PS2_TR_RX_PERR:
                printf("[%10lu +%7ld] RX!    %02X  PARITY ERROR (resend requested)\n",
                       (unsigned long)e.t_us, (long)dt, e.a);
                break;
            case PS2_TR_RESEND:
                printf("[%10lu +%7ld] RESEND %02X  host asked for last byte\n",
                       (unsigned long)e.t_us, (long)dt, e.a);
                break;
            case PS2_TR_BAT:
                printf("[%10lu +%7ld] BAT    %02X  %s\n",
                       (unsigned long)e.t_us, (long)dt, e.a, ps2_trace_name_tx(e.a));
                break;
            case PS2_TR_NOTE:
                printf("[%10lu +%7ld] NOTE   --  %s\n",
                       (unsigned long)e.t_us, (long)dt, ps2_trace_name_note(e.a));
                break;
            default:
                printf("[%10lu +%7ld] ????   %02X\n",
                       (unsigned long)e.t_us, (long)dt, e.a);
                break;
        }
    }

    if (s_dropped != 0) {
        printf("[trace] %lu event(s) dropped (buffer overflow)\n", (unsigned long)s_dropped);
        s_dropped = 0;   // report only NEW drops next time, not the running total
    }
}

#endif /* PS2_TRACE_ENABLED */
