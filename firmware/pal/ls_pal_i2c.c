/**
 * @file    ls_pal_i2c.c
 * @brief   OPTIGA(TM) Trust M PAL I2C implementation for the TI MSPM0G3518-Q1.
 *
 * Implements the `pal_i2c` contract of [57] `include/pal/pal_i2c.h` against the
 * MSPM0 I2C0 controller, driving U7 (OPTIGA(TM) Trust M V3) on SE_I2C_SCL (PA1)
 * and SE_I2C_SDA (PA8).
 *
 * Every register write below traces to [52] chapter 25, cited at the point of
 * use.  Register names and offsets live in `mspm0/ls_mspm0_i2c_regs.h`; board
 * facts live in `ls_board.h`.
 *
 * Attribution: the PAL function signatures and the event/status contract are
 * defined by the OPTIGA(TM) Trust M Host Library for C, Copyright (c) 2018-2024
 * Infineon Technologies AG, SPDX-License-Identifier: MIT ([57]).  The
 * MSPM0-specific implementation is original work for LibreServo v4.
 *
 * COMPLETION NOTE, deliberate and recorded rather than hidden: the transfers
 * here are BLOCKING and complete before returning, then invoke the upper-layer
 * handler synchronously with #PAL_I2C_EVENT_SUCCESS or #PAL_I2C_EVENT_ERROR.
 * `pal_i2c.h` permits this ("Returns when the I2C write is invoked
 * successfully") and it is the correct first implementation here: the whole
 * point of TODO.md 4.7 is that U7 is never in the control hot path, so the
 * longest blocking window this can create is one APDU frame at 400 kHz, at
 * boot, outside the servo loop.  Converting to interrupt-driven transfers is a
 * later optimisation, tracked as TODO.md 7.4, and must not be done before the
 * control loop of TODO.md 7.1 exists to be measured against.
 *
 * References (see REFERENCES.md):
 *   [45] Infineon, OPTIGA(TM) Trust M Datasheet, Rev 3.70.
 *   [46] TI, MSPM0G3519-Q1/MSPM0G3518-Q1 Datasheet, SLASFA6B.
 *   [52] TI, MSPM0 G-Series Technical Reference Manual, SLAU846E.
 *   [53] Infineon, OPTIGA(TM) Trust M Solution Reference Manual, Rev 3.70.
 *   [57] Infineon, OPTIGA(TM) Trust M Host Library for C, Ver 5.8.1.
 *
 * Written by Claude Opus 5 (claude-opus-5) under human direction, 2026-08-23.
 */

#include "pal_i2c.h"

#include "ls_board.h"

/**
 * Converter between the `void *` that [57] `pal_i2c_t` stores for
 * `upper_layer_event_handler` and the `upper_layer_callback_t` function pointer
 * that `optiga_lib_types.h` declares for it.
 *
 * A direct cast between an object pointer and a function pointer is not
 * something ISO C defines (`-Wpedantic` says so), even though every ABI this
 * code targets makes it work.  Going through a union keeps the conversion in
 * one clearly-labelled place instead of scattering casts the compiler is
 * entitled to complain about.
 */
typedef union ls_i2c_handler_cast {
    void *as_object_pointer;
    upper_layer_callback_t as_function_pointer;
} ls_i2c_handler_cast_t;

/**
 * Bound on how long any single byte or transaction phase may take before the
 * driver gives up and reports an error.
 *
 * Sized from the bus, not guessed: at #LS_SE_I2C_SCL_HZ a byte plus its ACK is
 * 9 SCL periods = 22.5 us at 400 kHz.  10 ms is over 400x that, which leaves
 * ample room for the target clock-stretching (CCR.CLKSTRETCH is enabled below)
 * while still being short enough that a stuck bus cannot wedge the boot
 * sequence indefinitely — a real risk on a servo that must reach a safe state.
 */
#define LS_I2C_PHASE_TIMEOUT_US     (10000UL)

/** Bound on waiting for the bus to go idle before starting a transaction. */
#define LS_I2C_BUS_FREE_TIMEOUT_US  (25000UL)

/**
 * @brief Wait until every bit in @p mask is clear in CSR, or time out.
 *
 * Uses unsigned subtraction against #ls_board_time_us so the wait is correct
 * across a counter wrap.
 *
 * @return 1 on success, 0 on timeout.
 */
static uint8_t ls_i2c_wait_csr_clear(uint32_t mask, uint32_t timeout_us) {
    const uint32_t started = ls_board_time_us();

    while ((ls_reg_read(LS_I2C0_BASE, LS_I2C_O_CSR) & mask) != 0UL) {
        if ((ls_board_time_us() - started) > timeout_us) {
            return 0U;
        }
    }
    return 1U;
}

/**
 * @brief Configure one PINCM register for a peripheral function.
 *
 * [52] section 8.2.1: the peripheral-select value goes in PF "while
 * simultaneously setting the PC and INENA bits".  The I2C lines additionally
 * need HIZ1 (open drain), because the bus is wired-AND with external pull-ups
 * R30/R31 ([45] p. 12 section 3 Figure 2) and a push-pull driver on either line
 * would fight the other device.
 */
static void ls_pincm_configure(uint32_t pincm_address, uint32_t pf, uint32_t extra_flags) {
    const uint32_t value = (pf & LS_PINCM_PF_Msk) | LS_PINCM_PC | LS_PINCM_INENA | extra_flags;

    *(volatile uint32_t *)(uintptr_t)pincm_address = value;
}

/**
 * @brief Initialise I2C0 and the two secure-element pins.
 *
 * Follows the bring-up order of [52] section 25.2 ("Configure SDA and SCL pin
 * functions ... Reset the peripheral ... Enable the power ... Select and
 * configure the I2C clock ... Set the desired SCL clock speed"), which is why
 * the steps below are in this order and not a more convenient one.
 */
pal_status_t pal_i2c_init(const pal_i2c_t *p_i2c_context) {
    (void)p_i2c_context; /* Single fixed instance on this board; see ls_board.h. */

    /* 1. Pin functions.  Both I2C lines are open-drain (HIZ1). */
    ls_pincm_configure(LS_PINCM_SE_I2C_SCL, LS_PF_SE_I2C_SCL, LS_PINCM_HIZ1);
    ls_pincm_configure(LS_PINCM_SE_I2C_SDA, LS_PF_SE_I2C_SDA, LS_PINCM_HIZ1);

    /* 2. Reset the peripheral, then clear the sticky reset flag.
     *    [52] section 25.3.2, p. 1313: writes require key 0xB1 in bits 31-24. */
    ls_reg_write(LS_I2C0_BASE, LS_I2C_O_RSTCTL,
                 LS_I2C_RSTCTL_KEY | LS_I2C_RSTCTL_RESETASSERT | LS_I2C_RSTCTL_RESETSTKYCLR);

    /* 3. Enable power.  [52] section 25.3.1, p. 1312: key 0x26. */
    ls_reg_write(LS_I2C0_BASE, LS_I2C_O_PWREN, LS_I2C_PWREN_KEY | LS_I2C_PWREN_ENABLE);

    /* 4. Clock source.  [52] section 25.3.6, p. 1317, Table 25-28: CLKSEL
     *    resets to 0x0, which selects NEITHER BUSCLK nor MFCLK — the module
     *    has no functional clock at all until this is written.  BUSCLK is
     *    selected explicitly (I2C0 is a PD0 peripheral per [46] p. 74/76, so
     *    BUSCLK here is ULPCLK, which equals MCLK/SYSOSC at 32 MHz under the
     *    reset-default clock configuration this design relies on — see the
     *    citation chain on LS_I2C_FUNCTIONAL_CLK_HZ in ls_board.h and
     *    TODO.md 7.3).  CLKDIV is left at its reset value (divide by 1). */
    ls_reg_write(LS_I2C0_BASE, LS_I2C_O_CLKSEL, LS_I2C_CLKSEL_BUSCLK_SEL);

    /* 5. SCL period.  Derived, not tabulated: see ls_i2c_tpr_from_clocks and
     *    [52] section 25.2.1 Equation 27. */
    {
        const uint32_t tpr = ls_i2c_tpr_from_clocks(LS_I2C_FUNCTIONAL_CLK_HZ, LS_SE_I2C_SCL_HZ);

        if (tpr > LS_I2C_CTPR_TPR_Msk) {
            /* The requested SCL is unreachable from this functional clock.
             * Failing loudly beats silently running the secure element's bus at
             * the wrong speed. */
            return PAL_STATUS_FAILURE;
        }
        ls_reg_write(LS_I2C0_BASE, LS_I2C_O_CTPR, tpr);
    }

    /* 6. FIFO trigger levels.  Programmed explicitly rather than left at reset,
     *    so the meaning of RIS.CTXFIFOTRG / RIS.CRXFIFOTRG in the transfer loops
     *    below is a stated fact and not a dependency on a reset value.
     *    [52] section 25.3.40, p. 1357. */
    ls_reg_write(LS_I2C0_BASE, LS_I2C_O_CFIFOCTL, LS_I2C_CFIFOCTL_BYTEWISE);

    /* 7. Enable the controller.  Clock stretching is enabled deliberately:
     *    [52] section 25.3.38 notes it "ensures compliance to the I2C standard",
     *    and the Trust M is entitled to stretch while it computes.  MCTL
     *    (multi-controller) is NOT set: U1 is the only controller on this
     *    two-device bus per the schematic, and enabling arbitration handling
     *    for a peer that does not exist only adds failure modes. */
    ls_reg_write(LS_I2C0_BASE, LS_I2C_O_CCR, LS_I2C_CCR_CLKSTRETCH | LS_I2C_CCR_ACTIVE);

    return PAL_STATUS_SUCCESS;
}

/**
 * @brief Set the bus bitrate.
 *
 * [57] `pal_i2c.h` requires this API to clamp to the master's maximum supported
 * value rather than fail.  This port clamps to #LS_SE_I2C_SCL_HZ, which is this
 * board's rated ceiling for reasons recorded in `ls_board.h` — it is a board
 * limit, not a peripheral limit, so raising it is a hardware decision.
 *
 * @param bitrate Requested bitrate in kHz, per the header's units.
 */
pal_status_t pal_i2c_set_bitrate(const pal_i2c_t *p_i2c_context, uint16_t bitrate) {
    uint32_t requested_hz = (uint32_t)bitrate * 1000UL;
    uint32_t tpr;

    (void)p_i2c_context;

    if ((requested_hz == 0UL) || (requested_hz > LS_SE_I2C_SCL_HZ)) {
        requested_hz = LS_SE_I2C_SCL_HZ;
    }

    if (!ls_i2c_wait_csr_clear(LS_I2C_CSR_BUSBSY | LS_I2C_CSR_BUSY, LS_I2C_BUS_FREE_TIMEOUT_US)) {
        return PAL_STATUS_I2C_BUSY;
    }

    tpr = ls_i2c_tpr_from_clocks(LS_I2C_FUNCTIONAL_CLK_HZ, requested_hz);
    if (tpr > LS_I2C_CTPR_TPR_Msk) {
        return PAL_STATUS_FAILURE;
    }
    ls_reg_write(LS_I2C0_BASE, LS_I2C_O_CTPR, tpr);

    return PAL_STATUS_SUCCESS;
}

/**
 * @brief Notify the upper layer, if it registered a handler.
 */
static void ls_i2c_notify(const pal_i2c_t *p_i2c_context, uint16_t event) {
    if (p_i2c_context->upper_layer_event_handler != NULL) {
        ls_i2c_handler_cast_t handler;

        handler.as_object_pointer = p_i2c_context->upper_layer_event_handler;
        handler.as_function_pointer(p_i2c_context->p_upper_layer_ctx, event);
    }
}

/**
 * @brief Start one controller transaction.
 *
 * [52] section 25.2: "the I2C controller target address I2Cx.MSA.SADDR register is
 * written with the desired address, the I2Cx.MSA.DIR bit should be set to 1 to
 * start a receive operation and 0 to start a transmit operation, and the control
 * register (I2Cx.MCTR) is written with ACK = X, STOP = 1, START = 1".
 *
 * ACK is set here because in receive mode it makes the controller acknowledge
 * each byte automatically; in transmit mode the bit is ignored.
 */
static void ls_i2c_start_transaction(uint8_t address, uint16_t length, uint8_t is_read) {
    uint32_t csa = ((uint32_t)address << LS_I2C_CSA_TADDR_Pos) & LS_I2C_CSA_TADDR_Msk;
    uint32_t cctr;

    if (is_read != 0U) {
        csa |= LS_I2C_CSA_DIR_RX;
    }
    ls_reg_write(LS_I2C0_BASE, LS_I2C_O_CSA, csa);

    cctr = (((uint32_t)length << LS_I2C_CCTR_CBLEN_Pos) & LS_I2C_CCTR_CBLEN_Msk)
           | LS_I2C_CCTR_ACK | LS_I2C_CCTR_STOP | LS_I2C_CCTR_START | LS_I2C_CCTR_BURSTRUN;
    ls_reg_write(LS_I2C0_BASE, LS_I2C_O_CCTR, cctr);
}

/**
 * @brief Common precondition check and length validation for read and write.
 *
 * @return #PAL_STATUS_SUCCESS if the transfer may proceed.
 */
static pal_status_t ls_i2c_prepare(const pal_i2c_t *p_i2c_context,
                                   const void *p_data,
                                   uint16_t length) {
    if ((p_i2c_context == NULL) || (p_data == NULL) || (length == 0U)) {
        return PAL_STATUS_INVALID_INPUT;
    }

    /* CCTR.CBLEN is 12 bits ([52] section 25.3.33), so a longer transfer would
     * silently truncate.  Reject it instead: a short read of an APDU is not a
     * recoverable condition, it is a corrupted one. */
    if (length > LS_I2C_CCTR_CBLEN_MAX) {
        return PAL_STATUS_INVALID_INPUT;
    }

    if (!ls_i2c_wait_csr_clear(LS_I2C_CSR_BUSBSY | LS_I2C_CSR_BUSY, LS_I2C_BUS_FREE_TIMEOUT_US)) {
        return PAL_STATUS_I2C_BUSY;
    }

    return PAL_STATUS_SUCCESS;
}

/**
 * @brief Poll RIS until any bit of @p wanted is set, an error appears, or the
 *        phase times out.
 *
 * RIS bits are sticky, so each caller clears the bit it consumed before waiting
 * again ([52] section 25.3.13, ICLR).
 *
 * @param wanted     RIS bits that mean "proceed".
 * @param timeout_us Phase timeout.
 * @return 1 if a wanted bit appeared, 0 on error or timeout.
 */
static uint8_t ls_i2c_wait_ris(uint32_t wanted, uint32_t timeout_us) {
    const uint32_t started = ls_board_time_us();

    for (;;) {
        const uint32_t ris = ls_reg_read(LS_I2C0_BASE, LS_I2C_O_RIS);

        if ((ris & LS_I2C_INT_ANY_ERROR) != 0UL) {
            return 0U;
        }
        if ((ris & wanted) != 0UL) {
            return 1U;
        }
        if ((ls_board_time_us() - started) > timeout_us) {
            return 0U;
        }
    }
}

/**
 * @brief Clear the given RIS bits via ICLR.
 */
static void ls_i2c_clear_ris(uint32_t bits) {
    ls_reg_write(LS_I2C0_BASE, LS_I2C_O_ICLR, bits);
}

/**
 * @brief Bring the bus back to a defined state after a failed transfer.
 *
 * A half-finished burst leaves bytes in a FIFO and stale sticky status bits;
 * the next transfer would then start from a state nobody reasoned about.  On a
 * safety-relevant part this matters more than the few cycles it costs.
 */
static void ls_i2c_recover(void) {
    ls_reg_write(LS_I2C0_BASE, LS_I2C_O_CFIFOCTL,
                 LS_I2C_CFIFOCTL_BYTEWISE | LS_I2C_CFIFOCTL_TXFLUSH
                     | LS_I2C_CFIFOCTL_RXFLUSH);
    ls_reg_write(LS_I2C0_BASE, LS_I2C_O_CFIFOCTL, LS_I2C_CFIFOCTL_BYTEWISE);
    ls_i2c_clear_ris(LS_I2C_INT_CONTROLLER_ALL);
}

/**
 * @brief Report a failed transfer: recover the bus, then notify the caller.
 */
static pal_status_t ls_i2c_fail(const pal_i2c_t *p_i2c_context) {
    ls_i2c_recover();
    ls_i2c_notify(p_i2c_context, PAL_I2C_EVENT_ERROR);
    return PAL_STATUS_FAILURE;
}

pal_status_t pal_i2c_write(const pal_i2c_t *p_i2c_context, uint8_t *p_data, uint16_t length) {
    const pal_status_t status = ls_i2c_prepare(p_i2c_context, p_data, length);
    uint16_t sent = 0U;

    if (status != PAL_STATUS_SUCCESS) {
        return status;
    }

    ls_i2c_clear_ris(LS_I2C_INT_CONTROLLER_ALL);

    /* [52] section 25.2 step ordering: the first data byte goes into CTXDATA
     * before CCTR starts the transaction, so the controller has something to
     * shift out as soon as the address phase completes. */
    ls_reg_write(LS_I2C0_BASE, LS_I2C_O_CTXDATA, p_data[sent]);
    sent++;

    ls_i2c_start_transaction(p_i2c_context->slave_address, length, 0U);

    while (sent < length) {
        if (!ls_i2c_wait_ris(LS_I2C_INT_CTXFIFOTRG, LS_I2C_PHASE_TIMEOUT_US)) {
            return ls_i2c_fail(p_i2c_context);
        }
        ls_i2c_clear_ris(LS_I2C_INT_CTXFIFOTRG);
        ls_reg_write(LS_I2C0_BASE, LS_I2C_O_CTXDATA, p_data[sent]);
        sent++;
    }

    /* Wait for the burst to complete and the STOP to go out.  CSR.BUSY is the
     * authoritative "transaction finished" signal per [52] section 25.2. */
    if (!ls_i2c_wait_csr_clear(LS_I2C_CSR_BUSY, LS_I2C_PHASE_TIMEOUT_US)) {
        return ls_i2c_fail(p_i2c_context);
    }

    if ((ls_reg_read(LS_I2C0_BASE, LS_I2C_O_CSR) & LS_I2C_CSR_ANY_ERROR) != 0UL) {
        return ls_i2c_fail(p_i2c_context);
    }
    if ((ls_reg_read(LS_I2C0_BASE, LS_I2C_O_RIS) & LS_I2C_INT_ANY_ERROR) != 0UL) {
        return ls_i2c_fail(p_i2c_context);
    }

    ls_i2c_clear_ris(LS_I2C_INT_CONTROLLER_ALL);
    ls_i2c_notify(p_i2c_context, PAL_I2C_EVENT_SUCCESS);
    return PAL_STATUS_SUCCESS;
}

pal_status_t pal_i2c_read(const pal_i2c_t *p_i2c_context, uint8_t *p_data, uint16_t length) {
    const pal_status_t status = ls_i2c_prepare(p_i2c_context, p_data, length);
    uint16_t received = 0U;

    if (status != PAL_STATUS_SUCCESS) {
        return status;
    }

    ls_i2c_clear_ris(LS_I2C_INT_CONTROLLER_ALL);
    ls_i2c_start_transaction(p_i2c_context->slave_address, length, 1U);

    while (received < length) {
        /* CRXFIFOTRG is configured (LS_I2C_CFIFOCTL_BYTEWISE) to assert at
         * ">= 1 byte", so one assertion means exactly one byte is safe to read.
         * An absent or unpowered U7 shows up here as RIS.CNACK on the address
         * phase, which ls_i2c_wait_ris reports as an error rather than letting
         * it burn the full timeout. */
        if (!ls_i2c_wait_ris(LS_I2C_INT_CRXFIFOTRG, LS_I2C_PHASE_TIMEOUT_US)) {
            return ls_i2c_fail(p_i2c_context);
        }
        ls_i2c_clear_ris(LS_I2C_INT_CRXFIFOTRG);
        p_data[received] = (uint8_t)(ls_reg_read(LS_I2C0_BASE, LS_I2C_O_CRXDATA) & 0xFFUL);
        received++;
    }

    if (!ls_i2c_wait_csr_clear(LS_I2C_CSR_BUSY, LS_I2C_PHASE_TIMEOUT_US)) {
        return ls_i2c_fail(p_i2c_context);
    }

    if ((ls_reg_read(LS_I2C0_BASE, LS_I2C_O_CSR) & LS_I2C_CSR_ANY_ERROR) != 0UL) {
        return ls_i2c_fail(p_i2c_context);
    }
    if ((ls_reg_read(LS_I2C0_BASE, LS_I2C_O_RIS) & LS_I2C_INT_ANY_ERROR) != 0UL) {
        return ls_i2c_fail(p_i2c_context);
    }

    ls_i2c_clear_ris(LS_I2C_INT_CONTROLLER_ALL);
    ls_i2c_notify(p_i2c_context, PAL_I2C_EVENT_SUCCESS);
    return PAL_STATUS_SUCCESS;
}

pal_status_t pal_i2c_deinit(const pal_i2c_t *p_i2c_context) {
    (void)p_i2c_context;

    if (!ls_i2c_wait_csr_clear(LS_I2C_CSR_BUSY, LS_I2C_PHASE_TIMEOUT_US)) {
        return PAL_STATUS_I2C_BUSY;
    }

    /* Clear CCR.ACTIVE before removing power: [52] section 25.3.38 warns the bit
     * "should not be set again unless it has been cleared", so leaving it set
     * across a power cycle would make a later re-init unreliable. */
    ls_reg_write(LS_I2C0_BASE, LS_I2C_O_CCR, 0UL);
    ls_reg_write(LS_I2C0_BASE, LS_I2C_O_PWREN, LS_I2C_PWREN_KEY);

    return PAL_STATUS_SUCCESS;
}
