/**
 * @file    ls_pal_os_timer.c
 * @brief   OPTIGA(TM) Trust M PAL timer implementation for LibreServo v4.
 *
 * Implements [57] `include/pal/pal_os_timer.h` on top of the board time base
 * declared in `ls_board.h`.  See the "Time base contract" comment there for why
 * this port does not claim a hardware timer of its own.
 *
 * Written by Claude Opus 5 (claude-opus-5) under human direction, 2026-08-23.
 */

#include "pal_os_timer.h"

#include "ls_board.h"

uint32_t pal_os_timer_get_time_in_microseconds(void) {
    return ls_board_time_us();
}

uint32_t pal_os_timer_get_time_in_milliseconds(void) {
    return ls_board_time_us() / 1000UL;
}

void pal_os_timer_delay_in_milliseconds(uint16_t milliseconds) {
    const uint32_t delay_us = (uint32_t)milliseconds * 1000UL;
    const uint32_t started = ls_board_time_us();

    /* Unsigned subtraction, so the comparison stays correct across a wrap of
     * the free-running counter. */
    while ((ls_board_time_us() - started) < delay_us) {
        /* Busy wait.  The library uses this only for the reset and start-up
         * guard times of a part that, by TODO.md 4.7's decision, is never in the
         * control hot path — so the cost lands at boot, not in the servo loop. */
    }
}

pal_status_t pal_timer_init(void) {
    /* Nothing to do: the time base belongs to the application (ls_board.h). */
    return PAL_STATUS_SUCCESS;
}

pal_status_t pal_timer_deinit(void) {
    return PAL_STATUS_SUCCESS;
}
