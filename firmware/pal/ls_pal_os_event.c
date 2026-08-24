/**
 * @file    ls_pal_os_event.c
 * @brief   OPTIGA(TM) Trust M PAL event implementation for LibreServo v4.
 *
 * Implements [57] `include/pal/pal_os_event.h` on top of the one-shot scheduling
 * contract declared in `ls_board.h`.
 *
 * The host library keeps exactly one outstanding one-shot at a time, so this
 * port holds a single static event structure rather than a pool.  That is a
 * deliberate consequence of the library's own API shape
 * (`pal_os_event_register_callback_oneshot` replaces rather than queues), not a
 * simplification of it — see the note on #ls_board_schedule_oneshot_us.
 *
 * Written by Claude Opus 5 (claude-opus-5) under human direction, 2026-08-23.
 */

#include "pal_os_event.h"

#include "ls_board.h"
#include "pal_os_lock.h"

/** The single event instance.  Static storage, consistent with the heapless
 *  memory policy in `ls_pal_os_memory.c`. */
static pal_os_event_t ls_pal_os_event_0;

/**
 * @brief Trampoline handed to the board scheduler.
 *
 * Copies the registered callback and context out under a critical section
 * before invoking, so a callback that re-registers a new one-shot (which the
 * host library does routinely) cannot have its own arguments overwritten
 * mid-dispatch.
 */
static void ls_pal_os_event_trampoline(void *unused) {
    register_callback callback;
    void *context;

    (void)unused;

    pal_os_lock_enter_critical_section();
    callback = ls_pal_os_event_0.callback_registered;
    context = ls_pal_os_event_0.callback_ctx;
    ls_pal_os_event_0.is_event_triggered = FALSE;
    pal_os_lock_exit_critical_section();

    if (callback != NULL) {
        callback(context);
    }
}

pal_os_event_t *pal_os_event_create(register_callback callback, void *callback_args) {
    if (callback != NULL) {
        pal_os_event_start(&ls_pal_os_event_0, callback, callback_args);
    }
    return &ls_pal_os_event_0;
}

void pal_os_event_destroy(pal_os_event_t *pal_os_event) {
    (void)pal_os_event;
    pal_os_event_stop(&ls_pal_os_event_0);
}

void pal_os_event_register_callback_oneshot(pal_os_event_t *p_pal_os_event,
                                            register_callback callback,
                                            void *callback_args,
                                            uint32_t time_us) {
    pal_os_event_t *const event =
        (p_pal_os_event != NULL) ? p_pal_os_event : &ls_pal_os_event_0;

    pal_os_lock_enter_critical_section();
    event->callback_registered = callback;
    event->callback_ctx = callback_args;
    event->timeout_us = time_us;
    event->is_event_triggered = TRUE;
    pal_os_lock_exit_critical_section();

    ls_board_schedule_oneshot_us(ls_pal_os_event_trampoline, NULL, time_us);
}

void pal_os_event_trigger_registered_callback(void) {
    /* Invoked by an application that drives the event from its own tick rather
     * than from the board scheduler.  Routing it through the same trampoline
     * keeps the two paths from diverging. */
    ls_pal_os_event_trampoline(NULL);
}

void pal_os_event_start(pal_os_event_t *p_pal_os_event,
                        register_callback callback,
                        void *callback_args) {
    pal_os_event_t *const event =
        (p_pal_os_event != NULL) ? p_pal_os_event : &ls_pal_os_event_0;

    if (event->is_event_triggered == FALSE) {
        event->is_event_triggered = TRUE;
        pal_os_event_register_callback_oneshot(event, callback, callback_args, 1000U);
    }
}

void pal_os_event_stop(pal_os_event_t *p_pal_os_event) {
    pal_os_event_t *const event =
        (p_pal_os_event != NULL) ? p_pal_os_event : &ls_pal_os_event_0;

    ls_board_cancel_oneshot();

    pal_os_lock_enter_critical_section();
    event->is_event_triggered = FALSE;
    event->callback_registered = NULL;
    event->callback_ctx = NULL;
    pal_os_lock_exit_critical_section();
}
