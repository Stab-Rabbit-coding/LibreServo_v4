/**
 * @file    ls_pal_ifx_i2c_config.c
 * @brief   The three PAL context objects [57] `pal_ifx_i2c_config.h` declares
 *          `extern`, defined for the LibreServo v4 board.
 *
 * References (see REFERENCES.md):
 *   [45] Infineon, OPTIGA(TM) Trust M Datasheet, Rev 3.70.
 *   [53] Infineon, OPTIGA(TM) Trust M Solution Reference Manual, Rev 3.70.
 *   [57] Infineon, OPTIGA(TM) Trust M Host Library for C, Ver 5.8.1.
 *
 * Written by Claude Opus 5 (claude-opus-5) under human direction, 2026-08-23.
 */

#include "pal_ifx_i2c_config.h"

#include "ls_board.h"

/** Defined in `ls_pal_gpio.c`; declared here rather than in a header because
 *  this file is its only consumer.  Non-const to match [57]'s `void *`
 *  `p_gpio_hw` field — see the note on the definition. */
extern struct ls_gpio_pin ls_gpio_se_rst;

/**
 * The single I2C context, addressing U7.
 *
 * `p_i2c_hw_config` is NULL because this board has exactly one I2C instance
 * dedicated to the secure element and `ls_pal_i2c.c` resolves it from
 * `ls_board.h` rather than from the context.  `p_upper_layer_ctx` and
 * `upper_layer_event_handler` are filled in by the library at run time.
 *
 * `slave_address` is 0x30 per [53] section 4.4, p. 48, Table 35 ("BASE_ADDR |
 * 0x30 | I2C base address default").
 */
pal_i2c_t optiga_pal_i2c_context_0 = {
    .p_i2c_hw_config = NULL,
    .p_upper_layer_ctx = NULL,
    .upper_layer_event_handler = NULL,
    .slave_address = LS_SE_I2C_ADDRESS,
};

/**
 * VDD control — intentionally absent on this board.
 *
 * U7's VCC is a hard-wired +3V3 rail with only decoupling (C43); there is no
 * load switch (PCB/OPTIGA-Trust-M-secure-element.md section 2).  `ls_pal_gpio.c`
 * treats a NULL `p_gpio_hw` as "no such pin" and succeeds, so the library's cold
 * reset path degrades cleanly — and the configuration selects a warm reset
 * anyway (OPTIGA_COMMS_DEFAULT_RESET_TYPE = 2).
 *
 * This is also the safer arrangement: [53] section 4.6.4, p. 77 caps VCC off/on
 * cycling at 200 000 times over the part's lifetime and advises against removing
 * power before the security event counter reaches 0.
 */
pal_gpio_t optiga_vdd_0 = {
    .p_gpio_hw = NULL,
};

/**
 * SE_RST — U1 pin 18 (PA14) to U7 pin 9 (RST).
 */
pal_gpio_t optiga_reset_0 = {
    .p_gpio_hw = &ls_gpio_se_rst,
};
