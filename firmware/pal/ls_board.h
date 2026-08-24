/**
 * @file    ls_board.h
 * @brief   LibreServo v4.0.0 board facts the OPTIGA(TM) Trust M PAL port needs,
 *          plus the small time-base contract the PAL expects the application
 *          to satisfy.
 *
 * This header is where the schematic meets the firmware.  Every pin, net and
 * IOMUX encoding below was read from a primary source in this session and is
 * cited inline; none is carried over from a similar design.
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

#ifndef LS_BOARD_H_
#define LS_BOARD_H_

#include <stdint.h>

#include "mspm0/ls_mspm0_i2c_regs.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ------------------------------------------------------------------------
 * Secure element (U7) net map
 *
 * Schematic: PCB/kicad/LibreServo-v4.0.0.kicad_sch.
 * Wiring record: PCB/OPTIGA-Trust-M-secure-element.md section 3, TODO.md 3.1/4.3.
 * ------------------------------------------------------------------------ */

/** U7 I2C target address.  [53] section 4.4, p. 48, Table 35: "BASE_ADDR | 0x30 |
 *  I2C base address default".  7-bit; the MSPM0 CSA.TADDR field takes the 7-bit
 *  value directly (the field position does the shifting, not the caller). */
#define LS_SE_I2C_ADDRESS           (0x30U)

/** SCL bus frequency.  Fast-mode.  [45] p. 7 section 1.5 rates the part to 1 MHz
 *  (FM+), and [52] section 25.1.2 lists Fm+ support on the controller side, so
 *  this is a deliberate choice to run slower than either part allows:
 *
 *    - [53] section 4.4.3 Table 64 measures every published command timing at
 *      "I2C FM mode (400kHz)", so 400 kHz is the only speed at which this
 *      design's latency budget (PCB/servo-bus-security-protocol.md section 4.7)
 *      can be checked against a published figure rather than an extrapolation;
 *    - the pull-ups are R30/R31 at 10 kOhm, which [45] p. 12 notes must be
 *      matched to bus capacitance and frequency.  10 kOhm is a comfortable Fm
 *      value and a marginal Fm+ one on an unmeasured board.
 *
 *  Raising this to 1 MHz requires re-checking the pull-ups against the
 *  fabricated board's measured bus capacitance first — see TODO.md 5.3. */
#define LS_SE_I2C_SCL_HZ            (400000UL)

/** Assumed I2C functional clock, in Hz, after CLKSEL/CLKDIV ([52] section 25.2.1).
 *
 *  UNVERIFIED — needs primary source (see TODO.md): this design has no clock-tree
 *  configuration yet (TODO.md 7.1: the firmware is a full rewrite and nothing in
 *  Src/ reflects the current MCU), so the functional clock this peripheral will
 *  actually see is not yet a settled fact.  32 MHz is used here because it is the
 *  value [52] section 25.2.1 works its own TPR example with, which makes the
 *  derived TPR checkable against the document.  `pal_i2c_init()` recomputes TPR
 *  from whatever this holds; correct this constant when the clock tree is
 *  designed — never hand-patch the TPR.  Tracked as TODO.md 7.3. */
#define LS_I2C_FUNCTIONAL_CLK_HZ    (32000000UL)

/* ------------------------------------------------------------------------
 * IOMUX pin control — [46] section 6, Table 6-2, read with column-preserving
 * extraction (`pdftotext -layout`); the addresses below are the table's own
 * absolute "IOMUX ADDR" column, not computed from an index.
 * ------------------------------------------------------------------------ */

/** SE_I2C_SCL — PA1, package pin 2 (RHB-32), PINCM2 at 0x40428004.
 *  [46] Table 6-2 (p. 15): PA1 IOMUX PF 3 = `I2C0_SCL`, signal type IOD. */
#define LS_PINCM_SE_I2C_SCL         (0x40428004UL)
#define LS_PF_SE_I2C_SCL            (3U)

/** SE_I2C_SDA — PA8, package pin 12 (RHB-32), PINCM19 at 0x40428048.
 *  [46] Table 6-2 (p. 18): PA8 IOMUX **PF 4** = `I2C0_SDA`, signal type IOD.
 *
 *  NOTE, and it is a correction: TODO.md 3.1 and
 *  PCB/OPTIGA-Trust-M-secure-element.md section 3 both recorded this as "PF3".
 *  PA8's PF3 is `SPI0_CS0`; `I2C0_SDA` is PF4.  Verified 2026-08-23 against
 *  Table 6-2 p. 18 with `pdftotext -layout`.  The pin selection is unaffected —
 *  PA8 is still the I2C0 SDA pin and the schematic is still correct — but
 *  firmware written from the "PF3" figure would mux an SPI chip-select onto the
 *  secure element's data line and the bus would never come up.  Tracked as
 *  TODO.md 3.4. */
#define LS_PINCM_SE_I2C_SDA         (0x40428048UL)
#define LS_PF_SE_I2C_SDA            (4U)

/** SE_RST — PA14, package pin 18 (RHB-32), PINCM36 at 0x4042808C.
 *  [46] Table 6-2: PA14 IOMUX PF 1 = `PA14` (plain GPIO), signal type IO. */
#define LS_PINCM_SE_RST             (0x4042808CUL)
#define LS_PF_SE_RST                (1U)
/** GPIOA bit position of PA14, for the DOUT/DOE registers of [52] Table 9-2. */
#define LS_GPIOA_PIN_SE_RST         (14U)

/* ------------------------------------------------------------------------
 * PINCM bit fields — [52] section 8.3.1, pp. 619-620, Table 8-5
 * ------------------------------------------------------------------------ */

/** Bits 5-0: peripheral function selection. */
#define LS_PINCM_PF_Msk             (0x3FUL)
/** Bit 7: "Peripheral is Connected" — the output latch becomes transparent. */
#define LS_PINCM_PC                 (1UL << 7)
/** Bit 16: pull-down enable. */
#define LS_PINCM_PIPD               (1UL << 16)
/** Bit 17: pull-up enable. */
#define LS_PINCM_PIPU               (1UL << 17)
/** Bit 18: input enable.  [52] section 8.2.1 requires PC and INENA be set
 *  together with the PF field when a peripheral function is assigned. */
#define LS_PINCM_INENA              (1UL << 18)
/** Bit 19: input hysteresis enable. */
#define LS_PINCM_HYSTEN             (1UL << 19)
/** Bit 25: "High output value will tri-state the output when this bit is
 *  enabled" — i.e. open-drain.  Required on both I2C lines: the bus is
 *  wired-AND and R30/R31 provide the pull-up ([45] p. 12 section 3 Figure 2). */
#define LS_PINCM_HIZ1               (1UL << 25)

/* ------------------------------------------------------------------------
 * GPIO register offsets — [52] section 9.3, pp. 626-627, Table 9-2
 * ------------------------------------------------------------------------ */

#define LS_GPIO_O_PWREN             (0x0800UL)  /**< section 9.3.5, p. 632 */
#define LS_GPIO_O_RSTCTL            (0x0804UL)  /**< section 9.3.6, p. 633 */
#define LS_GPIO_O_DOUT31_0          (0x1280UL)
#define LS_GPIO_O_DOUTSET31_0       (0x1290UL)
#define LS_GPIO_O_DOUTCLR31_0       (0x12A0UL)
#define LS_GPIO_O_DOE31_0           (0x12C0UL)
#define LS_GPIO_O_DOESET31_0        (0x12D0UL)
#define LS_GPIO_O_DOECLR31_0        (0x12E0UL)
#define LS_GPIO_O_DIN31_0           (0x1380UL)

/** GPIO PWREN write key, [52] section 9.3.5, p. 632: "26h = KEY to allow write
 *  access to this register" — the same key value the I2C peripheral uses, but
 *  read from the GPIO chapter rather than assumed to generalize. */
#define LS_GPIO_PWREN_KEY           (0x26UL << 24)
#define LS_GPIO_PWREN_ENABLE        (1UL << 0)
/** GPIO RSTCTL write key, [52] section 9.3.6, p. 633: "B1h = KEY to allow write
 *  access to this register". */
#define LS_GPIO_RSTCTL_KEY          (0xB1UL << 24)
#define LS_GPIO_RSTCTL_RESETASSERT  (1UL << 0)
#define LS_GPIO_RSTCTL_RESETSTKYCLR (1UL << 1)

/* ------------------------------------------------------------------------
 * Time base contract
 *
 * The OPTIGA PAL needs a microsecond time base and a one-shot deferred call.
 * This port deliberately does NOT seize a hardware timer to provide them.
 *
 * Reason: this is a servo controller.  The control loop owns timer allocation,
 * and TODO.md 7.1 records that the control loop does not exist yet (Src/ still
 * targets the superseded STM32 part).  A PAL that grabbed a TIMG instance now
 * would be making a system-level resource decision on behalf of firmware nobody
 * has written, and the conflict would surface late and expensively.
 *
 * The three functions below are the entire contract.  Everything else in
 * firmware/pal/ is complete and needs no vendor SDK.
 * ------------------------------------------------------------------------ */

/**
 * @brief Free-running microsecond counter.
 *
 * Must be monotonic, must not stall while interrupts are masked for the short
 * critical sections in `ls_pal_os_lock.c`, and is allowed to wrap.  Callers in
 * this port compare elapsed intervals using unsigned subtraction, which is
 * wrap-safe for any interval shorter than a full period.
 *
 * @return Microseconds since an arbitrary but fixed origin.
 */
uint32_t ls_board_time_us(void);

/**
 * @brief Register a single deferred callback, replacing any pending one.
 *
 * The host library's `pal_os_event` keeps exactly one outstanding one-shot at a
 * time ([57] `pal_os_event.h`: `pal_os_event_register_callback_oneshot`), so a
 * single slot is sufficient and a queue would be dead weight.
 *
 * The callback may run from interrupt or main-loop context, at the
 * application's choice, but must not run re-entrantly with respect to itself.
 *
 * @param callback  Function to invoke.  Never NULL when called from this port.
 * @param context   Opaque argument handed back to @p callback.
 * @param delay_us  Minimum delay before invocation, in microseconds.
 */
void ls_board_schedule_oneshot_us(void (*callback)(void *), void *context, uint32_t delay_us);

/**
 * @brief Cancel a pending one-shot registered by
 *        #ls_board_schedule_oneshot_us, if any.
 *
 * Must be safe to call when nothing is pending.
 */
void ls_board_cancel_oneshot(void);

#ifdef __cplusplus
}
#endif

#endif /* LS_BOARD_H_ */
