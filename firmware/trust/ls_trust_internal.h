/**
 * @file    ls_trust_internal.h
 * @brief   Shared internals of the LibreServo v4 trust layer.
 *
 * Private to firmware/trust/.  Not part of the API in `ls_trust.h`.
 *
 * Written by Claude Opus 5 (claude-opus-5) under human direction, 2026-08-23.
 */

#ifndef LS_TRUST_INTERNAL_H_
#define LS_TRUST_INTERNAL_H_

#include "ls_trust.h"
#include "optiga_crypt.h"
#include "optiga_util.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Upper bound on how long the trust layer waits for one asynchronous OPTIGA
 * operation to complete.
 *
 * Sized from the part, not guessed.  The slowest operation this design can
 * issue is `GenKeyPair` for ECC P-256 at ~55 ms ([53] section 4.4.3, Table 64) —
 * but a protected operation issued while the security monitor is throttling can
 * be delayed by up to t_max, 5 s ([53] section 4.6.4).  6 s therefore covers the
 * worst legitimate case with margin, and anything beyond it is a genuinely
 * stuck part rather than a slow one.
 *
 * RSA operations, which would blow this budget (RSA-2048 key generation is
 * "minimum 2900 ms"), are compiled out entirely — see
 * firmware/config/optiga_lib_config.h.
 */
#define LS_TRUST_OPERATION_TIMEOUT_MS (6000U)

/** Shared asynchronous-completion state, set by the library callback. */
extern volatile optiga_lib_status_t ls_trust_lib_status;

/** The two service-layer instances.  Created by `ls_trust_init()` and by
 *  `ls_trust_pair_with_host()`, which runs before init. */
extern optiga_util_t *ls_trust_util;
extern optiga_crypt_t *ls_trust_crypt;

/**
 * @brief Completion callback registered with both instances.
 */
void ls_trust_callback(void *context, optiga_lib_status_t return_status);

/**
 * @brief Block until the in-flight asynchronous operation completes.
 *
 * @param submit_status The value the optiga_* call itself returned.
 * @retval LS_TRUST_OK          The operation completed successfully.
 * @retval LS_TRUST_ERR_OPTIGA  Submission failed, the operation failed, or it
 *                              did not complete within
 *                              #LS_TRUST_OPERATION_TIMEOUT_MS.
 */
ls_trust_status_t ls_trust_await(optiga_lib_status_t submit_status);

/**
 * @brief Overwrite a buffer that held key material.
 *
 * Written through a `volatile` pointer so the compiler may not treat the store
 * as dead and remove it — the usual failure mode of a plain `memset` used for
 * zeroisation, and one that would leave a derived session key sitting in SRAM.
 */
void ls_trust_secure_wipe(void *p_buffer, uint16_t length);

#ifdef __cplusplus
}
#endif

#endif /* LS_TRUST_INTERNAL_H_ */
