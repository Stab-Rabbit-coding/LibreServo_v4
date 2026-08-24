/**
 * @file    ls_crypto_backend_openssl.c
 * @brief   HOST-TEST-ONLY binding of `ls_crypto_backend.h` to OpenSSL.
 *
 * Not a candidate for the target build.  It exists so the constructions in
 * `firmware/pal/ls_pal_crypt.c` can be exercised on a development host against
 * a widely reviewed AES and HMAC implementation, per `firmware/tests/README.md`.
 *
 * The target binding is a separate decision — see "Binding the crypto backend"
 * in `firmware/README.md` and TODO.md 7.6.
 *
 * Written by Claude Opus 5 (claude-opus-5) under human direction, 2026-08-23.
 */

#include "ls_crypto_backend.h"
#include <openssl/evp.h>
#include <openssl/hmac.h>
#include <string.h>

int ls_crypto_aes128_encrypt_block(const uint8_t *k, const uint8_t *in, uint8_t *out) {
    EVP_CIPHER_CTX *c = EVP_CIPHER_CTX_new();
    int len = 0, rc = LS_CRYPTO_ERROR;
    if (!c) return rc;
    if (EVP_EncryptInit_ex(c, EVP_aes_128_ecb(), NULL, k, NULL) == 1) {
        EVP_CIPHER_CTX_set_padding(c, 0);
        if (EVP_EncryptUpdate(c, out, &len, in, 16) == 1 && len == 16) rc = LS_CRYPTO_OK;
    }
    EVP_CIPHER_CTX_free(c);
    return rc;
}

int ls_crypto_hmac_sha256(const uint8_t *key, uint16_t klen, const uint8_t *msg,
                          uint16_t mlen, uint8_t *mac) {
    unsigned int outlen = 0;
    if (!HMAC(EVP_sha256(), key, (int)klen, msg, mlen, mac, &outlen)) return LS_CRYPTO_ERROR;
    return (outlen == 32) ? LS_CRYPTO_OK : LS_CRYPTO_ERROR;
}
