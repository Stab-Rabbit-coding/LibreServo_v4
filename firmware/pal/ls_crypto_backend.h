/**
 * @file    ls_crypto_backend.h
 * @brief   The two primitives LibreServo v4 needs from a vetted cryptographic
 *          implementation, and nothing more.
 *
 * `ls_pal_crypt.c` builds AES-128-CCM and the TLS 1.2 PRF on top of these.  It
 * implements the *constructions* — which are specified, reviewable and
 * testable against published vectors — and deliberately implements neither the
 * AES block cipher nor SHA-256 itself.
 *
 * That division is the point of this header.  Hand-rolling a block cipher or a
 * hash into a safety-relevant product is exactly the mistake that
 * `AGENTS.md` section 1 exists to prevent, and neither primitive has any
 * LibreServo-specific content: any correct implementation will do.
 *
 * Two bindings are anticipated (see firmware/README.md, "Binding the crypto
 * backend"); choosing between them is tracked as TODO.md 7.6:
 *
 *   1. **MSPM0 AESADV for AES, software SHA-256.**  [46] section 8.20 gives the
 *      MSPM0G3518-Q1 an on-die AES engine, and [49] Table 5-3 measures it at
 *      76 cycles / 0.95 us per 128-bit block at 80 MHz.  The part has no SHA
 *      accelerator, so SHA-256 would come from software.
 *   2. **Mbed TLS**, which is what the upstream reference PALs of [57] use
 *      (`examples/utilities/authenticate_chip/pal_crypt_mbedtls.c`).
 *
 * Whichever is chosen, the implementation must be constant-time with respect to
 * key material — `ls_pal_crypt.c`'s own MAC comparison already is.
 *
 * References (see REFERENCES.md):
 *   [46] TI, MSPM0G3519-Q1/MSPM0G3518-Q1 Datasheet, SLASFA6B.
 *   [49] TI, MSPM0 application note SLAAE29A.
 *   [57] Infineon, OPTIGA(TM) Trust M Host Library for C, Ver 5.8.1.
 *   [58] IETF RFC 5246, The TLS Protocol Version 1.2.
 *   [59] NIST SP 800-38C, CCM Mode for Authentication and Confidentiality.
 *
 * Written by Claude Opus 5 (claude-opus-5) under human direction, 2026-08-23.
 */

#ifndef LS_CRYPTO_BACKEND_H_
#define LS_CRYPTO_BACKEND_H_

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/** AES block size in bytes. */
#define LS_AES_BLOCK_SIZE       (16U)
/** AES-128 key size in bytes. */
#define LS_AES128_KEY_SIZE      (16U)
/** SHA-256 digest size in bytes. */
#define LS_SHA256_DIGEST_SIZE   (32U)

/** Backend result codes. */
#define LS_CRYPTO_OK            (0)
#define LS_CRYPTO_ERROR         (-1)

/**
 * @brief Encrypt one 16-byte block with AES-128.
 *
 * The forward cipher function `CIPH_K` of [59].  CCM never uses AES
 * decryption — both the CBC-MAC and the counter-mode keystream are built from
 * the forward direction only — so no decrypt entry point is declared here.
 *
 * @param p_key    #LS_AES128_KEY_SIZE key bytes.
 * @param p_input  #LS_AES_BLOCK_SIZE plaintext block.
 * @param[out] p_output #LS_AES_BLOCK_SIZE ciphertext block.  May alias
 *                      @p p_input.
 */
int ls_crypto_aes128_encrypt_block(const uint8_t *p_key,
                                   const uint8_t *p_input,
                                   uint8_t *p_output);

/**
 * @brief HMAC-SHA256 over one message.
 *
 * @param p_key         MAC key.
 * @param key_length    Its length in bytes.
 * @param p_message     Message.
 * @param message_length Its length in bytes.
 * @param[out] p_mac    #LS_SHA256_DIGEST_SIZE output bytes.
 */
int ls_crypto_hmac_sha256(const uint8_t *p_key,
                          uint16_t key_length,
                          const uint8_t *p_message,
                          uint16_t message_length,
                          uint8_t *p_mac);

#ifdef __cplusplus
}
#endif

#endif /* LS_CRYPTO_BACKEND_H_ */
