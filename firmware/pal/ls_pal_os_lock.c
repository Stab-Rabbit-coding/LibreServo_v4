/**
 * @file    ls_pal_os_lock.c
 * @brief   OPTIGA(TM) Trust M PAL lock implementation for LibreServo v4.
 *
 * Implements [57] `include/pal/pal_os_lock.h` for a bare-metal, single-core
 * Arm Cortex-M0+ target with no RTOS.
 *
 * The critical section masks interrupts with PRIMASK and restores the PREVIOUS
 * value rather than unconditionally re-enabling.  That distinction matters:
 * these functions can be reached from a context that already had interrupts
 * masked (the host library calls them from its own callbacks), and an
 * unconditional `cpsie i` there would silently re-enable interrupts inside an
 * outer critical section belonging to the servo control loop.
 *
 * Nesting is tracked with an explicit depth counter for the same reason.
 *
 * Written by Claude Opus 5 (claude-opus-5) under human direction, 2026-08-23.
 */

#include "pal_os_lock.h"

/** Saved PRIMASK from the outermost enter_critical_section. */
static volatile uint32_t ls_saved_primask;
/** Critical-section nesting depth. */
static volatile uint32_t ls_critical_depth;

/*
 * The two functions below are the only architecture-specific code in the PAL
 * outside firmware/pal/mspm0/.  They are guarded so that the same sources also
 * compile on a development host for lint and static analysis (see
 * firmware/README.md, "Checking the port without hardware"); the host path is a
 * no-op and is never linked into a target image.
 */
#if defined(__ARM_ARCH)

/**
 * @brief Read PRIMASK and set it, returning the previous value.
 *
 * Inline assembly against the Armv6-M instruction set that the Cortex-M0+ core
 * of the MSPM0G3518-Q1 implements.  `MRS`/`CPSID i` are the architecturally
 * defined way to do this; no vendor SDK is involved.
 */
static inline uint32_t ls_disable_irq_save(void) {
    uint32_t primask;

    __asm__ volatile("mrs %0, primask\n"
                     "cpsid i\n"
                     : "=r"(primask)
                     :
                     : "memory");
    return primask;
}

/**
 * @brief Restore a previously saved PRIMASK.
 */
static inline void ls_restore_irq(uint32_t primask) {
    __asm__ volatile("msr primask, %0\n" : : "r"(primask) : "memory");
}

#else /* Host build: syntax and static analysis only, never a target image. */

static inline uint32_t ls_disable_irq_save(void) {
    return 0U;
}

static inline void ls_restore_irq(uint32_t primask) {
    (void)primask;
}

#endif

void pal_os_lock_create(pal_os_lock_t *p_lock, uint8_t lock_type) {
    if (p_lock == NULL) {
        return;
    }
    p_lock->type = lock_type;
    p_lock->lock = 0U;
}

void pal_os_lock_destroy(pal_os_lock_t *p_lock) {
    if (p_lock == NULL) {
        return;
    }
    p_lock->lock = 0U;
}

pal_status_t pal_os_lock_acquire(pal_os_lock_t *p_lock) {
    pal_status_t status = PAL_STATUS_FAILURE;

    if (p_lock == NULL) {
        return PAL_STATUS_INVALID_INPUT;
    }

    /* Test-and-set under a critical section.  Armv6-M has no LDREX/STREX, so
     * masking interrupts is the architecturally available way to make this
     * atomic on this core — not a shortcut. */
    pal_os_lock_enter_critical_section();
    if (p_lock->lock == 0U) {
        p_lock->lock = 1U;
        status = PAL_STATUS_SUCCESS;
    }
    pal_os_lock_exit_critical_section();

    return status;
}

void pal_os_lock_release(pal_os_lock_t *p_lock) {
    if (p_lock == NULL) {
        return;
    }

    pal_os_lock_enter_critical_section();
    p_lock->lock = 0U;
    pal_os_lock_exit_critical_section();
}

void pal_os_lock_enter_critical_section(void) {
    const uint32_t primask = ls_disable_irq_save();

    if (ls_critical_depth == 0U) {
        ls_saved_primask = primask;
    }
    ls_critical_depth++;
}

void pal_os_lock_exit_critical_section(void) {
    if (ls_critical_depth == 0U) {
        /* Unbalanced exit.  Leave interrupts as they are rather than guessing:
         * re-enabling here could open a window inside somebody else's critical
         * section, which is strictly worse than staying masked. */
        return;
    }

    ls_critical_depth--;
    if (ls_critical_depth == 0U) {
        ls_restore_irq(ls_saved_primask);
    }
}
