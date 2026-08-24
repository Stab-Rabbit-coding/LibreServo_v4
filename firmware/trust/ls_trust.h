/**
 * @file    ls_trust.h
 * @brief   LibreServo v4 trust layer — the servo's interface to the OPTIGA(TM)
 *          Trust M secure element (U7).
 *
 * This is the API TODO.md 7.2 calls for: "the Trust M driver layer (I2C,
 * Shielded Connection, ECDSA device authentication, ECDHE session-key
 * agreement), honouring the 5 s protected-operation budget from 4.7."
 *
 * ------------------------------------------------------------------------
 * The one rule this API exists to enforce
 * ------------------------------------------------------------------------
 * **Nothing here may be called from the servo control loop.**  Every function
 * below performs at least one protected operation on U7 and blocks for tens to
 * hundreds of milliseconds ([53] section 4.4.3, Table 64).  They belong in boot
 * and commissioning paths only.
 *
 * The division of labour, from PCB/servo-bus-security-protocol.md section 4.7:
 * U7 establishes identity and agrees a session key, once, at boot.  The MCU's
 * own AESADV/CMAC engine ([46] sections 8.18/8.20) authenticates every bus frame
 * thereafter, using the key this layer hands it.  U7 is never in the hot path.
 *
 * References (see REFERENCES.md):
 *   [45] Infineon, OPTIGA(TM) Trust M Datasheet, Rev 3.70.
 *   [46] TI, MSPM0G3519-Q1/MSPM0G3518-Q1 Datasheet, SLASFA6B.
 *   [53] Infineon, OPTIGA(TM) Trust M Solution Reference Manual, Rev 3.70.
 *   [56] Infineon, OPTIGA(TM) Trust M configurations, Configuration Guide, Rev 2.2.
 *   [57] Infineon, OPTIGA(TM) Trust M Host Library for C, Ver 5.8.1.
 *
 * Written by Claude Opus 5 (claude-opus-5) under human direction, 2026-08-23.
 */

#ifndef LS_TRUST_H_
#define LS_TRUST_H_

#include "optiga_lib_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/** Length of a SHA-256 digest, in bytes. */
#define LS_TRUST_DIGEST_LENGTH          (32U)

/** Maximum DER-encoded ECDSA signature length this layer will accept.
 *  [53] section 6.2.3, p. 100 documents the ECDSA signature encoding; a P-384
 *  signature with maximal integer padding is the worst case among the curves
 *  this design permits (PCB/servo-bus-security-protocol.md section 4.9). */
#define LS_TRUST_SIGNATURE_MAX_LENGTH   (128U)

/** Length of the per-session bus authentication key this layer derives.
 *  32 bytes = AES-256, matching the MCU AESADV key width claimed in README.md
 *  and confirmed at [46] section 8.20. */
#define LS_TRUST_SESSION_KEY_LENGTH     (32U)

/** Length of the freshness nonce exchanged with the fleet controller.
 *  [53] section 6.6.3 is explicit that the Shielded Connection alone supplies no
 *  host-driven freshness, so the nonce is not optional. */
#define LS_TRUST_NONCE_LENGTH           (32U)

/**
 * @brief Trust-layer result codes.
 *
 * Distinct from `optiga_lib_status_t` on purpose: the application must be able
 * to tell "this unit was never paired" from "the part is throttled" from "the
 * bus is broken", because those three call for different responses on a
 * manufacturing line and in the field.
 */
typedef enum ls_trust_status {
    /// Operation completed.
    LS_TRUST_OK = 0,
    /// A parameter was NULL or out of range.
    LS_TRUST_ERR_PARAM,
    /// The host library or the part reported a failure.
    LS_TRUST_ERR_OPTIGA,
    /// No platform binding secret is provisioned on the host side.
    /// The unit has not completed the pairing step of [53] section 2.3.4.
    LS_TRUST_ERR_NOT_PAIRED,
    /// U7's security event counter is at or above #LS_TRUST_SEC_WARNING_LEVEL;
    /// protected operations were refused rather than issued into a throttle.
    LS_TRUST_ERR_THROTTLED,
    /// A signature or certificate did not verify.
    LS_TRUST_ERR_VERIFY,
} ls_trust_status_t;

/**
 * @brief Bring up the secure element and establish the Shielded Connection.
 *
 * Sequence, in order:
 *   1. Create the `optiga_util` and `optiga_crypt` instances.
 *   2. Refuse to continue if the host has no platform binding secret — an
 *      unpaired OPTIGA Trust M V3 ships 0xE140 with a **published default value
 *      and read access ALW** ([56] section 2, Table 1), so proceeding would key
 *      the Shielded Connection with a value an attacker can look up.
 *   3. Open the OPTIGA application ([57] `optiga_util_open_application`).
 *   4. Read the security event counter and refuse to go further if the part is
 *      already near the throttle threshold of [53] section 4.6.4.
 *
 * All subsequent traffic runs at OPTIGA_COMMS_FULL_PROTECTION, which is the
 * configured default (firmware/config/optiga_lib_config.h).
 *
 * @retval LS_TRUST_OK              Ready for use.
 * @retval LS_TRUST_ERR_NOT_PAIRED  Run `ls_trust_pair_with_host()` first.
 * @retval LS_TRUST_ERR_THROTTLED   The part is throttled; see the SEC value.
 * @retval LS_TRUST_ERR_OPTIGA      Bus or library failure.
 */
ls_trust_status_t ls_trust_init(void);

/**
 * @brief Close the OPTIGA application and release the instances.
 *
 * @param hibernate  If non-zero, request hibernation.  Note that [53] section 5.1
 *                   costs one hibernate cycle at five tearing-safe programming
 *                   cycles against a 2-million lifetime budget, so this should
 *                   not be used casually.
 */
ls_trust_status_t ls_trust_deinit(uint8_t hibernate);

/**
 * @brief Read the part's security event counter (OID 0xE0C5).
 *
 * Exposed because the throttle is observable and a servo that is silently being
 * slowed by it should say so rather than appear merely sluggish.
 *
 * @param[out] p_sec  Receives the SEC value, 0-255.
 */
ls_trust_status_t ls_trust_read_sec(uint8_t *p_sec);

/**
 * @brief Read the coprocessor UID (OID 0xE0C2) — the servo's hardware serial.
 *
 * @param[out]    p_uid        Destination buffer.
 * @param[in,out] p_uid_length Capacity in, bytes written out.
 */
ls_trust_status_t ls_trust_read_uid(uint8_t *p_uid, uint16_t *p_uid_length);

/**
 * @brief Read the device certificate (OID 0xE0E0) for presentation to the fleet
 *        controller.
 *
 * Costs no security event: the certificate is a data object with read AC = ALW
 * ([53] section 5.4, Table 68), not a key use.
 */
ls_trust_status_t ls_trust_read_device_certificate(uint8_t *p_cert, uint16_t *p_cert_length);

/**
 * @brief Prove this servo's identity by signing a controller-supplied challenge.
 *
 * Hashes @p p_challenge with SHA-256 on the part and signs the digest with the
 * fab-provisioned device private key (OID 0xE0F0).
 *
 * **This costs one "Private key use" security event** ([53] section 4.6.1,
 * Table 65) and about 65 ms plus the Shielded Connection's overhead
 * ([53] section 4.4.3, Table 64).  Call it once per boot, never per frame.
 *
 * @param p_challenge         Controller-supplied challenge bytes.
 * @param challenge_length    Length of the challenge.
 * @param[out] p_signature    DER-encoded signature.
 * @param[in,out] p_signature_length Capacity in, length out.
 */
ls_trust_status_t ls_trust_authenticate(const uint8_t *p_challenge,
                                        uint16_t challenge_length,
                                        uint8_t *p_signature,
                                        uint16_t *p_signature_length);

/**
 * @brief Agree a per-session bus authentication key with the fleet controller.
 *
 * Sequence:
 *   1. Generate an ephemeral ECC key pair on the part, exporting only the public
 *      half, with the private half kept in session context 0xE100
 *      ([53] section 4.4.3 Table 64: ~55 ms).
 *   2. Run ECDH against the controller's ephemeral public key, leaving the
 *      shared secret in the same session context — never exported to the host
 *      (~60 ms).
 *   3. Derive #LS_TRUST_SESSION_KEY_LENGTH bytes from that session-context
 *      secret with TLS v1.2 PRF SHA-256 (~50 ms), exported to the host so it can
 *      be loaded into the MCU's own AESADV key store.
 *
 * Every step operates on the SESSION CONTEXT, which is what keeps them outside
 * the security-event carve-outs of [53] section 4.6.1, Table 65 — each of
 * "Private key use", "Secret key use" and "Key derivation" excludes temporary
 * keys held in a session context.  Only step 1's key generation and the identity
 * signature of `ls_trust_authenticate()` spend budget.
 *
 * @param p_peer_public_key        Controller's ephemeral public key, DER/raw per
 *                                 [53] section 6.2.2.
 * @param peer_public_key_length   Its length.
 * @param p_label                  KDF label, mixed into the derivation.
 * @param label_length             Its length.
 * @param p_nonce                  Freshness nonce, #LS_TRUST_NONCE_LENGTH bytes.
 *                                 Required, not optional: [53] section 6.6.3 states
 *                                 the Shielded Connection carries no host nonce
 *                                 of its own.
 * @param[out] p_own_public_key    This servo's ephemeral public key, to send.
 * @param[in,out] p_own_public_key_length Capacity in, length out.
 * @param[out] p_session_key       Derived key, #LS_TRUST_SESSION_KEY_LENGTH
 *                                 bytes.  The caller MUST load it into AESADV
 *                                 and then wipe this buffer.
 */
ls_trust_status_t ls_trust_establish_session_key(const uint8_t *p_peer_public_key,
                                                 uint16_t peer_public_key_length,
                                                 const uint8_t *p_label,
                                                 uint16_t label_length,
                                                 const uint8_t *p_nonce,
                                                 uint8_t *p_own_public_key,
                                                 uint16_t *p_own_public_key_length,
                                                 uint8_t *p_session_key);

/**
 * @brief Draw random bytes from the part's TRNG.
 *
 * Used for the host-side half of the freshness nonce.  Costs no security event.
 */
ls_trust_status_t ls_trust_random(uint8_t *p_buffer, uint16_t length);

/**
 * @brief Pair this host with U7 by provisioning the platform binding secret.
 *
 * **Manufacturing step.  Runs exactly once per unit, and it is not reversible.**
 * Implements [53] section 2.3.4, Figure 12 "Pair OPTIGA Trust M with host
 * (pre-shared secret based)".  See `ls_trust_pairing.c` for the full sequence
 * and the access-condition choice it commits to.
 *
 * Must be called *before* `ls_trust_init()`, on a unit whose 0xE140 life-cycle
 * state is still below operational.
 *
 * @retval LS_TRUST_OK          The unit is now paired.
 * @retval LS_TRUST_ERR_OPTIGA  Pairing failed; the unit's state is described in
 *                              the function's own documentation.
 */
ls_trust_status_t ls_trust_pair_with_host(void);

#ifdef __cplusplus
}
#endif

#endif /* LS_TRUST_H_ */
