/**
 * @file    ls_pal_logger.c
 * @brief   OPTIGA(TM) Trust M PAL logger implementation for LibreServo v4.
 *
 * Implements [57] `include/pal/pal_logger.h` as a deliberate no-op sink.
 *
 * This is a security decision, not laziness.  The host library's per-layer log
 * switches (OPTIGA_LIB_ENABLE_COMMS_LOGGING and friends) dump APDUs — the very
 * bytes the Shielded Connection exists to keep off the wire ([53] section 6.6).
 * A servo that shipped with a UART echoing those would have paid the Shielded
 * Connection's latency cost ([53] section 4.4.3 Table 64) and thrown away the
 * benefit.
 *
 * All four per-layer switches are commented out in
 * `firmware/config/optiga_lib_config.h`, so nothing calls into here in a
 * production build.  A bench build that wants tracing should point
 * `logger_config_ptr` at a real UART in a local, uncommitted edit — deliberately
 * not made convenient.
 *
 * Written by Claude Opus 5 (claude-opus-5) under human direction, 2026-08-23.
 */

#include "pal_logger.h"

/**
 * The logger instance the host library references by name.
 *
 * [57] `src/common/optiga_lib_logger.c` declares `extern pal_logger_t
 * logger_console;` and passes its address to `pal_logger_write`.  It is
 * therefore a link-time requirement of the library whenever
 * OPTIGA_LIB_ENABLE_LOGGING is defined, not an optional convenience — omitting
 * it produces an "undefined reference to `logger_console'" at link.
 *
 * `logger_config_ptr` is NULL because every entry point in this file discards
 * its input; see the file header for why that is deliberate.
 */
pal_logger_t logger_console = {
    .logger_config_ptr = NULL,
    .logger_rx_flag = 0U,
    .logger_tx_flag = 0U,
};

pal_status_t pal_logger_init(void *p_logger_context) {
    (void)p_logger_context;
    return PAL_STATUS_SUCCESS;
}

pal_status_t pal_logger_deinit(void *p_logger_context) {
    (void)p_logger_context;
    return PAL_STATUS_SUCCESS;
}

pal_status_t
pal_logger_write(void *p_logger_context, const uint8_t *p_log_data, uint32_t log_data_length) {
    (void)p_logger_context;
    (void)p_log_data;
    (void)log_data_length;
    /* Report success: a logging failure must never be able to fail a
     * cryptographic operation. */
    return PAL_STATUS_SUCCESS;
}

pal_status_t
pal_logger_read(void *p_logger_context, uint8_t *p_log_data, uint32_t log_data_length) {
    (void)p_logger_context;
    (void)p_log_data;
    (void)log_data_length;
    return PAL_STATUS_FAILURE;
}
