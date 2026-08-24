/**
 * @file    ls_pal.c
 * @brief   OPTIGA(TM) Trust M PAL top-level init for LibreServo v4.
 *
 * Implements [57] `include/pal/pal.h`.  The host library calls `pal_init()`
 * because #OPTIGA_PAL_INIT_ENABLED is defined in
 * `firmware/config/optiga_lib_config.h`.
 *
 * Written by Claude Opus 5 (claude-opus-5) under human direction, 2026-08-23.
 */

#include "pal.h"

#include "pal_ifx_i2c_config.h"
#include "pal_os_timer.h"

pal_status_t pal_init(void) {
    /* The I2C peripheral and the SE_RST GPIO are initialised by the library's
     * own comms open path, through pal_i2c_init()/pal_gpio_init(), so doing it
     * again here would re-key PWREN and re-assert RSTCTL underneath an
     * in-progress session.  The timer init is the only thing that belongs at
     * this level, and on this port it is itself a no-op — see the "Time base
     * contract" comment in ls_board.h. */
    return pal_timer_init();
}

pal_status_t pal_deinit(void) {
    return pal_timer_deinit();
}
