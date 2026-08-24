/**
 * @file    ls_trust.c
 * @brief   LibreServo v4 trust layer implementation (TODO.md 7.2).
 *
 * See `ls_trust.h` for the API contract and for the rule that none of this may
 * run in the servo control loop.
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

#include "ls_trust.h"

#include <string.h>

#include "ls_secure_store.h"
#include "ls_trust_internal.h"
#include "ls_trust_oid.h"
#include "optiga_lib_common.h"
#include "pal_os_timer.h"

volatile optiga_lib_status_t ls_trust_lib_status;
optiga_util_t *ls_trust_util;
optiga_crypt_t *ls_trust_crypt;

/** Set once `ls_trust_init()` has opened the application. */
static uint8_t ls_trust_application_open;

void ls_trust_callback(void *context, optiga_lib_status_t return_status) {
    (void)context;
    ls_trust_lib_status = return_status;
}

void ls_trust_secure_wipe(void *p_buffer, uint16_t length) {
    volatile uint8_t *p = (volatile uint8_t *)p_buffer;
    uint16_t i;

    if (p_buffer == NULL) {
        return;
    }
    for (i = 0U; i < length; i++) {
        p[i] = 0U;
    }
}

ls_trust_status_t ls_trust_await(optiga_lib_status_t submit_status) {
    const uint32_t started_ms = pal_os_timer_get_time_in_milliseconds();

    if (submit_status != OPTIGA_LIB_SUCCESS) {
        return LS_TRUST_ERR_OPTIGA;
    }

    while (ls_trust_lib_status == OPTIGA_LIB_BUSY) {
        /* Unsigned subtraction keeps this correct across a millisecond-counter
         * wrap. */
        if ((pal_os_timer_get_time_in_milliseconds() - started_ms)
            > LS_TRUST_OPERATION_TIMEOUT_MS) {
            return LS_TRUST_ERR_OPTIGA;
        }
    }

    return (ls_trust_lib_status == OPTIGA_LIB_SUCCESS) ? LS_TRUST_OK : LS_TRUST_ERR_OPTIGA;
}

/**
 * @brief Create both service instances if they do not already exist.
 */
static ls_trust_status_t ls_trust_create_instances(void) {
    if (ls_trust_util == NULL) {
        ls_trust_util = optiga_util_create(0, ls_trust_callback, NULL);
        if (ls_trust_util == NULL) {
            return LS_TRUST_ERR_OPTIGA;
        }
    }
    if (ls_trust_crypt == NULL) {
        ls_trust_crypt = optiga_crypt_create(0, ls_trust_callback, NULL);
        if (ls_trust_crypt == NULL) {
            return LS_TRUST_ERR_OPTIGA;
        }
    }
    return LS_TRUST_OK;
}

/**
 * @brief Read one data object at full Shielded Connection protection.
 */
static ls_trust_status_t
ls_trust_read_object(uint16_t oid, uint8_t *p_buffer, uint16_t *p_length) {
    optiga_lib_status_t submitted;

    if ((p_buffer == NULL) || (p_length == NULL) || (*p_length == 0U)) {
        return LS_TRUST_ERR_PARAM;
    }
    if (ls_trust_util == NULL) {
        return LS_TRUST_ERR_OPTIGA;
    }

    ls_trust_lib_status = OPTIGA_LIB_BUSY;
    OPTIGA_UTIL_SET_COMMS_PROTECTION_LEVEL(ls_trust_util, OPTIGA_COMMS_FULL_PROTECTION);
    submitted = optiga_util_read_data(ls_trust_util, oid, 0U, p_buffer, p_length);

    return ls_trust_await(submitted);
}

ls_trust_status_t ls_trust_read_sec(uint8_t *p_sec) {
    uint8_t buffer[1];
    uint16_t length = (uint16_t)sizeof(buffer);
    ls_trust_status_t status;

    if (p_sec == NULL) {
        return LS_TRUST_ERR_PARAM;
    }

    status = ls_trust_read_object(LS_OID_SECURITY_EVENT_COUNTER, buffer, &length);
    if (status != LS_TRUST_OK) {
        return status;
    }
    if (length != 1U) {
        return LS_TRUST_ERR_OPTIGA;
    }

    *p_sec = buffer[0];
    return LS_TRUST_OK;
}

ls_trust_status_t ls_trust_init(void) {
    optiga_lib_status_t submitted;
    ls_trust_status_t status;
    uint8_t sec = 0U;

    status = ls_trust_create_instances();
    if (status != LS_TRUST_OK) {
        return status;
    }

    /* Refuse to open a session that would be keyed by a value an attacker can
     * look up.  [56] section 2 Table 1: on a V3 part 0xE140 ships with value
     * "Default" and read AC "ALW" until the pairing of [53] section 2.3.4 has run.
     * Checking the HOST side is the right test — if the host has no secret, the
     * Shielded Connection cannot be keyed correctly regardless of what U7
     * holds. */
    if (ls_secure_store_has_platform_binding_secret() == 0U) {
        return LS_TRUST_ERR_NOT_PAIRED;
    }

    /* Open the OPTIGA application.  perform_restore = 0: this design does not
     * hibernate (see ls_trust_deinit). */
    ls_trust_lib_status = OPTIGA_LIB_BUSY;
    OPTIGA_UTIL_SET_COMMS_PROTECTION_LEVEL(ls_trust_util, OPTIGA_COMMS_NO_PROTECTION);
    submitted = optiga_util_open_application(ls_trust_util, 0);
    status = ls_trust_await(submitted);
    if (status != LS_TRUST_OK) {
        return status;
    }
    ls_trust_application_open = 1U;

    /* Check the throttle budget before anything spends it.  A part that arrives
     * at boot already near the threshold of [53] section 4.6.4 will make every
     * subsequent protected operation slow, and a servo that reports that
     * plainly is far easier to diagnose than one that merely boots late. */
    status = ls_trust_read_sec(&sec);
    if (status != LS_TRUST_OK) {
        return status;
    }
    if (sec >= LS_TRUST_SEC_WARNING_LEVEL) {
        return LS_TRUST_ERR_THROTTLED;
    }

    return LS_TRUST_OK;
}

ls_trust_status_t ls_trust_deinit(uint8_t hibernate) {
    optiga_lib_status_t submitted;
    ls_trust_status_t status = LS_TRUST_OK;

    if ((ls_trust_util != NULL) && (ls_trust_application_open != 0U)) {
        ls_trust_lib_status = OPTIGA_LIB_BUSY;
        OPTIGA_UTIL_SET_COMMS_PROTECTION_LEVEL(ls_trust_util, OPTIGA_COMMS_FULL_PROTECTION);
        submitted = optiga_util_close_application(ls_trust_util, (hibernate != 0U) ? 1 : 0);
        status = ls_trust_await(submitted);
        ls_trust_application_open = 0U;
    }

    if (ls_trust_crypt != NULL) {
        (void)optiga_crypt_destroy(ls_trust_crypt);
        ls_trust_crypt = NULL;
    }
    if (ls_trust_util != NULL) {
        (void)optiga_util_destroy(ls_trust_util);
        ls_trust_util = NULL;
    }

    return status;
}

ls_trust_status_t ls_trust_read_uid(uint8_t *p_uid, uint16_t *p_uid_length) {
    return ls_trust_read_object(LS_OID_COPROCESSOR_UID, p_uid, p_uid_length);
}

ls_trust_status_t ls_trust_read_device_certificate(uint8_t *p_cert, uint16_t *p_cert_length) {
    return ls_trust_read_object(LS_OID_DEVICE_CERTIFICATE, p_cert, p_cert_length);
}

ls_trust_status_t ls_trust_random(uint8_t *p_buffer, uint16_t length) {
    optiga_lib_status_t submitted;

    if ((p_buffer == NULL) || (length == 0U)) {
        return LS_TRUST_ERR_PARAM;
    }
    if (ls_trust_crypt == NULL) {
        return LS_TRUST_ERR_OPTIGA;
    }

    ls_trust_lib_status = OPTIGA_LIB_BUSY;
    OPTIGA_CRYPT_SET_COMMS_PROTECTION_LEVEL(ls_trust_crypt, OPTIGA_COMMS_FULL_PROTECTION);
    submitted = optiga_crypt_random(ls_trust_crypt, OPTIGA_RNG_TYPE_TRNG, p_buffer, length);

    return ls_trust_await(submitted);
}

ls_trust_status_t ls_trust_authenticate(const uint8_t *p_challenge,
                                        uint16_t challenge_length,
                                        uint8_t *p_signature,
                                        uint16_t *p_signature_length) {
    uint8_t digest[LS_TRUST_DIGEST_LENGTH];
    hash_data_from_host_t hash_input;
    optiga_lib_status_t submitted;
    ls_trust_status_t status;
    uint8_t sec = 0U;

    if ((p_challenge == NULL) || (challenge_length == 0U) || (p_signature == NULL)
        || (p_signature_length == NULL) || (*p_signature_length < LS_TRUST_SIGNATURE_MAX_LENGTH)) {
        return LS_TRUST_ERR_PARAM;
    }
    if (ls_trust_crypt == NULL) {
        return LS_TRUST_ERR_OPTIGA;
    }

    /* This call is about to spend a "Private key use" security event
     * ([53] section 4.6.1, Table 65).  Check the budget first: issuing into a
     * throttled part turns a 65 ms operation into a multi-second one, and a
     * servo blocked for seconds at boot is a fault, not a delay. */
    status = ls_trust_read_sec(&sec);
    if (status != LS_TRUST_OK) {
        return status;
    }
    if (sec >= LS_TRUST_SEC_WARNING_LEVEL) {
        return LS_TRUST_ERR_THROTTLED;
    }

    /* 1. Hash the challenge on the part.  optiga_crypt_ecdsa_sign takes a
     *    digest, not a message ([57] optiga_crypt.h), and hashing costs no
     *    security event. */
    hash_input.buffer = p_challenge;
    hash_input.length = challenge_length;

    ls_trust_lib_status = OPTIGA_LIB_BUSY;
    OPTIGA_CRYPT_SET_COMMS_PROTECTION_LEVEL(ls_trust_crypt, OPTIGA_COMMS_FULL_PROTECTION);
    submitted = optiga_crypt_hash(ls_trust_crypt,
                                  OPTIGA_HASH_TYPE_SHA_256,
                                  OPTIGA_CRYPT_HOST_DATA,
                                  &hash_input,
                                  digest);
    status = ls_trust_await(submitted);
    if (status != LS_TRUST_OK) {
        return status;
    }

    /* 2. Sign with the fab-provisioned identity key in 0xE0F0. */
    ls_trust_lib_status = OPTIGA_LIB_BUSY;
    OPTIGA_CRYPT_SET_COMMS_PROTECTION_LEVEL(ls_trust_crypt, OPTIGA_COMMS_FULL_PROTECTION);
    submitted = optiga_crypt_ecdsa_sign(ls_trust_crypt,
                                        digest,
                                        (uint8_t)sizeof(digest),
                                        OPTIGA_KEY_ID_E0F0,
                                        p_signature,
                                        p_signature_length);
    status = ls_trust_await(submitted);

    ls_trust_secure_wipe(digest, (uint16_t)sizeof(digest));
    return status;
}

ls_trust_status_t ls_trust_establish_session_key(const uint8_t *p_peer_public_key,
                                                 uint16_t peer_public_key_length,
                                                 const uint8_t *p_label,
                                                 uint16_t label_length,
                                                 const uint8_t *p_nonce,
                                                 uint8_t *p_own_public_key,
                                                 uint16_t *p_own_public_key_length,
                                                 uint8_t *p_session_key) {
    public_key_from_host_t peer_key;
    optiga_lib_status_t submitted;
    ls_trust_status_t status;

    if ((p_peer_public_key == NULL) || (peer_public_key_length == 0U) || (p_label == NULL)
        || (label_length == 0U) || (p_nonce == NULL) || (p_own_public_key == NULL)
        || (p_own_public_key_length == NULL) || (p_session_key == NULL)) {
        return LS_TRUST_ERR_PARAM;
    }
    if (ls_trust_crypt == NULL) {
        return LS_TRUST_ERR_OPTIGA;
    }

    /* 1. Ephemeral key pair.  The private half stays in a session context
     *    (OPTIGA_KEY_ID_SESSION_BASED) and is never exported, which is what
     *    keeps every later step out of the security-event carve-outs of
     *    [53] section 4.6.1.  Curve is P-256, the floor set by
     *    PCB/servo-bus-security-protocol.md section 4.9. */
    {
        optiga_key_id_t ephemeral_key = OPTIGA_KEY_ID_SESSION_BASED;

        ls_trust_lib_status = OPTIGA_LIB_BUSY;
        OPTIGA_CRYPT_SET_COMMS_PROTECTION_LEVEL(ls_trust_crypt, OPTIGA_COMMS_FULL_PROTECTION);
        submitted = optiga_crypt_ecc_generate_keypair(ls_trust_crypt,
                                                      OPTIGA_ECC_CURVE_NIST_P_256,
                                                      (uint8_t)OPTIGA_KEY_USAGE_KEY_AGREEMENT,
                                                      FALSE, /* do not export the private key */
                                                      &ephemeral_key,
                                                      p_own_public_key,
                                                      p_own_public_key_length);
        status = ls_trust_await(submitted);
        if (status != LS_TRUST_OK) {
            return status;
        }

        /* 2. ECDH against the controller's ephemeral public key.  export_to_host
         *    is FALSE, so the shared secret stays in the session context and the
         *    raw secret never crosses the I2C bus at all — belt and braces on
         *    top of the Shielded Connection. */
        /* [57] types `public_key_from_host_t.public_key` as non-const even
         * though the library only reads it, so the const has to come off
         * somewhere.  The union keeps that in one labelled place. */
        {
            union {
                const uint8_t *as_const;
                uint8_t *as_mutable;
            } key_cast;

            key_cast.as_const = p_peer_public_key;
            peer_key.public_key = key_cast.as_mutable;
        }
        peer_key.length = peer_public_key_length;
        peer_key.key_type = (uint8_t)OPTIGA_ECC_CURVE_NIST_P_256;

        ls_trust_lib_status = OPTIGA_LIB_BUSY;
        OPTIGA_CRYPT_SET_COMMS_PROTECTION_LEVEL(ls_trust_crypt, OPTIGA_COMMS_FULL_PROTECTION);
        submitted = optiga_crypt_ecdh(ls_trust_crypt, ephemeral_key, &peer_key, FALSE, NULL);
        status = ls_trust_await(submitted);
        if (status != LS_TRUST_OK) {
            return status;
        }

        /* 3. Derive the bus key from the session-context secret.
         *
         *    The nonce is the seed, which is what supplies the freshness
         *    [53] section 6.6.3 says the Shielded Connection does not provide by
         *    itself.  export_to_host is TRUE here — and only here — because the
         *    MCU's AESADV engine, not U7, authenticates bus frames
         *    (PCB/servo-bus-security-protocol.md section 4.7), so it needs the
         *    key material. */
        ls_trust_lib_status = OPTIGA_LIB_BUSY;
        OPTIGA_CRYPT_SET_COMMS_PROTECTION_LEVEL(ls_trust_crypt, OPTIGA_COMMS_FULL_PROTECTION);
        submitted = optiga_crypt_tls_prf_sha256(ls_trust_crypt,
                                                (uint16_t)ephemeral_key,
                                                p_label,
                                                label_length,
                                                p_nonce,
                                                LS_TRUST_NONCE_LENGTH,
                                                LS_TRUST_SESSION_KEY_LENGTH,
                                                TRUE,
                                                p_session_key);
        status = ls_trust_await(submitted);
        if (status != LS_TRUST_OK) {
            ls_trust_secure_wipe(p_session_key, LS_TRUST_SESSION_KEY_LENGTH);
            return status;
        }
    }

    return LS_TRUST_OK;
}
