/**
 * @file    ls_secure_store.h
 * @brief   The one place LibreServo v4 decides where the platform binding
 *          secret physically lives.
 *
 * `ls_pal_os_datastore.c` calls these; nothing else should.  Keeping the
 * storage-medium decision behind this interface means TODO.md 4.13 can be
 * settled — flash region, protection scheme, provisioning tooling — without
 * touching the OPTIGA porting layer.
 *
 * See the long comment at the top of `ls_pal_os_datastore.c` for why the MSPM0
 * KEYSTORE cannot be the answer, and what it IS the right answer for.
 *
 * References (see REFERENCES.md):
 *   [53] Infineon, OPTIGA(TM) Trust M Solution Reference Manual, Rev 3.70.
 *   [57] Infineon, OPTIGA(TM) Trust M Host Library for C, Ver 5.8.1.
 *
 * Written by Claude Opus 5 (claude-opus-5) under human direction, 2026-08-23.
 */

#ifndef LS_SECURE_STORE_H_
#define LS_SECURE_STORE_H_

#include "pal.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Minimum acceptable platform binding secret length, in bytes.
 *
 * [53] section 6.5.8, p. 107: "The recommended length of platform binding shared
 * secret is 32 bytes or more."  This design takes the recommendation as a floor
 * and enforces it in `pal_os_datastore_write`.
 *
 * The upper bound is the library's own #OPTIGA_SHARED_SECRET_MAX_LENGTH (64).
 * `ls_trust_pairing.c` provisions the full 64 bytes.
 */
#define LS_PLATFORM_BINDING_SECRET_MIN_LENGTH (32U)

/**
 * Size of the RAM-resident Shielded Connection manage context.
 *
 * The host library does not publish a required size for this blob; it writes
 * whatever the presentation layer needs and reads it back.  64 bytes is chosen
 * as a bound comfortably above #OPTIGA_SHARED_SECRET_MAX_LENGTH, and
 * `pal_os_datastore_write` rejects anything longer rather than overflowing —
 * so if a future host-library revision needs more, it fails visibly at bring-up
 * instead of corrupting adjacent RAM.
 */
#define LS_MANAGE_CONTEXT_SIZE (64U)

/**
 * @brief Read the platform binding secret into @p p_buffer.
 *
 * Must fail rather than return a default, a zero-fill, or a partial value:
 * every one of those would silently key the Shielded Connection with something
 * an attacker can predict.
 *
 * @param[out]    p_buffer        Destination.
 * @param[in,out] p_buffer_length On entry, capacity; on success, bytes written.
 * @retval PAL_STATUS_SUCCESS     A provisioned secret was returned in full.
 * @retval PAL_STATUS_FAILURE     No secret is provisioned, or it could not be
 *                                read, or the buffer is too small.
 */
pal_status_t ls_secure_store_read_platform_binding_secret(uint8_t *p_buffer,
                                                          uint16_t *p_buffer_length);

/**
 * @brief Persist the platform binding secret.
 *
 * Called exactly once per unit, from `ls_trust_pairing.c`, during the
 * manufacturing pairing step of [53] section 2.3.4.  The implementation is
 * responsible for making the write durable before returning success — if the
 * OPTIGA side of the pairing has already committed and the host side has not,
 * the two are unpaired and the part is bricked for this purpose.
 *
 * @param p_buffer Secret bytes.
 * @param length   Length, already range-checked by the caller.
 */
pal_status_t ls_secure_store_write_platform_binding_secret(const uint8_t *p_buffer,
                                                           uint16_t length);

/**
 * @brief Report whether a platform binding secret is provisioned.
 *
 * Lets the trust layer distinguish "this unit has never been paired" from
 * "pairing failed", which are different manufacturing outcomes.
 *
 * @retval 1 A secret is present.
 * @retval 0 No secret is present.
 */
uint8_t ls_secure_store_has_platform_binding_secret(void);

/**
 * @brief Erase the platform binding secret.
 *
 * Intended for decommissioning a unit, not for normal operation.  Erasing it
 * makes the paired OPTIGA unusable by this host, which is the point.
 */
pal_status_t ls_secure_store_erase_platform_binding_secret(void);

#ifdef __cplusplus
}
#endif

#endif /* LS_SECURE_STORE_H_ */
