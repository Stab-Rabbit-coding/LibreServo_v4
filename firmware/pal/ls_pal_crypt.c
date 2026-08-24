/**
 * @file    ls_pal_crypt.c
 * @brief   Host-side cryptography the OPTIGA(TM) Trust M Shielded Connection
 *          requires — AES-128-CCM and the TLS 1.2 PRF.
 *
 * Implements [57] `include/pal/pal_crypt.h`.
 *
 * ------------------------------------------------------------------------
 * WHY THIS FILE EXISTS
 * ------------------------------------------------------------------------
 * Enabling the Shielded Connection is not free on the host side, and the cost
 * is not obvious from the datasheets.  Defining
 * OPTIGA_COMMS_SHIELDED_CONNECTION obliges the host to supply three
 * primitives — `pal_crypt_tls_prf_sha256`, `pal_crypt_encrypt_aes128_ccm` and
 * `pal_crypt_decrypt_aes128_ccm` — because the presentation layer of the IFX
 * I2C protocol [54] protects each APDU with AES-128-CCM under a key derived
 * from the platform binding secret with the TLS 1.2 PRF.  Without them the
 * image does not link.
 *
 * The constructions below are implemented here from their published
 * specifications.  The underlying AES block cipher and HMAC-SHA256 are NOT:
 * they come from `ls_crypto_backend.h`, for the reasons set out in that header.
 *
 * References (see REFERENCES.md):
 *   [54] Infineon, IFX I2C Protocol, Protocol Specification, Rev 2.03.
 *   [57] Infineon, OPTIGA(TM) Trust M Host Library for C, Ver 5.8.1.
 *   [58] IETF RFC 5246, The TLS Protocol Version 1.2, section 5, pp. 14-15.
 *   [59] NIST SP 800-38C, CCM Mode, sections 6.1, 6.2 and Appendix A.
 *
 * Written by Claude Opus 5 (claude-opus-5) under human direction, 2026-08-23.
 */

#include "pal_crypt.h"

#include <string.h>

#include "ls_crypto_backend.h"

/** Longest label + seed this port will assemble for the PRF, in bytes. */
#define LS_PRF_LABEL_SEED_MAX   (128U)

/** Longest CCM nonce this port accepts.  [59] Appendix A.1: n is an element of
 *  {7, 8, 9, 10, 11, 12, 13}. */
#define LS_CCM_NONCE_MIN        (7U)
#define LS_CCM_NONCE_MAX        (13U)

/** [59] Appendix A.1: n + q = 15. */
#define LS_CCM_NQ_SUM           (15U)

/**
 * @brief XOR @p length bytes of @p p_source into @p p_destination.
 */
static void ls_xor_into(uint8_t *p_destination, const uint8_t *p_source, uint16_t length) {
    uint16_t i;

    for (i = 0U; i < length; i++) {
        p_destination[i] ^= p_source[i];
    }
}

/* ========================================================================
 * TLS 1.2 PRF with SHA-256 — [58] section 5, pp. 14-15
 *
 *     P_hash(secret, seed) = HMAC_hash(secret, A(1) + seed) +
 *                            HMAC_hash(secret, A(2) + seed) + ...
 *     A(0) = seed
 *     A(i) = HMAC_hash(secret, A(i-1))
 *     PRF(secret, label, seed) = P_SHA256(secret, label + seed)
 *
 * [58] is explicit that the label is included "in the exact form it is given
 * without a length byte or trailing null character", which is why the
 * concatenation below copies label and seed verbatim.
 * ======================================================================== */

pal_status_t pal_crypt_tls_prf_sha256(pal_crypt_t *p_pal_crypt,
                                      const uint8_t *p_secret,
                                      uint16_t secret_length,
                                      const uint8_t *p_label,
                                      uint16_t label_length,
                                      const uint8_t *p_seed,
                                      uint16_t seed_length,
                                      uint8_t *p_derived_key,
                                      uint16_t derived_key_length) {
    /* label + seed, i.e. the combined "seed" argument to P_hash. */
    uint8_t label_seed[LS_PRF_LABEL_SEED_MAX];
    /* A(i), and then A(i) || label_seed for the second HMAC of each round. */
    uint8_t a_value[LS_SHA256_DIGEST_SIZE];
    uint8_t hmac_input[LS_SHA256_DIGEST_SIZE + LS_PRF_LABEL_SEED_MAX];
    uint8_t block[LS_SHA256_DIGEST_SIZE];
    uint16_t label_seed_length;
    uint16_t produced = 0U;
    pal_status_t status = PAL_STATUS_FAILURE;

    (void)p_pal_crypt;

    if ((p_secret == NULL) || (secret_length == 0U) || (p_label == NULL) || (p_seed == NULL)
        || (p_derived_key == NULL) || (derived_key_length == 0U)) {
        return PAL_STATUS_INVALID_INPUT;
    }

    label_seed_length = (uint16_t)(label_length + seed_length);
    if (label_seed_length > (uint16_t)sizeof(label_seed)) {
        /* Reject rather than truncate: a silently shortened seed would still
         * produce a key, and both ends would then disagree in a way that looks
         * like a bus fault rather than a bug. */
        return PAL_STATUS_INVALID_INPUT;
    }

    if (label_length > 0U) {
        (void)memcpy(label_seed, p_label, label_length);
    }
    if (seed_length > 0U) {
        (void)memcpy(&label_seed[label_length], p_seed, seed_length);
    }

    /* A(1) = HMAC(secret, A(0)) where A(0) = label + seed. */
    if (ls_crypto_hmac_sha256(p_secret, secret_length, label_seed, label_seed_length, a_value)
        != LS_CRYPTO_OK) {
        goto cleanup;
    }

    while (produced < derived_key_length) {
        uint16_t copy_length;

        /* HMAC(secret, A(i) + seed) */
        (void)memcpy(hmac_input, a_value, sizeof(a_value));
        (void)memcpy(&hmac_input[sizeof(a_value)], label_seed, label_seed_length);

        if (ls_crypto_hmac_sha256(p_secret,
                                  secret_length,
                                  hmac_input,
                                  (uint16_t)(sizeof(a_value) + label_seed_length),
                                  block)
            != LS_CRYPTO_OK) {
            goto cleanup;
        }

        copy_length = (uint16_t)(derived_key_length - produced);
        if (copy_length > (uint16_t)sizeof(block)) {
            copy_length = (uint16_t)sizeof(block);
        }
        (void)memcpy(&p_derived_key[produced], block, copy_length);
        produced = (uint16_t)(produced + copy_length);

        /* A(i+1) = HMAC(secret, A(i)) */
        if (ls_crypto_hmac_sha256(p_secret, secret_length, a_value, sizeof(a_value), a_value)
            != LS_CRYPTO_OK) {
            goto cleanup;
        }
    }

    status = PAL_STATUS_SUCCESS;

cleanup:
    /* These buffers held key-derived material; clear them whether or not the
     * derivation succeeded. */
    (void)memset(label_seed, 0, sizeof(label_seed));
    (void)memset(a_value, 0, sizeof(a_value));
    (void)memset(hmac_input, 0, sizeof(hmac_input));
    (void)memset(block, 0, sizeof(block));

    if (status != PAL_STATUS_SUCCESS) {
        (void)memset(p_derived_key, 0, derived_key_length);
    }
    return status;
}

/* ========================================================================
 * AES-128-CCM — [59] sections 6.1, 6.2 and Appendix A
 * ======================================================================== */

/**
 * @brief Build B0, the first CBC-MAC block.  [59] Appendix A.2.1, Tables 1 and 2.
 *
 * Flags octet: bit 6 = Adata (1 when a > 0), bits 5-3 = (t-2)/2, bits 2-0 = q-1.
 * Octets 1..15-q hold the nonce; octets 16-q..15 hold Q = [p] in q octets.
 */
static void ls_ccm_build_b0(uint8_t *p_b0,
                            const uint8_t *p_nonce,
                            uint8_t nonce_length,
                            uint16_t associated_data_length,
                            uint16_t payload_length,
                            uint8_t mac_size) {
    const uint8_t q = (uint8_t)(LS_CCM_NQ_SUM - nonce_length);
    uint8_t i;

    (void)memset(p_b0, 0, LS_AES_BLOCK_SIZE);

    p_b0[0] = (uint8_t)(((associated_data_length > 0U) ? 0x40U : 0x00U)
                        | (uint8_t)((((mac_size - 2U) / 2U) & 0x07U) << 3)
                        | (uint8_t)((q - 1U) & 0x07U));

    (void)memcpy(&p_b0[1], p_nonce, nonce_length);

    /* Q = [p]_8q, big-endian, right-aligned in the last q octets. */
    for (i = 0U; i < q; i++) {
        const uint8_t shift = (uint8_t)(8U * i);

        if (shift < 16U) {
            p_b0[LS_AES_BLOCK_SIZE - 1U - i] = (uint8_t)((payload_length >> shift) & 0xFFU);
        }
    }
}

/**
 * @brief Build counter block Ctr_i.  [59] Appendix A.3, Tables 3 and 4.
 *
 * Flags octet: bits 7-3 all zero (which is what keeps every Ctr distinct from
 * B0, whose flags octet always has a non-zero t field), bits 2-0 = q-1.
 */
static void ls_ccm_build_ctr(uint8_t *p_ctr,
                             const uint8_t *p_nonce,
                             uint8_t nonce_length,
                             uint32_t index) {
    const uint8_t q = (uint8_t)(LS_CCM_NQ_SUM - nonce_length);
    uint8_t i;

    (void)memset(p_ctr, 0, LS_AES_BLOCK_SIZE);
    p_ctr[0] = (uint8_t)((q - 1U) & 0x07U);
    (void)memcpy(&p_ctr[1], p_nonce, nonce_length);

    for (i = 0U; i < q; i++) {
        const uint8_t shift = (uint8_t)(8U * i);

        if (shift < 32U) {
            p_ctr[LS_AES_BLOCK_SIZE - 1U - i] = (uint8_t)((index >> shift) & 0xFFU);
        }
    }
}

/**
 * @brief Run the CBC-MAC of [59] section 6.1 steps 1-4 over (N, A, P) and return T.
 *
 * @param[out] p_tag  #LS_AES_BLOCK_SIZE bytes; the caller takes MSB_Tlen of it.
 */
static int ls_ccm_cbc_mac(const uint8_t *p_key,
                          const uint8_t *p_nonce,
                          uint8_t nonce_length,
                          const uint8_t *p_associated_data,
                          uint16_t associated_data_length,
                          const uint8_t *p_payload,
                          uint16_t payload_length,
                          uint8_t mac_size,
                          uint8_t *p_tag) {
    uint8_t y[LS_AES_BLOCK_SIZE];
    uint8_t b[LS_AES_BLOCK_SIZE];
    uint16_t offset;

    /* Y0 = CIPH_K(B0). */
    ls_ccm_build_b0(b, p_nonce, nonce_length, associated_data_length, payload_length, mac_size);
    if (ls_crypto_aes128_encrypt_block(p_key, b, y) != LS_CRYPTO_OK) {
        return LS_CRYPTO_ERROR;
    }

    /* Associated data blocks.  [59] Appendix A.2.2: for 0 < a < 2^16 - 2^8 the
     * length is encoded as [a]_16, two octets, prepended to A, and the result
     * is zero-padded to a block boundary.  This port never presents associated
     * data anywhere near 2^16 - 2^8 bytes — the whole comms buffer is 1557
     * bytes ([57] OPTIGA_MAX_COMMS_BUFFER_SIZE) — so only the two-octet case is
     * implemented, and anything larger is rejected by the caller rather than
     * mis-encoded. */
    if (associated_data_length > 0U) {
        uint16_t consumed = 0U;

        (void)memset(b, 0, sizeof(b));
        b[0] = (uint8_t)((associated_data_length >> 8) & 0xFFU);
        b[1] = (uint8_t)(associated_data_length & 0xFFU);

        {
            uint16_t first_chunk = (uint16_t)(LS_AES_BLOCK_SIZE - 2U);

            if (first_chunk > associated_data_length) {
                first_chunk = associated_data_length;
            }
            (void)memcpy(&b[2], p_associated_data, first_chunk);
            consumed = first_chunk;
        }

        ls_xor_into(b, y, LS_AES_BLOCK_SIZE);
        if (ls_crypto_aes128_encrypt_block(p_key, b, y) != LS_CRYPTO_OK) {
            return LS_CRYPTO_ERROR;
        }

        while (consumed < associated_data_length) {
            uint16_t chunk = (uint16_t)(associated_data_length - consumed);

            if (chunk > LS_AES_BLOCK_SIZE) {
                chunk = LS_AES_BLOCK_SIZE;
            }
            (void)memset(b, 0, sizeof(b));
            (void)memcpy(b, &p_associated_data[consumed], chunk);

            ls_xor_into(b, y, LS_AES_BLOCK_SIZE);
            if (ls_crypto_aes128_encrypt_block(p_key, b, y) != LS_CRYPTO_OK) {
                return LS_CRYPTO_ERROR;
            }
            consumed = (uint16_t)(consumed + chunk);
        }
    }

    /* Payload blocks, zero-padded to a block boundary ([59] Appendix A.2.3). */
    for (offset = 0U; offset < payload_length; offset = (uint16_t)(offset + LS_AES_BLOCK_SIZE)) {
        uint16_t chunk = (uint16_t)(payload_length - offset);

        if (chunk > LS_AES_BLOCK_SIZE) {
            chunk = LS_AES_BLOCK_SIZE;
        }
        (void)memset(b, 0, sizeof(b));
        (void)memcpy(b, &p_payload[offset], chunk);

        ls_xor_into(b, y, LS_AES_BLOCK_SIZE);
        if (ls_crypto_aes128_encrypt_block(p_key, b, y) != LS_CRYPTO_OK) {
            return LS_CRYPTO_ERROR;
        }
    }

    (void)memcpy(p_tag, y, LS_AES_BLOCK_SIZE);
    (void)memset(y, 0, sizeof(y));
    (void)memset(b, 0, sizeof(b));
    return LS_CRYPTO_OK;
}

/**
 * @brief Apply CTR-mode keystream from Ctr_1 onward to @p p_data in place.
 */
static int ls_ccm_ctr_crypt(const uint8_t *p_key,
                            const uint8_t *p_nonce,
                            uint8_t nonce_length,
                            uint8_t *p_data,
                            uint16_t length) {
    uint8_t ctr[LS_AES_BLOCK_SIZE];
    uint8_t s[LS_AES_BLOCK_SIZE];
    uint16_t offset;
    uint32_t index = 1U;

    for (offset = 0U; offset < length; offset = (uint16_t)(offset + LS_AES_BLOCK_SIZE)) {
        uint16_t chunk = (uint16_t)(length - offset);

        if (chunk > LS_AES_BLOCK_SIZE) {
            chunk = LS_AES_BLOCK_SIZE;
        }
        ls_ccm_build_ctr(ctr, p_nonce, nonce_length, index);
        if (ls_crypto_aes128_encrypt_block(p_key, ctr, s) != LS_CRYPTO_OK) {
            return LS_CRYPTO_ERROR;
        }
        ls_xor_into(&p_data[offset], s, chunk);
        index++;
    }

    (void)memset(ctr, 0, sizeof(ctr));
    (void)memset(s, 0, sizeof(s));
    return LS_CRYPTO_OK;
}

/**
 * @brief Validate the CCM parameter set against [59] Appendix A.1.
 */
static pal_status_t ls_ccm_check_parameters(const uint8_t *p_nonce,
                                            uint16_t nonce_length,
                                            uint16_t associated_data_length,
                                            uint8_t mac_size) {
    if (p_nonce == NULL) {
        return PAL_STATUS_INVALID_INPUT;
    }
    /* n is an element of {7, ..., 13}. */
    if ((nonce_length < LS_CCM_NONCE_MIN) || (nonce_length > LS_CCM_NONCE_MAX)) {
        return PAL_STATUS_INVALID_INPUT;
    }
    /* t is an element of {4, 6, 8, 10, 12, 14, 16}. */
    if ((mac_size < 4U) || (mac_size > 16U) || ((mac_size % 2U) != 0U)) {
        return PAL_STATUS_INVALID_INPUT;
    }
    /* Only the two-octet associated-data length encoding of [59] Appendix A.2.2
     * is implemented — see the note in ls_ccm_cbc_mac. */
    if (associated_data_length >= (uint16_t)(0xFF00U)) {
        return PAL_STATUS_INVALID_INPUT;
    }
    return PAL_STATUS_SUCCESS;
}

pal_status_t pal_crypt_encrypt_aes128_ccm(pal_crypt_t *p_pal_crypt,
                                          const uint8_t *p_plain_text,
                                          uint16_t plain_text_length,
                                          const uint8_t *p_encrypt_key,
                                          const uint8_t *p_nonce,
                                          uint16_t nonce_length,
                                          const uint8_t *p_associated_data,
                                          uint16_t associated_data_length,
                                          uint8_t mac_size,
                                          uint8_t *p_cipher_text) {
    uint8_t tag[LS_AES_BLOCK_SIZE];
    uint8_t s0[LS_AES_BLOCK_SIZE];
    uint8_t ctr0[LS_AES_BLOCK_SIZE];
    pal_status_t status;

    (void)p_pal_crypt;

    if ((p_plain_text == NULL) || (p_encrypt_key == NULL) || (p_cipher_text == NULL)) {
        return PAL_STATUS_INVALID_INPUT;
    }
    status = ls_ccm_check_parameters(p_nonce, nonce_length, associated_data_length, mac_size);
    if (status != PAL_STATUS_SUCCESS) {
        return status;
    }

    /* Steps 1-4: T = MSB_Tlen(CBC-MAC over the formatted (N, A, P)). */
    if (ls_ccm_cbc_mac(p_encrypt_key,
                       p_nonce,
                       (uint8_t)nonce_length,
                       p_associated_data,
                       associated_data_length,
                       p_plain_text,
                       plain_text_length,
                       mac_size,
                       tag)
        != LS_CRYPTO_OK) {
        return PAL_STATUS_FAILURE;
    }

    /* Steps 5-8: C = (P XOR MSB_Plen(S)) || (T XOR MSB_Tlen(S0)).
     * The payload uses Ctr_1 onward; the tag is masked with S0 = CIPH_K(Ctr_0). */
    (void)memcpy(p_cipher_text, p_plain_text, plain_text_length);
    if (ls_ccm_ctr_crypt(p_encrypt_key,
                         p_nonce,
                         (uint8_t)nonce_length,
                         p_cipher_text,
                         plain_text_length)
        != LS_CRYPTO_OK) {
        return PAL_STATUS_FAILURE;
    }

    ls_ccm_build_ctr(ctr0, p_nonce, (uint8_t)nonce_length, 0U);
    if (ls_crypto_aes128_encrypt_block(p_encrypt_key, ctr0, s0) != LS_CRYPTO_OK) {
        return PAL_STATUS_FAILURE;
    }
    ls_xor_into(tag, s0, mac_size);
    (void)memcpy(&p_cipher_text[plain_text_length], tag, mac_size);

    (void)memset(tag, 0, sizeof(tag));
    (void)memset(s0, 0, sizeof(s0));
    (void)memset(ctr0, 0, sizeof(ctr0));
    return PAL_STATUS_SUCCESS;
}

pal_status_t pal_crypt_decrypt_aes128_ccm(pal_crypt_t *p_pal_crypt,
                                          const uint8_t *p_cipher_text,
                                          uint16_t cipher_text_length,
                                          const uint8_t *p_decrypt_key,
                                          const uint8_t *p_nonce,
                                          uint16_t nonce_length,
                                          const uint8_t *p_associated_data,
                                          uint16_t associated_data_length,
                                          uint8_t mac_size,
                                          uint8_t *p_plain_text) {
    uint8_t received_tag[LS_AES_BLOCK_SIZE];
    uint8_t computed_tag[LS_AES_BLOCK_SIZE];
    uint8_t s0[LS_AES_BLOCK_SIZE];
    uint8_t ctr0[LS_AES_BLOCK_SIZE];
    uint16_t payload_length;
    uint8_t difference = 0U;
    uint8_t i;
    pal_status_t status;

    (void)p_pal_crypt;

    if ((p_cipher_text == NULL) || (p_decrypt_key == NULL) || (p_plain_text == NULL)) {
        return PAL_STATUS_INVALID_INPUT;
    }
    status = ls_ccm_check_parameters(p_nonce, nonce_length, associated_data_length, mac_size);
    if (status != PAL_STATUS_SUCCESS) {
        return status;
    }
    /* [59] section 6.2 step 1: "If Clen <= Tlen, then return INVALID." */
    if (cipher_text_length <= (uint16_t)mac_size) {
        return PAL_STATUS_FAILURE;
    }
    payload_length = (uint16_t)(cipher_text_length - mac_size);

    /* Steps 2-5: recover P. */
    (void)memcpy(p_plain_text, p_cipher_text, payload_length);
    if (ls_ccm_ctr_crypt(p_decrypt_key,
                         p_nonce,
                         (uint8_t)nonce_length,
                         p_plain_text,
                         payload_length)
        != LS_CRYPTO_OK) {
        return PAL_STATUS_FAILURE;
    }

    /* Step 6: T = LSB_Tlen(C) XOR MSB_Tlen(S0). */
    ls_ccm_build_ctr(ctr0, p_nonce, (uint8_t)nonce_length, 0U);
    if (ls_crypto_aes128_encrypt_block(p_decrypt_key, ctr0, s0) != LS_CRYPTO_OK) {
        return PAL_STATUS_FAILURE;
    }
    (void)memset(received_tag, 0, sizeof(received_tag));
    (void)memcpy(received_tag, &p_cipher_text[payload_length], mac_size);
    ls_xor_into(received_tag, s0, mac_size);

    /* Steps 7-9: recompute the MAC over the recovered plaintext. */
    if (ls_ccm_cbc_mac(p_decrypt_key,
                       p_nonce,
                       (uint8_t)nonce_length,
                       p_associated_data,
                       associated_data_length,
                       p_plain_text,
                       payload_length,
                       mac_size,
                       computed_tag)
        != LS_CRYPTO_OK) {
        return PAL_STATUS_FAILURE;
    }

    /* Step 10: constant-time comparison.  An early-exit `memcmp` here would
     * leak how many leading tag bytes an attacker got right, which is enough to
     * forge a tag one byte at a time. */
    for (i = 0U; i < mac_size; i++) {
        difference |= (uint8_t)(received_tag[i] ^ computed_tag[i]);
    }

    (void)memset(received_tag, 0, sizeof(received_tag));
    (void)memset(computed_tag, 0, sizeof(computed_tag));
    (void)memset(s0, 0, sizeof(s0));
    (void)memset(ctr0, 0, sizeof(ctr0));

    if (difference != 0U) {
        /* INVALID.  Destroy the recovered plaintext rather than returning
         * unauthenticated data alongside a failure code that a caller might
         * forget to check. */
        (void)memset(p_plain_text, 0, payload_length);
        return PAL_STATUS_FAILURE;
    }

    return PAL_STATUS_SUCCESS;
}

pal_status_t pal_crypt_version(uint8_t *p_crypt_lib_version_info, uint16_t *length) {
    static const char version[] = "LibreServo v4 pal_crypt (RFC5246 PRF, SP800-38C CCM)";
    const uint16_t version_length = (uint16_t)(sizeof(version) - 1U);

    if ((p_crypt_lib_version_info == NULL) || (length == NULL)) {
        return PAL_STATUS_INVALID_INPUT;
    }
    if (*length < version_length) {
        return PAL_STATUS_FAILURE;
    }
    (void)memcpy(p_crypt_lib_version_info, version, version_length);
    *length = version_length;
    return PAL_STATUS_SUCCESS;
}
