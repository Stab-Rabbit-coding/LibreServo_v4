/**
 * @file    ls_pal_gpio.c
 * @brief   OPTIGA(TM) Trust M PAL GPIO implementation for the TI MSPM0G3518-Q1.
 *
 * Implements the `pal_gpio` contract of [57] `include/pal/pal_gpio.h`.  On this
 * board the only GPIO the host library drives is SE_RST — U1 pin 18 (PA14) to
 * U7 pin 9 (RST), per PCB/OPTIGA-Trust-M-secure-element.md section 3.
 *
 * There is deliberately NO VDD GPIO.  U7's VCC is a hard-wired +3V3 rail with
 * only decoupling (C43); `optiga_vdd_0` in `ls_pal_ifx_i2c_config.c` is a null
 * context for that reason, and the library is configured for a WARM reset
 * (OPTIGA_COMMS_DEFAULT_RESET_TYPE = 2 in firmware/config/optiga_lib_config.h).
 *
 * References (see REFERENCES.md):
 *   [46] TI, MSPM0G3519-Q1/MSPM0G3518-Q1 Datasheet, SLASFA6B.
 *   [52] TI, MSPM0 G-Series Technical Reference Manual, SLAU846E.
 *   [57] Infineon, OPTIGA(TM) Trust M Host Library for C, Ver 5.8.1.
 *
 * Written by Claude Opus 5 (claude-opus-5) under human direction, 2026-08-23.
 */

#include "pal_gpio.h"

#include "ls_board.h"

/**
 * Platform-specific GPIO descriptor, pointed at by `pal_gpio_t.p_gpio_hw`.
 *
 * The PAL header types that field as `void *` and leaves its content entirely
 * to the port ([57] `pal_gpio.h`), so this shape is local to LibreServo.
 */
typedef struct ls_gpio_pin {
    /// GPIO port base address, e.g. #LS_GPIOA_BASE.
    uint32_t port_base;
    /// Bit position within the port's DOUT/DOE registers.
    uint32_t pin;
    /// Absolute address of the pin's PINCM register, from [46] Table 6-2.
    uint32_t pincm_address;
    /// IOMUX peripheral-function value to select plain GPIO on this pin.
    uint32_t pincm_pf;
} ls_gpio_pin_t;

/**
 * SE_RST descriptor.  Referenced by `optiga_reset_0` in
 * `ls_pal_ifx_i2c_config.c`.
 *
 * Not declared `const`, even though nothing ever writes it: [57] types
 * `pal_gpio_t.p_gpio_hw` as a plain `void *`, and a `const` object here would
 * force a const-stripping cast into a static initializer, which C does not
 * allow to be constant-folded.  Matching the library's own qualification is
 * less code and fewer places to get wrong than working around it.
 */
ls_gpio_pin_t ls_gpio_se_rst = {
    .port_base = LS_GPIOA_BASE,
    .pin = LS_GPIOA_PIN_SE_RST,
    .pincm_address = LS_PINCM_SE_RST,
    .pincm_pf = LS_PF_SE_RST,
};

pal_status_t pal_gpio_init(const pal_gpio_t *p_gpio_context) {
    const ls_gpio_pin_t *pin;

    /* A null context is legitimate and means "this board has no such pin" —
     * `optiga_vdd_0` is exactly that case.  Reporting success keeps the
     * library's own init path from treating an intentionally absent VDD control
     * as a hardware fault. */
    if ((p_gpio_context == NULL) || (p_gpio_context->p_gpio_hw == NULL)) {
        return PAL_STATUS_SUCCESS;
    }
    pin = (const ls_gpio_pin_t *)p_gpio_context->p_gpio_hw;

    /* Power and un-reset the GPIO port.  [52] section 9.3.6, p. 633 (RSTCTL, key
     * 0xB1) and section 9.3.5, p. 632 (PWREN, key 0x26). */
    ls_reg_write(pin->port_base, LS_GPIO_O_RSTCTL,
                 LS_GPIO_RSTCTL_KEY | LS_GPIO_RSTCTL_RESETASSERT
                     | LS_GPIO_RSTCTL_RESETSTKYCLR);
    ls_reg_write(pin->port_base, LS_GPIO_O_PWREN,
                 LS_GPIO_PWREN_KEY | LS_GPIO_PWREN_ENABLE);

    /* Drive the line high (U7 out of reset) BEFORE enabling the output, so
     * enabling the driver cannot glitch RST low.  [45] p. 17 Table 6 makes RST
     * an active-low reset input; a spurious low here would reset the secure
     * element mid-session. */
    ls_reg_write(pin->port_base, LS_GPIO_O_DOUTSET31_0, 1UL << pin->pin);

    /* Select the plain-GPIO peripheral function.  [52] section 8.2.1: the PF value
     * is written together with PC and INENA. */
    *(volatile uint32_t *)(uintptr_t)pin->pincm_address =
        (pin->pincm_pf & LS_PINCM_PF_Msk) | LS_PINCM_PC | LS_PINCM_INENA;

    /* Enable the output driver. */
    ls_reg_write(pin->port_base, LS_GPIO_O_DOESET31_0, 1UL << pin->pin);

    return PAL_STATUS_SUCCESS;
}

void pal_gpio_set_high(const pal_gpio_t *p_gpio_context) {
    const ls_gpio_pin_t *pin;

    if ((p_gpio_context == NULL) || (p_gpio_context->p_gpio_hw == NULL)) {
        return;
    }
    pin = (const ls_gpio_pin_t *)p_gpio_context->p_gpio_hw;

    /* DOUTSET31_0 is a write-1-to-set register ([52] section 9.3, Table 9-2), so
     * this is atomic with respect to other pins on the same port — no
     * read-modify-write race against the servo's own GPIO users. */
    ls_reg_write(pin->port_base, LS_GPIO_O_DOUTSET31_0, 1UL << pin->pin);
}

void pal_gpio_set_low(const pal_gpio_t *p_gpio_context) {
    const ls_gpio_pin_t *pin;

    if ((p_gpio_context == NULL) || (p_gpio_context->p_gpio_hw == NULL)) {
        return;
    }
    pin = (const ls_gpio_pin_t *)p_gpio_context->p_gpio_hw;

    ls_reg_write(pin->port_base, LS_GPIO_O_DOUTCLR31_0, 1UL << pin->pin);
}

pal_status_t pal_gpio_deinit(const pal_gpio_t *p_gpio_context) {
    const ls_gpio_pin_t *pin;

    if ((p_gpio_context == NULL) || (p_gpio_context->p_gpio_hw == NULL)) {
        return PAL_STATUS_SUCCESS;
    }
    pin = (const ls_gpio_pin_t *)p_gpio_context->p_gpio_hw;

    /* Release the driver but leave the line high first, so U7 is not left held
     * in reset by a floating pin with no defined pull. */
    ls_reg_write(pin->port_base, LS_GPIO_O_DOUTSET31_0, 1UL << pin->pin);
    ls_reg_write(pin->port_base, LS_GPIO_O_DOECLR31_0, 1UL << pin->pin);

    return PAL_STATUS_SUCCESS;
}
